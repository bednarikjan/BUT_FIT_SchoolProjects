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
#include "ial.h"
#include "error.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>
/* TODO: doplnit komentare */
////////////////////////////////////////////////////////////////////////////////
//                              SORT                                          //
////////////////////////////////////////////////////////////////////////////////

/**
 * Funkce iteraci prochazi otce a jeho dva syna v pripade spatneho usporadani
 * jejich pozici ve strome (v poli vymeni)
 * @param sorted - nove pole
 * @param root - otec 
 * @param botton - rozsah stromu, ktery budeme tridit 
 */
void moveDown(char* sorted, int root, int bottom) {
  int maxChild, temp;

  bool done = false;

  while (((root << 1) + 1 <= bottom) && (!done)) {
    maxChild = (root << 1) + 1;
    if (maxChild + 1 <= bottom
            && sorted[maxChild] < sorted[maxChild + 1])
      maxChild++; //pridani praveho syna do rozsahu
    if (sorted[root] < sorted[maxChild]) {
      temp = sorted[root];
      sorted[root] = sorted[maxChild];
      sorted[maxChild] = temp;
      root = maxChild; //zmena rozahu pod stromu
    }
    else {
      done = true;
    }
  }
}

/**
 * prochazi pod haldy, pro syny plati: (i = index otce) 2i+1 2i+2 
 * @param str - retezec na serazeni
 * @param len - delka (pocet znaku o 0 ktere budeme radit)
 */
char* heapSort(char* str, int len) {

  int i, temp;
  char* sorted = malloc(sizeof (char) * len + 1);
  if (sorted == NULL) {
    return NULL;
  }
  strcpy(sorted, str);
  //speedtest time - user time je bez mallocu  
  for (i = (len >> 1) - 1; i >= 0; i--) {//cyklus prosevani podstromu 
    //projizdi levou cast pole (tj. "nad strom" bez listu pouze otcove)
    moveDown(sorted, i, len - 1);
  }
  for (i = len - 1; i >= 1; i--) { //vlastni sort
    /*sorted[0] ^= sorted[i];
    sorted[i] ^= sorted[0];
    sorted[0] ^= sorted[i];*/
    temp = sorted[0];
    sorted[0] = sorted[i];
    sorted[i] = temp;
    moveDown(sorted, 0, i - 1);
  }

  return sorted;
}

string sort(string str) {
  string sorted;

  sorted = str;
  sorted.str = heapSort(str.str, str.length);

  return sorted;
}


////////////////////////////////////////////////////////////////////////////////
//                               FIND                                         //
////////////////////////////////////////////////////////////////////////////////

/**
 * Naplni pole shift posuny.
 *
 * @param pattern - podretezec ktery hledame
 * @param patternLen - delka hledaneho podretezce
 * @param shift   - pole hodnot spatnych posuvu
 */
void badShift(char *pattern, int patternLen, int *shift) {
  int i; //MJ smazano patLen

  /** Vsechny znaky nastav na posouvani o delku vyhledavaneho retezce */
  for (i = 0; i < ASIZE; i++)
    shift[i] = patternLen;
  /** Prenastav ty znaky ktere se vyskytuji ve vyhledavanem retezci */
  for (i = 0; i < patternLen - 1; i++)
    shift[(int) pattern[i]] = patternLen - i - 1;
}

/**
 *
 * @param pattern    - podretezec ktery hledame
 * @param patternLen - delka hledaneho podretezce
 * @param suff       - 
 */
void suffixes(char *pattern, int patternLen, int *suff) {
  int i, f, g;
  suff[patternLen - 1] = patternLen;
  g = patternLen - 1;
  f = 0;

  for (i = patternLen - 2; i >= 0; i--) {
    if (i > g && suff[i + patternLen - 1 - f] < i - g)
      suff[i] = suff[i + patternLen - 1 - f];
    else {
      if (i < g)
        g = i;
      f = i;
      for (; g >= 0 && pattern[g] == pattern[g + patternLen - 1 - f]; g--);
      suff[i] = f - g;
    }
  }
}

