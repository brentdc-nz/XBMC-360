/*
 * Xbox 360 XDK stubs for missing CRT/Win32 functions needed by libcurl
 */

#include "curl_setup.h"
#include <xtl.h>

/* Xbox 360 doesn't have stat()/fstat() — stub them out.
   We don't use multipart form uploads, so these never actually get called
   for our HTTP streaming use case. */
int stat(const char *path, struct stat *buf)
{
    (void)path;
    (void)buf;
    return -1;
}

int fstat(int fd, struct stat *buf)
{
    (void)fd;
    (void)buf;
    return -1;
}

/* Xbox 360 doesn't have ExpandEnvironmentStringsA — stub it.
   getenv.c uses this but we don't use environment variables. */
DWORD WINAPI ExpandEnvironmentStringsA(LPCSTR lpSrc, LPSTR lpDst, DWORD nSize)
{
    (void)lpSrc;
    (void)lpDst;
    (void)nSize;
    return 0;
}

/* Xbox 360 doesn't have gethostbyname() in xnet.lib.
   Implement it using XNetDnsLookup/XNetDnsRelease (async XNet DNS API).
   Based on Neptune's NptBsdSockets.cpp implementation.
   Requires XNET_STARTUP_BYPASS_SECURITY for DNS to work. */

/* Static storage for the result — matches standard gethostbyname() semantics
   where the returned pointer is valid until the next call. */
static struct hostent s_hostent;
static char  s_hostname[128];
static char  s_hostaddr[16];
static char *s_addr_list[2];

struct hostent* gethostbyname(const char* name)
{
    WSAEVENT hEvent;
    XNDNS   *pDns = NULL;
    INT      err;

    if(!name || !name[0])
        return NULL;

    hEvent = WSACreateEvent();
    if(hEvent == WSA_INVALID_EVENT)
        return NULL;

    err = XNetDnsLookup(name, hEvent, &pDns);
    if(err != 0 || !pDns) {
        WSACloseEvent(hEvent);
        return NULL;
    }

    WaitForSingleObject(hEvent, INFINITE);

    if(pDns->iStatus != 0) {
        XNetDnsRelease(pDns);
        WSACloseEvent(hEvent);
        return NULL;
    }

    /* Build the hostent result in static storage */
    strncpy(s_hostname, name, sizeof(s_hostname) - 1);
    s_hostname[sizeof(s_hostname) - 1] = '\0';

    memcpy(s_hostaddr, &pDns->aina[0], 4);
    s_addr_list[0] = s_hostaddr;
    s_addr_list[1] = NULL;

    s_hostent.h_name      = s_hostname;
    s_hostent.h_aliases   = NULL;
    s_hostent.h_addrtype  = AF_INET;
    s_hostent.h_length    = 4;
    s_hostent.h_addr_list = s_addr_list;

    XNetDnsRelease(pDns);
    WSACloseEvent(hEvent);

    return &s_hostent;
}
