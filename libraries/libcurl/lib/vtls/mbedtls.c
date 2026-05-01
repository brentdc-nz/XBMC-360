/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * mbedTLS backend for libcurl — Xbox 360 port
 * Based on the PolarSSL backend from libcurl 7.37.1, adapted for
 * mbedTLS 2.28 API.
 *
 ***************************************************************************/

#include "curl_setup.h"

#ifdef USE_MBEDTLS

#include <mbedtls/ssl.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/version.h>

#ifdef _XBOX
#include <xtl.h>
#include <winsockx.h>
#else
#include <mbedtls/net_sockets.h>
#endif

#include "urldata.h"
#include "sendf.h"
#include "inet_pton.h"
#include "mbedtls.h"
#include "vtls.h"
#include "connect.h"
#include "select.h"
#include "rawstr.h"

#define _MPRINTF_REPLACE
#include <curl/mprintf.h>
#include "curl_memory.h"
/* The last #include file should be: */
#include "memdebug.h"

/* Global entropy context — shared across all connections */
static mbedtls_entropy_context entropy;
static int entropy_initialized = 0;

static Curl_recv mbedtls_recv_func;
static Curl_send mbedtls_send_func;

/*
 * Custom BIO callbacks for mbedTLS — wraps recv()/send() on the curl socket.
 * Xbox 360 doesn't have MBEDTLS_NET_C (no _WIN32/unix), so we provide our own.
 */
static int mbedtls_bio_recv(void *ctx, unsigned char *buf, size_t len)
{
  curl_socket_t fd = *(curl_socket_t *)ctx;
  int ret = recv((SOCKET)fd, (char *)buf, (int)len, 0);
  if(ret < 0) {
#ifdef _XBOX
    int err = WSAGetLastError();
    if(err == WSAEWOULDBLOCK)
      return MBEDTLS_ERR_SSL_WANT_READ;
#endif
    return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
  }
  return ret;
}

static int mbedtls_bio_send(void *ctx, const unsigned char *buf, size_t len)
{
  curl_socket_t fd = *(curl_socket_t *)ctx;
  int ret = send((SOCKET)fd, (const char *)buf, (int)len, 0);
  if(ret < 0) {
#ifdef _XBOX
    int err = WSAGetLastError();
    if(err == WSAEWOULDBLOCK)
      return MBEDTLS_ERR_SSL_WANT_WRITE;
#endif
    return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
  }
  return ret;
}

static void mbedtls_init_entropy(void)
{
  if(!entropy_initialized) {
    mbedtls_entropy_init(&entropy);
    entropy_initialized = 1;
  }
}

int Curl_mbedtls_init(void)
{
  return 1; /* nothing to do globally */
}

void Curl_mbedtls_cleanup(void)
{
  if(entropy_initialized) {
    mbedtls_entropy_free(&entropy);
    entropy_initialized = 0;
  }
}

