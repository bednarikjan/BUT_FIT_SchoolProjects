/* SOUBOR: barbers.c
 * PROJEKT: IOS-DU2
 * DATUM: 21.4.2011
 * AUTOR: Jan Bednarik (xbedna45), xbedna45@stud.fit.vutbr.cz
 * FAKULTA: FIT VUT Brno
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>	// srand, rand
#include <stdio.h>	// setbuf
#include <stdbool.h>
#include <sys/types.h>	// key_t, pid_t
#include <sys/wait.h>	// waitpid
#include <sys/ipc.h>	// ftok
#include <sys/shm.h>	// shmget, shmat, shmdt, shmctl
#include <semaphore.h>	// sem_t, sem_open, sem_close, sem_unlink, sem_wait, sem_post
#include <fcntl.h>	// O_* konstanty
#include <unistd.h>	// fork, usleep
#include <time.h>	// time
#include <assert.h>
#include <signal.h>	// sigaction, kill

// Pocet ciselnych parametru programu.
#define PAR_NUM 4
// Pocet pouzivanych sdilenych promennych.
#define SH_VAR 3
// Pocet pouzivanych semaforu
#define SEMS 7
// Pocet semaforu inicializovanych na 0.
#define SEM_ZERO 4
// Informace pro fci Clean, ze nema danou pamet uklizet.
#define NO -1

// Makro pro nastaveni hodnot do pole cleanFlgs tak, aby fce Clean uklidila vse.
#define SetCleanFlgs()	cleanFlgs[1] = SH_VAR-1; cleanFlgs[0] = SH_VAR-1; \
			cleanFlgs[2] = SEMS-1; cleanFlgs[3] = SEMS-1

/* Makro pro nastaveni hodnot do pole cleanFlgs tak, aby fce Clean pouzila pouze
 * funkce shmdt a sem_close (uklizeni potomku). */
#define SetCleanFlgsChild()	cleanFlgs[1] = -1; cleanFlgs[0] = SH_VAR-1; \
				cleanFlgs[2] = SEMS-1; cleanFlgs[3] = -1

// Makro pro uzamceni semaforu s kontrolou chyby a pripadneho vraceni chyb. kodu.
#define SemWait(x) \
    if (sem_wait(sems[x]) == -1) \
      return S_SEMWAIT \

// Makro pro odemceni semaforu s kontrolou chyby a pripadneho vraceni chyb. kodu.
#define SemPost(x) \
    if (sem_post(sems[x]) == -1) \
      return S_SEMPOST \

// Parametry pri spusteni programu.
typedef struct {
  long Q;	// Pocet kresel.
  long GenC;	// Max. cas pro generovani noveho zakaznika (ms).
  long GenB;	// Max. cas pro strihani zakaznika (ms).
  long N;	// Pocet zakazniku.
  const char *f;// Jmeno souboru pro zapis.
} TParams;

// Deklarace lokalnich funkci.
int GetParams (TParams *prms, int argc, char *argv[]);
void Error (int s);
void Clean ();
int Barber (unsigned *shmPtr[], sem_t *sems[], FILE *file, TParams *params);
int Customer (unsigned *shmPtr[], sem_t *sems[], FILE *file, int i);
void Wexit (pid_t procs[], TParams *params);

// Globalni promenne.
/* Indexy, podle nichz fce Clean pozna, od kterych indexu ma uklizet
 * pole shmPtr, shmid a sems. Hodnota NO urcuje, ze se dane pole nema 
 * uklizet. Vyznam podle poradi: 1. shmdt, 2. shmctl, 3. sem_close 
 * 4. sem_unlink. */ 
int cleanFlgs[4] = {NO, NO, NO, NO};
int shmid[SH_VAR] = {0};		// Pole identifikatoru sdil. pameti.
unsigned *shmPtr[SH_VAR] = {NULL};	// Pole ukazatelu na sdilene promenne.
sem_t *sems[SEMS] = {NULL};		// Pole ukazatelu na pojmenovane semafory.
const char *pathname = NULL;		// Argument path pro fci ftok.
FILE *file = NULL;			// Ukazatel na vystupni soubor.

