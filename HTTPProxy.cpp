#include "HTTPProxy.hpp"

#include <string>

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

void HTTPProxy::processConnect(HTTPRequest * request) {
  int clientFd = request->getClientFd();
  int serverFd = -1;
  // connecting to server
  try {
    serverFd = pserver.connectServer(request->getHostnameAndPort());
  }
  catch (myException & e) {
    if(strcmp(e.what() , "connectServer: failed to connect")){
      const char * s = "404 NOT FOUND";
      if (send(request->getClientFd(), s, strlen(s) + 1, 0) == -1) {
        close(clientFd);
        throw myException("connectToServer: 404 sending failed");
      }
      close(clientFd);
      return;
    }
    std::cout << e.what() << std::endl;
    return;
  }
  //sending back and log
  std::string Success200("HTTP/1.1 200 OK\r\n\r\n");
  if (send(clientFd, Success200.c_str(), Success200.length(), 0) == -1) {
    close(clientFd);
    throw myException("sending 200 OK failed");
  }
  std::string Success200Log = std::to_string(request->getId()) + ": " + Success200;
  logger.writeLog(Success200Log);
  //Tunnel
  fd_set fds;
  struct timeval waitTime;
  waitTime.tv_sec = 1;
  waitTime.tv_usec = 0;
  while (true) {
    FD_ZERO(&fds);
    FD_SET(clientFd, &fds);
    FD_SET(serverFd, &fds);
    int fdmax = std::max(clientFd, serverFd) + 1;
    int result = select(fdmax, &fds, NULL, NULL, &waitTime);
    if (result <= 0) {
      break;  // failed or time out
    }
    if (FD_ISSET(clientFd, &fds)) {
      // receive from client
      char buf[MAXDATASIZE];
      memset(buf, 0, sizeof(buf));
      int numbytes = recv(clientFd, buf, MAXDATASIZE, 0);
      if (numbytes <= 0) {
        if (numbytes < 0) {
          // throw myException("receive from client failed");
          break;
        }
        break;
      }
      // send to server
      if (send(serverFd, buf, numbytes, 0) <= 0) {
        // throw myException("send to server failed");
        break;
      }
    }
    if (FD_ISSET(serverFd, &fds)) {
      // receive from server
      char buf[MAXDATASIZE];
      memset(buf, 0, sizeof(buf));
      int numbytes = recv(serverFd, buf, MAXDATASIZE, 0);
      if (numbytes <= 0) {
        if (numbytes < 0) {
          // throw myException("receive from server failed");
          break;
        }
        break;
      }
      // send to client
      if (send(clientFd, buf, numbytes, 0) <= 0) {
        // throw myException("send to server client");
        break;
      }
    }
  }
  close(serverFd);
  close(clientFd);
  std::string closeTunnelLog = std::to_string(request->getId()) + ": tunnel closed\n";
  logger.writeLog(closeTunnelLog);
}

