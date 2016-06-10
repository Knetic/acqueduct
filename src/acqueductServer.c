#include <sys/epoll.h>
#include <fcntl.h>

#include "_acqueduct.h"
#include "voidlist.h"
#include "CompressionBufferPair.h"

#define RECEIVE_BUFFER_SIZE 65536
#define RECEIVE_UNCOMPRESSED_SIZE RECEIVE_BUFFER_SIZE * 4
#define MAX_POLLED_DESCRIPTORS 2048

/*
	Represents a single connection to an Acqueduct client.
*/
typedef struct AcqueductClientConnection
{
	int socketDescriptor;
} AcqueductClientConnection;

static void receiveAcqueductData(AcqueductClientConnection* connection, CompressionBufferPair* buffers);
static void closeAcqueductSocket(VoidList* clientList, AcqueductClientConnection* clientConnection);
static AcqueductClientConnection* findConnection(VoidList* list, int socketDescriptor);
static int setNonblockingSocket(int descriptor);
static int addEpollDescriptor(int pollDescriptor, int descriptor);
static void printVoidList(VoidList* list);

int listenAcqueduct(AcqueductSocket* acqueductSocket)
{
	epoll_event* events;
	VoidList* connectionList;
	AcqueductClientConnection* clientConnection;
	CompressionBufferPair* buffers;
	epoll_event event;
	sockaddr_in clientAddress;
	socklen_t clientAddressLength;
	int clientDescriptor, pollDescriptor;
	int readyEvents;
	int status, i;
	char peek;

	connectionList = createVoidList(1024);
	buffers = createBufferPair(RECEIVE_BUFFER_SIZE, RECEIVE_UNCOMPRESSED_SIZE);
	events = (epoll_event*)calloc(MAX_POLLED_DESCRIPTORS, sizeof(epoll_event));

	clientConnection = (AcqueductClientConnection*)malloc(sizeof(AcqueductClientConnection));
	clientConnection->socketDescriptor = acqueductSocket->socketDescriptor;
	addVoidList(connectionList, clientConnection);

	pollDescriptor = epoll_create1(0);
	if(pollDescriptor == -1)
	{
		displayError("Unable to create epoll descriptor");
		return 14;
	}

	addEpollDescriptor(pollDescriptor, acqueductSocket->socketDescriptor);

	for(;;)
	{
		readyEvents = epoll_wait(pollDescriptor, events, MAX_POLLED_DESCRIPTORS, -1);

		for(i = 0; i < readyEvents; i++)
		{
			event = events[i];
			clientConnection = findConnection(connectionList, event.data.fd);
			clientDescriptor = clientConnection->socketDescriptor;

			// error listening?
			if ((event.events & EPOLLERR) ||
					(event.events & EPOLLHUP) ||
					(!(event.events & EPOLLIN)))
			{
					closeAcqueductSocket(connectionList, clientConnection);
					continue;
			}

			// new connection?
			if(event.data.fd == acqueductSocket->socketDescriptor)
			{
				clientAddressLength = 0;

				clientDescriptor = accept(acqueductSocket->socketDescriptor, (sockaddr*)&clientAddress, &clientAddressLength);
				if(clientDescriptor == -1)
				{
					displayError("Unable to accept incoming client");
					continue;
				}

				clientConnection = (AcqueductClientConnection*)malloc(sizeof(AcqueductClientConnection));
				clientConnection->socketDescriptor = clientDescriptor;
				addVoidList(connectionList, clientConnection);

				setNonblockingSocket(clientDescriptor);
				addEpollDescriptor(pollDescriptor, clientDescriptor);
				continue;
			}

			// a client-connected socket received.
			status = recv(clientConnection->socketDescriptor, &peek, 1, MSG_PEEK);
			if(status == 0)
			{
				closeAcqueductSocket(connectionList, clientConnection);
				continue;
			}

			receiveAcqueductData(clientConnection, buffers);
			continue;
		}
	}

	return 0;
}

