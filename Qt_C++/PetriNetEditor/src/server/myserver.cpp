/** @file: myserver.cpp
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro praci se serverem
  */

#include "myserver.h"
#include "../shared/myXML.h"
#include "cassert"


mainServer::mainServer(int port)
{
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(snewConnection()));
    server->setMaxPendingConnections(100);   

     // vytvoreni souboru s loginy a hesly, pokud jeste neni
     QFile file(LOGINS_FILE);
     if (!file.exists()) {
         file.open(QIODevice::WriteOnly);
         file.close();
     }
     // vytvoreni adresare pro ukladani siti
     QDir dir;
     if (!dir.exists(NET_DIR)) {
         dir.mkdir(NET_DIR);
     }
     // nastaveni addresy a portu
     bool listening = server->listen(QHostAddress::Any,port);

    if(not listening)
       {
          qDebug() << "Error, Server is not listening";
          return;
       }
    qDebug() << "Server is listening on port" << port<< "...";

}

// nove prichozi spojeni
void mainServer::snewConnection()
{
    // novy socket
    QTcpSocket* socket = server->nextPendingConnection();
    if(!socket)
        return;
    connect(socket, SIGNAL(readyRead()), SLOT(sreply()));
    connect(socket, SIGNAL(disconnected()), SLOT(sdisconnected()));

}

// novy pozadavek od klienta
void mainServer::sreply()
{

   QTcpSocket* socket = qobject_cast<QTcpSocket*>(this->sender());   

   myXML xml;
   xml.data=socket->readAll();

   //vrati typ odpovedi
   switch (xml.getRequest()) {

       // pozadavek na autentizaci
       case T_req_authen: {
          QString login,passwd;
          xml.get_req_authenNew(&login,&passwd);
          xml.clear();
          xml.add_rep_authen(findUser(login,passwd));
          author=login;
          this->write(socket,xml.data);
          break;
       }

       // pozadavek na pridani noveho uzivatele
       case T_req_newUser: {
          QString login,passwd;
          xml.get_req_authenNew(&login,&passwd);
          xml.clear();
          if (findLogin(login)) xml.add_rep_newUser(false);
          else xml.add_rep_newUser(addUser(login,passwd));
          author=login;
          this->write(socket,xml.data);
          break;
       }

       // pozadavek na ulozeni site
       case T_req_save: {
            QString name,pom,descr,version;
            if (xml.get_req_save(&name,&descr)) {
                xml.delete_message();
                pom=xml.data;
                xml.clear();
                if (saveNet(pom,name,&version,descr)) xml.add_rep_save(true,version);
                else xml.add_rep_save(false,version);
                this->write(socket,xml.data);
            }
            break;
       }

       // pozadavek na nacteni site
       case T_req_load: {
            QString name,version,net,log;            
            if (xml.get_req_load(&name,&version)) {
                xml.clear();
                if (loadNet(&net,name,version,&log)) xml.add_rep_load(true,net,log,version);
                else xml.add_rep_load(false,net,log,version);
                this->write(socket,xml.data);
            }
            break;
       }

       // pozadavek na zobrazeni ulozenych siti
       case T_req_showNets: {
           QString regex;
           if (xml.get_req_showNets(&regex)) {
               xml.clear();
               xml.add_rep_showNets(GetNets(regex));
               this->write(socket,xml.data);
           }
           break;
       }

       // pozadavek na start simulace
       case T_req_startSim: {
            QString name,version,net,log;
            bool doable;
            if (xml.get_req_startSim(&name,&version)) {
                xml.clear();
                if (loadNet(&net,name,version,&log)) { //nacteni site ze souboru
                    xml.data = net;
                    xml.createNetCore(&lists);
                    appendSim(name,version); // pridani logu o simulaci
                    
                    // DOPLNIT volani SIMULACE - ZJISTENI PRECHODU
                    
                    simul.init(&lists);
                    doable = simul.findDoableTransitions(&lists);
                    
                    if(doable) {
                        // zapis site po provedeni simulace do stringu net.
                        xml.clear();
                        xml.createSceneXML(&lists);
                        net = xml.data;
                        xml.clear();
                        
                        xml.add_rep_startSim(true,net); // odpoved i s prechody
                    } else {
                        // simulace je u konce.
                        xml.clear();
                        xml.add_rep_endSim();
                        this->write(socket,xml.data);
                        removeNet();
                    }
                }
                else xml.add_rep_startSim(false,net);                

                this->write(socket,xml.data);
            }
            break;
       }
       // pozadavek na celou simulaci
       case T_req_fullSim: {
            QString maxStr,net;
            int max;
            int stepNumber;
            int id;
            bool doable = true;
            if (xml.get_req_fullSim(&maxStr)) {
                max=maxStr.toInt();
                //DOPLNIT!! simulace cele site, v max je pocet kroku
                stepNumber = 0;
                
                while(doable && stepNumber < max) {
                    id = chooseDoableTransition();
                    simul.step(&lists, id);
                    doable = simul.findDoableTransitions(&lists);
                    stepNumber++;
                }

                xml.clear();
                xml.createSceneXML(&lists);
                net = xml.data;
                xml.clear();
                xml.add_rep_fullSim(net);
                this->write(socket,xml.data);
                removeNet();
            }
            break;
       }

       // pozadavek na simulaci jednoho kroku1,posle sit
       case T_req_stepSim: {
            int id;
            QString idStr,net;
            bool doable;
            if (xml.get_req_stepSim(&idStr)) {
                 id=idStr.toInt();
                 
                 simul.step(&lists, id);
                 doable = simul.findDoableTransitions(&lists);

                 xml.clear();
                 xml.createSceneXML(&lists);
                 net = xml.data;
                 xml.clear();
                 
                 if(doable) {
                     xml.add_rep_stepSim(net);

                    this->write(socket,xml.data);
                 } else {
                     // simulace je u konce.
                     xml.add_rep_fullSim(net);
                     //xml.add_rep_endSim();
                     this->write(socket,xml.data);                     
                     removeNet();
                 }
            }
            break;
       }

       // pozadavek na ukonceni simulace,nic se neposila
       case T_req_endSim: 
            removeNet();
            break;

       // neznama chyba, nejspise pri parsovani
       case T_bad_req: {
         qDebug() << "Unknown Error";
            break;
       }
   }

}