// Chybove stavy programu.
enum {
  S_OK,		// Vse OK.
  S_PARAMS,	// Chybne zadane parametry.
  S_NOCUSTOMER,	// Bylo zadano 0 zakazniku (parametr N).
  S_KEY,	// Nepodarena generace klice.
  S_SHMGET,	// Nepodarilo se ziskat sdilenou pamet.
  S_SHMAT,	// Nepodarilo se namapovat sdilenou pamet.
  S_SEMOPEN,	// Nepodarilo se vytvorit semafor.
  S_FORK,	// Chyba pri volani fork.
  S_FILE,	// Chyba pri otvirani (vytvareni) souboru pro zapis.
  S_SEMWAIT,	// Nepodarilo se uzamknout semafor.
  S_SEMPOST,	// Nepodarilo se odemknout semafor.
};

// Odkazy na sdilene promenne do pole shmPtr.
enum {  
  SHM_ACT,	// Cislovani provadenych akci.
  SHM_FPNUM,	// Pocet volnych mist v cekarne.
  SHM_CUST,	// Pocet odchozich zakazniku.
};

// Odkazy na semafory do pole sems.
enum {
  SEM_BSL,	// Spanek holice (ceka na probuzeni zakaznikem).
  SEM_BCH,	// Pristup barbera k holic. kreslu.
  SEM_CCH,	// Pristup customera k holic. kreslu.
  SEM_CEX,	// Vychod z holicstvi.
  SEM_FPNUM,	// Pristup k promenne s poctem volnych mist v cekarne.
  SEM_CNUM,	// Pristup k promenne s poctem odchozich zakazniku.  
  SEM_ANUM,	// Pristup k promenne s cislovanim provadenych akci.
};

// Jmena semaforu.
const char *semNames[SEMS] = {
  "/sBarbSleep",	// SEM_BSL  
  "/sBarbChair",	// SEM_BCH
  "/sCustChair",	// SEM_CCH
  "/sCustExit",		// SEM_CEX
  "/sFreePlNum", 	// SEM_FPNUM
  "/sCustNum",		// SEM_CNUM
  "/sActNum",		// SEM_ANUM
};

// Chybova hlaseni programu.
const char *errMessage[] = {
  // S_OK
  "OK",
  // S_PARAMS
  "CHYBA: Byly zadany chybne parametry.\n",
  // S_NOCUSTOMER
  "CHYBA: Nyl zadan nulovy pocet zakazniku.\n",
  // S_KEY
  "CHYBA: Nepodarilo se vygenerovat klic pro pristup ke sdilene pameti (ftok).\n",
  // S_SHMGET
  "CHYBA: Nepodarilo se ziskat sdilenou pamet (shmget).\n",
  // S_SHMAT
  "CHYBA: Nepodarilo se namapovat sdilenou pamet (shmat).\n",
  // S_SEMOPEN
  "CHYBA: Nepodarilo se vytvorit semafor (sem_open).\n",
  // S_FORK
  "CHYBA: Nastala chyba pri tvorbe procesu (fork).\n",
  // S_FILE
  "CHYBA: Nepodarilo se otverit/vytvorit soubor pro zapis (fopen).\n",
  // S_SEMWAIT
  "CHYBA: Nepodarilo se uzamknout semafor (sem_wait).\n",
  // S_SEMPOST
  "CHYBA: Nepodarilo se odemknout semafor (sem_post).\n",
};

/* Funkce vytiskne chybove hlaseni a ukonci program 
 * s chybovym navratovym kodem. */
void Error (int s)
{
  fprintf(stderr, "%s", errMessage[s]);
  exit(1);
}

/* Funkce zpracuje parametry. V pripade chyby vraci 
 * chybovy kod. */
