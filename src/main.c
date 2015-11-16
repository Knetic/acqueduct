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

inline int performAcqueductClient(const char* hostname, const char* port);
inline int performAcqueductServer();
inline void parseFlags(const char** args, const int argc, RuntimeFlags*);

int main(const int argc, const char** args)
{
  RuntimeFlags flags;

  parseFlags(args, argc, &flags);

  if(flags.isClient)
    return performAcqueductClient(flags.hostname, flags.port);
  return performAcqueductServer();
}

inline int performAcqueductClient(const char* hostname, const char* port)
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

inline void parseFlags(const char** args, const int argc, RuntimeFlags* out)
{
  RuntimeFlags flags = *out;
  char opt;

  flags.isClient = true;
  flags.hostname = "localhost";
  flags.port = "4004";

  while ((opt = getopt(argc, args, "shp")) != -1)
  {
    switch (opt)
    {
      case 's':
        flags.isClient = false;
        break;
      case 'h':
        flags.hostname = optarg;
        break;
      case 'w':
        flags.port = optarg;
        break;
    }
  }
}
