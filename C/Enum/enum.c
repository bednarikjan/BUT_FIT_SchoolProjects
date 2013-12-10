// Encoding: ISO-8859-2

/* SOUBOR: enum.c
 * PROJEKT: IJC-DU1, příklad a)
 * DATUM: 6.3.2011
 * AUTOR: Jan Bednařík (xbedna45), xbedna45@stud.fit.vutbr.cz 
 * FAKULTA: FIT VUT Brno 
 * PŘELOŽENO: gcc 4.4.5
 * POPIS: Modul enum.c čte a tiskne krátké a dlouhé verze jmen 
 * měsíců.
 */

// -- SYSTEMOVE HLAVICKOVE SOUBORY --
// ==================================

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> // tolower
#include <stdbool.h>
#include <string.h> //strcoll

// -- VLASTNI HLAVICKOVE SOUBORY --
// ================================

#include "enum.h"
#include "error.h"

// -- LOKALNI DEFINICE SYMBOLICKYCH KONSTANT --
// ============================================

#define MONTH_MAX 8  // Maximalni delka pole pro nacteni nazvu mesice.
#define MONTH_MIN 3  // Maximalni delka pole pro nacteni nazvu mesice.
#define ASCII_ALPHA_MAX 122  // Maximalni hodnota ascii alpha znaku.

// -- LOKALNI DEFINICE MAKER S PARAMETRY --
// ========================================

#define skip_spaces() while(isspace(c = getchar())); ungetc(c, stdin)

// -- UPLNE FUNKCNI PROTOYPY LOKALNICH FUNKCI --
// =============================================

int ReadChars(char month[], int *idx);
void AdjustMonth(char month[]);
bool IsDiacr(char month[], int idx);
int MonthsCompare(char month[], const char *months_names[]);

// -- DEFINICE STATICKYCH GLOBALNICH PROMENNYCH --
// ===============================================

// Dlouha jmena mesicu.
static const char *MonthL[] = {
"ChybnyMesic",
"Leden", "Únor", "Březen", "Duben", "Květen", "Červen",
"Červenec", "Srpen", "Září", "Říjen", "Listopad", "Prosinec",
};
// Kratka jmena mesicu.
static const char *MonthS[] = {
"ChybnyMesic",
"Led", "Úno", "Bře", "Dub", "Kvě", "Čen",
"Čec", "Srp", "Zář", "Říj", "Lis", "Pro",
};
// Dlouha jmena mesicu bez diakritiky.
static const char *MonthNDL[] = {
"ChybnyMesic",
"Leden", "Unor", "Brezen", "Duben", "Kveten", "Cerven",
"Cervenec", "Srpen", "Zari", "Rijen", "Listopad", "Prosinec",
};
// Kratka jmena mesicu bez diakritiky.
static const char *MonthNDS[] = {
"ChybnyMesic",
"Led", "Uno", "Bre", "Dub", "Kve", "Cen",
"Cec", "Srp", "Zar", "Rij", "Lis", "Pro",
};

// Funkcni stavy podprogramu;
enum ErrorStates {
OK,
ERROR,
};


// -- DEFINICE GLOBALNICH FUNKCI --
// ================================

/* Fuknce PrintMonthShort tiskne na stdout nazev pozadovaneho mesice 
 * ve zkracene forme. Jako formalni parametr prijima ciselnou 
 * reprezentaci pozadovaneho mesice. Pokud je cislo mimo smysluplny 
 * rozsah (1-12), vola funkci Error pro tisk chyboceho hlaseni.
 */
void PrintMonthShort (enum months d)
{
  if (d < Leden || d > Prosinec)
    Error("PrintMonthShort: Hodnota  %d je mimo rozsah\n", d);  
  printf("%s", MonthS[d]);;
  
  return;
}

/* Fuknce PrintMonth ma stejnou funkcionalitu, jako funkce 
 * PrintMonthShort. Pouze tiskne nazev pozadovaneho mesice 
 * v plne forme.
 */ 
void PrintMonth (enum months d)
{
  if (d < Leden || d > Prosinec) 
    Error("PrintMonth: Hodnota  %d je mimo rozsah\n", d);
  printf("%s", MonthL[d]);
  
  return;
}


enum months ReadMonth(void)
{
  int c;  			// Pomocna promenna.
  int idx = 0;  		// Indexace retezce.
  char month[MONTH_MAX+1];  	// Staticke pole pro nacteni retezce ze stdin.
  bool diacr = false;  		// Mesic zadan s / bez diakritiky.
  
  // Preskoceni bilych znaku ze stdin.
  skip_spaces();
  // Kontrola, zda na vstupu nebyly pouze bile znaky.
  if (c == EOF)
    return 0;
  // Nacteni znaku do pole month. 
  if(ReadChars(month, &idx) == ERROR)
    return 0;
  // Kontrola spravne delky retezce.
  if((!isspace(c = getchar()) && c != EOF) || idx < MONTH_MIN)
    return 0;
  // Prevede prvni znak na velke pismeno, ostatni na mala.
  AdjustMonth(month);
  // Kontrola, zda byl nacten retezec s diakritikou.
  diacr = IsDiacr(month, idx);
  
  // Retezec s diakritikou.
  if(diacr == true) {
    if(idx == 3)  // Retezec obsahuje prave 3 znaky.
      return MonthsCompare(month, MonthS);
    else  // Retezec obsahuje vice, nez 3 znaky.
      return MonthsCompare(month, MonthL);
  }
  
  // Retezec bez diakritiky, (diakr == false)
  if(diacr == false){
    if(idx == 3)  // Retezec obsahuje prave 3 znaky.
      return MonthsCompare(month, MonthNDS);
    else  // Retezec obsahuje vice, nez 3 znaky.
      return MonthsCompare(month, MonthNDL);
    }
  return ChybnyMesic;  
}


// -- DEFINICE LOKALNICH FUNKCI --
// ===============================

/* Funkce ReadChars nacte znaky do pole month a ukonci retezec 
 * nulovym znakem. Pokud se na vstupu objevi zakazny znak, 
 * vrati chybovou hodnotu ERROR.
 */
int ReadChars(char month[], int *idx)
{
  int c;
  
  while(!isspace(c = getchar()) && *idx < MONTH_MAX) {
    if(!isalpha(c))
      return ERROR;
    month[*idx] = c;
    (*idx)++;
  }
  ungetc(c, stdin);
  // Ukonceni retezce nulovym znakem.
  month[*idx] = '\0';
  
  return OK;
}

/* Funkce AdjustMonth prevede prvni pismeno retezce na velke 
 * a ostatni na mala.
 */
void AdjustMonth(char month[])
{
  month[0] = toupper(month[0]); 
  for(int i = 1; month[i] != '\0'; i++)
    month[i] = tolower(month[i]);
  
  return;
}

/* Funkce IsDiacr zjisti, zda byl retezec zadan s / bez 
 * diakritky. Tato informace je vracena jako logicka hodnota 
 * a ulozena do promenne diacr.
 */ 
bool IsDiacr(char month[], int idx)
{
  for(int i = 0; i < idx; i++) {
    if((unsigned char)month[i] > ASCII_ALPHA_MAX)
      return true;
  }
  return false;
}

/* Funkce MonthsCompare porovna nacteny retezec s jednotlivymi 
 * jmeny mesicu ulozenymiv danem konstantnim poli retezcu.
 */
int MonthsCompare(char month[], const char *months_names[])
{
  for(int i = 1; i <= Prosinec; i++) {
    if(strcoll(month, months_names[i]) == 0)
      return i;
  }
  return ChybnyMesic;
}
