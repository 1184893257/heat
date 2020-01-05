all: cgi daemon

CC = g++

CXXFLAGS += -O0 -g -std=gnu++11

cgi: cgi.o ipc.o

daemon: daemon.o ipc.o

install:
	cp cgi /var/www/html/heat/bin/heat.cgi
	cp daemon /var/www/html/heat/bin/
	cp index.html /var/www/html/heat/
	
	mkdir -p /usr/lib/systemd/system
	cp heat.service /usr/lib/systemd/system/
	systemctl enable heat.service
	systemctl restart heat.service