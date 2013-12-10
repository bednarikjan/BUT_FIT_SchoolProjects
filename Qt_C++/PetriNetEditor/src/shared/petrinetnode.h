/** @file: petrinetnode.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Bazova trida pro uzel petriho site
  */

#ifndef PETRINETNODE_H
#define PETRINETNODE_H

#include "lists.h"
#include <QPointF>

//class Lists;

/** Bazova trida pro uzel petriho site (pouze pro sit, netyka se grfickeho
  obalu). Dedi od ni PetriNetPlaceCore a PetriNetTransitionCore */
class PetriNetNode {    

public:
    /** konstruktor */
    PetriNetNode(int _id, Lists *_lists): id(_id), lists(_lists) {}
    
    /** Typ objektu jadra Petriho site */
    enum E_PetriNetCoreType {
        E_PlaceCore,
        E_TransitionCore
    };
    
    /** Vraceni typu */
    int getType() {return type;}
    /** Nastaveni typu */
    void setType(E_PetriNetCoreType _type) {type = _type;}
    /** Vraceni id */
    int getID() {return id;}
    /** Nastaveni ID */
    void setID(int _id) {id = _id;}
    /** Vraceni souradnic ve scene*/
    QPointF getCoords() {return coords;}
    /** Nastaveni souradnic ve scene*/
    void setCoords(QPointF _coords) {coords = _coords;}
    
protected:
    int id;                       /**< Ciselny identifikator mista pro ulozeni do XML */
    Lists *lists;                 /**< Ukazatel na objekt se seznamy */
    QPointF coords;               /**< Souradnice ve scene */
    E_PetriNetCoreType type;      /**< typ objektu */
    
};

#endif // PETRINETNODE_H
