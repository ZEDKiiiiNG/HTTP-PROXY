#ifndef __HTTPPROXY_H__
#define __HTTPPROXY_H__
#include "common_headers.h"
#include "proxy_server.h"
#include "myexception.h"


class httpProxy{
    public:
    //TODO: Log,Cache
    proxyServer proxy_server;
    int RequestID;
    void setUp();

    httpProxy():proxy_server("3490"){
        RequestID = 0;
    }
    ~httpProxy(){}

    recvRequest();
}