#include "HTTPMessage.hpp"

#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <iostream>
#include <sstream>
HTTPMessage::HTTPMessage() {
}
HTTPMessage::HTTPMessage(std::vector<char> vectorBuf, size_t id) : ID(id) {
  char * buffer = new char[vectorBuf.size()];
  for (size_t i = 0; i < vectorBuf.size(); i++) {
    buffer[i] = vectorBuf[i];
  }
  //TODO: call another constructer
  size_t location = 0;
  std::stringstream raw_stream;
  raw_stream << buffer;
  //parse start line
  std::getline(raw_stream, start_line);
  location += start_line.size();
  location += 1;
  //parse header line
  std::string headerLine;
  std::getline(raw_stream, headerLine);
  std::pair<std::string, std::string> headerPair;
  while (headerLine != "\r") {
    headerLine = headerLine.substr(0, headerLine.find("\r"));
    headerPair = splitHeaderLine(headerLine);
    headers.push_back(headerPair);
    location += headerLine.size();
    location += 2;
    std::getline(raw_stream, headerLine);
  }
  location += 2;
  //parse payload
  std::cout << location << "  " << vectorBuf.size() << std::endl;
  for (size_t i = location; i < vectorBuf.size(); i++) {
    message_body.push_back(vectorBuf[i]);
  }
  delete[] buffer;
}
struct addrinfo * HTTPMessage::getHost() {
  struct addrinfo hints, *hostinfo;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  std::pair<std::string, std::string> host = splitHost(getHeaders()["Host"]);
  //std::cout << host.first << ": " << host.second << std::endl;
  //TBD:Error check
  getaddrinfo(host.first.c_str(), host.second.c_str(), &hints, &hostinfo);
  return hostinfo;
}
std::string HTTPMessage::getStartLine() {
  return start_line;
}
std::map<std::string, std::string> HTTPMessage::getHeaders() {
  std::map<std::string, std::string> headerMap;
  for (size_t i = 0; i < headers.size(); i++) {
    headerMap.insert(headers[i]);
  }
  return headerMap;
}
void HTTPMessage::addHeader(std::pair<std::string, std::string> toAdd) {
  headers.push_back(toAdd);
}
std::vector<char> HTTPMessage::getPayload() {
  return message_body;
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
  std::string header = headerLine.substr(0, headerLine.find(':'));
  std::string content = headerLine.substr(headerLine.find(':') + 2);
  return std::pair<std::string, std::string>(header, content);
}
//this function is used for test and debug
//it simply prints the message into std::cout
void HTTPMessage::printMessage() {
  std::vector<char> msg = to_string();
  for (size_t i = 0; i < msg.size(); i++) {
    std::cout << msg[i];
  }
  std::cout << std::endl;
}
void vectorScp(std::vector<char> & target, const std::string & origin) {
  for (size_t i = 0; i < origin.size(); i++) {
    target.push_back(origin[i]);
  }
}
void vectorScp(std::vector<char> & target, const std::vector<char> & origin) {
  for (size_t i = 0; i < origin.size(); i++) {
    target.push_back(origin[i]);
  }
}
std::vector<char> HTTPMessage::to_string() {
  std::vector<char> result;
  vectorScp(result, start_line);
  result.push_back('\n');
  for (std::vector<std::pair<std::string, std::string> >::const_iterator it =
           headers.begin();
       it != headers.end();
       ++it) {
    vectorScp(result, it->first);
    vectorScp(result, ": ");
    vectorScp(result, it->second);
    vectorScp(result, CRLF);
  }
  vectorScp(result, CRLF);
  vectorScp(result, message_body);
  return result;
}
// directly get host name and port
// TODO HttpMessage format check !!!!!!!!!!!!!!
std::pair<std::string, std::string> HTTPMessage::getHostnameAndPort() {
  std::string host = getHeaders()["Host"];
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
