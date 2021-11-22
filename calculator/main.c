#include "calculator.h"

#include <stdio.h>
#include <stdlib.h>

#define STR_SIZE 256

void process()
{
    char *buffer = calloc(STR_SIZE + 1, sizeof(*buffer));
    scanf("%*s", STR_SIZE, buffer);
    Expression *expr = calculator_parse(buffer);
    char *result = calculator_result(expr);
    printf("%s\n", result);
    free(buffer);
    free(result);
}

int main()
{
    printf("Print q to quit or any else to call calculator\n");
    char c;
    while (scanf("%c", &c), c != 'q')
    {
        fflush(stdin);
        process();
    }
}
