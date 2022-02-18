#include "common_headers.h"
#include "my_exception.h"
#include "proxyServer.hpp"
class HTTPProxy {
 public:
  //TODO: Log,Cache
  proxyServer pserver;
  int RequestID;
  void setUp();

  HTTPProxy() : pserver("3490") { RequestID = 0; }
  ~HTTPProxy() {}

  void serveConnection(int clientSock);
  //HTTPRequest processRequest(HTTPRequest request);
  //HTTPResponse processResponse(HTTPResponse response);
  void run();
};
