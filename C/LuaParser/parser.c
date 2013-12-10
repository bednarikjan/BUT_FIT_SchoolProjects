/*
 * Implementace interpretu imperativniho jazyka IFJ2011
 *
 * Autori:
 *  xbedna45 - Bednarik Jan
 *  xblaha22 - Blaha Hynek
 *  xjanys00 - Janys Martin
 *  xmacha48 - Machacek Ondrej
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "parser.h"
#include "scaner.h"
#include "error.h"
#include "ilist.h"
#include "stack.h"
#include "str.h"
#include "stack_exp.h"

#define DEBUG 0
#define DEBUGEXP 0

// ---- Deklarace lokalnich funkci ----
// ====================================

// -- Zpracovani vyrazu --
int expression(int breakpoint);

// -- Pomocne funkce --
int generateInstruciton(operation instType, void *addr1, void *addr2, void *addr3);
int parseReadFormat(int *charNum);

// -- Rekurzivni sestup --
int Prog();
int RestOfFuncs();
int FormParams();
int FormParamsNext();
int Body();
int VarDecl();
int Stats();
int Assign(char *identificator);
int Liter();
int ParamsWrite();
int ParamsWriteNext();
int FuncOrExpr(stableItemPtr stItem);
int ActParams();
int ActParamsNext();
int IDorLiter();
//int ActParamsUsr();
//int ActParamsNextUsr();

// -- Pomocny seznam pro vzajemnou rekurzi --
void mrlInit(stableItemPtr*);
int mrlInsertFirst(stableItemPtr*, iItem*, char*);
void mrlDispose(stableItemPtr*);
int MutRec(stableItemPtr);


// ---- Makra ----
// ===============

// Ziskani dalsiho tokenu s overenim chyby.
#define GET_NEXT_TOKEN(t) \
  do { \
    getNextToken(t); \
    if ((t)->type == INTER_ERROR || (t)->type == LEX_ERROR) \
      return (t)->type; \
  } while(0)

// Ziskani dalsiho tokenu s pocitanim zavorek
#define GET_NEXT_TOKBRACKET(t) \
  do { \
    GET_NEXT_TOKEN(t); \
    if(token.type == L_BRACKET) bracketCounter++; \
    if(token.type == R_BRACKET) bracketCounter--; \
  } while(0)

// Kontrola, zda jiz dana funkce/promenna nebyla definovana.
#define LOOK_UP(table) (stableLookup((table), token.data.identificator) != NULL)
#define LOOK_UP_ID(table, identificator) stableLookup((table), (identificator))

// Vlozeni funkce/promenne do tabulky symbolu.
#define PUT_KEY(table, stItem) \
  if (((stItem) = stablePutKey((table), token.data.identificator)) == NULL) \
    return INTER_ERROR

// Vypocet realtivni adresy promenne.
#define GET_REL_ADDR(stItemPtr) (lTable->itemCounter - (stItemPtr)->number + 1)

// Generovani instrukce
#define GEN_INSTR(i,a1,a2,r) \
  if (generateInstruciton(i, a1, a2, r) != E_OK) return INTER_ERROR



// ---- Globalni promenne -----
// ============================

Ttoken token; // Aktualne zpracovavany token.
int ExprResult; // Vysledek vyrazu
pInstrList iList; // Ukazatel na seznam instrukci.
Tstable *gTable; // Ukazatel na globalni tabulku symbolu.
Tstable *lTable; // Ukazatel na lokalni tabulku symbolu.
Tvariable *literal; // Ukazatel na vylsednou hodnotu zpracovaneho vyrazu.
stableItemPtr mrList; // Pomocny seznam pro reseni vzajemne rekurze.
TstackExp se; // Vyrazovy zasobnik
bool IDfound; // Priznak, zda se ve vyrazu objevila promenna ID.
int ExprResultType; // Typ vysledku vyrazu.


// ---- Lokalni funkce ----
// ========================

////////////////////////////////////////
// -- Seznam pro vzajemnou rekurzi -- //
////////////////////////////////////////

/**
 * Inicializace seznamu.
 *
 * @param list - Seznam / prvni polozka seznamu.
 */
void mrlInit(stableItemPtr *list) {
  *list = NULL;
  return;
}

/**
 * Vlozeni prvku na prvni pozici.
 *
 * @param list - Seznam / prvni polozka seznamu.
 * @param instr - Ukazatel na instrukci I_CALL.
 * @param func - identifikator volane funkce.
 */
int mrlInsertFirst(stableItemPtr *list, iItem* instr, char* func) {
  stableItemPtr new;

  // Tvorba nove polozky
  if ((new = (stableItemPtr) malloc(sizeof (struct stableListItem))) == NULL) {
    return INTER_ERROR;
  }
  new->start = instr;
  new->key = func;
  new->next = *list;
  *list = new;

  return MRL_OK;
}

/**
 * Zruseni seznamu.
 *
 * @param list - Seznam / prvni polozka seznamu.
 */
void mrlDispose(stableItemPtr *list) {
  stableItemPtr aux;
  while (*list != NULL) {
    aux = *list;
    *list = (*list)->next;
    // Uvolneni stringu ID funkce
    free(aux->key);
    // Uvolneni prvku seznamu.
    free(aux);
  }
  return;
}

/**
 * Doplneni ukazatelu na zacatky volanych funkci do onstrukci I_CALL. Tyka
 * se funkci volanych pred jejich deklaraci.
 *
 * @param list - Seznam / prvni polozka seznamu.
 */
int MutRec(stableItemPtr listItem) {
  stableItemPtr aux;

  while (listItem != NULL) {
    // Overeni, zda byla funkce daneho identifikatoru definovana.
    if ((aux = LOOK_UP_ID(gTable, listItem->key)) == NULL) return SEM_ERROR;
    // Doplneni adresy volane funkce.
    listItem->start->tac.argument1 = aux->start;
    listItem = listItem->next;
  }

  return SEM_OK;
}

//////////////////////////
// -- Pomocne funkce -- //
//////////////////////////

/**
 * Generovani instrukci do instrukcniho seznamu.
 *
 * @param instType - typ instrukce
 * @param addr1    - adresa prvniho 1. operandu
 * @param addr2    - adresa prvniho 2. operandu
 * @param addr3    - adresa prvniho 3. operandu
 */
int generateInstruciton(operation instType, void *addr1, void *addr2, void *addr3) {
  int result;

  TAC instruction = {instType, addr1, addr2, addr3};
  result = listInsertLast(iList, instruction);

  return result;
}

/**
 *  Zpracovani argumentu prikazu read();
 */
int parseReadFormat(int *charNum) {
  int result = SEM_ERROR;

  switch (literal->type) {
      // kladne_cislo
    case NUM_TYPE:
      result = RF_CHARS;
      *charNum = (int) literal->data.num;
      break;

      // retezec
    case STR_TYPE:
      if (literal->data.str.str[0] == '*') {
        switch (literal->data.str.str[1]) {
          case 'n': result = RF_NUM;
            break;
          case 'l': result = RF_EOL;
            break;
          case 'a': result = RF_EOF;
            break;
        }
      }
      strFree(&(literal->data.str)); // Uvolneni stringu.
      break;

      // neocekavany typ  
    default: break;
  }

  // Uvolneni struktury Tvariable.
  free(literal);

  return result;
}