static CURLcode
mbedtls_connect_step1(struct connectdata *conn, int sockindex)
{
  struct SessionHandle *data = conn->data;
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];
  int ret;
  char errorbuf[128];

  mbedtls_init_entropy();

  /* Initialize mbedTLS structures */
  mbedtls_ssl_init(&connssl->ssl);
  mbedtls_ssl_config_init(&connssl->config);
  mbedtls_ctr_drbg_init(&connssl->ctr_drbg);
  mbedtls_x509_crt_init(&connssl->cacert);
  mbedtls_x509_crt_init(&connssl->clicert);
  mbedtls_pk_init(&connssl->pk);

  /* Seed the RNG */
  ret = mbedtls_ctr_drbg_seed(&connssl->ctr_drbg, mbedtls_entropy_func,
                               &entropy, NULL, 0);
  if(ret) {
    failf(data, "mbedTLS: ctr_drbg_seed returned (-0x%04X)", -ret);
    return CURLE_SSL_CONNECT_ERROR;
  }

  /* Set up SSL config — TLS client defaults */
  ret = mbedtls_ssl_config_defaults(&connssl->config,
                                     MBEDTLS_SSL_IS_CLIENT,
                                     MBEDTLS_SSL_TRANSPORT_STREAM,
                                     MBEDTLS_SSL_PRESET_DEFAULT);
  if(ret) {
    failf(data, "mbedTLS: ssl_config_defaults returned (-0x%04X)", -ret);
    return CURLE_SSL_CONNECT_ERROR;
  }

  /* Set TLS version range based on curl options */
  switch(data->set.ssl.version) {
  case CURL_SSLVERSION_DEFAULT:
  case CURL_SSLVERSION_TLSv1:
    mbedtls_ssl_conf_min_version(&connssl->config,
                                  MBEDTLS_SSL_MAJOR_VERSION_3,
                                  MBEDTLS_SSL_MINOR_VERSION_1);
    break;
  case CURL_SSLVERSION_TLSv1_0:
    mbedtls_ssl_conf_min_version(&connssl->config,
                                  MBEDTLS_SSL_MAJOR_VERSION_3,
                                  MBEDTLS_SSL_MINOR_VERSION_1);
    mbedtls_ssl_conf_max_version(&connssl->config,
                                  MBEDTLS_SSL_MAJOR_VERSION_3,
                                  MBEDTLS_SSL_MINOR_VERSION_1);
    break;
  case CURL_SSLVERSION_TLSv1_1:
    mbedtls_ssl_conf_min_version(&connssl->config,
                                  MBEDTLS_SSL_MAJOR_VERSION_3,
                                  MBEDTLS_SSL_MINOR_VERSION_2);
    mbedtls_ssl_conf_max_version(&connssl->config,
                                  MBEDTLS_SSL_MAJOR_VERSION_3,
                                  MBEDTLS_SSL_MINOR_VERSION_2);
    break;
  case CURL_SSLVERSION_TLSv1_2:
    mbedtls_ssl_conf_min_version(&connssl->config,
                                  MBEDTLS_SSL_MAJOR_VERSION_3,
                                  MBEDTLS_SSL_MINOR_VERSION_3);
    mbedtls_ssl_conf_max_version(&connssl->config,
                                  MBEDTLS_SSL_MAJOR_VERSION_3,
                                  MBEDTLS_SSL_MINOR_VERSION_3);
    break;
  case CURL_SSLVERSION_SSLv2:
  case CURL_SSLVERSION_SSLv3:
    failf(data, "mbedTLS does not support SSLv2 or SSLv3");
    return CURLE_SSL_CONNECT_ERROR;
  }

  /* Certificate verification — skip by default on Xbox 360 (no CA store) */
  if(data->set.ssl.verifypeer) {
    mbedtls_ssl_conf_authmode(&connssl->config, MBEDTLS_SSL_VERIFY_REQUIRED);
  }
  else {
    mbedtls_ssl_conf_authmode(&connssl->config, MBEDTLS_SSL_VERIFY_NONE);
  }

#ifdef MBEDTLS_FS_IO
  /* CA certs from file if specified */
  if(data->set.str[STRING_SSL_CAFILE]) {
    ret = mbedtls_x509_crt_parse_file(&connssl->cacert,
                                       data->set.str[STRING_SSL_CAFILE]);
    if(ret < 0) {
      failf(data, "mbedTLS: x509_crt_parse_file returned (-0x%04X)", -ret);
      if(data->set.ssl.verifypeer)
        return CURLE_SSL_CACERT_BADFILE;
    }
    mbedtls_ssl_conf_ca_chain(&connssl->config, &connssl->cacert, NULL);
  }

  /* Client cert if specified */
  if(data->set.str[STRING_CERT]) {
    ret = mbedtls_x509_crt_parse_file(&connssl->clicert,
                                       data->set.str[STRING_CERT]);
    if(ret) {
      failf(data, "mbedTLS: x509_crt_parse_file (client) returned (-0x%04X)",
            -ret);
      return CURLE_SSL_CERTPROBLEM;
    }
  }

  /* Client private key */
  if(data->set.str[STRING_KEY]) {
    ret = mbedtls_pk_parse_keyfile(&connssl->pk,
                                    data->set.str[STRING_KEY],
                                    data->set.str[STRING_KEY_PASSWD]);
    if(ret) {
      failf(data, "mbedTLS: pk_parse_keyfile returned (-0x%04X)", -ret);
      return CURLE_SSL_CERTPROBLEM;
    }
    mbedtls_ssl_conf_own_cert(&connssl->config, &connssl->clicert,
                               &connssl->pk);
  }
#endif /* MBEDTLS_FS_IO */

  /* Set RNG */
  mbedtls_ssl_conf_rng(&connssl->config, mbedtls_ctr_drbg_random,
                        &connssl->ctr_drbg);

  /* Apply config to SSL context */
  ret = mbedtls_ssl_setup(&connssl->ssl, &connssl->config);
  if(ret) {
    failf(data, "mbedTLS: ssl_setup returned (-0x%04X)", -ret);
    return CURLE_SSL_CONNECT_ERROR;
  }

  /* Set hostname for SNI */
  if(conn->host.name) {
    struct in_addr addr4;
#ifdef ENABLE_IPV6
    struct in6_addr addr6;
#endif
    /* Only set SNI for hostnames, not IP addresses */
    if(!Curl_inet_pton(AF_INET, conn->host.name, &addr4)
#ifdef ENABLE_IPV6
       && !Curl_inet_pton(AF_INET6, conn->host.name, &addr6)
#endif
       ) {
      mbedtls_ssl_set_hostname(&connssl->ssl, conn->host.name);
    }
  }

  /* Set I/O callbacks — use custom BIO wrapping curl's socket */
  mbedtls_ssl_set_bio(&connssl->ssl, &conn->sock[sockindex],
                       mbedtls_bio_send, mbedtls_bio_recv, NULL);

  infof(data, "mbedTLS: Connecting to %s:%d\n",
        conn->host.name, conn->remote_port);

  connssl->connecting_state = ssl_connect_2;
  return CURLE_OK;
}

