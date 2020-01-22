all: heat.cgi daemon ocr_main

CC = g++

CXXFLAGS += -O0 -g -std=gnu++11 -I/usr/include/opencv -DNOTPI

CV_LIBS = `pkg-config --libs opencv`

LDFLAGS += -Wl,--as-needed

heat.cgi: cgi.o ipc.o urldecoder.o config.o
	$(CC) $(CXXFLAGS) $(LDFLAGS) -o heat.cgi $^

daemon: daemon.o ipc.o sg90.o camera.o ocr.o config.o signal_handler.o
	$(CC) $(CXXFLAGS) $(LDFLAGS) -o daemon $^ -lwiringPi -lwiringPiDev $(CV_LIBS)

ocr_main: ocr.o ocr_main.o
	$(CC) $(CXXFLAGS) $(LDFLAGS) -o ocr_main $^ $(CV_LIBS)

clean:
	rm *.o
	rm heat.cgi daemon ocr_main

install:
	cp -f heat.cgi /var/www/html/heat/bin/heat.cgi
	chmod u+s /var/www/html/heat/bin/heat.cgi
	cp -f daemon /var/www/html/heat/bin/
	cp -f index.html /var/www/html/heat/
	
	mkdir -p /usr/lib/systemd/system
	cp -u heat.service /usr/lib/systemd/system/
	systemctl enable heat.service
	systemctl restart heat.service