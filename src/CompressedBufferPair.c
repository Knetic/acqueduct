#include "CompressionBufferPair.h"

CompressionBufferPair* createBufferPair(const int compressedLength, const int uncompressedLength)
{
  CompressionBufferPair* ret;

  ret = (CompressionBufferPair*)malloc(sizeof(CompressionBufferPair));
  ret->compressedBuffer   = (char*)malloc(sizeof(char) * compressedLength);
  ret->uncompressedBuffer = (char*)malloc(sizeof(char) * uncompressedLength);
  ret->maxCompressedLength   = compressedLength;
  ret->maxUncompressedLength = uncompressedLength;

  return ret;
}