/**
 * Uvolneni obsahu tokenu po pripadne chybe v parseru.
 *
 */
void DisposeToken() {
  if (token.type == ID)
    free(token.data.identificator);
  else if (token.type == STRING)
    free(token.data.str_data.str);
  
  return;
}

/////////////////////////////
// -- Zpracovani vyrazu -- //
/////////////////////////////

int expression(int breakpoint) {
  // Deklaracni cast
  Ttoken slave; // Pomocny token
  int tempVarMade = 0;
  int bracketCounter = 0; // Citac zavorek
  int successElem = 0; // Pocet odpovidajicich si casti pravidel pro zjednoduseni
  bool ruleFound; // Kontroluje, zda bylo nalezeno pravidlo
  stableItemPtr tempPtr = NULL; // Pomocny ukazatel do tabulky symbolu
  TE firstOp, secondOp; // Pomocne promenne pro gen instrukci
  IDfound = false;


  // Algoritmus
  stackExpTopToStart(&se);
  // Pocitani parity zavorek u prvniho tokenu
  if (token.type == L_BRACKET) bracketCounter++;
  // if (token.type == R_BRACKET) bracketCounter--;
  if (token.type == R_BRACKET) return SYNTAX_ERROR;
  if (token.type == breakpoint) {
    return SYNTAX_ERROR;
  }
  generateInstruciton(I_CLR, NULL, NULL, NULL);
  while (!(((token.type == breakpoint)
          && stackGetLastTerminal(&se)->type == ENDING_TOKEN)))
  {
    if ((token.type < PLUS || token.type > THEN) || ( token.type >= SEMICOLON && token.type != breakpoint )) {
      return SYNTAX_ERROR;
    }
#if DEBUGEXP
    stackExpInfo(&se);
    printf("Terminal na stackExp: %s %d, Terminal na vstupu: %s %d, Akce: %s\n, ",
            token2Str(stackGetLastTerminal(&se)->type), stackGetLastTerminal(&se)->type,
            token2Str(token.type), token.type,
            condition2Str(GET_PRIORITY(stackGetLastTerminal(&se)->type,
            ((token.type == breakpoint) ? ENDING_TOKEN : token.type))));
#endif
    // Nasledujici operaci urcime z tabulky priorit
    switch (GET_PRIORITY(stackGetLastTerminal(&se)->type,
            ((token.type == breakpoint) ? ENDING_TOKEN : token.type))) {
        // Ulozime na zasobnik
      case WAIT: // Ekvivalent "<"
        if (bracketCounter >= 0) stackExpPush(&se, token);
        GET_NEXT_TOKBRACKET(&token);
        break;
        // Hledame pravidlo pro zjednoduseni
      case WORK: // Ekvivalent ">"
        ruleFound = false; // K detekci, ze jsme v aktualnim pruchodu nasli pravidlo
        for (int i = 0; i < RULE_COUNT; i++) {
          successElem = 0; // Presli jsme na dalsi pravidlo, nulujeme citac

          // Kontroluje, zda pravidlo neni prilis dlouhe
          if (se.top > ruleTable[i][0]) {
            for (int j = 1; j <= ruleTable[i][0]; j++) {
              if (se.stackExp[se.top - j].type == ruleTable[i][j]) successElem++;
              else break;
            }
            // Pravidlo odpovida posloupnosti na zasobniku vyrazu
            if (successElem == ruleTable[i][0]) {
              ruleFound = true; // Pravidlo bylo v aktualnim pruchodu nalezeno

              switch (i) {
                case 0: // ID
                  // Hledame nazev promenne v lokalni tabulce symbolu
                  if ((tempPtr = LOOK_UP_ID(lTable, se.stackExp[se.top - 1].data.identificator)) == NULL) {
                    // Uvolneni retezce (ID) z aktualniho tokenu.
                    free(se.stackExp[se.top - 1].data.identificator);
                    se.stackExp[se.top - 1].data.identificator = NULL;
		    return SEM_ERROR;
                  }
                  // Uvolneni retezce (ID) z aktualniho tokenu.
                  free(se.stackExp[se.top - 1].data.identificator);
                  se.stackExp[se.top - 1].data.identificator = NULL;

                  slave.data.Edata.datatypeE = ID;
                  IDfound = true;
                  break;
                  /////////////////////////////////////////////////////////////
                case 1: // STRING

                  if ((literal = (Tvariable *) malloc(sizeof (Tvariable))) == NULL) {
                    return INTER_ERROR;
                  }
                  literal->type = STR_TYPE;
                  literal->data.str = stackGetLastTerminal(&se)->data.str_data;
                  generateInstruciton(I_PUSHL, (void *) literal, NULL, NULL);
                  slave.data.Edata.datatypeE = STRING;
                  break;
                  /////////////////////////////////////////////////////////////
                case 2: // NUMBER
                  if ((literal = (Tvariable *) malloc(sizeof (Tvariable))) == NULL) {
                    return INTER_ERROR;
                  }
                  literal->type = NUM_TYPE;
                  literal->data.num = stackGetLastTerminal(&se)->data.value;
                  generateInstruciton(I_PUSHL, (void *) literal, NULL, NULL);
                  slave.data.Edata.datatypeE = NUMBER;
                  break;
                  /////////////////////////////////////////////////////////////
                case 3: // FALSE
                  if ((literal = (Tvariable *) malloc(sizeof (Tvariable))) == NULL) {
                    return INTER_ERROR;
                  }
                  literal->type = BOOL_TYPE;
                  literal->data.boolean = false;
                  generateInstruciton(I_PUSHL, (void *) literal, NULL, NULL);
                  slave.data.Edata.datatypeE = FALSE;
                  break;
                  /////////////////////////////////////////////////////////////
                case 4: // TRUE
                  if ((literal = (Tvariable *) malloc(sizeof (Tvariable))) == NULL) {
                    return INTER_ERROR;
                  }
                  literal->type = BOOL_TYPE;
                  literal->data.boolean = true;
                  generateInstruciton(I_PUSHL, (void *) literal, NULL, NULL);
                  slave.data.Edata.datatypeE = TRUE;
                  break;
                  /////////////////////////////////////////////////////////////
                case 5: // NIL
                  if ((literal = (Tvariable *) malloc(sizeof (Tvariable))) == NULL) {
                    return INTER_ERROR;
                  }
                  literal->type = NIL_TYPE;
                  generateInstruciton(I_PUSHL, (void *) literal, NULL, NULL);
                  slave.data.Edata.datatypeE = NIL;
                  break;
                  /////////////////////////////////////////////////////////////
                case 6: // PLUS
                case 7: // MINUS
                case 8: // MULTIPLY
                case 9: // DIVIDE
                case 10: // POWER
                case 19: // ROZSIRENI: MODULO
                  firstOp = se.stackExp[se.top - 3].data.Edata;
                  secondOp = se.stackExp[se.top - 1].data.Edata;
                  // Slaba typova kontrola
                  if (!((firstOp.datatypeE == ID || firstOp.datatypeE == NUMBER) &&
                          (secondOp.datatypeE == ID || secondOp.datatypeE == NUMBER))) {
                    return SEM_ERROR;
                  }
                  slave.data.Edata.datatypeE = NUMBER;
                  switch (i) {
                    case 6:
                      generateInstruciton(I_ADD, (void *) firstOp.positionToAct, (void *) secondOp.positionToAct, (void *) tempVarMade);
                      break;
                    case 7:
                      generateInstruciton(I_SUB, (void *) firstOp.positionToAct, (void *) secondOp.positionToAct, (void *) tempVarMade);
                      break;
                    case 8:
                      generateInstruciton(I_MUL, (void *) firstOp.positionToAct, (void *) secondOp.positionToAct, (void *) tempVarMade);
                      break;
                    case 9:
                      generateInstruciton(I_DIV, (void *) firstOp.positionToAct, (void *) secondOp.positionToAct, (void *) tempVarMade);
                      break;
                    case 10:
                      generateInstruciton(I_POW, (void *) firstOp.positionToAct, (void *) secondOp.positionToAct, (void *) tempVarMade);
                      break;
                    case 19:
                      generateInstruciton(I_MOD, (void *) firstOp.positionToAct, (void *) secondOp.positionToAct, (void *) tempVarMade);
                      break;
                  }
                  break;
                  /////////////////////////////////////////////////////////////
                case 11: // CONCATENATION
                  firstOp = se.stackExp[se.top - 3].data.Edata;
                  secondOp = se.stackExp[se.top - 1].data.Edata;
                  // Slaba typova kontrola
                  if (!((firstOp.datatypeE == ID || firstOp.datatypeE == STRING) &&
                          (secondOp.datatypeE == ID || secondOp.datatypeE == STRING))) {
                    return SEM_ERROR;
                  }
                  slave.data.Edata.datatypeE = STRING;
                  generateInstruciton(I_CON, (void *) firstOp.positionToAct, (void *) secondOp.positionToAct, (void *) tempVarMade);
                  break;
                  /////////////////////////////////////////////////////////////
                case 12: // EQUAL
                case 13: // NOT_EQUAL
                  firstOp = se.stackExp[se.top - 3].data.Edata;
                  secondOp = se.stackExp[se.top - 1].data.Edata;
                  // Slaba typova kontrola
                  if (!((firstOp.datatypeE == ID || firstOp.datatypeE == STRING || firstOp.datatypeE == NUMBER || firstOp.datatypeE == FALSE || firstOp.datatypeE == TRUE || firstOp.datatypeE == NIL) &&
                          (secondOp.datatypeE == ID || secondOp.datatypeE == STRING || secondOp.datatypeE == NUMBER || secondOp.datatypeE == FALSE || secondOp.datatypeE == TRUE || secondOp.datatypeE == NIL))) {
                    return SEM_ERROR;
                  }
                  slave.data.Edata.datatypeE = TRUE;
                  // Silna typova kontrola
                  switch (firstOp.datatypeE) {
                    case ID:
                      break;
                    case STRING:
                      switch (secondOp.datatypeE) {
                        case ID:
                        case STRING:
                          break;
                        default:
                          return SEM_ERROR;
                      }
                      break;
                    case NUMBER:
                      switch (secondOp.datatypeE) {
                        case ID:
                        case NUMBER:
                          break;
                        default:
                          return SEM_ERROR;
                      }
                      break;
                    case FALSE:
                    case TRUE:
                      switch (secondOp.datatypeE) {
                        case ID:
                        case FALSE:
                        case TRUE:
                          break;
                        default:
                          return SEM_ERROR;
                      }
                      break;
                    case NIL:
                      switch (secondOp.datatypeE) {
                        case ID:
                        case NIL:
                          break;
                        default:
                          return SEM_ERROR;
                      }
                      break;
                  }
                  switch (i) {
                    case 12:
                      generateInstruciton(I_EQU, (void *) firstOp.positionToAct, (void *) secondOp.positionToAct, (void *) tempVarMade);
                      break;
                    case 13:
                      generateInstruciton(I_NEQ, (void *) firstOp.positionToAct, (void *) secondOp.positionToAct, (void *) tempVarMade);
                      break;
                  }
                  break;
                  /////////////////////////////////////////////////////////////
                case 14: // LESS
                case 15: // LESS_EQUAL
                case 16: // MORE
                case 17: // MORE_EQUAL
                  firstOp = se.stackExp[se.top - 3].data.Edata;
                  secondOp = se.stackExp[se.top - 1].data.Edata;
                  if (!((firstOp.datatypeE == ID || firstOp.datatypeE == STRING || firstOp.datatypeE == NUMBER) &&
                          (secondOp.datatypeE == ID || secondOp.datatypeE == STRING || secondOp.datatypeE == NUMBER))) {
                    return SEM_ERROR;
                  }
                  slave.data.Edata.datatypeE = TRUE;

                  switch (firstOp.datatypeE) {
                    case ID:
                      break;
                    case STRING:
                      switch (secondOp.datatypeE) {
                        case ID:
                        case STRING:
                          break;
                        default:
                          return SEM_ERROR;
                      }
                      break;
                    case NUMBER:
                      switch (secondOp.datatypeE) {
                        case ID:
                        case NUMBER:
                          break;
                        default:
                          return SEM_ERROR;
                      }
                      break;
                  }
                  switch (i) {
                    case 14: // LESS
                      generateInstruciton(I_LES, (void *) firstOp.positionToAct, (void *) secondOp.positionToAct, (void *) tempVarMade);
                      break;
                    case 15: // LESS_EQUAL
                      generateInstruciton(I_LSE, (void *) firstOp.positionToAct, (void *) secondOp.positionToAct, (void *) tempVarMade);
                      break;
                    case 16: // MORE
                      generateInstruciton(I_GRT, (void *) firstOp.positionToAct, (void *) secondOp.positionToAct, (void *) tempVarMade);
                      break;
                    case 17: // MORE_EQUAL
                      generateInstruciton(I_GRE, (void *) firstOp.positionToAct, (void *) secondOp.positionToAct, (void *) tempVarMade);
                      break;
                  }
                  break;
                  /////////////////////////////////////////////////////////////
                case 18: // (E) -> E
                  slave = se.stackExp[se.top - 2];
                  tempVarMade++;
                  break;
                  /////////////////////////////////////////////////////////////
                case 20: // #LENGTH
                  firstOp = se.stackExp[se.top - 1].data.Edata;
                  // Slaba typova kontrola
                  if (!(firstOp.datatypeE == ID || firstOp.datatypeE == STRING)) {
                    return SEM_ERROR;
                  }
                  slave.data.Edata.datatypeE = NUMBER;
                  generateInstruciton(I_LEN, (void *) firstOp.positionToAct, NULL, (void *) tempVarMade);
                  break;
                  /////////////////////////////////////////////////////////////
              }

              // Inicializace pomocneho tokenu
              slave.type = E;
              if (i == 0) slave.data.Edata.positionToAct = GET_REL_ADDR(tempPtr);
              else slave.data.Edata.positionToAct = tempVarMade--;
              // Vlozime na zasobnik vyrazu pomocny token
              stackExpMultiplePop(&se, ruleTable[i][0]);
              stackExpPush(&se, slave);
            }
            if (ruleFound) break;
          }
        }
        if (!ruleFound) {
          return SYNTAX_ERROR;
        }
        break;
        // Semanticka chyba
      case FSEM:
        if (stackGetLastTerminal(&se)->type == ENDING_TOKEN && bracketCounter < 0) {
#if DEBUGEXP
          stackExpInfo(&se);
#endif          
          ExprResult = stackExpTop(&se).data.Edata.positionToAct;
          ExprResultType = stackExpTop(&se).data.Edata.datatypeE;
          return EXPR_OK;
        }
        return SEM_ERROR;
        break;
        // Syntakticka chyba
      case FSYN:
        if (stackGetLastTerminal(&se)->type == ENDING_TOKEN && bracketCounter < 0) {
#if DEBUGEXP
          stackExpInfo(&se);
#endif          
          ExprResult = stackExpTop(&se).data.Edata.positionToAct;
          ExprResultType = stackExpTop(&se).data.Edata.datatypeE;
          return EXPR_OK;
        }
        return SYNTAX_ERROR;
        break;
      default:
        break;
    }
  }
#if DEBUGEXP
  printf("  KONEC  !!!!\n");
  stackExpInfo(&se);
  printf("VRACENA HODNOTA: %d\n\n\n", stackExpTop(&se).data.Edata.positionToAct);
#endif

  ExprResult = stackExpTop(&se).data.Edata.positionToAct; // 
  ExprResultType = stackExpTop(&se).data.Edata.datatypeE;
  return EXPR_OK;
}

