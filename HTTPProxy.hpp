#ifndef HTTPPROXY_HPP
#define HTTPPROXY_HPP
#include <mutex>
#include <thread>

#include "HTTPRequest.hpp"
#include "LogWritter.hpp"
#include "ProxyServer.hpp"
#include "common_headers.h"
#include "my_exception.h"
class HTTPProxy {
 public:
  //TODO: Log,Cache
  ProxyServer pserver;
  //size_t id
  size_t requestId;
  logWritter logger;
  std::mutex writeLock;

  HTTPProxy() : pserver("3490") { requestId = 0; }
  HTTPProxy(std::string port) : pserver(port) { requestId = 0; }
  ~HTTPProxy() {}

  void serveConnection(int clientSock, std::string clientIp);
  void processRequest(HTTPRequest * request);
  //HTTPResponse processResponse(HTTPResponse response);
  void startRun();

  //3 types:
  void processConnect(HTTPRequest * request);
  void processPost(HTTPRequest * request);
  void processGet(HTTPRequest * request);
  
};
#endif
