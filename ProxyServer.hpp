#ifndef PROXYSERVER_HPP
#define PROXYSERVER_HPP
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

#include "HTTPMessage.hpp"
#define MAXDATASIZE 65535
class ProxyServer {
 private:
  int sockfd;
  struct addrinfo * servinfo;
  std::string port_number;

 public:
  ProxyServer(std::string port_number);
  // server destructor
  ~ProxyServer() {
    close(sockfd);
    freeaddrinfo(servinfo);
  }

  // server accepts the client
  int acceptConnection(std::string & client_ip);
  HTTPMessage * recvMessage(int recvSock, std::string clientIp, size_t requestId);
  void sendMessage(int sendSock, HTTPMessage msg);
  int connectServer(struct addrinfo * serverInfo);
};
#endif
