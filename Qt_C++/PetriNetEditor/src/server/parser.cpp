/** @file: parser.cpp
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro parsovani vyrazu a vyhodnocnovani prechodu
  */

#include "parser.h"
#include "../shared/petrinetplacecore.h"
#include <QDebug>
#include <QStringList>
#include <cassert>

// destruktor
Parser::~Parser() {    
    foreach(QList<PetriNetArcCore *> *list, arcsStack)
        delete list;
    
    foreach(int *variable, variables.values())
        delete variable;
    
    foreach(int *constant, consts)
        delete constant;
}

/* Inicialiauje parser - pripravi a naplni si vnitrni struktury, aby 
 * uz pak mohl pouze vyhodnocovat vyraz straze. 
 */
void Parser::init() {
    createArcVariables();
    createPseudoStack();
    parseGuard();
}

void Parser::createArcVariables() {
    foreach(PetriNetArcCore *arc, trans->arcsIn) {
        // na hrane je promenna
        if(!arc->getVarOrConst().isEmpty() && isVar(arc->getVarOrConst())) {
            // promenna jeste neni ulozena v mape
            if(!variables[arc->getVarOrConst()]) {
                variables[arc->getVarOrConst()] = new int;
            }
        }
    }
}


void Parser::createPseudoStack() {
    QMap<QString,QList<PetriNetArcCore *> *> map;
    
    foreach(PetriNetArcCore *arc, trans->arcsIn) {
        // na hrane je promenna
        if(!arc->getVarOrConst().isEmpty() && isVar(arc->getVarOrConst())) {
            // promenna jeste neni v zadnem seznamu
            if(!map[arc->getVarOrConst()]) {
                arcsStack.append(new QList<PetriNetArcCore *>);
                arcsStack.last()->append(arc);
                map[arc->getVarOrConst()] = arcsStack.last();
            // hrana se stejnym jmenem promenne, ktere uz mela jina hrana
            } else {
                map[arc->getVarOrConst()]->append(arc);
            }
        }
    }
}

bool Parser::parseGuard() {    
    QString guard = trans->getGuard();
    if(!guard.isEmpty()) {
        QStringList exprs = guard.split("&&");
        
        foreach(QString expr, exprs) {
            exprList.append(new Expression);
            if(!extractOperand(&(exprList.last()->op1), expr)) return false;
            extractOperator(&(exprList.last()->oper), expr);
            if(!extractOperand(&(exprList.last()->op2), expr)) return false;

        }
    }
    
    return true;
}

bool Parser::evalGuard() {
    bool doable = false;
    bool bound = false;
    
    if(!neverDoable) {        
        // kontrola, zda do prechodu vedou nejake hrany
        if(trans->arcsIn.length() == 0)
            neverDoable = true;
        else {
            if(checkConstArcs()) {

                if(initVarBinding()) {

                    bound = true;
                }

            }

             
            // promenne se podarilo navazat
            while(bound) {
                if(evalExpressions()) {doable = true; break;}

                bound = bindNext();
            }
        }
    }    
    
    return doable;
}

// Zkontroluje hrany s konstantou, zda maji jejich mista potrebny token.
bool Parser::checkConstArcs() {
    bool doable = true;
    PetriNetPlaceCore *placeCore;
    
    foreach(PetriNetArcCore *arc, trans->arcsIn) {
        placeCore = static_cast<PetriNetPlaceCore *>(arc->getFrom());
        if(!isVar(arc->getVarOrConst()) &&
           (arc->getVarOrConst().isEmpty() || 
            placeCore->tokens.indexOf(arc->getVarOrConst().toInt()) == -1))
        {

            doable = false;
        }
    }
    
    return doable;
}

// Inicializuje navazani promennych.
bool Parser::initVarBinding() {
    bool bound = true;
    
    foreach(QList<PetriNetArcCore *> *list, arcsStack) {
        // vice hran s promennou dnaeho jmena        
        if(list->length() > 1) {
            if(!bindCommonVar(list)) {
                bound = false;
                break;
            }
        } 
        // pouze jedna hrana s promennou daneho jmena
        else {
            if(!bindVar(list)) {
                bound = false;
                break;
            }                                                                   
        }
    }
    
    return bound;
}

