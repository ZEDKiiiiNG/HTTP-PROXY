#include "HTTPRequest.hpp"
#include "assert.h"
void testGetUrl() {
  // std::string raw_message;
  // raw_message = "GET http://vcm-23973.vm.duke.edu:8000/ HTTP/1.1" CRLF
  //               "Host: vcm-23973.vm.duke.edu:8000" CRLF;
  // HTTPRequest req(raw_message.c_str(), 101, "127.0.0.1", "02-20-2022");
  // assert(req.getUrl() == "http://vcm-23973.vm.duke.edu:8000/");
}
void testGetMethod() {
  // std::string raw_message;
  // raw_message = "GET http://vcm-23973.vm.duke.edu:8000/ HTTP/1.1" CRLF
  //               "Host: vcm-23973.vm.duke.edu:8000" CRLF;
  // HTTPRequest req(raw_message.c_str(), 101, "127.0.0.1", "02-20-2022");
  // assert(req.getMethod() == "Get");
}
int main() {
  testGetUrl();
  testGetMethod();
  return EXIT_SUCCESS;
}
