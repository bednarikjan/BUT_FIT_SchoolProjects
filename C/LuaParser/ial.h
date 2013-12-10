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
#include "ilist.h"

#ifndef IAL_H
#define IAL_H

/* Inicializacni velikost tabulky symoblu. */
#define ST_SIZE 499

/** 2^8 Pocet vsech ASCII znaku - nutne pro predzpracovani tabulky u BM */
#define ASIZE 256
/* Macro MAX vraci vetsi hodnotu ze dvou predanych hodnot */
#define MAX(x,y) ((x) < (y) ? (y) : (x))
/* Macro MIN vraci mensi hodnotu ze dvou predanych hodnot */
#define MIN(x,y) ((x) < (y) ? (x) : (y))

/* Obalovaci fce heapSortu */
string sort(string str);
char* heapSort(char* str, int len);

/* Najde prvni pozici podretezce v retezci */
int find(string, string);
int findBM(char *, char *, int, int);

/* Polozka hashovaci tabulky */
typedef struct stableListItem {
    char* key; // identificator
    iItem* start;
    int number; //na globalni urvni pocet parametru funkce, na lokalni urovni poradove cislo promenne 
    struct stableListItem* next;
} *stableItemPtr;

/* hashovaci tabulka */
typedef struct {
    unsigned int itemCounter;
    unsigned int stableSize;
    stableItemPtr* uk;
} Tstable;

/* Funkce hashovaci tabulky */
unsigned int hashFunction(const char* str, unsigned stableSize);
Tstable* stableInit(unsigned size);
stableItemPtr stableLookup(Tstable* t, char *key);
stableItemPtr stablePutKey(Tstable* t, char *key);
int stableResize(Tstable* stable, unsigned int newSize);
void stableFree(Tstable *t);
void stableClear(Tstable* t);
#endif
