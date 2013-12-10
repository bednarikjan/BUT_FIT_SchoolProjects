/*
 * Implementace interpretu imperativniho jazyka IFJ2011
 *
 * Autori:
 *  xbedna45 - Bednarik Jan
 *  xblaha22 - Blaha Hynek
 *  xjanys00 - Janys Martin
 *  xmacha48 - Machacek Ondrej
 *
 *  Implementace interpretu.
 */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "str.h"
#include "ial.h"
#include "stack.h"
#include "interpret.h"
#include "error.h"

#define IOK 0
#define DEBUG_I 0

#if DEBUG_I
const char *instruction_name[] = {
  "I_PUSHV",   // vloz hodnotu
  "I_PUSHA",   // vloz adresu
  "I_PUSHL",	 // vloz literal
  "I_ASGNV",   // priradi hodnotu promene, na urcitem indexu
  "I_ACTTOP",  // srovna act s top
  "I_CLR",     // cisti mezivysledky v zaporne casti zasobniku (top-to-act)
  "I_POP",     // zrusi prvek na vrcholu zasobniku

  /* Aritmetika */
  "I_ADD",      /* soucet */
  "I_SUB",      /* odecitani */
  "I_MUL",      /* nasobeni */
  "I_DIV",      /* deleneni */
  "I_MOD",      /* modulo, pripadne rozsireni */
  "I_POW",
  "I_CON", 		/* konkatenace */
  "I_LEN",	  /* delka retezce */

  /* Relace/rovnost/logicke operace */
  "I_EQU",      /* == */
  "I_NEQ",	    /* != */
  "I_LES",      /* < */
  "I_GRT",      /* > */
  "I_LSE",      /* <= */
  "I_GRE",      /* >= */

  /* Skoky */
  "I_JMP",      /* skok/goto */
  /* argument1 - adresa pro true
   * argument2 - adresa pro false
   * result - vysledek vyrazu */
  "I_CJMP",	// Podmineny skok
  "I_CALL",
  "I_RET",
  "I_LBL",	

  /* Funkce */
  "I_WRT",      /* zapis na stdout */
  "I_READ",     /* cteni ze stdin */
  "I_SORT",     /* trideni */
  "I_TYPE",     /* typ dat */
  "I_SUBST",    /* vraci podretezec */
  "I_FIND",     /* vyhledani podretezce */
};
#endif

string types[] = {
		{"string", 7, 8},
		{"boolean", 8, 9},
		{"number", 7, 8},
		{"nil", 4, 5},
};

const int TYPE_PARAM = 1;
const int SORT_PARAM = 1;
const int FIND_PARAM = 2;
const int SUBSTR_PARAM = 3;

int interpret(pInstrList iList) {
	int err;
	Tstack stack; // zasobnik pro promene
  stackInit(&stack);
	err = interpret_(iList, &stack);

	if(err != IOK) {
		stackDisposeVariables(&stack, stack.top);
		stackFree(&stack);
	}
	return err;
}

/**
 * Interpretace triadresneho kodu.
 * 
 * @param instrList - seznam instrukci
 */
