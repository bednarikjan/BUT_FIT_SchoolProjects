/*
 * Implementace interpretu imperativniho jazyka IFJ2011
 *
 * Autori:
 *  xbedna45 - Bednarik Jan
 *  xblaha22 - Blaha Hynek
 *  xjanys00 - Janys Martin
 *  xmacha48 - Machacek Ondrej
 */

#ifndef STACK_H
#define STACK_H

#include <string.h>
#include <stdbool.h> 
#include <stdlib.h>
#include <stdio.h>
#include "ilist.h"
#include "str.h"

#define STACK_ERROR 1
#define STACK_SUCCESS 0

#define PROG_STACK_SIZE 1024

typedef enum idTypes {
    FUNC,
    BUILT_IN_FUNC,
    NUM_TYPE,
    STR_TYPE,
    BOOL_TYPE,
    NIL_TYPE //NIL je token
} VarType;

typedef union {
    double num;
    string str;
    bool boolean;
} UvariableData;

typedef struct {
    VarType type; //literal type
    UvariableData data;
} Tvariable;

//na zasobniku jsou adresy do inst. pasky(nealokujise/nedealokuji se)
//nebo promenne ktere je nutne predavat hodnotou

typedef union {
    Tvariable* variable;
    iItem* instruction;
} UstackItem;

typedef struct {
    UstackItem* stack;
    unsigned int act; //"base pointer" 
    unsigned int top; //vrchol
    unsigned int allocSize; // velikost alokovane pameti
} Tstack;

int stackInit(Tstack* s);
void stackFree(Tstack* s);

/** stackPrepareMem
 * posune vrchol zasobniku o n a inicialuzuje na NULL
 * pouziva se pri skoku do funkce (n = pocet loc. var)
 */
int stackPrepareMem(Tstack *s, unsigned int n);
int stackGrowUp(Tstack *s, unsigned int reqSize);




/** stacktPusthAddress
 * vlozi jendu adresu 
 * pouziva se pri vlozeni navratove adresy na zasobnik
 */
int stackPushAddress(Tstack* s, iItem *i);
bool stackEmpty(Tstack *s);
Tvariable *stackTopAndPop(Tstack* s);
Tvariable *stackTop(Tstack* s);
void stackPop(Tstack* s);
/** stackGetVal, stackGetAdr
 * vypocita index do pole a vrati jeho obsah od ACT!
 */
iItem* stackGetAdr(Tstack *s);


/** stackDisposeVariables
 * zrusi(uvolni) n pocet promennych na zasobniku
 * pouziva se pri navratu 
 */
void stackDisposeVariables(Tstack* s, int count);

/** stackDisposeAddress
 * polozka = NULL, neuvolnuje nesmi!
 * pouziva se pri navratu po nacteni a po uvolneni promennych protoze se jedna o POP bez dealokace musi byt na vrcholu
 */
void stackDisposeAddress(Tstack* s);
void actToTop(Tstack *s);
int getStackIndex(Tstack* s, int i);







int stackAlignParams(Tstack *s, int n);
void stackPrint(Tstack *s, unsigned int n);



// Inline funkce
// ==============

//uvolneni polozkyv poli na indexu i

inline void freeVar(Tstack *s, int i) {

  if (s->stack[i].variable != NULL) {
    if (s->stack[i].variable->type == STR_TYPE) {
      free(s->stack[i].variable->data.str.str);
    }
    free(s->stack[i].variable); // dealokace stacku od topu po hranici
    s->stack[i].variable = NULL;
  }
}

/** toToAct
 * posune to na act
 * a dealokuje pomocny promenny
 * pouziva se po dokonceni vyrazu
 */
inline void stackTopToAct(Tstack *s) {
  //dealokace pri ukonceni prace s pomocnymi promennymi
  for (unsigned int i = s->top; i >= s->act; i--) {
    freeVar(s, i); //uvolneni pomocnych promennych na (top, act>
  }
  s->top = s->act;
}

/** actToTop
 * posune act(BP) na vrchol
 */
inline void stackActToTop(Tstack *s) {
  s->act = s->top;
}

inline Tvariable* stackGetVal(Tstack *s, int i) {
  return s->stack[s->act - i].variable;
}

/** stackPush
 * vlozi jednu promennou na zasobnik
 * pouziva se k vlozeni paramentru pri volani fce
 */
inline int stackPush(Tstack* s, Tvariable *v) {

  if (s->top + 1 >= s->allocSize) { // overlimit
    if (stackGrowUp(s, s->top + 1) == STACK_ERROR) {
      return STACK_ERROR;
    }
  }

  if (v != NULL) {//nevklada se null
    if (s->stack[s->top].variable != NULL) {
      freeVar(s, s->top); //uvolnime dosavadni obsah
    }
    if ((s->stack[s->top].variable = malloc(sizeof (Tvariable))) == NULL) {
      return STACK_ERROR;
    }

    //duplikace obsahu
    s->stack[s->top].variable->type = v->type;
    s->stack[s->top].variable->data = v->data;

    if (v->type == STR_TYPE) {
      char *tmp = malloc(sizeof (char) * (v->data.str.allocSize));
      if (tmp == NULL) {
        return STACK_ERROR;
      }
      strcpy(tmp, v->data.str.str);
      s->stack[s->top].variable->data.str.str = tmp;
    }
    else if (v->type == NUM_TYPE) {
      s->stack[s->top].variable->data.num = v->data.num;
    }
  }
  s->top++;

  return STACK_SUCCESS;
}


inline int stackSetVal(Tstack *s, Tvariable *v, int i) {

  int workIndex = s->act - i;

  if (i < 0) { //indexujem pod zasobik potrebujem overit jestli je dost velky
    unsigned int reqIndex = s->act - i;
    if (reqIndex >= s->allocSize) {
      if (stackGrowUp(s, reqIndex) == STACK_ERROR) {
        return STACK_ERROR;
      }
      if (s->top - i > s->top) {
        s->top = s->top - i;
      }
    }
  }

  if (s->stack[workIndex].variable != NULL) {
    freeVar(s, workIndex); //printf("%d \n", s->stack[workIndex].variable->type);
  }

  if (v != NULL) {
    if ((s->stack[workIndex].variable = malloc(sizeof (Tvariable))) == NULL) {
      return STACK_ERROR;
    }

    //duplikace polozky
    s->stack[workIndex].variable->type = v->type;
    s->stack[workIndex].variable->data = v->data;
    if (v->type == STR_TYPE) {
      char *tmp = malloc(sizeof (char) * (v->data.str.allocSize));
      if (tmp == NULL) {
        return STACK_ERROR;
      }
      strcpy(tmp, v->data.str.str);
      s->stack[workIndex].variable->data.str.str = tmp;
    }
    else if (v->type == NUM_TYPE) {
      s->stack[workIndex].variable->data.num = v->data.num;
    }
  }

  if (s->act - i >= s->top) {
    s->top = s->act - i + 1;
  }

  return STACK_SUCCESS;
}


#endif
