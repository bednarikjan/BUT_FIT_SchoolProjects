/** @file: main.cpp
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Hlavni modul klienta
  */

#include <QApplication>
#include "appwindow.h"
#include "../shared/publictypes.h"

// Hlavni funkce programu
int main(int argc, char **argv) {
    Q_INIT_RESOURCE(petrinet);
    
    QApplication app(argc, argv);

    AppWindow appwindow;
    appwindow.setGeometry(APP_X, APP_Y, APP_WIDTH, APP_HEIGHT);
    appwindow.show();
    
    int execRet;
    
    try { 
        execRet = app.exec();
    } catch (const std::bad_alloc &) {
        return 1;
    }
    
    return execRet;
}
