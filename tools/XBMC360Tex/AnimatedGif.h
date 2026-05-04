// AnimatedGif.h - GIF animation loader for XBMC360Tex
// Simplified from xbmc4xbox's AnimatedGif.h

#pragma once

#include <windows.h>
#include <vector>
#include <cstdio>

#pragma pack(1)

struct COLOR { unsigned char b, g, r, x; };
#undef ALIGN
#define ALIGN sizeof(int)

typedef struct tagGUIRGBQUAD
{
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} GUIRGBQUAD;

typedef struct tagGUIBITMAPINFOHEADER
{
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} GUIBITMAPINFOHEADER;

typedef struct tagGUIBITMAPINFO
{
  GUIBITMAPINFOHEADER bmiHeader;
  GUIRGBQUAD bmiColors[1];
} GUIBITMAPINFO;

#pragma pack()

typedef PALETTEENTRY PALETTEENTRY_GIF;

class CAnimatedGif
{
public:
  CAnimatedGif();
  virtual ~CAnimatedGif();

  int Width, Height;
  int BPP;
  char* Raster;
  COLOR* Palette;
  int BytesPerRow;
  int Transparent;

  int nLoops;
  int xPos, yPos;
  int Delay;
  int Transparency;
  GUIBITMAPINFO* pbmi;

  CAnimatedGif& operator=(CAnimatedGif& rhs);
  void Init(int iWidth, int iHeight, int iBPP, int iLoops = 0);
  inline char& Pixel(int x, int y) { return Raster[y * BytesPerRow + x]; }
};

class CAnimatedGifSet
{
public:
  CAnimatedGifSet();
  virtual ~CAnimatedGifSet();

  int FrameWidth, FrameHeight;
  int nLoops;

  std::vector<CAnimatedGif*> m_vecimg;

  void AddImage(CAnimatedGif*);
  int GetImageCount() const;
  int LoadGIF(const char* szFile);
  void Release();

protected:
  unsigned char getbyte(FILE* fd);
};

#pragma pack()