int interpret_(pInstrList iList, Tstack *stack)
{
  pTAC i;	// aktualni instrukce
  
  stackPushAddress(stack, NULL);
  stackActToTop(stack);

  while((i = listGetInstruction(iList)) != NULL) {
	#if DEBUG_I
	  stackPrint(stack, 0);
	  fprintf(stderr, "\n");
	  fprintf(stderr, "%s\t\t[a1 = %d, a2 = %d, r = %d]\n", instruction_name[(int)i->operator], (int)i->argument1, (int)i->argument2, (int)i->result);
	#endif
    switch ((int)i->operator) {
	  /* Zasobnik */
	  case I_PUSHV:
	  {
	    /**
		 * Uloz na vrchol zasobniku hodnotu ktera se nachazi
		 *  na zasobniku na indexu (int)i->argument1
		 */
		  Tvariable *v;
		  // Pokud je (int)i->argument1 == 0, vloz na zasobnik nil
		  if(!(int)i->argument1) {
			  v = stackGetVal(stack, 0);
			  if(v == NULL) {
			  // pokud neni null nic nedelej
			  // precti ukazatel na zaporne cislo, prepis nullem, clearzasobnik(I_CLR), potom pushni ukazatel ktery jsem si nacet
				  v = (Tvariable *)malloc(sizeof(Tvariable));
				  if(!v)
					  return INTER_ERROR; //E_ALLOC;
				  v->type = NIL_TYPE;
				  stackPush(stack, v);
          free(v);
			  } else {
				  stackActToTop(stack);
			  }
		  } else if((int)i->argument1 > 0) {
			  v = stackGetVal(stack, (int)i->argument1);
			  stackPush(stack, v);
		  } else {
			  v = stackGetVal(stack, (int)i->argument1);
			  stack->stack[stack->act - (int)i->argument1].variable = NULL;
			  stackTopToAct(stack);
			  stackPush(stack, v);
        free(v);
		  }
      }
		break;
	  case I_PUSHA:
	    /**
		 * Vlozi na zasobnik adresu, ktery je predan v
		 * (iItem *)i->argument1
		 */

		stackPushAddress(stack, (iItem*)i->argument1);
	    break;
	  case I_PUSHL:
	  {
		/**
		 * Vlozi na zasobnik literal, ktery je predan v
		 * (Tvariable *)i->argument1
		 */
		Tvariable *v = (Tvariable *)i->argument1;

		stackPush(stack, v);
	  }
		break;
	  case I_ASGNV:
	  {
		/**
		 * Uloz na zasobnik na indexu (int)i->argument2, hodnotu
		 * ktera se nachazi na zasobniku na indexu (int)i->argument1
		 */
		Tvariable *v;
		v = stackGetVal(stack, (int)i->argument1);
		stackSetVal(stack, v, (int)i->argument2);
    }
		break;
	  case I_ACTTOP:
		/**
		 * Srovna ukaztel na act s ukazetalem na top
		 */
		stackActToTop(stack);
		break;
	  case I_CLR:
		stackTopToAct(stack);
		break;
	  case I_POP:
		stackPop(stack);
		stackActToTop(stack);
		break;
	  /* Aritmetika */
	  case I_ADD:
	  case I_SUB:
	  case I_MUL:
	  case I_DIV:
	  case I_MOD:
	  case I_POW:
	  {
		  int e;
		  e = aritmetika(stack, i);
		  if(e != IOK)
			  return e;
	  }
	  break;
	  case I_LEN:
	  {
		  Tvariable *v1 = stackGetVal(stack, (int)i->argument1);
		  Tvariable r;

		  if(v1->type != STR_TYPE)
			  return INTERPRET_ERROR;
		  r.type = NUM_TYPE;
		  //strInit(&r.data.str);
		  r.data.num = v1->data.str.length;
		  if(stackSetVal(stack, &r, (int)i->result) != STACK_SUCCESS)
				return INTER_ERROR;
      
	  }
	  break;
	  case I_CON:
	  {
		  Tvariable *v1 = stackGetVal(stack, (int)i->argument1);
		  Tvariable *v2 = stackGetVal(stack, (int)i->argument2);
		  Tvariable r;

		  if(v1->type != STR_TYPE || v2->type != STR_TYPE)
			  return INTERPRET_ERROR;
		  r.type = STR_TYPE;
		  r.data.str = strConcat(v1->data.str, v2->data.str);
		  if(stackSetVal(stack, &r, (int)i->result) != STACK_SUCCESS)
			  return INTER_ERROR;
		  // Uvolneni stringu
		  strFree(&r.data.str);
	  }
	  break;
	  /* Relace/rovnost/logicke operace */
	  case I_EQU:
	  case I_NEQ:
	  case I_LES:
	  case I_GRT:
	  case I_LSE:
	  case I_GRE:
	  {
		  int e;
		  e = cmp(stack, i);
		  if(e != IOK)
			  return INTERPRET_ERROR;
	  }
	  break;
	  /* Skoky */
	  case I_JMP:
		/**
		 * Skoci na instrukci predanou jako (iItem*)i->argument1
		 */
		listGoto(iList, (iItem*)i->argument1);
		continue;
		break;
	  case I_CJMP:
	  {
		/**
		 * Skoci podle hodnoty (int)i->result na zapornou((iItem*)i->argument2)
		 * nebo kladnou vetev((iItem*)i->argument1)
		 */
		Tvariable *v = stackGetVal(stack, (int)i->result);
		if((v->type == BOOL_TYPE && v->data.boolean == false) || (v->type == NIL_TYPE)) {
			listGoto(iList, (iItem*)i->argument2);
		} else {
			listGoto(iList, (iItem*)i->argument1);
		}
		continue;
	  }
		break;
	  case I_LBL:
		break;
	  case I_RET:
	  {
		//  iItem *adr = NULL;
	       iItem *adr = stack->stack[stack->act - (int)i->argument1].instruction;
		stack->stack[stack->act - (int)i->argument1].variable = NULL;
		//if(!adr) {
			
		//}
		Tvariable *v;
		if(!(int)i->result) {
			v = (Tvariable *) malloc(sizeof(Tvariable));
			if(!v)
				return INTER_ERROR; //E_ALLOC;
			v->type = NIL_TYPE;
			//stackPush(stack, v);
		} else {
			v = stackGetVal(stack, (int)i->argument2);

			//stackPush(stack, v);
		}
		stackSetVal(stack, v, (int)i->argument1);
		stackTopToAct(stack);
		stackDisposeVariables(stack, (int)i->argument1-1);
		
		listGoto(iList, adr);
		stackActToTop(stack);
    
    if(!(int)i->result) {
      free(v); // TODO posoudit
    }
		continue;
	  }
		break;
	  case I_CALL:
		stackAlignParams(stack, (int)((iItem*)i->argument1)->tac.argument1 + 1);
		listGoto(iList, (iItem*)i->argument1);
		continue;
		break;
	  /* Funkce */
	  case I_WRT:
	  {
		// nesmi byt nil a boolean
		Tvariable *val;
		val = stackGetVal(stack, (int)i->argument1);

		if(val->type == BOOL_TYPE || val->type == NIL_TYPE) {
			return INTERPRET_ERROR; //EWRITE;
		}
		if(val->type == NUM_TYPE) {
			printf("%g", val->data.num);
		} else if(val->type == STR_TYPE) {
			printf("%s", val->data.str.str);
		}
	  }
		break;
	  case I_READ:
	  {
		Tvariable val;

		switch((int)i->argument2) {
			case RF_NUM:
				readNum(&val);
				break;
			case RF_EOL:
				readStringEOL(&val);
				break;
			case RF_EOF:
				readStringEOF(&val);
				break;
			case RF_CHARS:
				readStringNum(&val, (int)i->result);
				break;
			default:
				return INTERPRET_ERROR; // EREAD
		}
		stackSetVal(stack, &val, (int)i->argument1);
    if ((int)i->argument2 != RF_NUM) {
      strFree(&val.data.str);
    }
		break;
	  }
	  case I_SORT:
	  {
		  stackAlignParams(stack, SORT_PARAM);
		  Tvariable v, *val;
		  v.type = STR_TYPE;

		  val = stackGetVal(stack, 1);

		  if(val->type != STR_TYPE)
			  return INTERPRET_ERROR; //EINVARG;
		  //strInit(&v.data.str); -M
		  string s = sort(val->data.str);
      if (s.str == NULL) {
        return INTER_ERROR;
      }
      v.data.str = s;
		  //if(strCopyString(&v.data.str, &s) != STR_SUCCESS) -M
			  //return INTER_ERROR; //EALLOC;
		  stackDisposeVariables(stack, SORT_PARAM);
		  stackSetVal(stack, &v, (int)i->argument1);
      strFree(&s);
		  stackActToTop(stack);
	  }
		break;
	  case I_FIND:
	  {
		  stackAlignParams(stack, FIND_PARAM);
#if DEBUG_I
		  stackPrint(stack, 0);
#endif
		  Tvariable *val1 = stackGetVal(stack, 1);
		  Tvariable *val2 = stackGetVal(stack, 2);
		  Tvariable val;
		  double d;

		  if(val1->type != STR_TYPE || val2->type != STR_TYPE) {
			  val.type = NIL_TYPE;
	  	  } else {
	  		  val.type = NUM_TYPE;
	  		  d = (double)find(val2->data.str, val1->data.str) + 1;
	  		  if(d == 0) {
	  			val.type = BOOL_TYPE;
	  			val.data.boolean = false;
	  		  } else {
	  			  val.data.num = (double)find(val2->data.str, val1->data.str) + 1;
	  		  }
	  		  if(val1->data.str.length == 0)
	  			val.data.num = 0;
		  }
		  stackDisposeVariables(stack, FIND_PARAM);
		  stackSetVal(stack, &val, (int)i->argument1);
		  stackActToTop(stack);
	  }
		break;
	  case I_SUBST:
	  {
		  stackAlignParams(stack, SUBSTR_PARAM);
		  Tvariable val;
		  Tvariable *num2  = stackGetVal(stack, 1);
		  Tvariable *num1 = stackGetVal(stack, 2);
		  Tvariable *str = stackGetVal(stack, 3);

      string s;
      
      int flag = num2->type != NUM_TYPE || num1->type != NUM_TYPE || str->type != STR_TYPE;
      
		  if(flag) {
			  val.type = NIL_TYPE;
		  } else {
			  //strInit(&val.data.str); -M
			  val.type = STR_TYPE;
			  s = substr(str->data.str, (int)num1->data.num, (int)num2->data.num);
			  //strCopyString(&val.data.str, &s); -M
        if (s.str == NULL) {
          return INTER_ERROR;
        }
        val.data.str = s;
		  }
		  stackDisposeVariables(stack, SUBSTR_PARAM);
		  stackSetVal(stack, &val, (int)i->argument1);
      if (!flag) 
        strFree(&s);//-M
		  
      stackActToTop(stack);
	  }
		break;
	  case I_TYPE:
	  {
		  stackAlignParams(stack, TYPE_PARAM);
		  Tvariable r;
		  Tvariable* v = stackGetVal(stack, 1);

		  r.type = STR_TYPE;
		  strInit(&r.data.str);
		  if(v->type == STR_TYPE) {
			  strCopyString(&r.data.str, &types[IC_STR]);
		  } else if(v->type == NUM_TYPE) {
			  strCopyString(&r.data.str, &types[IC_NUM]);
		  } else if(v->type == BOOL_TYPE) {
			  strCopyString(&r.data.str, &types[IC_BOOL]);
		  } else if(v->type == NIL_TYPE) {
			  strCopyString(&r.data.str, &types[IC_NIL]);
		  }

		  stackDisposeVariables(stack, TYPE_PARAM);
		  if(stackSetVal(stack, &r, (int)i->argument1) == STACK_ERROR) {
			  return INTER_ERROR;
		  }
		  stackActToTop(stack);
      strFree(&r.data.str);
	  }
		break;
	  default:
		  return INTERPRET_ERROR;
	}
	listNext(iList);

  }
  stackDisposeVariables(stack, stack->top);
  stackFree(stack);
  return IOK;
}

