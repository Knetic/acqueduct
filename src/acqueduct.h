typedef struct AcqueductSocket
{
  char* hostname;
  char* port;
  int socketDescriptor;
} AcqueductSocket;

/*
  Connects to an acqueduct server at the given [hostname] and [port],
  returning data through [out].
  If the return code is zero, connection was successful.
  Otherwise, an error is written to stderr, and the return code is nonzero.
*/
int connectAcqueduct(char* hostname, char* port, AcqueductSocket* out);

/*
  Forwards all input from the given file descriptor through the given
  Acqueduct socket.
*/
int forwardAcqueductInput(const int fd, const AcqueductSocket localSocket);
