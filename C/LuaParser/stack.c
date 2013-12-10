/*
 * Implementace interpretu imperativniho jazyka IFJ2011
 *
 * Autori:
 *  xbedna45 - Bednarik Jan
 *  xblaha22 - Blaha Hynek
 *  xjanys00 - Janys Martin
 *  xmacha48 - Machacek Ondrej
 */

#include "str.h"
#include "stack.h"
#include <string.h>
#include <stdbool.h> 
#include <stdlib.h>
#include <stdio.h>

int stackInit(Tstack* s) {
  if ((s->stack = (UstackItem*) malloc(PROG_STACK_SIZE * sizeof (UstackItem))) == NULL) {
    return STACK_ERROR;
  }
  s->top = 0;
  s->act = 0;
  s->allocSize = PROG_STACK_SIZE;

  for (int i = 0; i < PROG_STACK_SIZE; i++) {
    s->stack[i].variable = NULL; //inicializace celeho stacku
  }

  return STACK_SUCCESS;
}

void stackFree(Tstack* s) {
  free(s->stack);
  s->stack = NULL;
}

//uvolni pocet promenych od topu

void stackDisposeVariables(Tstack* s, int count) {
  int bound = s->top - count;
  bound = bound < 0 ? 0 : bound;

  for (int i = (int) s->top; i >= bound; i--) {
    freeVar(s, i);
  }
  s->top = bound;
  s->act = bound;
}

void stackDisposeAddress(Tstack* s) {
  s->stack[s->top - 1].instruction = NULL; // prepsani adresy na vrcholu
  s->top--;
}

//rust zasobniku

int stackGrowUp(Tstack *s, unsigned int reqSize) {

  unsigned int actSize = s->allocSize;

  while (reqSize >= actSize) { //vypocet mista ktere budem potrebovat
    actSize += PROG_STACK_SIZE;
  }

  //realokace
  if ((s->stack = (UstackItem*) realloc(s->stack, actSize * sizeof (UstackItem))) == NULL) {
    return STACK_ERROR;
  }
  s->allocSize = actSize;

  for (unsigned int i = s->top; i < s->allocSize; i++) {
    s->stack[i].variable = NULL; //inicializace
  }

  return STACK_SUCCESS;
}

//pouze na testy - nepouzivana

int stackPrepareMem(Tstack *s, unsigned int n) {

  unsigned int newTop = s->top + n;

  if (newTop >= s->allocSize) {
    if (stackGrowUp(s, newTop) == STACK_ERROR) {
      return STACK_ERROR;
    }
  }
  s->top = newTop;

  return STACK_SUCCESS;
}





int stackPushAddress(Tstack* s, iItem *i) {

  if (s->top + 1 >= s->allocSize) { // overlimit

    if ((s->stack = (UstackItem*) realloc(s->stack, s->top + PROG_STACK_SIZE)) == NULL) {
      return STACK_ERROR;
    }
    s->allocSize = s->top + PROG_STACK_SIZE;
  }

  s->stack[s->top].instruction = i;
  s->top++;

  return STACK_SUCCESS;
}

bool stackEmpty(Tstack *s) {
  return s->top == 0;
}

iItem* stackGetAdr(Tstack *s) {
  return s->stack[s->top - 1].instruction;
}

Tvariable *stackTopAndPop(Tstack* s) {

  if (!stackEmpty(s)) {
    s->top--;
    Tvariable* pom = s->stack[s->top - 1].variable;
    s->stack[s->top - 1].variable = NULL;
    return pom;
  }
  else {
    return NULL;
  }
}

Tvariable *stackTop(Tstack* s) {
  return s->stack[s->top - 1].variable;
}

void stackPop(Tstack* s) {

  if (!stackEmpty(s)) {
    freeVar(s, s->top - 1);
    s->stack[s->top - 1].variable = NULL;
    s->top--;
  }
}

//zarovnani parametru pred skokem do funkce, n = pocet parametru

int stackAlignParams(Tstack *s, int n) {
  if (s->act + n < s->top) {
    for (unsigned int i = s->top; i > s->act + n; i--) {
      stackPop(s);
    }
  }
  else if (s->act + n > s->top) {
    for (unsigned int i = s->top; i < s->act + n; i++) {
      Tvariable *v = (Tvariable *) malloc(sizeof (Tvariable));
      if (!v)
        return STACK_ERROR;
      v->type = NIL_TYPE;
      stackPush(s, v);
      free(v);
    }
  }
  stackActToTop(s);
  return STACK_SUCCESS;
}

void stackPrint(Tstack* s, unsigned int n) {
  n = 0;
  fprintf(stderr, "Stack\n");
  fprintf(stderr, "|-----------------------------------------|\n");
  for (unsigned int i = n; i <= s->top; i++) {
    if (s->stack[i].variable == NULL) {
      fprintf(stderr, "| [%d] == NULL", i);
    }
    else {
      switch ((int) s->stack[i].variable->type) {
        case STR_TYPE:
          fprintf(stderr, "| [%d] == string(%s)", i, s->stack[i].variable->data.str.str);
          break;
        case NUM_TYPE:
          fprintf(stderr, "| [%d] == number(%g)", i, s->stack[i].variable->data.num);
          break;
        case BOOL_TYPE:
          fprintf(stderr, "| [%d] == boolean(%s)", i, s->stack[i].variable->data.boolean == 1 ? "true" : "false");
          break;
        case NIL_TYPE:
          fprintf(stderr, "| [%d] == nil(%s)", i, "nil");
          break;
      }
    }
    if (i == s->act) {
      fprintf(stderr, " <= act");
    }
    if (i == s->top) {
      fprintf(stderr, " <= top");
    }
    fprintf(stderr, "\n");
  }
}
