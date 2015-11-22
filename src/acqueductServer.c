#include "_acqueduct.h"
#include "voidlist.h"
#include "CompressionBufferPair.h"

#define RECEIVE_BUFFER_SIZE 65536
#define RECEIVE_UNCOMPRESSED_SIZE RECEIVE_BUFFER_SIZE * 4

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
static void populateDescriptors(fd_set* readDescriptors, VoidList* connectionList);
static void printVoidList(VoidList* list);

int listenAcqueduct(AcqueductSocket* acqueductSocket)
{
  fd_set readDescriptors;
  sockaddr_in clientAddress;
  VoidList* connectionList;
  AcqueductClientConnection* clientConnection;
  CompressionBufferPair* buffers;
  socklen_t clientAddressLength;
  int clientDescriptor;
  int status, i;
  char peek;

  connectionList = createVoidList(1024);
  buffers = createBufferPair(RECEIVE_BUFFER_SIZE, RECEIVE_UNCOMPRESSED_SIZE);

  clientConnection = (AcqueductClientConnection*)malloc(sizeof(AcqueductClientConnection));
  clientConnection->socketDescriptor = acqueductSocket->socketDescriptor;
  addVoidList(connectionList, clientConnection);

  for(;;)
  {
    populateDescriptors(&readDescriptors, connectionList);
    status = select(FD_SETSIZE, &readDescriptors, NULL, NULL, NULL);

    if(status == -1)
    {
      displayError("Unable to wait for input");
      return 30;
    }

    for(i = 0; i < connectionList->length; i++)
    {
      clientConnection = ((AcqueductClientConnection*)connectionList->entries[i]);
      clientDescriptor = clientConnection->socketDescriptor;

      if(FD_ISSET(clientDescriptor, &readDescriptors))
      {
        // server received new connection?
        if(acqueductSocket->socketDescriptor == clientDescriptor)
        {
          clientDescriptor = accept(acqueductSocket->socketDescriptor, (sockaddr*)&clientAddress, &clientAddressLength);

          clientConnection = (AcqueductClientConnection*)malloc(sizeof(AcqueductClientConnection));
          clientConnection->socketDescriptor = clientDescriptor;

          addVoidList(connectionList, clientConnection);
          continue;
        }

        // a client-connected socket received.
        clientConnection = findConnection(connectionList, ((AcqueductClientConnection*)(connectionList->entries[i]))->socketDescriptor);
        status = recv(clientConnection->socketDescriptor, &peek, 1, MSG_PEEK);

        if(status > 0)
        {
          receiveAcqueductData(clientConnection, buffers);
          continue;
        }
        if(status == 0)
        {
          closeAcqueductSocket(connectionList, clientConnection);
          continue;
        }
      }
    }
  }

  return 0;
}

int bindAcqueduct(char* port, AcqueductSocket* acqueductSocket)
{
  addrinfo* localAddress;
  int socketDescriptor;
  int status;

  localAddress = resolveHostname("localhost", ACQUEDUCT_DEFAULT_PORT);

  socketDescriptor = socket(localAddress->ai_family, localAddress->ai_socktype, 0);
  if(socketDescriptor < 0)
  {
    displayError("Unable to open local socket");
    return 11;
  }

  status = bind(socketDescriptor, localAddress->ai_addr, localAddress->ai_addrlen);
  if(status != 0)
  {
    displayError("Unable to bind local socket");
    return 12;
  }

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
  status = uncompress(buffers->uncompressedBuffer, &buffers->actualLength, buffers->compressedBuffer, buffers->actualLength);

  printf("Received from client %d: \n%s\n--\n", connection->socketDescriptor, buffers->uncompressedBuffer);
}

static void closeAcqueductSocket(VoidList* clientList, AcqueductClientConnection* clientConnection)
{
  close(clientConnection->socketDescriptor);
  removeVoidList(clientList, clientConnection);
}

/*
  Populates the given fd_set with the contents of the given connectionList descriptors.
*/
static void populateDescriptors(fd_set* readDescriptors, VoidList* connectionList)
{
  AcqueductClientConnection* connection;

  FD_ZERO(readDescriptors);

  for(int i = 0; i < connectionList->length; i++)
  {
    connection = (AcqueductClientConnection*)connectionList->entries[i];
    FD_SET(connection->socketDescriptor, readDescriptors);
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
