
.PYTHON: clean

CC=g++
CFLAGS=-Wall -O -g -lboost_system -lserved -ljsoncpp -lpthread

app:
	$(CC) -o app md5.cpp threadpool.cpp Source.cpp HttpServer.cpp main.cpp $(CFLAGS)

clean:
	rm -f *.o app
