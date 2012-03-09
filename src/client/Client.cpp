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
}


Client::~Client() {
   freeaddrinfo(serverInfo);
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
}


int Client::send(string data) {
   return ::send(connSock, data.c_str(), data.length(), 0);
}


int Client::receive(string *reply) {
   char *tmpReply = new char[BUFFER];

   int recvLen = recv(connSock, tmpReply, BUFFER-1, 0);

   // Add null terminator only if recvLen is >= 0 to avoid going out of bounds
   if(recvLen >= 0) {
      tmpReply[recvLen] = '\0';
      *reply = tmpReply;
   }

   delete tmpReply;
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