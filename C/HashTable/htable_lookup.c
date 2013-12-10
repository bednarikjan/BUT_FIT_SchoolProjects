/* SOUBOR: htable_lookup.c
 * PROJEKT: IJC-DU2
 * DATUM: 17.4.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz
 * FAKULTA: FIT VUT Brno
 * PRELOZENO: gcc 4.4.5
 * POPIS: Modul htable_lookup.c knihovny libhtabel.a 
 * a libhatable.so programu wordcount.
 */

#include <string.h>
#include <stdlib.h>
#include "htable.h"

/* Funkce vyhleda prvek v tabulce podle klice a vrati 
 * ukazatel na nej. Pokud prvek nenajde, vlozi ho jako 
 * novy a vrati ukazatel na nej. */
htable_listitem *htable_lookup(htable_t *t, const char *key)
{
  int i;			// Index to tabulky.
  htable_listitem *node = NULL;	// Docasne prvky tabulky. 
  htable_listitem *tmp = NULL;
  
  // Ziskani indexu do tabulky.
  i = hash_function(key, t->htable_size);
  node = t->ht_ptr[i];
  
  // Hledani prvku podle klice.
  while (node != NULL) {
    // Prvek byl nalezen.
    if (strcmp(key, node->key) == 0)
      return node;
    tmp = node;
    node = node->next;
  }
  
  // Alokace pameti pro novy prvek.
  if ((node = (htable_listitem *) malloc(sizeof(htable_listitem))) == NULL)
    return NULL;
  
  // Inicializace node.
  node->key = key;
  node->data = 0;  
  node->next = NULL;
  // Vlozeni noveho prvku node do seznamu;
  (tmp == NULL) ? (t->ht_ptr[i] = node) : (tmp->next = node);
  
  return node;
}
