#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP
#include <iostream>
#include <string>

#include "HTTPMessage.hpp"
class HTTPRequest : public HTTPMessage {
 private:
  //TODO: time recv_time;
  std::string client_ip;
  std::string recv_time;
  int clientFd;

 public:
  HTTPRequest(const char * buffer,
              size_t id,
              std::string client_ip,
              std::string recv_time,
              int clientFd) :
      HTTPMessage(buffer, id), client_ip(client_ip), recv_time(recv_time), clientFd(clientFd) {}
  HTTPRequest(std::vector<char> buffer,
              size_t id,
              std::string client_ip,
              std::string recv_time,
              int clientFd) :
      HTTPMessage(buffer, id), client_ip(client_ip), recv_time(recv_time), clientFd(clientFd) {}
  std::string getClientIp() { return client_ip; }
  //std::string log(std::ostream output) { output << getID() << ":" << }
  std::string getRecvTime() { return recv_time; }
  std::string getUrl() {
    std::pair<std::string, std::string> hostpair = getHostnameAndPort();
    std::string host = hostpair.first + hostpair.second;
    std::string s = start_line;
    size_t r1 = s.find(" ");
    size_t r2 = s.find(" ", s.find(" ") + 1);
    //without hostname
    std::string s1 = s.substr(s.find(" "), r2 - r1);
    return s1;
  }
  std::string getMethod() {
    std::string s = start_line;
    size_t r1 = s.find(" ");
    std::string s1 = s.substr(0, r1);
    return s1;
  }
  int getClientFd() { return clientFd; }
};
#endif
