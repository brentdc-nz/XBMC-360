// Surface.cpp - Image loading using GDI+ (no DirectX SDK needed)

#include "Surface.h"
#include <objidl.h>
#include <gdiplus.h>
#include <stdio.h>

#pragma comment(lib, "gdiplus.lib")

CGraphicsDevice g_device;

static bool g_gdiplusInit = false;
static ULONG_PTR g_gdiplusToken = 0;

static void EnsureGdiPlus()
{
  if (!g_gdiplusInit) {
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartup(&g_gdiplusToken, &input, NULL);
    g_gdiplusInit = true;
  }
}

bool CGraphicsDevice::Create()
{
  EnsureGdiPlus();
  return true;
}

CSurface::CSurface()
{
  m_pixels = NULL;
  m_width = 0;
  m_height = 0;
  m_bpp = 0;
  m_locked = false;
  memset(&m_info, 0, sizeof(m_info));
}

CSurface::~CSurface()
{
  if (m_pixels)
    free(m_pixels);
}

bool CSurface::Create(unsigned int width, unsigned int height, FORMAT format)
{
  if (m_pixels) {
    free(m_pixels);
    m_pixels = NULL;
  }

  m_info.width = width;
  m_info.height = height;
  m_info.format = format;
  m_bpp = (format == FMT_PALETTED) ? 1 : 4;
  m_width = width;
  m_height = height;

  m_pixels = (BYTE*)calloc(m_width * m_height * m_bpp, 1);
  return (m_pixels != NULL);
}

bool CSurface::CreateFromFile(const char* Filename, FORMAT format)
{
  EnsureGdiPlus();

  // Convert filename to wide string
  int len = MultiByteToWideChar(CP_ACP, 0, Filename, -1, NULL, 0);
  wchar_t* wFilename = (wchar_t*)malloc(len * sizeof(wchar_t));
  MultiByteToWideChar(CP_ACP, 0, Filename, -1, wFilename, len);

  Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromFile(wFilename);
  free(wFilename);

  if (!bmp || bmp->GetLastStatus() != Gdiplus::Ok) {
    if (bmp) delete bmp;
    return false;
  }

  unsigned int imgWidth = bmp->GetWidth();
  unsigned int imgHeight = bmp->GetHeight();

  m_info.width = imgWidth;
  m_info.height = imgHeight;
  m_info.format = format;
  m_bpp = 4;
  m_width = imgWidth;
  m_height = imgHeight;

  m_pixels = (BYTE*)calloc(m_width * m_height * m_bpp, 1);
  if (!m_pixels) {
    delete bmp;
    return false;
  }

  // Lock GDI+ bitmap and copy pixels
  Gdiplus::BitmapData bmpData;
  Gdiplus::Rect rect(0, 0, imgWidth, imgHeight);
  bmp->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData);

  for (unsigned int y = 0; y < imgHeight; y++) {
    BYTE* src = (BYTE*)bmpData.Scan0 + y * bmpData.Stride;
    BYTE* dst = m_pixels + y * m_width * 4;
    for (unsigned int x = 0; x < imgWidth; x++) {
      // GDI+ BGRA -> our internal ARGB (as D3D A8R8G8B8 in memory on LE):
      // Memory layout for A8R8G8B8 on little-endian: B, G, R, A bytes
      // GDI+ gives us: B, G, R, A bytes in PixelFormat32bppARGB
      // So they're actually the same memory layout!
      dst[x * 4 + 0] = src[x * 4 + 0]; // B
      dst[x * 4 + 1] = src[x * 4 + 1]; // G
      dst[x * 4 + 2] = src[x * 4 + 2]; // R
      dst[x * 4 + 3] = src[x * 4 + 3]; // A
    }
  }

  bmp->UnlockBits(&bmpData);
  delete bmp;
  return true;
}

void CSurface::Clear()
{
  if (m_pixels)
    memset(m_pixels, 0, m_width * m_height * m_bpp);
}

bool CSurface::Lock(CSurfaceRect* rect)
{
  if (!m_pixels || !rect) return false;
  rect->pBits = m_pixels;
  rect->Pitch = m_width * m_bpp;
  m_locked = true;
  return true;
}

bool CSurface::Unlock()
{
  m_locked = false;
  return true;
}
