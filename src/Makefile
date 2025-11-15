CC=gcc
CFLAGS=-Wall -Wextra -O2
LDFLAGS=
BIN=asynserver

all: $(BIN)

$(BIN): server.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

server.o: server.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(BIN)

.PHONY: all clean
