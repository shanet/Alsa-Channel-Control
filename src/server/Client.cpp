// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include "Client.h"

Client::Client(int socket, sockaddr_storage clientInfo, int id) {
   this->socket = socket;
   this->clientInfo = clientInfo;
   this->id = id;
}


Client::Client() {
	socket = -1;
}


Client::~Client() {}


int Client::send(string data) {
   return ::send(socket, data.c_str(), data.length(), 0);
}


int Client::receive(string *reply) {
   char *tmpReply = new char[BUFFER];

   int recvLen = recv(socket, tmpReply, BUFFER, 0);

   // Add null terminator only if recvLen is >= 0 to avoid going out of bounds
   if(recvLen >= 0) {
      tmpReply[recvLen] = '\0';
      *reply = tmpReply;
   }

   delete tmpReply;

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