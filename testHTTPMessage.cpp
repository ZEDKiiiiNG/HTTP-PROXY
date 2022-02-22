#include <assert.h>

#include <iostream>

#include "HTTPMessage.hpp"
char * vec2array(std::vector<char> vectorBuffer) {
  size_t length = vectorBuffer.size();
  char * array = new char[length];
  for (size_t i = 0; i < length; i++) {
    array[i] = vectorBuffer[i];
  }
  return array;
}
void testSplitHost() {
  std::string host;
  HTTPMessage msg;
  std::pair<std::string, std::string> hostPair;

  host = "www.google.com:80";
  hostPair = msg.splitHost(host);

  assert(hostPair.first == "www.google.com" && hostPair.second == "80");
  host = "vcm-23973.vm.duke.edu:8000";
  hostPair = msg.splitHost(host);
  assert(hostPair.first == "vcm-23973.vm.duke.edu" && hostPair.second == "8000");

  host = "vcm-23974.vm.duke.edu:3490";
  hostPair = msg.splitHost(host);
  assert(hostPair.first == "vcm-23974.vm.duke.edu" && hostPair.second == "3490");
}
void testSplitHeaderLine() {
  std::string headerLine;
  HTTPMessage msg;
  std::pair<std::string, std::string> headerPair;

  headerLine = "Host: www.google.com:80";
  headerPair = msg.splitHeaderLine(headerLine);
  assert(headerPair.first == "Host" && headerPair.second == "www.google.com:80");

  headerLine = "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:97.0) "
               "Gecko/20100101 Firefox/97.0";
  headerPair = msg.splitHeaderLine(headerLine);
  assert(headerPair.first == "User-Agent" &&
         headerPair.second == "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:97.0) "
                              "Gecko/20100101 Firefox/97.0");

  headerLine = "Accept-Language: en-US,en;q=0.5";
  headerPair = msg.splitHeaderLine(headerLine);
  assert(headerPair.first == "Accept-Language" && headerPair.second == "en-US,en;q=0.5");
}
void testHTTPMessageConstructor() {
  std::string raw_message;
  raw_message = "GET http://vcm-23973.vm.duke.edu:8000/ HTTP/1.1"
                "\n"
                "Host: vcm-23973.vm.duke.edu:8000" CRLF CRLF "Hello, World!";
  HTTPMessage msg(raw_message.c_str(), 1);
  assert(msg.getStartLine() == "GET http://vcm-23973.vm.duke.edu:8000/ HTTP/1.1");
  assert(msg.getHeaders()["Host"] == "vcm-23973.vm.duke.edu:8000");
  assert(std::string(vec2array(msg.getPayload())) == "Hello, World!");
  msg.printMessage();
}
void testToString() {
  std::string raw_message;
  raw_message = "GET http://vcm-23973.vm.duke.edu:8000/ HTTP/1.1"
                "\n"
                "Host: vcm-23973.vm.duke.edu:8000" CRLF CRLF "Hello, World!"
                "\n"
                "I am Xianming!";
  HTTPMessage msg(raw_message.c_str(), 1);
  assert(std::string(vec2array(msg.to_string())) == raw_message);
}
int main() {
  testSplitHeaderLine();
  testSplitHost();
  testHTTPMessageConstructor();
  testToString();
  return EXIT_SUCCESS;
}
