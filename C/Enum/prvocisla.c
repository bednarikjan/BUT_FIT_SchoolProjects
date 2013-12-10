/* SOUBOR: prvocisla.c
 * PROJEKT: IJC-DU1, priklad b)
 * DATUM: 13.3.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz
 * FAKULTA: FIT VUT Brno
 * PRELOZENO: gcc 4.4.5
 * POPIS: Programy prvocisla a prvocisla-inline definuji makra 
 * s parametry pro praci s bitovym polem. Jako testovaci 
 * priklad je implementovano Eratostenovo sito pro vypocet 
 * prvocisel v rozsahu 0-90000000 a vypis poslednich dvaceti 
 * z nich. Program prvocisla pouziva makra s parametry a 
 * program prvocisla-inline ekvivalentni inline funkce.
 */

// -- SYSTEMOVE HLAVICKOVE SOUBORY -- 
// ==================================

#include <stdio.h> //printf
#include <stdlib.h>
#include <math.h> // sqrt
#include <limits.h> 


// -- VLASTNI HLAVICKOVE SOUBORY --
// ================================

#include "error.h"


// -- LOKALNI DEFINICE SYMBOLICKYCH KONSTANT --
// ============================================

// Delka dat. typu unsigned long v bitech.
#define ULONG (CHAR_BIT*sizeof(long))

// Delka bitoveho pole pro realizaci Erathostenova sita (max 2^(sizeof(int)*CHAR_BIT -1))
#define ERAT_LENG_BIT 90000001

// Pocet poslednich prvocisel, ktera nas zajimaji.
#define LAST_PRIMES 20

//Odmocnina z delky bitoveho pole pro realizaci Erathostenova sita.
#define ERAT_LENG_BIT_SQRT sqrt(ERAT_LENG_BIT)


// -- LOKALNI DEFINICE MAKER S PARAMETRY --
// ============================================

/* Definuje pole bitu o zadane velikosti reprezentovane dat. typem unsigned long int.
 * Inicializuje vsechny prvky pole na hodnotu 0. Definuje promennou dat. typu
 * unsigned long, ktera uchovava informaci o delce daneho pole (v bitech). */
#define BitArray(arrayName, length) \
    unsigned long arrayName [(length)/ULONG + ((length)%ULONG ? 1 : 0) + 1] = {0}; \
    arrayName[0] = length
    

/* Pri prekladu NEbyla definovana konstanta USE_INLINE. */
#if !defined USE_INLINE


/* Nastavi bit daneho pole na danem indexu na hodnotu podminenou vysledkem daneho vyrazu.
 * Nulovy vysledek vyrazu nastavi dany byt na 0, nenulovy na 1. */
#define SetBit(arrayName, idx, expr) \
  do { \
    if ((idx) >= 0 && (idx) < arrayName[0]) { \
      if (expr) arrayName[1 + (idx)/ULONG] |= (unsigned long) 1 << (idx) % ULONG; \
      else arrayName[1 + (idx)/ULONG] &= ~(unsigned long) 1 << (idx) % ULONG; \
    } \
    else Error("Index %ld mimo rozsah 0..%ld", (long)(idx), (long)arrayName[0] - 1); \
  } while (0)

/* Ziska hodnotu zadaneho bitu adresovaneho danym indexem. Vraci hodnotu 0, nebo 1. */
#define GetBit(arrayName, idx) \
  ((idx) >= 0 && (idx) < arrayName[0] ? \
  arrayName[1 + (idx)/ULONG] >> (idx) % ULONG & 1 : \
  (Error("Index %ld mimo rozsah 0..%lu", (long)(idx), (long)arrayName[0] - 1), 0))


/* Pri prekladu byla definovana konstanta USE_INLINE. */
#else


// -- DEFINICE INLINE FUNKCI --
// ============================

/* Inline funkce zastavajici funkci makra SetBit. */
inline void SetBit(unsigned long arrayName[], unsigned long idx, int expr)
{
  if(idx >= 0 && idx < arrayName[0]) { 
    if(expr)
      arrayName[1 + (idx)/ULONG] |= (unsigned long) 1 << (idx) % ULONG;
    else
      arrayName[1 + (idx)/ULONG] &= ~(unsigned long) 1 << (idx) % ULONG;
  }
  else
    Error("Index %ld mimo rozsah 0..%ld", (long)(idx), (long)arrayName[0] - 1);
  
  return;
}

/* Inline funkce zastavajici funkci makra GetBit. */
inline unsigned int GetBit(unsigned long arrayName[], unsigned long idx)
{
  if(idx >= 0 && idx < arrayName[0])
    return (arrayName[1 + (idx)/ULONG] >> (idx % ULONG)) & 1;
  else {
    Error("Index %ld mimo rozsah 0..%lu", (long)(idx), (long)arrayName[0] - 1);
    return EXIT_FAILURE;
  }
}


/* USE_INLINE */
#endif


// -- FUNKCE MAIN --
// =================

int main()
{
  BitArray(bitfield, ERAT_LENG_BIT);	// Definice bitoveho pole.
  unsigned long idx_a, idx_b;		// Indexace bitoveho pole.
  int count;				// Citac poslednich LAST_PRIMES prvocisel.
  int primes[LAST_PRIMES];		// Pole pro ulozeni indexu poslednich LAST_PRIMES prvocisel.
  
  // Implementace Erathostenova sita.
  for(idx_a = 2; idx_a <= ERAT_LENG_BIT_SQRT; idx_a++) {
    if(GetBit(bitfield,idx_a)) 
      continue;
    for(idx_b = 2*idx_a; idx_b < ERAT_LENG_BIT; idx_b+=idx_a)
      SetBit(bitfield, idx_b, 1);
  }
  
  // Ulozeni indexu poslednich LAST_PRIMES prvocisel do pole primes.
  for (idx_a = ERAT_LENG_BIT - 1, count = 0; count < LAST_PRIMES; idx_a--) {
    if(!GetBit(bitfield, idx_a)) {
      primes[count] = idx_a;
      count++;
    }
  }

  // Tisk indexu ulozenych v poli primes (od posledniho k prvnimu).
  for(count = LAST_PRIMES-1; count >= 0; count--)
    printf("%d\n", primes[count]);

 return EXIT_SUCCESS;
}
