#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "acqueduct.h"

typedef struct RuntimeFlags
{
  char* hostname;
  char* port;
  char* fifoPath;
  bool isClient;
} RuntimeFlags;

static int performAcqueductClient(char* hostname, char* port, char* fifoPath);
static int performAcqueductServer();
static int makeFifo(char* path);
static void parseFlags(char** args, int argc, RuntimeFlags*);

int main(int argc, char** args)
{
  RuntimeFlags flags;

  parseFlags(args, argc, &flags);

  if(flags.isClient)
    return performAcqueductClient(flags.hostname, flags.port, flags.fifoPath);
  return performAcqueductServer();
}

static int performAcqueductClient(char* hostname, char* port, char* fifoPath)
{
  AcqueductSocket localSocket;
  int fifoDescriptor;
  int status;

  status = connectAcqueduct(hostname, port, &localSocket);
  if(status != 0)
    return status;

  fifoDescriptor = makeFifo(fifoPath);
  if(fifoDescriptor == -1)
    return 40;

  status = forwardAcqueductInput(fifoDescriptor, localSocket);
  if(status == -1)
    return 41;

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

static int makeFifo(char* path)
{
  int status;

  status = mkfifo(path, 0666);
  if(status == -1)
    perror("Unable to make fifo pipe");

  status = open(path, O_RDONLY);
  if(status < 0)
  {
    perror("Unable to open fifo pipe");
    return status;
  }

  return status;
}

static void parseFlags(char** args, int argc, RuntimeFlags* out)
{
  int opt;

  out->isClient = true;
  out->hostname = "localhost";
  out->port = ACQUEDUCT_DEFAULT_PORT;
  out->fifoPath = "./stdin";

  while ((opt = getopt(argc, args, "sh:p:i:")) != -1)
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
      case 'i':
        out->fifoPath = optarg;
        break;
    }
  }
}
