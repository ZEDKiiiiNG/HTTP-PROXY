CPPFLAGS=-std=gnu++11 -pedantic -Wall -Werror -ggdb3
all: testHTTPMessage testHTTPRequest testHTTPResponse testHTTPProxy

#server: server.o HTTPMessage.o
#	g++ -o server server.o HTTPMessage.o
#server.o: server.c
#	g++ $(CPPFLAGS) -c server.c
#test: test.o HTTPMessage.o proxyServer.o
#	g++ -o test test.o HTTPMessage.o proxyServer.o
#test.o: test.cpp
#	g++ $(CPPFLAGS) -c test.cpp
testHTTPMessage: testHTTPMessage.cpp HTTPMessage.o
	g++ -o testHTTPMessage testHTTPMessage.cpp HTTPMessage.o
testHTTPRequest: testHTTPRequest.cpp HTTPRequest.hpp HTTPMessage.o
	g++ -o testHTTPRequest testHTTPRequest.cpp HTTPRequest.hpp HTTPMessage.o
testHTTPResponse: testHTTPResponse.cpp HTTPRequest.hpp HTTPMessage.o
	g++ -o testHTTPResponse testHTTPResponse.cpp HTTPRequest.hpp HTTPMessage.o
testHTTPProxy: testHTTPProxy.cpp HTTPProxy.o ProxyServer.o HTTPMessage.o LogWritter.o
	g++ -o testHTTPProxy testHTTPProxy.cpp HTTPProxy.o ProxyServer.o HTTPMessage.o LogWritter.o -lpthread 
%.o: %.cpp
	g++ $(CPPFLAGS) -c $<
.PHONY: clean
clean:
	rm -f *.o *.c~ *.h~ *.cpp~ *.hpp~
