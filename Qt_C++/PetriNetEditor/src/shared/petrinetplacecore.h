/** @file: petrinetplacecore.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida obsahuje zakladni data a metody prvku mista
  */

#ifndef PETRINETPLACECORE_H
#define PETRINETPLACECORE_H

#include "petrinetnode.h"
#include "lists.h"
#include <QVector>
#include <iostream>

// dopredna dekalrace
class PetriNetPlaceGUI;
class Parser;

/** Zakladni data a metdoy prvku MISTO. Dedi od bazove tridy popsiujici obene UZEL. */
class PetriNetPlaceCore: public PetriNetNode{

public:
    /** Konstruktor.
      * Ulozi ukazatel na tento objekt do seznamu MIST. */
    PetriNetPlaceCore(int _id, Lists *_lists, PetriNetPlaceGUI *_placeGUI)
        : PetriNetNode(_id, _lists), placeGUI(_placeGUI) {
        setType(PetriNetNode::E_PlaceCore);
        lists->places.append(this);
        std::cout << "nove MISTO: " << id << std::endl;
        lists->printPlaces();
    }
    /** Destruktor.
      * Odstrani ukazatel sam na sebe ze seznamu MIST. */
    ~PetriNetPlaceCore() {lists->places.removeOne(this);}
    
    // TODO vratit retezec tokenu

    /** Nastavi vektor cisel z retezce tokenu */
    void tokensStrToInt(QString str);
    
    /** VRati retezec s cisly oddeleneymi carkou */
    QString tokensIntToStr();
    
    /** Ukazatel na graficky obal. Potrebny pri vystavbe site z XML v klientovi, 
      * protoze nejprve se tvori jadro a pak az graficky obal v druhem pruchodu */ 
    PetriNetPlaceGUI *placeGUI; 
    
    friend class Parser;
    
private:
    QVector<int> tokens;  /**< Vektor cisel zadnaych uzivatelem */
    int index;            /**< Index aktualne navazaneho tokenu na promennou */         
};

#endif // PETRINETPLACECORE_H
