/*
 * Custom hardware entropy source for Xbox 360
 *
 * Implements mbedtls_hardware_poll() using Xbox 360 QueryPerformanceCounter
 * and GetTickCount as entropy sources. This is adequate for an embedded
 * console where /dev/urandom and CryptGenRandom are not available.
 */

#if defined(MBEDTLS_CONFIG_FILE)
#include MBEDTLS_CONFIG_FILE
#else
#include "mbedtls/config.h"
#endif

#if defined(MBEDTLS_ENTROPY_HARDWARE_ALT)

#include <xtl.h>
#include <string.h>
#include "mbedtls/entropy.h"

int mbedtls_hardware_poll(void *data, unsigned char *output,
                          size_t len, size_t *olen)
{
    LARGE_INTEGER qpc;
    DWORD tick;
    size_t pos = 0;
    size_t chunk;

    (void)data;

    while (pos < len) {
        QueryPerformanceCounter(&qpc);
        tick = GetTickCount();

        /* Mix QPC and tick count together */
        qpc.QuadPart ^= ((LONGLONG)tick << 16) ^ ((LONGLONG)tick);

        chunk = sizeof(qpc.QuadPart);
        if (chunk > len - pos)
            chunk = len - pos;

        memcpy(output + pos, &qpc.QuadPart, chunk);
        pos += chunk;
    }

    *olen = len;
    return 0;
}

#endif /* MBEDTLS_ENTROPY_HARDWARE_ALT */