/**
 * Nacte cislo do pararametru.
 *
 * @param n - odkaz na pamet kam se cislo ma ulozit
 * @return - CHYBA || OK
 */
int readNum(Tvariable *v) {
	errno = 0;
	int r;
	if(((r = scanf("%lf", &v->data.num)) != 1) || (errno != 0)) {
		v->type = NIL_TYPE;
		return INTERPRET_ERROR; //ENUMBER;
	}else
		v->type = NUM_TYPE;
	return IOK;
}

int readStringNum(Tvariable *val, int num) {
	strInit(&val->data.str);
	val->type = STR_TYPE;

	int c;
	while(num-- && (c = getchar()) != EOF) {
		if(strAddChar(&val->data.str, c) == STR_ERROR) {
			return INTER_ERROR;
		}
	}

	if(val->data.str.length == 0) {
		val->type = NIL_TYPE;
		strClear(&val->data.str);
	}

	return IOK;
}

int readStringEOL(Tvariable *val) {
	strInit(&val->data.str);
	val->type = STR_TYPE;

	int c;
	while((c = getchar()) != '\n' && c != EOF) {
		if(strAddChar(&val->data.str, c) == STR_ERROR) {
			return INTER_ERROR;
		}
	}

	if(val->data.str.length == 0 && c != '\n') {
		val->type = NIL_TYPE;
		strClear(&val->data.str);
	}

	return IOK;
}

