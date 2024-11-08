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

bool consume(int code){
	if(tokens[iTk].code==code){
		consumed=&tokens[iTk++];
		return true;
		}
	return false;
	}


// bool consume(int code){
//       printf("consume(%s)",getCodeName(code));
//       if(tokens[iTk].code==code){
//             consumed=&tokens[iTk++];
//             printf(" => consumed\n");
//             return true;
//             }
//       printf(" => found %s\n",getCodeName(tokens[iTk].code));
//       return false;
//       }


bool baseType(){
	if(consume(TYPE_INT))
		return true;
	else if(consume(TYPE_REAL))
		return true;
	else if(consume(TYPE_STR))
		return true;
	else tkerr("invalid data type.");
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
bool funcParams(){
	int start = iTk;
	if(funcParam() == true){
		while (consume(COMMA)) {
			if (!funcParam()) {
				tkerr("invalid function parameter after ,");
				return false;
			}
		}
		return true;
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
							while (defVar()) {}
							if(block()){
								if(consume(END)){
									return true;
								}
							}
							else tkerr("function not entirely defined.");
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

	else if (consume(LPAR)){
		if (expr()){
			if (consume(RPAR)){
				return true;
			}
			else tkerr("missing ) after expression declaration.");
		}
		else tkerr("missing expression inside parenthesis.");
	}

	// ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
	else if (consume(ID))
	{
		if(consume(LPAR)){
			if(expr()){
				while (consume(COMMA)) {
					if (!expr()) {
						tkerr("invalid expression after , in function declaration.");
					}
				}
			}
		if(consume(RPAR)){
			return true;
		}
		else tkerr("missing ) after function declaration.");
		}
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
		while (consume(ADD) || consume(SUB)) {
			if (!exprMul()) {
				tkerr("invalid expression after add/sub operator.");
			}
		}
		return true;
	}
	iTk = start;
	return false;
}

//exprMul ::= exprPrefix ( ( MUL | DIV ) exprPrefix )*
bool exprMul(){
	int start = iTk;
	if(exprPrefix()){
		//printf("expression Prefix NOT OK.\n");
		while (consume(MUL) || consume(DIV)) {
			if (!exprPrefix()) {
				tkerr("invalid expression after multiplication/division operator.");
			}
		}
		return true;
	}
	iTk = start;
	return false;
}

//exprPrefix ::= ( SUB | NOT )? factor
bool exprPrefix(){
	int start = iTk;
	if(consume(SUB) || consume(NOT)){
		if(factor()){
			return true;
		}
	}

	iTk = start;
	return factor();
}

//exprComp ::= exprAdd ( ( LESS | EQUAL ) exprAdd )?
bool exprComp(){
	int start = iTk;
	if(exprAdd()){
		while (consume(LESS) || consume(EQUAL)) {
			if (!exprAdd()) {
				tkerr("invalid expression after equality operators.");
			}
		}
		return true;
	}
	iTk = start;
	return false;
}


//exprAssign ::= ( ID ASSIGN )? exprComp
bool exprAssign(){
	int start = iTk;
	if(consume(ID)){
		if (consume(ASSIGN)) {
			if (exprComp()) {
				return true;
			}
			else {
				tkerr("invalid assignment expression.");
				return false;
			}
		}
	}
	iTk = start;
	return exprComp();
}

//exprLogic ::= exprAssign ( ( AND | OR ) exprAssign )*
bool exprLogic(){
	int start = iTk;
		if(exprAssign()){
			while (consume(AND) || consume(OR)) {
			if (!exprAssign()) {
				tkerr("invalid statement after logic operator.");
			}
		}
		return true;
	}
	iTk = start;
	return false;
}

//expr ::= exprLogic
bool expr(){
	int start = iTk;
	if(exprLogic()){
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
	if (expr()) {
		if (consume(SEMICOLON)) {
			return true;
		}
		else tkerr("missing ; after expression");
	}

	if (consume(SEMICOLON)) {
		return true;
	}

	if (consume(IF)) {
		if (consume(LPAR)) {
			if (expr()) {
				if (consume(RPAR)) {
					if (block()) {
						if (consume(ELSE)) {
							if (block()) { }
							else tkerr("invalid statements after else.");
						}
						if (consume(END)) {
							return true;
						}
						else tkerr("missing end after function declaration.");
					}
					else tkerr("invalid declaration after if statement.");
				}
				else tkerr("missing ) after if declaration.");
			}
			else tkerr("invalid if condition.");
		}
		else tkerr("missing ( after if declaration.");
	}

	if (consume(RETURN)) {
		if (!expr()) {
			tkerr("invalid expression after return.");
		}
		if (consume(SEMICOLON)) {
			return true;
		}
		else tkerr("missing ; after return statement.");
	}

	if (consume(WHILE)) {
		if (consume(LPAR)) {
			if (expr()) {
				if (consume(RPAR)) {
					if (block()) {
						if (consume(END)) {
							return true;
						}
						else tkerr("while is never closed.");
					}
					else tkerr("invalid while statemens.");
				}
				else tkerr("missing ) after while statement.");
			}
			else tkerr("invalid condition in while.");
		}
		else tkerr("missing ( after while statement.");
	}

	iTk = start;
	return false;
}

// block ::= instr+
bool block(){
	int start = iTk;
	if(instr()){
		while(instr()){}
		return true;
	}	
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
		}else tkerr("syntax error.");
	return false;
	}

void parse(){
	iTk=0;
	program();
	}
