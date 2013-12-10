/* SOUBOR: htable_begin.c
 * PROJEKT: IJC-DU2
 * DATUM: 17.4.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz
 * FAKULTA: FIT VUT Brno
 * PRELOZENO: gcc 4.4.5
 * POPIS: Modul htable_begin.c knihovny libhtabel.a 
 * a libhatable.so programu wordcount.
 */

#include <stdlib.h>
#include "htable.h"

/* Funkce vrati iterator ukazujici na prvni polozku tabulky. */
htable_iterator htable_begin(htable_t *t)
{
  // Docasny iterator inicializovany indexem a ukazatelem za posledni prvek.
  htable_iterator tmp = {t, t->htable_size - 1, NULL};
  
  // Prochazi tabulkou, dokud nenajde prvni obsazeny prvek.
  for(unsigned i = 0; i < t->htable_size; i++) {
    // Naplni docasny iterator indexem a ukazatelem na prvni prvek tabulky.
    if(t->ht_ptr[i] != NULL) {
      tmp.index = i;
      tmp.ptr = t->ht_ptr[i];
      break; 
    }
  }
  
  return tmp;
}
