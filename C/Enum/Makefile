# SOUBOR: makefile
# PROJEKT: IJC-DU1
# DATUM: 17.3.2011
# AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz 
# FAKULTA: FIT VUT Brno 
# POPIS: Program make bez parametru vytvori spustitelne soubory 
# enumtest, prvocisla, prvocisla-inline.


# Prekladac
CC=gcc
# Parametry prekladace.
CFLAGS=-std=c99 -pedantic -Wall
# Objektove soubory.
OBJFILES=error.o enum.o enumtest.o prvocisla.o prvocisla-inline.o
# Parametry sestaveni.
LIBS=-lm
# Objektove soubory pro program enumtest
OBJS_ENUMTEST=enumtest.o enum.o error.o
# Objektove soubory pro program prvocisla
OBJS_PRVOCISLA=prvocisla.o error.o
# Objektove soubory pro program prvocisla-inline
OBJS_PRVOCISLAINLINE=prvocisla-inline.o error.o

#Startovaci pravidlo
all: enumtest prvocisla prvocisla-inline

# Pravidla pro vystupni soubory
enumtest: $(OBJS_ENUMTEST)
	$(CC) $(CFLAGS) -o $@ $(OBJS_ENUMTEST)
prvocisla: $(OBJS_PRVOCISLA)
	$(CC) $(CFLAGS) $(LIBS) -o $@ $(OBJS_PRVOCISLA)
prvocisla-inline: $(OBJS_PRVOCISLAINLINE)
	$(CC) $(CFLAGS) $(LIBS) -o $@ $(OBJS_PRVOCISLAINLINE) 

# Obecne pravidlo
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Pravidla pro objektove soubory
error.o: error.c error.h
enum.o: enum.c enum.h error.h
enumtest.o: enumtest.c error.h enum.h
prvocisla.o: prvocisla.c error.h
	$(CC) $(CFLAGS) -O2 -c -o prvocisla.o $<
prvocisla-inline.o: prvocisla.c error.h
	$(CC) $(CFLAGS) -O2 -c -DUSE_INLINE -o prvocisla-inline.o $<

.PHONY: clean pack

# Smazani objektovych a spustitelnych souboru.
clean: 
	-rm -f *.o enumtest prvocisla prvocisla-inline
# Zabaleni *.c a *.h souboru a makefilu do archivu.
pack: 
	-zip xbedna45.zip *.c *.h Makefile

