#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "lexer.h"
#include "parser.h"
#include "ad.h"

int main()
{
    char *buffer = loadFile("test/1.q");
    //puts(buffer);
    tokenize(buffer);
    //showTokens();
    parse();
    free(buffer);


    return 0;
}