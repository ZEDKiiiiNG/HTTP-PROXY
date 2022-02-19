#include <string>

#include "HTTPMessage.hpp"
#include "common_headers.h"
#define MAXDATASIZE 65535

class proxyServer {
 private:
  int sockfd;
  struct addrinfo * servinfo;
  std::string port_number;

 public:
  proxyServer(std::string port_number);
  // server destructor
  ~proxyServer() {
    close(sockfd);
    freeaddrinfo(servinfo);
  }

  // server accepts the client
  int acceptConnection(std::string & client_ip);
  HTTPMessage* recvMessage(int recvSock, std::string clientIp, size_t requestId);
  void sendMessage(int sendSock, HTTPMessage msg);
  int connectServer(struct addrinfo * serverInfo);
};
