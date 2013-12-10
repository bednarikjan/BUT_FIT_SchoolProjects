/* SOUBOR: hash_function.c
 * PROJEKT: IJC-DU2
 * DATUM: 17.4.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz
 * FAKULTA: FIT VUT Brno
 * PRELOZENO: gcc 4.4.5
 * POPIS: Modul hash_function.c knihovny libhtabel.a 
 * a libhatable.so programu wordcount.
 */

/* Funkce vypocita ze zadaneho retezce index do hashovaci tabulky. */
unsigned int hash_function(const char *str, unsigned htable_size) 
{
  unsigned int h=0;
  unsigned char *p;
  
  for(p=(unsigned char*)str; *p!='\0'; p++)
    h = 31*h + *p;
  
  return h % htable_size;
}
