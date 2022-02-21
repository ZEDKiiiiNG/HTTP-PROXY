#ifndef HTTPMESSAGE_HPP
#define HTTPMESSAGE_HPP
#include <map>
#include <string>
#include <vector>
#define CRLF "\r\n"
#define MAXDATASIZE 65535

class HTTPMessage {
 protected:
  std::string start_line;
  std::map<std::string, std::string> headers;
  std::vector<char> message_body;
  size_t ID;

 public:
  HTTPMessage();
  HTTPMessage(const char * buffer, size_t id);
  HTTPMessage(std::vector<char>, size_t id);
  // std::string getId();
  std::string getStartLine();
  std::map<std::string, std::string> getHeaders();
  struct addrinfo * getHost();
  size_t getId() { return ID; }
  std::vector<char> to_string();
  void printMessage();
  // protected:
  std::pair<std::string, std::string> splitHost(std::string host);
  std::pair<std::string, std::string> splitHeaderLine(std::string headerLine);
  // get hostname and port in string:
  std::pair<std::string, std::string> getHostnameAndPort();
};
#endif
