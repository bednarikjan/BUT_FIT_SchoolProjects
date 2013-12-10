# SOUBOR: makefile
# PROJEKT: IOS-DU2
# DATUM: 1.5.2011
# AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz 
# FAKULTA: FIT VUT Brno 
# POPIS: Program make bez parametru vytvori spustitelny 
# soubor barbers.

# Prekladac.
CC=gcc
# Parametry prekladace.
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic -lrt

# Pravidlo pro cilovy soubor.
barbers: barbers.c
	$(CC) $(CFLAGS) -o $@ $<

phony: clean pack

clean:
	rm -f *.o barbers
pack:
	zip xbedna45 *.c Makefile
