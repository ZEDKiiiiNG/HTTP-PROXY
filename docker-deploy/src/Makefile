CPPFLAGS=-std=gnu++11 -pedantic -Wall -Werror -ggdb3
proxy: proxy.cpp HTTPProxy.o HTTPMessage.o HTTPCache.o  ProxyServer.o LogWritter.o HTTPRequest.hpp HTTPResponse.hpp
	g++ -o proxy proxy.cpp  HTTPProxy.o HTTPMessage.o HTTPCache.o HTTPRequest.hpp HTTPResponse.hpp ProxyServer.o LogWritter.o -lpthread
%.o: %.cpp
	g++ $(CPPFLAGS) -c $<
.PHONY: clean
clean:
	rm -f *.o *.c~ *.h~ *.cpp~ *.hpp~ proxy proxy.log
