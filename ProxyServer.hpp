#ifndef PROXYSERVER_HPP
#define PROXYSERVER_HPP
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

#include "HTTPMessage.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#define MAXDATASIZE 65535
class ProxyServer {
 private:
  int sockfd;
  struct addrinfo * servinfo;
  std::string port_number;

 protected:
  std::vector<char> recvMessage(int recvSock);

 public:
  ProxyServer(std::string port_number);
  // server destructor
  ~ProxyServer() {
    close(sockfd);
    freeaddrinfo(servinfo);
  }

  // server accepts the client
  int acceptConnection(std::string & client_ip);
  HTTPRequest * recvRequest(int recvSock, std::string clientIp, size_t requestId);
  HTTPResponse * recvResponse(int recvSock);
  void sendMessage(int sendSock, HTTPMessage msg);
  int connectServer(std::pair<std::string, std::string> host);
};
#endif
