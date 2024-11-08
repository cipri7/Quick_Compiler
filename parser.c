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

bool consume(int code);
bool baseType();
bool defVar();
bool funcParam();
bool funcParams();
bool defFunc();
bool factor();
bool exprAdd();
bool exprMul();
bool exprPrefix();
bool exprComp();
bool exprAssign();
bool exprLogic();
bool expr();
bool instr();
bool block();
bool program();
const char* getCodeName(enum Codes cod);

// bool consume(int code){
// 	if(tokens[iTk].code==code){
// 		consumed=&tokens[iTk++];
// 		return true;
// 		}
// 	return false;
// 	}


bool consume(int code){
      printf("consume(%s)",getCodeName(code));
      if(tokens[iTk].code==code){
            consumed=&tokens[iTk++];
            printf(" => consumed\n");
            return true;
            }
      printf(" => found %s\n",getCodeName(tokens[iTk].code));
      return false;
      }


bool baseType(){
	int start = iTk;
	if(consume(TYPE_INT))
		return true;
	else if(consume(TYPE_REAL))
		return true;
	else if(consume(TYPE_STR))
		return true;
	// necessary error?
	else tkerr("invalid data type.");

	iTk = start;
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
					else tkerr("missing ; after variable definition.");
				}
				else tkerr("invalid variable type.");
			}
			else tkerr("missing : after variable name.");

		}
		else tkerr("missing variable name.");

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
			else tkerr("invalid parameter type.");
		}
		else tkerr("missing : after parameter.");
	}
	iTk = start;
	return false;
}

// funcParams ::= funcParam ( COMMA funcParam )*
//TODO: eroare aici
bool funcParams(){
	int start = iTk;
	if(funcParam() == true){
		//printf("funcParam OK\n");
		for(;;){
			if(consume(COMMA)){
				if(funcParam() == true){
					//printf("funcParam OK\n");
					return true;
					break;
				}
				else
					tkerr("missing additional function argument.");
			}
			else tkerr("missing , after multiple function parameters.");
		}
	}
	else tkerr("invalid function parameters.");
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
				//printf("%d",tokens[iTk].code);
				if(consume(RPAR)){
					if(consume(COLON)){
						if(baseType()){
							for(;;){
								if(defVar()){}
								else
									break;
							}
							if(block()){
								if(consume(END)){
									return true;
								}
							}
							//else tkerr("error in  ");
						}
						else tkerr("invalid data type in function declaration.");
					}
					else tkerr("missing : in function declaration.");
				}
				else tkerr("missing ) after function parameters.");
			}
			else tkerr("missing ( before function parameters.");
		}
		else tkerr("missing function name.");
	}
	iTk = start;
	return false;
}

// bool instrWhile(){
// 	int start = iTk;
// 	if (consume(WHILE)){
// 		if (consume(LPAR)){
// 			if (expr()){
// 				if (consume(RPAR)){
// 					if (instr())
// 						return true;
// 				}
// 			}
// 		}
// 	}
// 	iTk = start;
// 	return false;
// }

/*
factor ::= INT
| REAL
| STR
| LPAR expr RPAR
| ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
*/
bool factor(){
	int start = iTk;
	if (consume(INT)){
		return true;
	}
	if (consume(REAL)){
		return true;
	}
	if (consume(STR)){
		return true;
	}

	if (consume(LPAR)){
		if (expr()){
			if (consume(RPAR)){
				return true;
			}
			else tkerr("missing ) after expression declaration.");
		}
		else tkerr("missing expression inside parenthesis.");
	}
	//else tkerr("missing ( before expression declaration.");//error

	// ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
	if (consume(ID))
	{
		//printf("%d",tokens[iTk].code);
		if(consume(LPAR)){}
			if(expr()){
				for(;;){
					if(consume(COMMA)){
						if(expr()){}
						else
						break;
					}
					else
						break;
				}
			}
		if(consume(RPAR)){}
		return true;
	}
	iTk = start;
	return false;
}


