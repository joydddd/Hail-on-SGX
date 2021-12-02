#include <cryptopp/cryptlib.h>
#include <cryptopp/rijndael.h>
#include <cryptopp/modes.h>
#include <cryptopp/files.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/rsa.h>

#include <iostream>
#include <string>
#include <mutex>

#ifndef _AES_CRYPTO_H_
#define _AES_CRYPTO_H_

class AESCrypto {
    public:
        AESCrypto();

        std::string encrypt_line(const byte* line, int line_size);

        std::string encode(const byte* data, int data_size);

        std::string decode(const std::string& encoded_data);

        std::string get_key_and_iv(CryptoPP::RSAES<CryptoPP::OAEP<CryptoPP::SHA256> >::Encryptor& rsa_encryptor);

    private:
        CryptoPP::AutoSeededRandomPool prng;
        CryptoPP::SecByteBlock key;
        CryptoPP::SecByteBlock iv;
        CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryptor;
        // TEST PURPOSES ONLY! DELETE THIS SOON!!!
        //CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryptor;

        CryptoPP::Base64Encoder encoder;
        std::string encoded;
        CryptoPP::Base64Decoder decoder;
        std::string decoded;

        std::mutex encoding_lock;

        std::mutex decoding_lock;

        
};


#endif