int GetParams (TParams *params, int argc, char *argv[])
{
  char *err = NULL;	// Pripadny chybny znak v ciselne hodnote.
  long val;		// Nactena ciselna hodnota.
  
  // Chybny pocet parametru.
  if (argc != 6)
    return S_PARAMS;
  
  errno = 0;

  // Zpracovani ciselnych parametru.
  if (((val = strtol(argv[1], &err, 10)) < 0) || *err != '\0' || errno == ERANGE)
      return S_PARAMS;
  params->Q = val;
  if (((val = strtol(argv[2], &err, 10)) < 0) || *err != '\0' || errno == ERANGE)
      return S_PARAMS;
  params->GenC = val;
  if (((val = strtol(argv[3], &err, 10)) < 0) || *err != '\0' || errno == ERANGE)
      return S_PARAMS;
  params->GenB = val;
  if (((val = strtol(argv[4], &err, 10)) < 0) || *err != '\0' || errno == ERANGE)
      return S_PARAMS;
  params->N = val;

  // Zpracovani jmena souboru.
  strcmp(argv[5], "-") == 0 ? (params->f = NULL) : (params->f = argv[5]);
  
  return S_OK;
}


/* Funkce clean uklizi po funkcich shmat, shmget, 
 * sem_open. Parametr cleanFlgs urcuje, ktera pole 
 * a od kterych indexu se maji uklizet. */
void Clean ()
{
  // Odpojeni sdilene pameti z adresoveho prostoru volajiciho procesu.
  if (cleanFlgs[0] != NO) {
    for (int i = cleanFlgs[0]; i >= 0; i--) {
      if (shmdt(shmPtr[i]) == -1)
	fprintf(stderr, "CHYBA: Nepodarilo se odpojit sdilenou pamet (shmdt).\n");
    }
  }
  
  // Zruseni segmentu sdilene pameti.
  if (cleanFlgs[1] != NO) {
    for (int i = cleanFlgs[1]; i >= 0; i--) {
      if (shmctl(shmid[i], IPC_RMID, NULL) == -1)
	fprintf(stderr, "CHYBA: Nepodarilo se zrusit segment sdilene pameti (shmctl).\n");
    }
  }
  
  // Zavreni semaforu semaforu.
  if (cleanFlgs[2] != NO) {
    for (int i = cleanFlgs[2]; i >= 0; i--) {
      if (sem_close(sems[i]) == -1)
	fprintf(stderr, "CHYBA: Nepodarilo se zavrit semafor (sem_close).\n");
    }
  }
  
  // Smazani semaforu.
  if (cleanFlgs[3] != NO) {
    for (int i = cleanFlgs[2]; i >= 0; i--) {
      if (sem_unlink(semNames[i]) == -1)
	fprintf(stderr, "CHYBA: Nepodarilo se smazat semafor (sem_unlink).\n");
    }
  }
  
  // Zavreni souboru.
  fclose(file);
  
  return;
}

