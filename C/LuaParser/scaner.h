/*
 * Implementace interpretu imperativniho jazyka IFJ2011
 *
 * Autori:
 *  xbedna45 - Bednarik Jan
 *  xblaha22 - Blaha Hynek
 *  xjanys00 - Janys Martin
 *  xmacha48 - Machacek Ondrej
 */

#ifndef SCANER_H
#define SCANER_H

#include <stdio.h>
#include "str.h"
#include "error.h"

enum tokens {
    //LEX_ERROR = 1,#include "error.h"
    //INTER_ERROR = 5,#include "error.h"

    // -- START TABULKY PRIORIT
    PLUS = 6, // '+'
    MINUS, // '-'
    MUL, // '*'
    DIV, // '/'
    CARET, // '^' //10
    L_BRACKET, // '('
    R_BRACKET, // ')'
    ID, //  
    STRING, //  
    NUMBER, //  
    FALSE, //
    TRUE, //
    NIL, //
    ENDING_TOKEN, // $  POUZIVAN POUZE VE VYRAZECH
    CONCAT, // ".." // 20
    EQUAL, // '==' 
    NOT_EQUAL, // '~='
    LESS, // '<'
    LESS_EQUAL, // '<='
    MORE, // '>'
    MORE_EQUAL, // '>='
    MOD, // '%'
    SHARP, // '#'
    AND,
    OR,
    NOT,
    // -- MOZNE UKONCOVACI TOKENY VYRAZU
    SEMICOLON, // ';'
    COMMA, // ','
    DO,
    THEN,
    E, // E POUZIVAN POUZE VE VYRAZECH
    ASSIGN, // '='



    //vestavene funkce
    SORT,
    TYPE,
    SUBSTR,
    FIND,

    // rezervovana slova

    ELSE,
    END,
    FUNCTION, //40
    IF,
    LOCAL,
    READ,
    RETURN,
    WHILE,
    WRITE,
    BREAK,
    ELSEIF,
    FOR,
    IN,
    REPEAT,
    UNTIL,

    EndOfFile, //60
};

enum states {
    ST_INIT,
    ST_PRE_N_EQ, // ~
    ST_PRE_CONCAT, // .
    ST_EQ,
    ST_MINUS,
    ST_LESS,
    ST_MORE,

    ST_SHORT_COMMENT,
    ST_LONG_COMMENT,

    ST_ID,

    ST_STRING,
    ST_ESCAPE,
    ST_ESCAPED_DDD,

    ST_INT,
    ST_POINT,
    ST_FLOAT,
    ST_E,
    ST_SIGN,
    ST_EXPONENT,
};

typedef struct {
    int positionToAct;
    int datatypeE;
} TE;

typedef union {
    string str_data; // string
    char* identificator; // jmeno-ID, je tam string ale lze pristupovat pres char*
    double value; // number
    TE Edata; // POUZIVAN POUZE VE VYRAZECH
} Udata;

typedef struct {
    int type;
    Udata data;
} Ttoken;

typedef struct table {
    char* keyWord;
    int ID;
} searchTableItem;

void setSourceFile(FILE *f);

/*
  funkce getNexTtoken vraci tokeny (struktury) s typem (z enumu vyse) a s unionem dat.
  data jsou inicializovana pouze pri ID, NUM, STRING
  ID nese string str_data, ktery obsahuje identifikator
  NUM nese value - double nacteneho cisla
  STRING - nese string
 */
void getNextToken(Ttoken *token);
char* token2Str(int);

#endif
