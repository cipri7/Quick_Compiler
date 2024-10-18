#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "lexer.h"

int main()
{
    char *buffer = loadFile("test/1.q");
    //puts(buffer);
    tokenize(buffer);
    showTokens();
    free(buffer);


    return 0;
}