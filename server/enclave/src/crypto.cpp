// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include "crypto.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>

void aes_decrypt_data(mbedtls_aes_context* aes_context, 
                      unsigned char* aes_iv, 
                      const unsigned char* input_data, 
                      int input_size, 
                      unsigned char* output_data) {
    int ret = mbedtls_aes_crypt_cbc(
                aes_context,
                MBEDTLS_AES_DECRYPT,
                input_size, // input data length in bytes,
                aes_iv, // Initialization vector (updated after use)
                input_data,
                output_data);
    if (ret != 0) {
        std::cout << "Decryption failed with error: " << ret << std::endl;
        exit(0);
    }
}

RSACrypto::RSACrypto() {
    m_initialized = false;
    int res = -1;

    mbedtls_ctr_drbg_init(&m_ctr_drbg_context);
    mbedtls_entropy_init(&m_entropy_context);
    mbedtls_pk_init(&m_pk_context);

    // Initialize entropy.
    res = mbedtls_ctr_drbg_seed(
        &m_ctr_drbg_context,
        mbedtls_entropy_func,
        &m_entropy_context,
        nullptr,
        0);
    if (res != 0) {
        return;
    }

    // Initialize RSA context.
    res = mbedtls_pk_setup(&m_pk_context, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
    if (res != 0) {
        return;
    }

    // Generate an ephemeral 2048-bit RSA key pair with
    // exponent 65537 for the enclave.
    res = mbedtls_rsa_gen_key(
        mbedtls_pk_rsa(m_pk_context),
        mbedtls_ctr_drbg_random,
        &m_ctr_drbg_context,
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

RSACrypto::~RSACrypto() {
    mbedtls_pk_free(&m_pk_context);
    mbedtls_entropy_free(&m_entropy_context);
    mbedtls_ctr_drbg_free(&m_ctr_drbg_context);
}

// Compute the sha256 hash of given data.
int RSACrypto::sha256(const uint8_t* data, size_t data_size, uint8_t sha256[32]) {
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
 * decrypt the given data using current enclave's private key.
 * Used to receive encrypted data from another enclave.
 */
bool RSACrypto::decrypt(
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
    res = mbedtls_rsa_rsaes_oaep_decrypt(
        rsa_context,
        mbedtls_ctr_drbg_random,
        &m_ctr_drbg_context,
        MBEDTLS_RSA_PRIVATE,
        NULL,
        0,
        &output_size,
        encrypted_data,
        data,
        output_size);
    if (res != 0) {
        std::cout << "RSA decryption failed with error: " << res << std::endl;
        goto exit;
    }
    *data_size = output_size;
    ret = true;

exit:
    return ret;
}

uint8_t* RSACrypto::get_pub_key() {
    return m_public_key;
}