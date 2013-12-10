#Projekt:	mypmtud - projekt do predmetu ISA
#	Autor:		Jan Bednarik (xbedna45)
#	Datum:		18.11.2012

## Prekaldac a priznaky
CC=g++
CFLAGS=-std=c++0x -pedantic -Wall -Wextra

## Objektove soubory
OBJ=main.o params.o targethost.o

## Vsechna pravidla.
all: mypmtud

## Pravidla pro vysledny spustitelny program
mypmtud: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

## Pravidla pro jednotlive objektove soubory.
main.o: main.cpp error.h params.h targethost.h
	$(CC) $(CFLAGS) -c -o $@ $<
	
params.o: params.cpp params.h error.h
	$(CC) $(CFLAGS) -c -o $@ $<
	
targethost.o: targethost.cpp targethost.h error.h
	$(CC) $(CFLAGS) -c -o $@ $<
	
.PHONY: clean pack

clean:
	rm -f $(OBJ) mypmtud
