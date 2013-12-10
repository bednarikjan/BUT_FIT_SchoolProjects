/* SOUBOR: htable_free.c
 * PROJEKT: IJC-DU2
 * DATUM: 17.4.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz
 * FAKULTA: FIT VUT Brno
 * PRELOZENO: gcc 4.4.5
 * POPIS: Modul htable_free.c knihovny libhtabel.a 
 * a libhatable.so programu wordcount.
 */

#include <stdlib.h>
#include "htable.h"

/* Funkce uvolni pamet alokovanou pro tabulku. */
void htable_free (htable_t *t)
{
  // Uvolneni pameti alokovane pro prvky tabulky.
  htable_clear(t);
  // Uvolneni pameti alokovane pro strukturu nesouci informace o tabulce.
  free(t);
  
  return;
}