/**
 * Naplni pole shift posuny. (Lepsi)
 *
 * @param pattern    - podretezec ktery hledame
 * @param patternLen - delka hledaneho podretezce
 * @param good       - pole hodnot spravnych posuvu
 */
void goodShift(char *pattern, int patternLen, int *shift) {
  int i, j, suff[patternLen + 1];
  i = j = 0;
  suffixes(pattern, patternLen, suff);

  /* Napln vsechny prvky shiftu hodnotou patternLen */
  while (i < patternLen)
    shift[i++] = patternLen;

  /*  */
  for (i = patternLen - 1; i >= 0; i--)
    if (suff[i] == i + 1)
      for (; j < patternLen - 1 - i; j++)
        if (shift[j] == patternLen)
          shift[j] = patternLen - 1 - i;
  /* */
  for (i = 0; i <= patternLen - 2; i++)
    shift[patternLen - 1 - suff[i]] = patternLen - 1 - i;
}

int find(string s1, string s2) {
  return findBM(s1.str, s2.str, s1.length, s2.length);
}

/**
 * Turbo Boyer-Moore - upravena metoda boyer-moore pro vyhledavani podretezce v retezci.
 * vice info: http://www-igm.univ-mlv.fr/~lecroq/string/node15.html
 *
 * @param pattern      - podretezec ktery hledame
 * @param text         - retezec ve kterem hledame podretezec
 * @return position    - pozici prvniho vyskytu na ktere byl retezec nalezen, pokud nebyl vraci -1
 */
int findBM(char *text, char *pattern, int textLen, int patternLen) {
  int unmatched, index, bad_shift[ASIZE], shift, matched, turboShift, bcShift, overlap;

  shift = patternLen;
  index = 0;
  overlap = 0;

  int good_shift[patternLen + 1];

  goodShift(pattern, patternLen, good_shift);
  badShift(pattern, patternLen, bad_shift);

  while (index <= textLen - patternLen) {
    unmatched = patternLen - 1;
    while (unmatched >= 0 && (pattern[unmatched] == text[unmatched + index])) {
      unmatched--;
      if (overlap != 0 && (unmatched == (patternLen - 1 - shift)))
        unmatched -= overlap;
    }
    if (unmatched < 0) {
      return index;
      shift = good_shift[0];
      overlap = patternLen - shift;
    }
    else {
      matched = patternLen - 1 - unmatched;
      turboShift = overlap - matched;
      bcShift = bad_shift[(int) text[unmatched + index]] - patternLen + 1 + unmatched;
      shift = MAX(turboShift, bcShift);
      shift = MAX(shift, good_shift[unmatched]);
      if (shift == good_shift[unmatched])
        overlap = MIN(patternLen - shift, matched);
      else {
        if (turboShift < bcShift)
          shift = MAX(shift, overlap + 1);
        overlap = 0;
      }
    }
    index += shift;
  }
  return -1;
}

////////////////////////////////////////////////////////////////////////////////
//                              stable                                        //
////////////////////////////////////////////////////////////////////////////////

unsigned int hashFunction(const char* str, unsigned stableSize) {
  unsigned int h = 0;
  unsigned char * p;

  for (p = (unsigned char*) str; *p != '\0'; p++) {
    h = 31 * h + *p;
  }

  return h % stableSize;
}

/* Funkce inicializuje hashovaci tabulku a vraci ukazatel 
 * na tabulku (strukturu Tstable). */
