#Projekt:	mypmtud - projekt do predmetu ISA
#	Autor:		Jan Bednarik (xbedna45)
#	Datum:		18.11.2012

#Pouziti:
#	make - prelozi vse
#	make clean - vymaze vsechny soubory vznikle pri prekladu
#	make pack - vytvori archiv pro odevzdani

all:
	make -C src
	cp src/mypmtud .
	
.PHONY: clean pack

clean:
	make -C src clean
	rm mypmtud
	
pack:
	tar -cf xbedna45.tar *