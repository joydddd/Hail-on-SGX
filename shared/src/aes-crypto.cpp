#include "aes-crypto.h"

AESCrypto::AESCrypto() {
    key = CryptoPP::SecByteBlock(CryptoPP::AES::DEFAULT_KEYLENGTH);
    iv = CryptoPP::SecByteBlock(CryptoPP::AES::BLOCKSIZE);
    prng.GenerateBlock(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
    prng.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE);

    // std::string s1("2973AC956985BE259E84A48E29132793");
    // std::string s2("0118D0B93074455740D175BDEF3475C6");
    
    // key = CryptoPP::SecByteBlock((const byte*)&s1[0], CryptoPP::AES::DEFAULT_KEYLENGTH);
    // iv = CryptoPP::SecByteBlock((const byte*)&s2[0], CryptoPP::AES::BLOCKSIZE);
    encryptor.SetKeyWithIV(key, key.size(), iv);
    //decryptor.SetKeyWithIV(key, key.size(), iv);

    encoder.Attach(new CryptoPP::StringSink(encoded));
    decoder.Attach(new CryptoPP::StringSink(decoded));
}

std::string AESCrypto::encrypt_line(const byte* line, int line_size) {
    std::string cipher;
    // class has implicit garbage collection - no need to free this memory.
    CryptoPP::StringSource ss(line, line_size, true /*pumpAll*/, 
                    new CryptoPP::StreamTransformationFilter(encryptor,
                        new CryptoPP::StringSink(cipher)
                    ) // StreamTransformationFilter
                );

    return encode((const byte*)&cipher[0], cipher.size());
}

std::string AESCrypto::encode(const byte* data, int data_size) {
    std::lock_guard<std::mutex> raii(encoding_lock);
    encoded.clear();
    encoder.Put(data, data_size);
    encoder.MessageEnd();

    return encoded;
}

std::string AESCrypto::decode(const std::string& encoded_line) {
    std::lock_guard<std::mutex> raii(decoding_lock);
    decoded.clear();
    decoder.Put((const byte*)&encoded_line[0], encoded_line.size());
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
    return encode((const byte*)enc_key.data(), enc_key.size()) + '\t' + encode((const byte*)enc_iv.data(), enc_iv.size());
}