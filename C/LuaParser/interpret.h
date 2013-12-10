/*
 * Implementace interpretu imperativniho jazyka IFJ2011
 *
 * Autori:
 *  xbedna45 - Bednarik Jan
 *  xblaha22 - Blaha Hynek
 *  xjanys00 - Janys Martin
 *  xmacha48 - Machacek Ondrej
 */

#ifndef IINTERPRET_H
#define IINTERPRET_H

#include "ilist.h"

#define EOL -2

int aritmetika(Tstack *stack, pTAC i);
int cmp(Tstack *stack, pTAC);
double count_num(double v1, double v2, operation op);
int readStringNum(Tvariable *val, int num);
int readStringEOL(Tvariable *val);
int readStringEOF(Tvariable *val);
int readNum(Tvariable *v);
int interpret(pInstrList);
int interpret_(pInstrList iList, Tstack *stack);

/* ------ TODO:PRESUNOUT ------- */
enum err {
    EOK = 0,
    ETYPE = -1, // chyba interpertace, nekompatiblini operatory pro danou operaci
    EALLOC = -2, // chyba pri realokaci zasobniku
    EINVNUM = -3, // neplatne cislo, pri nacitani read
    ENUMBER = -4, // chyba pri nacitani cisla
    EREAD = -5, // chyba pri nacitani ze stdin
    EWRITE = -6, // chyba pri vypisouvani na stdout
    EINVARG = -7, // predan neplatny datovy typ pro danou operaci
};

/* ----------------------------- */

enum e_types {
    IC_STR,
    IC_BOOL,
    IC_NUM,
    IC_NIL,
};

#endif
