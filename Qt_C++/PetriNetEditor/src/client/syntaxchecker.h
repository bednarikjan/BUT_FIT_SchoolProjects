/** @file: syntaxchecker.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro parsovani vstupnich udaju mist, hran, prechodu
  */

#ifndef SYNTAXCHECKER_H
#define SYNTAXCHECKER_H

#include <QString>

class SyntaxChecker {
public:    
    /** Konstruktor */
    SyntaxChecker() {}
    
    /** Zkontroluje spravnost syntaxe straze PRECHODU. */
    bool checkGuardSyntax(QString *guard);
    /** Zkontroluje spravnost syntaxe vystupni akce PRECHODU. */
    bool checkActionSyntax(QString *action);
    /** Zkontroluje spravnost syntaxe promenne nebo konstanty HRANY. */
    bool checkArcSyntax(QString *varOrConst);
};

#endif // SYNTAXCHECKER_H
