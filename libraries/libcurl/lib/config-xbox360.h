#ifndef HEADER_CURL_CONFIG_XBOX360_H
#define HEADER_CURL_CONFIG_XBOX360_H
/***************************************************************************
 * Hand crafted config file for Xbox 360 XDK
 * Based on config-win32.h from libcurl 7.37.1
 ***************************************************************************/

/* ---------------------------------------------------------------- */
/*                    Xbox 360 specific settings                    */
/* ---------------------------------------------------------------- */

/* Disable protocols we don't need */
#define CURL_DISABLE_LDAP    1
#define CURL_DISABLE_TELNET  1
#define CURL_DISABLE_DICT    1
#define CURL_DISABLE_TFTP    1
#define CURL_DISABLE_RTMP    1
#define CURL_DISABLE_IMAP    1
#define CURL_DISABLE_POP3    1
#define CURL_DISABLE_RTSP    1
#define CURL_DISABLE_GOPHER  1
#define CURL_DISABLE_SMTP    1
#define CURL_DISABLE_FILE    1
#define CURL_DISABLE_FTP     1

/* No SSL/TLS on Xbox 360 */
#define CURL_DISABLE_CRYPTO_AUTH 1

/* Use synchronous DNS (Xbox 360 doesn't have getaddrinfo_a) */
#define USE_SYNC_DNS 1

/* ---------------------------------------------------------------- */
/*                          HEADER FILES                            */
/* ---------------------------------------------------------------- */

#define HAVE_ASSERT_H    1
#define HAVE_ERRNO_H     1
#define HAVE_LIMITS_H    1
#define HAVE_STDLIB_H    1
#define HAVE_TIME_H      1

/* Xbox 360 XDK has these via xtl.h/winsockx.h */
#define HAVE_WINDOWS_H   1
#define HAVE_WINSOCK_H   1
#define HAVE_WINSOCK2_H  1

/* Xbox 360 XDK does NOT have these */
/* #undef HAVE_ARPA_INET_H */
/* #undef HAVE_NETDB_H */
/* #undef HAVE_NETINET_IN_H */
/* #undef HAVE_SYS_SOCKET_H */
/* #undef HAVE_SYS_SELECT_H */
/* #undef HAVE_SYS_TIME_H */
/* #undef HAVE_SYS_PARAM_H */
/* #undef HAVE_UNISTD_H */
/* #undef HAVE_FCNTL_H */
/* #undef HAVE_IO_H */
/* #undef HAVE_PROCESS_H */
/* #undef HAVE_SIGNAL_H */
/* #undef HAVE_LOCALE_H */
/* #undef HAVE_SYS_STAT_H */
/* #undef HAVE_SYS_TYPES_H */
/* #undef HAVE_SYS_UTIME_H */
/* #undef HAVE_WS2TCPIP_H */

/* ---------------------------------------------------------------- */
/*                        OTHER HEADER INFO                         */
/* ---------------------------------------------------------------- */

#define HAVE_SIG_ATOMIC_T    1
#define STDC_HEADERS         1
#define HAVE_STRUCT_TIMEVAL  1

/* ---------------------------------------------------------------- */
/*                          SIZE DEFINES                            */
/* ---------------------------------------------------------------- */

#define SIZEOF_SHORT     2
#define SIZEOF_INT       4
#define SIZEOF_SIZE_T    4
#define SIZEOF_TIME_T    4

/* Xbox 360 OS identification string */
#define OS "ppc-xbox360"

/* ---------------------------------------------------------------- */
/*                    MISSING POSIX / CRT TYPES                     */
/* ---------------------------------------------------------------- */

/* Xbox 360 XDK doesn't have <sys/stat.h>. Provide minimal struct stat
   so that formdata.c can compile. stat()/fstat() will fail at runtime
   but we don't use multipart form uploads on Xbox 360.             */
#ifndef _STAT_DEFINED
#define _STAT_DEFINED
struct stat {
  unsigned long st_mode;
  long long     st_size;
};
#define S_ISDIR(m) (0)  /* never true on Xbox 360 */
int stat(const char *path, struct stat *buf);
int fstat(int fd, struct stat *buf);
#endif

/* ---------------------------------------------------------------- */
/*                             FUNCTIONS                            */
/* ---------------------------------------------------------------- */

