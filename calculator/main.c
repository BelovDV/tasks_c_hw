#include "calculator.h"

#include <stdio.h>
#include <stdlib.h>

#define STR_SIZE 32
#define STR_S_SIZE "32"

#define STORAGE_S 10

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
        if (c == '\n') {
            // fflush(stdin); // ?
            return;
        }
    }
}

void help() {
    printf(
        "Operations:\n"
        "  quit (q)         - close application;\n"
        "  in (i)           - read new equation;\n"
        "  reduce (r)       - simplify current equation;\n"
        "  save (s)         - save current equation\n"
        "  load (l)         - load saved equation\n"
        "  change (c)       - set value for variable;\n"
        "  watch (w)        - show current equation;\n"
        "  differentiate (d)- differentiate current equation;\n"
        "  graph (g)        - print graphviz text\n"
        "  latex (x)        - print latex text\n"
        "  help (h)         - show this text.\n"
        "Format:\n"
        "  double numbers (-123.123)\n"
        "  signs ( + - * / ^ )\n"
        "  variables - one letter after $\n"
    );
}

Expression* in() {
    printf("Print required equation:\n");
    char *buffer = calloc(STR_SIZE + 1, sizeof(*buffer));
    scanf("%" STR_S_SIZE "s", buffer);
    flush();
    Expression *expr = calculator_parse(buffer);
    if (!expr) printf("Error: cannot understand equation\n");
    free(buffer);
    return expr;
}

void watch(Expression* expr) {
    char* view = calculator_result(expr);
    printf("%s\n", view);
    free(view);
}

void save(Expression* expr, Expression* storage[STORAGE_S]) {
    printf("Print number of storage where to save (lesser %d)\n", STORAGE_S);
    int number = -1;
    do {
        scanf("%d", &number);
        flush();
    } while (number < 0 || number >= STORAGE_S);
    storage[number] = calculator_copy(expr);
}

Expression* load(Expression* storage[STORAGE_S]) {
    printf("Print number of storage where save from (lesser %d)\n", STORAGE_S);
    int number = -1;
    do {
        scanf("%d", &number);
        flush();
    } while (number < 0 || number >= STORAGE_S);
    return calculator_copy(storage[number]);
}

void change(Expression* expr, Expression* storage[STORAGE_S]) {
    printf("Print name of variable which to replace\n");
    char name = read_char();
    flush();
    printf("Print number of storage where save from (lesser %d)\n", STORAGE_S);
    int number = -1;
    do {
        scanf("%d", &number);
        flush();
    } while (number < 0 || number >= STORAGE_S);
    if (!storage[number]) {
        printf("Error: cannot use empty function\n");
        return;
    }
    calculator_define_variable(expr, name, storage[number]);
}

void differentiate(Expression* expr) {
    printf("Print name of variable by which to differentiate\n");
    char name = read_char();
    flush();
    calculator_differentiate(expr, name);
}

void graph(Expression* expr) {
    printf("Print path to file there to store\n");
    char path[64] = {0};
    scanf("%63s", path);
    FILE* file = NULL;
    while (!(file = fopen(path, "w"))) {
        printf("Cannot open file, try again\n");
        scanf("%63s", path);
    }
    calculator_graph(expr, file);
}

void latex(Expression* expr) {
    printf("Print path to file there to store\n");
    char path[64] = {0};
    scanf("%63s", path);
    FILE* file = NULL;
    while (!(file = fopen(path, "w"))) {
        printf("Cannot open file, try again\n");
        scanf("%63s", path);
    }
    calculator_latex(expr, file);
}

int main() {
    printf("Calculator starts work\n");
    char c = 0;
    Expression* expr = NULL;
    Expression* storage[STORAGE_S] = {};
    while ((c = read_char()) != 'q') {
        flush();
        switch (c) {
            case 'h':
                help();
                break;
            case 'i':
                calclulator_destroy(expr);
                expr = in();
                break;
            case 'r':
                calculator_simplify(expr);
                break;
            case 'w':
                watch(expr);
                break;
            case 's':
                save(expr, storage);
                break;
            case 'l':
                calclulator_destroy(expr);
                expr = load(storage);
                break;
            case 'c':
                change(expr, storage);
                break;
            case 'd':
                differentiate(expr);
                break;
            case 'g':
                graph(expr);
                break;
            case 'x':
                latex(expr);
                break;
            default:
                printf("Unknown command\n");
        }
    }
    calclulator_destroy(expr);
    for (int i = 0; i < STORAGE_S; ++i) calclulator_destroy(storage[i]);
    printf("Calculator ends work\n");
}
