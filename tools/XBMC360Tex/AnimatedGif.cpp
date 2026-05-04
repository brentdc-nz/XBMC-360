// AnimatedGif.cpp - GIF animation loader for XBMC360Tex
// Adapted from xbmc4xbox's AnimatedGif.cpp
// Copyright (c) 2000, Juan Soulie <jsoulie@cplusplus.com>

#include "AnimatedGif.h"
#include "xbox360.h"

#define fopen_utf8 fopen

#pragma pack(1)

#ifndef BI_RGB
#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L
#endif

#define SWAP16(X) (void)X
#define SWAP32(X) (void)X

#define ERRORMSG(PARAM) {}

int LZWDecoder(char*, char*, short, int, int, int, const int);

CAnimatedGif::CAnimatedGif()
{
  Height = Width = 0;
  Raster = NULL;
  Palette = NULL;
  pbmi = NULL;
  BPP = Transparent = BytesPerRow = 0;
  xPos = yPos = Delay = Transparency = 0;
  nLoops = 1;
}

CAnimatedGif::~CAnimatedGif()
{
  delete[] pbmi;
  delete[] Raster;
}

void CAnimatedGif::Init(int iWidth, int iHeight, int iBPP, int iLoops)
{
  if (Raster) { delete[] Raster; Raster = NULL; }
  if (pbmi) { delete[] pbmi; pbmi = NULL; }

  Transparent = -1;
  BytesPerRow = Width = iWidth;
  Height = iHeight;
  BPP = iBPP;
  xPos = yPos = Delay = Transparency = 0;
  nLoops = iLoops;

  if (BPP == 24) {
    BytesPerRow *= 3;
    pbmi = (GUIBITMAPINFO*)new char[sizeof(GUIBITMAPINFO)];
  } else {
    pbmi = (GUIBITMAPINFO*)new char[sizeof(GUIBITMAPINFOHEADER) + (1 << BPP) * sizeof(COLOR)];
    Palette = (COLOR*)((char*)pbmi + sizeof(GUIBITMAPINFOHEADER));
  }

  BytesPerRow += (ALIGN - Width % ALIGN) % ALIGN;
  int size = BytesPerRow * Height;
  Raster = new char[size];

  pbmi->bmiHeader.biSize = sizeof(GUIBITMAPINFOHEADER);
  pbmi->bmiHeader.biWidth = Width;
  pbmi->bmiHeader.biHeight = -Height;
  pbmi->bmiHeader.biPlanes = 1;
  pbmi->bmiHeader.biBitCount = (BPP < 8 ? 8 : BPP);
  pbmi->bmiHeader.biCompression = BI_RGB;
  pbmi->bmiHeader.biSizeImage = 0;
  pbmi->bmiHeader.biXPelsPerMeter = 11811;
  pbmi->bmiHeader.biYPelsPerMeter = 11811;
  pbmi->bmiHeader.biClrUsed = 0;
  pbmi->bmiHeader.biClrImportant = 0;
}

CAnimatedGif& CAnimatedGif::operator=(CAnimatedGif& rhs)
{
  Init(rhs.Width, rhs.Height, rhs.BPP);
  memcpy(Raster, rhs.Raster, BytesPerRow * Height);
  memcpy((char*)Palette, (char*)rhs.Palette, (1 << BPP) * sizeof(*Palette));
  return *this;
}

CAnimatedGifSet::CAnimatedGifSet()
{
  FrameHeight = FrameWidth = 0;
  nLoops = 1;
}

CAnimatedGifSet::~CAnimatedGifSet()
{
  Release();
}

void CAnimatedGifSet::Release()
{
  FrameWidth = 0;
  FrameHeight = 0;
  for (int i = 0; i < (int)m_vecimg.size(); ++i)
    delete m_vecimg[i];
  m_vecimg.clear();
}

void CAnimatedGifSet::AddImage(CAnimatedGif* newimage)
{
  m_vecimg.push_back(newimage);
}

int CAnimatedGifSet::GetImageCount() const
{
  return (int)m_vecimg.size();
}

unsigned char CAnimatedGifSet::getbyte(FILE* fd)
{
  unsigned char uchar;
  if (fread(&uchar, 1, 1, fd) == 1)
    return uchar;
  return 0;
}

