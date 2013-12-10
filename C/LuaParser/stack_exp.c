/*
 * Implementace interpretu imperativniho jazyka IFJ2011
 *
 * Autori:
 *  xbedna45 - Bednarik Jan
 *  xblaha22 - Blaha Hynek
 *  xjanys00 - Janys Martin
 *  xmacha48 - Machacek Ondrej
 */

#include "stack_exp.h"
#include "scaner.h"
#include "ial.h"
#include <stdbool.h>
#include <stdlib.h>

// Inicializace zasobniku pouzivaneho pro vyrazy.

int stackExpInit(TstackExp* se) {
  Ttoken first;
  first.type = ENDING_TOKEN;
  first.data.identificator = NULL;

  if ((se->stackExp = (Ttoken *) malloc(STACK_EXP_SIZE * sizeof (Ttoken))) == NULL) {
    return STACK_EXP_ERROR;
  }

  se->top = 0;
  se->allocSize = STACK_EXP_SIZE;
  stackExpPush(se, first);

  return STACK_EXP_SUCCESS;
}

void stackExpDestroy(TstackExp* se) {
  free(se->stackExp);
  se->top = 0;
  se->allocSize = 0;
  se->stackExp = NULL;
}

Ttoken stackExpTop(TstackExp* se) {
  return se->stackExp[se->top - 1];
}

// Zvetseni zasobniku pouzivaneho pro vyrazy.

int stackExpResize(TstackExp* se) {
  se->allocSize *= 2; // Velikost vyrazoveho zasobniku bude zdojnasobena.

  if ((se->stackExp = (Ttoken *) realloc(se->stackExp, se->allocSize * sizeof (Ttoken))) == NULL) {
    return STACK_EXP_ERROR;
  }

  return STACK_EXP_SUCCESS;
}

// Uvolneni zasobniku pouzivaneho pro vyrazy.

void stackExpFree(TstackExp* se) {
  free(se->stackExp);
  se->stackExp = NULL;

  return;
}

// Vlozi na zasobnik kopii prichoziho tokenu.

int stackExpPush(TstackExp* se, Ttoken global) {
  if (se->top >= se->allocSize - 1) {
    if (stackExpResize(se) == STACK_EXP_ERROR) {
      return STACK_EXP_ERROR;
    }
  }

  se->stackExp[se->top] = global;
  se->top++;

  return STACK_EXP_SUCCESS;
}

// Odstraneni urciteho poctu prvku ze zasobniku

void stackExpMultiplePop(TstackExp* se, int count) {
  se->top = se->top - count < 0 ? 0 : se->top - count;

  return;
}

// Pomocna funkce na vypis informaci o zasobniku

void stackExpInfo(TstackExp *se) {
  if (se->stackExp == NULL) {
    printf("NEINICIALIZOVANY ZASOBNIK\n");
    printf("-------------------------\n\n");
  }
  else {
    printf("-------------------------\n\n");
    for (int i = 0; i < se->top; i++) {
      switch (se->stackExp[i].type) {
        case ID:
          printf("-- %d: TYPE = ID, NAME = %s\n", i, se->stackExp[i].data.identificator);
          break;
        case NUMBER:
          printf("-- %d: TYPE = NUM, VALUE = %lf\n", i, se->stackExp[i].data.value);
          break;
        case STRING:
          printf("-- %d: TYPE = STRING, TEXT = %s\n", i, se->stackExp[i].data.str_data.str);
          break;
        case L_BRACKET:
          printf("-- %d: TYPE = OPERATOR, SIGN = '(' \n", i);
          break;
        case R_BRACKET:
          printf("-- %d: TYPE = OPERATOR, SIGN = ')' \n", i);
          break;
        case CONCAT:
          printf("-- %d: TYPE = OPERATOR, SIGN = '..' \n", i);
          break;
        case PLUS:
          printf("-- %d: TYPE = OPERATOR, SIGN = '+' \n", i);
          break;
        case MINUS:
          printf("-- %d: TYPE = OPERATOR, SIGN = '-' \n", i);
          break;
        case MUL:
          printf("-- %d: TYPE = OPERATOR, SIGN = '*' \n", i);
          break;
        case DIV:
          printf("-- %d: TYPE = OPERATOR, SIGN = '/' \n", i);
          break;
        case CARET:
          printf("-- %d: TYPE = OPERATOR, SIGN = '^' \n", i);
          break;
        case EQUAL:
          printf("-- %d: TYPE = OPERATOR, SIGN = '==' \n", i);
          break;
        case NOT_EQUAL:
          printf("-- %d: TYPE = OPERATOR, SIGN = '~=' \n", i);
          break;
        case LESS:
          printf("-- %d: TYPE = OPERATOR, SIGN = '<' \n", i);
          break;
        case LESS_EQUAL:
          printf("-- %d: TYPE = OPERATOR, SIGN = '<=' \n", i);
          break;
        case MORE:
          printf("-- %d: TYPE = OPERATOR, SIGN = '>' \n", i);
          break;
        case MORE_EQUAL:
          printf("-- %d: TYPE = OPERATOR, SIGN = '>=' \n", i);
          break;
        case E:
          printf("-- %d: TYPE = E, POSITION = %d , PrevType = %s\n", i, se->stackExp[i].data.Edata.positionToAct, token2Str(se->stackExp[i].data.Edata.datatypeE));
          break;
        case TRUE:
          printf("-- %d: TYPE = TRUE\n", i);
          break;
        case FALSE:
          printf("-- %d: TYPE = FALSE\n", i);
          break;
        case ENDING_TOKEN:
          printf("-- %d: TYPE = $\n", i);
          break;
        case NIL:
          printf("-- %d: TYPE = NIL\n", i);
          break;
        default:
          printf("-- %d: D  E  F  A  U  L  T\n", i);
          break;
      }
    }
  }
  return;
}

Ttoken *stackGetLastTerminal(TstackExp* se) {
  if (se->stackExp != NULL) {
    for (int i = se->top - 1; i >= 0; i--) {
      if (se->stackExp[i].type != E) return (&(se->stackExp[i]));
    }
  }

  return NULL;
}

void stackExpTopToStart(TstackExp* se) {
  se->top = 1;
  return;
}
