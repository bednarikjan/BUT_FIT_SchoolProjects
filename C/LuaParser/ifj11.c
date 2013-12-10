/*
 * Implementace interpretu imperativniho jazyka IFJ2011
 *
 * Autori:
 *  xbedna45 - Bednarik Jan
 *  xblaha22 - Blaha Hynek
 *  xjanys00 - Janys Martin
 *  xmacha48 - Machacek Ondrej
 */

#include "scaner.h"
#include "ial.h"
#include "error.h"
#include "str.h"
#include "stack.h"
#include "ilist.h"
#include "parser.h"
#include "interpret.h"
#include "stack_exp.h"

int main(int argc, char **argv) {

  // Nastaveni vstupniho souboru.
  FILE *f;
  if (argc == 1) {
      // fprintf(stderr, "Neni zadan vstupni soubor\n");
      return INTER_ERROR;
   }
   if ((f = fopen(argv[1], "r")) == NULL) {
      // fprintf(stderr, "Soubor se nepodarilo otevrit\n");
      return INTER_ERROR;
   }
   setSourceFile(f);

  // Seznam instrukci
  pInstrList iList = NULL;
  if ((iList = (pInstrList) malloc(sizeof(instrList))) == NULL) return INTER_ERROR;
  listInit(iList);

  // Globalni tabulka symbolu.
  Tstable *gTable;
  if ((gTable = stableInit(ST_SIZE)) == NULL) return INTER_ERROR;

  // Lokalni tabulka symbolu.
  Tstable *lTable = NULL;
  if ((lTable = stableInit(ST_SIZE)) == NULL) return INTER_ERROR;


  int result;
  // Syntatkicky analyzator.
  result = parse(gTable, lTable, iList);

  switch (result)
  {
     case LEX_ERROR:
     case SYNTAX_ERROR:
     case SEM_ERROR:
     case INTERPRET_ERROR:
     case INTER_ERROR:
       stableFree(lTable);
       stableFree(gTable);
       listFree(iList);
       free(iList);
       fclose(f);
       DisposeToken();
       return result;
     break;
     // jinak probehlo vse v poradku, muzeme provadet kod
  }
  
  // Zavreni vstupniho souboru.
  fclose(f);
  // Uvolneni globalni a lokalni tabulky symbolu.
  stableFree(lTable);
  stableFree(gTable);
  // interpretace
  result = interpret(iList);

  switch (result)
  {
     case LEX_ERROR:
     case SYNTAX_ERROR:
     case SEM_ERROR:
     case INTERPRET_ERROR:
     case INTER_ERROR:
       listFree(iList);
       free(iList);
       return result;
     break;
     // jinak probehlo vse v poradku, muzeme provadet kod
  }

  // Uvolneni instrukcni pasky
  listFree(iList);
  free(iList);


  return 0;
}
