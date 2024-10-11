#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"
#include "utils.h"

Token tokens[MAX_TOKENS];
int nTokens;

int line=1;		// the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token *addTk(int code){
	if(nTokens==MAX_TOKENS)err("too many tokens");
	Token *tk=&tokens[nTokens];
	tk->code=code;
	tk->line=line;
	nTokens++;
	return tk;
	}

// copy in the dst buffer the string between [begin,end)
char *copyn(char *dst,const char *begin,const char *end){
	char *p=dst;
	if(end-begin>MAX_STR)err("string too long");
	while(begin!=end)*p++=*begin++;
	*p='\0';
	return dst;
	}


// TODO : Trebuie sa extraga toti atomii lexicali
void tokenize(const char *pch){ // pch = Pointer Current Character
	const char *start;
	Token *tk;
	char buf[MAX_STR+1];

	// Bucla infinita
	for(;;){
		switch(*pch){
			case ' ':case '\t':pch++;break;
			case '\r':		// handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
				if(pch[1]=='\n')pch++;
				// fallthrough to \n
			case '\n':
				line++;
				pch++;
				break;
			case '\0':addTk(FINISH);return;
			case ',':addTk(COMMA);pch++;break;
			case '=':
				if(pch[1]=='='){
					addTk(EQUAL);
					pch+=2;
					}else{
					addTk(ASSIGN);
					pch++;
					}
				break;
				case '+':addTk(ADD);pch++;break;
				case '-':addTk(SUB);pch++;break;
				case '*':addTk(MUL);pch++;break;
				case '/':addTk(DIV);pch++;break;
				case '&&':addTk(AND);pch++;break;
				case '||':addTk(OR);pch++;break;
				case '!=':addTk(NOTEQ);pch++;break;
				case '<':addTk(LESS);pch++;break;
				case '>':addTk(GREATER);pch++;break;
				case '>=':addTk(GREATERQ);pch++;break;

			default:

			// TOTO: 
				if(isalpha(*pch)||*pch=='_'){
					for(start=pch++;isalnum(*pch)||*pch=='_';pch++){}
					char *text=copyn(buf,start,pch);
					// Daca este cuvant cheie dam un cod
					// TODO: Adauga celelalte cuvinte cheie
					if(strcmp(text,"int")==0)addTk(TYPE_INT);
					else if(strcmp(text,"real")==0)addTk(TYPE_REAL);
					else if(strcmp(text,"str")==0)addTk(TYPE_STR);
					else if(strcmp(text,"if")==0)addTk(IF);
					else if(strcmp(text,"else")==0)addTk(ELSE);
					else if(strcmp(text,"while")==0)addTk(WHILE);
					else{
						tk=addTk(ID);
						strcpy(tk->text,text);
						}
					}
					// else if (isdigit(*pch)){

					//}
				else err("invalid char: %c (%d)",*pch,*pch);
			}
		}
	}

void showTokens(){
	for(int i=0;i<nTokens;i++){
		Token *tk=&tokens[i];
		printf("%d\n",tk->code);
		}
	}