/////////////////////////////
// -- Rekurzivni sestup -- //
/////////////////////////////

/**
 *  Funkce zpracujici neterminal <RestOfFuncs>.
 */
int RestOfFuncs() {
  // Pravidlo <RestOfFuncs> -> ID ( <formParams> ) <body> function <restOfFuncs>

  int result;

  /* 'ID' */
  if (token.type != ID) return SYNTAX_ERROR;

  // ID je "main".
  if (strcmp(token.data.identificator, "main") == 0) return SYNTAX_OK;

  // Kontrola zda uz funkce daneho jmena nebyla definovana.
  if (LOOK_UP(gTable)) return SEM_ERROR;
  // Vlozeni funkce do glob. TS.
  stableItemPtr stItem = NULL;
  PUT_KEY(gTable, stItem);
  GET_NEXT_TOKEN(&token);

  /* '(' */
  if (token.type != L_BRACKET) return SYNTAX_ERROR;
  GET_NEXT_TOKEN(&token);

  /* <formParams> */
  if ((result = FormParams()) != SYNTAX_OK) return result;

  /* ')' */
  if (token.type != R_BRACKET) return SYNTAX_ERROR;
  GET_NEXT_TOKEN(&token);

  // Generovani instrukce I_LBL navesti zacatku tela funkce. V addr1 pocet parametru fce.
  GEN_INSTR(I_LBL, (void *) lTable->itemCounter, NULL, NULL);
  // Ulozeni ukazatele na prvni instrukci funkce.
  stItem->start = (iItem*) listGetPointerLast(iList);
  // Ulozeni poctu parametru zpracovavane funkce do glob. tabulky symoblu.
  stItem->number = lTable->itemCounter;

  /* <body> */
  if ((result = Body()) != SYNTAX_OK) return result;

  // Vycisteni lokalni tabulky symbolu.
  stableClear(lTable);

  /* 'function' */
  if (token.type != FUNCTION) return SYNTAX_ERROR;
  GET_NEXT_TOKEN(&token);

  /* <restOfFuncs> */
  return RestOfFuncs();
}

