/* SOUBOR: wordcount.h
 * PROJEKT: IJC-DU2, priklad b)
 * DATUM: 17.4.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz 
 * FAKULTA: FIT VUT Brno 
 * PRELOZENO: gcc 4.4.5
 * POPIS: Hlavickovy soubor nesouci rozhrani pro moduly operujicimi 
 * s hashovaci tabulkou: htable_init.c, htable_begin.c, htable_end.c, 
 * htable_iter_deref.c, htable_iter_eq.c, htable_iter_next.c, 
 * htable_lookup.c, htable_clear.c, htable_free.c.
 */

#ifndef HTABLE_H
#define HTABLE_H

#include <stdbool.h>
#include <stdlib.h>


//// Definice globalnich typu ////

// Prvek seznamu.
typedef struct htable_listitem {
  const char *key;		// Klic reprezentovany retezcem.
  unsigned data;		// Pocet vyskytu retezce ve vstup. textu.
  struct htable_listitem *next;	// Ukazatel na nasledujici prvek seznamu.
} htable_listitem;

// Udaje o hashovaci tabulce.
typedef struct {
  unsigned htable_size;		// Velikost tabulky. 
  htable_listitem *ht_ptr[];	// Ukazatel na tabulku.
} htable_t;

// Iterator.
typedef struct {
  htable_t *ht_ptr;		// Ukazatel na tabulku.
  unsigned index;		// Pozice iteratoru v poli.
  htable_listitem *ptr;		// Ukazatel na polozku.
} htable_iterator;


//// Deklarace globalnich funkci ////

htable_t *htable_init(unsigned size);
unsigned int hash_function(const char *str, unsigned htable_size);
void htable_clear(htable_t *t);
void htable_free(htable_t *t);
htable_iterator htable_begin(htable_t *t);
htable_iterator htable_end(htable_t *t);
htable_listitem *htable_lookup(htable_t *t, const char *key);


//// Definice inline funkci ////

/* Funkce vrati ukazatel na prvek tabulky, na nejz ukazuje iterator. */
inline htable_listitem *htable_iter_deref(htable_iterator tmp)
{
  return tmp.ptr;
}

/* Funkce porovna dva iteratory a vysledek vrati prostrednictvim dat. typu bool. */
inline bool htable_iter_eq (htable_iterator i1, htable_iterator i2)
{
  return ((i1.ht_ptr == i2.ht_ptr) && (i1.index == i2.index) && (i1.ptr == i2.ptr));
}

/* Funkce posouva iterator na dalsi prvek tabulky. 
 * Pokud iterator ukazuje za posledni prvek tanulky, 
 * vrati iterator beze zmeny. */
inline htable_iterator htable_iter_next (htable_iterator tmp)
{
  // Iterator ukazuje za posledni prvek tabulky.
  if (tmp.ptr == NULL)
    return tmp;  
  // Aktualni prvek neni posledni v danem seznamu, nebo je posledni v cele tabulce.
  if (tmp.ptr->next != NULL || tmp.index == tmp.ht_ptr->htable_size - 1)
    tmp.ptr = tmp.ptr->next;
  // Aktualni prvek je poslednim prvkem daneho seznamu.
  else {  // (tmp.ptr->next == NULL && tmp.index != tmp.ht_ptr->htable_size - 1)
    // Prochazi tabulku, dokud nenarazi na obsazeny prvek, nbeo konec tabulky.
    for (tmp.index++; tmp.ht_ptr->ht_ptr[tmp.index] == NULL; tmp.index++) {
      if (tmp.index == tmp.ht_ptr->htable_size - 1)
	break;
    }
    tmp.ptr = tmp.ht_ptr->ht_ptr[tmp.index];
  }
  
  return tmp;
}

#endif
