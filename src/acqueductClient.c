#include "_acqueduct.h"

// 64k default buffer size.
#define DEFAULT_STDIN_BUFFER_SIZE 1024*64

/*
	Reads input out of stdin, zips it with gzip, and sends it across the given
	AcqueductSocket.
*/
int forwardAcqueductInput(const int fd, const AcqueductSocket localSocket)
{
	char* input;
	char* compressedInput;
	size_t inputLength, compressedLength;
	int status;

	input = (char*)malloc(DEFAULT_STDIN_BUFFER_SIZE);
	compressedInput = (char*)malloc(DEFAULT_STDIN_BUFFER_SIZE);

	for(;;)
	{
		// read.
		inputLength = read(fd, input, DEFAULT_STDIN_BUFFER_SIZE);
		if(inputLength <= 0)
			continue;

		compressedLength = DEFAULT_STDIN_BUFFER_SIZE;

		// compress.
		status = compress(compressedInput, &compressedLength, input, inputLength);
		if(status != Z_OK)
		{
			if(status == Z_MEM_ERROR)
			{
				fprintf(stderr, "Unable to compress message");
				return 22;
			}

			if(status == Z_BUF_ERROR)
			{
				fprintf(stderr, "Buffer error when compressing message");
				return 23;
			}

			fprintf(stderr, "Unable to compress message, unknown error %d", status);
			return 29;
		}

		// write.
		status = write(localSocket.socketDescriptor, compressedInput, compressedLength);
		if(status < 0)
		{
			displayError("Unable to write to remote host");
			return 20;
		}
	}
}

int connectAcqueduct(char* hostname, char* port, AcqueductSocket* out)
{
	addrinfo* remoteAddress;
	int socketDescriptor;

	remoteAddress = resolveHostname(hostname, port);
	if(remoteAddress == NULL)
		return 10;

	socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
	if(socketDescriptor < 0)
	{
		displayError("Unable to open local socket");
		return 11;
	}

	if(connect(socketDescriptor, remoteAddress->ai_addr, remoteAddress->ai_addrlen) < 0)
	{
		displayError("Unable to connect to remote");
		return 12;
	}

	out->socketDescriptor = socketDescriptor;
	out->hostname = hostname;
	out->port = port;
	return 0;
}