/**
 *  Funkce zpracujici neterminal <formParams>.
 *  Zpracuje prvni formalni parametr funkce.
 */
int FormParams() {
  int result;

  switch (token.type) {
      // Pravidlo <formParams> -> *epsilon*
    case R_BRACKET: return SYNTAX_OK;
      break;

      // Pravidlo <formParams> -> ID <formParamsNext>
    case ID:
      // Kontrola zda uz promenna daneho jmena nebyla deklarovana.
      if (LOOK_UP(lTable)) return SEM_ERROR;
      if (LOOK_UP(gTable)) return SEM_ERROR;
      // Vlozeni promenne do lokalni TS.
      stableItemPtr stItem;
      PUT_KEY(lTable, stItem);
      // Nastaveni poradoveho cisla promenne - v tomto pripade je PRVNI.
      assert(lTable->itemCounter == 1);
      stItem->number = lTable->itemCounter;
#if DEBUG
      printf("%s -> %d\n", stItem->key, stItem->number);
#endif
      GET_NEXT_TOKEN(&token);

      /* '<formParamsNext>' */
      if ((result = FormParamsNext()) != SYNTAX_OK) return result;

      return SYNTAX_OK;
      break;
  }
  return SYNTAX_ERROR;
}

/**
 *  Funkce zpracujici neterminal <formParamsNext>.
 *  Zpracuje ostatni formalni parametry funkce.
 */
int FormParamsNext() {
  switch (token.type) {
      // Pravidlo formParamsNext -> *epsilon*
    case R_BRACKET: return SYNTAX_OK;
      break;

      // Pravidlo formParamsNext -> , ID <fomrParamsNext>
    case COMMA: /* ',' */
      GET_NEXT_TOKEN(&token);

      /* 'ID' */
      if (token.type != ID) return SYNTAX_ERROR;
      // Kontrola zda uz promenna daneho jmena nebyla deklarovana.
      if (LOOK_UP(lTable)) return SEM_ERROR;
      if (LOOK_UP(gTable)) return SEM_ERROR;
      // Vlozeni promenne do lokalni TS.
      stableItemPtr stItem;
      PUT_KEY(lTable, stItem);
      // Nastaveni poradoveho cisla promenne.
      stItem->number = lTable->itemCounter;
#if DEBUG
      printf("%s -> %d\n", stItem->key, stItem->number);
#endif
      GET_NEXT_TOKEN(&token);

      /* '<formParamsNext>' */
      return FormParamsNext();
      break;
  }

  return SYNTAX_ERROR;
}

/**
 *  Funkce zpracujici neterminal <Body>.
 *  Zpracuje telo definice funkce.
 */