// Provede dalsi mozne navazani (system backtrackingu)
bool Parser::bindNext() {
    bool bound = true;
    PetriNetPlaceCore *place;
    int idx;
    int i;
    
    for(i = 0; i < arcsStack.size(); i++) {
        place = static_cast<PetriNetPlaceCore *>(arcsStack[i]->first()->getFrom());
        idx = place->index + 1;
        // vice hran s promennou daneho jmena
        if(arcsStack[i]->size() > 1) {
            if(!bindCommonVar(arcsStack[i],idx))
                bindCommonVar(arcsStack[i]);
            else
                break;
        } 
        // pouze jedna hrana s promennou daneho jmena
        else {
            if(!bindVar(arcsStack[i],idx))
                bindVar(arcsStack[i]);
            else
                break;
        }
    }
    if((i != 0) && (i == arcsStack.size())) bound = false;
    
    return bound;
}

// Pokusi se navazat hodnotu na promennou, jejiz jmeno je na vice hranach.
bool Parser::bindCommonVar(QList<PetriNetArcCore *> *list, int startIdx) {
    bool succeeded = false;
    PetriNetPlaceCore *placeFirst, *placeOther;
    int token, index;
    int i, j;
    
    placeFirst = static_cast<PetriNetPlaceCore *>(list->first()->getFrom());
    
    for(i = startIdx; i < placeFirst->tokens.size(); i++) {
        token = placeFirst->tokens[i];
        for(j = 1; j < list->size(); j++) {
            placeOther = static_cast<PetriNetPlaceCore *>((*list)[j]->getFrom());
            if((index = placeOther->tokens.indexOf(token)) != -1)
                placeOther->index = index;
            else
                break;   
        }
        
        if(j == list->size()) {
            placeFirst->index = i;
            *(variables[list->first()->getVarOrConst()]) = placeFirst->tokens[i];
            succeeded = true;
            break;
        }
    }
    
    return succeeded;
}

// Pokusi se navazat hodnotu na promennou, ktera je unikatni pro vsechny vstupni hrany.
bool Parser::bindVar(QList<PetriNetArcCore *> *list, int startIdx) {
    bool succeeded = false;
    PetriNetPlaceCore *place;
    
    place = static_cast<PetriNetPlaceCore *>(list->first()->getFrom());
    
    if(startIdx < place->tokens.size()) {
        place->index = startIdx;
        *(variables[list->first()->getVarOrConst()]) = place->tokens[startIdx];
        succeeded = true;
    }
    
    return succeeded;
}

// Vyhodnoti seznam vyrazu (binarnich relaci)
bool Parser::evalExpressions() {
    bool doable = true;
    
    foreach(Expression *expr, exprList) {
        switch(expr->oper) {
            case Less:
                if(!(*(expr->op1) <  *(expr->op2))) doable = false;
                break;
            
            case LessEq:
                if(!(*(expr->op1) <= *(expr->op2))) doable = false;
                break;
                
            case Greater:
                if(!(*(expr->op1) >  *(expr->op2))) doable = false;                    
                break;
                
            case GreaterEq:
                if(!(*(expr->op1) >= *(expr->op2))) doable = false;                    
                break;
                
            case Equal:
                if(!(*(expr->op1) == *(expr->op2))) doable = false;
                break;
                
            case NotEqual:
                if(!(*(expr->op1) != *(expr->op2))) doable = false;
                break;
        }
        if(!doable) break;
    }  
    
    return doable;
}

// Extrahuje z vyrazu operand
bool Parser::extractOperand(int **op, QString &expr) {
    QString substr;
    int constant;
    int strLen = expr.length();
    int start, end;
    int i = 0;
    
    while(i < strLen && expr[i].isSpace()) i++;
    assert(i < strLen);
    start = i;
    
    // operand je konstanta
    if(expr[i].isNumber()) {
        while(i < strLen && expr[i].isNumber()) i++;
        end = i;
        
        substr = expr.mid(start, end - start);
        constant = substr.toInt();
        *op = new int(constant);
        consts.append(*op);

    } 
    // operand je promenna
    else {
        while(i < strLen && expr[i].isLetterOrNumber()) i++;
        end = i;
        
        substr = expr.mid(start, end - start);
        if(!variables[substr]) {
            neverDoable = true;

            return false;
        } else {
            *op = variables[substr];

        }
    }

    // uriznuti uz parsovane casti ze stringu
    expr = expr.remove(0, end);
    
    return true;    
}

// Extrahuje z vyrazu operator
void Parser::extractOperator(operators *oper, QString &expr) {
    int strLen = expr.length();
    int start, end = 0;
    int i = 0;
    
    while(i < strLen && expr[i].isSpace()) i++;
    assert(i < strLen);
    start = i;
    
    // vyber operace
    switch(expr[i].toAscii()) {
        case '<':
            if(expr[i+1] == '=') {*oper = LessEq; end = i + 2; }
            else {*oper = Less; end = i + 1; }
            break;
        
        case '>':
            if(expr[i+1] == '=') {*oper = GreaterEq; end = i + 2; }
            else {*oper = Greater; end = i + 1; }
            break;
        
        case '=':
            *oper = Equal; 
            end = i + 2;

            break;
        
        case '!':
            *oper = NotEqual; 
            end = i + 2;

            break;
        
        default:
            assert(end != 0);
            break;
    }
    
    // uriznuti uz parsovane casti ze stringu
    expr = expr.remove(0, end);
}

