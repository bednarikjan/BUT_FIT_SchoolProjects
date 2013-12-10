/** @file: simulation.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro rizeni simulace
  */

#ifndef SIMULATION_H
#define SIMULATION_H

#include "../shared/lists.h"

/** Trida pro rizeni simulaci */
class Simulation {
public:
    /** Konstruktor */
    Simulation() {}
    
    /** Provede inicializaci simulace */
    void init(Lists *lists);
    /** Provede jeden krok simulace */
    bool findDoableTransitions(Lists *lists);
    /** Vyhodnoti a provede vystupni akci */
    void step(Lists *lists, int idx);
};


#endif // SIMULATION_H
