/* SOUBOR: enumtest.c
 * PROJEKT: IJC-DU1, priklad a)
 * DATUM: 6.3.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz 
 * FAKULTA: FIT VUT Brno 
 * PRELOZENO: gcc 4.4.5
 * POPIS: Modul enumtest.c nacte retezec ze stdin, vyhodnoti, 
 * zda se jedna o nazev nektereho mesice a dany mesic vytiskne 
 * na stdout. Dale vytiskne jmena vsech mesicu na stdout.
 */

// -- SYSTEMOVE HLAVICKOVE SOUBORY --
// ==================================

#include <stdio.h>
#include <locale.h>

// -- VLASTNI HLAVICKOVE SOUBORY --
// ================================

#include "enum.h"
#include "error.h"

int main(void) 
{
  char *l = setlocale(LC_ALL,"cs_CZ.iso-8859-2");
  if(l==NULL)
    Error("setlocale: Nelze nastavit ceskou lokalizaci.\n");
  enum months m;
  m = ReadMonth();			// Cte mesic.
  PrintMonthShort(m);			// Tiskne kratke jmeno.
  printf("\n");
  PrintMonth(m);			// Tiskne dlouhe jmeno.
  printf("\n\n");
  for( m = Leden; m < 15; m++ ) {	// Umyslna chyba.
    PrintMonthShort(m);
    printf("\n");
  }
  return 0;
}
  