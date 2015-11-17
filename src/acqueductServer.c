#include "_acqueduct.h"

int listenAcqueduct(AcqueductSocket* acqueductSocket)
{
  return 0;
}

int bindAcqueduct(char* port, AcqueductSocket* acqueductSocket)
{
  addrinfo* localAddress;
  int socketDescriptor;
  int status;

  localAddress = resolveHostname("localhost", "4004");

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

  acqueductSocket->hostname = "localhost";
  acqueductSocket->port = port;
  acqueductSocket->socketDescriptor = socketDescriptor;
  return 0;
}
