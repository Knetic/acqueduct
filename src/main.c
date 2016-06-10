#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

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

static void handleSignals();
static void sigpipeHandler();

int main(int argc, char** args)
{
  RuntimeFlags flags;

  parseFlags(args, argc, &flags);

  if(flags.isClient)
  {
    handleSignals();
    return performAcqueductClient(flags.hostname, flags.port, flags.fifoPath);
  }
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
  closeAcqueduct(&localSocket);

  if(status == -1)
    return 41;

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

  status = open(path, O_RDONLY);
  if(status < 0)
  {
    mkfifo(path, 0666);
    status = open(path, O_RDONLY);

    if(status < 0)
        perror("Unable to create or open fifo pipe");
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

/*
  Sets up signal handlers for any cases where we need to do something.
  Currently only handles SIGPIPE, so that we get broken pipe errors when sending.
*/
static void handleSignals()
{
  struct sigaction* signalAction;

  signalAction = malloc(sizeof(struct sigaction));

  signalAction->sa_handler = sigpipeHandler;
  sigemptyset(&signalAction->sa_mask);
  signalAction->sa_flags = 0;
  sigaction(SIGPIPE, signalAction, (struct sigaction*)NULL);
}

static void sigpipeHandler()
{
  // no-op, only here to make sure the SIGPIPE is handled.
  // Clients will get a "broken pipe" errno when they try to send.
}
