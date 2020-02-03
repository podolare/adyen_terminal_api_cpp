//
// Created by Przemyslaw Podolski on 30/01/2020.
//

#include "nexoCrypto.h"
#include "POI.h"
#include "base64.h"
#include "escapeJson.h"

#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <iostream>

nexoCrypto::nexoCrypto(const char *passphrase)
{
    const uint8_t salt[] = "AdyenNexoV1Salt";
    const int16_t saltlen = sizeof(salt) - 1;
    const int16_t rounds = 4000;
    PKCS5_PBKDF2_HMAC_SHA1(passphrase, strlen(passphrase), salt, saltlen, rounds, sizeof(derivedKeys), (unsigned char *)&this->dk);
}

uint8_t *nexoCrypto::cryptoOperation(const char *data, size_t dataLen, int do_encrypt, int &outDataLen)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL || EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, NULL, NULL, do_encrypt) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    if (sizeof(dk.iv) != EVP_CIPHER_CTX_iv_length(ctx)) {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    if (sizeof(dk.cipher_key) != EVP_CIPHER_CTX_key_length(ctx)) {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    auto *out = (uint8_t *)malloc(dataLen + EVP_CIPHER_CTX_block_size(ctx));
    if (out == NULL) {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    uint8_t iv[sizeof(dk.iv)];
    for (size_t i = 0; i < sizeof(dk.iv); i++) {
        iv[i] = dk.iv[i] ^ ivmod[i];
    }
    if (EVP_CipherInit_ex(ctx, NULL, NULL, dk.cipher_key, iv, do_encrypt) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(out);
        return NULL;
    }

    if (EVP_CipherUpdate(ctx, out, &outDataLen, (uint8_t* )data, dataLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(out);
        return NULL;
    }
    int extra;
    if (EVP_CipherFinal_ex(ctx, out + outDataLen, &extra) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(out);
        return NULL;
    }
    EVP_CIPHER_CTX_free(ctx);
    outDataLen += extra;
    return out;
}

bool nexoCrypto::computeSha256withHmac(const uint8_t *data, int dataLen, std::string &sha256)
{
    uint8_t result[32];
    if (HMAC(EVP_sha256(), dk.hmac_key, sizeof(dk.hmac_key), data, dataLen, result, NULL) == NULL) {
        return false;
    }
    sha256 = base64_encode(result, 32);
    return true;
}

bool nexoCrypto::encrypt(Json::Value &inOutJson)
{
    RAND_pseudo_bytes(ivmod, sizeof(ivmod));
    std::string b64ivmod = base64_encode((unsigned const char*)ivmod, sizeof(ivmod));

    std::string escapedJson = escapeJSON(inOutJson.toStyledString());
    const char *data = escapedJson.c_str();
    size_t dataLen = escapedJson.size();

    int cryptoDataLen;
    uint8_t *cryptoData = cryptoOperation(data, dataLen, 1, cryptoDataLen);
    if (cryptoData)
    {
        inOutJson["SaleToPOIRequest"]["NexoBlob"] = base64_encode(cryptoData, cryptoDataLen);
        free(cryptoData);
        inOutJson["SaleToPOIRequest"]["SecurityTrailer"]["AdyenCryptoVersion"] = POI::getAdyenCryptoVersion();
        std::string sha256;
        if (computeSha256withHmac((uint8_t *)data, dataLen, sha256)) {
            inOutJson["SaleToPOIRequest"]["SecurityTrailer"]["Hmac"] = sha256;
        }
        inOutJson["SaleToPOIRequest"]["SecurityTrailer"]["KeyIdentifier"] = POI::getKeyIdentifier();
        inOutJson["SaleToPOIRequest"]["SecurityTrailer"]["KeyVersion"] = POI::getKeyVersion();
        inOutJson["SaleToPOIRequest"]["SecurityTrailer"]["Nonce"] = base64_encode(ivmod, 16);
        return true;
    }
    return false;
}

bool nexoCrypto::validateHmac(std::string &receivedHmac, const uint8_t *decrytpedData, int decrytedDataLen)
{
    std::string computedHmac;
    if (computeSha256withHmac(decrytpedData, decrytedDataLen, computedHmac))
    {
        if (computedHmac == receivedHmac) return true;
    }
    return false;
}

Json::Value nexoCrypto::decrypt(Json::Value &inOutJson)
{
    Json::Value response;
    std::string keyIdentifier = inOutJson["SaleToPOIResponse"]["SecurityTrailer"]["KeyIdentifier"].asString();
    if (keyIdentifier != POI::getKeyIdentifier())
    {
        std::cerr << "Wrong key identifier" << std::endl;
    }
    int keyVersion = inOutJson["SaleToPOIResponse"]["SecurityTrailer"]["KeyVersion"].asInt();
    if (keyVersion != POI::getKeyVersion())
    {
        std::cerr << "Wrong key version" << std::endl;
    }
    int adyenCryptoVer = inOutJson["SaleToPOIResponse"]["SecurityTrailer"]["AdyenCryptoVersion"].asInt();
    if (adyenCryptoVer != POI::getAdyenCryptoVersion())
    {
        std::cerr << "Wrong adyen crypto version" << std::endl;
    }

    std::string nonce = base64_decode(inOutJson["SaleToPOIResponse"]["SecurityTrailer"]["Nonce"].asString());
    memcpy(ivmod, nonce.data(), nonce.size());

    std::string nexoBlob = base64_decode(inOutJson["SaleToPOIResponse"]["NexoBlob"].asString());
    int decryptedDataLen = 0;
    uint8_t *decryptedData = cryptoOperation((const char *)nexoBlob.data(), nexoBlob.size(), 0, decryptedDataLen);
    if (decryptedData)
    {
        std::string hmac = inOutJson["SaleToPOIResponse"]["SecurityTrailer"]["Hmac"].asString();
        if (validateHmac(hmac, decryptedData, decryptedDataLen)) {
            std::string responseFromPOI{(char *) decryptedData, static_cast<size_t>(decryptedDataLen)};
            Json::CharReaderBuilder builder;
            Json::CharReader *jsonReader = builder.newCharReader();
            std::string parseError;
            if (jsonReader &&
                jsonReader->parse((const char *) decryptedData, (const char *) decryptedData + decryptedDataLen,
                                  &response, &parseError)) {
                std::cout << "Parse successful" << std::endl;
            } else {
                std::cout << "Parse failed, reason:" << parseError << std::endl;
            }
            delete jsonReader;
        }
        free(decryptedData);
    }
    return response;
}

nexoCrypto::derivedKeys nexoCrypto::dk = {0};
uint8_t nexoCrypto::ivmod[16] = {0};