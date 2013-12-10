/** @file: myserver.h
  * @author Petr Konvalinka
  *
  * Trida pro praci se serverem
  */

#ifndef MYSERVER_H
#define MYSERVER_H

#include <QObject>
#include <QAbstractSocket>
#include <QByteArray>
#include <QHostAddress>
#include <QtNetwork>
#include <QDebug>
#include <QSettings>
#include <QFile>
#include <qmap.h>
#include <QDir>
#include <iostream>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QTime>
#include "../shared/lists.h"
#include "simulation.h"
#include "parser.h"

#define LOGINS_FILE "logins.txt" /**< Soubor s loginy a hesly. */
#define NET_DIR "saveNets/"  /**< Adresar pro ukladani siti */

class QTcpSocket;
class QTcpServer;

typedef QMap<QString, QString> Tpole; /**< Asociovane pole pro loginy a hesla. */

/** Trida pro server.
  */
class mainServer : public QObject
{
     Q_OBJECT
public:
    mainServer(int port); /**< Konstruktor. */
    void zk();
    QString rawdata; /**< Prichozi data. */
private:
     QTcpServer* server;   /**< Socket pro komunikaci.*/
     Lists lists;          /**< Seznamy mist, prechodu a hran */
     Simulation simul;     /**< Trida s metodami pro rizeni simulace. */ 

     bool findUser(QString login,QString heslo); /**< Nalezeni uzivatele a porovnani hesla. */
     bool addUser(QString login,QString heslo); /**< Pridani noveho uzivatele. */
     bool saveNet(QString net,QString name, QString *version,QString description); /**< Ulozeni site. */
     bool loadNet(QString *net,QString name,QString version, QString *log); /**< Nacteni site. */
     QStringList GetNets(QString regex); /**< Nacteni seznamu ulozenych siti,podle klicoveho slova */
     bool findLogin(QString login); /**< Nalezeni uzivatele */
     QString author;   /**< Aktualni uzivatel */
     void write(QTcpSocket* socket,QString data); /**< Odesle zpravu klientovi. */
     void appendSim(QString name,QString version); /**< Prida vypis o simulaci do logoveho souboru */
     /** Smaze sit - uvolni pamet */
     void removeNet();
     /** Vybere proveditelny prechod */
     int chooseDoableTransition();

private slots:   
    void snewConnection(); /**< Nove prichozi spojeni. */
    void sreply();         /**< Prichozi odpoved enbo zprava od klienta. */
    void sdisconnected();  /**< Odpojeni od klienta. */

};

#endif // MYSERVER_H
