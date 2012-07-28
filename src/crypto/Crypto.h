// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/err.h>

#include <stdio.h>
#include <string>
#include <string.h>

#define DEBUG

#ifdef DEBUG
#define DEFAULT_RSA_KEYLEN 1024
#define DEFAULT_AES_KEYLEN 128
#define AES_ROUNDS 3
#else
#define DEFAULT_RSA_KEYLEN 2048
#define DEFAULT_AES_KEYLEN 256
#define AES_ROUNDS 6
#endif

//#define PSUEDO_CLIENT

#define SALT         "alsa_channel_control"
#define AES_KEY_PASS "alsa_channel_control"

#define SUCCESS 0
#define FAILURE -1

#define KEY_SERVER_PRI 0
#define KEY_SERVER_PUB 1
#define KEY_CLIENT_PUB 2

#ifndef CRYPTO_H
#define CRYPTO_H

class Crypto {
public:
    Crypto(int isClient=0);

    Crypto(unsigned char *remotePubKey, size_t remotePubKeyLen, int isClient=0);

    Crypto(unsigned char *remotePubKey, size_t remotePubKeyLen, size_t rsaKeyLen, size_t aesKeyLen, int isClient=0);

    ~Crypto();

    int rsaEncrypt(std::string msg, unsigned char **encMsg);

    int rsaEncrypt(const unsigned char *msg, size_t msgLen, unsigned char **encMsg);

    int aesEncrypt(std::string msg, unsigned char **encMsg);

    int aesEncrypt(const unsigned char *msg, size_t msgLen, unsigned char **encMsg);

    std::string rsaDecrypt(unsigned char *encMsg, size_t encMsgLen);

    int rsaDecrypt(unsigned char *encMsg, size_t encMsgLen, unsigned char **decMsg);

    std::string aesDecrypt(unsigned char *encMsg, size_t encMsgLen);

    int aesDecrypt(unsigned char *encMsg, size_t encMsgLen, char **decMsg);

    int writeKeyToFile(FILE *fd, int key);

    int getRemotePubKey(unsigned char **pubKey);

    int setRemotePubKey(unsigned char *pubKey, size_t pubKeyLen);

    int getLocalPubKey(unsigned char **pubKey);

    int getLocalPriKey(unsigned char **priKey);

    int getAESKey(unsigned char **aesKey);

    int setAESKey(unsigned char *aesKey, size_t aesKeyLen);

private:
    static EVP_PKEY *localKeypair;
    EVP_PKEY *remotePubKey;

    EVP_CIPHER_CTX *rsaClientEncryptCtx;
    EVP_MD_CTX     *rsaServerEncryptCtx;
    EVP_CIPHER_CTX *aesEncryptCtx;

    EVP_CIPHER_CTX *rsaServerDecryptCtx;
    EVP_MD_CTX     *rsaClientDecryptCtx;
    EVP_CIPHER_CTX *aesDecryptCtx;

    unsigned char *rsaSymKey;
    int rsaSymKeyLen;
    unsigned char *rsaIV;

    unsigned char *aesKey;
    unsigned char *aesIV;
    int aesKeyLen;

    size_t encryptLen;

    int mIsClient;

    int init(size_t rsaKeyLen, size_t aesKeyLen);
    int genTestClientKey(int keyLen);
};

#endif