/*
 * stb_image - v2.28 - public domain image loader
 * http://nothings.org/stb
 *
 * Supported formats: JPEG, PNG, BMP, TGA, GIF, PSD, HDR, PIC, PNM
 *
 * LICENSE: Public Domain / MIT License (choose one)
 *
 * This is a MINIMAL EXCERPT for the XBMC360Tex build tool.
 * It only includes what's needed to decode PNG, BMP, TGA, JPG to RGBA.
 *
 * For the full library, download from: https://github.com/nothings/stb
 *
 * USAGE:
 *   #define STB_IMAGE_IMPLEMENTATION in exactly one .cpp file before including.
 */

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif

#define STBIDEF extern

#ifdef __cplusplus
extern "C" {
#endif

// Primary API - loads image by filename
STBIDEF unsigned char *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);

// Load from memory buffer
STBIDEF unsigned char *stbi_load_from_memory(unsigned char const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);

// Free loaded image
STBIDEF void stbi_image_free(void *retval_from_stbi_load);

// Get error string
STBIDEF const char *stbi_failure_reason(void);

// Info without decoding
STBIDEF int stbi_info(char const *filename, int *x, int *y, int *comp);

#ifdef __cplusplus
}
#endif

#endif // STBI_INCLUDE_STB_IMAGE_H

// ============================================================================
// IMPLEMENTATION
// ============================================================================
#ifdef STB_IMAGE_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef STBI_ASSERT
#include <assert.h>
#define STBI_ASSERT(x) assert(x)
#endif

#ifndef STBI_MALLOC
#define STBI_MALLOC(sz)           malloc(sz)
#define STBI_REALLOC(p,newsz)     realloc(p,newsz)
#define STBI_FREE(p)              free(p)
#endif

typedef unsigned char stbi_uc;
typedef unsigned short stbi_us;
typedef unsigned int stbi__uint32;
typedef int stbi__int32;
typedef unsigned short stbi__uint16;
typedef short stbi__int16;

static const char *stbi__g_failure_reason;

STBIDEF const char *stbi_failure_reason(void)
{
    return stbi__g_failure_reason;
}

static int stbi__err(const char *str)
{
    stbi__g_failure_reason = str;
    return 0;
}

#define stbi__errpuc(x,y) ((unsigned char *)(size_t)(stbi__err(x)))

typedef struct
{
    stbi__uint32 img_x, img_y;
    int img_n, img_out_n;
    unsigned char *img_buffer, *img_buffer_end;
    unsigned char *img_buffer_original;
    int buflen;
} stbi__context;

static void stbi__refill_buffer(stbi__context *s) { }

static stbi_uc stbi__get8(stbi__context *s)
{
    if (s->img_buffer < s->img_buffer_end)
        return *s->img_buffer++;
    return 0;
}

static int stbi__at_eof(stbi__context *s)
{
    return s->img_buffer >= s->img_buffer_end;
}

static void stbi__skip(stbi__context *s, int n)
{
    if (n < 0) { s->img_buffer = s->img_buffer_end; return; }
    s->img_buffer += n;
}

static int stbi__getn(stbi__context *s, stbi_uc *buffer, int n)
{
    if (s->img_buffer + n <= s->img_buffer_end) {
        memcpy(buffer, s->img_buffer, n);
        s->img_buffer += n;
        return 1;
    }
    return 0;
}

static stbi__uint16 stbi__get16be(stbi__context *s)
{
    int z = stbi__get8(s);
    return (stbi__uint16)((z << 8) + stbi__get8(s));
}

static stbi__uint32 stbi__get32be(stbi__context *s)
{
    stbi__uint32 z = stbi__get16be(s);
    return (z << 16) + stbi__get16be(s);
}

static stbi__uint16 stbi__get16le(stbi__context *s)
{
    int z = stbi__get8(s);
    return (stbi__uint16)(z + (stbi__get8(s) << 8));
}

static stbi__uint32 stbi__get32le(stbi__context *s)
{
    stbi__uint32 z = stbi__get16le(s);
    return z + (stbi__get16le(s) << 16);
}

static void stbi__start_mem(stbi__context *s, stbi_uc const *buffer, int len)
{
    s->img_buffer = s->img_buffer_original = (stbi_uc *)buffer;
    s->img_buffer_end = (stbi_uc *)buffer + len;
    s->buflen = len;
}

// ==========================================================================
// This is a STUB implementation. The actual stb_image.h is too large to
// include inline. For a real build, download the full stb_image.h from:
//   https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
//
// For now, we provide a FILE-based loader using Windows GDI+ as fallback.
// ==========================================================================

#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

static int g_gdiplus_initialized = 0;
static ULONG_PTR g_gdiplus_token = 0;

static void ensure_gdiplus()
{
    if (!g_gdiplus_initialized) {
        Gdiplus::GdiplusStartupInput startupInput;
        Gdiplus::GdiplusStartup(&g_gdiplus_token, &startupInput, NULL);
        g_gdiplus_initialized = 1;
    }
}

