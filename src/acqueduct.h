#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "zlib.h"

typedef struct AcqueductSocket
{
  char* hostname;
  int port;
  int socketDescriptor;
} AcqueductSocket;

/*
  Connects to an acqueduct server at the given [hostname] and [port],
  returning data through [out].
  If the return code is zero, connection was successful.
  Otherwise, an error is written to stderr, and the return code is nonzero.
*/
int connectAcqueduct(char* hostname, const int port, AcqueductSocket* out);

/*
  Forwards all input from the given file descriptor through the given
  Acqueduct socket.
*/
int forwardAcqueductInput(const int fd, const AcqueductSocket localSocket);

// convenience, so that we don't need to litter code with the "struct" keyword
// before variable declarations.
typedef struct sockaddr sockaddr;
typedef struct addrinfo addrinfo;
