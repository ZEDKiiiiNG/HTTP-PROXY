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
  int clientfd;

 public:
  HTTPRequest(const char * buffer,
              size_t id,
              std::string client_ip,
              std::string recv_time) :
      HTTPMessage(buffer, id), client_ip(client_ip), recv_time(recv_time) {}
  std::string getClientIp() { return client_ip; }
  //std::string log(std::ostream output) { output << getID() << ":" << }
  std::string getRecvTime() { return recv_time; }
};
#endif