Tstable* stableInit(unsigned size) {

  Tstable* stable;
  stable = malloc(sizeof (Tstable));
  if (stable == NULL)
    return NULL;

  stable->stableSize = size;
  stable->itemCounter = 0;
  stable->uk = malloc(size * sizeof (stableItemPtr));
  if (stable->uk == NULL)
    return NULL;

  for (unsigned i = 0; i < size; i++)
    stable->uk[i] = NULL;

  return stable;
  /*Tstable* t_tmp; // Ukazatel na tabulku (strukturu Tstable).

  // Alokace pameti pro strukturu nesouci udaje o tabulce Tstable.
  if ((t_tmp = (Tstable *) malloc(sizeof (Tstable)
          + size * sizeof (stableItemPtr))) == NULL) {
    return NULL;
  }

  // Ulozeni velikosti tabulky a inicializace ukazatelu na stable_listitem na NULL.
  t_tmp->stableSize = size;
  for (unsigned i = 0; i < size; i++) {
    t_tmp->uk[i] = NULL;
  }

  return t_tmp;*/
}

/**
 * maze zaznamy tabulky
 * @param t ukazatel na tabulku
 */
void stableClear(Tstable* t) {

  stableItemPtr pom;

  for (int i = 0; i < (int) t->stableSize; i++) {
    while (t->uk[i] != NULL) {
      pom = t->uk[i]->next;
      free(t->uk[i]->key);
      free(t->uk[i]);
      t->uk[i] = pom;
    }
  }
  // Vynulovani pocitadla.
  t->itemCounter = 0;
}

void stableFree(Tstable *t) {
  stableClear(t);
  free(t->uk);
  free(t);
  t = NULL;
}

/**
 * vyhledaní prvku v tabulce, pri nenalezeni vraci NULL
 * @param t ukazatel na tabulku
 * @param key retezce ktery hledame
 * @return ukazatel na polozku tabulky
 */
stableItemPtr stableLookup(Tstable* t, char *key) {

  unsigned int index = hashFunction(key, t->stableSize);

  for (stableItemPtr i = t->uk[index]; i != NULL; i = i->next) {
    if (strcmp(i->key, key) == 0) {
      return i;
    }
  }
  return NULL;
}

stableItemPtr stablePutKey(Tstable* t, char *key) {

  unsigned int index = hashFunction(key, t->stableSize);

  stableItemPtr item = t->uk[index];
  stableItemPtr newitem = malloc(sizeof (struct stableListItem));
  if (newitem == NULL) {
    return NULL;
  }

  //key je naalokovan ze scaneru
  /*newitem->key = malloc((strlen(key) + 1) * sizeof (char));
  if (newitem->key == NULL) {
    free(newitem);
    return NULL;
  }*/

  newitem->key = key; //prirazeni klice
  newitem->start = NULL;
  newitem->next = item; //vkladani na zacatek
  t->uk[index] = newitem;

  t->itemCounter++; //zvetseni counteru itemu

  //t->uk[index]->type = 0;
  //stable->uk[index]->next = item;

  return newitem;
}

int stableResize(Tstable* stable, unsigned int newSize) {

  Tstable* newstable = stableInit(newSize);
  if (newstable == NULL) {
    return 1; // neni dostatek pameti pro zvetseni
    // TODO HYNEK: Nevracet magicky cislo.
  }
  //stable_listitem* prev_item = NULL;
  int index;
  //struct stable_listitem* listitem;
  stableItemPtr pom, pom2;

  for (int i = 0; i < (int) stable->stableSize; i++) {
    while (stable->uk[i] != NULL) {
      pom = stable->uk[i]->next;
      index = hashFunction(stable->uk[i]->key, newSize);
      if (newstable->uk[index] == NULL) {
        newstable->uk[index] = stable->uk[i];
        newstable->uk[index]->next = NULL;
        //iter_begin.ptr = new_stable->uk[index]->next;
      }
      else {
        pom2 = newstable->uk[index];
        newstable->uk[index] = stable->uk[i];
        newstable->uk[index]->next = pom2;
      }
      stable->uk[i] = pom;
    }
  }

  free(stable->uk);

  stable->uk = newstable->uk;
  stable->stableSize = newSize;

  free(newstable);

  return 0;
}