static CURLcode
mbedtls_connect_step2(struct connectdata *conn, int sockindex)
{
  int ret;
  struct SessionHandle *data = conn->data;
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];

  conn->recv[sockindex] = mbedtls_recv_func;
  conn->send[sockindex] = mbedtls_send_func;

  for(;;) {
    ret = mbedtls_ssl_handshake(&connssl->ssl);
    if(ret == 0)
      break;

    if(ret != MBEDTLS_ERR_SSL_WANT_READ &&
       ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
      char errorbuf[128];
      mbedtls_strerror(ret, errorbuf, sizeof(errorbuf));
      failf(data, "mbedTLS: ssl_handshake returned (-0x%04X) %s",
            -ret, errorbuf);
      return CURLE_SSL_CONNECT_ERROR;
    }

    if(ret == MBEDTLS_ERR_SSL_WANT_READ) {
      connssl->connecting_state = ssl_connect_2_reading;
      return CURLE_OK;
    }
    if(ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
      connssl->connecting_state = ssl_connect_2_writing;
      return CURLE_OK;
    }
  }

  /* Check certificate verification result */
  if(data->set.ssl.verifypeer) {
    uint32_t flags = mbedtls_ssl_get_verify_result(&connssl->ssl);
    if(flags != 0) {
      if(flags & MBEDTLS_X509_BADCERT_EXPIRED)
        failf(data, "Cert verify failed: BADCERT_EXPIRED");
      if(flags & MBEDTLS_X509_BADCERT_REVOKED)
        failf(data, "Cert verify failed: BADCERT_REVOKED");
      if(flags & MBEDTLS_X509_BADCERT_CN_MISMATCH)
        failf(data, "Cert verify failed: BADCERT_CN_MISMATCH");
      if(flags & MBEDTLS_X509_BADCERT_NOT_TRUSTED)
        failf(data, "Cert verify failed: BADCERT_NOT_TRUSTED");
      return CURLE_PEER_FAILED_VERIFICATION;
    }
  }

  infof(data, "mbedTLS: Handshake complete, cipher is %s\n",
        mbedtls_ssl_get_ciphersuite(&connssl->ssl));

  connssl->connecting_state = ssl_connect_3;
  infof(data, "SSL connected\n");

  return CURLE_OK;
}

static CURLcode
mbedtls_connect_step3(struct connectdata *conn, int sockindex)
{
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];

  DEBUGASSERT(ssl_connect_3 == connssl->connecting_state);

  connssl->connecting_state = ssl_connect_done;
  return CURLE_OK;
}

static ssize_t mbedtls_send_func(struct connectdata *conn,
                                  int sockindex,
                                  const void *mem,
                                  size_t len,
                                  CURLcode *curlcode)
{
  int ret;

  ret = mbedtls_ssl_write(&conn->ssl[sockindex].ssl,
                           (unsigned char *)mem, len);
  if(ret < 0) {
    *curlcode = (ret == MBEDTLS_ERR_SSL_WANT_WRITE) ?
      CURLE_AGAIN : CURLE_SEND_ERROR;
    ret = -1;
  }

  return (ssize_t)ret;
}

static ssize_t mbedtls_recv_func(struct connectdata *conn,
                                  int num,
                                  char *buf,
                                  size_t buffersize,
                                  CURLcode *curlcode)
{
  int ret;

  ret = mbedtls_ssl_read(&conn->ssl[num].ssl,
                          (unsigned char *)buf, buffersize);

  if(ret <= 0) {
    if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
      return 0;

    *curlcode = (ret == MBEDTLS_ERR_SSL_WANT_READ) ?
      CURLE_AGAIN : CURLE_RECV_ERROR;
    return -1;
  }

  return (ssize_t)ret;
}

void Curl_mbedtls_close_all(struct SessionHandle *data)
{
  (void)data;
}

