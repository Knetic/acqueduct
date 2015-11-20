#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "acqueduct.h"

typedef struct RuntimeFlags
{
  char* hostname;
  char* port;
  bool isClient;
} RuntimeFlags;

static int performAcqueductClient(char* hostname, char* port);
static int performAcqueductServer();
static void parseFlags(char** args, int argc, RuntimeFlags*);

int main(int argc, char** args)
{
  RuntimeFlags flags;

  parseFlags(args, argc, &flags);
  printf("Hostname: %s\nPort: %s\n", flags.hostname, flags.port);

  if(flags.isClient)
    return performAcqueductClient(flags.hostname, flags.port);
  return performAcqueductServer();
}

static int performAcqueductClient(char* hostname, char* port)
{
  AcqueductSocket localSocket;
  int status;

  status = connectAcqueduct(hostname, port, &localSocket);
  if(status != 0)
    return status;

  printf("Connection established, waiting for input.\n");
  status = forwardAcqueductInput(STDIN_FILENO, localSocket);
  
  closeAcqueduct(&localSocket);
  return status;
}

static int performAcqueductServer(char* port)
{
  AcqueductSocket localSocket;
  int status;

  status = bindAcqueduct(port, &localSocket);
  if(status != 0)
    return status;

  printf("Socket bound, waiting for connections.\n");
  status = listenAcqueduct(&localSocket);

  closeAcqueduct(&localSocket);
  return status;
}

static void parseFlags(char** args, int argc, RuntimeFlags* out)
{
  int opt;

  out->isClient = true;
  out->hostname = "localhost";
  out->port = ACQUEDUCT_DEFAULT_PORT;

  while ((opt = getopt(argc, args, "sh:p:")) != -1)
  {
    switch (opt)
    {
      case 's':
        out->isClient = false;
        break;
      case 'h':
        out->hostname = optarg;
        break;
      case 'p':
        out->port = optarg;
        break;
    }
  }
}
