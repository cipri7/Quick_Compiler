#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"
#include "utils.h"

const char* Codes_String[] = {
		"ID"
		// keywords
		,"TYPE_INT", "TYPE_REAL", "TYPE_STR",
		"VAR", "FUNCTION",
		"IF", "ELSE", "WHILE",
		"END", "RETURN",
		// delimiters
		"COMMA","FINISH", "COLON", "SEMICOLON",
		// operators
		"ASSIGN","EQUAL",
		"ADD", "SUB", "MUL", "DIV",
		"LESS", "GREATER", "GREATERQ","NOTEQ",
		//logic operators
		"AND", "OR", "NOT",  
		//symbols
		"LPAR", "RPAR",
		"SPACE", "COMMENT",

		//constants
		"INT", "REAL", "STR"
};

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

	const char* getCodeName(enum Codes cod) {
    return Codes_String[cod];  // Return the corresponding string
}

// Function to check if a string is a valid integer
int isInteger(const char* pch) {
	int number = *pch;
    printf("number1 %d", number);
    // Check if the rest of the characters are digits
    while (*pch) {
		pch++;
        if (isdigit(*pch)) {
            // If the character is a digit, add it to the current number
            number = number * 10 + (*pch - '0');  // Build multi-digit number
			printf("number2 %d", number);
        }
		else break;
    } 
    return number;  // All characters are digits
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
			case ';':addTk(SEMICOLON);pch++;break;
			case ':':addTk(COLON);pch++;break;
			case '(':addTk(LPAR);pch++;break;
			case ')':addTk(RPAR);pch++;break;

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
				case '&':
					if(pch[1]=='&'){
						addTk(AND);
						pch+=2;
						}else{
						err("invalid char: %c (%d)",*pch,*pch);
					}
				break;
				case '|':
				if(pch[1]=='|'){
						addTk(OR);
						pch+=2;
						}else{
						err("invalid char: %c (%d)",*pch,*pch);
					}
				break;
				case '!':
				if(pch[1]=='='){
					addTk(NOTEQ);
					pch+=2;
					}else{
					addTk(NOT);
					pch++;
					}
				break;
				case '<':addTk(LESS);pch++;break;
				case '>':
				if(pch[1]=='='){
					addTk(GREATERQ);
					pch+=2;
					}else{
					addTk(GREATER);
					pch++;
					}
				break;
				case '#':
					for(start=pch++;isalnum(*pch)||*pch=='_' || *pch == ' ';pch++){}
				break;

				case '\"':
				pch++;
					for(start=pch++;((isalnum(*pch) || ispunct(*pch)) && *pch != '\"');pch++){}
					pch++;
					char *text = copyn(buf,start,pch-1);
					tk = addTk(STR);
					strcpy(tk->Constante.text, text);
				break;

			default:

			// TODO: 
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
					else if(strcmp(text,"function")==0)addTk(FUNCTION);
					else if(strcmp(text,"var")==0)addTk(VAR);
					else if(strcmp(text,"return")==0)addTk(RETURN);
					else if(strcmp(text,"end")==0)addTk(END);
					else{
						tk = addTk(ID);
						strcpy(tk->Constante.text,text);
						}
					}
					else if (isdigit(*pch)){
						for(start=pch++;isdigit(*pch) && *pch != '.';pch++){}
						
						char *text=copyn(buf,start,pch);
						int num = atoi(text);
						if(*pch == '.'){
							for(start=pch++;isdigit(*pch);pch++){}
							char *text=copyn(buf,start,pch);
							double real = atof(text);
							real += num;
							tk = addTk(REAL);
							tk->Constante.r=real;
							break;
						}
						tk = addTk(INT);
						tk->Constante.i=num;
					
					}
					
					// else if (*pch == "\""){
					// 	for(start=pch++;isalnum(*pch);pch++){}
					// 	char *text=copyn(buf,start,pch);
					// }
					
				else err("invalid char: %c (%d)",*pch,*pch);
			}
		}
	}

void showTokens(){
	for(int i=0;i<nTokens;i++){
		Token *tk=&tokens[i];
		// TODO: Show code Value
		printf("%d %s",tk->line,getCodeName(tk->code));

		if(strcmp(getCodeName(tk->code), "ID")==0)
			printf(":%s",tk->Constante.text);

		else if (strcmp(getCodeName(tk->code), "INT")==0)
			printf(":%d",tk->Constante.i);

		else if (strcmp(getCodeName(tk->code), "REAL")==0)
			printf(":%f",tk->Constante.r);
		
		else if (strcmp(getCodeName(tk->code), "STR")==0)
			printf(":%s",tk->Constante.text);
		

		printf("\n");
		}
	}
