all: cgi

CC = g++

cgi: cgi.o

install:
        cp cgi /var/www/html/heat/bin/heat.cgi
        cp index.html /var/www/html/heat/
