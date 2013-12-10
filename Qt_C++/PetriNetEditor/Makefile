# Projekt:	pn2012 - projekt do predmetu ICP
#	Autori:		Jan Bednarik, Petr Konvalinka 
#	Datum:		5.5.2012

# pouziti:
#	make - prelozi vse
#	make run	- spusti klienta
# make runserver	- spusti server
#	make clean - vymaze vsechny soubory vznikle pri prekladu
#	make pack	 - vytvori archiv pro odevzdani

.PHONY= all run runserver pack clean

export PATH:=/usr/local/share/Qt-4.7.3/bin/:$(PATH)

#prelozi vse
all:
	cd src; cd client; qmake
	cd src; cd client; make
	cd src; cd server; qmake
	cd src; cd server; make
#spusti klienta
run: 
	src/client/pn2012	
      
#spusti server
runserver: 
	cd src; cd server; ./server2012 -p 48825 

# vygeneruje dokumentaci
doxygen:
	doxygen doxyConfig_PetriNet.cfg
		
# zabali vse pro odevzdani
pack:	clean
		zip xkonva03.zip -r ../xkonva03
       
# smaze soubory vznikle pri prekladu
clean:
	rm -rf doc/* src/client/moc_*.cpp src/server/moc_*.cpp src/client/*.o src/server/*.o  src/client/pn2012 src/client/Makefile src/server/server2012 src/server/Makefile src/server/logins.txt	

 