int Body() {
  int result;

  switch (token.type) {
      // Pravidlo <body> -> <varDecl> <stats> end
    case LOCAL:
    case WRITE:
    case IF:
    case WHILE:
    case REPEAT:
    case RETURN:
    case ID:
    case END:
      /* '<varDecl>' */
      if ((result = VarDecl()) != SYNTAX_OK) return result;
      // Generovani instrukce I_ACTTOP pro srovnani ukazatelu na vrchol zasobniku.
      GEN_INSTR(I_ACTTOP, NULL, NULL, NULL);

      /* '<stats>' */
      if ((result = Stats()) != SYNTAX_OK) return result;

      /* 'end' */
      if (token.type != END) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);
      // Generovani instrukce I_RET pro pripad, ze uzivatel v tele funkce nepouzil return <exp> ;
      GEN_INSTR(I_RET, (void *) (lTable->itemCounter + 1), NULL, (void *) IMPL_RET);

      return SYNTAX_OK;
      break;
  }

  return SYNTAX_ERROR;
}

/**
 * Funkce zpracuje deklaracni cast tela funkce.
 */
int VarDecl() {
  int result;
  char *identificator; // Pomocna promenna uchvavajici nazev lokalni promenne.

  switch (token.type) {
      // Pravidlo <varDecl> -> *epsilon*
    case WRITE:
    case IF:
    case WHILE:
    case REPEAT:
    case RETURN:
    case ID:
    case END:
      return SYNTAX_OK;
      break;

      // Pravidlo <varDecl> -> local ID <assign> ; <varDecl>
    case LOCAL: /* 'local' */
      GET_NEXT_TOKEN(&token);

      // Generovani instrukce I_ACTTOP pro srovnani ukazatelu na vrchol zasobniku.
      GEN_INSTR(I_ACTTOP, NULL, NULL, NULL);

      /* 'ID' */
      if (token.type != ID) return SYNTAX_ERROR;
      // Kontrola zda uz promenna daneho jmena nebyla deklarovana.
      if (LOOK_UP(lTable)) return SEM_ERROR;
      if (LOOK_UP(gTable)) return SEM_ERROR;
      // Lok.prom. se do lTable ulozi az po prip. zprac. inic., zatim si ulozime jeji ID.
      identificator = token.data.identificator;
      GET_NEXT_TOKEN(&token);

      // Vynul adresy vysledku vyrazu pro pripad, ze promenna neni explicitne inic.
      ExprResult = 0;

      /* '<assign>' */
      if ((result = Assign(identificator)) != SYNTAX_OK) return result;

      // Vlozeni promenne do lokalni TS.
      stableItemPtr stItem;
      if ((stItem = stablePutKey(lTable, identificator)) == NULL) return INTER_ERROR;
      // Nastaveni poradoveho cisla promenne.
      stItem->number = lTable->itemCounter;
      // Gen. instr. I_PUSHV - ulozeni promenne na zasobnik.
      GEN_INSTR(I_PUSHV, (void *) ExprResult, NULL, NULL);
#if DEBUG
      printf("%s -> %d\n", stItem->key, stItem->number);
#endif

      /* ';' */
      if (token.type != SEMICOLON) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);

      /* '<varDecl>' */
      return VarDecl();
      break;
  }

  return SYNTAX_ERROR;
}

/**
 *  Zpracovani inicializace promenne pri jeji deklaraci. 
 */
int Assign(char *identificator) {
  int result;

  switch (token.type) {
      // Pravidlo <assign> -> *epsilon*
    case SEMICOLON:
      return SYNTAX_OK;
      break;

      // Pravidlo <assign> -> = <expr>
    case ASSIGN: /* '=' */
      GET_NEXT_TOKEN(&token);

      /* '<expr>' */
      if ((result = expression(SEMICOLON)) != EXPR_OK) return result;

      return SYNTAX_OK;
      break;
    default:
      // Uvolneni ID lokalni promenne.
      free(identificator);
      identificator = NULL;
  }

  return SYNTAX_ERROR;
}

/**
 *  Zpracovani literalu.
 */
int Liter() {
  // Tvorba nove prime hodnoty (literalu) Tvariable.
  if ((literal = (Tvariable *) malloc(sizeof (Tvariable))) == NULL) return INTER_ERROR;

  switch (token.type) {

    case NUMBER:
      // Pravidlo <liter> -> NUMBER  
      literal->type = NUM_TYPE;
      literal->data.num = token.data.value;
      break;

    case STRING:
      // Pravidlo <liter> -> STRING
      literal->type = STR_TYPE;
      literal->data.str = token.data.str_data;
      break;

    case TRUE:
    case FALSE:
      // Pravidlo <liter> -> true  
      // Pravidlo <liter> -> false
      literal->type = BOOL_TYPE;
      literal->data.boolean = (token.type == TRUE) ? true : false;
      break;

    case NIL:
      // Pravidlo <liter> -> nil
      literal->type = NIL_TYPE;
      break;

      // Neocekavany token.
    default:
      // Kontrola, zda neprisel token ID. Tehdy potreba uvolnit pamet alokovanou pro key.
      if (token.type == ID) free(token.data.identificator);
      // Uvolneni Tvariable.
      free(literal);

      return SYNTAX_ERROR;
      break;
  }
  GET_NEXT_TOKEN(&token);

  return SYNTAX_OK;
}

/**
 *  Zpracovani sekvence prikazu v tele funkce.
 */
