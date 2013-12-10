/** @file: list.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro praci se seznamy ukazatelu na objekty mista,hrany,prechody
  */

#ifndef LISTS_H
#define LISTS_H

//#include "petrinettransition.h"
//#include <iostream>
#include <QList>

class PetriNetPlaceCore;
class PetriNetArcCore;
class PetriNetTransitionCore;

/** Trida nese tri seznamy ukazatelu na mista, prechody a hrany.
  * Tridu pouziva klient i server stejnym zpusobem pro souhrnne
  * ulozeni ukazatelu na vsechny objekty dane site. */
class Lists {
public:
    QList<PetriNetPlaceCore *>      places;      /**< mista */
    QList<PetriNetArcCore *>        arcs;        /**< hrany */
    QList<PetriNetTransitionCore *> transitions; /**< prechody */
    
    // DEBUG
    // vypis prvku seznamu MIST
    void printPlaces();
    void printTransitions();
    void printArcs();
};

#endif // LISTS_H
