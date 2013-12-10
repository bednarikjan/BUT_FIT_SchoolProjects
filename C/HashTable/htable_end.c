/* SOUBOR: htable_end.c
 * PROJEKT: IJC-DU2
 * DATUM: 17.4.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz
 * FAKULTA: FIT VUT Brno
 * PRELOZENO: gcc 4.4.5
 * POPIS: Modul htable_end.c knihovny libhtabel.a 
 * a libhatable.so programu wordcount.
 */

#include <stdlib.h> //NULL
#include "htable.h"

/* Funkce vrati iterator ukazuici za posledni prvek tabulky. */
htable_iterator htable_end(htable_t *t)
{
  // Docasny iterator inicializovany ukazatelem za posledni prvek tabulky.
  htable_iterator tmp = {t, t->htable_size - 1, NULL};
  
  return tmp;
}