// Zjisti, zda je string promenna nebo konstanta.
bool Parser::isVar(QString str) {
    bool ok;
    str.toInt(&ok, 10);
    return ok ? false : true;
}

// Vyhodnoti a provede vystupni akci.
void Parser::outputAction() {    
    QString outputVariable;
    int result;
    //bool actionDoable;
    QStringList actionStr;
    
    if(!trans->action.isEmpty()) {
        actionStr = trans->action.split("=");    
        assert(actionStr.length() == 2);
        
        outputVariable = getOutputVariable(actionStr.first());
        result = getResult(actionStr.last());
    }
    
    // aktulizace vstupnich a vystupnich mist
    updateInputPlaces();
    updateOutputPlaces(outputVariable, result);
}

// Vrati nazev vystupni promenne.
QString Parser::getOutputVariable(QString var) {
    QString substr;
    int strLen = var.length();
    int start, end;
    int i = 0;
    
    while(i < strLen && var[i].isSpace()) i++;
    assert(i < strLen);
    start = i;
    
    while(i < strLen && var[i].isLetterOrNumber()) i++;
    end = i;
    
    substr = var.mid(start, end - start);

    
    return substr;
}

// Vyhodnoti vysledek vystupni akce
int Parser::getResult(QString expr) {
    int result = 0;
    int i = 0;
    int value;
    int exprLen = expr.size();
    bool addOrSub = true; // add -> true, sub -> false
    
    while(i < exprLen) {
        value = getNextValue(expr, i);

        updateResult(result, value, addOrSub);
        addOrSub = getNextOperator(expr, i);

    }
    
    return result;
}

// Ziska nasledujici hodnotu vyrazu vystupni akce (z promenne nebo konstanty)
int Parser::getNextValue(QString &expr, int &i) {
    int value;
    int exprLen = expr.size();
    QString substr;
    int start, end;
    
    // preskoci mezery
    while(i < expr.size() && expr[i].isSpace()) i++;
    assert(i < exprLen);
    start = i;
    
    if(expr[i].isNumber()) {
        i++;
        while(i < exprLen && expr[i].isDigit()) i++;
        end = i;
    } else {
        while(expr[i].isLetter() || expr[i].isDigit()) i++;
    }    
    end = i;
    
    substr = expr.mid(start, end - start);
        
    // promenna
    if(expr[start].isNumber()) {
        value = substr.toInt();
    }
    // konstanta
    else {
        if(!variables[substr])
            value = 0;
        else 
            value = (*(variables[substr]));
    }
    
    return value;
}

// Prepocita vysledek vystupni akce
void Parser::updateResult(int &result, int value, bool addOrSub) {
    result = addOrSub ? (result + value) : (result - value);
}

// Ziska nasledujici operator ve vyrazu vystupni akce
bool Parser::getNextOperator(QString &expr, int &i) {
    bool oper;
    
    // preskoci mezery
    while(i < expr.size() && expr[i].isSpace()) i++;
    
    if(i >= expr.size())
        return false;
    
    if(expr[i].toAscii() == '+') oper = true;
    else oper = false;
    i++;
    
    return oper;
}

// Aktualizuje vstupni mista
void Parser::updateInputPlaces() {
    PetriNetPlaceCore *place;
    QString varName;
    
    foreach(PetriNetArcCore *arc, trans->arcsIn) {
        place = static_cast<PetriNetPlaceCore *>(arc->getFrom());
        if(isVar(varName = arc->getVarOrConst()))
            place->tokens.remove(place->tokens.indexOf(*(variables[varName])));
        else
            place->tokens.remove(place->tokens.indexOf(arc->getVarOrConst().toInt()));
    }
}

// Aktualizuje vystupni mista
void Parser::updateOutputPlaces(QString &varName, int &value) {
    PetriNetPlaceCore *place;
    
    foreach(PetriNetArcCore *arc, trans->arcsOut) {
        place = static_cast<PetriNetPlaceCore *>(arc->getTo());
        if(isVar(arc->getVarOrConst())) {
            if(varName.compare(arc->getVarOrConst()) == 0)
                place->tokens.append(value);    
        } else {
            place->tokens.append(arc->getVarOrConst().toInt());
        }        
    }
}
