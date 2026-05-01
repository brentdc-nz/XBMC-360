#ifndef HEADER_CURL_MBEDTLS_H
#define HEADER_CURL_MBEDTLS_H
/***************************************************************************
 * mbedTLS backend header for libcurl — Xbox 360 port
 ***************************************************************************/
#include "curl_setup.h"

#ifdef USE_MBEDTLS

int  Curl_mbedtls_init(void);
void Curl_mbedtls_cleanup(void);

CURLcode Curl_mbedtls_connect(struct connectdata *conn, int sockindex);
CURLcode Curl_mbedtls_connect_nonblocking(struct connectdata *conn,
                                           int sockindex, bool *done);

void Curl_mbedtls_close_all(struct SessionHandle *data);
void Curl_mbedtls_close(struct connectdata *conn, int sockindex);
void Curl_mbedtls_session_free(void *ptr);
size_t Curl_mbedtls_version(char *buffer, size_t size);

/* API setup for mbedTLS */
#define curlssl_init()        Curl_mbedtls_init()
#define curlssl_cleanup()     Curl_mbedtls_cleanup()
#define curlssl_connect       Curl_mbedtls_connect
#define curlssl_connect_nonblocking Curl_mbedtls_connect_nonblocking
#define curlssl_session_free(x) Curl_mbedtls_session_free(x)
#define curlssl_close_all     Curl_mbedtls_close_all
#define curlssl_close         Curl_mbedtls_close
#define curlssl_shutdown(x,y) 0
#define curlssl_set_engine(x,y)         (x=x, y=y, CURLE_NOT_BUILT_IN)
#define curlssl_set_engine_default(x)   (x=x, CURLE_NOT_BUILT_IN)
#define curlssl_engines_list(x)         (x=x, (struct curl_slist *)NULL)
#define curlssl_version       Curl_mbedtls_version
#define curlssl_check_cxn(x)  (x=x, -1)
#define curlssl_data_pending(x,y) ((void)x, (void)y, 0)

#endif /* USE_MBEDTLS */
#endif /* HEADER_CURL_MBEDTLS_H */