#define HAVE_CLOSESOCKET 1
#define HAVE_GETHOSTBYADDR 1
/* #undef HAVE_GETHOSTNAME -- Xbox 360 has no gethostname(); Curl_gethostname will return -1 */
#define HAVE_INET_ADDR 1
#define HAVE_IOCTLSOCKET 1
#define HAVE_IOCTLSOCKET_FIONBIO 1
#define HAVE_SELECT 1
#define HAVE_SOCKET 1
#define HAVE_STRDUP 1
#define HAVE_STRSTR 1
#define HAVE_STRICMP 1
#define HAVE_STRNICMP 1
#define HAVE_RECV 1
#define HAVE_SEND 1
#define HAVE_RECVFROM 1
#define HAVE_FTRUNCATE 1

/* Xbox 360 XDK does NOT have these */
/* #undef HAVE_GETSERVBYNAME */
/* #undef HAVE_GETPROTOBYNAME */
/* #undef HAVE_PERROR */
/* #undef HAVE_SETLOCALE */
/* #undef HAVE_SETMODE */
/* #undef HAVE_SETVBUF */
/* #undef HAVE_STRFTIME */
/* #undef HAVE_UTIME */
/* #undef HAVE_GETENV */
/* #undef HAVE_STRTOLL */

/* ---------------------------------------------------------------- */
/*                        FUNCTION SIGNATURES                       */
/* ---------------------------------------------------------------- */

#define RECV_TYPE_ARG1   SOCKET
#define RECV_TYPE_ARG2   char *
#define RECV_TYPE_ARG3   int
#define RECV_TYPE_ARG4   int
#define RECV_TYPE_RETV   int

#define RECVFROM_TYPE_ARG1 SOCKET
#define RECVFROM_TYPE_ARG2 char
#define RECVFROM_TYPE_ARG3 int
#define RECVFROM_TYPE_ARG4 int
#define RECVFROM_TYPE_ARG5 struct sockaddr
#define RECVFROM_TYPE_ARG6 int
#define RECVFROM_TYPE_RETV int

#define SEND_TYPE_ARG1   SOCKET
#define SEND_QUAL_ARG2   const
#define SEND_TYPE_ARG2   char *
#define SEND_TYPE_ARG3   int
#define SEND_TYPE_ARG4   int
#define SEND_TYPE_RETV   int

/* ---------------------------------------------------------------- */
/*                       TYPEDEF REPLACEMENTS                       */
/* ---------------------------------------------------------------- */

#define in_addr_t unsigned long
#define RETSIGTYPE void

#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
#define ssize_t int
#endif

/* ---------------------------------------------------------------- */
/*                        COMPILER SPECIFIC                         */
/* ---------------------------------------------------------------- */

/* MSVC static library */
#undef CURL_EXTERN
#define CURL_EXTERN

/* Use Windows crypto API for random */
/* #undef USE_WIN32_CRYPTO */

/* ---------------------------------------------------------------- */
/*                         END OF DEFINES                           */
/* ---------------------------------------------------------------- */

/* Workaround: Xbox 360 XDK uses xtl.h instead of windows.h */
#ifdef _XBOX
#  undef HAVE_WINDOWS_H
#endif

/* ---------------------------------------------------------------- */
/*              MISSING DEFINITIONS FROM WINSOCKX.H                 */
/* ---------------------------------------------------------------- */

/* Xbox 360 winsockx.h doesn't define SO_KEEPALIVE */
#ifndef SO_KEEPALIVE
#define SO_KEEPALIVE 0x0008
#endif

/* Needed by SIO_KEEPALIVE_VALS macro */
#ifndef IOC_VENDOR
#define IOC_VENDOR 0x18000000
#endif

/* Xbox 360 XDK doesn't provide struct hostent — define it for curl's DNS code.
   With XNET_STARTUP_BYPASS_SECURITY, gethostbyname() is available at runtime. */
#ifndef _STRUCT_HOSTENT_DEFINED
#define _STRUCT_HOSTENT_DEFINED
struct hostent {
  char  *h_name;
  char **h_aliases;
  short  h_addrtype;
  short  h_length;
  char **h_addr_list;
};
#define h_addr h_addr_list[0]

/* Declare gethostbyname so the linker can find it from xnet.lib */
struct hostent* gethostbyname(const char* name);
#endif

#endif /* HEADER_CURL_CONFIG_XBOX360_H */
