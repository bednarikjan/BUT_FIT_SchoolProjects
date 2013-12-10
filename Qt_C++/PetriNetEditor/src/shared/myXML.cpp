/** @file: myXML.cpp
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro praci s formatem XML
  */

#include "myXML.h"


// konstruktor
myXML::myXML() {
    data="";
    writer= new QXmlStreamWriter(&data);
    writer->setAutoFormatting(true);
    writer->writeStartDocument();
}

// vrati typ pozadavku
TtypeRequest myXML::getRequest() {
    QXmlStreamReader reader(data);
    reader.readNextStartElement();
    if (reader.isStartElement() && reader.name()=="message") {
        return TtypeRequest(reader.attributes().value("type").toString().toInt());
    }
    return T_bad_req;
}
// vrati typ odpovedi
TtypeReply myXML::getReply() {
    QXmlStreamReader reader(data);
    reader.readNextStartElement();
    if (reader.isStartElement() && reader.name()=="message") {
        return TtypeReply(reader.attributes().value("type").toString().toInt());
    }
    return T_bad_rep;
}

// prida typ pozadavek
void myXML::addMessageElReq(TtypeRequest type) {
    QString cislo = QString::number(type);
    writer->writeStartElement("message");
    writer->writeAttribute("type",cislo);
}

// prida typ odpoved
void myXML::addMessageElRep(TtypeReply type) {
    QString cislo = QString::number(type);
    writer->writeStartElement("message");
    writer->writeAttribute("type",cislo);
}

//prida pozadavek na autentizaci
void myXML::add_req_authen(QString login,QString passwd) {
    addMessageElReq(T_req_authen);
    writer->writeStartElement("user");
    writer->writeAttribute("login",login);
    writer->writeAttribute("passwd",passwd);
    writer->writeEndElement();
    writer->writeEndElement();
}

//vrati pozadavek na autentizaci nebo noveho uzivatele
bool myXML::get_req_authenNew(QString *login,QString *passwd) {
    QXmlStreamReader reader(data);
    reader.readNextStartElement();
    if (reader.isStartElement() && reader.name()=="message") {
        reader.readNextStartElement();
        if (reader.isStartElement() && reader.name()=="user") {
           *login=reader.attributes().value("login").toString();
           *passwd=reader.attributes().value("passwd").toString();
           return true;
        }
    }
    return false;
}

//prida pozadavek na pridani noveho uzivatele
void myXML::add_req_newUser(QString login,QString passwd) {
    addMessageElReq(T_req_newUser);
    writer->writeStartElement("user");
    writer->writeAttribute("login",login);
    writer->writeAttribute("passwd",passwd);
    writer->writeEndElement();
    writer->writeEndElement();
}

// prida pozadavek na ulozeni site
void myXML::add_req_save(Lists *list,QString name,QString description) {
    addMessageElReq(T_req_save);
    writer->writeTextElement("name",name);
    writer->writeTextElement("descr",description);
    writer->writeEndElement();
    createSceneXML(list);
    //writer->writeEndElement();
}

// vrati pozadavek na ulozeni site
bool myXML::get_req_save(QString *name,QString *descr) {
    QXmlStreamReader reader(data);
    reader.readNextStartElement();
    if (reader.isStartElement() && reader.name()=="message") {
        reader.readNextStartElement();
        if (reader.isStartElement() && reader.name()=="name") {
           *name=reader.readElementText();
            reader.readNextStartElement();
            if (reader.isStartElement() && reader.name()=="descr") {
                *descr=reader.readElementText();
            }
           return true;
        }
    }
    return false;
}


void myXML::delete_message() { //odstrani z dat hlavicku message,zustane scena k ulozeni
    QRegExp rex("<message.*message>");    
    data.replace(rex,"");
    QRegExp rex2("<message[^\n]*\n");
    data.replace(rex2,"");
}

// prida pozadavek na nacteni site
void myXML::add_req_load(QString name,QString version) {
    addMessageElReq(T_req_load);
    writer->writeTextElement("name",name);
    writer->writeTextElement("version",version);
    writer->writeEndElement();
}

// vrati jmeno souboru a verzi
bool  myXML::get_req_load(QString *name,QString *version) {
    QXmlStreamReader reader(data);
    reader.readNextStartElement();
    if (reader.isStartElement() && reader.name()=="message") {
        reader.readNextStartElement();
        if (reader.isStartElement() && reader.name()=="name") {
            *name=reader.readElementText();
            reader.readNextStartElement();
            if (reader.isStartElement() && reader.name()=="version") {
               *version=reader.readElementText();
                return true;
            }
        }
    }
    return false;
}

