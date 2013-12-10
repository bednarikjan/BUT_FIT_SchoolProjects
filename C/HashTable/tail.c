/* SOUBOR: tail.c
 * PROJEKT: IJC-DU2
 * DATUM: 6.3.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz
 * FAKULTA: FIT VUT Brno
 * PRELOZENO: gcc 4.4.5
 * POPIS: Program tail tiskne poslednich 10 radku zadaneho souboru.
 * Chovani programu a pocet tisknutych radku je mozno menit pomoci 
 * prepinace -n a poctu radku.
 */

// -- SYSTEMOVE HLAVICKOVE SOUBORY --
// ==================================

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>


// -- LOKALNI DEFINICE SYMBOLICKYCH KONSTANT --
// ============================================

// Maximalni delka jednoho radku vcetne pripadneho znaku konce radku.
#define LINE_LEN 1025
// Pocet tisknutych radku (implicitne)
#define LINE_COUNT 10


// -- DEFINICE GLOBALNICH PROMENNYCH --
// ====================================

// Chybova hlaseni programu.
const char *errCodeMsg[] =
{
  // S_OK
  "OK.\n",
  // S_WRONG_PARAMS
  "Byly zadany chybne parametry.\n",
  // S_ALLOC
  "Nastala chyba pri alokaci pameti.\n",
  //S_FILE
  "Textovy soubor nelze otevrit.\n",
  //S_LINE_LEN
  "Prekrocena maximalni povolena delka jednoho radku (1024 znaku).\n",
};

// -- LOKALNI DEFINICE NOVYCH TYPU --
// ==================================

// Struktura uchovavajici parametry zadane pri spusteni.
typedef struct {
  int mode;
  long lines;
  char *fileName;
} Tparams;


// -- UPLNE FUNKCNI PROTOYPY LOKALNICH FUNKCI --
// =============================================

int GetParams (int argc, char *argv[], Tparams *params);
int ExtractNumber (char *number, Tparams *params);
int AllocLines(char *lineField[], int n);
void FreeLines (char *lineField[], int i);
void PrintLast (Tparams *params, char *lineField[], int printFrom);
void PrintFrom (FILE *file);


// Chybove stavy programu.
enum ErrorStates {
  // OK
  S_OK,
  // Chybne zadane parametry programu.
  S_WRONG_PARAMS,
  // Chyba pri alokaci pameti.
  S_ALLOC,
  // Chyba pri otevirani souboru.
  S_FILE,
  // Prekrocen limit implementacni delky radku.
  S_LINE_LEN,
};

// Rezimy programu tail.
enum TailMode {
  // Tisk poslednich N radku.
  M_LAST,
  // Tisk od N-teho radku.
  M_FROM,
};

// Promenna uchovava informaci, zda uz byla tisknuta chyba.
bool error_printed = false;


// -- DEFINICE LOKALNICH FUNKCI --
// ===============================

/* Funkce GetParams zpracuje parametry z prikazove radky.
 * Pri vyskytu nelegalne zadanych parametru vraci chybovy
 * kod. */
int GetParams (int argc, char *argv[], Tparams *params)
{
  // Vycet typu nasledujiciho ocekavaneho parametru.
  enum {PARAM_N, NUMBER, FILE} next;

  if (argc > 4)
    return S_WRONG_PARAMS;

  // Prvni parametr je prepinac "-n".
  if (strcmp(argv[1], "-n") == 0) {
    // Za prepinacem -n chyby dalsi parametr.
    if (argc == 2)
      return S_WRONG_PARAMS;
    next = NUMBER;
  }
  // Prvni parametr je retezec predstavujici nazev souboru.
  else {
    params->fileName = argv[1];
    next = PARAM_N;
  }

  for (int a = 2; a < argc; a++) {
    switch (next) {
      // Ocekavany parametr je typu cislo.
      case NUMBER:
	if (ExtractNumber (argv[a], params) != S_OK)
	  return S_WRONG_PARAMS;
	next = FILE;
	continue;
      // Ocekavany parametr je typu retezec "-n".
      case PARAM_N:
	if (strcmp(argv[a], "-n") != 0 || argc == a+1)
	  return S_WRONG_PARAMS;
	next = NUMBER;
	continue;
      // Ocekavany parametr je typu retezec predtavujici nazev souboru.
      case FILE:
	params->fileName = argv[a];
	next = PARAM_N;
	continue;
    }
  }

  return S_OK;
}

/* Funkce ExtractNumber prevede retzec na cislo a zjisti,
 * zda cislu predchazel znak '+'. V pripade chyby vraci
 * chybovy kod. */
int ExtractNumber (char *number, Tparams *params)
{
  char * err;	// PRipadny nelegalni znak.
  
  // Prvni znak neni cislice.
  if (isdigit(number[0]) == 0) {
    if (number[0] == '+')
      params->mode = M_FROM;
    else
      return S_WRONG_PARAMS;
  }
  errno = 0;
  params->lines = strtol(number, &err, 10);

  // Prevod se nezdaril.
  if (errno == ERANGE || errno == EINVAL || *err != '\0')
    return S_WRONG_PARAMS;

  return S_OK;
}

