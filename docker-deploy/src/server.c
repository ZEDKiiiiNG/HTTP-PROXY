#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "HTTPMessage.hpp"
#define MAXDATASIZE 65535
void sigchld_handler(int s) {
  int saved_errno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
  errno = saved_errno;
}
void * get_in_addr(struct sockaddr * sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
/*
void printIP(struct addrinfo * res) {
  printf("IP addresses:\n\n");
  char ipstr[INET6_ADDRSTRLEN];
  struct addrinfo * p;
  for (p = res; p != NULL; p = p->ai_next) {
    void * addr;
    char * ipver;
    if (p->ai_family == AF_INET) {
      struct sockaddr_in * ipv4 = (struct sockaddr_in *)p->ai_addr;
      addr = &(ipv4->sin_addr);
      ipver = "IPv4";
    }
    else {
      struct sockaddr_in6 * ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &(ipv6->sin6_addr);
      ipver = "IPv6";
    }
    inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
    printf(" %s: %s\n", ipver, ipstr);
  }
}
*/
int main() {
  int status;
  //getlocaladdr, get socket, bind, and listen
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr;
  socklen_t sin_size;
  int sockfd, new_fd;
  int yes = 1;
  char s[INET6_ADDRSTRLEN];
  char buf[MAXDATASIZE];
  struct sigaction sa;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return EXIT_FAILURE;
  }
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }
    break;
  }
  freeaddrinfo(servinfo);
  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }
  if (listen(sockfd, 10) == -1) {
    perror("listen");
    exit(1);
  }
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  printf("server: waiting for connections...\n");
  while (1) {
    sin_size = sizeof(their_addr);
    printf("server: accepting connections...\n");
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }
    printf("server: connetion established\n");
    inet_ntop(
        their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
    printf("server: got connection from %s\n", s);

    if (!fork()) {
      close(sockfd);
      /*if (send(new_fd, "Have you finished http proxy?", 29, 0) == -1) {
        perror("send");
	}*/
      int numbytes;
      if ((numbytes = recv(new_fd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        exit(1);
      }
      buf[numbytes] = '\0';
      printf("Proxy server: recieved HTTP message:\n%s", buf);
      HTTPMessage msg(buf);
      //msg.printMessage();
      //connect to target server and forward
      int fwdSockFd;
      struct addrinfo * tgtServerInfo = msg.getHost();
      for (p = tgtServerInfo; p != NULL; p = p->ai_next) {
        if ((fwdSockFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
          perror("client: socket");
          continue;
        }
        if (connect(fwdSockFd, p->ai_addr, p->ai_addrlen) == -1) {
          close(sockfd);
          perror("client: connect");
          continue;
        }
        break;
      }
      if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
      }
      inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
      printf("proxy: connecting to %s\n", s);
      freeaddrinfo(tgtServerInfo);

      //send to target server transparently
      int len, bytes_sent;
      len = strlen(buf);
      if ((bytes_sent = send(fwdSockFd, buf, len, 0)) == -1) {
        perror("send");
        close(fwdSockFd);
      }
      printf("Proxy: %d bytes sent\n", bytes_sent);
      //recv data from target server
      if ((numbytes = recv(fwdSockFd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        exit(1);
      }
      buf[numbytes] = '\0';
      printf("Proxy server: recieved HTTP message:\n%s", buf);
      close(fwdSockFd);
      //forward data to client
      len = strlen(buf);
      if ((bytes_sent = send(new_fd, buf, len, 0)) == -1) {
        perror("send");
        close(new_fd);
      }
      printf("Proxy: %d bytes sent\n", bytes_sent);
      close(new_fd);
      exit(0);
    }
    close(new_fd);
  }

  return EXIT_SUCCESS;
}
