// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include "Crypto.h"

using namespace std;

// Static member variables
EVP_PKEY* Crypto::localKeypair;

Crypto::Crypto() {
    localKeypair  = NULL;
    remotePubKey  = NULL;
    rsaSymKey     = NULL;
    rsaSymKeyLen  = 0;
    rsaIV         = NULL;
    encryptLen    = 0;

    #ifdef PSUEDO_CLIENT
        genTestClientKey(DEFAULT_RSA_KEYLEN);
    #endif

    init(DEFAULT_RSA_KEYLEN, DEFAULT_AES_KEYLEN);
}


Crypto::Crypto(unsigned char *remotePubKey, size_t remotePubKeyLen) {
    localKeypair  = NULL;
    remotePubKey  = NULL;
    rsaSymKey     = NULL;
    rsaSymKeyLen  = 0;
    rsaIV         = NULL;
    encryptLen    = 0;

    #ifdef PSUEDO_CLIENT
        genTestClientKey(DEFAULT_RSA_KEYLEN);
    #endif

    setRemotePubKey(remotePubKey, remotePubKeyLen);
    init(DEFAULT_RSA_KEYLEN, DEFAULT_AES_KEYLEN);
}


Crypto::Crypto(unsigned char *remotePubKey, size_t remotePubKeyLen, size_t rsaKeyLen, size_t aesKeyLen) {
    localKeypair  = NULL;
    remotePubKey  = NULL;
    rsaSymKey     = NULL;
    rsaSymKeyLen  = 0;
    rsaIV         = NULL;
    encryptLen    = 0;

    #ifdef PSUEDO_CLIENT
        genTestClientKey(DEFAULT_RSA_KEYLEN);
    #else
        setRemotePubKey(remotePubKey, remotePubKeyLen);
    #endif

    init(rsaKeyLen, aesKeyLen);
}


Crypto::~Crypto() {
    EVP_PKEY_free(localKeypair);
    EVP_PKEY_free(remotePubKey);

    EVP_CIPHER_CTX_cleanup(rsaEncryptCtx);
    EVP_CIPHER_CTX_cleanup(aesEncryptCtx);

    EVP_CIPHER_CTX_cleanup(rsaDecryptCtx);
    EVP_CIPHER_CTX_cleanup(aesDecryptCtx);

    free(rsaEncryptCtx);
    free(aesEncryptCtx);

    free(rsaDecryptCtx);
    free(aesDecryptCtx);

    free(rsaSymKey);
    free(rsaIV);
    
    free(aesKey);
    free(aesIV);
}


int Crypto::rsaEncrypt(const unsigned char *msg, size_t msgLen, unsigned char **encMsg, unsigned char **ek, size_t *ekl, unsigned char **iv, size_t *ivl) {
    size_t encMsgLen = 0;
    size_t blockLen  = 0;

    *ek = (unsigned char*)malloc(EVP_PKEY_size(remotePubKey));
    *iv = (unsigned char*)malloc(EVP_MAX_IV_LENGTH);
    *ivl = EVP_MAX_IV_LENGTH;

    *encMsg = (unsigned char*)malloc(EVP_PKEY_size(remotePubKey));
    if(encMsg == NULL) return FAILURE;

    if(!EVP_SealInit(rsaEncryptCtx, EVP_aes_256_cbc(), ek, (int*)ekl, *iv, &remotePubKey, 1)) {
        return FAILURE;
    }

    if(!EVP_SealUpdate(rsaEncryptCtx, *encMsg + encMsgLen, (int*)&blockLen, (const unsigned char*)msg, (int)msgLen)) {
        return FAILURE;
    }
    encMsgLen += blockLen;

    if(!EVP_SealFinal(rsaEncryptCtx, *encMsg + encMsgLen, (int*)&blockLen)) {
        return FAILURE;
    }
    encMsgLen += blockLen;

    //EVP_CIPHER_CTX_cleanup(rsaEncryptCtx);

    return (int)encMsgLen;
}


int Crypto::aesEncrypt(const unsigned char *msg, size_t msgLen, unsigned char **encMsg) {
    size_t blockLen  = 0;
    size_t encMsgLen = 0;

    *encMsg = (unsigned char*)malloc(msgLen + AES_BLOCK_SIZE);
    if(encMsg == NULL) return FAILURE;

    char *err = (char*)malloc(130);

    if(!EVP_EncryptInit_ex(aesEncryptCtx, EVP_aes_256_cbc(), NULL, aesKey, aesIV)) {
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        fprintf(stderr, "Error encrypting message1: %s\n", err);
        return FAILURE;
    }

    if(!EVP_EncryptUpdate(aesEncryptCtx, *encMsg, (int*)&blockLen, (unsigned char*)msg, msgLen)) {
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        fprintf(stderr, "Error encrypting message2: %s\n", err);
        return FAILURE;
    }
    encMsgLen += blockLen;

    if(!EVP_EncryptFinal_ex(aesEncryptCtx, *encMsg + encMsgLen, (int*)&blockLen)) {
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        fprintf(stderr, "Error encrypting message3: %s\n", err);
        return FAILURE;
    }

    //EVP_CIPHER_CTX_cleanup(aesEncryptCtx);

    return encMsgLen + blockLen;
}


