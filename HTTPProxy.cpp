#include "HTTPProxy.hpp"

#include <string>
#include <thread>

void HTTPProxy::run() {
  while (1) {
    int clientSock;
    std::string clientIp;
    clientSock = pserver.acceptConnection(clientIp);
    //TODO:Multi-threading
    // serveConnection(clientSock,clientIp);
    std::thread t(&HTTPProxy::serveConnection, this, clientSock, clientIp);
    t.detach();
  }
}
void processRequest(HTTPRequest * request) {
  //if (request->getMethod() == "POST") {
}
void HTTPProxy::serveConnection(int clientSock, std::string clientIp) {
  try {
    //HTTPMessage * request = pserver.recvMessage(clientSock, clientIp, requestId);
    //wrtieLog
    writeLock.lock();
    requestId++;
    /*
    std::string recvMessageLog =
        std::to_string(request->getId()) + ": \"" + request->getStartLine() + "\" from " +
        request->getClientIp() + " @ " + request->getRecvTime() + "\n";
    logger.writeLog(recvMessageLog);
    */
    writeLock.unlock();
  }
  catch (myException & e) {
    std::cout << e.what() << std::endl;
  }
  /*
  struct addrinfo * serverInfo = request.getHost();
    request = processRequest(request);
  int serverSock = pServer.connectServer(serverInfo);
  pServer.sendMessage(serverSock, request);
  HTTPMessage response = pServer.recvMessage(serverSock);
  response = processResponse(response);
  pServer.sendMessage(clientSock, response);
server.closeSock()
  */
}
