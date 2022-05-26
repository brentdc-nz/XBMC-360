#ifndef H_LIBSMB2_XBOX_DEFINES
#define H_LIBSMB2_XBOX_DEFINES

#include <xtl.h>
#include <stddef.h>

typedef unsigned char uint8_t;

#define uint64_t unsigned __int64
#define uint32_t unsigned __int32
#define uint16_t unsigned __int16
#define uint_t unsigned int

typedef short int16_t;
typedef short int_least16_t;
typedef int int32_t;
typedef long long int64_t;
typedef int intptr_t;

#define snprintf _snprintf
#define _U_ 

//#define offsetof(st, m) ((size_t)((char *)&((st *)(1024))->m - (char *)1024))

#endif // H_LIBSMB2_XBOX_DEFINES