int Crypto::rsaDecrypt(unsigned char *encMsg, size_t encMsgLen, unsigned char *ek, size_t ekl, unsigned char *iv, size_t ivl, unsigned char **decMsg) {
    size_t decLen   = 0;
    size_t blockLen = 0;
    EVP_PKEY *key;

    *decMsg = (unsigned char*)malloc(encMsgLen + ivl);
    if(decMsg == NULL) return FAILURE;

    /*#ifdef PSUEDO_CLIENT
        key = remotePubKey;
    #else*/
        key = localKeypair;
    //#endif

    char *err = (char*)malloc(130);

    if(!EVP_OpenInit(rsaDecryptCtx, EVP_aes_256_cbc(), ek, ekl, iv, key)) {
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        fprintf(stderr, "Error decrypting message1: %s\n", err);
        return FAILURE;
    }

    if(!EVP_OpenUpdate(rsaDecryptCtx, (unsigned char*)*decMsg + decLen, (int*)&blockLen, encMsg, (int)encMsgLen)) {
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        fprintf(stderr, "Error decrypting message2: %s\n", err);
        return FAILURE;
    }
    decLen += blockLen;

    if(!EVP_OpenFinal(rsaDecryptCtx, (unsigned char*)*decMsg + decLen, (int*)&blockLen)) {
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        fprintf(stderr, "Error decrypting message3: %s\n", err);
        return FAILURE;
    }
    decLen += blockLen;

    (*decMsg)[decLen] = '\0';

    //EVP_CIPHER_CTX_cleanup(rsaDecryptCtx);

    return (int)decLen;
}


int Crypto::aesDecrypt(unsigned char *encMsg, size_t encMsgLen, char **decMsg) {
    size_t decLen   = 0;
    size_t blockLen = 0;

    *decMsg = (char*)malloc(encMsgLen);
    if(*decMsg == NULL) return FAILURE;

    if(!EVP_DecryptInit_ex(aesDecryptCtx, EVP_aes_256_cbc(), NULL, aesKey, aesIV)) {
        return FAILURE;
    }

    if(!EVP_DecryptUpdate(aesDecryptCtx, (unsigned char*)*decMsg, (int*)&blockLen, encMsg, (int)encMsgLen)) {
        return FAILURE;
    }
    decLen += blockLen;

    if(!EVP_DecryptFinal_ex(aesDecryptCtx, (unsigned char*)*decMsg + decLen, (int*)&blockLen)) {
        return FAILURE;
    }
    decLen += blockLen;

    (*decMsg)[decLen] = '\0';

    EVP_CIPHER_CTX_cleanup(aesDecryptCtx);

    return encMsgLen;
}


int Crypto::writeKeyToFile(FILE *fd, int key) {
    if(key == KEY_SERVER_PRI) {
        if(!PEM_write_PrivateKey(fd, localKeypair, NULL, NULL, 0, 0, NULL)) {
            return FAILURE;
        }
    } else if(key == KEY_SERVER_PUB) {
        if(!PEM_write_PUBKEY(fd, localKeypair)) {
            return FAILURE;
        }
    } else if(key == KEY_CLIENT_PUB) {
        if(!PEM_write_PUBKEY(fd, remotePubKey)) {
            return FAILURE;
        }
    }

    return SUCCESS;
}


int Crypto::getRemotePubKey(unsigned char **pubKey) {
    BIO *bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(bio, remotePubKey);

    int pubKeyLen = BIO_pending(bio);
    *pubKey = (unsigned char*)malloc(pubKeyLen);
    if(pubKey == NULL) return FAILURE;
    
    BIO_read(bio, *pubKey, pubKeyLen);

    // Insert the null terminator
    (*pubKey)[pubKeyLen-1] = '\0';

    BIO_free_all(bio);

    return pubKeyLen;
}



int Crypto::setRemotePubKey(unsigned char* pubKey, size_t pubKeyLen) {
    //BIO *bio = BIO_new(BIO_f_base64());
    BIO *bio = BIO_new(BIO_s_mem());
    if(BIO_write(bio, pubKey, pubKeyLen) != (int)pubKeyLen) {
        return FAILURE;
    }

    RSA *_pubKey = (RSA*)malloc(sizeof(RSA));
    if(_pubKey == NULL) return FAILURE;

    //remotePubKey = EVP_PKEY_new();

    //PEM_read_bio_RSAPublicKey(bio, &_pubKey, NULL, NULL);
    /*PEM_read_bio_RSA_PUBKEY(bio, &_pubKey, NULL, NULL);
    
    if(!EVP_PKEY_assign_RSA(remotePubKey, _pubKey)) {
        return FAILURE;
    }*/

    PEM_read_bio_PUBKEY(bio, &remotePubKey, NULL, NULL);

    BIO_free_all(bio);

    return SUCCESS;
}

