#define ACQUEDUCT_DEFAULT_PORT "4004"

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
  Closes the given socket, be it client or server.
*/
void closeAcqueduct(AcqueductSocket* acqueductSocket);

/*
  Forwards all input from the given file descriptor through the given
  Acqueduct socket. This is a blocking function.
*/
int forwardAcqueductInput(const int fd, const AcqueductSocket localSocket);

/*
  Accepts connections on the given port, sending all unzipped content on stdout.
  This is a blocking function.
*/
int listenAcqueduct(AcqueductSocket* acqueductSocket);

/*
  Sets up a socket on the given port, ready to listen for acqueduct requests.
*/
int bindAcqueduct(char* port, AcqueductSocket* out);
