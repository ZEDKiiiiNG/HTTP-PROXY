#include <iostream>
#include <string>

#include "HTTPMessage.hpp"
class HTTPRequest : public HTTPMessage {
 private:
  //TODO: time recv_time;
  std::string client_ip;

 public:
  HTTPRequest(const char * buffer, size_t id) : HTTPMessage(buffer, id) {}
  std::string getClientIp() { return client_ip; }
  //std::string log(std::ostream output) { output << getID() << ":" << }
};