void HTTPProxy::processPost(HTTPRequest * request) {
  std::string connectRequestLog = std::to_string(request->getId()) + ": Requesting \"" +
                                  request->getStartLine() + "\" from " +
                                  request->getUrl() + "\n";
  logger.writeLog(connectRequestLog);
  int serverFd = -1;
    //conenct to server
    int clientFd = request->getClientFd();
    // connecting to server
    try{
        serverFd = pserver.connectServer(request->getHostnameAndPort());
    }
    catch (myException & e) {
    if(strcmp(e.what() , "connectServer: failed to connect")){
      const char * s = "404 NOT FOUND";
      if (send(request->getClientFd(), s, strlen(s) + 1, 0) == -1) {
        close(clientFd);
        throw myException("connectToServer: 404 sending failed");
      }
      close(clientFd);
      return;
    }
    std::cout << e.what() << std::endl;
    return;
  }
    //send message to server
    std::vector<char> vChar= request->to_string();
    char *buf = new char[vChar.size()+1];
    //TODO: check method or: char * buf = vChar.empty() ? 0 : & vChar[0];
	  copy(vChar.begin(),vChar.end(), buf);
    if(send(serverFd, buf, strlen(buf) + 1, 0) == -1){
      delete[] buf;
      close(serverFd);
      close(clientFd);
      throw myException("send request to server failed");
    }
    delete[] buf;
    //then recv response from the server
    HTTPResponse response;
    try{
    response = pserver.recvResponse(serverFd, request->getUrl(), request->getId());
    std::string recvResponseLog = std::to_string(request->getId()) + ": Received \"" + response.getStartLine() + "\" from " + request->getUrl() + "\n";
    logger.writeLog(recvResponseLog);
    }
    catch(myException &e){
      std::cout<< e.what() << std::endl;
    }
    
    //then send back specific message according to response 
    std::string postStatus = response.getStatus();
      if(postStatus == "200"){
        try{
          std::string sendToClientLog = std::to_string(request->getId()) + ": Responding \"" + response.getStartLine()  + "\"\n";
          logger.writeLog(sendToClientLog);
          pserver.sendResponse(clientFd,response);
        }catch(myException &e){
          close(serverFd);
          std::cout<< e.what() << std::endl;
        }
      }
      else if(postStatus[0] == '3'){
          try{
          std::string sendToClientLog = std::to_string(request->getId()) + ": Warning " + response.getStartLine()  + "\n";
          logger.writeLog(sendToClientLog);
          pserver.sendResponse(clientFd,response);
        }catch(myException &e){
          close(serverFd);
          std::cout<< e.what() << std::endl;
        }
      }
      else if(postStatus[0] == '4' || postStatus[0] == '5'){
        try{
          std::string sendToClientLog = std::to_string(request->getId()) + ": Error " + response.getStartLine()  + "\n";
          logger.writeLog(sendToClientLog);
          pserver.sendResponse(clientFd,response);
        }catch(myException &e){
          close(serverFd);
          std::cout<< e.what() << std::endl;
        }
      }

      else{
        try{
          std::string sendToClientLog = std::to_string(request->getId()) + ": Error unkown error \n";
          logger.writeLog(sendToClientLog);
          pserver.sendResponse(clientFd,response);
        }catch(myException &e){
          close(serverFd);
          std::cout<< e.what() << std::endl;
        }
      }    
      close(serverFd);
      close(clientFd);
}
void HTTPProxy::processGet(HTTPRequest * request) {
  //TODO
}

void HTTPProxy::processRequest(HTTPRequest * request) {
  if (request->getMethod() == "CONNECT") {
    try {
      processConnect(request);
    }
    catch (myException & e) {
      std::cout << e.what() << std::endl;
    }
  }
  else if (request->getMethod() == "POST") {
    try {
      processPost(request);
    }
    catch (myException & e) {
      std::cout << e.what() << std::endl;
    }
  }
  else if (request->getMethod() == "GET") {
    try {
      processGet(request);
    }
    catch (myException & e) {
      std::cout << e.what() << std::endl;
    }
  }
  else {
    std::string noMethodLog = ("HTTP/1.1 t found no method exist\r\n\r\n");
    noMethodLog = std::to_string(request->getId()) + ": " + noMethodLog;
    logger.writeLog(noMethodLog);
    //TODO: Response form
    if(send(request->getClientFd(), noMethodLog.c_str(), noMethodLog.length(), 0) == -1){
      throw myException("process request: no method sending failed");
    }

  }
  //TODO check free and close
}
void HTTPProxy::serveConnection(int clientSock, std::string clientIp) {
  try {
    writeLock.lock();
    size_t thisId = requestId++;
    writeLock.unlock();
    HTTPRequest * request = pserver.recvRequest(clientSock, clientIp, thisId);
    //wrtieLog
    writeLock.lock();
    std::string recvMessageLog =
        std::to_string(request->getId()) + ": \"" + request->getStartLine() + "\" from " +
        request->getClientIp() + " @ " + request->getRecvTime() + "\n";
    logger.writeLog(recvMessageLog);
    writeLock.unlock();
    processRequest(request);
    delete request;
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

