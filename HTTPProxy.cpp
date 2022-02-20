#include "HTTPProxy.hpp"

#include <string>

void HTTPProxy::run() {
  while (1) {
    int clientSock;
    std::string clientIp;
    clientSock = pserver.acceptConnection(clientIp);
    //TODO:Multi-threading
    // serveConnection(clientSock,clientIp);
    std::thread t(&HTTPProxy::serveConnection, this, clientSock , clientIp);
    t.detach();
  }
}
// conenct to server in requset
int connectToServer(HTTPRequest * request){
  int status, serverFd;
  struct addrinfo hints, *servinfo, *p;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME;//AI_PASSIVE 被动的，用于bind，通常用于server socket
  // host and port:
  // TODO : getHost in HTTPMessage
  std::pair<std::string, std::string> host = request->getHostnameAndPort();
  //std::cout << host.first << ": " << host.second << std::endl;

  if(status = getaddrinfo(host.first.c_str(), host.second.c_str(), &hints, &servinfo) != 0){
    const char* s = "404 NOT FOUND";
    if(send(request->getClientFd(), s, strlen(s) + 1, 0) == -1){
      throw myException("connectToServer: 404 sending failed");
    }
    return -1;
  }
  // connect to server
  for(p = servinfo; p != NULL; p = p->ai_next){
    serverFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (serverFd == -1) {
        throw myException("connectToServer: get socket failed");
        continue;
    }
    if (connect(serverFd, p->ai_addr, p->ai_addrlen) == -1) {
        // connect failed
        throw myException("connectToServer: get connect failed");
        if (close(serverFd) == -1) {
            throw myException("connectToServer: close failed");
        }
        continue;
    }
    break; 
  }
  freeaddrinfo(servinfo);
  if (p == NULL) {
    // cannnot bind successfully 
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }
  return serverFd;
}

void processConnect(HTTPRequest * request){
  int clientFd = request->getClientFd();
  // connecting to server
  try{
      serverFd = connectToServer(request);
  }
  catch(myException &e){
      std::cout<< e.what() <<std::endl;
  }
  //sending back and log
  std::string Success200("HTTP/1.1 200 OK\r\n\r\n");
    if (send(clientFd, Success200.c_str(), Success200.length(), 0) == -1) {
        throw myException("sending 200 OK failed");
    }
    std::string Success200Log = std::to_string(request->getId()) + ": " + Success200;
    logger.writeLog(Success200Log);
  //Tunnel
  fd_set fds;
  struct timeval waitTime;
  waitTime.tv_sec = 1;
  waitTime.tv_usec = 0; 
  while (true){
    FD_ZERO(&fds);
    FD_SET(clientFd, &fds);
    FD_SET(serverFd, &fds);
    int fdmax = max(clientFd, serverFd) + 1;
    int result = select(fdmax, &fds, NULL, NULL, &waitTime);
    if (result <= 0) {
        break;// failed or time out 
    }
    if (FD_ISSET(clientFd, &fds)){
      // receive from client
      char buf[MAXDATASIZE];
      memset(buf, 0, sizeof(buf));
      int numbytes = recv(clientFd, buf, MAXDATASIZE, 0);
      if (numbytes <= 0) {
        if( numbytes < 0){
          thorw myException("receive from client failed");
          break;
        }
        break;
      }
      // send to server
      if (send(serverFd, buf, numbytes, 0) <= 0) {
        thorw myException("send to server failed");
        break;
      }
    }
    if (FD_ISSET(serverFd, &fds)){
      // receive from server
      char buf[MAXDATASIZE];
      memset(buf, 0, sizeof(buf));
      int numbytes = recv(serverFd, buf, MAXDATASIZE, 0);
      if (numbytes <= 0) {
        if( numbytes < 0){
          thorw myException("receive from server failed");
          break;
        }
        break;
      }
      // send to client
      if (send(clientFd, buf, numbytes, 0) <= 0) {
        thorw myException("send to server client");
        break;
      }
    }
  }
  close(serverFd);
  close(clientFd);
  std::string closeTunnelLog = std::to_string(request->getId()) + ": tunnel closed\n";
  logger.writeLog(closeTunnelLog);
  
}

  void processPost(HTTPRequest * request){
    std::string connectRequestLog = std::to_string(request->getId()) + ": Requesting \"" + request->getStartLine() + "\" from " + request->getUrl() + "\n";
    logger.writeLog(connectRequestLog);
  }
  void processGet(HTTPRequest * request);

void processRequest(HTTPRequest* request){
  if(request->getMethod() == "CONNECT"){
    try{
        processConnect(request);
      }
      catch(myException &e){
        std::cout<< e.what() << std::endl;
      }
  }else if(request->getMethod() == "POST"){
    try{
        processPOST(request);
      }
      catch(myException &e){
        std::cout<< e.what() << std::endl;
      }
  }else if(request->getMethod() == "GET"){
    try{
        processGet(request);
      }
      catch(myException &e){
        std::cout<< e.what() << std::endl;
      }
  }else{
    std::string noMethodLog = ("HTTP/1.1 t found no method exist\r\n\r\n");
    std::string noMethodLog = std::to_string(request->getId()) + ": " + noMethodLog;
    logger.writeLog(noMethodLog);
    //TODO: Response form 
    send(request->clientFd, noMethodLog.c_str(), noMethodLog.length(), 0);
  }
}
void HTTPProxy::serveConnection(int clientSock, std::string clientIp) {
  try{
    writeLock.lock();
    size_t thisId = requestId++;
    writeLock.unlock();
    HTTPMessage* request = pServer.recvMessage(clientSock,clientIp,thisId);
    //wrtieLog
    writeLock.lock();
    std::string recvMessageLog = std::to_string(request->getId()) + ": \"" + request->getStartLine() + "\" from " + request->getClientIp() + " @ " + request->getRecvTime() + "\n";
    logger.writeLog(recvMessageLog);
    writeLock.unlock();
  }
  catch(myException &e){
    std::cout<< e.what() << std::endl;
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
