all: process server server_concurr

process: src/process.c src/file_reader.c
	gcc src/process.c src/file_reader.c -o src/process

CFLAGS=-Iinclude
LIBS=-lssl -lcrypto

server: src/server.c src/parse_cert.c src/file_reader.c src/server_helper.c src/check_sig.c
	gcc $(CFLAGS) src/server.c src/file_reader.c src/parse_cert.c src/server_helper.c src/check_sig.c -o src/server $(LIBS)

server_concurr: src/server_concurr.c src/parse_cert.c src/file_reader.c src/server_helper.c src/check_sig.c
	gcc $(CFLAGS) src/server_concurr.c src/file_reader.c src/parse_cert.c src/server_helper.c src/check_sig.c -o src/server_concurr $(LIBS)

clean:
	rm -f src/process src/server src/server_concurr