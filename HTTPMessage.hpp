#include <map>
#include <string>
#include <vector>
#include "common_headers.h"
#define CRLF "\r\n"
class HTTPMessage {
 private:
  std::string start_line;
  std::map<std::string, std::string> headers;
  std::vector<char> message_body;
  size_t ID;

 public:
  HTTPMessage();
  HTTPMessage(const char * buffer, size_t id);
  std::string getId();
  std::string getStartLine();
  std::map<std::string, std::string> getHeaders();
  struct addrinfo * getHost();
  size_t getID() { return ID; }
  std::vector<char> to_string();
  void printMessage();
  // protected:
  std::pair<std::string, std::string> splitHost(std::string host);
  std::pair<std::string, std::string> splitHeaderLine(std::string headerLine);
  
};
