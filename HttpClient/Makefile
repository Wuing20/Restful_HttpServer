
.PYTHON: clean

CC=g++
CFLAGS=-Wall -O -g -lcurl -lpthread

src=$(wildcard *.cpp)
obj=$(patsubst %.c,%.o,$(src))
target=httpclient

$(target):$(obj)
	$(CC) $^ -o $@ $(CFLAGS)

%.o:%.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	-rm -f $(target)
	-rm -f *.o
