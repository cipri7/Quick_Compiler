#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "lexer.h"
#include "ad.h"
#include "at.h"
#include "gen.h"

int iTk;	// the iterator in tokens
Token* consumed;	// the last consumed token

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

bool consume(int code) {
	//printf("consume(%s)", tkCodeName(code));
	if (tokens[iTk].code == code) {
		consumed = &tokens[iTk++];
		//printf(" => consumed\n");
		return true;
	}
	//printf(" => found %s\n", tkCodeName(tokens[iTk].code));
	return false;
}


// baseType ::= TYPE_INT | TYPE_REAL | TYPE_STR
bool baseType() {
	if (consume(TYPE_INT)) {

		ret.type = TYPE_INT;

		return true;
	}
	else if (consume(TYPE_REAL)) {

		ret.type = TYPE_REAL;

		return true;
	}
	else if (consume(TYPE_STR)) {

		ret.type = TYPE_STR;

		return true;
	}
	else tkerr("invalid data type.");
	return false;
}

bool block();
bool defVar();
bool expr();

/*factor ::= INT
			| REAL
			| STR
			| LPAR expr RPAR
			| ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?*/
bool factor() {
	int start = iTk;
	if (consume(INT)) {

		setRet(TYPE_INT, false);
		Text_write(crtCode,"%d",consumed->Constante.i);

		return true;
	}
	else if (consume(REAL)) {

		setRet(TYPE_REAL, false);
		Text_write(crtCode,"%g",consumed->Constante.r);
		return true;
	}
	else if (consume(STR)) {

		setRet(TYPE_STR, false);
		Text_write(crtCode,"\"%s\"",consumed->Constante.text);

		return true;
	}
	else if (consume(LPAR)) {
		Text_write(crtCode,"(");
		if (expr()) {
			if (consume(RPAR)) {
				Text_write(crtCode,")");
				return true;
			}
			else tkerr("missing ) after expression declaration.");
		}
		else tkerr("missing expression inside parenthesis.");
	}
	else if (consume(ID)) {

		Symbol* s = searchSymbol(consumed->Constante.text);
		if (!s)tkerr("undefined symbol: %s", consumed->Constante.text);
		Text_write(crtCode,"%s",s->name);

		if (consume(LPAR)) {

			if (s->kind != KIND_FN)tkerr("%s cannot be called, because it is not a function", s->name);
			Symbol* argDef = s->args;
			Text_write(crtCode,"(");
			if (expr()) {

				if (!argDef)tkerr("the function %s is called with too many arguments", s->name);
				if (argDef->type != ret.type)tkerr("the argument type at function %s call is different from the one given at its definition",s->name);
					argDef = argDef->next;

				while (consume(COMMA)) {
					Text_write(crtCode,",");
					if (!expr()) {

						tkerr("invalid expression after , in function declaration.");
					}
					else {
						if (!argDef)tkerr("the function %s is called with too many arguments", s->name);
						if (argDef->type != ret.type)tkerr("the argument type at function %s call is different from the one given at its definition", s->name);
						argDef = argDef->next;
					}
				}
			}
			if (consume(RPAR)) {

				if (argDef)tkerr("the function %s is called with too few arguments", s->name);
				setRet(s->type, false);
				Text_write(crtCode,")");
				return true;
			}
			else tkerr("missing ) after function declaration.");

		}
		else {

			if (s->kind == KIND_FN)tkerr("the function %s can only be called", s->name);
			setRet(s->type, true);

			return true;
		}
	}
	iTk = start;
	return false;
}

// exprPrefix ::= ( SUB | NOT )? factor
bool exprPrefix() {
	int start = iTk;
	if (consume(SUB)) {
		Text_write(crtCode,"-");
		if (factor()) {

			if (ret.type == TYPE_STR)tkerr("the expression of unary - must be of type int or real");
			ret.lval = false;

			return true;
		}
	}
	else if (consume(NOT)) {
		Text_write(crtCode,"!");
		if (factor()) {

			if (ret.type == TYPE_STR)tkerr("the expression of ! must be of type int or real");
			setRet(TYPE_INT, false);

			return true;
		}
	}
	iTk = start;
	return factor();
}

// exprMul ::= exprPrefix ( ( MUL | DIV ) exprPrefix )*
bool exprMul() {
	int start = iTk;
	if (exprPrefix()) {
		while (consume(MUL) || consume(DIV)) {
			if(consumed->code == MUL) Text_write(crtCode,"*");
			else if(consumed->code == DIV) Text_write(crtCode,"/");
			Ret leftType = ret;
			if (leftType.type == TYPE_STR)tkerr("the operands of * or / cannot be of type str");

			if (!exprPrefix()) {

				if (leftType.type != ret.type)tkerr("different types for the operands of * or /");
				ret.lval = false;

				tkerr("invalid expression after multiplication/division operator.");
			}
		}
		return true;
	}
	iTk = start;
	return false;
}

