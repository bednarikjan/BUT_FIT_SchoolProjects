##
#
# Implementace interpretu imperativniho jazyka IFJ2011
#
# Autori:
#  xbedna45 - Bednarik Jan
#  xblaha22 - Blaha Hynek
#  xjanys00 - Janys Martin
#  xmacha48 - Machacek Ondrej
#
##

## Prekaldac a priznaky
CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -O3

## Objektove soubory
OBJ=ial.o ilist.o interpret.o parser.o scaner.o stack.o str.o ifj11.o stack_exp.o

## Vsechna pravidla.
all: ifj11

## Pravidla pro vysledny spustitelny program
ifj11: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) -lm

## Pravidla pro jednotlive objektove soubory.
ifj11.o: ifj11.c ilist.h scaner.h ial.h parser.h error.h stack_exp.h
	$(CC) $(CFLAGS) -c -o $@ $<
ial.o: ial.c ial.h str.h error.h
	$(CC) $(CFLAGS) -c -o $@ $<
ilist.o: ilist.c ilist.h
	$(CC) $(CFLAGS) -c -o $@ $<
interpret.o: interpret.c interpret.h str.h ilist.h ial.h stack.h
	$(CC) $(CFLAGS) -lm -c -o $@ $<
parser.o: parser.c parser.h scaner.h error.h scaner.h
	$(CC) $(CFLAGS) -c -o $@ $<
scaner.o: scaner.c scaner.h str.h error.h
	$(CC) $(CFLAGS) -c -o $@ $<
stack.o: stack.c stack.h str.h
	$(CC) $(CFLAGS) -c -o $@ $<
str.o: str.c str.h error.h
	$(CC) $(CFLAGS) -c -o $@ $<
stack_exp.o: stack_exp.c stack_exp.h scaner.h ial.h
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean pack
clean:
	rm -f $(OBJ)
pack:
	cp dokumentace/ifj.pdf dokumentace.pdf
	zip -r -q xbedna45 Makefile *.c *.h rozdeleni rozsireni dokumentace.pdf

