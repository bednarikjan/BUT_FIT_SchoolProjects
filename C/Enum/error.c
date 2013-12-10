/* SOUBOR: error.c
 * PROJEKT: IJC-DU1, priklad a)
 * DATUM: 6.3.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz 
 * FAKULTA: FIT VUT Brno 
 * PŘELOŽENO: gcc 4.4.5
 * POPIS: Modul error.c tiskne chybove hlaseni v zavislosti 
 * na prijatem formatovacim retezci a parametrech. 
 */

// -- SYSTEMOVE HLAVICKOVE SOUBORY --
// ==================================

#include <stdio.h>   // fprintf
#include <stdarg.h>  // vfprintf
#include <stdlib.h>  // exit

// -- VLASTNI HLAVICKOVE SOUBORY --
// ================================

#include "error.h"

// -- DEFINICE GLOBALNICH FUNKCI --
// ================================

/* Funkce Error vytiskne chybove hlaseni s ohledem na format 
 * jednoltivych promennych, ktere prijima jako parametry.
 */
void Error(const char *fmt, ...)
{
  va_list args; 
  
  fprintf(stderr, "CHYBA: ");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  exit(1);
}