int CAnimatedGifSet::LoadGIF(const char* szFileName)
{
  int n;
  int GlobalBPP;
  COLOR* GlobalColorMap;

  struct GIFGCEtag {
    unsigned char BlockSize;
    unsigned char PackedFields;
    unsigned short Delay;
    unsigned char Transparent;
  } gifgce;

  struct GIFNetscapeTag {
    unsigned char comment[11];
    unsigned char SubBlockLength;
    unsigned char reserved;
    unsigned short iIterations;
  } gifnetscape;

  int GraphicExtensionFound = 0;

  FILE* fd = fopen_utf8(szFileName, "rb");
  if (!fd) return 0;

  char szSignature[6];
  int iRead = (int)fread(szSignature, 1, 6, fd);
  if (iRead != 6) { fclose(fd); return 0; }
  if (memcmp(szSignature, "GIF", 3) != 0) { fclose(fd); return 0; }

  struct GIFLSDtag {
    unsigned short ScreenWidth;
    unsigned short ScreenHeight;
    unsigned char PackedFields;
    unsigned char Background;
    unsigned char PixelAspectRatio;
  } giflsd;

  iRead = (int)fread(&giflsd, 1, sizeof(giflsd), fd);
  if (iRead != sizeof(giflsd)) { fclose(fd); return 0; }

  SWAP16(giflsd.ScreenWidth);
  SWAP16(giflsd.ScreenHeight);

  GlobalBPP = (giflsd.PackedFields & 0x07) + 1;
  FrameWidth = giflsd.ScreenWidth;
  FrameHeight = giflsd.ScreenHeight;
  nLoops = 1;

  GlobalColorMap = new COLOR[1 << GlobalBPP];
  if (giflsd.PackedFields & 0x80)
    for (n = 0; n < 1 << GlobalBPP; n++) {
      GlobalColorMap[n].r = getbyte(fd);
      GlobalColorMap[n].g = getbyte(fd);
      GlobalColorMap[n].b = getbyte(fd);
    }
  else
    for (n = 0; n < 256; n++)
      GlobalColorMap[n].r = GlobalColorMap[n].g = GlobalColorMap[n].b = (unsigned char)n;

  do {
    int charGot = getbyte(fd);

    if (charGot == 0x21) {
      unsigned char extensionType = getbyte(fd);
      switch (extensionType) {
      case 0xF9:
        if (fread((char*)&gifgce, 1, sizeof(gifgce), fd) == sizeof(gifgce))
          SWAP16(gifgce.Delay);
        GraphicExtensionFound++;
        getbyte(fd);
        break;
      case 0xFE:
        while (int nBlockLength = getbyte(fd))
          for (n = 0; n < nBlockLength; n++) getbyte(fd);
        break;
      case 0x01:
        while (int nBlockLength = getbyte(fd))
          for (n = 0; n < nBlockLength; n++) getbyte(fd);
        break;
      case 0xFF: {
        int nBlockLength = getbyte(fd);
        if (nBlockLength == 0x0b) {
          struct GIFNetscapeTag tag;
          if (fread((char*)&tag, 1, sizeof(gifnetscape), fd) == sizeof(gifnetscape)) {
            SWAP16(tag.iIterations);
            nLoops = tag.iIterations;
          } else
            nLoops = 0;
          if (nLoops) nLoops++;
          getbyte(fd);
        } else {
          do {
            for (n = 0; n < nBlockLength; n++) getbyte(fd);
          } while ((nBlockLength = getbyte(fd)) != 0);
        }
        break;
      }
      default:
        while (int nBlockLength = getbyte(fd))
          for (n = 0; n < nBlockLength; n++) getbyte(fd);
        break;
      }
    }
    else if (charGot == 0x2c) {
      CAnimatedGif* NextImage = new CAnimatedGif();

      struct GIFIDtag {
        unsigned short xPos;
        unsigned short yPos;
        unsigned short Width;
        unsigned short Height;
        unsigned char PackedFields;
      } gifid;

      memset(&gifid, 0, sizeof(gifid));
      int LocalColorMap = 0;

      if (fread((char*)&gifid, 1, sizeof(gifid), fd) == sizeof(gifid)) {
        SWAP16(gifid.xPos);
        SWAP16(gifid.yPos);
        SWAP16(gifid.Width);
        SWAP16(gifid.Height);
        LocalColorMap = (gifid.PackedFields & 0x80) ? 1 : 0;
      }

      NextImage->Init(gifid.Width, gifid.Height,
                      LocalColorMap ? (gifid.PackedFields & 7) + 1 : GlobalBPP);

      NextImage->xPos = gifid.xPos;
      NextImage->yPos = gifid.yPos;
      if (GraphicExtensionFound) {
        NextImage->Transparent = (gifgce.PackedFields & 0x01) ? gifgce.Transparent : -1;
        NextImage->Transparency = (gifgce.PackedFields & 0x1c) > 1 ? 1 : 0;
        NextImage->Delay = gifgce.Delay * 10;
      }

      if (NextImage->Transparent != -1)
        memset(NextImage->Raster, NextImage->Transparent, NextImage->BytesPerRow * NextImage->Height);
      else
        memset(NextImage->Raster, giflsd.Background, NextImage->BytesPerRow * NextImage->Height);

      size_t palSize = sizeof(COLOR) * (1 << NextImage->BPP);
      bool isPalRead = false;
      if (LocalColorMap && fread((char*)NextImage->Palette, 1, palSize, fd) == palSize)
        isPalRead = true;
      if (!isPalRead)
        memcpy(NextImage->Palette, GlobalColorMap, palSize);

      short firstbyte = getbyte(fd);

      long ImgStart, ImgEnd;
      ImgEnd = ImgStart = ftell(fd);
      while ((n = getbyte(fd)) != 0) fseek(fd, ImgEnd += n + 1, SEEK_SET);
      fseek(fd, ImgStart, SEEK_SET);

      char* pCompressedImage = new char[ImgEnd - ImgStart + 4];
      char* pTemp = pCompressedImage;
      while (int nBlockLength = getbyte(fd)) {
        fread(pTemp, 1, nBlockLength, fd);
        pTemp += nBlockLength;
      }

      n = LZWDecoder(pCompressedImage, (char*)NextImage->Raster,
                     firstbyte, NextImage->BytesPerRow,
                     gifid.Width, gifid.Height,
                     ((gifid.PackedFields & 0x40) ? 1 : 0));

      if (n)
        AddImage(NextImage);
      else
        delete NextImage;

      delete[] pCompressedImage;
      GraphicExtensionFound = 0;
    }
    else if (charGot == 0x3b) {
      break;
    }
  } while (!feof(fd));

  delete[] GlobalColorMap;
  fclose(fd);
  return GetImageCount();
}

