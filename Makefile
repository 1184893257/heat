all: cgi daemon ocr_main

CC = g++

CXXFLAGS += -O0 -g -std=gnu++11 -I/usr/include/opencv

CV_LIBS = `pkg-config --libs opencv`

cgi: cgi.o ipc.o urldecoder.o

daemon: daemon.o ipc.o sg90.o
	$(CC) $(CXXFLAGS) -o daemon $^ -lwiringPi -lwiringPiDev $(CV_LIBS)

ocr_main: ocr.o ocr_main.o
	$(CC) $(CXXFLAGS) -o ocr_main $^ -lwiringPi -lwiringPiDev $(CV_LIBS)

clean:
	rm *.o
	rm cgi daemon ocr_main

install:
	cp -f cgi /var/www/html/heat/bin/heat.cgi
	cp -f daemon /var/www/html/heat/bin/
	cp -f index.html /var/www/html/heat/
	
	mkdir -p /usr/lib/systemd/system
	cp -u heat.service /usr/lib/systemd/system/
	systemctl enable heat.service
	systemctl restart heat.service