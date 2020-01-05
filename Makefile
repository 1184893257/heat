all: cgi daemon

CC = g++

CXXFLAGS += -O0 -g -std=gnu++11

cgi: cgi.o ipc.o urldecoder.o

daemon: daemon.o ipc.o

clean:
	rm *.o
	rm cgi daemon

install:
	cp -f cgi /var/www/html/heat/bin/heat.cgi
	cp -f daemon /var/www/html/heat/bin/
	cp -f index.html /var/www/html/heat/
	
	mkdir -p /usr/lib/systemd/system
	cp -u heat.service /usr/lib/systemd/system/
	systemctl enable heat.service
	systemctl restart heat.service