#include "proxyServer.hpp"

void * get_in_addr(struct sockaddr * sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
proxyServer::proxyServer(std::string port_number) : port_number(port_number) {
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
int proxyServer::acceptConnection(std::string & client_ip) {
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
  client_ip = ip;
  return clientfd;
}