// zavreni spojeni
void mainServer::sdisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>( this->sender() );
    socket->deleteLater();
}

// nalezeni loginu
bool mainServer::findLogin(QString login){
    QFile file(LOGINS_FILE);
    if (!file.open(QIODevice::ReadOnly)) {
               return false;
    }
    Tpole pole;
    QDataStream in(&file);
    pole.empty();
    in >> pole;
    file.close();
    Tpole::Iterator it;

    // vyhledame zadany login
    for ( it = pole.begin(); it != pole.end(); ++it ) {
        if (it.key()==login) {
            return true;
        }
    }
    return false;
}
// nalezeni loginu a porovnani hesla
bool mainServer::findUser(QString login,QString heslo) {
    QFile file(LOGINS_FILE);
    if (!file.open(QIODevice::ReadOnly)) {
               return false;
    }
    Tpole pole;
    QDataStream in(&file);
    pole.empty();
    in >> pole;
    file.close();
    Tpole::Iterator it;

    // vyhledame zadany login a a porovname heslo
    for ( it = pole.begin(); it != pole.end(); ++it ) {
        if (it.key()==login) {
            if  (it.value()==heslo) {
                return true;
            }
            return false;
        }
    }
    return false;
}

// pridani noveho uzivatele
bool mainServer::addUser(QString login,QString heslo) {
    QFile file(LOGINS_FILE);
    if (!file.open(QIODevice::ReadOnly)) {
               return false;
    }
    Tpole pole;
    QDataStream in(&file);
    pole.empty();
    in >> pole;
    file.close();
    pole[login]=heslo;   // pridani do pole

    if (!file.open(QIODevice::WriteOnly)) {        
        return false;
    }

    QDataStream out(&file);
    out << pole;
    file.close();
    return true;
}

