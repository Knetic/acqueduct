#include "acqueduct.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct RuntimeFlags
{
  char* hostname;
  char* port;
  bool isClient;
} RuntimeFlags;

inline int performAcqueductClient(char* hostname, char* port);
inline int performAcqueductServer();
inline void parseFlags(char** args, int argc, RuntimeFlags*);

int main(int argc, char** args)
{
  RuntimeFlags flags;

  parseFlags(args, argc, &flags);
  printf("Hostname: %s\nPort: %s\n", flags.hostname, flags.port);

  if(flags.isClient)
    return performAcqueductClient(flags.hostname, flags.port);
  return performAcqueductServer();
}

inline int performAcqueductClient(char* hostname, char* port)
{
  AcqueductSocket localSocket;
  int status;

  status = connectAcqueduct(hostname, port, &localSocket);
  if(status != 0)
    return status;

  printf("Connection established, waiting for input.\n");
  forwardAcqueductInput(STDIN_FILENO, localSocket);
  return 0;
}

inline int performAcqueductServer()
{
  return 0;
}

inline void parseFlags(char** args, int argc, RuntimeFlags* out)
{
  int opt;

  out->isClient = true;
  out->hostname = "localhost";
  out->port = "4004";

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
