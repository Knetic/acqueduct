#include "_acqueduct.h"
#include "voidlist.h"

/*
  Represents a single connection to an Acqueduct client.
*/
typedef struct AcqueductClientConnection
{
  int socketDescriptor;
} AcqueductClientConnection;

static void receiveAcqueductData(AcqueductClientConnection* connection);
static AcqueductClientConnection* findConnection(VoidList* list, int socketDescriptor);
static void populateDescriptors(fd_set* readDescriptors, VoidList* connectionList);
static void printVoidList(VoidList* list);

int listenAcqueduct(AcqueductSocket* acqueductSocket)
{
  fd_set readDescriptors;
  sockaddr_in clientAddress;
  VoidList* connectionList;
  AcqueductClientConnection* clientConnection;
  socklen_t clientAddressLength;
  int clientDescriptor;
  int status, i;

  connectionList = createVoidList(1024);

  clientConnection = (AcqueductClientConnection*)malloc(sizeof(AcqueductClientConnection));
  clientConnection->socketDescriptor = acqueductSocket->socketDescriptor;
  addVoidList(connectionList, clientConnection);

  for(;;)
  {
    populateDescriptors(&readDescriptors, connectionList);
    status = select(connectionList->length, &readDescriptors, NULL, NULL, NULL);

    printf("Selected.\n");

    if(status == -1)
    {
      fprintf(stderr, "Unable to wait for input");
      return 30;
    }

    for(i = 0; i < FD_SETSIZE; i++)
    {
      clientConnection = ((AcqueductClientConnection*)connectionList->entries[i]);
      clientDescriptor = clientConnection->socketDescriptor;

      if(FD_ISSET(clientDescriptor, &readDescriptors))
      {
        // server received new connection?
        if(acqueductSocket->socketDescriptor == i)
        {
          printf("Accepting client.\n");

          clientDescriptor = accept(acqueductSocket->socketDescriptor, (sockaddr*)&clientAddress, &clientAddressLength);

          clientConnection = (AcqueductClientConnection*)malloc(sizeof(AcqueductClientConnection));
          clientConnection->socketDescriptor = clientDescriptor;

          addVoidList(connectionList, clientConnection);
          continue;
        }

        // a client-connected socket received.
        clientConnection = findConnection(connectionList, ((AcqueductClientConnection*)(connectionList->entries[i]))->socketDescriptor);
        receiveAcqueductData(clientConnection);
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

  socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
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

/*
  Called when data is available on the given connection.
*/
static void receiveAcqueductData(AcqueductClientConnection* connection)
{
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
