
.PYTHON: clean

CC=g++
CFLAGS=-Wall -O -g -lboost_system -lserved -ljsoncpp -lpthread

app:
	$(CC) -o app HttpServer.cpp main.cpp $(CFLAGS)

clean:
	rm -f *.o app