// FUNKCE CUSTOMER
//--------------------------------------------------------------------------------------//
int Customer (unsigned *shmPtr[], sem_t *sems[], FILE *file, int i)
{
  // Zamceni semaforu cislovani akci.
  SemWait(SEM_ANUM);
    fprintf(file, "%d: customer %d: created\n", (*shmPtr[SHM_ACT])++, i);
  //Odemceni semaforu cislovani akci.
  SemPost(SEM_ANUM);
  
  // Zamceni semaforu poctu volnych mist.
  SemWait(SEM_FPNUM);  
    // Zamceni semaforu cislovani akci.
    SemWait(SEM_ANUM);
      fprintf(file, "%d: customer %d: enters\n", (*shmPtr[SHM_ACT])++, i);
    //Odemceni semaforu cislovani akci.
    SemPost(SEM_ANUM);
    // V cekarne neni zadne volne misto.
    if (*shmPtr[SHM_FPNUM] == 0) {
      // Zamceni semaforu cislovani akci.
      SemWait(SEM_ANUM);
	fprintf(file, "%d: customer %d: refused\n", (*shmPtr[SHM_ACT])++, i);
      // Odemceni semaforu cislovani akci.
      SemPost(SEM_ANUM);
      // Zamceni semaforu poctu odchozivsich zakazniku.
      SemWait(SEM_CNUM);
	(*shmPtr[SHM_CUST])++;
      // ODemceni semaforu poctu odchozivsich zakazniku.
      SemPost(SEM_CNUM);
      // Odemceni semaforu poctu volnych mist.
      SemPost(SEM_FPNUM);
      return S_OK;
    }
    // Obsazeni mista v cekarne.
    (*shmPtr[SHM_FPNUM])--;
  // Odemceni semaforu poctu volnych mist.
  SemPost(SEM_FPNUM);
  
  // Odemceni semaforu spanku holice.
  SemPost(SEM_BSL);
  
  // Zamceni semaforu vstupu zakaznika do holic. kresla.
  SemWait(SEM_CCH);
  
  // Zamceni semaforu cislovani akci.
  SemWait(SEM_ANUM);
    fprintf(file, "%d: customer %d: ready\n", (*shmPtr[SHM_ACT])++, i);
  //Odemceni semaforu cislovani akci.
  SemPost(SEM_ANUM);
  
  // Odemceni semaforu pristupu holice k holic. kreslu.
  SemPost(SEM_BCH);
  
  // Zamceni semaforu odchodu zakaznika z holicstvi.
  SemWait(SEM_CEX);
  
  // Zamceni semaforu cislovani akci.
  SemWait(SEM_ANUM);
    fprintf(file, "%d: customer %d: served\n", (*shmPtr[SHM_ACT])++, i);
  //Odemceni semaforu cislovani akci.
  SemPost(SEM_ANUM);      
  
  return S_OK;
}

// FUNKCE BARBER
//--------------------------------------------------------------------------------------//
int Barber (unsigned *shmPtr[], sem_t *sems[], FILE *file, TParams *params)
{
  long custLeft = 0;	// Pocet odchozivsich zakazniku.
    
  // V cekarne nejsou zadna mista, barber neprovadi zadnou akci.
  if (params->Q == 0)
    return S_OK;
  
  // Nastaveni zakladu pro generovani nahodnych cisel.
  srand((unsigned) time(NULL));
  
  while (custLeft != params->N) {
    // Zamceni semaforu cislovani akci.
    SemWait(SEM_ANUM);
      fprintf(file, "%d: barber: checks\n", (*shmPtr[SHM_ACT])++);
    //Odemceni semaforu cislovani akci.
    SemPost(SEM_ANUM);
  
    // Zamceni semaforu spanku holice.
    SemWait(SEM_BSL);
  
    // Zamceni semaforu poctu volnych mist.
    SemWait(SEM_FPNUM);
      // Zvyseni poctu volnych mist, pokud neni cekarna cela volna.
      if (*shmPtr[SHM_FPNUM] != (unsigned) params->Q)
	(*shmPtr[SHM_FPNUM])++;
      // Zamceni semaforu cislovani akci.
      SemWait(SEM_ANUM);
	fprintf(file, "%d: barber: ready\n", (*shmPtr[SHM_ACT])++);
      // Odemceni semaforu cislovani akci.
      SemPost(SEM_ANUM);
    // Odemceni semaforu poctu volnych mist.
    SemPost(SEM_FPNUM);
  
    // Odemceni semaforu vstupu zakaznika do holic. kresla.
    SemPost(SEM_CCH);
  
    // Zamceni semaforu vstupu holice k holic. kreslu.
    SemWait(SEM_BCH);
    
    // Strihani zakaznika
    usleep((rand() % (params->GenB + 1)) * 1000);
  
    // Zamceni semaforu cislovani akci.
    SemWait(SEM_ANUM);
      fprintf(file, "%d: barber: finished\n", (*shmPtr[SHM_ACT])++);
    // Odemceni semaforu cislovani akci.
    SemPost(SEM_ANUM);
  
    // Odemceni semaforu odchodu zakaznika z holicstvi.
    SemPost(SEM_CEX);
  
    // Zamceni semaforu poctu odchozivsich zakazniku.
    SemWait(SEM_CNUM);
      (*shmPtr[SHM_CUST])++;
      custLeft = *shmPtr[SHM_CUST];
    // Odemceni semaforu poctu odchozivsich zakazniku.
    SemPost(SEM_CNUM);
  }
  
  return S_OK;
}

