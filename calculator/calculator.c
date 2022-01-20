#include "calculator.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

enum Types_expression
{
    e_none,

    e_type_const, // the only one that has not 2 children
    e_type_var,
    e_type_add,
    e_type_mul,
    e_type_div,
    e_type_sub,
    e_type_step,
    e_type_comp,
    e_type_func,

    e_tok_open_br,
    e_tok_close_br,

    /*
    e_leaf_... const, var, func_name, ???
    e_knot_... add, div - operators
    e_control_... - brackets
    */
};

typedef struct {
    Expression* left;
    Expression* right;
    Expression* parent;
} Expr_knot;
typedef struct {
    Type value;
} Expr_leaf;

struct Expression_
{
    enum Types_expression type;
    union {
        Expr_knot knot;
        Expr_leaf leaf;
    };
};

void calclulator_destroy(Expression* expr) {
    if (!expr) return;
    if (expr->type != e_type_const) {
        calclulator_destroy(expr->knot.left);
        calclulator_destroy(expr->knot.right);
    }
    free(expr);
}

// ===== // PARSER // ===== //

typedef struct {
    enum Types_expression type;
    Type value; // for const
} Token;

static size_t generate_tokens(const char* expression, Token* result) {
    size_t amount = 0;
    size_t iter = 0;
    while (expression[iter]) {
        int ln = 0;
        if (expression[iter] == '+')
            result[amount++].type = e_type_add, ln = 1;
        else if (expression[iter] == '*')
            result[amount++].type = e_type_mul, ln = 1;
        else if (expression[iter] == '/')
            result[amount++].type = e_type_div, ln = 1;
        else if (expression[iter] == '-')
            result[amount++].type = e_type_sub, ln = 1;
        else if (expression[iter] == '(')
            result[amount++].type = e_tok_open_br, ln = 1;
        else if (expression[iter] == ')')
            result[amount++].type = e_tok_close_br, ln = 1;
        else if (sscanf(expression + iter, "%lf%n", &result[amount].value, &ln))
            result[amount++].type = e_type_const;
        else {
            printf("Error: unknoun token at position %lu\n", iter);
            printf("\t'%s'\n", expression + iter);
            return 0;
        }
        iter += ln;
    }
    return amount;
}

#define ALLOC(ptr) ptr = calloc(1, sizeof(*ptr)) // arg only single ptr
#define CUR (tokens[iter])
#define TYPE (tokens[iter].type)

static size_t iter;
static size_t amount;
static Token* tokens;
static Expression* generate_tree();
static Expression* generate_tree_3() {
    if (iter == amount) return NULL;
    Expression* result = NULL;
    if (TYPE == e_type_const) {
        ALLOC(result);
        result->type = e_type_const;
        result->leaf.value = CUR.value;
        ++iter;
    }
    else if (TYPE == e_tok_open_br) {
        ++iter;
        result = generate_tree();
        if (TYPE != e_tok_close_br) {
            calclulator_destroy(result);
            result = NULL;
        }
        ++iter;
    }
    return result;
}
static Expression* generate_tree_2() {
    return generate_tree_3(); // TODO - '^'
}
static Expression* generate_tree_1() {
    Expression* root = generate_tree_2();
    if (!root) return NULL;
    while (iter < amount && (TYPE == e_type_mul || TYPE == e_type_div)) {
        Expression* vsp = NULL;
        ALLOC(vsp);
        vsp->type = TYPE;
        vsp->knot.left = root;
        root = vsp;
        ++iter;
        root->knot.right = generate_tree_2();
        if (!root->knot.right) {
            calclulator_destroy(root);
            return NULL;
        }
    }
    return root;
}
static Expression* generate_tree() {
    Expression* root = generate_tree_1();
    if (!root) return NULL;
    while (iter < amount && (TYPE == e_type_add || TYPE == e_type_sub)) {
        Expression* vsp = NULL;
        ALLOC(vsp);
        vsp->type = TYPE;
        vsp->knot.left = root;
        root = vsp;
        ++iter;
        root->knot.right = generate_tree_1();
        if (!root->knot.right) {
            calclulator_destroy(root);
            return NULL;
        }
    }
    return root;
}

Expression *calculator_parse(const char *expression)
{
    size_t length = strlen(expression);
    tokens = calloc(length, sizeof(Token));
    if (!tokens) return NULL;
    size_t count = generate_tokens(expression, tokens);

    iter = 0;
    amount = count;

    Expression* result = generate_tree();
    free(tokens);
    return result;
}

// ===== // HARD // ===== //

void calculator_define_variable(Expression *expr, char name, Type value)
{
    (void)expr;
    (void)name;
    (void)value;
}

// ===== // SIMPLIFY // ===== //

#define OPER(x) { \
    calculator_simplify_step(expr->knot.left, voice);   \
    calculator_simplify_step(expr->knot.right, voice);  \
    Type left = expr->knot.left->leaf.value;            \
    Type right = expr->knot.right->leaf.value;          \
    calclulator_destroy(expr->knot.left);               \
    calclulator_destroy(expr->knot.right);              \
    expr->type = e_type_const;                          \
    expr->leaf.value = left x right;                    \
    return 1;                                           \
}

int calculator_simplify_step(Expression *expr, int voice) {
    switch (expr->type) {
        case e_type_const:
            return 0;
        case e_type_add: OPER(+);
        case e_type_sub: OPER(-);
        case e_type_mul: OPER(*);
        case e_type_div: OPER(/);
        default: return 0;
    }
}

void calculator_simplify(Expression *expr) {
    while (calculator_simplify_step(expr, 0));
}

// ===== // RESULT // ===== //

static int add_position(Expression* root, char* dest, size_t pos) {
    if (root->type == e_type_const) {
        int length = 0;
        sprintf(dest + pos, "%lf%n", root->leaf.value, &length);
        pos += length;
        return length;
    }
    dest[pos] = '(';
    int delta = 1;
    delta += add_position(root->knot.left, dest, pos + delta);
    switch (root->type) {
        case e_type_add: dest[pos + delta++] = '+'; break;
        case e_type_mul: dest[pos + delta++] = '*'; break;
        case e_type_sub: dest[pos + delta++] = '-'; break;
        case e_type_div: dest[pos + delta++] = '/'; break;
        default: dest[0] = 0;
    }
    delta += add_position(root->knot.right, dest, pos + delta);
    dest[pos + delta] = ')';
    delta += 1;
    return delta;
}

char *calculator_result(Expression *expr)
{
    // TODO - assume, 4096 - good size
    char* result = calloc(1, 4096);
    add_position(expr, result, 0);
    return result;
}