int Crypto::getLocalPubKey(unsigned char** pubKey) {
    BIO *bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(bio, localKeypair);

    int pubKeyLen = BIO_pending(bio);
    *pubKey = (unsigned char*)malloc(pubKeyLen);
    if(pubKey == NULL) return FAILURE;
    
    BIO_read(bio, *pubKey, pubKeyLen);

    // Insert the null terminator
    (*pubKey)[pubKeyLen-1] = '\0';

    BIO_free_all(bio);

    return pubKeyLen;
}


int Crypto::getLocalPriKey(unsigned char **priKey) {
    BIO *bio = BIO_new(BIO_s_mem());

    // TODO fix this! It doesn't work.
    PEM_write_bio_PrivateKey(bio, localKeypair, EVP_aes_128_cbc(), rsaSymKey, rsaSymKeyLen, 0, NULL);

    int priKeyLen = BIO_pending(bio);
    *priKey = (unsigned char*)malloc(priKeyLen + 1);
    if(priKey == NULL) return FAILURE;
    
    BIO_read(bio, *priKey, priKeyLen);

    // Insert the null terminator
    (*priKey)[priKeyLen] = '\0';

    BIO_free_all(bio);

    return priKeyLen;
}


int Crypto::getAESKey(unsigned char **aesKey) {
    *aesKey = this->aesKey;
    return aesKeyLen;
}


int Crypto::setAESKey(unsigned char *aesKey, size_t aesKeyLen) {
    // Ensure the new key isn't larger than the one currently being used
    if((int)aesKeyLen > this->aesKeyLen) {
        return FAILURE;
    }

    strncpy((char*)this->aesKey, (const char*)aesKey, aesKeyLen);
    this->aesKeyLen = aesKeyLen;

    return SUCCESS;
}



int Crypto::init(size_t rsaKeyLen, size_t aesKeyLen) {
    // Initalize contexts
    rsaEncryptCtx = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));
    aesEncryptCtx = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));

    rsaDecryptCtx = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));
    aesDecryptCtx = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));

    // Always a good idea to check if malloc failed
    if(rsaEncryptCtx == NULL || aesEncryptCtx == NULL || rsaDecryptCtx == NULL || aesDecryptCtx == NULL) {
        return FAILURE;
    }

    // Init these here to make valgrind happy
    EVP_CIPHER_CTX_init(rsaEncryptCtx);
    EVP_CIPHER_CTX_init(aesEncryptCtx);

    EVP_CIPHER_CTX_init(rsaDecryptCtx);
    EVP_CIPHER_CTX_init(aesDecryptCtx);

    // Init RSA
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);

    if(EVP_PKEY_keygen_init(ctx) <= 0) {
        return FAILURE;
    }

    if(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, (int)rsaKeyLen) <= 0) {
        return FAILURE;
    }

    if(EVP_PKEY_keygen(ctx, &localKeypair) <= 0) {
        return FAILURE;
    }

    EVP_PKEY_CTX_free(ctx);

    rsaSymKey = (unsigned char*)malloc(rsaKeyLen/8);
    rsaIV = (unsigned char*)malloc(EVP_MAX_IV_LENGTH);

    // Init AES
    aesKey = (unsigned char*)malloc(aesKeyLen/8 + EVP_MAX_IV_LENGTH);
    aesIV = (unsigned char*)malloc(aesKeyLen/8);
    this->aesKeyLen = aesKeyLen/8 + EVP_MAX_IV_LENGTH;

    if(rsaSymKey == NULL || rsaIV == NULL || aesKey == NULL || aesIV == NULL) {
        return FAILURE;
    }
    
    if(!EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), (const unsigned char*)SALT, (const unsigned char*)AES_KEY_PASS, strlen(AES_KEY_PASS), AES_ROUNDS, aesKey, aesIV)) {
        return FAILURE;
    }

    return SUCCESS;
}


int Crypto::genTestClientKey(int keyLen) {
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);

    if(EVP_PKEY_keygen_init(ctx) <= 0) {
        return FAILURE;
    }

    if(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keyLen) <= 0) {
        return FAILURE;
    }

    if(EVP_PKEY_keygen(ctx, &remotePubKey) <= 0) {
        return FAILURE;
    }

    EVP_PKEY_CTX_free(ctx);

    return SUCCESS;
}
