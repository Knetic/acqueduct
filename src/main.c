#include "acqueduct.h"

int main(int argc, char** arg)
{
  AcqueductSocket localSocket;
  int status;

  status = connectAcqueduct("localhost", 4004, &localSocket);
  if(status != 0)
    return status;

  printf("Connection established, waiting for input.\n");
  forwardAcqueductInput(STDIN_FILENO, localSocket);
  return 0;
}
