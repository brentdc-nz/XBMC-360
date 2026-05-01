/**
 * \file config-xbox360.h
 *
 * \brief Minimal mbedTLS configuration for Xbox 360 XDK (TLS 1.2 client only)
 *
 * Used as MBEDTLS_CONFIG_FILE via preprocessor define.
 */

#ifndef MBEDTLS_CONFIG_XBOX360_H
#define MBEDTLS_CONFIG_XBOX360_H

/* ----------------------------------------------------------------
 *  System capabilities
 * ---------------------------------------------------------------- */
#define MBEDTLS_HAVE_TIME           /* Xbox 360 has time() */
/* NO MBEDTLS_HAVE_TIME_DATE — gmtime_r not available */
/* NO MBEDTLS_HAVE_ASM — XDK compiler is MSVC-based, not GCC */

/* ----------------------------------------------------------------
 *  Platform / memory
 * ---------------------------------------------------------------- */
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY     /* use platform calloc/free */

/* ----------------------------------------------------------------
 *  Entropy — custom hardware source via Xbox 360 QPC
 * ---------------------------------------------------------------- */
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_ENTROPY_C

/* ----------------------------------------------------------------
 *  Random number generation
 * ---------------------------------------------------------------- */
#define MBEDTLS_CTR_DRBG_C

/* ----------------------------------------------------------------
 *  Symmetric ciphers (for TLS)
 * ---------------------------------------------------------------- */
#define MBEDTLS_AES_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_GCM_C
#define MBEDTLS_CBC_C
#define MBEDTLS_CIPHER_MODE_CBC

/* ----------------------------------------------------------------
 *  Hashes
 * ---------------------------------------------------------------- */
#define MBEDTLS_MD_C
#define MBEDTLS_MD5_C
#define MBEDTLS_SHA1_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_SHA512_C

/* ----------------------------------------------------------------
 *  Public key / certificates
 * ---------------------------------------------------------------- */
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_RSA_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_ECDSA_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_OID_C
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_PEM_PARSE_C

/* ECC curves — cover what modern servers use */
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_DP_SECP384R1_ENABLED
#define MBEDTLS_ECP_DP_SECP521R1_ENABLED

/* PKCS padding for RSA */
#define MBEDTLS_PKCS1_V15
#define MBEDTLS_PKCS1_V21

/* Key exchange families */
#define MBEDTLS_KEY_EXCHANGE_RSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED

/* ----------------------------------------------------------------
 *  SSL / TLS — client only, TLS 1.0 – 1.2
 * ---------------------------------------------------------------- */
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_PROTO_TLS1
#define MBEDTLS_SSL_PROTO_TLS1_1
#define MBEDTLS_SSL_PROTO_TLS1_2
/* NO MBEDTLS_SSL_SRV_C — client only */
/* NO MBEDTLS_SSL_PROTO_DTLS */

/* Cipher suites — enable enough for real-world servers */
#define MBEDTLS_SSL_CIPHERSUITES                        \
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,     \
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,     \
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,   \
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,   \
    MBEDTLS_TLS_RSA_WITH_AES_128_GCM_SHA256,            \
    MBEDTLS_TLS_RSA_WITH_AES_256_GCM_SHA384,            \
    MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA,               \
    MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA

/* SNI (Server Name Indication) so virtual hosts work */
#define MBEDTLS_SSL_SERVER_NAME_INDICATION

/* ----------------------------------------------------------------
 *  Features we do NOT need on Xbox 360
 * ---------------------------------------------------------------- */
/* NO MBEDTLS_NET_C — Xbox 360 is not _WIN32 or unix; custom I/O callbacks */
/* NO MBEDTLS_FS_IO — no filesystem cert loading on Xbox 360 */
/* NO MBEDTLS_TIMING_C — not needed for blocking I/O */
/* NO MBEDTLS_DEBUG_C — enable if needed for troubleshooting */
/* NO MBEDTLS_SELF_TEST — saves code size */
/* NO MBEDTLS_THREADING_C — curl handles its own threading */
/* NO MBEDTLS_CERTS_C — no built-in test certs needed */

/* Error string support — useful for debugging SSL issues */
#define MBEDTLS_ERROR_C

/* Version info */
#define MBEDTLS_VERSION_C

/* ----------------------------------------------------------------
 *  Final consistency check
 * ---------------------------------------------------------------- */
#include "mbedtls/check_config.h"

#endif /* MBEDTLS_CONFIG_XBOX360_H */