/* Funkce zasle vsem potomkum signal SIGTERM 
 * a pocka na jejich dokonceni. */
void Wexit (pid_t procs[], TParams *params)
{
  // Zasle SIGTERM.
  for (int j = 0; j < params->N + 1; j++) {
    if (procs[j] != 0)
      kill(procs[j], SIGTERM);
  }
  // Pocka na dokonceni ostatnich procesu.
  while (waitpid(0, NULL, WNOHANG) == 0);
  
  return;
}

/* Obsluha prichoziho signalu SIGTERM procesu potomku. */
void Terminate(int s)
{
  SetCleanFlgsChild();
  Clean();
  exit(s);
}


// HLAVNI FUNKCE PROGRAMU
//--------------------------------------------------------------------------------------//
int main (int argc, char *argv[])
{
  TParams params;	// Parametry programu.
  file = stdout;	// Inicializace souboru pro zapis.
  pathname = argv[0];
  
  // Zpracovani parametru.
  if (GetParams(&params, argc, argv) != S_OK)
    Error(S_PARAMS);
  
  // Jako parametr N (pocet zakazniku) bylo zadano 0.
  if (params.N == 0)
    Error(S_NOCUSTOMER);
  
  // Otevreni (a pripadne vytvoreni) souboru pro zapis.
  if (params.f != NULL) {
    if ((file = fopen(params.f, "w")) == NULL)
      Error(S_FILE);
  }
  
  // Nastaveni bufferu pro vyst. text do souboru na radk. buffer (stejne, jako u stdout).
  setvbuf(file, NULL, _IOLBF, 0);

  // Definice promennych pro praci se sdilenou pameti, procesy a semafory.
  key_t keys[SH_VAR];		// Pole klicu pro pristup ke sdil. pametem.
  pid_t procs[params.N + 1];	// Pole pid vytvorenych procesu (barber a customers).
  pid_t pid;			// Pomocna promenna (pro fork).
  char c = 'a';			// Pomocna promenna (id pro ftok).
  int state = S_OK;		// chybove stavy programu.
  struct sigaction action;	// Struktura pro fci sigaction.
  
  // Inicializace pole pid vytvorenych procesu.
  for (int i = 0; i < params.N+1; i++)
    procs[i] = 0; 
  
  // Generovani klicu pro pristup do sdil. pameti.
  for (int i = 0; i < SH_VAR; i++, c++) {
    if ((keys[i] = ftok(pathname, c)) == -1)
      Error(S_KEY);
  }
  
  // Ziskani identifikatoru pro sdileny segment.
  for (int i = 0; i < SH_VAR; i++) {
    if ((shmid[i] = shmget(keys[i], sizeof(unsigned), IPC_CREAT | IPC_EXCL | 0666)) 
	 == -1) {
      cleanFlgs[1] = i-1;
      Clean();
      Error(S_SHMGET);
    }
  }
  
  // Namapovani fyzicke pameti do adresoveho prostoru volajiciho procesu.
  for (int i = 0; i < SH_VAR; i++) {
    if ((shmPtr[i] = (unsigned *) shmat(shmid[i], NULL, 0)) == (void *) -1) {
      cleanFlgs[1] = SH_VAR-1; 
      cleanFlgs[0] = i-1;
      Clean();
      Error(S_SHMAT);
    }
  }
  
  // Vytvoreni pojmen. semaforu. Prvnich SEM_ZERO inicializovanych na 0, zbytek na 1.
  for (int i = 0; i < SEMS; i++) {
    if ((sems[i] = sem_open(semNames[i], O_CREAT | O_EXCL, 0666, i < SEM_ZERO ? 0 : 1)) 
      == SEM_FAILED) {
      cleanFlgs[1] = SH_VAR-1;
      cleanFlgs[0] = SH_VAR-1;
      cleanFlgs[2] = i-1;
      cleanFlgs[3] = i-1;
      Clean();
      Error(S_SEMOPEN);
    }
  }
  
  // Inicializace sdilenych promennych.
  *shmPtr[SHM_ACT] = 1;		// Citac provadenych akci.
  *shmPtr[SHM_CUST] = 0;	// Pocet odchozivsich zakazniku.
  *shmPtr[SHM_FPNUM] = params.Q;// Pocet volnych mist v cekarne.
  
  // Vytvoreni procesu holice.
  pid = fork();
  // Chyba.
  if (pid == -1) {
    SetCleanFlgs();
    Clean();
    Error(S_FORK);
  }
  // Proces rodice.
  else if (pid > 0)
    procs[0] = pid;
  // Proces potomka (holic).
  else if (pid == 0) {
    // Nastaveni obsluhy prichozich signalu.
    sigset_t mask;
    sigemptyset(&mask);
    action.sa_handler = Terminate;
    action.sa_mask = mask;
    action.sa_flags = 0;
    sigaction(SIGTERM, &action, NULL);
    // Volani funkce holice.
    state = Barber(shmPtr, sems, file, &params);
    action.sa_handler = SIG_IGN;
    sigaction(SIGTERM, &action, NULL);
    // Uklid pameti.
    SetCleanFlgsChild();
    Clean();
    // V pripade chyboveho navratu fce Barber tisk chyboveho hlaseni.
    if (state != S_OK)
      Error(state);
    exit(0);    
  }
  
  // Nastaveni zakladu pro generovani nahodnych cisel.
  srand((unsigned) time(NULL));
  // Vytvoreni procesu zakazniku.
  for (int i = 1; i <= params.N; i++) {
    // Prodleni pred generovanim noveho zakaznika.
    usleep((rand() % (params.GenC + 1)) * 1000);
    pid = fork();
    // Chyba.
    if (pid == -1) {
      Wexit(procs, &params);
      SetCleanFlgs();
      Clean();      
      Error(S_FORK);
    }
    // Proces rodice.
    else if (pid > 0)
      procs[i] = pid;
    // Proces potomka (zakaznici).
    else if (pid == 0) {
      // Nastaveni obsluhy prichozich signalu.
      sigset_t mask;
      sigemptyset(&mask);
      action.sa_handler = Terminate;
      action.sa_mask = mask;
      action.sa_flags = 0;
      sigaction(SIGTERM, &action, NULL);
      // Volani procesu zakaznika.
      state = Customer(shmPtr, sems, file, i);
      action.sa_handler = SIG_IGN;
      sigaction(SIGTERM, &action, NULL);
      // Uklid pameti.
      SetCleanFlgsChild();
      Clean();
      // V pripade chyboveho navratu fce Customer tisk chyboveho hlaseni.
      if (state != S_OK)
	Error(state);
      exit(0); 
    }
  }
  
  int status = 0;	// Status funkce waitpid.
  int j;		// Pomocna indexacni promenna.
  
  // Cekani na dokonceni potomku (holice a zkazniku).
  for (int i = 0; i < params.N + 1; i++) {
    pid = waitpid(0, &status, 0);
    // Vymaze skonceny proces z pole zijicich procesu.
    for (j = 0; procs[j] != pid; j++)
      assert(j < params.N + 1);
    procs[j] = 0;
    // V pripade chyb. navratov. kodu procesu zasle vsem zijicim procesum SIGTERM.
    if (WEXITSTATUS(status)) {
      Wexit(procs, &params);
      SetCleanFlgs();
      Clean();
      return EXIT_FAILURE;
    }
  }
  
  // Uklid.
  SetCleanFlgs();
  Clean();
  
  return EXIT_SUCCESS;
}
