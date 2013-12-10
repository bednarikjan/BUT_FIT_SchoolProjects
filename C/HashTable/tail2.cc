/* SOUBOR: tail.c	
 * PROJEKT: IJC-DU2
 * DATUM: 6.3.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz
 * FAKULTA: FIT VUT Brno
 * PRELOZENO: g++ 4.4.5
 * POPIS: Program tail tiskne poslednich 10 radku zadaneho souboru.
 * Chovani programu a pocet tisknutych radku je mozno menit pomoci 
 * prepinace -n a poctu radku.
 */

// -- SYSTEMOVE HLAVICKOVE SOUBORY --
// ==================================

#include <iostream>
#include <fstream>
#include <deque>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cerrno>

using namespace std;


// -- DEFINICE GLOBALNICH PROMENNYCH --
// ====================================

// Chybova hlaseni programu.
const char *errCodeMsg[] =
{
  // S_OK
  "OK.\n",
  // S_WRONG_PARAMS
  "Byly zadany chybne parametry.\n",
  //S_FILE
  "Textovy soubor nelze otevrit.\n",
};


// -- LOKALNI DEFINICE NOVYCH TYPU --
// ==================================

// Struktura uchovavajici parametry zadane pri spusteni.
typedef struct {
  int mode;
  long lines;
  string(fileName);
} Tparams;


// -- UPLNE FUNKCNI PROTOYPY LOKALNICH FUNKCI --
// =============================================

int GetParams (int argc, char *argv[], Tparams *params);
int ExtractNumber (char *number, Tparams *params);
void PrintFrom (int n, string fileName, ifstream *file);
void PrintLast (int n, string fileName, ifstream *file);


// Chybove stavy programu.
enum ErrorStates {
  // OK
  S_OK,
  // Chybne zadane parametry programu.
  S_WRONG_PARAMS,
  // Chyba pri otevirani souboru.
  S_FILE
};

// Rezimy programu tail.
enum TailMode {
  // Tisk poslednich N radku.
  M_LAST,
  // Tisk od N-teho radku.
  M_FROM
};


// -- DEFINICE LOKALNICH FUNKCI --
// ===============================

/* Funkce GetParams zpracuje parametry z prikazove radky.
 * Pri vyskytu nelegalne zadanych parametru vraci chybovy
 * kod. */
int GetParams (int argc, char *argv[], Tparams *params)
{
  // Vycet typu nasledujiciho ocekavaneho parametru.
  enum {PARAM_N, NUMBER, FILE} next;
  string n = "-n";

  if (argc > 4)
    return S_WRONG_PARAMS;

  // Prvni parametr je prepinac "-n".
  if (n.compare(argv[1]) == 0) {
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
	if (n.compare(argv[a]) != 0 || argc == a+1)
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
  char *err;	// Pripadny nelegalni znak.
  
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

/* Funkce PrintFrom tiskne obsah vstupniho textu od radku 
 * zadaneho parametrem n. */
void PrintFrom (int n, string fileName, ifstream *file)
{
  char c; // Pomocna promenna.
  
  // Preskoceni prvnich n radku.
  for (int i = 1; i < n; i++) {
    while (file->get() != '\n') {
      if (file->eof())
	break;
    }
  }
  // Tisk zbyvajicich znaku.
  while ((c = file->get()) != EOF)
    cout.put(c);
}

/* Funkce PrintLast vytiskne poslednich N radku vstupniho 
 * textu, kdy pocet radku je dany parametrem n. */
void PrintLast (int n, string fileName, ifstream *file)
{
  string tmp;		// Pomocna promenna.
  deque<string> lines;	// Obousmerna fronta.
  
  // Nacteni prvnich n radku vstupniho textu.
  for (int i = 0; i < n && !file->eof(); i++) {
    if (file->peek() == EOF)
      break;	
    getline(*file, tmp);
    lines.push_front(tmp);
  } 
  
  // Postupne vkladani radku na zacatek a ruseni radku na konci fronty.
  while (!file->eof()) {
    if (file->peek() == EOF)
      break;	
    getline(*file, tmp);
    lines.push_front(tmp);
    lines.pop_back();
  }
  
  // Tisk radku ulozenych ve fronte.
  while (!lines.empty()) {
    cout << lines.back() << endl;
    lines.pop_back();
  }
}

// -- FUNKCE MAIN --
// =================

int main (int argc, char *argv[])
{
  Tparams params = {M_LAST, 10, };	// Struktura pro ulozeni parametru.
  ifstream *file = (ifstream *) &cin;	// Ukazatel na datovy proud pro cteni.
  ifstream tmpfile;
  
  // Zpracovani parametru. Musi byt zadan aspon 1 parametr,
  if ((argc > 1) && (GetParams(argc, argv, &params) != S_OK)) {
    cerr << errCodeMsg[S_WRONG_PARAMS];
    return EXIT_FAILURE;
  }
  
  // Otevreni souboru pro cteni.
  if (!params.fileName.empty()) {
    tmpfile.open(params.fileName.c_str());
    if (cin.fail()) {
      cerr << errCodeMsg[S_FILE];
      return EXIT_FAILURE;
    }
    file = &tmpfile;
  }
  
  // Program spusten v modu "tisk poslednich N radku".
  if (params.mode == M_LAST)
    PrintLast(params.lines, params.fileName, file);
  else if (params.mode == M_FROM)
    PrintFrom(params.lines, params.fileName, file);
  
  file->close();
  
  return EXIT_SUCCESS;
}
