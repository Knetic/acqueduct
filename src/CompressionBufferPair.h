#include <stdlib.h>

typedef struct CompressionBufferPair
{
  char* compressedBuffer;
  char* uncompressedBuffer;
  size_t maxCompressedLength;
  size_t maxUncompressedLength;
  size_t actualLength;
} CompressionBufferPair;

CompressionBufferPair* createBufferPair(const int compressedLength, const int uncompressedLength);
