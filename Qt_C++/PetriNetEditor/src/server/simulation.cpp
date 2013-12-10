/** @file: simulation.cpp
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro rizeni simulace
  */

#include "simulation.h"
#include "parser.h"
#include <cassert>

// Provede inicializaci simulace
void Simulation::init(Lists *lists) {
    // Torba a inicializace parseru pro kazdy prechod
    foreach(PetriNetTransitionCore *trans, lists->transitions) {
        trans->parser = new Parser(trans);
        trans->parser->init();
    }
}

// Nastavi priznak proveditelnosti vsem proveditelnym prechodum
bool Simulation::findDoableTransitions(Lists *lists) {
    bool doableTransExists = false;
    
    foreach(PetriNetTransitionCore *trans, lists->transitions) {
        if(trans->parser->evalGuard()) { 
            doableTransExists = true;
            trans->setDoable(true);
        } else
            trans->setDoable(false);
    }
    
    return doableTransExists;
}

// Vyhodnoti a provede vystupni akci
void Simulation::step(Lists *lists, int idx) {
    PetriNetTransitionCore *transition = 0;
    
    // nalezeni prechodu vybraneho uzivatelem
    foreach(PetriNetTransitionCore *trans, lists->transitions) {
        if(trans->getID() == idx) {
            transition = trans;
            break;
        }
    }
    assert(transition != 0);
    
    transition->parser->outputAction();
}
