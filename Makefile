all: heat.cgi

heat.cgi: cgi.o
	g++ -o heat.cgi $^

install:
	cp heat.cgi /var/www/html/heat/bin/
	cp index.html /var/www/html/heat/