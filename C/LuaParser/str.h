/*
 * Implementace interpretu imperativniho jazyka IFJ2011
 *
 * Autori:
 *  xbedna45 - Bednarik Jan
 *  xblaha22 - Blaha Hynek
 *  xjanys00 - Janys Martin
 *  xmacha48 - Machacek Ondrej
 */

#include "error.h"

#ifndef STR_H
#define STR_H

#define STR_ERROR   1
#define STR_SUCCESS 0

typedef struct {
    char* str; // misto pro dany retezec ukonceny znakem '\0'
    int length; // skutecna delka retezce
    int allocSize; // velikost alokovane pameti
} string;


int strInit(string *s);
void strFree(string *s);

void strClear(string *s);
int strAddChar(string *s1, char c);
int strCopyString(string *s1, string *s2);
int strCmpString(string *s1, string *s2);
int strCmpConstStr(string *s1, char *s2);
int strDuplicateStr(string *s1, string *s2);

char *strGetStr(string *s);
int strGetLength(string *s);

string substr(string s, int f, int t);
string strConcat(string s1, string s2);

#endif
