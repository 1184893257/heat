all: cgi

CC = g++

CXXFLAGS += -std=gnu++11

cgi: cgi.o ipc.o

daemon: daemon.o ipc.o

install:
	cp cgi /var/www/html/heat/bin/heat.cgi
	cp index.html /var/www/html/heat/