int Stats() {
  int result;
  iItem *auxInstPtr1 = NULL;
  iItem *auxInstPtr2 = NULL;
  stableItemPtr stItem = NULL;

  switch (token.type) {
      // Pravidlo <stats> -> *epsilon*
    case END:
    case ELSE:
    case UNTIL:
      return SYNTAX_OK;
      break;

      // Pravidlo <stats> -> write ( <paramsWrite> ) ; <stats>
    case WRITE: /* WRITE */
      GET_NEXT_TOKEN(&token);

      /* '(' */
      if (token.type != L_BRACKET) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);

      /* '<paramsWrite>' */
      if ((result = ParamsWrite()) != SYNTAX_OK) return result;

      /* ')' */
      if (token.type != R_BRACKET) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);

      /* ';' */
      if (token.type != SEMICOLON) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);

      /* '<stats>' */
      return Stats();
      break;

      // Pravidlo <stats> -> if <expr> then <stats> else <stats> end ; <stats>
    case IF: /* 'IF' */
      GET_NEXT_TOKEN(&token);

      /* '<expr>' */
      if ((result = expression(THEN)) != EXPR_OK) return result;
      // Generovani instrukce CJMP podmineneho skoku na zaklade vysledku vyrazu.
      GEN_INSTR(I_CJMP, NULL, NULL, (void *) ExprResult);
      // Ulozeni adresy instrukce skoku pro pozdejsi doplneni adresy navesti.
      auxInstPtr1 = (iItem *) listGetPointerLast(iList);

      /* 'THEN' */
      if (token.type != THEN) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);
      // Generovani instrukce navesti I_LBL pro kladne vyhodnoceni vyrazu.
      GEN_INSTR(I_LBL, NULL, NULL, NULL);
      // Ulozeni adresy navesti pro kladne vyhodnoceni vyrazu do instrukce CJMP.
      auxInstPtr1->tac.argument1 = listGetPointerLast(iList);

      /* '<stats>' */
      if ((result = Stats()) != SYNTAX_OK) return result;
      // Generovani instrukce JMP za konec vetve zaporneho vyhodnoceni vyrazu.
      GEN_INSTR(I_JMP, NULL, NULL, NULL);
      // Ulozeni ukazatele na instrukci JMP pro pozdejsi ulozeni adresy ciloveho navesti.
      auxInstPtr2 = (iItem *) listGetPointerLast(iList);

      /* 'ELSE' */
      if (token.type != ELSE) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);
      // Generovani instrukce navesti I_LBL pro zaporne vyhodnoceni vyrazu.
      GEN_INSTR(I_LBL, NULL, NULL, NULL);
      // Ulozeni adresy navesti pro zaporne vyhodnoceni vyrazu do instrukce CJMP.
      auxInstPtr1->tac.argument2 = listGetPointerLast(iList);

      /* '<stats>' */
      if ((result = Stats()) != SYNTAX_OK) return result;

      /* 'END' */
      if (token.type != END) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);

      /* ';' */
      if (token.type != SEMICOLON) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);
      // Generovani instr. navesti I_LBL pro pokracovani vetve kladeho vyhodnoceni vyrazu.
      GEN_INSTR(I_LBL, NULL, NULL, NULL);
      // Ulozeni adresy navesti pro pokracovani kladne vyhodnocene vetve.
      auxInstPtr2->tac.argument1 = listGetPointerLast(iList);

      /* '<stats>' */
      return Stats();
      break;

      // Pravidlo <stats> -> while <expr> do <stats> end ; <stats>
    case WHILE: /* 'WHILE' */
      GET_NEXT_TOKEN(&token);

      // Generovani instr. navesti I_LBL pro novou iteraci cyklu.
      GEN_INSTR(I_LBL, NULL, NULL, NULL);
      // Ulozeni adresy navesti I_LBL pro pozdejsi doplneni do instrukce I_JMP.
      auxInstPtr1 = (iItem *) listGetPointerLast(iList);

      /* '<expr>' */
      if ((result = expression(DO)) != EXPR_OK) return result;
      // Generovani instrukce I_CJMP podmineneho skoku na zaklade vysledku vyrazu.
      GEN_INSTR(I_CJMP, NULL, NULL, (void *) ExprResult);
      // Ulozeni adresy instrukce I_CJMP pro pozdejsi doplneni adres navesti.
      auxInstPtr2 = (iItem *) listGetPointerLast(iList);

      /* 'DO' */
      if (token.type != DO) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);
      // Generovani instr. navesti I_LBL pro vetev provedeni cyklu.
      GEN_INSTR(I_LBL, NULL, NULL, NULL);
      // Doplneni adresy navesti provedeni tela cyklu do instrukce I_CJMP.
      auxInstPtr2->tac.argument1 = listGetPointerLast(iList);

      /* '<stats>' */
      if ((result = Stats()) != SYNTAX_OK) return result;
      // Generovani instrukce I_JMP na zacatek cyklu.
      GEN_INSTR(I_JMP, (void *) auxInstPtr1, NULL, NULL);

      /* 'END' */
      if (token.type != END) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);
      // Generovani instr. navesti I_LBL pro preskoceni cyklu.
      GEN_INSTR(I_LBL, NULL, NULL, NULL);
      // Doplneni adresy navesti preskoceni cyklu do instrukce I_CJMP.
      auxInstPtr2->tac.argument2 = listGetPointerLast(iList);

      /* ';' */
      if (token.type != SEMICOLON) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);

      /* '<stats>' */
      return Stats();
      break;

      // Pravidlo <stats> -> repeat <stats> until <expr> ; <stats>
    case REPEAT: /* 'REPEAT' */
      GET_NEXT_TOKEN(&token);

      // Generovani instr. navesti I_LBL pro novou iteraci cyklu.
      GEN_INSTR(I_LBL, NULL, NULL, NULL);
      // Ulozeni adresy navesti I_LBL pro pozdejsi doplneni do instrukce I_JMP.
      auxInstPtr1 = (iItem *) listGetPointerLast(iList);

      /* '<stats>' */
      if ((result = Stats()) != SYNTAX_OK) return result;

      /* 'UNTIL' */
      if (token.type != UNTIL) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);

      /* '<expr>' */
      if ((result = expression(SEMICOLON)) != EXPR_OK) return result;
      // Generovani instrukce I_CJMP podmineneho skoku na zaklade vysledku vyrazu.
      GEN_INSTR(I_CJMP, NULL, (void *) auxInstPtr1, (void *) ExprResult);
      // Ulozeni adresy instrukce I_CJMP pro pozdejsi doplneni adresy pro neopakocani cyklu.
      auxInstPtr2 = (iItem *) listGetPointerLast(iList);
      // Generovani instr. navesti I_LBL pro neopakovani cyklu.
      GEN_INSTR(I_LBL, NULL, NULL, NULL);
      // Doplneni adresy navesti pro neopakovani cyklu do I_CJMP
      auxInstPtr2->tac.argument1 = listGetPointerLast(iList);

      /* ';' */
      if (token.type != SEMICOLON) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);

      /* '<stats>' */
      return Stats();
      break;

      // Pravidlo <stats> -> return <expr> ; <stats>
    case RETURN: /* 'RETURN' */
      GET_NEXT_TOKEN(&token);
      /* '<expr>' */
      if ((result = expression(SEMICOLON)) != EXPR_OK) return result;
      // Generovani instrukce I_RET pro ulozeni navratove hodnoty a navrat z funkce.
      GEN_INSTR(I_RET, (void *) (lTable->itemCounter + 1), (void *) ExprResult, (void *) EXPL_RET);

      /* ';' */
      if (token.type != SEMICOLON) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);

      /* '<stats>' */
      return Stats();
      break;

      // Pravidlo <stats> -> ID = <funcOrExpr> ; <stats>
    case ID:
      // Kontrola, zda uz byla promena deklarovana.
      if ((stItem = stableLookup(lTable, token.data.identificator)) == NULL) return SEM_ERROR;
      // Uvolneni retezce (ID) z aktualniho tokenu.
      free(token.data.identificator);
      token.data.identificator = NULL;
      GET_NEXT_TOKEN(&token);

      /* '=' */
      if (token.type != ASSIGN) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);

      /* '<funcOrExpr>' */
      if ((result = FuncOrExpr(stItem)) != SYNTAX_OK) return result;

      /* ';' */
      if (token.type != SEMICOLON) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);

      /* '<stats>' */
      return Stats();
      break;
  }

  return SYNTAX_ERROR;
}

/**
 * Zpracovani volani funkce nebo prirazeni vysledku vyrazu.
 * @param stItem - ukazatel na polozku tabulky symbolu - promennou, do niz se bude
 *                 prirazovat navratova bodnota volane funkce.
 */