// prida pozadavek na zobrazeni ulozenych siti
void myXML::add_req_showNets(QString regex) {
    addMessageElReq(T_req_showNets);
    writer->writeTextElement("regexp",regex);
    writer->writeEndElement();
}


// vrati pozadavek na zobrazeni siti
bool myXML::get_req_showNets(QString *regex) {
    QXmlStreamReader reader(data);
    reader.readNextStartElement();
    if (reader.isStartElement() && reader.name()=="message") {
        reader.readNextStartElement();
        if (reader.isStartElement() && reader.name()=="regexp") {
           *regex=reader.readElementText();
           return true;
        }
    }
    return false;
}

// prida pozadavek na celou simulaci
void myXML::add_req_fullSim(QString max) {
    addMessageElReq(T_req_fullSim);
    writer->writeTextElement("max",max);
    writer->writeEndElement();
}
// vrati pozadavek na celou simulaci
bool myXML::get_req_fullSim(QString *max) {
    QXmlStreamReader reader(data);
    reader.readNextStartElement();
    if (reader.isStartElement() && reader.name()=="message") {
        reader.readNextStartElement();
        if (reader.isStartElement() && reader.name()=="max") {
           *max=reader.readElementText();
           return true;
        }
    }
    return false;
}

// prida pozadavek na simulaci jednoho kroku1,posle id prechodu
void myXML::add_req_stepSim(QString id) {
    addMessageElReq(T_req_stepSim);
    writer->writeTextElement("id",id);
    writer->writeEndElement();
}
bool myXML::get_req_stepSim(QString *id) {
    QXmlStreamReader reader(data);
    reader.readNextStartElement();
    if (reader.isStartElement() && reader.name()=="message") {
        reader.readNextStartElement();
        if (reader.isStartElement() && reader.name()=="id") {
           *id=reader.readElementText();
           return true;
        }
    }
    return false;
}

// prida odpoved na autentizaci
void myXML::add_rep_authen(bool result) {
    addMessageElRep(T_rep_authen);
    if (result) writer->writeTextElement("result","OK");
    else writer->writeTextElement("result","BAD");
    writer->writeEndElement();
}
// Posle klientovi zpravu o konci simulace
void  myXML::add_rep_endSim() {
    addMessageElRep(T_rep_endSim);
    writer->writeEndElement();
}

// prida odpoved na pridani noveho uzivatele
void myXML::add_rep_newUser(bool result) {
    addMessageElRep(T_rep_new);
    if (result) writer->writeTextElement("result","OK");
    else writer->writeTextElement("result","BAD");
    writer->writeEndElement();
}

//  prida odpoved na ulozeni site
void myXML::add_rep_save(bool result,QString version) {
    addMessageElRep(T_rep_save);
    if (result) writer->writeTextElement("result","OK");
    else writer->writeTextElement("result","BAD");
    writer->writeTextElement("version",version);
    writer->writeEndElement();
}

// vrati vysledek odpovedi
bool myXML::get_rep_save(QString *version) {
    QXmlStreamReader reader(data);
    reader.readNextStartElement();
    if (reader.isStartElement() && reader.name()=="message") {
        reader.readNextStartElement();
        if (reader.isStartElement() && reader.name()=="result") {
            if (reader.readElementText()=="OK")
                reader.readNextStartElement();
                if (reader.isStartElement() && reader.name()=="version") {
                    *version=reader.readElementText();
                    return true;
                }
        }
    }
    return false;
}
// vrati vysledek odpovedi
bool myXML::getResult() {
    QXmlStreamReader reader(data);
    reader.readNextStartElement();
    if (reader.isStartElement() && reader.name()=="message") {
        reader.readNextStartElement();
        if (reader.isStartElement() && reader.name()=="result") {
            if (reader.readElementText()=="OK") return true;
        }
    }
    return false;
}

// prida odpoved na nacteni site
void myXML::add_rep_load(bool result,QString net,QString log,QString version) {
    addMessageElRep(T_rep_load);
    if (result) writer->writeTextElement("result","OK");
    else writer->writeTextElement("result","BAD");
    writer->writeTextElement("log",log);
    writer->writeTextElement("version",version);
    writer->writeEndElement();
    // odstranime z site hlavicku, ktera uz tam je
    QString pom=net;
    pom = pom.section('>',1);
    data.append(pom);  // nakonec pripojime sit

}

// vrati log zobrazovany pri prohlizeni site a verzi
bool  myXML::get_rep_load(QString *simLog,QString *version) {
    QXmlStreamReader reader(data);
    reader.readNextStartElement();
    if (reader.isStartElement() && reader.name()=="message") {
        reader.readNextStartElement();
        if (reader.isStartElement() && reader.name()=="result") {
            if (reader.readElementText()=="OK") {
                reader.readNextStartElement();
                if (reader.isStartElement() && reader.name()=="log") {
                   *simLog=reader.readElementText();
                    reader.readNextStartElement();
                    if (reader.isStartElement() && reader.name()=="version") {
                        *version=reader.readElementText();
                        return true;
                    }
                }
            }
        }
    }
    return false;

}


