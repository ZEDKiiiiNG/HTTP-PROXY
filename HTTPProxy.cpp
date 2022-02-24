#include "HTTPProxy.hpp"

#include <string>

void HTTPProxy::startRun() {
  while (1) {
    int clientSock;
    std::string clientIp;
    std::cout << "start service" << std::endl;
    clientSock = pserver.acceptConnection(clientIp);
    //TODO:Multi-threading
    // serveConnection(clientSock,clientIp);
    std::cout << "accept successful" << std::endl;
    std::thread t(&HTTPProxy::serveConnection, this, clientSock, clientIp);
    t.detach();
    //single thread:
    //HTTPProxy::serveConnection(clientSock, clientIp);
  }
}

void HTTPProxy::processConnect(HTTPRequest * request) {
  int clientFd = request->getClientFd();
  int serverFd = -1;
  // connecting to server
  std::cout << "process Connect start" << std::endl;
  try {
    serverFd = pserver.connectServer(request->getHostnameAndPort());
  }
  catch (myException & e) {
    std::cout << "process Connect Exception 1" << std::endl;
    if (strcmp(e.what(), "connectServer: failed to connect")) {
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
  std::cout << "process Connect serverFd" << serverFd << std::endl;
  //sending back and log
  std::string Success200("HTTP/1.1 200 OK\r\n\r\n");
  if (send(clientFd, Success200.c_str(), Success200.length(), 0) == -1) {
    close(clientFd);
    throw myException("sending 200 OK failed");
  }
  std::cout << "process Connect with serverFd " << serverFd << "and clientFd " << clientFd
            << " 200 OK send success" << std::endl;
  //sending back and log
  std::string Success200Log = std::to_string(request->getId()) + ": " + Success200;
  writeLock.lock();
  logger.writeLog(Success200Log);
  writeLock.unlock();
  //Tunnel
  fd_set fds;
  struct timeval waitTime;
  waitTime.tv_sec = 1;
  waitTime.tv_usec = 0;
  while (true) {
    std::vector<char> message(MAXDATASIZE);
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
      // char buf[MAXDATASIZE];
      // memset(buf, 0, sizeof(buf));
      int numbytes = recv(clientFd, &message.data()[0], MAXDATASIZE, 0);
      if (numbytes <= 0) {
        if (numbytes < 0) {
          throw myException("receive from client failed");
          // break;
        }
        break;
      }
      // send to server
      if (send(serverFd, message.data(), numbytes, 0) < 0) {
        throw myException("send to server failed");
        // break;
      }
    }
    if (FD_ISSET(serverFd, &fds)) {
      // receive from server
      // char buf[MAXDATASIZE];
      // memset(buf, 0, sizeof(buf));
      int numbytes = recv(serverFd, &message.data()[0], MAXDATASIZE, 0);
      if (numbytes <= 0) {
        if (numbytes < 0) {
          throw myException("receive from server failed");
          // break;
        }
        break;
      }
      // send to client
      if (send(clientFd, message.data(), numbytes, 0) < 0) {
        throw myException("send to server client");
        // break;
      }
    }
  }
  std::cout << "process Connect with serverFd" << serverFd << "Tunnel complete"
            << std::endl;
  close(serverFd);
  close(clientFd);
  std::string closeTunnelLog = std::to_string(request->getId()) + ": tunnel closed\n";
  writeLock.lock();
  logger.writeLog(closeTunnelLog);
  writeLock.unlock();
  std::cout << "process Connect with serverFd" << serverFd << "Tunnel closed"
            << std::endl;
}

void HTTPProxy::processPost(HTTPRequest * request) {
  int serverFd = -1;
  //conenct to server
  int clientFd = request->getClientFd();
  // connecting to server
  try {
    serverFd = pserver.connectServer(request->getHostnameAndPort());
  }
  catch (myException & e) {
    if (strcmp(e.what(), "connectServer: failed to connect")) {
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
  std::cout << request->getId() << ": processPost connectServer complete" << std::endl;
  //send message to server
  std::vector<char> vChar = request->to_string();
  if (send(serverFd, vChar.data(), vChar.size(), 0) == -1) {
    close(serverFd);
    close(clientFd);
    throw myException("send request to server failed");
  }

  std::cout << request->getId() << ": processPost sendToServer complete" << std::endl;
  //then recv response from the server
  HTTPResponse response;
  try {
    response = pserver.recvChunk(serverFd, request->getUrl(), request->getId());
    std::string recvResponseLog = std::to_string(request->getId()) + ": Received \"" +
                                  response.getStartLine() + "\" from " +
                                  request->getUrl() + "\n";
    writeLock.lock();
    logger.writeLog(recvResponseLog);
    writeLock.unlock();
    std::string postStatus = response.getStatus();
  }
  catch (myException & e) {
    std::cout << e.what() << std::endl;
  }
  catch (...) {
    std::cout << "recv response failed" << std::endl;
    std::string Response500("HTTP/1.1 500 Bad response\r\n\r\n");
    try {
      if (send(clientFd, Response500.c_str(), Response500.length(), 0) == -1) {
        close(clientFd);
        throw myException("sending 500 response OK failed");
      }
      return;
    }
    catch (myException & e) {
      std::cout << e.what() << std::endl;
      return;
    }
  }
  std::cout << request->getId() << ": processPost recvFromServer complete" << std::endl;

  //then send back specific message according to response
  std::string postStatus = response.getStatus();
  std::cout << "--------get response status " << postStatus << "." << std::endl;
  if (postStatus == "200") {
    try {
      std::string sendToClientLog = std::to_string(request->getId()) + ": Responding \"" +
                                    response.getStartLine() + "\"\n";
      writeLock.lock();
      logger.writeLog(sendToClientLog);
      writeLock.unlock();
      pserver.sendResponse(clientFd, response);
      std::cout << "--------send response 200" << std::endl;
    }
    catch (myException & e) {
      close(serverFd);
      std::cout << e.what() << std::endl;
    }
  }
  else if (postStatus[0] == '3') {
    try {
      std::string sendToClientLog = std::to_string(request->getId()) + ": Warning " +
                                    response.getStartLine() + "\n";
      writeLock.lock();
      logger.writeLog(sendToClientLog);
      writeLock.unlock();
      pserver.sendResponse(clientFd, response);
      std::cout << "--------send response 300" << std::endl;
    }
    catch (myException & e) {
      close(serverFd);
      std::cout << e.what() << std::endl;
    }
  }
  else if (postStatus[0] == '4' || postStatus[0] == '5') {
    try {
      std::string sendToClientLog =
          std::to_string(request->getId()) + ": Error " + response.getStartLine() + "\n";
      writeLock.lock();
      logger.writeLog(sendToClientLog);
      writeLock.unlock();
      pserver.sendResponse(clientFd, response);
      std::cout << "--------send response 400 or 500" << std::endl;
    }
    catch (myException & e) {
      close(serverFd);
      std::cout << e.what() << std::endl;
    }
  }

  else {
    try {
      std::string sendToClientLog =
          std::to_string(request->getId()) + ": Error unkown error \n";
      writeLock.lock();
      logger.writeLog(sendToClientLog);
      writeLock.unlock();
      pserver.sendResponse(clientFd, response);
    }
    catch (myException & e) {
      close(serverFd);
      std::cout << e.what() << std::endl;
    }
  }
  close(serverFd);
  close(clientFd);
}
void HTTPProxy::processGet(HTTPRequest * request) {
  HTTPResponse response;
  if (cache.exists(request->getUrl())) {
    response = cache.get(request->getUrl());
    //no-cache means revalidation needed
    if (response.getHeaders()["Cache-Control"].find("no-cache") != std::string::npos) {
      int serverFd = pserver.connectServer(request->getHostnameAndPort());
      bool isValid = validate(response, *request, serverFd);
      if (isValid) {
        //valid, get from cache
        std::string cacheLog = std::to_string(request->getId()) + ": in cache, valid\n";
        writeLock.lock();
        logger.writeLog(cacheLog);
        writeLock.unlock();
        response = cache.get(request->getUrl());
        std::cout << request->getId() << " processGet found in cache" << std::endl;
      }
      else {
        //invalid, ask for server and renew cache
        std::string cacheLog = std::to_string(request->getId()) + ": in cache, invalid\n";
        writeLock.lock();
        logger.writeLog(cacheLog);
        writeLock.unlock();
        response = askServer(*request);
        cache.add(std::pair<std::string, HTTPResponse>(request->getUrl(), response));
      }
    }
    //check expiration
    else {
      bool isExpired = checkTime(response);
      if (isExpired) {
        //expired, ask for server and renew cache
        std::string cacheLog = std::to_string(request->getId()) + ": in cache, expired\n";
        writeLock.lock();
        logger.writeLog(cacheLog);
        writeLock.unlock();
        response = askServer(*request);
        cache.add(std::pair<std::string, HTTPResponse>(request->getUrl(), response));
      }
      else {
        //not expired, get from cache
        std::string cacheLog = std::to_string(request->getId()) + ": in cache, valid\n";
        writeLock.lock();
        logger.writeLog(cacheLog);
        writeLock.unlock();
        response = cache.get(request->getUrl());
        std::cout << request->getId() << " processGet found in cache" << std::endl;
      }
    }
    pserver.sendResponse(request->getClientFd(), response);
    std::cout << request->getId() << " processGet response sent" << std::endl;
  }
  //not found in cache
  else {
    //TODO:logger
    //TODO:try catch
    std::string requestLog = std::to_string(request->getId()) + ": Requesting \"" +
                             request->getStartLine() + "\" from " + request->getUrl() +
                             " @ " + request->getRecvTime() + "\n";
    writeLock.lock();
    logger.writeLog(requestLog);
    writeLock.unlock();

    int serverFd = pserver.connectServer(request->getHostnameAndPort());
    pserver.sendMessage(serverFd, *request);

    HTTPResponse response =
        pserver.recvChunk(serverFd, request->getUrl(), request->getId());

    pserver.sendResponse(request->getClientFd(), response);
    std::string respondLog = std::to_string(request->getId()) + ": Responding \"" +
                             response.getStartLine() + "\"\n";
    writeLock.lock();
    logger.writeLog(respondLog);
    writeLock.unlock();
    //302
    if (response.getStatus() == "302") {
      std::cout << request->getId() << ":302 recieved" << std::endl;
      HTTPRequest * req = pserver.recvRequest(
          request->getClientFd(), request->getClientIp(), request->getId());
      processGet(req);
      delete req;
      return;
    }
    //    response.printMessage();
    if (response.getHeaders()["Transfer-Encoding"] == "chunked") {
      //chunked responses not cacheable
      std::string cacheLog =
          std::to_string(request->getId()) + ": uncacheable, chunked\n";
      writeLock.lock();
      logger.writeLog(cacheLog);
      writeLock.unlock();

      fwdChunks(serverFd, request->getClientFd());
    }
    //if response is not cachable becasue no store
    else if (response.getHeaders()["Cache-Control"].find("no-store") !=
             std::string::npos) {
      std::string cacheLog =
          std::to_string(request->getId()) + ": uncacheable, no-store\n";
      writeLock.lock();
      logger.writeLog(cacheLog);
      writeLock.unlock();
    }
    //otherwise cache it
    else {
      cache.add(std::pair<std::string, HTTPResponse>(request->getUrl(), response));
      std::map<std::string, std::string> headers = response.getHeaders();
      writeLock.lock();
      if (headers.find("Cache-Control") != headers.end()) {
        std::string cacheLog = std::to_string(request->getId()) +
                               ": NOTE Cache-Control: " + headers["Cache-Control"];
        logger.writeLog(cacheLog);
        if (headers["Cache-Control"].find("must-revalidate") != std::string::npos ||
            headers["Cache-Control"].find("no-cache") != std::string::npos) {
          cacheLog =
              std::to_string(request->getId()) + ": cached, but requires re-validation\n";
          logger.writeLog(cacheLog);
        }
      }
      if (headers.find("ETag") != headers.end()) {
        std::string cacheLog =
            std::to_string(request->getId()) + ": NOTE ETag: " + headers["ETag"];
        logger.writeLog(cacheLog);
      }
      writeLock.unlock();
    }

    cache.printKeyList();
    cache.printMap();
    std::cout << request->getId() << ":"
              << "processGet complete" << std::endl;
    close(request->getClientFd());
    close(serverFd);
  }
}
bool HTTPProxy::validate(HTTPResponse response, HTTPRequest request, int serverFd) {
  std::map<std::string, std::string> resHeaders = response.getHeaders();
  if (resHeaders.find("ETag") == resHeaders.end() &&
      resHeaders.find("Last-Modified: ") == resHeaders.end()) {
    return true;
  }
  if (resHeaders.find("ETag") != resHeaders.end()) {
    request.addHeader(
        std::pair<std::string, std::string>("If-None-Match", resHeaders["ETag"]));
  }
  if (resHeaders.find("Last-Modified") != resHeaders.end()) {
    request.addHeader(std::pair<std::string, std::string>(
        "If-Modified-Since", resHeaders[resHeaders["Last-Modified"]]));
  }
  //TODO check if send successfulyy
  pserver.sendMessage(serverFd, request);
  //TODO recv chunk?
  HTTPResponse validationRes =
      pserver.recvResponse(serverFd, request.getUrl(), request.getId());
  if (validationRes.getStatus() != "200") {
    //TODO logger
    std::cout << request.getId() << ":validate in cache, requires validation"
              << validationRes.getStatus() << std::endl;
  }
  else {
    std::cout << request.getId() << ":validate success " << std::endl;
  }
  return true;
}
void HTTPProxy::processRequest(HTTPRequest * request) {
  std::string method = request->getMethod();
  if (method == "CONNECT" || method == "POST" || method == "GET") {
    std::string recvMessageLog =
        std::to_string(request->getId()) + ": \"" + request->getStartLine() + "\" from " +
        request->getClientIp() + " @ " + request->getRecvTime() + "\n";
    writeLock.lock();
    logger.writeLog(recvMessageLog);
    writeLock.unlock();
  }

  if (method == "CONNECT") {
    try {
      processConnect(request);
    }
    catch (myException & e) {
      std::cout << e.what() << std::endl;
    }
  }
  else if (method == "POST") {
    try {
      processPost(request);
    }
    catch (myException & e) {
      std::cout << e.what() << std::endl;
    }
  }
  else if (method == "GET") {
    try {
      processGet(request);
      std::cout << "ProcessGet complete ------------------------------" << std::endl;
    }
    catch (myException & e) {
      std::cout << e.what() << std::endl;
    }
  }
  else {
    std::string noMethodLog = ("HTTP/1.1 t found no method exist\r\n\r\n");
    noMethodLog = std::to_string(request->getId()) + ": " + noMethodLog;
    writeLock.lock();
    logger.writeLog(noMethodLog);
    writeLock.unlock();
    //TODO: Response form
    if (send(request->getClientFd(), noMethodLog.c_str(), noMethodLog.length(), 0) ==
        -1) {
      throw myException("process request: no method sending failed");
    }
  }
  //TODO check free and close
}
void HTTPProxy::serveConnection(int clientSock, std::string clientIp) {
  try {
    std::cout << "serveConnection start with clientSock " << clientSock << " with id"
              << requestId << std::endl;
    writeLock.lock();
    size_t thisId = requestId++;
    writeLock.unlock();
    std::cout << "serveConnection start to get request with id " << thisId << std::endl;
    HTTPRequest * request = pserver.recvRequest(clientSock, clientIp, thisId);
    //wrtieLog
    processRequest(request);
    std::cout << "serveConnection Complete with Id " << request->getId() << std::endl;
    delete request;
  }
  catch (myException & e) {
    std::cout << e.what() << std::endl;
  }
  catch (...) {
    std::cout << "recv request failed" << std::endl;
    std::string Request400("HTTP/1.1 400 Bad Request\r\n\r\n");
    try {
      if (send(clientSock, Request400.c_str(), Request400.length(), 0) == -1) {
        close(clientSock);
        throw myException("sending 400 request OK failed");
      }
    }
    catch (myException & e) {
      std::cout << e.what() << std::endl;
    }
    close(clientSock);
  }
}
HTTPResponse HTTPProxy::askServer(HTTPRequest request) {
  std::cout << request.getId() << ":asking server for " << request.getUrl() << std::endl;
  int serverFd = pserver.connectServer(request.getHostnameAndPort());
  pserver.sendMessage(serverFd, request);
  HTTPResponse response =
      pserver.recvResponse(serverFd, request.getUrl(), request.getId());

  std::cout << request.getId() << ": recv " << response.getStartLine()
            << "after asking server" << std::endl;
  close(serverFd);
  return response;
}

bool HTTPProxy::checkTime(HTTPResponse res) {
  std::map<std::string, std::string> headers = res.getHeaders();
  if (headers.find("expires") != headers.end()) {
    std::time_t expireTime = timeTransfer(headers["expires"]);
    std::time_t currentTime = time(0);
    if (currentTime > expireTime) {
      return false;
    }
  }
  //  if ()
  return false;
}

//   time transfer function
std::time_t HTTPProxy::timeTransfer(std::string time) {
  struct tm tRes;
  if (strptime(time.c_str(), "%a, %d %b %Y %H:%M:%S", &tRes) == NULL) {
    std::cerr << "strptime failed" << std::endl;
  }
  else {
    tRes.tm_isdst = -1;
    std::time_t res = mktime(&tRes);
    if (res != -1) {
      return res;
    }
  }
  return -1;
}
void HTTPProxy::fwdChunks(int serverFd, int clientFd) {
  while (1) {
    char buf[MAXDATASIZE];
    int numBytes = recv(serverFd, buf, MAXDATASIZE - 1, 0);
    buf[numBytes] = 0;
    if (numBytes == -1) {
      break;
    }
    //end of chunk stream
    if (strcmp(buf, "0\r\n\r\n") == 0) {
      break;
    }
    //TODO: error checking

    numBytes = send(clientFd, buf, numBytes, 0);
    //std::cout << "Send " << numBytes << " bytes" << std::endl;
  }
}