void Curl_mbedtls_close(struct connectdata *conn, int sockindex)
{
  mbedtls_pk_free(&conn->ssl[sockindex].pk);
  mbedtls_x509_crt_free(&conn->ssl[sockindex].clicert);
  mbedtls_x509_crt_free(&conn->ssl[sockindex].cacert);
  mbedtls_ssl_config_free(&conn->ssl[sockindex].config);
  mbedtls_ctr_drbg_free(&conn->ssl[sockindex].ctr_drbg);
  mbedtls_ssl_free(&conn->ssl[sockindex].ssl);
}

void Curl_mbedtls_session_free(void *ptr)
{
  free(ptr);
}

size_t Curl_mbedtls_version(char *buffer, size_t size)
{
  unsigned int version = mbedtls_version_get_number();
  return snprintf(buffer, size, "mbedTLS/%d.%d.%d",
                  (version >> 24) & 0xff,
                  (version >> 16) & 0xff,
                  (version >> 8) & 0xff);
}

static CURLcode
mbedtls_connect_common(struct connectdata *conn, int sockindex,
                        bool nonblocking, bool *done)
{
  CURLcode retcode;
  struct SessionHandle *data = conn->data;
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];
  curl_socket_t sockfd = conn->sock[sockindex];
  long timeout_ms;
  int what;

  if(ssl_connection_complete == connssl->state) {
    *done = TRUE;
    return CURLE_OK;
  }

  if(ssl_connect_1 == connssl->connecting_state) {
    timeout_ms = Curl_timeleft(data, NULL, TRUE);
    if(timeout_ms < 0) {
      failf(data, "SSL connection timeout");
      return CURLE_OPERATION_TIMEDOUT;
    }
    retcode = mbedtls_connect_step1(conn, sockindex);
    if(retcode)
      return retcode;
  }

  while(ssl_connect_2 == connssl->connecting_state ||
        ssl_connect_2_reading == connssl->connecting_state ||
        ssl_connect_2_writing == connssl->connecting_state) {

    timeout_ms = Curl_timeleft(data, NULL, TRUE);
    if(timeout_ms < 0) {
      failf(data, "SSL connection timeout");
      return CURLE_OPERATION_TIMEDOUT;
    }

    if(connssl->connecting_state == ssl_connect_2_reading ||
       connssl->connecting_state == ssl_connect_2_writing) {

      curl_socket_t writefd = ssl_connect_2_writing ==
        connssl->connecting_state ? sockfd : CURL_SOCKET_BAD;
      curl_socket_t readfd = ssl_connect_2_reading ==
        connssl->connecting_state ? sockfd : CURL_SOCKET_BAD;

      what = Curl_socket_ready(readfd, writefd,
                               nonblocking ? 0 : timeout_ms);
      if(what < 0) {
        failf(data, "select/poll on SSL socket, errno: %d", SOCKERRNO);
        return CURLE_SSL_CONNECT_ERROR;
      }
      else if(0 == what) {
        if(nonblocking) {
          *done = FALSE;
          return CURLE_OK;
        }
        failf(data, "SSL connection timeout");
        return CURLE_OPERATION_TIMEDOUT;
      }
    }

    retcode = mbedtls_connect_step2(conn, sockindex);
    if(retcode || (nonblocking &&
                   (ssl_connect_2 == connssl->connecting_state ||
                    ssl_connect_2_reading == connssl->connecting_state ||
                    ssl_connect_2_writing == connssl->connecting_state))) {
      if(!retcode && nonblocking) {
        *done = FALSE;
      }
      return retcode;
    }
  }

  if(ssl_connect_3 == connssl->connecting_state) {
    retcode = mbedtls_connect_step3(conn, sockindex);
    if(retcode)
      return retcode;
  }

  if(ssl_connect_done == connssl->connecting_state) {
    connssl->state = ssl_connection_complete;
    conn->recv[sockindex] = mbedtls_recv_func;
    conn->send[sockindex] = mbedtls_send_func;
    *done = TRUE;
  }
  else
    *done = FALSE;

  connssl->connecting_state = ssl_connect_1;
  return CURLE_OK;
}

CURLcode
Curl_mbedtls_connect_nonblocking(struct connectdata *conn,
                                  int sockindex, bool *done)
{
  return mbedtls_connect_common(conn, sockindex, TRUE, done);
}

CURLcode
Curl_mbedtls_connect(struct connectdata *conn, int sockindex)
{
  CURLcode retcode;
  bool done = FALSE;

  retcode = mbedtls_connect_common(conn, sockindex, FALSE, &done);
  if(retcode)
    return retcode;

  DEBUGASSERT(done);
  return CURLE_OK;
}

#endif /* USE_MBEDTLS */
