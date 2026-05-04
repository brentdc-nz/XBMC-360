// Bundler360 - LZO-compressed XPR bundle writer for Xbox 360
// Adapted from xbmc4xbox's Bundler with big-endian header support

#pragma once

#include <cstdlib>
#include <ctype.h>
#include <cstdio>
#include <list>
#include "xbox360.h"

class CBundler360
{
  XPR_FILE_HEADER XPRHeader;
  struct FileHeader_t
  {
    // 128 bytes total
    char Name[116];
    DWORD Offset;
    DWORD UnpackedSize;
    DWORD PackedSize;
  };
  std::list<FileHeader_t> FileHeaders;
  BYTE* Data;
  DWORD DataSize;

public:
  CBundler360() { Data = NULL; DataSize = 0; }
  ~CBundler360() {}

  bool StartBundle();
  int WriteBundle(const char* Filename);

  bool AddFile(const char* Filename, int nBuffers, const void** Buffers, DWORD* Sizes);
};