int LZWDecoder(char* bufIn, char* bufOut,
               short InitCodeSize, int AlignedWidth,
               int Width, int Height, const int Interlace)
{
  int n;
  int row = 0, col = 0;
  int nPixels, maxPixels;

  short CodeSize;
  short ClearCode;
  short EndCode;

  long whichBit;
  long LongCode;
  short Code;
  short PrevCode;
  short OutCode;

  short Prefix[4096];
  unsigned char Suffix[4096];
  short FirstEntry;
  short NextEntry;

  unsigned char OutStack[4097];
  int OutIndex;
  int RowOffset;

  CodeSize = InitCodeSize + 1;
  ClearCode = (1 << InitCodeSize);
  EndCode = ClearCode + 1;
  NextEntry = FirstEntry = ClearCode + 2;

  whichBit = 0;
  nPixels = 0;
  maxPixels = Width * Height;
  RowOffset = 0;
  PrevCode = 0;

  while (nPixels < maxPixels) {
    OutIndex = 0;

    LongCode = *((long*)(bufIn + whichBit / 8));
    LongCode >>= (whichBit & 7);
    Code = (short)((LongCode & ((1 << CodeSize) - 1)));
    whichBit += CodeSize;

    if (Code == EndCode) break;

    if (Code == ClearCode) {
      CodeSize = InitCodeSize + 1;
      NextEntry = FirstEntry;
      PrevCode = Code;
      continue;
    }

    if (Code < NextEntry)
      OutCode = Code;
    else {
      OutIndex++;
      OutCode = PrevCode;
    }

    while (OutCode >= FirstEntry) {
      if (OutIndex > 4096 || OutCode >= 4096) return 0;
      OutStack[OutIndex++] = Suffix[OutCode];
      OutCode = Prefix[OutCode];
    }

    if (OutIndex > 4096) return 0;
    OutStack[OutIndex++] = (unsigned char)OutCode;

    if (PrevCode != ClearCode) {
      Prefix[NextEntry] = PrevCode;
      Suffix[NextEntry] = (unsigned char)OutCode;
      NextEntry++;
      if (NextEntry >= 4096) return 0;
      if (NextEntry >= (1 << CodeSize)) {
        if (CodeSize < 12) CodeSize++;
      }
    }

    PrevCode = Code;

    if (nPixels + OutIndex > maxPixels) OutIndex = maxPixels - nPixels;

    for (n = OutIndex - 1; n >= 0; n--) {
      if (col == Width) {
        if (Interlace) {
          if ((row & 7) == 0) { row += 8; if (row >= Height) row = 4; }
          else if ((row & 3) == 0) { row += 8; if (row >= Height) row = 2; }
          else if ((row & 1) == 0) { row += 4; if (row >= Height) row = 1; }
          else row += 2;
        } else
          row++;
        RowOffset = row * AlignedWidth;
        col = 0;
      }
      bufOut[RowOffset + col] = OutStack[n];
      col++;
      nPixels++;
    }
  }

  return (int)whichBit;
}

#pragma pack()
