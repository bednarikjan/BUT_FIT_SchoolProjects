################################################################################
#
# Project: IMS - analog lowpass filter
# File: Makefile
# Authors: 
#   Martin Janys (xjanys00)
#   Jan Bednarik (xbedna45)
#
################################################################################

# Compiler and flags
CC=g++
CFLAGS=

# Object files
OBJ=lowpass.o outputsignal.o wavedata.o
SIMLIB=/usr/local/lib

# All rules
all:lowpass

lowpass:lowpass.o outputsignal.o wavedata.o
	g++ ${CFLAGS} -o $@ ${OBJ} ${SIMLIB}/libsimlib.so -lm

lowpass.o:lowpass.cc lowpass.h outputsignal.h wavedata.h
	g++ ${CFLAGS} -c -o $@ $<

outputsignal.o:outputsignal.cc outputsignal.h
	g++ ${CFLAGS} -c -o $@ $<

wavedata.o:wavedata.cc wavedata.h
	g++ ${CFLAGS} -c -o $@ $<

###

.PHONY:run clean

run:
	./lowpass

clean:
	rm ${OBJ} lowpass
	