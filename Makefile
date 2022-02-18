CPPFLAGS=-std=gnu++11 -pedantic -Wall -Werror -ggdb3
#all: test
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
proxyServer.o: proxyServer.cpp
	g++ $(CPPFLAGS) -c proxyServer.cpp
HTTPRequest.o: HTTPRequest.cpp
	g++ $(CPPFLAGS) -c HTTPRequest.cpp
HTTPResponse.o: HTTPResponse.cpp
	g++ $(CPPFLAGS) -c HTTPResponse.cpp
