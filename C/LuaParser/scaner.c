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
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "str.h"
#include "scaner.h"
#include "error.h"

#define DEBUG 0

FILE *input;
/*bool lookHead;*/

#if DEBUG
char* token2Str(int i);
#endif

searchTableItem keyWordsTable[] = {
  {"if", IF},
  {"then", THEN},
  {"else", ELSE},
  {"local", LOCAL},
  {"do", DO},
  {"while", WHILE},
  {"end", END},
  {"true", TRUE},
  {"false", FALSE},
  {"read", READ},
  {"write", WRITE},
  {"sort", SORT},
  {"substr", SUBSTR},
  {"type", TYPE},
  {"find", FIND},
  {"function", FUNCTION},
  {"return", RETURN},
  {"nil", NIL},
  {"repeat", REPEAT},
  {"until", UNTIL},
  {"and", AND},
  {"break", BREAK},
  {"elseif", ELSEIF},
  {"for", FOR},
  {"in", IN},
  {"not", NOT},
  {"or", OR},
};

const int keywordsTableSize =
        sizeof (keyWordsTable) / sizeof (keyWordsTable[0]);

/**
 * Nastavi vstupni soubor
 * @param f
 */
void setSourceFile(FILE *f) {
  input = f;
  /*lookAhead = false; */
}
/*
void setLookAhead() {
  lookAhead = true;
#if DEBUG
  fprintf(stderr, "Look ahead\n");
#else
}*/

/* implementace vyhledavaci tabulkou s pravdepodobnosti vyskytu */
int isReservedWord(string *s) {
  char *str = s->str;
  int i = 0;
  searchTableItem tmp;

  //porovnani s prvni prvken u prvniho neprohazujeme
  if (strcmp(str, keyWordsTable[i].keyWord) == 0) {
    return keyWordsTable[i].ID;
  }

  for (i = 1; i < keywordsTableSize; i++) {
    //printf("%s\n", keyWordsTable[i].keyWord);
    if (strcmp(str, keyWordsTable[i].keyWord) == 0) {
      //pri nalezeni posouvame prvek o 1 dopredu
      //printf("%s %d - %s %d\n", keyWordsTable[i].keyWord, keyWordsTable[i].ID, keyWordsTable[i-1].keyWord, keyWordsTable[i-1].keyWord);

      tmp = keyWordsTable[i - 1];
      keyWordsTable[i - 1] = keyWordsTable[i];
      keyWordsTable[i] = tmp;

      //printf("%s %d - %s %d\n", keyWordsTable[i].keyWord, keyWordsTable[i].ID, keyWordsTable[i-1].keyWord, keyWordsTable[i-1].ID);
      return keyWordsTable[i - 1].ID; //vraceni indexu posunuteho prvku
    }
  }
  return ID;
}
#if DEBUG
int line_counter = 1;
int count = 0;
void getNextTokenX(Ttoken *token);

void getNextToken(Ttoken *token) {

  getNextTokenX(token);

  fprintf(stderr, "%d. token = < %s >\n", ++count, token2Str((int) token->type));

}
#endif
/**
 * 
 * @param token
 */
#if DEBUG

