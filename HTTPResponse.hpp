#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP
#include "HTTPMessage.hpp"
#include "common_headers.h"

class HTTPResponse : public HTTPMessage{
    private:
    std::string url;
    public:
    //TODO check the constructor
    HTTPResponse(){}
    HTTPResponse(const char * buffer,
                size_t id,
                std::string url) :
        HTTPMessage(buffer, id), url(url) {}
    HTTPResponse(std::vector<char> buffer,
                size_t id,
                std::string url) :
        HTTPMessage(buffer, id), url(url) {}
    std::string getUrl(){
        return url;
    }

    std::string getStatus() {
    std::string s = start_line;
    size_t r1 = s.find(" ");
    size_t r2 = s.find(" ", s.find(" ") + 1);
    std::string s1 = s.substr(s.find(" ")+1 , r2 - r1);
    return s1;
  }
};
#endif