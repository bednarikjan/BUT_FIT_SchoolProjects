/** @file: petrinetarccore.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida obsahuje zakladni data a metody prvku hrana
  */

#ifndef PETRINETARCCORE_H
#define PETRINETARCCORE_H

#include "petrinetnode.h"
#include "lists.h"
#include <QString>

// dopredna dekalrace
class PetriNetArcGUI;

/** Zakladni data a metdoy prvku HRANA */
class PetriNetArcCore {

public:
    /** Konstruktor.
      * Ulozi ukazatel na tento objekt do seznamu HRAN. */
    PetriNetArcCore(Lists *_lists, PetriNetArcGUI *_arcGUI, PetriNetNode *_from, PetriNetNode *_to)
        : arcGUI(_arcGUI), lists(_lists), from(_from), to(_to) {
        lists->arcs.append(this);

        lists->printArcs();
    }
    /** Destruktor.
      * Odstrani ukazatel sam na sebe ze seznamu MIST. */
    ~PetriNetArcCore() {lists->arcs.removeOne(this);}
    
    /** Vrati promennou nebo konstantu */    
    QString getVarOrConst() {return varOrConst;}
    /** Nastavi promennou nebo konstantu */    
    void setVarOrConst(QString _varOrConst) {varOrConst = _varOrConst;}
    
    /** Ukazatel na graficky obal. Potrebny pri vystavbe site z XML v klientovi, 
      * protoze nejprve se tvori jadro a pak az graficky obal v druhem pruchodu */ 
    PetriNetArcGUI *arcGUI; 
    
    /** Vrati ID od */
    int getIDFrom() {return from->getID();}
    /** Vrati ID do */
    int getIDTo() {return to->getID();}
    
    PetriNetNode *getFrom() {return from;}
    PetriNetNode *getTo() {return to;}
    
private:
    Lists *lists;       /**< Ukazatel na objekt se seznamy */
    QString varOrConst; /**< Promenna nebo konstanta zadana na hrane */   
    PetriNetNode *from;
    PetriNetNode *to;
};

#endif // PETRINETARCCORE_H
