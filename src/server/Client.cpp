// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include "Client.h"

Client::Client(int socket, sockaddr_storage clientInfo, int id) {
   this->socket     = socket;
   this->clientInfo = clientInfo;
   this->id         = id;
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


int Client::send(string data, int useEnc) {
   unsigned char *msg = NULL;
   size_t msgLen;
   int sendStatus;

   // If using encryption, encrypt the data to send first with the specified encryption type
   if(useEnc && crypto != NULL) {
      if((msgLen = crypto->aesEncrypt(data, &msg)) == FAILURE) {
         return FAILURE;
      }
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
   unsigned char *tmpReply = (unsigned char*) malloc(BUFFER);

   int recvLen = recv(socket, tmpReply, BUFFER, 0);

   // Attempt decryption with the given encryption type if asked for
   if(useEnc && crypto != NULL) {
      *reply = crypto->aesDecrypt(tmpReply, recvLen);
    
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


int Client::initCrypto() {
   if(crypto != NULL) {
      return FAILURE;
   }

   crypto = new Crypto();
   return SUCCESS;
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


int Client::sendLocalPubKey() {
   unsigned char *pubKey;

   int pubKeyLen = crypto->getLocalPubKey(&pubKey);

   printf("Sending pubkey: %s\n", (char*)pubKey);

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
   }

   // Set the public key in the crypto object
   return crypto->setRemotePubKey((unsigned char*)pubKey.c_str(), pubKeyLen);
}


int Client::sendAESKey() {
   unsigned char *aesKey;
   unsigned char *encAesKey;
   size_t aesKeyLen;
   size_t encAesKeyLen;
   int sendStatus;

   // Get the AES key
   aesKeyLen = crypto->getLocalAESKey(&aesKey);

   // Encrypt the AES with RSA
   encAesKeyLen = crypto->rsaEncrypt(aesKey, aesKeyLen, &encAesKey);

   // Send the encrypted AES key to remote
   sendStatus = ::send(socket, encAesKey, encAesKeyLen, 0);

   // Encrypted messages are dynamically allocated in the encrypt function so they need free'd
   free(encAesKey);
   encAesKey = NULL;

   return sendStatus;
}


int Client::receiveAESKey() {
   string aesKey;
   int aesKeyLen = receive(&aesKey);

   // Validate the reply
   if(aesKeyLen == FAILURE) {
      return FAILURE;
   } else if(aesKeyLen == 0) {
      return END;
   }

   // Set the public key in the crypto object
   return crypto->setRemotePubKey((unsigned char*)aesKey.c_str(), aesKeyLen);
}