// exprAdd ::= exprMul ( ( ADD | SUB ) exprMul )*
bool exprAdd() {
	int start = iTk;
	if (exprMul()) {
		while (consume(ADD) || consume(SUB)) {
			if(consumed->code == ADD) Text_write(crtCode,"+");
			else if(consumed->code == SUB) Text_write(crtCode,"-");

			Ret leftType = ret;
			if (leftType.type == TYPE_STR)tkerr("the operands of + or - cannot be of type str");

			if (!exprMul()) {

				if (leftType.type != ret.type)tkerr("different types for the operands of + or -");
				ret.lval = false;

				tkerr("invalid expression after + - operator.");
			}
		}
		return true;
	}
	iTk = start;
	return false;
}

// exprComp ::= exprAdd ( ( LESS | EQUAL ) exprAdd )?
bool exprComp() {
	int start = iTk;
	if (exprAdd()) {
		while (consume(LESS) || consume(EQUAL)) {
			if(consumed->code == LESS) Text_write(crtCode,"<");
			else if(consumed->code == EQUAL) Text_write(crtCode,"==");
			Ret leftType = ret;

			if (!exprAdd()) {

				if (leftType.type != ret.type)tkerr("different types for the operands of < or ==");
				setRet(TYPE_INT, false); // the result of comparation is int 0 or 1

				tkerr("invalid expression after equality operators.");
			}
		}
		return true;
	}
	iTk = start;
	return false;
}

