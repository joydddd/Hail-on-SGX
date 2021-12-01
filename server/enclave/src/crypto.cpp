// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include "crypto.h"
#include <stdlib.h>
#include <string.h>

Crypto::Crypto() {
    m_initialized = false;
    int res = -1;

    mbedtls_ctr_drbg_init(&m_ctr_drbg_contex);
    mbedtls_entropy_init(&m_entropy_context);
    mbedtls_pk_init(&m_pk_context);

    // Initialize entropy.
    res = mbedtls_ctr_drbg_seed(
        &m_ctr_drbg_contex,
        mbedtls_entropy_func,
        &m_entropy_context,
        nullptr,
        0);
    if (res != 0) {
        return;
    }

    // Initialize RSA context.
    res = mbedtls_pk_setup(
        &m_pk_context, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
    if (res != 0) {
        return;
    }

    // Generate an ephemeral 2048-bit RSA key pair with
    // exponent 65537 for the enclave.
    res = mbedtls_rsa_gen_key(
        mbedtls_pk_rsa(m_pk_context),
        mbedtls_ctr_drbg_random,
        &m_ctr_drbg_contex,
        2048,
        65537);
    if (res != 0) {
        return;
    }

    // Write out the public key in PEM format for exchange with other enclaves.
    res = mbedtls_pk_write_pubkey_pem(&m_pk_context, m_public_key, sizeof(m_public_key));
    if (res != 0) {
        return;
    }
    m_initialized = true;
}

Crypto::~Crypto() {
    mbedtls_pk_free(&m_pk_context);
    mbedtls_entropy_free(&m_entropy_context);
    mbedtls_ctr_drbg_free(&m_ctr_drbg_contex);
}

// Compute the sha256 hash of given data.
int Crypto::Sha256(const uint8_t* data, size_t data_size, uint8_t sha256[32])
{
    int ret = 0;
    mbedtls_sha256_context ctx;

    mbedtls_sha256_init(&ctx);

    ret = mbedtls_sha256_starts_ret(&ctx, 0);
    if (ret)
        goto exit;

    ret = mbedtls_sha256_update_ret(&ctx, data, data_size);
    if (ret)
        goto exit;

    ret = mbedtls_sha256_finish_ret(&ctx, sha256);
    if (ret)
        goto exit;

exit:
    mbedtls_sha256_free(&ctx);
    return ret;
}

/**
 * Encrypt encrypts the given data using the given public key.
 * Used to encrypt data using the public key of another enclave.
 */
bool Crypto::Encrypt(
    const uint8_t* pem_public_key,
    const uint8_t* data,
    size_t data_size,
    uint8_t* encrypted_data,
    size_t* encrypted_data_size) {
    bool result = false;
    mbedtls_pk_context key;
    size_t key_size = 0;
    int res = -1;
    mbedtls_rsa_context* rsa_context;

    mbedtls_pk_init(&key);

    if (!m_initialized)
        goto exit;

    // Read the given public key.
    key_size = strlen((const char*)pem_public_key) + 1; // Include ending '\0'.
    res = mbedtls_pk_parse_public_key(&key, pem_public_key, key_size);
    if (res != 0) {
        goto exit;
    }

    rsa_context = mbedtls_pk_rsa(key);
    rsa_context->padding = MBEDTLS_RSA_PKCS_V21;
    rsa_context->hash_id = MBEDTLS_MD_SHA256;

    // Encrypt the data.
    res = mbedtls_rsa_pkcs1_encrypt(
        rsa_context,
        mbedtls_ctr_drbg_random,
        &m_ctr_drbg_contex,
        MBEDTLS_RSA_PUBLIC,
        data_size,
        data,
        encrypted_data);
    if (res != 0) {
        goto exit;
    }

    *encrypted_data_size = rsa_context->len;
    result = true;
exit:
    mbedtls_pk_free(&key);
    return result;
}

/**
 * decrypt the given data using current enclave's private key.
 * Used to receive encrypted data from another enclave.
 */
bool Crypto::decrypt(
    const uint8_t* encrypted_data,
    size_t encrypted_data_size,
    uint8_t* data,
    size_t* data_size) {
    bool ret = false;
    size_t output_size = 0;
    int res = 0;
    mbedtls_rsa_context* rsa_context;

    if (!m_initialized)
        goto exit;

    mbedtls_pk_rsa(m_pk_context)->len = encrypted_data_size;
    rsa_context = mbedtls_pk_rsa(m_pk_context);
    rsa_context->padding = MBEDTLS_RSA_PKCS_V21;
    rsa_context->hash_id = MBEDTLS_MD_SHA256;

    output_size = *data_size;
    res = mbedtls_rsa_pkcs1_decrypt(
        rsa_context,
        mbedtls_ctr_drbg_random,
        &m_ctr_drbg_contex,
        MBEDTLS_RSA_PRIVATE,
        &output_size,
        encrypted_data,
        data,
        output_size);
    if (res != 0) {
        goto exit;
    }
    *data_size = output_size;
    ret = true;

exit:
    return ret;
}