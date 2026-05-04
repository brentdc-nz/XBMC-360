// lzo_compress.h - LZO compression wrapper for the build tool
// Links against the prebuilt lzo.lib from xbmc4xbox

#ifndef LZO_COMPRESS_H
#define LZO_COMPRESS_H

// Matches LZO 1.00 types from LZOCONF.H
typedef unsigned int lzo_uint;
typedef unsigned int lzo_uint32;
typedef unsigned char* lzo_bytep;
typedef void* lzo_voidp;

// Function pointer types needed for sizeof() in lzo_init macro
typedef int (__cdecl *lzo_compress_t)(const unsigned char*, lzo_uint,
                                     unsigned char*, lzo_uint*, lzo_voidp);

#define LZO_VERSION 0x1000
#define LZO_E_OK 0
#define LZO1X_999_MEM_COMPRESS ((lzo_uint)(14 * 16384L * sizeof(unsigned char*)))

#ifdef __cplusplus
extern "C" {
#endif

// The real init function has this signature (exported as ___lzo_init in lib)
int __cdecl __lzo_init(unsigned, int, int, int, int, int, int, int);

// lzo_init() is a macro that checks type sizes match between caller and lib
#define lzo_init() __lzo_init(LZO_VERSION,(int)sizeof(short),(int)sizeof(int),\
    (int)sizeof(long),(int)sizeof(lzo_uint32),(int)sizeof(lzo_uint),\
    (int)sizeof(lzo_voidp),(int)sizeof(lzo_compress_t))

int __cdecl lzo1x_999_compress(const unsigned char* src, lzo_uint src_len,
                               unsigned char* dst, lzo_uint* dst_len,
                               lzo_voidp wrkmem);

int __cdecl lzo1x_optimize(unsigned char* in, lzo_uint in_len,
                           unsigned char* out, lzo_uint* out_len,
                           lzo_voidp wrkmem);

#ifdef __cplusplus
}
#endif

#endif // LZO_COMPRESS_H