// exprAssign ::= ( ID ASSIGN )? exprComp
bool exprAssign() {
	int start = iTk;
	if (consume(ID)) {

		const char* name = consumed->Constante.text;

		if (consume(ASSIGN)) {
			Text_write(crtCode,"%s=",name);
			if (exprComp()) {

				Symbol* s = searchSymbol(name);
				if (!s)tkerr("undefined symbol: %s", name);
				if (s->kind == KIND_FN)tkerr("a function (%s) cannot be used as a destination for assignment ", name);
				if (s->type != ret.type)tkerr("the source and destination for assignment must have the same type");
				ret.lval = false;

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

// exprLogic ::= exprAssign ( ( AND | OR ) exprAssign )*
bool exprLogic() {
	int start = iTk;
	if (exprAssign()) {
		while (consume(AND) || consume(OR)) {
			if(consumed->code == AND) Text_write(crtCode,"&&");
			else if (consumed->code == OR) Text_write(crtCode,"||");

			Ret leftType = ret;
			if (leftType.type == TYPE_STR)tkerr("the left operand of && or || cannot be of type str");

			if (!exprAssign()) {

				if (ret.type == TYPE_STR)tkerr("the right operand of && or || cannot be of type str");
				setRet(TYPE_INT, false);

				tkerr("invalid statement after logic operator.");
			}
		}
		return true;
	}
	iTk = start;
	return false;
}

// expr ::= exprLogic
bool expr() {
	int start = iTk;
	if (exprLogic()) {
		return true;
	}
	iTk = start;
	return false;

}

/*instr ::= expr? SEMICOLON
			| IF LPAR expr RPAR block ( ELSE block )? END
			| RETURN expr SEMICOLON
			| WHILE LPAR expr RPAR block END*/
bool instr() {
	int start = iTk;
	if (expr()) {
		if (consume(SEMICOLON)) {
			Text_write(crtCode,";\n");
			return true;
		}
		else tkerr("missing ; after expression");
	}

	// if (consume(SEMICOLON)) {
	// 	return true;
	// }

	if (consume(IF)) {
		if (consume(LPAR)) {
			Text_write(crtCode,"if(");
			if (expr()) {

				if (ret.type == TYPE_STR)tkerr("the if condition must have type int or real");

				if (consume(RPAR)) {
					Text_write(crtCode,"){\n");
					if (block()) {
						Text_write(crtCode,"}\n");
						if (consume(ELSE)) {
							Text_write(crtCode,"else{\n");
							if (block()) {
								Text_write(crtCode,"}\n");
							}
							
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
		Text_write(crtCode,"return ");
		if (!expr()) {

			if (!crtFn)tkerr("return can be used only in a function");
			if (ret.type != crtFn->type)tkerr("the return type must be the same as the function return type");


			tkerr("invalid expression after return.");
		}
		if (consume(SEMICOLON)) {
			Text_write(crtCode,";\n");
			return true;
		}
		else tkerr("missing ; after return statement.");
	}

	if (consume(WHILE)) {
		Text_write(crtCode,"while(");
		if (consume(LPAR)) {
			if (expr()) {

				if (ret.type == TYPE_STR)tkerr("the while condition must have type int or real");

				if (consume(RPAR)) {
					Text_write(crtCode,"){\n");
					if (block()) {
						if (consume(END)) {
							Text_write(crtCode,"}\n");
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
		else tkerr("issing ( after while statement.");
	}

	iTk = start;
	return false;
}

// block ::= instr+
bool block() {
	int start = iTk;
	if (instr()) {
		while (instr()) {}
		return true;
	}
	iTk = start;
	return false;
}

// funcParam ::= ID COLON baseType
bool funcParam() {
	int start = iTk;
	if (consume(ID)) {

		const char* name = consumed->Constante.text;
		Symbol* s = searchInCurrentDomain(name);
		if (s)tkerr("symbol redefinition: %s", name);
		s = addSymbol(name, KIND_ARG);
		Symbol* sFnParam = addFnArg(crtFn, name);

		if (consume(COLON)) {
			if (baseType()) {

				s->type = ret.type;
				sFnParam->type = ret.type;

				Text_write(&tFnHeader,"%s %s",cType(ret.type),name);

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
bool funcParams() {
	int start = iTk;
	if (funcParam()) {
		while (consume(COMMA)) {
			Text_write(&tFnHeader,",");
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
bool defFunc() {
	int start = iTk;
	if (consume(FUNCTION)) {
		if (consume(ID)) {

			const char* name = consumed->Constante.text;
			Symbol* s = searchInCurrentDomain(name);
			if (s)tkerr("symbol redefinition: %s", name);
			crtFn = addSymbol(name, KIND_FN);
			crtFn->args = NULL;
			addDomain();

			crtCode=&tFunctions;
			crtVar=&tFunctions;
			Text_clear(&tFnHeader);
			Text_write(&tFnHeader,"%s(",name);

			if (consume(LPAR)) {
				if (funcParams()) {}
				if (consume(RPAR)) {
					if (consume(COLON)) {
						if (baseType()) {

							crtFn->type = ret.type;
							Text_write(&tFunctions,"\n%s %s){\n",cType(ret.type),tFnHeader.buf);

							while (defVar()) {}
							if (block()) {
								if (consume(END)) {

									delDomain();
									crtFn = NULL;

									Text_write(&tFunctions,"}\n");
									crtCode=&tMain;
									crtVar=&tBegin;

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

// defVar ::= VAR ID COLON baseType SEMICOLON
bool defVar() {
	int start = iTk;
	if (consume(VAR)) {
		if (consume(ID)) {

			const char* name = consumed->Constante.text;
			Symbol* s = searchInCurrentDomain(name);
			if (s)tkerr("symbol redefinition: %s", name);
			s = addSymbol(name, KIND_VAR);
			s->local = crtFn != NULL;

			if (consume(COLON)) {
				if (baseType()) {

					s->type = ret.type;

					if (consume(SEMICOLON)) {
						Text_write(crtVar,"%s %s;\n",cType(ret.type),name);
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

// program ::= ( defVar | defFunc | block )* FINISH
bool program() {
	addDomain(); // creates the global domain

	addPredefinedFns(); // it will be inserted after the code for domain analysis

	// the code for code generation will be added after the code of the other modules
	crtCode=&tMain;
	crtVar=&tBegin;
	Text_write(&tBegin,"#include \"quick.h\"\n\n");
	Text_write(&tMain,"\nint main(){\n");

	for (;;) {
		if (defVar()) {}
		else if (defFunc()) {}
		else if (block()) {}
		else break;
	}
	if (consume(FINISH)) {

		delDomain(); // deletes the global domain

		Text_write(&tMain,"return 0;\n}\n");
		FILE *fis=fopen("1.c","w");
		if(!fis){
		printf("cannot write to file 1.c\n");
		exit(EXIT_FAILURE);
		}
		fwrite(tBegin.buf,sizeof(char),tBegin.n,fis);
		fwrite(tFunctions.buf,sizeof(char),tFunctions.n,fis);
		fwrite(tMain.buf,sizeof(char),tMain.n,fis);
		fclose(fis);

		return true;
	}


	else tkerr("syntax error.");


	return false;
}

void parse() {
	iTk = 0;
	program();
}
