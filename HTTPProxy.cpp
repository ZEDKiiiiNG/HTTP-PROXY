#include "HTTPProxy.hpp"

#include <string>

void HTTPProxy::run() {
  while (1) {
    int clientSock;
    std::string clientIp;
    clientSock = pserver.acceptConnection(clientIp);
    //TODO:Multi-threading
    serveConnection(clientSock);
  }
}

void HTTPProxy::serveConnection(int clientSock) {
  /*
  HTTPMessage request = pServer.recvMessage(clientSock);
  
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
