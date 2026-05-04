// Surface.h - Image loading surface for XBMC360Tex
// Uses GDI+ for image decoding (no DirectX SDK dependency)

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "xbox360.h"

class CSurfaceRect
{
public:
  CSurfaceRect() { pBits = NULL; Pitch = 0; }
  BYTE* pBits;
  DWORD Pitch;
};

class CSurface
{
public:
  CSurface();
  ~CSurface();

  enum FORMAT { FMT_ARGB, FMT_LIN_ARGB, FMT_PALETTED };
  struct ImageInfo
  {
    unsigned int width;
    unsigned int height;
    FORMAT format;
  };

  bool CreateFromFile(const char* Filename, FORMAT format);
  bool Create(unsigned int width, unsigned int height, FORMAT format);
  bool Lock(CSurfaceRect* rect);
  bool Unlock();

  unsigned int Width() const { return m_width; }
  unsigned int Height() const { return m_height; }
  unsigned int BPP() const { return m_bpp; }
  unsigned int Pitch() const { return m_width * m_bpp; }
  const ImageInfo& Info() const { return m_info; }

private:
  void Clear();
  BYTE* m_pixels;
  unsigned int m_width;
  unsigned int m_height;
  unsigned int m_bpp;
  ImageInfo m_info;
  bool m_locked;
};

// No graphics device needed - we use GDI+ directly
class CGraphicsDevice
{
public:
  CGraphicsDevice() {}
  ~CGraphicsDevice() {}
  bool Create();
};

extern CGraphicsDevice g_device;