// prida odpoved na zobrazeni ulozenych siti
void myXML::add_rep_showNets(QStringList list) {
    addMessageElRep(T_rep_showNets);
    int i;
    QString str;
    //vytvorime string ze seznamu souborou
    if (!list.isEmpty()) {
       for (i = 0; i < (list.size()-1);i+=1) {
           str.append(list.at(i));
           str.append(":");  // oddelovac
       }
       str.append(list.at(i)); // aby nakonci nebyl oddelovac
    }
    writer->writeTextElement("names",str);
    writer->writeEndElement();
}

// vrati retezec s ulozenymi sitemi
bool myXML::get_rep_showNets(QString *str) {
    QXmlStreamReader reader(data);
    reader.readNextStartElement();
    if (reader.isStartElement() && reader.name()=="message") {
        reader.readNextStartElement();
        if (reader.isStartElement() && reader.name()=="names") {
           *str=reader.readElementText();
           return true;
        }
    }
    return false;
}

// prida odpoved na celou simulaci
void myXML::add_rep_fullSim(QString net) {
    addMessageElRep(T_rep_fullSim);
    writer->writeEndElement();
    // odstranime z site hlavicku, ktera uz tam je
    QString pom=net;
    pom = pom.section('>',1);
    data.append(pom);  // nakonec pripojime sit
}

// prida odpoved na simulaci jednoho kroku1,posle prechody
void myXML::add_rep_stepSim(QString net) {
    addMessageElRep(T_rep_stepSim);
    writer->writeEndElement();
    // odstranime z site hlavicku, ktera uz tam je
    QString pom=net;
    pom = pom.section('>',1);
    data.append(pom);  // nakonec pripojime sit
}

//zmaze a znovu inicializuje data
void myXML::clear() {
    data="";
    writer->writeStartDocument();
}

// prida pozadavek na start simulace
 void myXML::add_req_startSim(QString name,QString version) {
     addMessageElReq(T_req_startSim);
     writer->writeTextElement("name",name);
     writer->writeTextElement("version",version);
     writer->writeEndElement();

 }
 // vrati pozadavek na start simulace
bool myXML::get_req_startSim(QString *name,QString *version) {
      QXmlStreamReader reader(data);
      reader.readNextStartElement();
      if (reader.isStartElement() && reader.name()=="message") {
          reader.readNextStartElement();
          if (reader.isStartElement() && reader.name()=="name") {
              *name=reader.readElementText();
              reader.readNextStartElement();
              if (reader.isStartElement() && reader.name()=="version") {
                 *version=reader.readElementText();
                  return true;
              }
          }
      }
      return false;
 }

// prida odpoved na start simulace
void myXML::add_rep_startSim(bool result,QString net) {
    addMessageElReq(T_req_startSim);
    if (result) writer->writeTextElement("result","OK");
    else writer->writeTextElement("result","BAD");
    writer->writeEndElement();
    // odstranime z site hlavicku, ktera uz tam je
    QString pom=net;
    pom = pom.section('>',1);
    data.append(pom);  // nakonec pripojime sit
}

// prida pozadavek na ukonceni simulace
void myXML::add_req_endSim() {
    addMessageElReq(T_req_endSim);
    writer->writeEndElement();
}

// funkce vytvori xml ze sceny
void myXML::createSceneXML(Lists *list) {
    QString id;
    // scena
    writer->writeStartElement("scene");

     // mista
     writer->writeStartElement("places");
     foreach (PetriNetPlaceCore *place,list->places) {
        writer->writeStartElement("place");
        writer->writeAttribute("id",QString::number(place->getID()));
        writer->writeAttribute("x",QString::number(place->getCoords().x()));
        writer->writeAttribute("y",QString::number(place->getCoords().y()));
        writer->writeCharacters(place->tokensIntToStr());   // zapise string tokenu
        writer->writeEndElement();
     }
     writer->writeEndElement();

     // prechody
     writer->writeStartElement("transitions");
     foreach (PetriNetTransitionCore *trans,list->transitions) {
       writer->writeStartElement("transition");
       writer->writeAttribute("id",QString::number(trans->getID()));
       writer->writeAttribute("x",QString::number(trans->getCoords().x()));
       writer->writeAttribute("y",QString::number(trans->getCoords().y()));
       writer->writeTextElement("guard",trans->getGuard());
       writer->writeTextElement("action",trans->getAction());
       if (trans->isDoable()) writer->writeTextElement("doable","1");
       else writer->writeTextElement("doable","0");
       writer->writeEndElement();
     }
     writer->writeEndElement();

     // hrany
     writer->writeStartElement("arcs");
     foreach (PetriNetArcCore *arc,list->arcs) {
       writer->writeStartElement("arc");
       writer->writeAttribute("idFrom",QString::number(arc->getIDFrom()));
       writer->writeAttribute("idTo",QString::number(arc->getIDTo()));
       writer->writeCharacters(arc->getVarOrConst()); //zapise promennou nebo konstantu u hrany
       writer->writeEndElement();
     }
     writer->writeEndElement();
    writer->writeEndElement();
}

