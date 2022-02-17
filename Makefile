CPPFLAGS=-std=gnu++11 -pedantic -Wall -Werror -ggdb3
all: server test
server: server.o HTTPMessage.o
	g++ -o server server.o HTTPMessage.o
server.o: server.c
	g++ $(CPPFLAGS) -c server.c
test: test.o HTTPMessage.o
	g++ -o test test.o HTTPMessage.o
test.o: test.cpp
	g++ $(CPPFLAGS) -c test.cpp
HTTPMessage.o: HTTPMessage.cpp HTTPMessage.hpp
	g++ $(CPPFLAGS) -c HTTPMessage.cpp