int bindAcqueduct(char* port, AcqueductSocket* acqueductSocket)
{
	addrinfo* localAddress;
	int socketDescriptor;
	int status;

	localAddress = resolveHostname("0.0.0.0", ACQUEDUCT_DEFAULT_PORT);

	socketDescriptor = socket(localAddress->ai_family, localAddress->ai_socktype, 0);
	if(socketDescriptor < 0)
	{
		displayError("Unable to open local socket");
		return 11;
	}

	#ifdef SO_REUSEADDR
		setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &status, sizeof(status));
	#endif
	#ifdef SO_REUSEPORT
		setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEPORT, &status, sizeof(status));
	#endif

	status = bind(socketDescriptor, localAddress->ai_addr, localAddress->ai_addrlen);
	if(status != 0)
	{
		displayError("Unable to bind local socket");
		return 12;
	}

	status = setNonblockingSocket(socketDescriptor);
	if(status != 0)
		return status;

	status = listen(socketDescriptor, 1024);
	if(status != 0)
	{
		displayError("Unable to listen on local socket");
		return 13;
	}

	acqueductSocket->hostname = "localhost";
	acqueductSocket->port = port;
	acqueductSocket->socketDescriptor = socketDescriptor;

	return 0;
}

void closeAcqueduct(AcqueductSocket* acqueductSocket)
{
	close(acqueductSocket->socketDescriptor);
}

/*
	Called when data is available on the given connection.
*/
static void receiveAcqueductData(AcqueductClientConnection* connection, CompressionBufferPair* buffers)
{
	int status;

	buffers->actualLength = recv(connection->socketDescriptor, buffers->compressedBuffer, buffers->maxCompressedLength, 0);
	status = uncompress(buffers->uncompressedBuffer, &buffers->maxUncompressedLength, buffers->compressedBuffer, buffers->actualLength);

	printf("Received from client %d: \n%s\n--\n", connection->socketDescriptor, buffers->uncompressedBuffer);
}

static void closeAcqueductSocket(VoidList* clientList, AcqueductClientConnection* clientConnection)
{
	close(clientConnection->socketDescriptor);
	removeVoidList(clientList, clientConnection);
}

static int setNonblockingSocket(int descriptor)
{
	int flags;
	int status;

	flags = fcntl (descriptor, F_GETFL, 0);
	if (flags == -1)
	{
		perror ("Unable to get flags from client socket");
		return 31;
	}

	flags |= O_NONBLOCK;
	status = fcntl (descriptor, F_SETFL, flags);

	if (status == -1)
	{
		perror ("Unable to set nonblocking flag on client socket");
		return 32;
	}

	return 0;
}

static int addEpollDescriptor(int pollDescriptor, int descriptor)
{
	epoll_event event;

	event.data.fd = descriptor;
	event.events = EPOLLIN | EPOLLET;
	return epoll_ctl(pollDescriptor, EPOLL_CTL_ADD, descriptor, &event);
}

/*
	Populates the given fd_set with the contents of the given connectionList descriptors.
*/
static void populateDescriptors(epoll_event* descriptors, VoidList* connectionList)
{
	AcqueductClientConnection* connection;
	epoll_event event;

	for(int i = 0; i < connectionList->length; i++)
	{
		connection = (AcqueductClientConnection*)connectionList->entries[i];
		event.data.fd = connection->socketDescriptor;
		descriptors[i] = event;
	}
}

/*
	Finds (and returns) the client connection by socket descriptor.
	Returns NULL if the given descriptor does not exist in the list.
*/
static AcqueductClientConnection* findConnection(VoidList* list, int socketDescriptor)
{
	AcqueductClientConnection* connection;

	for(int i = 0; i < list->length; i++)
	{
		connection = (AcqueductClientConnection*)list->entries[i];

		if(connection->socketDescriptor == socketDescriptor)
			return connection;
	}

	return NULL;
}

static void printVoidList(VoidList* list)
{
	AcqueductClientConnection* connection;

	printf("[");

	for(int i = 0; i < list->length-1; i++)
	{
		connection = (AcqueductClientConnection*)list->entries[i];
		printf("%d, ", connection->socketDescriptor);
	}

	connection = (AcqueductClientConnection*)list->entries[list->length-1];
	printf("%d]\n", connection->socketDescriptor);
}
