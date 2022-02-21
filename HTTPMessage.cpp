#include "HTTPMessage.hpp"

#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <iostream>
#include <sstream>
HTTPMessage::HTTPMessage() {
}
HTTPMessage::HTTPMessage(const char * buffer, size_t id) : ID(id) {
  //cast char * into stringStream for further parsing
  std::stringstream raw_stream;
  raw_stream << buffer;
  std::getline(raw_stream, start_line);
  std::string headerLine;
  std::getline(raw_stream, headerLine);
  while (headerLine != "\r") {
    headers.insert(splitHeaderLine(headerLine));
    std::getline(raw_stream, headerLine);
  }
}
HTTPMessage::HTTPMessage(std::vector<char> vectorBuf, size_t id) : ID(id) {
  char * buffer = new char[vectorBuf.size()];
  for (size_t i = 0; i < vectorBuf.size(); i++) {
    buffer[i] = vectorBuf[i];
  }
  //TODO: call another constructer
  std::stringstream raw_stream;
  raw_stream << buffer;
  std::getline(raw_stream, start_line);
  std::string headerLine;
  std::getline(raw_stream, headerLine);
  while (headerLine != "\r") {
    headers.insert(splitHeaderLine(headerLine));
    std::getline(raw_stream, headerLine);
  }
}
struct addrinfo * HTTPMessage::getHost() {
  struct addrinfo hints, *hostinfo;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  std::pair<std::string, std::string> host = splitHost(headers["Host"]);
  //std::cout << host.first << ": " << host.second << std::endl;
  //TBD:Error check
  getaddrinfo(host.first.c_str(), host.second.c_str(), &hints, &hostinfo);
  return hostinfo;
}
std::string HTTPMessage::getStartLine() {
  return start_line;
}
std::map<std::string, std::string> HTTPMessage::getHeaders() {
  return headers;
}
//this function is used to split host into hostname and hostport
//E.g. "vcm-23974.vm.duke.edu:8000" ----> ("vcm-23974.vm.duke.edu", "8000")
//"www.google.com" ----> ("www.google.com", "80")
std::pair<std::string, std::string> HTTPMessage::splitHost(std::string host) {
  std::string hostName, hostPort;
  size_t colonPos = host.find(':');
  if (colonPos != std::string::npos) {
    hostName = host.substr(0, host.find(':'));
    hostPort = host.substr(host.find(':') + 1);
  }
  else {
    hostName = host;
    hostPort = "80";
  }
  return std::pair<std::string, std::string>(hostName, hostPort);
}
//this function is used to split headerline into header and content
//E.g. "Host: vcm-23974.vm.duke.edu:8000" ----> ("Host", "vcm-23974.vm.duke.edu:8000")
std::pair<std::string, std::string> HTTPMessage::splitHeaderLine(std::string headerLine) {
  headerLine = headerLine.substr(0, headerLine.find('\r'));
  std::string header = headerLine.substr(0, headerLine.find(':'));
  std::string content = headerLine.substr(headerLine.find(':') + 2);
  return std::pair<std::string, std::string>(header, content);
}
//this function is used for test and debug
//it simply prints the message into std::cout
void HTTPMessage::printMessage() {
  std::cout << "Printing HTTP Message..." << std::endl;
  std::cout << start_line << std::endl;
  for (std::map<std::string, std::string>::const_iterator it = headers.begin();
       it != headers.end();
       ++it) {
    std::cout << it->first << ": " << it->second << std::endl;
  }
  std::cout << std::endl;
  std::cout << "End of the HTTP Message" << std::endl;
}
// directly get host name and port
// TODO HttpMessage format check !!!!!!!!!!!!!!
std::pair<std::string, std::string> HTTPMessage::getHostnameAndPort() {
  std::string host = headers["Host"];
  std::string hostName, hostPort;
  size_t colonPos = host.find(':');
  if (colonPos != std::string::npos) {
    hostName = host.substr(0, host.find(':'));
    hostPort = host.substr(host.find(':') + 1);
  }
  else {
    hostName = host;
    hostPort = "80";
  }
  return std::pair<std::string, std::string>(hostName, hostPort);
}
