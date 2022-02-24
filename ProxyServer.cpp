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
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = 0;
  int yes = 1;
  std::cout << "build ProxyServer start" << std::endl;
  // convert hints into servinfo
  if ((status = getaddrinfo(NULL, port_number.c_str(), &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    perror("Proxy: getaddrinfo");
  }
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
  std::cout << "get listen complete" << std::endl;
}
int ProxyServer::acceptConnection(std::string & client_ip) {
  char ip[INET6_ADDRSTRLEN];
  struct sockaddr_storage client_addr;
  socklen_t sin_size = sizeof(client_addr);
  std::cout << "acceptConnection waiting for connections" << std::endl;
  int clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
  if (clientfd == -1) {
    perror("accept failed");
  }
  std::cout << "acceptConnection accpet Connection with " << clientfd << std::endl;
  // struct sockaddr_in * c_addr = (struct sockaddr_in *) &client_addr;
  // char * ip_addr = inet_ntoa(c_addr->sin_addr);
  inet_ntop(client_addr.ss_family,
            get_in_addr((struct sockaddr *)&client_addr),
            ip,
            sizeof(ip));
  client_ip = std::string(ip);
  return clientfd;
}
std::vector<char> ProxyServer::recvMessage(int recvSock, bool loop) {
  std::vector<char> message;
  message.resize(MAXDATASIZE);
  size_t totalSize = 0;
  while (true) {
    std::cout << "RecvMessage waiting for recv..." << std::endl;
    int numbytes = recv(recvSock, &message.data()[totalSize], MAXDATASIZE - 1, 0);
    if (numbytes == -1) {
      throw myException("recv messsage length error!");
    }
    totalSize = totalSize + numbytes;
    if (totalSize != 0 && numbytes == 0) {
      break;
    }
    if (!loop) {
      if (totalSize == 0) {
        throw myException("recvMessage error: the total length is 0");
      }
      break;
    }
  }
  message.resize(totalSize);
  return message;
}
HTTPRequest * ProxyServer::recvRequest(int recvSock,
                                       std::string clientIp,
                                       size_t requestId) {
  std::vector<char> vectorBuf = recvMessage(recvSock, false);
  time_t now = time(0);
  tm * gmtm = gmtime(&now);
  char * dt = asctime(gmtm);
  dt[strlen(dt) - 1] = 0;
  std::string recv_time = std::string(dt);
  HTTPRequest * result =
      new HTTPRequest(vectorBuf, requestId, clientIp, recv_time, recvSock);
  return result;
}

int ProxyServer::connectServer(std::pair<std::string, std::string> host) {
  int status;
  int serverFd = -1;
  struct addrinfo hints, *servinfo, *p;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME;
  std::cout << "connectServer start: " << std::endl;
  if ((status = getaddrinfo(
           host.first.c_str(), host.second.c_str(), &hints, &servinfo)) != 0) {
    //TODO: failed to connect to server, send 404 to client
    freeaddrinfo(servinfo);
    throw myException("connectServer: failed to connect");
    //    const char * s = "404 NOT FOUND";
    //if (send(request->getClientFd(), s, strlen(s) + 1, 0) == -1) {
    //throw myException("connectToServer: 404 sending failed");
  }
  // connect to server
  for (p = servinfo; p != NULL; p = p->ai_next) {
    serverFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (serverFd == -1) {
      //throw myException("connectToServer: get socket failed");
      continue;
    }
    if (connect(serverFd, p->ai_addr, p->ai_addrlen) == -1) {
      // connect failed
      //throw myException("connectToServer: get connect failed");
      if (close(serverFd) == -1) {
        //throw myException("connectToServer: close failed");
      }
      continue;
    }
    break;
  }
  freeaddrinfo(servinfo);
  if (p == NULL) {
    // cannnot connect successfully
    throw myException("connectServer: failed to connect");
  }
  std::cout << "connectServer end with serverFd: " << serverFd << std::endl;
  return serverFd;
}

HTTPResponse ProxyServer::recvResponse(int recvSock, std::string url, size_t requestId) {
  std::vector<char> vectorBuf = recvMessage(recvSock, true);

  HTTPResponse result(vectorBuf, requestId, url);
  return result;
}

HTTPResponse ProxyServer::recvChunk(int recvSock, std::string url, size_t requestId) {
  std::vector<char> vectorBuf = recvMessage(recvSock, false);

  HTTPResponse result(vectorBuf, requestId, url);
  return result;
}
void ProxyServer::sendResponse(int recvSock, HTTPResponse response) {
  std::vector<char> vChar = response.to_string();
  if (send(recvSock, vChar.data(), vChar.size(), 0) == -1) {
    close(recvSock);
    throw myException("send request to server failed");
  }
}
void ProxyServer::sendMessage(int sendSock, HTTPMessage msg) {
  std::vector<char> vChar = msg.to_string();
  if (send(sendSock, vChar.data(), vChar.size(), 0) == -1) {
    close(sendSock);
    throw myException("failed to send message");
  }
}
