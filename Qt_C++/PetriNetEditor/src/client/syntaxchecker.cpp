/** @file: syntaxchecker.cpp
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro parsovani vstupnich udaju mist, hran, prechodu
  */

#include "syntaxchecker.h"
#include <QRegExp>

// Zkontroluje spravnost syntaxe promenne nebo konstanty HRANY.
bool SyntaxChecker::checkArcSyntax(QString *varOrConst) {
    QRegExp space("^\\s*$");
    if (space.exactMatch(*varOrConst)) {
        varOrConst->clear();
        return true;
    }
    QRegExp reg("\\s*(([0-9-][0-9]*)|([a-zA-Z][a-zA-Z0-9]*))\\s*");
    if (reg.exactMatch(*varOrConst)) {
          *varOrConst=reg.cap(1);
          return true;
    }
    else return false;
}

// Zkontroluje spravnost syntaxe straze PRECHODU.
bool SyntaxChecker::checkGuardSyntax(QString *guard) {
    QRegExp space("^\\s*$");
    if (space.exactMatch(*guard)) {
        guard->clear();
        return true;
    }
    QRegExp reg("\\s*([a-zA-Z][a-zA-Z0-9]*)\\s*(<|<=|>|>=|==|!=)\\s*(([a-zA-Z][a-zA-Z0-9]*)|([0-9-][0-9]*))(\\s*&&\\s*([a-zA-Z][a-zA-Z0-9]*)\\s*(<|<=|>|>=|==|!=)\\s*(([a-zA-Z][a-zA-Z0-9]*)|([0-9-][0-9]*)))*\\s*");

    if (reg.exactMatch(*guard)) {
        return true;
    }
    else return false;
}

// Zkontroluje spravnost syntaxe vystupni akce PRECHODU.
bool SyntaxChecker::checkActionSyntax(QString *action) {
    QRegExp space("^\\s*$");
    if (space.exactMatch(*action)) {
        action->clear();
        return true;
    }
    QRegExp reg("\\s*([a-zA-Z][a-zA-Z0-9]*)\\s*=\\s*(([a-zA-Z][a-zA-Z0-9]*)|([1-9-][0-9]*))(\\s*[+-]\\s*(([a-zA-Z][a-zA-Z0-9]*)|([1-9-][0-9]*)))*\\s*");
    if (reg.exactMatch(*action)) {
        action->append(" ");
          return true;
    }
    else return false;
}
