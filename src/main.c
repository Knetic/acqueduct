#include "acqueduct.h"

inline void displayError(const char*);
inline addrinfo* resolveHostname(const char*, const char* port);
inline void printAddress(addrinfo* address);
inline void displayErrorCode(const char* prefix, int code);

int main(int argc, char** arg)
{
  AcqueductSocket localSocket;
  addrinfo* remoteAddress;
  int socketDescriptor;

  remoteAddress = resolveHostname("localhost", "4004");
  if(remoteAddress == NULL)
  {
    return 10;
  }

  socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
  if(socketDescriptor < 0)
  {
    displayError("Unable to open local socket");
    return 11;
  }

  localSocket.socketDescriptor = socketDescriptor;
  if(connect(socketDescriptor, remoteAddress->ai_addr, remoteAddress->ai_addrlen) < 0)
  {
    displayError("Unable to connect to remote");
    return 12;
  }

  return 0;
}

/*
  Displays the current errno on stderr.
*/
inline void displayError(const char* prefix)
{
  displayErrorCode(prefix, errno);
}

inline void displayErrorCode(const char* prefix, int code)
{
  char* errorString;

  errorString = strerror(code);
  fprintf(stderr, "%s: %s\n", prefix, errorString);
}

inline addrinfo* resolveHostname(const char* hostname, const char* port)
{
  addrinfo hints;
  addrinfo* result;
  addrinfo* currentResult;
  addrinfo* bestResult;
  int resultCode;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  resultCode = getaddrinfo(hostname, port, &hints, &result);
  if(resultCode != 0)
  {
    displayErrorCode("Unable to resolve remote host", resultCode);
    return NULL;
  }

  for(currentResult = result; currentResult != NULL; currentResult = currentResult->ai_next)
  {
    if(currentResult->ai_socktype == SOCK_STREAM &&
      (currentResult->ai_family == AF_INET ||
      currentResult->ai_family == AF_INET6))
    {
      bestResult = currentResult;
    }
  }

  if(bestResult == NULL)
  {
    fprintf(stderr, "No suitable remote connection method found\n");
    return NULL;
  }

  return bestResult;
}

inline void printAddress(addrinfo* address)
{
  char* remoteString;
  remoteString = malloc(sizeof(char) * 64);

  if(address->ai_family == AF_INET)
    inet_ntop(AF_INET, address->ai_addr, remoteString, 64);
  else
    inet_ntop(AF_INET6, address->ai_addr, remoteString, 64);

  printf("Selected remote host: %s\n", remoteString);
}
