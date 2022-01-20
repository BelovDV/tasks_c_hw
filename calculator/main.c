#include "calculator.h"

#include <stdio.h>
#include <stdlib.h>

#define STR_SIZE 16
#define STR_S_SIZE "16"

char read_char() {
    char c;
    while (1) {
        scanf("%c", &c);
        if (c != ' ' && c != '\n' && c != '\t') return c;
    }
}

void flush() {
    char c;
    while (1) {
        scanf("%c", &c);
        if (c == '\n') return;
    }
}

void process()
{
    char *buffer = calloc(STR_SIZE + 1, sizeof(*buffer));
    scanf("%" STR_S_SIZE "s", buffer);
    Expression *expr = calculator_parse(buffer);
    if (!expr)
        printf("Error\n");
    else {
        do {
            char *result = calculator_result(expr);
            printf("%s\n", result);
            free(result);
        } while (calculator_simplify_step(expr, 1));
    }
    calclulator_destroy(expr);
    free(buffer);
}

int main()
{
    printf("Print q to quit or any else to call calculator\n");
    while (read_char() != 'q')
    {
        flush();
        process();
        printf("Print q to quit or any else to call calculator\n");
    }
}
