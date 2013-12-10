/* SOUBOR: enum.h
 * PROJEKT: IJC-DU1, priklad a)
 * DATUM: 6.3.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz 
 * FAKULTA: FIT VUT Brno 
 * PRELOZENO: gcc 4.4.5
 * POPIS: Hlavickovy soubor pro modul enum.c
 */

#ifndef ENUM_H
#define ENUM_H

// -- DEFINICE VYCTOVEHO TYPU POUZIVANEHO I V JINYCH MODULECH --
// =============================================================

enum months { 
ChybnyMesic=0,
Leden=1,
Unor,
Brezen, 
Duben,
Kveten,
Cerven,
Cervenec,
Srpen,
Zari,
Rijen,
Listopad,
Prosinec,
};

// -- UPLNE FUNKCNI PROTOTYPY GLOBALNICH FUNKCI MODULU ENUM.C --
// =============================================================

extern void PrintMonthShort(enum months d);
extern void PrintMonth(enum months d);
extern enum months ReadMonth(void);

#endif
