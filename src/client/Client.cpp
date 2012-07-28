// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include "Client.h"
#include <iostream>

Client::Client(string host, int port) {
   this->host = host;

   // Let setPort do validation on the port range
   setPort(port);

   // Init other member variables
   serverInfo = NULL;
   crypto = NULL;
   mIsConnected = 0;
}


Client::Client() {
   host         = "";
   port         = -1;
   connSock     = -1;
   serverInfo   = NULL;
   crypto       = NULL;
   mIsConnected = 0;
}


Client::~Client() {
   freeaddrinfo(serverInfo);

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
   return SUCCESS;
}


int Client::connectToServer(int aiFamily) {
   // Get server info
   if(getAddressInfo(aiFamily) == FAILURE) {
     return FAILED_TO_GET_ADDR_INFO;
   }

   // Connect to server
   if(connect() == FAILURE) {
      return FAILED_TO_CONNECT;
   }

   // Yay!
   mIsConnected = 1;
   return SUCCESS;
}


int Client::getAddressInfo(int aiFamily) {
	stringstream host, port;
   struct addrinfo hints;
   
   // Convert host and port to strings so they can be passed as a char* below
   host << this->host;
   port << this->port;

   // Make sure hints memory is clear
   memset(&hints, 0, sizeof hints);

   hints.ai_family = aiFamily;      // IPv4 or IPv6?
   hints.ai_socktype = SOCK_STREAM; // Always use sock stream

   // Get the list of results (res)
   return (getaddrinfo(host.str().c_str(), port.str().c_str(), &hints, &serverInfo) == SUCCESS) ? SUCCESS : FAILURE;
   
}


int Client::connect() {
   addrinfo *tmpServerInfo;

   // Traverse list of results and bind to first socket possible
   for(tmpServerInfo = serverInfo; tmpServerInfo != NULL; tmpServerInfo = tmpServerInfo->ai_next) {
      // Try to get socket
      if((connSock = socket(tmpServerInfo->ai_family, tmpServerInfo->ai_socktype, tmpServerInfo->ai_protocol)) == FAILURE) {
         continue;
      }

      // Try to connect to server
      if(::connect(connSock, tmpServerInfo->ai_addr, tmpServerInfo->ai_addrlen) == FAILURE) {
         // Close the opened socket
         close(connSock);      
         continue;
      }

      // No problems above? We're all set up. Move on!
      break;
   }

   // If tmpServerInfo is null, we failed to bind
   if(tmpServerInfo == NULL) {
      connSock = FAILURE;
      return FAILURE;
   } else {
      return SUCCESS;
   }
}


void Client::closeConnection() {
   close(connSock);
   mIsConnected = 0;
}


int Client::send(string data, int useEnc) {
   unsigned char *msg = NULL;
   size_t msgLen;
   int sendStatus;

   // Attempt data encryption if requested
   if(useEnc && crypto != NULL) {
      if((msgLen = crypto->aesEncrypt(data, &msg)) == FAILURE) {
         return FAILURE;
      }
   // Otherwise, just send the data as a c-string
   } else {
      msg = (unsigned char*) data.c_str();
      msgLen = data.length();
   }

   sendStatus = ::send(connSock, msg, msgLen, 0);

   // Encrypted messages are dynaimcally allocated so it needs free'd
   if(useEnc) {
      free(msg);
      msg = NULL;
   }

   return sendStatus;
}


int Client::receive(string *reply, int useEnc) {
   unsigned char *tmpReply = (unsigned char*) malloc(BUFFER);

   int recvLen = recv(connSock, tmpReply, BUFFER, 0);

   // Attempt decryption if using 
   if(useEnc && crypto != NULL) {
      *reply = crypto->aesDecrypt(tmpReply, recvLen);
    
      // If the decryption resulted in an empty string, it failed
      if(reply->length() == 0) {
         return FAILURE;
      }
   // No encryption. Just convert the c-string to a string object
   // Add null terminator only if recvLen is >= 0 to avoid going out of bounds
   } else if(recvLen >= 0) {
      tmpReply[recvLen] = '\0';
      *reply = (char*) tmpReply;
   }

   free(tmpReply);
   tmpReply = NULL;

   return recvLen;
}

  
string Client::getServerIPAddress() const {
	// Make the IP long enough for IPv6 addresses, even though we currently only support IPv4
   char ip[INET6_ADDRSTRLEN];

   inet_ntop(serverInfo->ai_family, &((sockaddr_in*)serverInfo)->sin_addr, ip, sizeof ip);

   return string(ip);
}


int Client::getConnSock() const {
   return connSock;
}


int Client::getPort() const {
	return port;
}


void Client::setPort(int port) {
   // Check that the port is within the valid range
   this->port = (port > 0 && port <= 65535) ? port : -1;
}


void Client::setHost(string host) {
   this->host = host;
}


int Client::isConnected() const {
   return mIsConnected;
}


int Client::sendLocalPubKey() {
   unsigned char *pubKey;

   int pubKeyLen = crypto->getLocalPubKey(&pubKey);

   return ::send(connSock, pubKey, pubKeyLen, 0);
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
   aesKeyLen = crypto->getAESKey(&aesKey);

   // Encrypt the AES with RSA
   encAesKeyLen = crypto->rsaEncrypt(aesKey, aesKeyLen, &encAesKey);

   // Send the encrypted AES key to remote
   sendStatus = ::send(connSock, encAesKey, encAesKeyLen, 0);

   // Encrypted messages are dynamically allocated in the encrypt function so they need free'd
   free(encAesKey);
   encAesKey = NULL;

   return sendStatus;
}


int Client::receiveAESKey() {
   unsigned char *encAesKey = (unsigned char*) malloc(BUFFER);
   unsigned char *aesKey;
   int encAesKeyLen;
   int aesKeyLen;
   int setAesStatus;

   encAesKeyLen = recv(connSock, encAesKey, BUFFER, 0);

   // Validate the reply
   if(encAesKeyLen == 0) {
      return END;
   }

   fprintf(stderr, "\n\nenc AES key: %d\n", (int)encAesKeyLen);
   for(int i=0; i<(int)encAesKeyLen; i++) {
      fprintf(stderr, "%d: %x|\n", i, *(encAesKey+i));
   }

   // Decrypt the AES key
   if(crypto == NULL) return FAILURE;

   if((aesKeyLen = crypto->rsaDecrypt(encAesKey, encAesKeyLen, &aesKey)) == FAILURE) {
      fprintf(stderr, "DECRY FAILED\n");
      return FAILURE;
   }

   fprintf(stderr, "AES KEY: %s\n", aesKey);

   // Set the public key in the crypto object
   setAesStatus = crypto->setAESKey(aesKey, aesKeyLen);

   free(encAesKey);
   free(aesKey);
   encAesKey = aesKey = NULL;

   return setAesStatus;
}