/** @file: main.cpp
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Hlavni modul pro server
  */

#include "myserver.h"
#include <QCoreApplication>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    bool ok;
    if((argc != 3) || (a.arguments().at(1)!="-p"))
    {
        qDebug() << "Bad run, usage: ./server2012 -p PORT";
        return 1;
    }
    int port = a.arguments().at(2).toInt(&ok);
    if (!ok) {
        qDebug() << "Bad run, usage: ./server2012 -p PORT";
        return 1;
    }
    mainServer server(port);
    return a.exec();
}
