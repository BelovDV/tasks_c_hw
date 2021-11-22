#include "calculator.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct Expression_ // binary tree parent class
{
    Expression *left;
    Expression *right;
    enum Types_expression type;
};

typedef struct
{
    Expression *left;
    Expression *right;
    enum Types_expression type;
    Type value;
} Constant;

Expression *calculator_parse(const char *expression)
{
    size_t length = strlen(expression);
    int length = 0;
    Constant *root = calloc(1, sizeof(*root));
    root->type = e_type_const;
    sscanf(expression, "%lf%n", &root->value, &length);
    return root;
}

void calculator_define_variable(Expression *expr, const char *name, Type value)
{
    (void)expr;
    (void)name;
    (void)value;
}

void calculator_simplify(Expression *expr)
{
    (void)expr;
}

const char *calculator_result(Expression *expr)
{
}
