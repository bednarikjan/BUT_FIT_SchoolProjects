/** @file: parser.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro parsovani vyrazu a vyhodnocnovani prechodu
  */

#ifndef PARSER_H
#define PARSER_H

#include "../shared/petrinettransitioncore.h"
#include "../shared/petrinetarccore.h"
#include <QString>
#include <QMap>

// dopredna deklarace
class Expression;

/** Trida pro parsovani vyrazu a vyhodnocnovani prechodu */
class Parser {
public:
    /** Konstruktor */
    Parser(PetriNetTransitionCore * _trans): trans(_trans), neverDoable(false) {}
    /** Destruktor */
    ~Parser();    
    
    /** Inicialiauje parser - pripravi a naplni si vnitrni struktury, aby 
      * uz pak mohl pouze vyhodnocovat vyraz straze. */
    void init();
    /** Vyhodnoti straz. Rozhodne o proveditelnosti prechodu. */
    bool evalGuard();
    /** Vyhodnoti a provede vystupni akci */
    void outputAction();
    
    friend class Expression;    
    
private:
    /** Typ relacniho operatoru */
    enum operators {
      Less,     /**< \<  */
      LessEq,   /**< \<= */
      Greater,  /**< \>  */
      GreaterEq,/**< \>= */
      Equal,    /**< ==  */
      NotEqual  /**< !=  */
    };
    PetriNetTransitionCore *trans;  /**< Ukazatel na objekt PRECHOD */
    QMap<QString, int *> variables; /**< Mapa pro promenne u hran a navazane hodnoty. */
    QList<QList<PetriNetArcCore *> *> arcsStack; /**< Pseudozasobnik pro seznamy hran s promennymi. */
    QList<Expression *> exprList;   /**< Seznam binarnich relacich operaci straze */
    QList<int *> consts;            /**< Ukazatele na alok. misto pro konstanty ve strazi */
    bool neverDoable;               /**< Priznak rika, ze neni prechod nikdy proveditelny. */
    
    /** Zjisti, zda je string promenna*/
    bool isVar(QString str);
    
    /** Alokuje misto pro vsechny promenne vstupnich hran prechodu.
      * Vytvori zaznamy do hash mapy 'variables'. */
    void createArcVariables();
    /** Vytvori a naplni pseude-zasobnik seznamu vstupnich hran. V kazdem seznamu
      * jsou pouze ty ukazatele na hrany, ktere maji stejne pojmenovanou promennou. */
    void createPseudoStack();
    /** Parsuje podminku ve strazi. Tvori seznam struktur - jedna struktura pro
      * pro jednu relacni operaci nad dvema operandy */
    bool parseGuard();
    /** Zkontroluje, zda v mistech, ktere vedou do prechodu pres hranu s konstantou,
      * je potrebny token */
    bool checkConstArcs();
    /** Inicializuje navazani promennych */
    bool initVarBinding();
    /** Vyhodnoti seznam vyrazu (binarnich relaci) */
    bool evalExpressions();
    
    /** Pokusi se navazat hodnotu na promennou, jejiz jmeno je na vice hranach. */
    bool bindCommonVar(QList<PetriNetArcCore *> *list, int startIdx = 0);
    /** Pokusi se navazat hodnotu na promennou, ktera je unikatni pro vsechny vstupni hrany. */
    bool bindVar(QList<PetriNetArcCore *> *list, int startIdx = 0);
    /** Provede dalsi mozne navazani (system backtrackingu) */
    bool bindNext();
    
    /** Extrahuje z vyrazu operand */
    bool extractOperand(int **op, QString &expr);
    /** Extrahuje z vyrazu operator */
    void extractOperator(operators *oper, QString &expr);
    
    /** Vrati nazev vystupni promenne */
    QString getOutputVariable(QString var);
    /** Vyhodnoti vysledek vystupni akce */
    int getResult(QString expr);
    
    /** Aktualizuje vstupni mista */
    void updateInputPlaces();
    /** Aktualizuje vystupni mista */
    void updateOutputPlaces(QString &varName, int &value);
    
    /** Ziska nasledujici hodnotu vyrazu vystupni akce (z promenne nebo konstanty) */
    int getNextValue(QString &expr,int &i);
    /** Prepocita vysledek vystupni akce*/
    void updateResult(int &result, int value, bool addOrSub);
    /** Ziska nasledujici operator ve vyrazu vystupni akce */
    bool getNextOperator(QString &expr, int &i);
};

/** Struktura pro jednu relacni operaci. */
class Expression {
public:
    /** Konstruktor. Nuluje ukazatele na operandy */
    Expression(): op1(0), op2(0) {}
    
    Parser::operators oper;  /**< operator */
    int *op1;                /**< levy operand */
    int *op2;                /**< pravy operand */
};

#endif // PARSER_H