// vytvori z formatu XML nove objekty-mista,hrany,prechody
bool myXML::createNetCore(Lists *list, int *maxID) {
    int _maxID = 1;
    
    QXmlStreamReader reader(data);
    int id,to,from;
    QPointF coord;
    QString tokens,guard,action,varConst,pom;
    bool doable;
    
    // hash mapa pro pozdejsi propojeni mist a prechodu hranami
    QMap<int, PetriNetNode *> map;
    PetriNetPlaceCore *placeCore;
    PetriNetTransitionCore *transitionCore;
    PetriNetArcCore *arcCore;

    //cela scena
    reader.readNextStartElement();
    if (!(reader.isStartElement() && reader.name()=="scene")) {
        return false;
    }

    // mista
    reader.readNextStartElement();
    if (!(reader.isStartElement() && reader.name().toString()=="places")) {
        return false;
    }
    while (reader.readNextStartElement() && reader.name()=="place" ) {

        id=reader.attributes().value("id").toString().toInt();
        coord.setX(reader.attributes().value("x").toString().toInt());
        coord.setY(reader.attributes().value("y").toString().toInt());
        tokens=reader.readElementText();

        if(id > _maxID) _maxID = id;
        
        // novy objekt jadra MISTA
        placeCore = new PetriNetPlaceCore(id, list, 0);
        placeCore->setCoords(coord);
        placeCore->tokensStrToInt(tokens);
        
        // pridani objektu do hash mapy
        map[id] = placeCore;        


    }
    reader.readNextStartElement(); // preskoceni end places
    
    // prechody
    if (!(reader.isStartElement() && reader.name()=="transitions")) {
        return false;
    }
    while (reader.readNextStartElement() && reader.name()=="transition" ) {

        id=reader.attributes().value("id").toString().toInt();
        coord.setX(reader.attributes().value("x").toString().toInt());
        coord.setY(reader.attributes().value("y").toString().toInt());

        if(id > _maxID) _maxID = id;
        
        reader.readNextStartElement();
        if (!(reader.isStartElement() && reader.name()=="guard")) {
            return false;
        }
        guard=reader.readElementText();
        reader.readNextStartElement();
        if (!(reader.isStartElement() && reader.name()=="action")) {
            return false;
        }
        action=reader.readElementText();
        reader.readNextStartElement();

        if (!(reader.isStartElement() && reader.name()=="doable")) {
            return false;
        }
        if (reader.readElementText()=="0") {doable=false;}
        else doable=true;
        // preskoceni end elementu transition
        reader.readNextStartElement();

        // novy objekt jadra PRECHODU
        transitionCore = new PetriNetTransitionCore(id, list, 0);
        transitionCore->setCoords(coord);
        transitionCore->setGuard(guard);
        transitionCore->setAction(action);
        transitionCore->setDoable(doable);
        
        // pridani objektu do hash mapy
        map[id] = transitionCore;        


    }
    reader.readNextStartElement(); // preskoceni end transitons
    
    // hrany
    if (!(reader.isStartElement() && reader.name()=="arcs")) {
        return false;
    }
    while (reader.readNextStartElement() && reader.name()=="arc") {

        from=reader.attributes().value("idFrom").toString().toInt();
        to=reader.attributes().value("idTo").toString().toInt();
        varConst=reader.readElementText();

        // novy objetk jadra HRANY
        arcCore = new PetriNetArcCore(list, 0, map[from], map[to]);
        arcCore->setVarOrConst(varConst);
        
        if(arcCore->getFrom()->getType() == PetriNetNode::E_TransitionCore) {

            static_cast<PetriNetTransitionCore *>(arcCore->getFrom())->appendArcOut(arcCore);
        } else 
        if(arcCore->getTo()->getType() == PetriNetNode::E_TransitionCore) {

            static_cast<PetriNetTransitionCore *>(arcCore->getTo())->appendArcIn(arcCore);
        }
        

    }
    
    if(maxID) *maxID = ++_maxID;
    return true;
}