// funkce ulozi xml sit,tvar: "nazev"__v"cisloverze"
bool mainServer::saveNet(QString net,QString name,QString *version,QString description) {
    QString cesta=NET_DIR;
    QString log;
    int numVersion=0;
    cesta.append(name);
    cesta.append("__v");
    //zjisteni zda uz sit neexistuje pripadne vybere nejvetsi verzi
    QStringList listvalue=GetNets("");
    for (int i = 0; i < listvalue.size();i+=2) {
        if (name==listvalue.at(i)) {            
            if (numVersion<listvalue.at(i+1).toInt()) {
                numVersion=listvalue.at(i+1).toInt();
            }
        }
    }
    *version = QString::number(++numVersion);
    cesta.append(version);
    log=cesta;
    cesta.append(".xml");
    log.append(".log");
    QFile file(cesta);
    QFile fileLog(log);
    if (file.open(QIODevice::WriteOnly)) { // vytvoreni noveho souboru pro sit
       if (fileLog.open(QIODevice::WriteOnly)) { // vytvoreni noveho souboru pro log
          QByteArray pom;
          pom.append(net);
          file.write(pom);
          pom.clear();
          pom.append("Name: ");
          pom.append(name);
          pom.append("\nVersion: ");
          pom.append(*version);
          pom.append("\nAuthor: ");
          pom.append(author);
          pom.append("\nDescription: ");
          pom.append(description);
          pom.append("\n\nSimulations(Author, DateTime):");
          fileLog.write(pom);
          fileLog.close();
          file.close();
       return true;
       }
    }
    return false;
}

// funkce nacte sit do retezce
bool mainServer::loadNet(QString *net,QString name,QString version,QString *log) {
    QString cestaLog;
    QString cesta=NET_DIR;
    cesta.append(name);
    cesta.append("__v");
    cesta.append(version);
    cestaLog=cesta;
    cesta.append(".xml");
    cestaLog.append(".log");
    QFile fileLog(cestaLog);
    QFile file(cesta);
    if ((!file.open(QIODevice::ReadOnly)) || (!fileLog.open(QIODevice::ReadOnly)))  {
               return false;
    }
    net->append(file.readAll());
    log->append(fileLog.readAll());
    fileLog.close();
    file.close();
    return true;
 }

// funkce vrati seznam ulozenych siti
// seznam ma tvar: nazev,verze,nazev,verze....
QStringList mainServer::GetNets(QString regex) {
    QDir dir(NET_DIR);
    QStringList listvalue;
    QStringList listdir=dir.entryList(QDir::Files);

    // hledat se bude i podle klicoveho slova,nemusi byt zadano
    QString pom=".*";
    if (!(regex=="")) {
        pom.append(regex);
        pom.append(".*");
    }
    // regularni vyraz vytahne ze souboru nazev a verzi
    QRegExp rxlen("^(.*)__v(\\d+).xml$");
    QRegExp key(pom);
    for (int i = 0; i < listdir.size();i++) {
        if (rxlen.exactMatch(listdir.at(i))) {
            if (key.exactMatch(rxlen.cap(1))) { // pokud obsahuje klicove slovo
              listvalue.append(rxlen.cap(1));  // nazev
              listvalue.append(rxlen.cap(2));  // verze
            }
        }
    }
    return listvalue;
}

void mainServer::write(QTcpSocket* socket,QString data) {
    QByteArray pom;
    pom.append(data);
    socket->write(pom);
    socket->flush();
}

// prida log o simulaci do souboru
void  mainServer::appendSim(QString name,QString version) {
    QString cesta=NET_DIR;
    QTime time;
    QDate date;
    cesta.append(name);
    cesta.append("__v");
    cesta.append(version);
    cesta.append(".log");
    QFile file(cesta);
    if (file.open(QIODevice::Append)) {
        QByteArray data;
        data.append("\n");
        data.append(author);
        data.append(", ");
        data.append(date.currentDate().toString("d.M.yyyy"));
        data.append(" ");
        data.append(time.currentTime().toString());
        file.write(data);
    }
    file.close();
}

// Smaze sit - uvolni pamet
void mainServer::removeNet() {
    // mista
    foreach(PetriNetPlaceCore *place, lists.places)
        delete place;
    
    // prechody
    foreach(PetriNetTransitionCore *transition, lists.transitions) {
        delete transition->parser;
        delete transition;
    }
    
    // hrany
    foreach(PetriNetArcCore *arc, lists.arcs)
        delete arc;
}

// Vybere proveditelny prechod
int mainServer::chooseDoableTransition() {
    int id = 0;
    
    foreach(PetriNetTransitionCore *trans, lists.transitions) {
        if(trans->isDoable()) {
            id = trans->getID();
            break;
        }
    }
    
    assert(id != 0);
    return id;
}
