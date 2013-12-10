/* SOUBOR: htable_init.c
 * PROJEKT: IJC-DU2
 * DATUM: 17.4.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz
 * FAKULTA: FIT VUT Brno
 * PRELOZENO: gcc 4.4.5
 * POPIS: Modul htable_init.c knihovny libhtabel.a 
 * a libhatable.so programu wordcount.
 */

#include <stdlib.h>
#include "htable.h"

/* Funkce inicializuje hashovaci tabulku a vraci ukazatel 
 * na tabulku (strukturu htable_t). */
htable_t *htable_init(unsigned size)
{
  htable_t *t_tmp;	// Ukazatel na tabulku (strukturu htable_t).
  
  // Alokace pameti pro strukturu nesouci udaje o tabulce htable_t.
  if ((t_tmp = (htable_t *) malloc(sizeof(htable_t) 
       + size * sizeof(htable_listitem *))) == NULL)
    return NULL;
  
  // Ulozeni velikosti tabulky a inicializace ukazatelu na htable_listitem na NULL.
  t_tmp->htable_size = size;
  for (unsigned i = 0; i < size; i++) {
    t_tmp->ht_ptr[i] = NULL;
  }  
  
  return t_tmp;
}
