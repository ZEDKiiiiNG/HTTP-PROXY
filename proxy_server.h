#ifndef _PROXY_SERVER_H
#define _PROXY_SERVER_H
#include "common_headers.h"
class proxyServer{
  private:
    int sockfd;
    struct addrinfo * servinfo;
    std::string port_number

  void * get_in_addr(struct sockaddr * sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
    }



  public:
    proxyServer(std::string port_number):port_number(port_number) {
        int status;
        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC; //Allow both IPv4 IPv6
        hints.ai_socktype = SOCK_STREAM; //当protocol为0时，会自动选择type类型对应的默认协议。
        hints.ai_flags = AI_PASSIVE; //For wildcard IP address 
        hints.ai_protocol = 0; //FOr any protocal
        int yes = 1;
        
        // convert hints into servinfo
        if ((status = getaddrinfo(NULL, port_number, &hints, &servinfo)) != 0){
            // perror("getaddrinfo failed");
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
            return EXIT_FAILURE;
        }
        //TODO: 循环结构？
        // create socket file descriptor
        if((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
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

        // server destructor
        ~proxyServer() {
            close(sockfd);
            freeaddrinfo(servinfo);
        }

        // server accepts the client
        int proxyServerAccept() {
            char client_ip[INET6_ADDRSTRLEN];
            struct sockaddr_storage client_addr;
            socklen_t sin_size = sizeof(client_addr);
            int clientfd = accept(sockfd, (struct sockaddr *) &client_addr, &sin_size);
            if (clientfd == -1) {
                perror("accept failed");
            }
        // struct sockaddr_in * c_addr = (struct sockaddr_in *) &client_addr;
        // char * ip_addr = inet_ntoa(c_addr->sin_addr);
        inet_ntop(client_addr.ss_family, this->get_in_addr((struct sockaddr *)&client_addr), client_ip, sizeof(client_ip));

        return clientfd;
        }
