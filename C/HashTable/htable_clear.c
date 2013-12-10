/* SOUBOR: htable_clear.c
 * PROJEKT: IJC-DU2
 * DATUM: 17.4.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz
 * FAKULTA: FIT VUT Brno
 * PRELOZENO: gcc 4.4.5
 * POPIS: Modul htable_clear.c knihovny libhtabel.a 
 * a libhatable.so programu wordcount.
 */

#include <stdlib.h>
#include "htable.h"

/* Funkce uvolni pamet alokovanou pro prvky tabulky. */
void htable_clear(htable_t *t)
{
  htable_listitem *node, *tmp;	// Docasne ukazatele na prvek tabulky.
  
  // Uvolni pamet alokovanou pro kazdy prvek tabulky.
  for (unsigned i = 0; i < t->htable_size; i++) {
    node = t->ht_ptr[i];
    while (node != NULL) {
      tmp = node->next;
      free((char *) node->key);	// Uvolneni pameti pro slovo.
      free(node);		// Uvolneni pameti pro prvek;
      node = tmp;
    }
  }
  
  return;
}
