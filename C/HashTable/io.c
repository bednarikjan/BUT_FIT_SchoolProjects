/* SOUBOR: io.c
 * PROJEKT: IJC-DU2
 * DATUM: 17.4.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz
 * FAKULTA: FIT VUT Brno
 * PRELOZENO: gcc 4.4.5
 * POPIS: Modul io.c programu wordcount.
 */

#include <ctype.h>
#include <stdio.h>

/* Funkce nacte slovo ze souboru f, ulozi do pole s 
 * a vrati jeho delku. Pokud slovo presahne max-1 znaku, 
 * vraci max. */
int fgetword(char *s, int max, FILE *f)
{
  int c;	// Pomocna promenna.
  int i = 0;	// Indexacni promenna.
  
  // Preskoceni pripadnych pocatecnich mezer.
  while (isspace(c = fgetc(f)));
  ungetc(c, f);
  
  while (!isspace(c = fgetc(f)) && c != EOF) {
    // Prilis dlouhe slovo.
    if (i == max - 1) {
      // Preskoci zbytek slova.
      fscanf(f, "%*s");
      s[i] = '\0';
      return max;
    }
    s[i++] = c;
  }
  // Vlozeni zarazky za konec slova.
  s[i] = '\0';
  
  return i;
}
