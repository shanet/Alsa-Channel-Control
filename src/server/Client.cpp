// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include "Client.h"

Client::Client(int socket, sockaddr_storage clientInfo, int id, int useEnc) {
   this->socket     = socket;
   this->clientInfo = clientInfo;
   this->id         = id;
   this->useEnc     = useEnc;
   crypto           = NULL;
}


Client::Client() {
   // This constructor should never be called. It's only here for the good 
   // practice of providing a default constructor
   socket = -1;
   id     = -1;
   crypto = NULL;
}


Client::~Client() {
   if(crypto != NULL) {
      delete crypto;
      crypto = NULL;
   }
}


int Client::initCrypto() {
   if(crypto != NULL) {
      return FAILURE;
   }

   crypto = new Crypto();
   useEnc = 1;

   return SUCCESS;
}


int Client::send(string data, int useEnc) {
   unsigned char *msg = NULL;
   size_t msgLen;
   int sendStatus;

   // Attempt data encryption if requested
   if(useEnc && crypto != NULL) {
      if((msgLen = crypto->aesEncrypt((const unsigned char*)data.c_str(), data.length(), &msg)) == (unsigned int)FAILURE) {
         return FAILURE;
      }
   // Otherwise, just send the data as a c-string
   } else {
      msg = (unsigned char*) data.c_str();
      msgLen = data.length();
   }

   sendStatus = ::send(socket, msg, msgLen, 0);

   // Encrypted messages are dynaimcally allocated so it needs free'd
   if(useEnc) {
      free(msg);
      msg = NULL;
   }

   return sendStatus;
}


int Client::receive(string *reply, int useEnc) {
   unsigned char *tmpReply = (unsigned char*)malloc(BUFFER);
   char *decReply;

   int recvLen = recv(socket, tmpReply, BUFFER, 0);

   // Attempt decryption with the given encryption type if asked for
   if(useEnc && crypto != NULL) {
      crypto->aesDecrypt(tmpReply, recvLen, &decReply);
      (*reply) = string(decReply);
    
      // If the decryption resulted in an empty string, it failed
      if(reply->length() == 0) {
         return FAILURE;
      }
   // Add null terminator only if recvLen is >= 0 to avoid going out of bounds
   } else if(recvLen >= 0) {
      tmpReply[recvLen] = '\0';
      *reply = (char*) tmpReply;
   }

   free(tmpReply);
   tmpReply = NULL;

   return recvLen;
}


void Client::close() {
   ::close(socket);
}


string Client::getIPAddress() {
   // Make the IP long enough for IPv6 addresses, even though we currently only support IPv4
   char ip[INET6_ADDRSTRLEN];

   inet_ntop(clientInfo.ss_family, &((sockaddr_in*)&clientInfo)->sin_addr, ip, sizeof ip);

   return string(ip);
}


int Client::getSocket() {
   return socket;
}


sockaddr_storage Client::getClientInfo() {
   return clientInfo;
}


int Client::getID() {
   return id;
}


int Client::isEnc() {
   return useEnc;
}

void Client::setEnc(int enc) {
   useEnc = enc;
}


int Client::sendLocalPubKey() {
   unsigned char *pubKey;

   int pubKeyLen = crypto->getLocalPubKey(&pubKey);

   int sendStatus = ::send(socket, pubKey, pubKeyLen, 0);

   free(pubKey);
   pubKey = NULL;

   return sendStatus;
}


int Client::receiveRemotePubKey() {
   string pubKey;
   int pubKeyLen = receive(&pubKey);

   // Validate the reply
   if(pubKeyLen == FAILURE) {
      return FAILURE;
   } else if(pubKeyLen == 0) {
      return END;
   } else if(pubKey.compare("err\n") == 0) {
      return FAILURE;
   }

   // Set the public key in the crypto object
   return crypto->setRemotePubKey((unsigned char*)pubKey.c_str(), pubKeyLen);
}


int Client::sendAESKey() {
   unsigned char *aesKey;
   unsigned char *encAesKey;
   unsigned char *ek;
   unsigned char *iv;
   size_t aesKeyLen;
   size_t encAesKeyLen;
   size_t ekl;
   size_t ivl;
   int sendStatus;

   // Get the AES key
   aesKeyLen = crypto->getAESKey(&aesKey);

   // Encrypt the AES key with RSA
   encAesKeyLen = crypto->rsaEncrypt(aesKey, aesKeyLen, &encAesKey, &ek, &ekl, &iv, &ivl);

   // Send the encrypted AES key to the client
   sendStatus = ::send(socket, iv, (int)ivl, 0);
   usleep(50000);
   sendStatus = ::send(socket, ek, (int)ekl, 0);
   usleep(50000);
   sendStatus = ::send(socket, encAesKey, (int)encAesKeyLen, 0);

   // Encrypted messages are dynamically allocated in the encrypt function so they need free'd
   free(encAesKey);
   encAesKey = NULL;

   return sendStatus;
}