int FuncOrExpr(stableItemPtr lSide) {
  int result;
  int charNum = 0; // Pocet nacitanych znaku prikazem read();
  stableItemPtr rSide = NULL; // Pomocny ukazatel na prvek lTable pro funkci type().
  iItem *auxInstPtr1;
  int firstTermInRule;
  char *fID;

  // Cisteni pripadnych mezivysledku zpracovani vyrazu na zaporne casti zasobniku pred
  // volanim funkce.
  GEN_INSTR(I_CLR, NULL, NULL, NULL);

  switch (token.type) {
      // Pravidlo <funcOrExpr> -> read ( <liter> )
    case READ: /* 'READ' */
      GET_NEXT_TOKEN(&token);

      /* '(' */
      if (token.type != L_BRACKET) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);

      /* '<liter>' */
      if ((result = Liter()) != SYNTAX_OK) return result;
      // Parsovani formatu
      if ((result = parseReadFormat(&charNum)) == SEM_ERROR) return SEM_ERROR;
      // Generovani instrukce I_READ.
      GEN_INSTR(I_READ, (void *) GET_REL_ADDR(lSide), (void *) result, (void *) charNum);

      /* ')' */
      if (token.type != R_BRACKET) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);

      break;

      // Pravidlo <funcOrExpr> -> type ( <actParams> )
      // Pravidlo <funcOrExpr> -> sort ( <actParams> )
      // Pravidlo <funcOrExpr> -> find ( <actParams> )
      // Pravidlo <funcOrExpr> -> substr ( <actParams> )
    case TYPE:
    case SORT:
    case FIND:
    case SUBSTR:
      firstTermInRule = token.type;
      GET_NEXT_TOKEN(&token);

      /* '(' */
      if (token.type != L_BRACKET) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);

      // Vlozeni predavanych parametru na zasobnik.
      if ((result = ActParams()) != SYNTAX_OK) return result;

      // Generovani patricne instrukce.
      switch (firstTermInRule) {
        case TYPE: GEN_INSTR(I_TYPE, (void *) GET_REL_ADDR(lSide), NULL, NULL);
          break;
        case SORT: GEN_INSTR(I_SORT, (void *) GET_REL_ADDR(lSide), NULL, NULL);
          break;
        case FIND: GEN_INSTR(I_FIND, (void *) GET_REL_ADDR(lSide), NULL, NULL);
          break;
        case SUBSTR: GEN_INSTR(I_SUBST, (void *) GET_REL_ADDR(lSide), NULL, NULL);
          break;
      }

      /* ')' */
      if (token.type != R_BRACKET) return SYNTAX_ERROR;
      GET_NEXT_TOKEN(&token);
      break;

      // Pravidlo <funcOrExpr> -> ID ( <actParams> ), nebo 
      //          <funcOrExpr> -> <expr>
    case ID:
      /* '<expr>' */
      // Zjisteni, zda je nactene ID deklarovana promenna.
      if (LOOK_UP(lTable)) {
        // Volani zpracovani vyrazu.
        if ((result = expression(SEMICOLON)) != EXPR_OK) return result;
        // Generovani instrukce I_ASGNV prirazeni vyrazu do promenne.
        GEN_INSTR(I_ASGNV, (void *) ExprResult, (void *) (GET_REL_ADDR(lSide)), NULL);
      }
        /* volani_funkce */
      else {
        // Pokus o nalezeni funkce v glob. tabluce symoblu.
        rSide = stableLookup(gTable, token.data.identificator);
        fID = token.data.identificator;
        GET_NEXT_TOKEN(&token);

        /* '(' */
        if (token.type != L_BRACKET) {
	  // Uvolneni stringu pro ID.
	  free(fID);
	  fID = NULL;  
	  return SEM_ERROR;
	}
        GET_NEXT_TOKEN(&token);

        // Generovani instrukce I_PUSHA pro ulozeni navratove adresy.
        GEN_INSTR(I_PUSHA, NULL, NULL, NULL);
        // Uloz. adresy instr. I_PUSHA pro pozd. doplneni adresy navesti navratu z funkce.
        auxInstPtr1 = (iItem *) listGetPointerLast(iList);

        // Naskladani predavanych parametru na zasobnik.
        /* '<actParams>' */
        if ((result = ActParams()) != SYNTAX_OK) {
	  free(fID);
	  fID = NULL;
	  return result;
	}

        if (rSide != NULL) {
          // Generovani instrukce I_CALL pro volani jiz definovane funkce.
          GEN_INSTR(I_CALL, (void *) rSide->start, NULL, NULL);
          // Uvolneni retezce (ID) z aktualniho tokenu.
          free(fID);
          fID = NULL;
        }
        else {
          // Generovani instrukce I_CALL pro volani zatim nedefinovane funkce.
          GEN_INSTR(I_CALL, NULL, NULL, NULL);
          if (mrlInsertFirst(&mrList, (iItem *) listGetPointerLast(iList), fID) != MRL_OK)
            return INTER_ERROR;
        }

        // Generovani navesti I_LBL pro navrat z funkce.
        GEN_INSTR(I_LBL, NULL, NULL, NULL);
        // Doplneni adresy navesti funkce do instrukce I_PUSHA.
        auxInstPtr1->tac.argument1 = listGetPointerLast(iList);

        // Gen. instr. I_ASGNV pro ulozeni navrat. hodnoty -> na vrcholu zasobniku. 
        GEN_INSTR(I_ASGNV, (void *) 1, (void *) ((GET_REL_ADDR(lSide)) + 1), NULL);

        // Generovani instrukce I_POP pro odstraneni navrat. hodnoty z vrcholu zasobniku.
        GEN_INSTR(I_POP, NULL, NULL, NULL);

        /* ')' */
        if (token.type != R_BRACKET) return SYNTAX_ERROR;
        GET_NEXT_TOKEN(&token);

      }
      break;

      // Pravidlo <funcOrExpr> -> <expr> (vsechny tokeny, kterymy muze zacinat vyraz)
      // Pripadny neocekavany token si odchyti precedencni SA.
    default:
      // Volani zpracovani vyrazu.
      if ((result = expression(SEMICOLON)) != EXPR_OK) return result;
      // Generovani instrukce I_ASGNV prirazeni vyrazu do promenne.
      GEN_INSTR(I_ASGNV, (void *) ExprResult, (void *) (GET_REL_ADDR(lSide)), NULL);
      break;
  }

  return SYNTAX_OK;
}

/**
 *  Zpracovani prvniho parametru uzivatelem definovane funkce.
 */
int ActParams() {
  int result;

  // Pravidlo <actParamsUsr> -> *epsilon*
  /* ')' */
  if (token.type == R_BRACKET) return SYNTAX_OK;

  // Pravidlo <actParamsUsr> -> <IDorLiter> <actParamsNextUsr>
  /* '<IDorLiter>' */
  if ((result = IDorLiter()) != SYNTAX_OK) return result;

  /* '<actParamsNextUsr>' */
  if ((result = ActParamsNext()) != SYNTAX_OK) return result;

  return SYNTAX_OK;
}

/**
 *  Zpracovani ostatnich parametru uzivatelem definovane funkce.
 */
int ActParamsNext() {
  int result;

  // Pravidlo <actParamsNextUsr> -> *epsilon*
  /* ')' */
  if (token.type == R_BRACKET) return SYNTAX_OK;

  // Pravidlo <actParamsNextUsr> -> , <IDorLiter> <actParamsNext>
  /* ',' */
  if (token.type != COMMA) return SYNTAX_ERROR;
  GET_NEXT_TOKEN(&token);

  /* '<IDorLiter>' */
  if ((result = IDorLiter()) != SYNTAX_OK) return result;

  /* '<actParamsNext>' */
  return ActParamsNext();
}