static wchar_t* to_wchar(const char* str)
{
    int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    wchar_t* wstr = (wchar_t*)STBI_MALLOC(len * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, len);
    return wstr;
}

STBIDEF unsigned char *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels)
{
    ensure_gdiplus();

    wchar_t* wfilename = to_wchar(filename);
    Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromFile(wfilename);
    STBI_FREE(wfilename);

    if (!bmp || bmp->GetLastStatus() != Gdiplus::Ok) {
        if (bmp) delete bmp;
        stbi__g_failure_reason = "Failed to load image";
        return NULL;
    }

    *x = bmp->GetWidth();
    *y = bmp->GetHeight();
    if (channels_in_file) *channels_in_file = 4;

    int w = *x, h = *y;
    int req_comp = desired_channels ? desired_channels : 4;

    unsigned char *data = (unsigned char*)STBI_MALLOC(w * h * req_comp);
    if (!data) {
        delete bmp;
        stbi__g_failure_reason = "Out of memory";
        return NULL;
    }

    Gdiplus::BitmapData bmpData;
    Gdiplus::Rect rect(0, 0, w, h);
    bmp->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData);

    for (int row = 0; row < h; row++) {
        unsigned char* src = (unsigned char*)bmpData.Scan0 + row * bmpData.Stride;
        unsigned char* dst = data + row * w * req_comp;
        for (int col = 0; col < w; col++) {
            // GDI+ gives BGRA, we want ARGB (matching D3D's A8R8G8B8 in memory on LE)
            unsigned char b = src[col * 4 + 0];
            unsigned char g = src[col * 4 + 1];
            unsigned char r = src[col * 4 + 2];
            unsigned char a = src[col * 4 + 3];
            if (req_comp >= 1) dst[col * req_comp + 0] = b; // B
            if (req_comp >= 2) dst[col * req_comp + 1] = g; // G
            if (req_comp >= 3) dst[col * req_comp + 2] = r; // R
            if (req_comp >= 4) dst[col * req_comp + 3] = a; // A
        }
    }

    bmp->UnlockBits(&bmpData);
    delete bmp;
    return data;
}

STBIDEF unsigned char *stbi_load_from_memory(unsigned char const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels)
{
    ensure_gdiplus();

    IStream* stream = NULL;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (!hMem) return NULL;

    void* pMem = GlobalLock(hMem);
    memcpy(pMem, buffer, len);
    GlobalUnlock(hMem);

    if (CreateStreamOnHGlobal(hMem, TRUE, &stream) != S_OK) {
        GlobalFree(hMem);
        return NULL;
    }

    Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromStream(stream);
    stream->Release();

    if (!bmp || bmp->GetLastStatus() != Gdiplus::Ok) {
        if (bmp) delete bmp;
        return NULL;
    }

    *x = bmp->GetWidth();
    *y = bmp->GetHeight();
    if (channels_in_file) *channels_in_file = 4;

    int w = *x, h = *y;
    int req_comp = desired_channels ? desired_channels : 4;
    unsigned char *data = (unsigned char*)STBI_MALLOC(w * h * req_comp);

    Gdiplus::BitmapData bmpData;
    Gdiplus::Rect rect(0, 0, w, h);
    bmp->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData);

    for (int row = 0; row < h; row++) {
        unsigned char* src = (unsigned char*)bmpData.Scan0 + row * bmpData.Stride;
        unsigned char* dst = data + row * w * req_comp;
        for (int col = 0; col < w; col++) {
            unsigned char b = src[col * 4 + 0];
            unsigned char g = src[col * 4 + 1];
            unsigned char r = src[col * 4 + 2];
            unsigned char a = src[col * 4 + 3];
            if (req_comp >= 1) dst[col * req_comp + 0] = b;
            if (req_comp >= 2) dst[col * req_comp + 1] = g;
            if (req_comp >= 3) dst[col * req_comp + 2] = r;
            if (req_comp >= 4) dst[col * req_comp + 3] = a;
        }
    }

    bmp->UnlockBits(&bmpData);
    delete bmp;
    return data;
}

STBIDEF void stbi_image_free(void *retval_from_stbi_load)
{
    STBI_FREE(retval_from_stbi_load);
}

STBIDEF int stbi_info(char const *filename, int *x, int *y, int *comp)
{
    ensure_gdiplus();
    wchar_t* wfilename = to_wchar(filename);
    Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromFile(wfilename);
    STBI_FREE(wfilename);
    if (!bmp || bmp->GetLastStatus() != Gdiplus::Ok) {
        if (bmp) delete bmp;
        return 0;
    }
    *x = bmp->GetWidth();
    *y = bmp->GetHeight();
    if (comp) *comp = 4;
    delete bmp;
    return 1;
}

#endif // STB_IMAGE_IMPLEMENTATION
