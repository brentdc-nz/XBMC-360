// Bundler360 - LZO-compressed XPR bundle writer for Xbox 360
// Writes big-endian headers so the 360 CPU reads them natively

#include "Bundler360.h"
#include "lzo_compress.h"

// Link against the prebuilt LZO library
#pragma comment(lib, "lzo.lib")

// alignment of file blocks - must be multiple of disk sector size and power of 2
// HDD sector = 512 bytes on Xbox 360
#undef ALIGN
#define ALIGN (512)

bool CBundler360::StartBundle()
{
  DataSize = 0;
  FileHeaders.clear();

  lzo_init();

  return true;
}

int CBundler360::WriteBundle(const char* Filename)
{
  // calc data offset
  DWORD headerSize = sizeof(XPR_FILE_HEADER) + (DWORD)FileHeaders.size() * sizeof(FileHeader_t);

  // setup header - version 5 to distinguish from OG Xbox (version 2)
  XPRHeader.dwMagic = XPR_MAGIC_HEADER_VALUE | (('5' - '0') << 24);
  XPRHeader.dwHeaderSize = headerSize;

  headerSize = (headerSize + (ALIGN - 1)) & ~(ALIGN - 1);
  XPRHeader.dwTotalSize = headerSize + DataSize;

  // create our header in memory
  BYTE* headerBuf = (BYTE*)malloc(headerSize);
  if (!headerBuf) return -1;

  BYTE* buf = headerBuf;

  // Write header - byte-swap for big-endian Xbox 360
  XPR_FILE_HEADER beHeader;
  beHeader.dwMagic = ByteSwap32(XPRHeader.dwMagic);
  beHeader.dwTotalSize = ByteSwap32(XPRHeader.dwTotalSize);
  beHeader.dwHeaderSize = ByteSwap32(XPRHeader.dwHeaderSize);
  memcpy(buf, &beHeader, sizeof(XPR_FILE_HEADER));
  buf += sizeof(XPR_FILE_HEADER);

  for (std::list<FileHeader_t>::iterator i = FileHeaders.begin(); i != FileHeaders.end(); ++i)
  {
    i->Offset += headerSize;

    // Write file header with byte-swapped DWORD fields
    FileHeader_t beFile;
    memcpy(beFile.Name, i->Name, sizeof(beFile.Name)); // Names stay as ASCII
    beFile.Offset = ByteSwap32(i->Offset);
    beFile.UnpackedSize = ByteSwap32(i->UnpackedSize);
    beFile.PackedSize = ByteSwap32(i->PackedSize);
    memcpy(buf, &beFile, sizeof(FileHeader_t));
    buf += sizeof(FileHeader_t);
  }
  memset(buf, 0, headerBuf + headerSize - buf);

  // write file
  FILE* file = fopen(Filename, "wb");
  if (!file)
  {
    free(Data);
    free(headerBuf);
    return -1;
  }

  size_t n = fwrite(headerBuf, 1, headerSize, file);
  if (n != headerSize)
  {
    free(Data);
    free(headerBuf);
    fclose(file);
    return -1;
  }

  n = fwrite(Data, 1, DataSize, file);
  if (n != DataSize)
  {
    free(Data);
    free(headerBuf);
    fclose(file);
    return -1;
  }

  free(Data);
  free(headerBuf);
  fclose(file);

  return DataSize + headerSize;
}

bool CBundler360::AddFile(const char* Filename, int nBuffers, const void** Buffers, DWORD* Sizes)
{
  FileHeader_t Header;

  memset(Header.Name, 0, sizeof(Header.Name));
  for (int i = 0; i < (int)sizeof(Header.Name) && Filename[i]; ++i)
    Header.Name[i] = tolower(Filename[i]);
  Header.Name[sizeof(Header.Name) - 1] = 0;

  Header.Offset = DataSize;
  Header.PackedSize = 0;
  Header.UnpackedSize = 0;
  for (int i = 0; i < nBuffers; ++i)
    Header.UnpackedSize += Sizes[i];

  // allocate enough memory for the total unpacked size
  BYTE* buf = (BYTE*)malloc(Header.UnpackedSize);
  if (!buf) return false;

  BYTE* p = buf;
  for (int i = 0; i < nBuffers; ++i)
  {
    memcpy(p, Buffers[i], Sizes[i]);
    p += Sizes[i];
  }

  // grab a temporary buffer for compressing into
  BYTE* compressedBuf = (BYTE*)malloc(Header.UnpackedSize + Header.UnpackedSize / 16 + 64 + 3);
  if (!compressedBuf)
  {
    free(buf);
    return false;
  }

  // working buffer for lzo
  lzo_voidp workingBuf = malloc(LZO1X_999_MEM_COMPRESS);
  if (!workingBuf)
  {
    free(buf);
    free(compressedBuf);
    return false;
  }

  if (lzo1x_999_compress(buf, Header.UnpackedSize, compressedBuf, (lzo_uint*)&Header.PackedSize, workingBuf) != LZO_E_OK)
  {
    printf("Compression failure\n");
    free(buf);
    free(compressedBuf);
    free(workingBuf);
    return false;
  }
  free(workingBuf);

  lzo_uint s = Header.UnpackedSize;
  lzo1x_optimize(compressedBuf, Header.PackedSize, buf, &s, NULL);
  free(buf);

  // align to sector boundary
  DWORD ExtraNeeded = (Header.PackedSize + (ALIGN - 1)) & ~(ALIGN - 1);

  Data = (BYTE*)realloc(Data, DataSize + ExtraNeeded);

  memcpy(Data + DataSize, compressedBuf, Header.PackedSize);
  free(compressedBuf);

  memset(Data + DataSize + Header.PackedSize, 0, ExtraNeeded - Header.PackedSize);
  DataSize += ExtraNeeded;
  FileHeaders.push_back(Header);

  return true;
}
