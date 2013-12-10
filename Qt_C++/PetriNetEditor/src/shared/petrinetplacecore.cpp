/** @file: petrinetplacecore.cpp
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida obsahuje zakladni data a metody prvku mista
  */

#include "petrinetplacecore.h"
#include <QString>
#include <QStringList>

// Metoda jadra - prevede retezec tokenu na vektor cisel
void PetriNetPlaceCore::tokensStrToInt(QString str) {
    int converted;
    bool ok;
    
    tokens.clear();
    QStringList list = str.split(",");
    
    foreach(QString s, list) {
        converted = s.toInt(&ok, 10);
        if(ok) 
            tokens.append(converted);
    }
    

}

// Metoda jadra - prevede vektor intu na retezec
QString PetriNetPlaceCore::tokensIntToStr() {
    QString result("");
    
    foreach(int i, tokens) {
        result += QString("%1,").arg(i);
    }    

    
    return result;
}
