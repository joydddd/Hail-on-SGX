#include "aes-crypto.h"

AESCrypto::AESCrypto() {
    key = CryptoPP::SecByteBlock(CryptoPP::AES::DEFAULT_KEYLENGTH);
    iv = CryptoPP::SecByteBlock(CryptoPP::AES::BLOCKSIZE);
    prng.GenerateBlock(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
    prng.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE);

    encryptor.SetKeyWithIV(key, key.size(), iv);
}

std::string AESCrypto::encrypt_line(const CryptoPP::byte* line, int line_size) {
    std::string cipher;
    // class has implicit garbage collection - no need to free this memory.
    CryptoPP::StringSource ss(line, line_size, true /*pumpAll*/, 
                    new CryptoPP::StreamTransformationFilter(encryptor,
                        new CryptoPP::StringSink(cipher)
                    ) // StreamTransformationFilter
                );
    return encode((const CryptoPP::byte*)&cipher[0], cipher.size());
}

std::string AESCrypto::encode(const CryptoPP::byte* data, int data_size) {
    CryptoPP::Base64Encoder encoder;
    std::string encoded;
    encoder.Attach(new CryptoPP::StringSink(encoded));

    encoder.Put(data, data_size);
    encoder.MessageEnd();

    return encoded;
}

std::string AESCrypto::decode(const std::string& encoded_line) {
    CryptoPP::Base64Decoder decoder;
    std::string decoded;
    decoder.Attach(new CryptoPP::StringSink(decoded));

    decoder.Put((const CryptoPP::byte*)&encoded_line[0], encoded_line.size());
    decoder.MessageEnd();

    return decoded;
}

std::string AESCrypto::get_key_and_iv(CryptoPP::RSAES<CryptoPP::OAEP<CryptoPP::SHA256> >::Encryptor& rsa_encryptor) {
    std::string enc_key;
    std::string enc_iv;
    CryptoPP::ArraySource keysource(key, key.size(), true, /* pump all data */
        new CryptoPP::PK_EncryptorFilter(prng, rsa_encryptor,
            new CryptoPP::StringSink(enc_key)
        )
    );

    CryptoPP::ArraySource ivsource(iv, iv.size(), true, /* pump all data */
        new CryptoPP::PK_EncryptorFilter(prng, rsa_encryptor,
            new CryptoPP::StringSink(enc_iv)
        )
    );
    return encode((const CryptoPP::byte*)enc_key.data(), enc_key.size()) + '\t' + encode((const CryptoPP::byte*)enc_iv.data(), enc_iv.size());
}

// std::string AESCrypto::rsa_encrypt(std::string& input, CryptoPP::RSAES<CryptoPP::OAEP<CryptoPP::SHA256> >::Encryptor& rsa_encryptor) {
//     std::string enc_input;
//     CryptoPP::StringSource keysource(input, true, /* pump all data */
//         new CryptoPP::PK_EncryptorFilter(prng, rsa_encryptor,
//             new CryptoPP::StringSink(enc_input)
//         )
//     );
//     return;
// }