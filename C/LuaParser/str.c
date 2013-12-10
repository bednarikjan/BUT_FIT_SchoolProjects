/*
 * Implementace interpretu imperativniho jazyka IFJ2011
 *
 * Autori:
 *  xbedna45 - Bednarik Jan
 *  xblaha22 - Blaha Hynek
 *  xjanys00 - Janys Martin
 *  xmacha48 - Machacek Ondrej
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "str.h"
#include "error.h"

#define STR_LEN_INC 8
// konstanta STR_LEN_INC udava, na kolik bytu provedeme pocatecni alokaci pameti
// pokud nacitame retezec znak po znaku, pamet se postupne bude alkokovat na
// nasobky tohoto cisla 

int strInit(string *s) {
  if ((s->str = (char*) malloc(STR_LEN_INC)) == NULL) {
    return STR_ERROR;
  }
  s->str[0] = '\0';
  s->length = 0;
  s->allocSize = STR_LEN_INC;

  return STR_SUCCESS;
}

void strFree(string *s) {
  free(s->str);
  s->str = NULL;
}

void strClear(string *s) {
  s->str[0] = '\0';
  s->length = 0;
}

int strAddChar(string *s1, char c) {
  if (s1->length + 1 >= s1->allocSize) {
    // pamet nestaci, je potreba provest realokaci
    if ((s1->str = (char*) realloc(s1->str, s1->length + STR_LEN_INC)) == NULL) {
      return STR_ERROR;
    }
    s1->allocSize = s1->length + STR_LEN_INC;
  }
  s1->str[s1->length] = c;
  s1->length++;
  s1->str[s1->length] = '\0';

  return STR_SUCCESS;
}

int strCopyString(string *s1, string *s2) {
  int newLength = s2->length;
  if (newLength >= s1->allocSize) {
    // pamet nestaci, je potreba provest realokaci
    if ((s1->str = (char*) realloc(s1->str, newLength + 1)) == NULL)
      return STR_ERROR;
    s1->allocSize = newLength + 1;
  }
  strcpy(s1->str, s2->str);
  s1->length = newLength;

  return STR_SUCCESS;
}

int strDuplicateStr(string *s1, string *s2) {
  s2->allocSize = s1->length + 1;
  s2->length = s1->length;

  if ((s2->str = (char*) malloc(sizeof (char) * s2->allocSize)) == NULL) {
    return STR_ERROR;
  }
  strcpy(s2->str, s1->str);
  return STR_SUCCESS;
}

int strCmpString(string *s1, string *s2) {
  return strcmp(s1->str, s2->str);
}

int strCmpConstStr(string *s1, char* s2) {
  return strcmp(s1->str, s2);
}

char *strGetStr(string *s) {
  return s->str;
}

int strGetLength(string *s) {
  return s->length;
}
//2 -1

string substr(string str, int f, int t) {
  //priprava hodnot
  int from, to;
  string sub;
  t++; //posun indexu <- index je posledni tisknuty znak //t = 0
  if (f < 0) {
    from = str.length + f;
    from = from < 0 ? 0 : from;
  }
  else {
    from = f == 0 ? 0 : f - 1; //from = 1
  }

  if (t <= 0) {
    to = str.length + t;
    to = to < 0 ? 0 : to;
  }
  else {
    to = t == 0 ? 0 : t - 1;
  }

  if (from < to) {
    //priprava stringu
    sub.allocSize = to - from + 1;
    sub.str = malloc((sizeof (char) * sub.allocSize));
    sub.length = 0;
    if (sub.str == NULL) {
      return sub;
    }
    //printf("allsize=%d\n", sub.allocSize);
  }
  else {//spatne indexy -> prazdny retezec
    sub.allocSize = 1;
    sub.str = malloc((sizeof (char) * sub.allocSize));
    sub.length = 0;
    if (sub.str == NULL) {
      return sub;
    }
    //printf("allsize=%d\n", sub.allocSize);
  }

  //printf("f=%d t=%d from=%d to=%d len=%d alsize=%d\n", f,t-1,from,to, sub.length, sub.allocSize);
  //kopie
  int i, j = 0;
  if (from < to) { //spravne indexy
    for (i = from, j = 0; i < to && i < str.length; i++, j++) {
      sub.str[j] = str.str[i];
    }
  }
  sub.str[j] = '\0';
  sub.length = strlen(sub.str);
  return sub;
}

string strConcat(string s1, string s2) {
  string str; //s1..s2

  str.length = s1.length + s2.length;
  str.allocSize = str.length + 1;
  str.str = malloc(sizeof (char) * str.allocSize);
  if (str.str == NULL) {
    return str;
  }

  strcpy(str.str, s1.str);
  strcat(str.str, s2.str);

  return str;
}
