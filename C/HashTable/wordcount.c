/* SOUBOR: wordcount.c
 * PROJEKT: IJC-DU2
 * DATUM: 11.4.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz
 * FAKULTA: FIT VUT Brno
 * PRELOZENO: gcc 4.4.5
 * POPIS: Program wordcount vypise pro dany vstupni text 
 * jednotliva slova a jejich cetnosti.
 */

// Hlavickove soubory.
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "htable.h"
#include "io.h"

/* Velikost tabulky. 
 * Hodnota byla zvolena vzhledem k mnozstvi anglickych slov. 
 * Uznavany anglicky slovnik Oxford English Dictionary obsahuje 
 * pres 600000 slov (zdroj: www.oup.com). Vychazim-li z 
 * predpokladu, ze se slova idealne rovnomerne rozmisti po 
 * hashovaci tabulce, pricemz akceptuji maximalne 10prvkove 
 * seznamy kvuli akceptovatelne efektivite, potom hledam hodnotu 
 * 600000 / 10 tak, aby se navic jednalo o prvocislo (udajne 
 * efektivnejsi vystup hash. funkce). Nejblizsi hodnota vychazi 60013. */
#define T_SIZE 60013	
// Maximalni delka nacteneho slova.
#define W_MAX 255	


// Chybova hlaseni programu.
const char *err_code_msg[] = {
  // S_OK
  "OK.",
  // S_ALLOC
  "CHYBA: Nepodarilo se alokovat pamet.\n",
  // S_WMAX
  "CHYBA: Vstupni slovo bylo prilis dlouhe, zkraceno.\n",
};

// Vycet chybovych stavu programu.
enum err_states {
  // Vse v poradku.
  S_OK,
  // Chyba pri alokaci pameti.
  S_ALLOC,
  // Nacteno prilis dlouhe slovo.
  S_WMAX,
};

/* Promenna nese informaci, zda uz bylo tiksnuto chybove hlaseni 
 * o prekroceni max. delky nacteneho slova. */
bool error_printed = false;

/* Funkce vytiskne chybove hlaseni programu 
 * a ukoncuje program s chybovym kodem. */
void print_err_msg(int state)
{
  fprintf(stderr, "%s", err_code_msg[state]);
  if (state != S_WMAX)
    exit(1);
}

/* Hlavni funkce programu */
int main (void)
{
  htable_t *t = NULL;	// Ukazatel na hashovaci tabulku.
  htable_listitem *tmp;	// Docasny ukazatel na prvek tabulky.
  char *word;		// Ukazatel na retezec (nacitane slovo).
  int l;		// Pomocna promenna.
  
  // Inicializace tabulky.
  if ((t = htable_init(T_SIZE)) == NULL)
    print_err_msg(S_ALLOC);
  
  // Alokace pameti pro nacitane slovo.
  if ((word = (char *) malloc((W_MAX+1)*sizeof(char))) == NULL) {
    htable_free(t);
    print_err_msg(S_ALLOC);
  }
  
  // Nacitani slov do tabulky.
  while ((l = fgetword(word, W_MAX + 1, stdin)) != 0) {
    // Nactene slovo bylo delsi, nez W_MAX.
    if ((l == W_MAX + 1) && (error_printed == false)) {
      print_err_msg(S_WMAX);
      error_printed = true;
    }
          
    // Ziskani ukazatele na vlkadany / existujici prvek.
    tmp = htable_lookup(t, word);
    
    // Pokud uz bylo dane slovo drive ulozeno, uvolneni pameti pro slovo.
    if (tmp->data > 0)
      free(word);
    
    tmp->data++;
    
    // Alokace pameti pro dalsi slovo.
    if ((word = (char *) malloc((W_MAX+1)*sizeof(char))) == NULL) {
      htable_free(t);
      print_err_msg(S_ALLOC);
    }      
  }
  free(word);
  
  // Tisk slov a jejich cetnosti.
  htable_iterator i = htable_begin(t);
  htable_iterator j = htable_end(t);
  for( ; !htable_iter_eq(i,j); i = htable_iter_next(i))
    printf("%s\t%u\n", htable_iter_deref(i)->key, htable_iter_deref(i)->data);

  // Uvolneni pameti alokovane pro tabulku.
  htable_free(t);
  
  return 0;
}
