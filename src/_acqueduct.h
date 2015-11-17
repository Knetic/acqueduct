/*
  Convenience header for internal acqueduct files.
*/

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
#include "acqueduct.h"

// convenience, so that we don't need to litter code with the "struct" keyword
// before variable declarations.
typedef struct sockaddr sockaddr;
typedef struct addrinfo addrinfo;

addrinfo* resolveHostname(const char*, const char* port);
