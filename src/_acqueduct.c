#include "_acqueduct.h"

/*
	Displays the current errno on stderr.
*/
void displayError(const char* prefix)
{
	displayErrorCode(prefix, errno);
}

void displayErrorCode(const char* prefix, int code)
{
	char* errorString;

	errorString = strerror(code);
	fprintf(stderr, "%s: %s\n", prefix, errorString);
}

addrinfo* resolveHostname(const char* hostname, const char* port)
{
	addrinfo hints;
	addrinfo* result;
	addrinfo* currentResult;
	addrinfo* bestResult;
	int resultCode;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	resultCode = getaddrinfo(hostname, port, &hints, &result);
	if(resultCode != 0)
	{
		displayErrorCode("Unable to resolve remote host", resultCode);
		return NULL;
	}

	for(currentResult = result; currentResult != NULL; currentResult = currentResult->ai_next)
	{
		if(currentResult->ai_socktype == SOCK_STREAM &&
			(currentResult->ai_family == AF_INET ||
			currentResult->ai_family == AF_INET6))
		{
			bestResult = currentResult;
		}
	}

	if(bestResult == NULL)
	{
		fprintf(stderr, "No suitable remote connection method found\n");
		return NULL;
	}

	return bestResult;
}
