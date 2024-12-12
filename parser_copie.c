#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lexer.h"
#include "ad.h"

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
	if(consume(TYPE_INT)){
		ret.type=TYPE_INT;
		return true;
	}
	else if(consume(TYPE_REAL)){
		ret.type=TYPE_REAL;
		return true;
	}
	else if(consume(TYPE_STR)){
		ret.type=TYPE_STR;
		return true;
	}
	else tkerr("invalid data type.");
	return false;
}

// defVar ::= VAR ID COLON baseType SEMICOLON
bool defVar(){
	int start = iTk;
	if(consume(VAR)){
		if(consume(ID)){
			const char *name=consumed->Constante.text;
			Symbol *s=searchInCurrentDomain(name);
			if(s)tkerr("symbol redefinition: %s",name);
			s=addSymbol(name,KIND_VAR);
			s->local=crtFn!=NULL;
			if(consume(COLON)){
				if(baseType()) {
					s->type=ret.type;
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
		const char *name=consumed->Constante.text;
		Symbol *s=searchInCurrentDomain(name);
		if(s)tkerr("symbol redefinition: %s",name);
		s=addSymbol(name,KIND_ARG);
		Symbol *sFnParam=addFnArg(crtFn,name);
		if(consume(COLON)){
			if(baseType()){
				s->type=ret.type;
				sFnParam->type=ret.type;
				return true;
			}
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
			const char *name=consumed->Constante.text;
			Symbol *s=searchInCurrentDomain(name);
			if(s)tkerr("symbol redefinition: %s",name);
			crtFn=addSymbol(name,KIND_FN);
			crtFn->args=NULL;
			addDomain();

			if(consume(LPAR)){
				if(funcParams()){}
				if(consume(RPAR)){
					if(consume(COLON)){
						if(baseType()){
							crtFn->type=ret.type;
							while (defVar()) {}
							if(block()){
								if(consume(END)){
									delDomain();
									crtFn=NULL;
									return true;
								}
								else tkerr("missing END after function.");
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

//modificat: ID ( LPAR ( expr ( COMMA expr )* )? RPAR | epsilon )
bool factor(){
	int start = iTk;
	if (consume(INT)){
		//setRet(TYPE_INT,false);
		return true;
	}
	if (consume(REAL)){
		//setRet(TYPE_REAL,false);
		return true;
	}
	if (consume(STR)){
		//setRet(TYPE_STR,false);
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

	//modificat: ID ( LPAR ( expr ( COMMA expr )* )? RPAR | epsilon )
	if (consume(ID))
	{
		// Symbol *s=searchSymbol(consumed->Constante.text);
		// if(!s)tkerr("undefined symbol: %s",consumed->Constante.text);
		if(consume(LPAR)){
			// if(s->kind!=KIND_FN)tkerr("%s cannot be called, because it is not a function",s->name);
			// Symbol *argDef=s->args;
			if(expr()){
				// if(!argDef)tkerr("the function %s is called with too many arguments",s->name);
				// if(argDef->type!=ret.type)tkerr("the argument type at function %s call is different from the one given at its definition",s->name);
				// argDef=argDef->next;


				// if(!argDef)tkerr("the function %s is called with too many arguments",s->name);
				// if(argDef->type!=ret.type)tkerr("the argument type at function %s call is different
				// from the one given at its definition",s->name);
				// argDef=argDef->next;

//modificat: ID ( LPAR ( expr ( COMMA expr )* )? RPAR | epsilon )

				for(;;){
					if(consume(COMMA)){}
					else if(expr()){}
					else break;
				}

			}
			else tkerr("invalid expression after , in function declaration.");
		if(consume(RPAR)){
			// if(argDef)tkerr("the function %s is called with too few arguments",s->name);
			// setRet(s->type,false);
			return true;
		}
		else tkerr("missing ) after function declaration.");
		}
		// if(s->kind==KIND_FN)tkerr("the function %s can only be called",s->name);
		// setRet(s->type,true);
		
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
			if(consume(AND) || consume(SUB)){}
			else if(exprMul()){}
			else break;
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

		for(;;){
			if(consume(MUL) || consume(DIV)){}
			else if(exprPrefix()){}
			else break;
		}

		return true;
		}
	iTk = start;
	return false;
}

//exprPrefix ::= ( SUB | NOT )? factor
//modificat: SUB factor | NOT factor | factor
bool exprPrefix(){
	int start = iTk;
	if(consume(SUB)){
		if(factor()){
			// if(ret.type==TYPE_STR)tkerr("the expression of unary - must be of type int or real");
			// ret.lval=false;
			return true;
		}
		// if(ret.type==TYPE_STR)tkerr("the expression of ! must be of type int or real");
		// setRet(TYPE_INT,false);
	}

	if(consume(NOT)){
		if(factor()){
			return true;
		}
	}

	if(factor()){
		return true;
	}

	iTk = start;
	return factor();
}

//exprComp ::= exprAdd ( ( LESS | EQUAL ) exprAdd )?
bool exprComp(){
	int start = iTk;
	if(exprAdd()){
		while (consume(LESS) || consume(EQUAL)) {
			//Ret leftType=ret;
			if (!exprAdd()) {
				tkerr("invalid expression after equality operators.");
			}
		}
		// if(leftType.type!=ret.type)tkerr("different types for the operands of < or ==");
		// setRet(TYPE_INT,false); // the result of comparation is int 0 or 1
		return true;
	}
	iTk = start;
	return false;
}


//exprAssign ::= ( ID ASSIGN )? exprCom

//modificat: ID ASSIGN exprComp | exprComp
bool exprAssign(){
	int start = iTk;
	if(consume(ID)){
		const char *name=consumed->Constante.text;
		if (consume(ASSIGN)) {
			if (exprComp()) {
				// Symbol *s=searchSymbol(name);
				// if(!s)tkerr("undefined symbol: %s",name);
				// if(s->kind==KIND_FN)tkerr("a function (%s) cannot be used as a destination for assignment ",name);
				// if(s->type!=ret.type)tkerr("the source and destination for assignment must have the same type");
				// ret.lval=false;
				return true;
			}
			else {
				tkerr("invalid assignment expression.");
				return false;
			}
		}
	}
	if (exprComp()){
		return true;
	}
	iTk = start;
	return exprComp();
}

//exprLogic ::= exprAssign ( ( AND | OR ) exprAssign )*
bool exprLogic(){
	int start = iTk;
		if(exprAssign()){

			for(;;){
				if(consume(AND) || consume(OR)){}
				else if (exprAssign()){}
				else break;
			}
		// 	while (consume(AND) || consume(OR)) {
		// 	if (!exprAssign()) {
		// 		tkerr("invalid statement after logic operator.");
		// 	}

		// }
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
				//if(ret.type==TYPE_STR)tkerr("the if condition must have type int or real");
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
		//if(!crtFn)tkerr("return can be used only in a function");
		//if(ret.type!=crtFn->type)tkerr("the return type must be the same as the function return type");
		if (consume(SEMICOLON)) {
			return true;
		}
		else tkerr("missing ; after return statement.");
	}

	if (consume(WHILE)) {
		if (consume(LPAR)) {
			if (expr()) {
				//if(ret.type==TYPE_STR)tkerr("the while condition must have type int or real");
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
	addDomain(); // creates the global domain
	//addPredefinedFns(); // it will be inserted after the code for domain analysis
	for(;;){
		if(defVar()){}
		else if(defFunc()){}
		else if(block()){}
		else break;
		}
	if(consume(FINISH)){
		delDomain(); // deletes the global domain
		return true;
		}else tkerr("syntax error.");
	return false;
	}

void parse(){
	iTk=0;
	program();
	}