/** @file: myXML.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro praci s formatem XML
  */

#ifndef MYXML_H
#define MYXML_H

#include <iostream>
#include <QDebug>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <assert.h>
#include <QGraphicsScene>
#include <QMenu>
#include <QGraphicsLineItem>
#include <QFile>

#include "lists.h"
#include "petrinetplacecore.h"
#include "petrinetarccore.h"
#include "petrinettransitioncore.h"
#include "petrinetnode.h"



enum TtypeRequest {   /**< typ zpravy. */
    T_bad_req,     /**< spatny pozadavek */
    T_req_authen, /**< pozadavek na autentizaci. */
    T_req_newUser,/**< pozadavek na pridani noveho uzivatele. */
    T_req_save,   /**< pozadavek na ulozeni site. */
    T_req_load,   /**< pozadavek na nacteni site. */
    T_req_showNets,/**< pozadavek na zobrazeni ulozenych siti. */
    T_req_startSim,/**< pozadavek na zapocati simulace */
    T_req_fullSim, /**< pozadavek na celou simulaci. */
    T_req_stepSim, /**< pozadavek na simulaci jednoho kroku1,posle sit. */
    T_req_endSim   /**< pozadavek na ukonceni simulace */
};
enum TtypeReply {
    T_bad_rep,     /**< spatna odpoved. */
    T_rep_authen,  /**< odpoved na autentizaci. */
    T_rep_new,     /**< odpoved na autentizaci. */
    T_rep_save,    /**< odpoved na ulozeni site. */
    T_rep_load,    /**< odpoved na nacteni site. */
    T_rep_showNets,/**< odpoved na zobrazeni ulozenych siti. */
    T_rep_startSim,/**< odopoved na zapocati simulace */
    T_rep_fullSim, /**< odpoved na celou simulaci. */
    T_rep_stepSim, /**< odpoved na simulaci jednoho kroku */
    T_rep_endSim  /**< server posle klientovi info o ukonceni simulace */
};

/** Trida pro zpravy ve formatu XML
  */
class myXML {

 public:
     myXML(); /**< konstruktor,do dat vlozi hlavicku XML. */


     void createSceneXML(Lists *list); /**< Vytvori format xml ze zadane sceny,scena jako parametr. */
     bool createNetCore(Lists *list, int *maxID = 0);  /**< Vytvori z foramu xml nove jadro. */

     QString data; /**< Data ve kterych je ulozen format xml. */

     TtypeRequest getRequest(); /**< vrati typ pozadavku. */
     TtypeReply getReply(); /**< vrati typ odpovedi. */
     bool getResult();       /**< vrati vysledek. */
     void clear();      /**< vycisti xml a znovu inicializuje. */
     void delete_message(); /** <odstrani ze zpravy hlavicku message,zustane scena k ulozeni. */

     void add_req_authen(QString login,QString passwd);/**< prida pozadavek na autentizaci. */
     void add_req_newUser(QString login,QString passwd);/**< prida pozadavek na pridani noveho uzivatele. */
     void add_req_save(Lists *list,QString name,QString description); /**< prida pozadavek na ulozeni site. */
     void add_req_load(QString name, QString version);  /**< prida pozadavek na nacteni site. */
     void add_req_showNets(QString regex);/**< prida pozadavek na zobrazeni ulozenych siti. */
     void add_req_fullSim(QString max);/**< prida pozadavek na celou simulaci */
     void add_req_stepSim(QString id);/**< prida pozadavek na simulaci jednoho kroku1,posle sit */
     void add_req_startSim(QString name,QString version);/**< prida pozadavek na start simulace. */
     void add_req_endSim(); /**< pozadavek na ukonceni simulace. */
     void add_rep_authen(bool result);  /**< prida odpoved na autentizaci. */
     void add_rep_newUser(bool result); /**< prida odpoved na pridani noveho uzivatele. */
     void add_rep_save(bool result, QString version);   /**<  prida odpoved na ulozeni site. */
     void add_rep_load(bool result, QString net,QString log, QString version);/**< prida odpoved na nacteni site. */
     void add_rep_showNets(QStringList list);/**< prida odpoved na zobrazeni ulozenych siti. */
     void add_rep_startSim(bool result,QString net); /**< prida odpoved na start simulace. */
     void add_rep_fullSim(QString net); /**< prida odpoved na celou simulaci. */
     void add_rep_stepSim(QString net);/**< prida odpoved na simulaci jednoho kroku. */
     void add_rep_endSim(); /**< Posle klientovi zpravu o konci simulace */

     bool get_req_authenNew(QString *login,QString *passwd);/**< vrati pozadavek na autentizaci. */
     bool get_req_save(QString *name,QString *descr);  /**< vrati pozadavek na ulozeni site. */
     bool get_req_load(QString *name,QString *version);   /**< vrati pozadavek na nacteni site. */
     bool get_req_showNets(QString *regex);/**< vrati pozadavek na zobrazeni ulozenych siti. */
     bool get_req_startSim(QString *name,QString *version); /**< vrati pozadavek na start simulace. */
     bool get_req_fullSim(QString *max);/**< vrati pozadavek na celou simulaci. */
     bool get_req_stepSim(QString *id);/**< vrati pozadavek na simulaci jednoho kroku1,posle sit. */
     bool get_rep_save(QString *version);   /**<  vrati odpoved na ulozeni site. */
     bool get_rep_load(QString *simLog, QString *version); /**< vrati odpoved na nacteni site. */
     bool get_rep_showNets(QString *str);/**< vrati odpoved na zobrazeni ulozenych siti. */
     bool get_rep_fullSim();  // vrati odpoved na celou simulaci
     bool get_rep_stepSim(); //vrati odpoved na simulaci jednoho kroku1,posle prechody
     bool result(); /** <vrati vysledek operace. */

 private:
    void addMessageElReq(TtypeRequest type); /** <Prida hlavicku z typem pro pozadavky. */
    void addMessageElRep(TtypeReply type); /** <Prida hlavivku s typem pro odpovedi. */
    QXmlStreamWriter *writer;   /** < Ukazatel na objekt, ktery zapisuje format XML. */
 };
#endif // MYXML_H
