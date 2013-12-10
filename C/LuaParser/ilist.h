/*
 * Implementace interpretu imperativniho jazyka IFJ2011
 *
 * Autori:
 *  xbedna45 - Bednarik Jan
 *  xblaha22 - Blaha Hynek
 *  xjanys00 - Janys Martin
 *  xmacha48 - Machacek Ondrej
 */

#ifndef ILIST_H
#define ILIST_H

#include <stdlib.h>

enum errs {
    E_OK = 0, // OK
    E_ALLOC, // chyba alokace
};

/* Vycet typu formatu argumentu prikazu read();` */
enum readFormatTypes {
    RF_NUM = 4, // "*n"         Nacte cislo.
    RF_EOL, // "*l"         Nacte vsechny znaky az po konec radku.
    RF_EOF, // "*a"         Nacte vsechny znaky az po EOF.
    RF_CHARS, // kladne_cislo Nacte dany pocet (kladne cislo) znaku.
};

/* Priznak instrukce I_RET, zda byla pouzita explicitne programatorem s danou
   navratovou hodnotu, ci implicitne (tedy se vrati nil) */
enum returnDefined {
    IMPL_RET,
    EXPL_RET,
};

/* Instrukcni sada jazyka ifj11 */
typedef enum {
    /* Zasobnik */
    I_PUSHV, // vloz hodnotu
    I_PUSHA, // vloz adresu
    I_PUSHL, // vloz literal
    I_ASGNV, // priradi hodnotu promene, na urcitem indexu
    I_ACTTOP, // srovna act s top
    I_CLR, // cisti mezivysledky v zaporne casti zasobniku (top-to-act)
    I_POP, // zrusi prvek na vrcholu zasobniku

    /* Aritmetika */
    I_ADD, /* soucet */
    I_SUB, /* odecitani */
    I_MUL, /* nasobeni */
    I_DIV, /* deleneni */
    I_MOD, /* modulo, pripadne rozsireni */
    I_POW, /* mocnina */
    I_CON, /* konkatenace */
    I_LEN, /* delka retezce */

    /* Relace/rovnost/logicke operace */
    I_EQU, /* == */
    I_NEQ, /* != */
    I_LES, /* < */
    I_GRT, /* > */
    I_LSE, /* <= */
    I_GRE, /* >= */

    /* Skoky */
    I_JMP, /* skok/goto */
    /* argument1 - adresa pro true
     * argument2 - adresa pro false 
     * result - vysledek vyrazu */
    I_CJMP, // Podmineny skok  
    I_CALL,
    I_RET,
    I_LBL, // TODO

    /* Funkce */
    I_WRT, /* zapis na stdout */
    I_READ, /* cteni ze stdin */
    I_SORT, /* trideni */
    I_TYPE, /* typ dat */
    I_SUBST, /* vraci podretezec */
    I_FIND, /* vyhledani podretezce */
} operation;

typedef enum {
    STACK,
    HEAP,
    NO_SEG,
} seg;

/* Struktura 3adresneho kodu */
typedef struct {
    operation operator; // definuje operaci
    void *argument1; // prvni argument operace
    void *argument2; // druhy argument operace
    void *result; // vysledek operace
} *pTAC, TAC;

/* struktura polozky v seznamu */
typedef struct item {
    TAC tac;
    struct item *next;
} iItem;

/* struktura listu s instrukcemi */
typedef struct {
    iItem *first; // ukazatel na prvni prvek
    iItem *last; // ukazatel na posledni prvek
    iItem *active; // ukazatel na aktivni prvek
} *pInstrList, instrList;

/* Funkce seznamu */
void listInit(pInstrList);
void listFree(pInstrList);
int listInsertLast(pInstrList, TAC);
void listFirst(pInstrList);
void listNext(pInstrList);
void listGoto(pInstrList, void *gotoInstr);
void *listGetPointerLast(pInstrList);
pTAC listGetInstruction(pInstrList);

#endif
