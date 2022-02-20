#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP
#include <string>

#include "HTTPMessage.hpp"

class HTTPResponse : public HTTPMessage {
 private:
  std::string server_name;
  
 public:
  HTTPResponse(const char * buffer, size_t id) : HTTPMessage(buffer, id) {}
  std::string getServerName() { return server_name; }
};
#endif
