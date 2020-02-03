//
// Created by Przemyslaw Podolski on 30/01/2020.
//

#ifndef TERMINAL_API_POC_NEXOCRYPTO_H
#define TERMINAL_API_POC_NEXOCRYPTO_H

#include <openssl/evp.h>
#include <vector>
#include <json/json.h>

class nexoCrypto
{
public:
    nexoCrypto(const char *passphrase);
    static bool encrypt(Json::Value &inputJson);
    static Json::Value decrypt(Json::Value &inOutJson);

private:
    static uint8_t *cryptoOperation(const char *data, size_t dataLen, int do_encrypt, int &outDataLen);
    static bool computeSha256withHmac(const uint8_t *data, int dataLen, std::string &sha256);
    static bool validateHmac(std::string &receivedHmac, const uint8_t *decrytpedData, int decrytedDataLen);

    struct derivedKeys
    {
        uint8_t hmac_key[32];
        uint8_t cipher_key[32];
        uint8_t iv[16];
    };

    static derivedKeys dk;
    static uint8_t ivmod[16];
};

#endif //TERMINAL_API_POC_NEXOCRYPTO_H