/* Funkce AllocLines alokuje pamet pro jednotlive radky textu. */
int AllocLines(char *lineField[], int n)
{
  for (int i = 0; i < n; i++) {
    if ((lineField[i] = (char *) calloc(sizeof (char), (LINE_LEN + 1) * sizeof(char))) == NULL) {
      FreeLines(lineField, i-1);
      return S_ALLOC;
    }
  }

  return S_OK;
}

/* Funkce FreeLines uvolni alokovanou pamet. */
void FreeLines (char *lineField[], int i)
{
  while (i >= 0)
    free(lineField[i--]);

  return;
}

/* Funkce GetLines nacita do kruhoveho pole tvoreneho buffery o delce 
 * LINE_LEN, dokud nenarazi na konec souboru. */
int GetLines(Tparams *params, char *lineField[], int *printFrom, FILE *file)
{
  int token;	// Pomocna promenna.
  int r = 0;	// Indexace pole ukazatelu na znak.
  int lines = params->lines;
  
  // Pocet tisknutych radku je 0.
  if (params->lines == 0)
    return S_OK;
  
  // Nacitani radku do kruhoveho bufferu.
  while ((fgets(lineField[r % lines], LINE_LEN, file)) != NULL) {
    // Delka radku dosahla maxima.
    if (strlen(lineField[r % lines]) == LINE_LEN - 1) {
      // Posledni znak neni znak noveho radku.
      if (lineField[r % lines][LINE_LEN - 2] != '\n') {
	// Nasledujici znak je znak noveho radku.
	if ((token = fgetc(file)) == '\n') {
	  lineField[r % lines][LINE_LEN - 1] = '\n';
	  lineField[r % lines][LINE_LEN] = '\0';
	}
	// Nasledujici znak neni znak noveho radku ani konce souboru.
	else if (token != EOF) {
	  if (!error_printed) {
	    fprintf(stderr, "%s", errCodeMsg[S_LINE_LEN]);
	    error_printed = true;
	  }
	  lineField[r % lines][LINE_LEN - 1] = '\n';
	  lineField[r % lines][LINE_LEN] = '\0';
	}
	ungetc(token, file);
	while (((token = fgetc(file)) != '\n') && (token != EOF));
      }
    }
    r++;
  }
  
  // Nastaveni radku, od nejz se bude tisknout.
  *printFrom = r;

  return S_OK;
}

/* Funkce PrintLast tiskne obsah kruhove pole bufferu od 
 * indexu, ktery predstavuje formalni parametr printFrom. */
void PrintLast (Tparams *params, char *lineField[], int printFrom)
{
  for (int i = 0; i < params->lines; i++, printFrom++)
    printf ("%s", lineField[printFrom % params->lines]);
}

/* Funkce SkipFirstLines nacte a zahodi nekolik prvnich radku 
 * jejich pocet je dan parametrem lines. */ 
void SkipFirstLines (char *lineField[], int lines, FILE *file)
{
  int token;	// Pomocna promenna.
  
  for (int count = 1; count < lines; count++) {
    while ((token = fgetc(file)) != '\n') {
      if (token == EOF)
	return;
    }
  }
  
  return;
}

/* Funkce PrintFrom tiskne znaky az do konce souboru */
void PrintFrom (FILE *file)
{
  int token;	// Pomocna promenna.
  
  while ((token = fgetc(file)) != EOF)
    putchar(token);
  
  return;
}


// -- FUNKCE MAIN --
// =================

int main (int argc, char *argv[])
{
  Tparams params = {M_LAST, LINE_COUNT, NULL};	// Struktura pro ulozeni parametru.
  FILE *file = stdin;				// Ukazatel na soubor.
  int printFrom = 0;				// Radek bufferu, od nejz probehne tisk.

  // Zpracovani parametru. Musi byt zadan aspon 1 parametr,
  if ((argc > 1) && (GetParams(argc, argv, &params) != S_OK)) {
    fprintf(stderr, "%s", errCodeMsg[S_WRONG_PARAMS]);
    return EXIT_FAILURE;
  }

  // Otevreni souboru pro cteni.
  if (params.fileName != NULL) {
    if ((file = fopen(params.fileName, "r")) == NULL) {
      fprintf(stderr, "%s", errCodeMsg[S_FILE]);
      return EXIT_FAILURE; 
    }
  }

  // Definice pole ukazatelu na retezce (jednotlive radky textu).
  char *lineField[params.lines];
  // Alokace pameti pro jednotlive radky vstupniho textu.
  if (AllocLines(lineField, params.lines) != S_OK) {
    fclose(file);
    fprintf(stderr, "%s", errCodeMsg[S_ALLOC]);
    return EXIT_FAILURE; 
  }
  
  // Program spusten v modu "tisk poslednich N radku".
  if (params.mode == M_LAST) {
    if (GetLines(&params, lineField, &printFrom, file) != S_OK) {
      FreeLines(lineField, params.lines-1);
      fclose(file);
      fprintf(stderr, "%s", errCodeMsg[S_LINE_LEN]);
      return EXIT_FAILURE;
    }
    PrintLast(&params, lineField, printFrom);
  }
  // Program spusten v modu "tisk od N-teho radku".
  else if (params.mode == M_FROM) {
    SkipFirstLines(lineField, params.lines, file);
    PrintFrom(file);
  }
  // Uvolneni alokovane pameti.
  FreeLines(lineField, params.lines-1);
  // Uzavreni souboru
  fclose(file);

  return EXIT_SUCCESS;
}
