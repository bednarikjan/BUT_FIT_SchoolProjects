################################################################################
#
# Project: IMS - analog lowpass filter
# File: Makefile
# Date: 9.12.2012
# Authors: 
#   Martin Janys (xjanys00)
#   Jan Bednarik (xbedna45)
#
################################################################################

#Pouziti:
#	make - prelozi vse
#	make clean - vymaze vsechny soubory vznikle pri prekladu
#	make pack - vytvori archiv pro odevzdani

all:
	make -C src
	cp src/lowpass .
	
.PHONY: clean pack run

run:
	make -C src run

clean:
	make -C src clean
	rm lowpass
	
pack:
	zip -r -q 10_xbedna45_xjanys00.zip Makefile Readme src/* doc/dokumentace.pdf wav/drums.wav  