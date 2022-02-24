#ifndef HTTPPROXY_HPP
#define HTTPPROXY_HPP
#include <mutex>
#include <thread>

#include "HTTPCache.hpp"
#include "HTTPRequest.hpp"
#include "LogWritter.hpp"
#include "ProxyServer.hpp"
#include "common_headers.h"
#include "my_exception.h"
class HTTPProxy {
 private:
  HTTPCache cache;

 public:
  ProxyServer pserver;
  //size_t id
  size_t requestId;
  logWritter logger;
  std::mutex writeLock;

  HTTPProxy() : cache(50), pserver("3490") { requestId = 0; }
  HTTPProxy(std::string port) : cache(50), pserver(port) { requestId = 0; }
  ~HTTPProxy() {}

  void serveConnection(int clientSock, std::string clientIp);
  void processRequest(HTTPRequest * request);
  //HTTPResponse processResponse(HTTPResponse response);
  void startRun();

  //3 types:
  void processConnect(HTTPRequest * request);
  void processPost(HTTPRequest * request);
  void processGet(HTTPRequest * request);

  bool validate(HTTPResponse response, HTTPRequest, int serverFd);
  bool checkTime(HTTPResponse res);
  HTTPResponse askServer(HTTPRequest request);
  void fwdChunks(int serverFd, int clientFd);
  std::time_t timeTransfer(std::string time);
};
#endif
