#include "common_headers.h"
#include "my_exception.h"
#include "proxyServer.hpp"
#include "LogWritter.hpp"
class HTTPProxy {
 public:
  //TODO: Log,Cache
  proxyServer pserver;
  //size_t id 
  size_t requestId;
  logWritter logger;
  std::mutex writeLock;
  void setUp();


  HTTPProxy() : pserver("3490") { requestId = 0; }
  ~HTTPProxy() {}

  void serveConnection(int clientSock, std::string clientIp);
  void processRequest(HTTPRequest* request);
  //HTTPResponse processResponse(HTTPResponse response);
  void run();
};