//exprAdd ::= exprMul ( ( ADD | SUB ) exprMul )*
bool exprAdd(){
	int start = iTk;
	if(exprMul()){
		//printf("expression MUL NOT OK.\n");
		for(;;){
			if(consume(ADD) || consume(SUB)){
				if(exprMul()){
					return true;
				}
				else
					break;
			}
			else
				break;
		}
	}


	iTk = start;
	return false;
}

//exprMul ::= exprPrefix ( ( MUL | DIV ) exprPrefix )*
bool exprMul(){
	int start = iTk;
	if(exprPrefix()){
		//printf("expression Prefix NOT OK.\n");
		for(;;){
			if(consume(MUL) || consume(DIV)){
				if(exprPrefix()){
					return true;
				}
				else
					break;
			}
			else
				break;
		}
	}

	iTk = start;
	return false;
}

//exprPrefix ::= ( SUB | NOT )? factor
bool exprPrefix(){
	int start = iTk;
	if(consume(SUB)){}
	if(consume(NOT)){}
	if(factor()){
		//printf("FACTOR NOT OK.\n");
		return true;
	}

	iTk = start;
	return false;
}

//exprComp ::= exprAdd ( ( LESS | EQUAL ) exprAdd )?
bool exprComp(){
	int start = iTk;
	if(exprAdd()){
		printf("expression ADD OK .\n");
		if(consume(LESS) || consume(EQUAL)){
			if(exprAdd()){}
			return true;
		}


	}
	else 
		printf("expression ADD NOT OK .\n");

	iTk = start;
	return false;
}


//exprAssign ::= ( ID ASSIGN )? exprComp
bool exprAssign(){
	int start = iTk;
	if(consume(ID)){
		printf("expression Assign OK ID.\n");
	}
	if(consume(ASSIGN)){
		printf("expression Assign OK ASSIGN.\n");
	}
	if(exprComp()){
		printf("expression COMP  OK.\n");
		return true;
	}
	iTk = start;
	return false;
}

//exprLogic ::= exprAssign ( ( AND | OR ) exprAssign )*
bool exprLogic(){
	int start = iTk;
		if(exprAssign()){
			//printf("exprAssign NOT OK!!\n");
			for(;;){
				if(consume(AND) || consume(OR)){
					if(exprAssign()){
						return true;
					}
					else
						break;

				}
			}
		}
		iTk = start;
		return false;


}

//expr ::= exprLogic
bool expr(){
	int start = iTk;
	if(exprLogic()){
		//printf("expression Logic error.\n");
		return true;
	}
	iTk = start;
	return false;
	
}


/*
instr ::= expr? SEMICOLON
| IF LPAR expr RPAR block ( ELSE block )? END
| RETURN expr SEMICOLON
| WHILE LPAR expr RPAR block END
*/
bool instr(){
	int start = iTk;
	if(expr()){
		//printf("expression error.\n");

	}
	if(consume(SEMICOLON)){
		return true;
	}
	//IF LPAR expr RPAR block ( ELSE block )? END
	else if(consume(IF)){
		printf("IF GASIT\n");
		if(consume(LPAR)){
			printf("LPAR GASIT\n");
			if(expr()){
				printf("expr GASIT\n");
				if(consume(RPAR)){
					printf("RPAR GASIT\n");
					if(block()){
						printf("BLOCK OK\n");
						if(consume(ELSE)){}
							//printf("ELSE GASIT\n");
							if(block()){}
						
						if(consume(END)){
							return true;
						}
					}
					else tkerr("missing expression inside IF.");
				}
			}
		}
	}
	else if(consume(RETURN)){
		if(expr()){
			if(consume(SEMICOLON)){
				return true;
			}
		}
	}
	else if(consume(WHILE)){
		if(consume(LPAR)){
			if(expr()){
				if(consume(RPAR)){
					if(block()){
						if(consume(END)){
							return true;
						}
					}
				}
			}
		}
	}
	iTk = start;
	return false;
}

// block ::= instr+
bool block(){
	int start = iTk;
	if(instr()){
		while(instr()){}
		printf("INSTR OK\n");
		return true;
	}
	else printf("INSTR  NOT OK\n");
	
	iTk = start;
	return false;
}

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
