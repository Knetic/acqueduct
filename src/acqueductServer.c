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

int listenAcqueduct(AcqueductSocket* acqueductSocket)
{
  fd_set readDescriptors;
  sockaddr_in clientAddress;
  VoidList* connectionList;
  AcqueductClientConnection* clientConnection;
  socklen_t clientAddressLength;
  int clientDescriptor;
  int status, i;

  FD_ZERO(&readDescriptors);
  FD_SET(acqueductSocket->socketDescriptor, &readDescriptors);

  for(;;)
  {
    status = select(connectionList->length+1, &readDescriptors, NULL, NULL, NULL);

    if(status == -1)
    {
      fprintf(stderr, "Unable to wait for input");
      return 30;
    }

    for(i = 0; i < FD_SETSIZE; i++)
      if(FD_ISSET(i, &readDescriptors))
      {
        // server received new connection?
        if(acqueductSocket->socketDescriptor == i)
        {
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
