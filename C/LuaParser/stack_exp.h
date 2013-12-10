/*
 * Implementace interpretu imperativniho jazyka IFJ2011
 *
 * Autori:
 *  xbedna45 - Bednarik Jan
 *  xblaha22 - Blaha Hynek
 *  xjanys00 - Janys Martin
 *  xmacha48 - Machacek Ondrej
 */

#ifndef STACK_EXP_H
#define STACK_EXP_H

#include "scaner.h"
#include "ial.h"
#include <stdio.h>
#include "error.h"

#define STACK_EXP_ERROR 1
#define STACK_EXP_SUCCESS 0

#define STACK_EXP_SIZE 4

typedef struct {
    Ttoken* stackExp; // Zasobnik tokenu
    int top; // vrchol zasobniku tokenu
    int allocSize; // velikost alokovane pameti
} TstackExp;

int stackExpInit(TstackExp* se);
void stackExpDestroy(TstackExp* se);
int stackExpResize(TstackExp* se);
void stackExpFree(TstackExp* se);
int stackExpPush(TstackExp* se, Ttoken global);
void stackExpMultiplePop(TstackExp* se, int count);
void stackExpInfo(TstackExp *se);
Ttoken *stackGetLastTerminal(TstackExp* se);
Ttoken stackExpTop(TstackExp* se);
char* condition2Str(int);
void stackExpTopToStart(TstackExp* se);


#endif

