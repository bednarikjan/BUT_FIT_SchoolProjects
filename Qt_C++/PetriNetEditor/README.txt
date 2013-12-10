************************************************
Projekt do predmetu ICP - Simulator Petriho siti
************************************************

Autori: Jan Bednarik a Petr Konvalinka

Datum:  6.5.2012

Zadanim projektu bylo vytvorit editor a simulator vysokourovnovych Petriho siti.
Aplikace je typu klient-server.

********Adresarova struktura projektu:**********
xkonva03/src/				- zdrojove soubory
xkonva03/doc/				- dokumentace
xkonva03/examples/clientExampleNets 	- lokalni petriho site na ukazku
xkonva03/examples/serverExampleNets	- verzovane site na serveru(pouze na ukazku,pro splneni zadani
					  server si to uklada do sve slozky saveNets)

xkonva03/README.txt			- zakladni prehled
xkonva03/Makefile 			- makefile pro vytvoreni spustitelnych souboru



********Kompilace a spusteni:*******************
make 			- prelozi vse
make run		- spusti klienta
make runserver		- spusti server
make clean		- vymaze vsechny soubory vznikle pri prekladu
make pack		- vytvori archiv pro odevzdani
make doxygen	 	- vygeneruje dokumentaci

********Pouziti**********************************

Priklad simulace site ze serveru, neregistrovaneho uzivatele:

Server->new user->File->Open from server->Simulation->Start->Whole net

Priklad ulozeni site na server:

Server->Connect->File->Save to server