int readStringEOF(Tvariable *val) {
	strInit(&val->data.str);
	val->type = STR_TYPE;

	int c;
	while((c = getchar()) != EOF) {
		if(strAddChar(&val->data.str, c) == STR_ERROR) {
			return INTER_ERROR;
		}
	}

	return IOK;
}

/*
 *
 */
int aritmetika(Tstack *stack, pTAC i) {
	Tvariable t, *val1, *val2;
	val1 = stackGetVal(stack, (int)i->argument1);
	val2 = stackGetVal(stack, (int)i->argument2);

	t.type = NUM_TYPE;
	if(val1->type != NUM_TYPE || val2->type != NUM_TYPE)
		return INTERPRET_ERROR;
	switch((int)i->operator) {
		case I_MUL:
			t.data.num = val1->data.num * val2->data.num;
			break;
		case I_SUB:
			t.data.num = val1->data.num - val2->data.num;
			break;
		case I_ADD:
			t.data.num = val1->data.num + val2->data.num;
			break;
		case I_DIV:
			if(val2->data.num == 0.0)
				return INTERPRET_ERROR;
			t.data.num = val1->data.num / val2->data.num;
			break;
		case I_MOD:
			t.data.num = (int)val1->data.num % (int)val2->data.num;
			break;
		case I_POW:
			t.data.num = pow(val1->data.num, val2->data.num);
			break;
	}
	if(stackSetVal(stack, &t, (int)i->result) != STACK_SUCCESS)
		return INTER_ERROR;
	return IOK;
}

