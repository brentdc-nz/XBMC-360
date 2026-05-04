// Xbox 360 texture format definitions for the XBMC360Tex build tool
// This file defines the structures written into the XPR bundle that
// the Xbox 360 runtime will read.

#pragma once

typedef unsigned long DWORD;
typedef unsigned int UINT;
typedef int BOOL;
typedef const void* LPCVOID;
typedef void* LPVOID;
typedef unsigned char BYTE;
typedef unsigned short WORD;

// XPR bundle header - same structure as original, new version number
struct XPR_FILE_HEADER
{
  DWORD dwMagic;
  DWORD dwTotalSize;
  DWORD dwHeaderSize;
};

// Magic value: "XPR0" base, version stored in high byte
#define XPR_MAGIC_HEADER_VALUE (('0' << 24) | ('R' << 16) | ('P' << 8) | 'X')

// Xbox 360 texture metadata - replaces the OG D3DTexture struct
// This is what gets stored per-texture inside the XPR data section
#pragma pack(push, 1)
struct XPR360_TextureInfo
{
  DWORD Width;        // Real image width in pixels
  DWORD Height;       // Real image height in pixels
  DWORD Format;       // D3DFMT value (D3DFMT_LIN_A8R8G8B8, etc.)
  DWORD Pitch;        // Row pitch in bytes (for linear textures)
  DWORD DataOffset;   // Offset from start of unpacked data to pixel data
  DWORD DataSize;     // Size of pixel data in bytes
  DWORD Flags;        // XPR360_FLAG_* values
  DWORD Delay;        // Frame delay for animations (ms), 0 for stills
};
#pragma pack(pop)

// Flags for XPR360_TextureInfo
#define XPR360_FLAG_ANIM       0x00000001  // This texture is part of an animation
#define XPR360_FLAG_HASALPHA   0x00000002  // Texture has meaningful alpha channel

// Xbox 360 XPR per-file header: stored after decompression
// Layout: XPR360_FileInfo, then N x XPR360_TextureInfo, then pixel data (128-byte aligned)
#pragma pack(push, 1)
struct XPR360_FileInfo
{
  DWORD nImages;      // Number of texture frames (1 for stills)
  DWORD nLoops;       // Animation loop count (0 = infinite)
  DWORD TotalSize;    // Total size of all pixel data
};
#pragma pack(pop)

// Xbox 360 uses D3DFMT_LIN_A8R8G8B8 = 0x46 for linear ARGB textures
// We define our own constant here since we're building on PC
#define X360_D3DFMT_LIN_A8R8G8B8  0x46

// Byte-swap utilities for big-endian output
inline DWORD ByteSwap32(DWORD v)
{
  return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) | ((v & 0xFF000000) >> 24);
}

inline WORD ByteSwap16(WORD v)
{
  return (v >> 8) | (v << 8);
}

// Byte-swap a DWORD array in-place
inline void ByteSwapBuffer32(DWORD* buf, DWORD count)
{
  for (DWORD i = 0; i < count; i++)
    buf[i] = ByteSwap32(buf[i]);
}

// Byte-swap pixel data: swap each 4-byte pixel for A8R8G8B8
inline void ByteSwapPixels32(void* data, DWORD sizeInBytes)
{
  DWORD* p = (DWORD*)data;
  DWORD count = sizeInBytes / 4;
  for (DWORD i = 0; i < count; i++)
    p[i] = ByteSwap32(p[i]);
}

DWORD inline PadPow2(DWORD x)
{
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return ++x;
}
