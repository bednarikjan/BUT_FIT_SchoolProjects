# SOUBOR: makefile
# PROJEKT: IJC-DU2
# DATUM: 17.3.2011
# AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz 
# FAKULTA: FIT VUT Brno 
# POPIS: Program make bez parametru vytvori spustitelne soubory 
# tail, tail2, wordcount, wordcount-dynamic.


# Prekladace
CC=gcc
CPPC=g++
# Parametry prekladace.
CFLAGS=-pedantic -Wall
# Objektove soubory staticke knihovny libhatable.a.
S_LIB_OBJFILES=s_htable_init.o s_hash_function.o s_htable_clear.o s_htable_free.o \
s_htable_begin.o s_htable_end.o s_htable_lookup.o 
# Objektove soubory dynamicke knihovny libhatable.so.
D_LIB_OBJFILES=d_htable_init.o d_hash_function.o d_htable_clear.o d_htable_free.o \
d_htable_begin.o d_htable_end.o d_htable_lookup.o 


#Startovaci pravidlo
all: tail tail2 wordcount wordcount-dynamic

# Pravidla pro vystupni soubory
tail: tail.o
	$(CC) -o $@ tail.o
tail2: tail2.o
	$(CPPC) -o $@ tail2.o
wordcount: wordcount.o io.o libhtable.a
	$(CC) -static -o $@ wordcount.o io.o libhtable.a
wordcount-dynamic: wordcount.o io.o libhtable.so
	$(CC) -o $@ wordcount.o io.o libhtable.so

# Obecne pravidlo pro objektove soubory.
%.o: %.c
	$(CC) -std=c99 $(CFLAGS) -c -O2 -o $@ $<

# Obecne pravidlo pro moduly staticke knihovny programu wordcount.
s_%.o: %.c
	$(CC) -std=c99 $(CFLAGS) -c -O2 -o $@ $<

# Obecne pravidlo pro moduly dynamicke knihovny programu wordcount.
d_%.o: %.c
	$(CC) -std=c99 $(CFLAGS) -fPIC -O2 -c -o $@ $<

# Pravidla pro objektove soubory.
tail.o: tail.c
tail2.o: tail2.cc
	$(CPPC) -std=c++98 $(CFLAGS) -c -o $@ $<
wordcount.o: wordcount.c
io.o: io.c

# Pravidla pro objektove soubory staticke knihovny programu wordcount.
s_htable_init.o: htable_init.c
s_hash_function.o: hash_function.c
s_htable_clear.o: htable_clear.c
s_htable_free.o: htable_free.c
s_htable_begin.o: htable_begin.c
s_htable_end.o: htable_end.c
s_htable_iter_next.o: htable_iter_next.c
s_htable_iter_eq.o: htable_iter_eq.c
s_htable_iter_deref.o: htable_iter_deref.c
s_htable_lookup.o: htable_lookup.c
##s_htable_iter_deref.o: htable_iter_deref.c

# Pravidla pro objektove soubory staticke knihovny programu wordcount.
d_htable_init.o: htable_init.c
d_hash_function.o: hash_function.c
d_htable_clear.o: htable_clear.c
d_htable_free.o: htable_free.c
d_htable_begin.o: htable_begin.c
d_htable_end.o: htable_end.c
d_htable_iter_next.o: htable_iter_next.c
d_htable_iter_eq.o: htable_iter_eq.c
d_htable_iter_deref.o: htable_iter_deref.c
d_htable_lookup.o: htable_lookup.c
##d_htable_iter_deref.o: htable_iter_deref.c


# Pravidla pro knihovny programu wordcount.
libhtable.a: $(S_LIB_OBJFILES)
	ar -rsc libhtable.a $(S_LIB_OBJFILES)
libhtable.so: $(D_LIB_OBJFILES)
	$(CC) $(CFLAGS) -shared -fPIC $(D_LIB_OBJFILES) -o libhtable.so

.PHONY: clean cleanall pack

# Smazani objektovych a spustitelnych souboru.
clean: 
	-rm -f *.o *.a
cleanall:
	-rm -f *.o *.a *.so tail tail2 wordcount wordcount-dynamic
pack:
	zip xbedna45.zip *.c *.cc *.h makefile
