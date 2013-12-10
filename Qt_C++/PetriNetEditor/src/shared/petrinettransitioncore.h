/** @file: petrinettransition.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida obsahuje zakladni data a metody prvku prechod
  */

#ifndef PETRINETTRANSITIONCORE_H
#define PETRINETTRANSITIONCORE_H

#include "petrinetnode.h"
#include "lists.h"
#include <iostream>
#include <QString>

// dopredna dekalrace
class PetriNetTransitionGUI;
class Parser;

/** Zakladni data a metdoy prvku PRECHOD */
class PetriNetTransitionCore: public PetriNetNode {

public:
    /** Konstruktor.
      * Ulozi ukazatel na tento objekt do seznamu PRECHODU. */
    PetriNetTransitionCore(int _id, Lists *_lists, PetriNetTransitionGUI *_transGUI)
        : PetriNetNode(_id, _lists), transGUI(_transGUI), 
          parser(0), doable(false) {
        setType(PetriNetNode::E_TransitionCore);
        lists->transitions.append(this);
        std::cout << "novy PRECHOD: " << id << std::endl;
        lists->printTransitions();
    }
    /** Destruktor.
      * Odstrani ukazatel sam na sebe ze seznamu MIST. */
    ~PetriNetTransitionCore() {lists->transitions.removeOne(this);}
    
    /** Vrati guard */
    QString getGuard() {return guard;}
    /** Nastavi guard */
    void setGuard(QString _guard) {guard = _guard;}
    /** Vrati akci */
    QString getAction() {return action;}
    /** Nastavi akci */
    void setAction(QString _action) {action = _action;}
    /** Vrati priznak proveditoelnosti */
    bool isDoable() {return doable;}
    /** Nastavi priznak proveditelnosti */
    void setDoable(bool _doable) {doable = _doable;}
    
    /** Ulozi hranu do seznamu vystupnich hran */
    void appendArcOut(PetriNetArcCore *arcCore) {arcsOut.append(arcCore);}
    /** Ulozi hranu do seznamu vstupnich hran */
    void appendArcIn(PetriNetArcCore *arcCore) {arcsIn.append(arcCore);}
    
    /** Ukazatel na graficky obal. Potrebny pri vystavbe site z XML v klientovi, 
      * protoze nejprve se tvori jadro a pak az graficky obal v druhem pruchodu */ 
    PetriNetTransitionGUI *transGUI; 
    /** Parser daneho prechodu - pouziva se az pri siumlaci. */    
    Parser *parser;
    friend class Parser;
    
private:
    QString guard;  /**< Straz prechodu */
    QString action; /**< Vystupni akce */
    bool doable;    /**< Priznak proveditelnosti prechodu */
    QList<PetriNetArcCore *> arcsOut; /**< Seznam hran vedoucich Z prechodu */
    QList<PetriNetArcCore *> arcsIn;  /**< Seznam hran vedoucich DO prechodu */    
};

#endif // PETRINETTRANSITIONCORE_H
