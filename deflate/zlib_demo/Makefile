CC = gcc
CFLAGS = -Wall

zlib_compressor: zlib_compressor.c
	$(CC) $(CFLAGS) -o $@ $< -lz

clean:
	rm -f zlib_compressor
