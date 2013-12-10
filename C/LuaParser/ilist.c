/*
 * Implementace interpretu imperativniho jazyka IFJ2011
 *
 * Autori:
 *  xbedna45 - Bednarik Jan
 *  xblaha22 - Blaha Hynek
 *  xjanys00 - Janys Martin
 *  xmacha48 - Machacek Ondrej
 */

#include <stdio.h>
#include <stdlib.h>
#include "ilist.h"
#include "stack.h"

/**
 * Inicilalizace seznamu.
 * 
 * @param l - instrukcni list
 */
void listInit(pInstrList l) {
  l->first = NULL;
  l->last = NULL;
  l->active = NULL;
}

/**
 * Dealokace seznamu.
 * 
 * @param l - instrukcni list
 */
void listFree(pInstrList l) {
  iItem *ptr;
  while (l->first != NULL) {
    ptr = l->first;
    l->first = l->first->next;
    if (ptr->tac.operator == I_PUSHL) {
      if (ptr->tac.argument1 != NULL) {
        if (((Tvariable *) (ptr->tac.argument1))->type == STR_TYPE) {
          Tvariable* v = (Tvariable*) ptr->tac.argument1;
          free(v->data.str.str);
        }
        free(ptr->tac.argument1);
      }
    }
    free(ptr);
  }
}

/**
 * Vlozi intrukci na konec seznamu.
 * 
 * @param l - instrukcni list
 * @param i - instrukce
 */
int listInsertLast(pInstrList l, TAC i) {
  iItem *newItem;
  newItem = malloc(sizeof (iItem));
  if (newItem == NULL)
    return E_ALLOC;
  newItem->tac = i;
  newItem->next = NULL;
  if (l->first == NULL) {
    l->first = newItem;
  }
  else {
    l->last->next = newItem;
  }
  l->last = newItem;
  return E_OK;
}

/**
 * Aktivace prvni intrukce.
 * 
 * @param l - instrukcni list
 */
void listFirst(pInstrList l) {
  l->active = l->first;
}

/**
 * Aktivuj nasleduji intrukci.
 * 
 * @param l - instrukcni list
 */
void listNext(pInstrList l) {
  if (l->active != NULL)
    l->active = l->active->next;
}

/**
 * Nastavi aktivni instrukci podle paramtru address.
 * 
 * @param l - instrukcni list
 * @param address - adresa instrukce, na kterou chceme skocit
 */
void listGoto(pInstrList l, void *address) {
  l->active = (iItem*) address;
}

/**
 * Vrati adresu posledni instrukce.
 * 
 * @param l - instrukcni list
 */
void *listGetPointerLast(pInstrList l) {
  return (void*) l->last;
}

/**
 * Vrati aktivni instrukci.
 * 
 * @param l - instrukcni list
 */
pTAC listGetInstruction(pInstrList l) {
  if (l->active == NULL) {
    return NULL;
  }
  else {
    return &(l->active->tac);
  }
}
