CPPFLAGS=-std=gnu++11 -pedantic -Wall -Werror -ggdb3
all: HTTPProxy.o HTTPMessage.o ProxyServer.o HTTPRequest.o HTTPResponse.o
#server: server.o HTTPMessage.o
#	g++ -o server server.o HTTPMessage.o
#server.o: server.c
#	g++ $(CPPFLAGS) -c server.c
#test: test.o HTTPMessage.o proxyServer.o
#	g++ -o test test.o HTTPMessage.o proxyServer.o
#test.o: test.cpp
#	g++ $(CPPFLAGS) -c test.cpp
HTTPProxy.o: HTTPProxy.cpp
	g++ $(CPPFLAGS) -c HTTPProxy.cpp
HTTPMessage.o: HTTPMessage.cpp
	g++ $(CPPFLAGS) -c HTTPMessage.cpp
ProxyServer.o: ProxyServer.cpp
	g++ $(CPPFLAGS) -c ProxyServer.cpp
HTTPRequest.o: HTTPRequest.hpp
	g++ $(CPPFLAGS) -c HTTPRequest.hpp
HTTPResponse.o: HTTPResponse.hpp
	g++ $(CPPFLAGS) -c HTTPResponse.hpp
