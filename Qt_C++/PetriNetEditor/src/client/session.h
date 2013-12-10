/** @file: session.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro ziskani a ulozeni informaci o sezeni
  */

#ifndef SESSION_H
#define SESSION_H

#include <QString>

/** Trida pro ziskani a ulozeni informaci o sezeni. 
 * Insance programu umoznuje pouze jedno sezeni (jeden uzivatel
 * a jedno pripojeni k serveru), ktere je sdileno vsemi
 * otevrenymi kartami */
class Session {
public:
    /** konstruktor. Implicitni server je loopback na portu 55555 */
    Session() {}

    // Nastaveni a ziskani privatnich dat.
    QString &getLogin() {return login;}
    void setLogin(QString &_login) {login = _login;}
    
    QString &getPassword() {return password;}
    void setPassword(QString &_password) {password= _password;}
    
    QString &getServer() {return server;}
    void setServer(QString &_server) {server = _server;}
    
    QString &getPort() {return port;}
    void setPort(QString &_port) {port = _port;}
    
private:
    QString login;
    QString password;
    QString server;
    QString port;
};

#endif // SESSION_H
