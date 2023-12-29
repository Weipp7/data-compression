CC = gcc
CFLAGS = -Wall -std=c99
LDFLAGS = -lbz2

all: bz2_compressor

bz2_compressor: bz2_compressor.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f bz2_compressor
