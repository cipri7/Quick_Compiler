#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lexer.h"

int iTk;	// the iterator in tokens
Token *consumed;	// the last consumed token

// same as err, but also prints the line of the current token
_Noreturn void tkerr(const char *fmt,...){
	fprintf(stderr,"error in line %d: ",tokens[iTk].line);
	va_list va;
	va_start(va,fmt);
	vfprintf(stderr,fmt,va);
	va_end(va);
	fprintf(stderr,"\n");
	exit(EXIT_FAILURE);
	}

bool consume(int code){
	if(tokens[iTk].code==code){
		consumed=&tokens[iTk++];
		return true;
		}
	return false;
	}

bool baseType(){
	if(consume(TYPE_INT))
		return true;
	if(consume(TYPE_REAL))
		return true;
	if(consume(TYPE_STR))
		return true;

	return false;
}

// defVar ::= VAR ID COLON baseType SEMICOLON
bool defVar(){
	int start = iTk;
	if(consume(VAR)){
		if(consume(ID)){
			if(consume(COLON)){
				if(baseType()) {
					if(consume(SEMICOLON)){
						return true;				
					}
				}
			}
		}

	}
	iTk = start;
	return false;
}

// funcParam ::= ID COLON baseType
bool funcParam(){
	int start = iTk;
	if(consume(ID)){
		if(consume(COLON)){
			if(baseType())
				return true;
		}
	}
	iTk = start;
	return false;
}

// funcParams ::= funcParam ( COMMA funcParam )*
bool funcParams(){
	int start = iTk;
	if(funcParam()){
		for(;;){
			if(consume(COMMA)){
				if(funcParam){}
				else
					break;
			}
		}
	}
	iTk = start;
	return false;
}

// defFunc ::= FUNCTION ID LPAR funcParams? RPAR COLON baseType defVar* block END
bool defFunc(){
	int start = iTk;
	if(consume(FUNCTION)){
		if(consume(ID)){
			if(consume(LPAR)){
				if(funcParams()){}
				if(consume(RPAR)){
					if(consume(COLON)){
						if(baseType()){
							if(defVar()){}
							if(block()){
								if(consume(END)){
									return true;
								}
							}
						}
					}
				}
			}
		}
	}
	iTk = start;
	return false;
}

bool instrWhile(){
	int start = iTk;
	if (consume(WHILE)){
		if (consume(LPAR)){
			if (expr()){
				if (consume(RPAR)){
					if (instr())
						return true;
				}
			}
		}
	}
	iTk = start;
	return false;
}

/*
factor ::= INT
| REAL
| STR
| LPAR expr RPAR
| ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
*/
// bool factor(){
// int start = iTk;
// if(consume(ID)){
// return true;
// }
// if(consume(LPAR)){
// if(expr()){
// if(consume(RPAR)){
// return true;
// }
// }
// iTk = start;
// }
// if(consume(NR)){
// return true;
// }
// return false;
// }

// program ::= ( defVar | defFunc | block )* FINISH
bool program(){
	for(;;){
		if(defVar()){}
		else if(defFunc()){}
		else if(block()){}
		else break;
		}
	if(consume(FINISH)){
		return true;
		}else tkerr("syntax error");
	return false;
	}

void parse(){
	iTk=0;
	program();
	}