void getNextTokenX(Ttoken *token) {
#else

void getNextToken(Ttoken *token) {
#endif
  assert(input != NULL);
  assert(token != NULL);

  /*if (!lookAhead) {
    return;
  }*/

  string number;

  int escapeIter = 0; // for \ddd
  int escapedChar = 0; // for \dd
  int state = ST_INIT;
  int c; // char
  int prev_c = 0; // predchozi char detekce "]]" 

  while (1) {
    c = fgetc(input);
#if DEBUG
    if (c == '\n')
      fprintf(stderr, "--- LINE %d ---\n", ++line_counter);
#endif
    switch (state) {
      case ST_INIT:
        if (isspace(c)) {
          continue;
        }
        switch (c) {
            // stavy bez prechodu
          case '(':
            token->type = L_BRACKET;
            return;

          case ')':
            token->type = R_BRACKET;
            return;

          case ';':
            token->type = SEMICOLON;
            return;

          case ',':
            token->type = COMMA;
            return;

          case '+':
            token->type = PLUS;
            return;

          case '%':
            token->type = MOD;
            return;

          case '#':
            token->type = SHARP;
            return;

          case '*':
            token->type = MUL;
            return;

          case '/':
            token->type = DIV;
            return;

          case '^':
            token->type = CARET;
            return;
            // stavy s prechodem
          case '-':
            state = ST_MINUS; // INIT -> -
            break;

          case '=':
            state = ST_EQ; // = -> ==
            break;

          case '.':
            state = ST_PRE_CONCAT; // INIT -> .
            break;

          case '>':
            state = ST_MORE; // INIT -> >
            break;

          case '<':
            state = ST_LESS; // INIT -> <
            break;

          case '~':
            state = ST_PRE_N_EQ; // INIT -> ~
            break;

          case '"':
            if (strInit(&token->data.str_data) == STR_ERROR) {//init string
              token->type = INTER_ERROR;
              return;
            }
            state = ST_STRING; // INIT -> STR
            break;
          case EOF:
            token->type = EndOfFile;
            return;

          default: // switch(c)
            if (isalpha(c) || c == '_') {
              if (strInit(&token->data.str_data) == STR_ERROR) {//init string
                token->type = INTER_ERROR;
                return;
              }
              if (strAddChar(&token->data.str_data, c) == STR_ERROR) {
                token->type = INTER_ERROR;
                return;
              }
              state = ST_ID; // INIT -> ID
            }
            else if (isdigit(c)) {
              if (strInit(&number) == STR_ERROR) {
                token->type = INTER_ERROR;
                return;
              }
              if (strAddChar(&number, c) == STR_ERROR) {
                token->type = INTER_ERROR;
                return;
              }
              state = ST_INT;
            }
            else {
              strFree(&token->data.str_data);
              token->type = LEX_ERROR;
              return;
            }
            break;
        } //switch c
        break; //INIT STATE

      case ST_MINUS:
        if (c != '-') {
          if (!isspace(c)) {
            ungetc(c, input);
          }
          token->type = MINUS;
          return;
        }
        else {
          state = ST_SHORT_COMMENT; // - -> --
        }
        break; //ST_MINUS

      case ST_SHORT_COMMENT:
        // accept all

        if (c == '\n') {
          state = ST_INIT;
        }
        else if (c == EOF) { //pro bezpecnost
          token->type = EndOfFile;
          return;
        }
        else if (prev_c == '[' && c == '[') {
          state = ST_LONG_COMMENT; // -- -> --[[
        }
        prev_c = c;
        break; //ST_SHORT_COMMENT

      case ST_LONG_COMMENT:
        // accept all
        // mozna cyklit tu
        if (c == EOF) {
          token->type = LEX_ERROR;
          return;
        }
        else if (prev_c == ']' && c == ']') {
          state = ST_INIT; // --[[ -> INIT
        }
        prev_c = c;
        break; // ST_LONG_COMMENT

      case ST_EQ:
        if (c == '=') {
          token->type = EQUAL;
        }
        else {
          if (!isspace(c)) {
            ungetc(c, input);
          }
          token->type = ASSIGN;
        }
        return;
        break; // ST_EQ

      case ST_MORE:
        if (c == '=') {
          token->type = MORE_EQUAL;
        }
        else {
          if (!isspace(c)) {
            ungetc(c, input);
          }
          token->type = MORE;
        }
        return;
        break; // ST_MORE

      case ST_LESS:
        if (c == '=') {
          token->type = LESS_EQUAL;
        }
        else {
          if (!isspace(c)) {
            ungetc(c, input);
          }
          token->type = LESS;
        }
        return;
        break; // ST_LESS

      case ST_PRE_N_EQ:
        if (c == '=') {
          token->type = NOT_EQUAL;
        }
        else {
          token->type = LEX_ERROR;
        }
        return;
        break; // ST_PRE_N_EQ

      case ST_PRE_CONCAT:
        if (c == '.') {
          token->type = CONCAT;
        }
        else {
          token->type = LEX_ERROR;
        }
        return;
        break; // ST_PRE_CONCAT

      case ST_ID:
        if (isalnum(c) || c == '_') {
          if (strAddChar(&token->data.str_data, c) == STR_ERROR) {
            token->type = INTER_ERROR;
            return;
          }
        }
        else {
          if (!isspace(c)) {
            ungetc(c, input);
          }

          int status = isReservedWord(&token->data.str_data);
          token->type = status;
          if (status != ID) { //neni to ID bylo to rezervovane slovo nepotrebujem retezec
            strFree(&token->data.str_data);
          }
          /* neni potreba-> else { //vytah char* ze struct string
            token->data.identificator = token->data.str_data.str;
          }*/
          return;
        }
        break; // ST_ID

      case ST_STRING:
        if (c == '\\') {
          state = ST_ESCAPE; // STR -> ESC
        }
        else if (c == '"') {
          token->type = STRING;
          return; // konec retezce
        }
        else if (c == EOF) {
          strFree(&token->data.str_data);
          token->type = LEX_ERROR;
          return;
        }
        else {
          if (c > 1) {
            if (strAddChar(&token->data.str_data, c) == STR_ERROR) {
              token->type = INTER_ERROR;
              return;
            }
          }
          else {
            strFree(&token->data.str_data);
            token->type = LEX_ERROR;
          }
        }
        break; // ST_STRING

      case ST_ESCAPE:
        if (c == 'n') {
          if (strAddChar(&token->data.str_data, '\n') == STR_ERROR) {
            token->type = INTER_ERROR;
            return;
          }
          state = ST_STRING; // ESC -> STR
        }
        else if (c == 't') {
          if (strAddChar(&token->data.str_data, '\t') == STR_ERROR) {//init string
            token->type = INTER_ERROR;
            return;
          }
          state = ST_STRING; // ESC -> STR
        }
        else if (c == '"') {
          if (strAddChar(&token->data.str_data, '"') == STR_ERROR) {//init string
            token->type = INTER_ERROR;
            return;
          }
          state = ST_STRING; // ESC -> STR
        }
        else if (c == '\\') {
          if (strAddChar(&token->data.str_data, '\\') == STR_ERROR) {//init string
            token->type = INTER_ERROR;
            return;
          }
          state = ST_STRING; // ESC -> STR
        }
        else if (c >= '0' && c <= '2') {
          escapeIter = 0;
          escapedChar = c - '0';
          state = ST_ESCAPED_DDD; // ESC -> ddd
        }
        else {
          strFree(&token->data.str_data);
          token->type = LEX_ERROR;
          return;
        }
        break; // ST_ESCAPE

      case ST_ESCAPED_DDD:
        if (!isdigit(c)) {
          strFree(&token->data.str_data);
          token->type = LEX_ERROR;
          return;
        }
        escapedChar *= 10;
        escapedChar += (c - '0'); // c bude vzdy digit
        escapeIter++;
        if (escapeIter == 2) {
          if (escapedChar == 0 || escapedChar > 255) { //mensi nez 0 byt nemuze - nebyl by prijat automatem
            strFree(&token->data.str_data);
            token->type = LEX_ERROR;
            return;
          }
          else {
            if (strAddChar(&token->data.str_data, escapedChar) == STR_ERROR) {//init string
              token->type = INTER_ERROR;
              return;
            }
            state = ST_STRING;
            break;
          }
          state = ST_STRING;
        }
        break; // ST_ESCAPED_DDD
      case ST_INT:
        if (isdigit(c)) {
          if (strAddChar(&number, c) == STR_ERROR) {//init string
            token->type = INTER_ERROR;
            return;
          }
        }
        else if (c == '.') {
          if (strAddChar(&number, c) == STR_ERROR) {//init string
            token->type = INTER_ERROR;
            return;
          }
          state = ST_POINT;
        }
        else if (c == 'e' || c == 'E') {
          if (strAddChar(&number, c) == STR_ERROR) {//init string
            token->type = INTER_ERROR;
            return;
          }
          state = ST_E;
        }
        else {
          if (!isspace(c)) {
            ungetc(c, input);
          }
          token->type = NUMBER;
          token->data.value = strtod(number.str, NULL); // neni potreba endptr
          // spravnost zajisti automat
          strFree(&number);
          return;
        }
        break; // ST_INT

      case ST_POINT:
        if (isdigit(c)) {
          if (strAddChar(&number, c) == STR_ERROR) {//init string
            token->type = INTER_ERROR;
            return;
          }
          state = ST_FLOAT;
        }
        else {
          token->type = LEX_ERROR;
          strFree(&number);
          return;
        }
        break; // ST_POINT

      case ST_FLOAT:
        if (isdigit(c)) {
          if (strAddChar(&number, c) == STR_ERROR) {
            token->type = INTER_ERROR;
            return;
          }
        }
        else if (c == 'e' || c == 'E') {
          if (strAddChar(&number, c) == STR_ERROR) {
            token->type = INTER_ERROR;
            return;
          }
          state = ST_E;
        }
        else {
          if (!isspace(c)) {
            ungetc(c, input);
          }
          token->type = NUMBER;
          token->data.value = strtod(number.str, NULL); // neni potreba endptr
          // spravnost zajisti automat
          strFree(&number);
          return;
        }
        break; // ST_FLOAT

      case ST_E:
        if (isdigit(c)) {
          if (strAddChar(&number, c) == STR_ERROR) {
            token->type = INTER_ERROR;
            return;
          }
          state = ST_EXPONENT;
        }
        else if (c == '-' || c == '+') {
          if (strAddChar(&number, c) == STR_ERROR) {
            token->type = INTER_ERROR;
            return;
          }
          state = ST_SIGN;
        }
        else {
          strFree(&number);
          token->type = LEX_ERROR;
          return;
        }
        break; // ST_E

      case ST_SIGN:
        if (isdigit(c)) {
          if (strAddChar(&number, c) == STR_ERROR) {
            token->type = INTER_ERROR;
            return;
          }
          state = ST_EXPONENT;
        }
        else {
          strFree(&number);
          token->type = LEX_ERROR;
          return;
        }
        break; // ST_SIGN

      case ST_EXPONENT:
        if (isdigit(c)) {
          if (strAddChar(&number, c) == STR_ERROR) {
            token->type = INTER_ERROR;
            return;
          }
        }
        else {
          if (!isspace(c)) {
            ungetc(c, input);
          }
          token->type = NUMBER;
          token->data.value = strtod(number.str, NULL); // neni potreba endptr
          // spravnost zajisti automat
          strFree(&number);
          return;
        }
        break; // ST_EXPONENT

      default:
        fprintf(stderr, "Impl err\n");
        exit(1);
    } //switch state
  }
}

char* token2Str(int i) {

  switch (i) {
    case PLUS: return "+";
    case MINUS: return "-";
    case MUL: return "*";
    case DIV: return "/";
    case CARET: return "^";
    case L_BRACKET: return "(";
    case R_BRACKET: return ")";
    case CONCAT: return "..";
    case EQUAL: return "==";
    case NOT_EQUAL: return "~=";
    case LESS: return "<";
    case LESS_EQUAL: return "<=";
    case MORE: return ">";
    case MORE_EQUAL: return ">=";

    case SEMICOLON: return ";";
    case ASSIGN: return "=";
    case COMMA: return ",";

    case MOD: return "%";

    case SHARP: return "#";

      //vestavene funkce
    case SORT: return "SORT";
    case TYPE: return "TYPE";
    case SUBSTR: return "SUBSTR";
    case FIND: return "FIND";

      // rezervovana slova
    case DO: return "DO";
    case ELSE: return "ELSE";
    case END: return "END";
    case FALSE: return "FALSE";
    case FUNCTION: return "FUNCTION";
    case IF: return "IF";
    case LOCAL: return "LOCAL";
    case NIL: return "NIL";
    case READ: return "READ";
    case RETURN: return "RETURN";
    case THEN: return "THEN";
    case TRUE: return "TRUE";
    case WHILE: return"WHILE";

    case WRITE: return "WRITE";

    case AND: return "AND";
    case BREAK: return "BREAK";
    case ELSEIF: return "ELSEIF";
    case FOR: return "FOR";
    case IN: return "IN";
    case NOT: return "NOT";
    case OR: return "OR";
    case REPEAT: return "REPEAT";
    case UNTIL: return "UNTIL";

    case NUMBER: return "NUMBER";
    case STRING: return "STRING";
    case ID: return "ID";

    case LEX_ERROR: return "LEX_ERR";
    case INTER_ERROR: return "INTER_ERR";

    case EndOfFile: return "EOF";
    default: return "DEFAULT TOKEN";
  }
}
