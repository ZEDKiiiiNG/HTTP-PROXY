#include "ProxyServer.hpp"

#include <arpa/inet.h>

#include <cstring>
#include <string>

#include "HTTPRequest.hpp"
#include "my_exception.h"
void * get_in_addr(struct sockaddr * sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
ProxyServer::ProxyServer(std::string port_number) : port_number(port_number) {
  int status;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;  //Allow both IPv4 IPv6
  hints.ai_socktype = SOCK_STREAM;  //当protocol为0时，会自动选择type类型对应的默认协议。
  hints.ai_flags = AI_PASSIVE;  //For wildcard IP address
  hints.ai_protocol = 0;        //FOr any protocal
  int yes = 1;

  // convert hints into servinfo
  if ((status = getaddrinfo(NULL, port_number.c_str(), &hints, &servinfo)) != 0) {
    // perror("getaddrinfo failed");
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    perror("Proxy: getaddrinfo");
  }
  //TODO: 循环结构？
  // create socket file descriptor
  if ((sockfd = socket(
           servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
    perror("server: socket");
  }
  // keep using the port
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt failed");
  }
  // bind the socket file descriptor
  if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
    perror("bind failed");
  }
  // listening
  if (listen(sockfd, 100) == -1) {
    perror("listen failed");
  }
}
int ProxyServer::acceptConnection(std::string & client_ip) {
  char ip[INET6_ADDRSTRLEN];
  struct sockaddr_storage client_addr;
  socklen_t sin_size = sizeof(client_addr);
  int clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
  if (clientfd == -1) {
    perror("accept failed");
  }
  // struct sockaddr_in * c_addr = (struct sockaddr_in *) &client_addr;
  // char * ip_addr = inet_ntoa(c_addr->sin_addr);
  inet_ntop(client_addr.ss_family,
            get_in_addr((struct sockaddr *)&client_addr),
            ip,
            sizeof(ip));
  client_ip = std::string(ip);
  return clientfd;
}
std::vector<char> ProxyServer::recvMessage(int recvSock) {
  int numbytes;
  char buf[MAXDATASIZE];
  if ((numbytes = recv(recvSock, buf, MAXDATASIZE - 1, 0)) == -1) {
    throw myException("recv messsage length error!");
  }
  buf[numbytes] = '\0';
  printf("Proxy server: recieved HTTP message:\n%s", buf);
  std::vector<char> vectorBuf(buf, buf + numbytes);
  return vectorBuf;
}
HTTPRequest * ProxyServer::recvRequest(int recvSock,
                                       std::string clientIp,
                                       size_t requestId) {
  std::vector<char> vectorBuf = recvMessage(recvSock);

  // HTTPMessage msg(buf);
  //get the recv time
  // 基于当前系统的当前日期/时间
  time_t now = time(0);
  // 把 now 转换为 tm 结构
  tm * gmtm = gmtime(&now);
  char * dt = asctime(gmtm);
  std::string recv_time = std::string(dt);
  HTTPRequest * result = new HTTPRequest(vectorBuf, requestId, clientIp, recv_time);
  return result;
}

int ProxyServer::connectServer(std::pair<std::string, std::string> host) {
  int status, serverFd;
  struct addrinfo hints, *servinfo, *p;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME;  //AI_PASSIVE 被动的，用于bind，通常用于server socket

  if ((status = getaddrinfo(
           host.first.c_str(), host.second.c_str(), &hints, &servinfo)) != 0) {
    //TODO: failed to connect to server, send 404 to client
    throw myException("connectServer: failed to connect");
    //    const char * s = "404 NOT FOUND";
    //if (send(request->getClientFd(), s, strlen(s) + 1, 0) == -1) {
    //throw myException("connectToServer: 404 sending failed");
  }
  // connect to server
  for (p = servinfo; p != NULL; p = p->ai_next) {
    serverFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (serverFd == -1) {
      throw myException("connectToServer: get socket failed");
      continue;
    }
    if (connect(serverFd, p->ai_addr, p->ai_addrlen) == -1) {
      // connect failed
      throw myException("connectToServer: get connect failed");
      if (close(serverFd) == -1) {
        throw myException("connectToServer: close failed");
      }
      continue;
    }
    break;
  }
  freeaddrinfo(servinfo);
  if (p == NULL) {
    // cannnot bind successfully
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }
  return serverFd;
}