/**
 * Vyhodnoti relaci dvou operatoru.
 * 
 * @param stack - zasobnik
 * @param i     - aktualni instrukce
 * @return      - CHYBA || OK
 */
int cmp(Tstack *stack, pTAC i) {
	Tvariable t, *val1, *val2;
	val1 = stackGetVal(stack, (int)i->argument1);
	val2 = stackGetVal(stack, (int)i->argument2);

	t.type = BOOL_TYPE;

	if((int)i->operator == I_EQU && (val1->type != val2->type)) {
		t.data.boolean = false;
		if(stackSetVal(stack, &t, (int)i->result) != STACK_SUCCESS)
			return INTER_ERROR;
		return IOK;
	}
	if((int)i->operator == I_EQU && (val1->type == BOOL_TYPE && val2->type == BOOL_TYPE)) {
		t.data.boolean = val1->data.boolean == val2->data.boolean;
		if(stackSetVal(stack, &t, (int)i->result) != STACK_SUCCESS)
			return INTER_ERROR;
		return IOK;
	}
	if((int)i->operator == I_EQU && (val1->type == NIL_TYPE && val2->type == NIL_TYPE)) {
		t.data.boolean = true;
		if(stackSetVal(stack, &t, (int)i->result) != STACK_SUCCESS)
			return INTER_ERROR;
		return IOK;
	}
  
  if((int)i->operator == I_NEQ && (val1->type != val2->type)) {
		t.data.boolean = true;
		if(stackSetVal(stack, &t, (int)i->result) != STACK_SUCCESS)
			return INTER_ERROR;
		return IOK;
	}
	if((int)i->operator == I_NEQ && (val1->type == BOOL_TYPE && val2->type == BOOL_TYPE)) {
		t.data.boolean = val1->data.boolean != val2->data.boolean;
		if(stackSetVal(stack, &t, (int)i->result) != STACK_SUCCESS)
			return INTER_ERROR;
		return IOK;
	}
	if((int)i->operator == I_NEQ && (val1->type == NIL_TYPE && val2->type == NIL_TYPE)) {
		t.data.boolean = false;
		if(stackSetVal(stack, &t, (int)i->result) != STACK_SUCCESS)
			return INTER_ERROR;
		return IOK;
	}

	double c1, c2;
	if(val1->type == STR_TYPE && val2->type == STR_TYPE) {
		c1 = strCmpString(&val1->data.str, &val2->data.str);
		c2 = 0;
	} else if(val1->type == NUM_TYPE && val2->type == NUM_TYPE) {
		c1 = val1->data.num;
		c2 = val2->data.num;
	} else {
		return INTERPRET_ERROR;
	}
	switch((int)i->operator) {
		case I_EQU:
			t.data.boolean = c1 == c2;
			break;
		case I_NEQ:
			t.data.boolean = c1 != c2;
			break;
		case I_LES:
			t.data.boolean = c1 < c2;
			break;
		case I_LSE:
			t.data.boolean = c1 <= c2;
			break;
		case I_GRE:
			t.data.boolean = c1 >= c2;
			break;
		case I_GRT:
			t.data.boolean = c1 > c2;
			break;
	}
	if(stackSetVal(stack, &t, (int)i->result) != STACK_SUCCESS)
		return INTER_ERROR;
	return IOK;
}
