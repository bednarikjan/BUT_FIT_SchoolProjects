# Projekt: Klient/server
# Predmet: IPK
# Program: Makefile
# Autor: Jan Bednarik
# Login: xbedna45
# Datum: 7.4.2012

CC=g++
CFLAGS=-std=c++98 -pedantic -Wall -Wextra

all: server client

server: server.cc
	$(CC) $(CFLAGS) $< -o $@

client: client.cc
	$(CC) $(CFLAGS) $< -o $@


.PHONY: clean pack

clean:
	rm server client
pack:
	tar -czf xbedna45.tar.gz server.cc client.cc Makefile client-server_DOC.pdf