/**
 *  Zpracovani ID nebo literalu
 */
int IDorLiter() {
  stableItemPtr stItem;
  int result;

  switch (token.type) {
      // Pravidlo <IDorLiter> -> ID
    case ID:
      // Kontrola zda jiz byla promenna deklarovana
      if ((stItem = stableLookup(lTable, token.data.identificator)) == NULL)
        return SEM_ERROR;
      // Uvolneni stringu pro ID promenne.
      free(token.data.identificator);
      token.data.identificator = NULL;
      // Vlozeni promenne na zasobnik;
      GEN_INSTR(I_PUSHV, (void *) GET_REL_ADDR(stItem), NULL, NULL);
      GET_NEXT_TOKEN(&token);
      return SYNTAX_OK;
      break;

      // Pravidlo <IDorLiter> -> <liter>
    case NUMBER:
    case STRING:
    case TRUE:
    case FALSE:
    case NIL:
      /* '<liter>' */
      if ((result = Liter()) != SYNTAX_OK) return result;
      // Gen. instr, I_PUSHL pro vlozeni literalu na zasobnik.
      GEN_INSTR(I_PUSHL, (void *) literal, NULL, NULL);
      // TODO pokud se nevlozi do instr. pasky, tak zustane literal neuvolneny.

      return SYNTAX_OK;
      break;

      // neocekavany token
      // default: return SYNTAX_ERROR;    
  }
  return SYNTAX_ERROR;
}

/**
 *  Zpracovani prvniho parametru ve forme vyrazu prikazu write().
 */
int ParamsWrite() {
  int result;

  // Pravidlo <paramsWrite> -> *epsilon*
  if (token.type == R_BRACKET) return SYNTAX_OK;

  // Pravidlo <paramsWrite -> <expr> <paramsWriteNext>
  /* '<expr>' */
  if ((result = expression(COMMA)) != EXPR_OK) return result;
  // Kontrola spravneho typu vyrazu
  if (!IDfound) {
    if (ExprResultType != NUMBER && ExprResultType != STRING)
      return SEM_ERROR;
  }
  // Generovani instrukce I_WRT
  GEN_INSTR(I_WRT, (void *) ExprResult, NULL, NULL);

  /* '<paramsWriteNext>' */
  return ParamsWriteNext();
  //if ((result = ParamsWriteNext()) != SYNTAX_OK) return result;
}

/**
 *  Zpracovani ostatnich parametru ve forme vyrazu prikazu write().
 */
int ParamsWriteNext() {
  int result;

  switch (token.type) {
      // Pravidlo <paramsWriteNext> -> *epsilon*
    case R_BRACKET:
      return SYNTAX_OK;
      break;

      // Pravidlo <paramsWriteNext> -> , <expr> <paramsWriteNext>
    case COMMA: /* ',' */
      GET_NEXT_TOKEN(&token);

      /* '<expr>' */
      if ((result = expression(COMMA)) != EXPR_OK) return result;
      // Kontrola spravneho typu vyrazu
      if (!IDfound) {
        if (ExprResultType != NUMBER && ExprResultType != STRING)
          return SEM_ERROR;
      }
      
      // Generovani instrukce I_WRT
      GEN_INSTR(I_WRT, (void *) ExprResult, NULL, NULL);

      /* '<paramsWriteNext>' */
      return ParamsWriteNext();
      break;
  }
  return SYNTAX_ERROR;
}

/**
 *  Rekurzivni sestup 
 */
int Prog() {
  // Pravidlo '<prog> -> function <restOfFuncs> main ( ) <body> ;'

  int result;

  /* 'function' */
  if (token.type != FUNCTION) return SYNTAX_ERROR;
  GET_NEXT_TOKEN(&token);

  /* '<restOfFuncs>' */
  if ((result = RestOfFuncs()) != SYNTAX_OK) return result;

  /* 'main' */
  // Ulozeni ID funkce do globalni tabulky symbolu.
  stableItemPtr stItem = NULL;
  PUT_KEY(gTable, stItem);

  // Generovani instrukce navesti funkce I_LBL (main ma 0 parametru).
  GEN_INSTR(I_LBL, (void *) 0, NULL, NULL);
  // Ulozeni ukazatele na prvni instrukci funkce.
  stItem->start = (iItem*) listGetPointerLast(iList);
  // Nastaveni startovaci instrukce celeho programu.
  iList->active = stItem->start;
  GET_NEXT_TOKEN(&token);

  /* '(' */
  if (token.type != L_BRACKET) return SYNTAX_ERROR;
  GET_NEXT_TOKEN(&token);

  /* ')' */
  if (token.type != R_BRACKET) return SYNTAX_ERROR;
  GET_NEXT_TOKEN(&token);

  // Ulozeni hodnoty poctu formalnich parametru funkce. V tomto pripade 0.
  assert(lTable->itemCounter == 0);
  stItem->number = lTable->itemCounter;

  /* '<body>' */
  if ((result = Body()) != SYNTAX_OK) return result;

  /* ';' */
  if (token.type != SEMICOLON) return SYNTAX_ERROR;
  GET_NEXT_TOKEN(&token);

  // 'EOF'
  if (token.type != EndOfFile) return SYNTAX_ERROR;

  // Doplneni ukazatelu do instrukci I_CALL pro volani funkci pri vzajemne rekurzi.
  if ((result = MutRec(mrList)) != SEM_OK) return result;

  // Vycisteni lokalni a globalni tabulky symbolu.
  stableClear(lTable);
  stableClear(gTable);

  return SYNTAX_OK;
}

/**
 *  Zahajeni syntakticke analyzy.
 *  
 *  @param gTable - Ukazatel na glob. tabulku symoblu.
 *  @param iList - Ukazatel na seznam instrukci.   
 */
int parse(Tstable *tableG, Tstable *tableL, pInstrList list) {
  int result;

  gTable = tableG;
  lTable = tableL;
  iList = list;

  // Inicializace seznamu pro vzajemnou rekurzi.
  mrlInit(&mrList);
  // Inicializace zasobniku pro vyrazy.
  stackExpInit(&se);

  getNextToken(&token);
  if (token.type == INTER_ERROR || token.type == LEX_ERROR) {
    // Chyba ihned pri prvnim tokenu.
    result = token.type;
  }
  else {
    result = Prog();
  }

  // Zruseni seznamu pro vzajemnou rekurzi.
  mrlDispose(&mrList);
  // Zruseni zasobniku pro vyrazy.
  stackExpFree(&se);

  return result;
}

char* condition2Str(int cond) {
  switch (cond) {
    case WAIT: return "CEKAM";
    case WORK: return "ZJEDNODUSUJI";
    case FSYN: return "NAVRAT/CHYBA SYN";
    case FSEM: return "NAVRAT/CHYBA SEM";
    default: return "???";
  }

}

