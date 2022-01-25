#include "calculator.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>

enum Types_expression {
    e_none,

    e_leaf_const,
    e_leaf_var,
    e_leaf_func,

    e_knot_add,
    e_knot_sub,
    e_knot_div,
    e_knot_mul,
    e_knot_step,
    e_knot_oper,

    e_control_open,
    e_control_close,

    e_func_log,
};

static int is_leaf(int type) {
    return type == e_leaf_const || type == e_func_log || type == e_leaf_var;
}

typedef struct {
    Expression* left;
    Expression* right;
    // Expression* parent;
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
#if 0
    static void* temp_buffer[STACK_DEPTH];
    Expression** stack = temp_buffer;
    size_t position = 0;
    stack[position++] = expr;
    while (position) {
        if (is_leaf(stack[position - 1]->type)) {
            free(stack[--position]);
        }
        else if (stack[position - 1]->knot.left) {
            stack[position++] = stack[position - 1]->knot.left;
        }
        else if (stack[position - 1]->knot.right) {
            stack[position++] = stack[position - 1]->knot.right;
        }
        else {
            free(stack[--position]);
        }
        if (position == STACK_DEPTH) {
            printf("ERROR: stack end reached, please, increase "
                    "STACK_DEPTH in calculator.h\n");
            return;
        }
    }
#else
    if (!expr) return;
    if (!is_leaf(expr->type)) {
        calclulator_destroy(expr->knot.left);
        calclulator_destroy(expr->knot.right);
    }
    free(expr);
#endif
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
            result[amount++].type = e_knot_add, ln = 1;
        else if (expression[iter] == '*')
            result[amount++].type = e_knot_mul, ln = 1;
        else if (expression[iter] == '/')
            result[amount++].type = e_knot_div, ln = 1;
        else if (expression[iter] == '-')
            result[amount++].type = e_knot_sub, ln = 1;
        else if (expression[iter] == '^')
            result[amount++].type = e_knot_step, ln = 1;
        else if (expression[iter] == '(')
            result[amount++].type = e_control_open, ln = 1;
        else if (expression[iter] == ')')
            result[amount++].type = e_control_close, ln = 1;
        else if (expression[iter] == '$') {
            if (expression[iter + 1] < 'a' || expression[iter + 1] > 'z') {
                printf("Error: variables should be letters\n");
                return 0;
            }
            result[amount].type = e_leaf_var;
            result[amount].value = expression[iter + 1];
            ++amount;
            ln = 2;
        }
        else if (sscanf(expression + iter, "%lf%n", &result[amount].value, &ln))
            result[amount++].type = e_leaf_const;
        else {
            printf("Error: unknoun token at position %lu\n", iter);
            printf("\t'%s'\n", expression + iter);
            return 0;
        }
        iter += ln;
    }
    return amount;
}

#define ALLOC(ptr, errval)          \
    ptr = calloc(1, sizeof(*ptr));  \
    if (!ptr) {                     \
        errno = ENOMEM;             \
        return errval;              \
    }
#define CUR (tokens[iter])
#define TYPE (tokens[iter].type)

static size_t iter;
static size_t amount;
static Token* tokens;
static Expression* generate_tree();
// third priority (maximum)
static Expression* generate_tree_3() {
    if (iter == amount) return NULL;
    Expression* result = NULL;
    if (TYPE == e_control_open) {
        ++iter;
        result = generate_tree();
        if (iter >= amount || TYPE != e_control_close) {
            calclulator_destroy(result);
            result = NULL;
        }
        ++iter;
    }
    else if (TYPE == e_leaf_const || TYPE == e_leaf_var) {
        ALLOC(result, NULL);
        result->type = TYPE;
        result->leaf.value = CUR.value;
        ++iter;
    }
    return result;
}
// second priority
static Expression* generate_tree_2() {
    Expression* root = generate_tree_3();
    if (!root) return NULL;
    while (iter < amount && (TYPE == e_knot_step)) {
        Expression* vsp = NULL;
        ALLOC(vsp, NULL);
        vsp->type = TYPE;
        vsp->knot.left = root;
        root = vsp;
        ++iter;
        root->knot.right = generate_tree_3();
        if (!root->knot.right) {
            calclulator_destroy(root);
            return NULL;
        }
    }
    return root;
}
// thirst priority
static Expression* generate_tree_1() {
    Expression* root = generate_tree_2();
    if (!root) return NULL;
    while (iter < amount && (TYPE == e_knot_mul || TYPE == e_knot_div)) {
        Expression* vsp = NULL;
        ALLOC(vsp, NULL);
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
    while (iter < amount && (TYPE == e_knot_add || TYPE == e_knot_sub)) {
        Expression* vsp = NULL;
        ALLOC(vsp, NULL);
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
    if (iter != amount && TYPE != e_control_close) {
        calclulator_destroy(root);
        return NULL;
    }
    return root;
}

Expression *calculator_parse(const char *expression)
{
    size_t length = strlen(expression);
    tokens = calloc(length, sizeof(*tokens));
    if (!tokens) return NULL;
    size_t count = generate_tokens(expression, tokens);

    iter = 0;
    amount = count;

    Expression* result = generate_tree();
    free(tokens);
    return result;
}

// ===== // SUBSTITUTE // ===== //

Expression* calculator_copy(Expression* expr) {
    if (!expr) return NULL;
    Expression* result = NULL;
    ALLOC(result, NULL);
    memcpy(result, expr, sizeof(*result));
    if (is_leaf(expr->type)) return result;
    result->knot.left = calculator_copy(expr->knot.left);
    result->knot.right = calculator_copy(expr->knot.right);
    return result;
}

void calculator_define_variable(Expression *expr, char name, Expression* func) {
    if (!expr) return;
    if (expr->type == e_leaf_var && expr->leaf.value == name) {
        Expression* copy = calculator_copy(func);
        memcpy(expr, copy, sizeof(*expr));
        free(copy);
    }
    else if (!is_leaf(expr->type)) {
        calculator_define_variable(expr->knot.left, name, func);
        calculator_define_variable(expr->knot.right, name, func);
    }
}

// ===== // SIMPLIFY // ===== //

static int simplify_simple(Expression* expr) {
    if (!is_leaf(expr->type)) {
        if (expr->knot.right->type == e_leaf_const && 
            (expr->type == e_knot_add || expr->type == e_knot_mul)) {
            Expression* vsp = expr->knot.right;
            expr->knot.right = expr->knot.left;
            expr->knot.left = vsp;
        }
    }
    if (expr->type == e_knot_mul) {
        if (expr->knot.left->type == e_leaf_const) {
            if (expr->knot.left->leaf.value == 0) {
                calclulator_destroy(expr->knot.left);
                calclulator_destroy(expr->knot.right);
                expr->type = e_leaf_const;
                expr->leaf.value = 0;
                return 1;
            }
            if (expr->knot.left->leaf.value == 1) {
                free(expr->knot.left);
                Expression* vsp = expr->knot.right;
                memcpy(expr, expr->knot.right, sizeof(*expr));
                free(vsp);
                return 1;
            }
        }
    }
    if (expr->type == e_knot_add) {
        if (expr->knot.left->type == e_leaf_const) {
            if (expr->knot.left->leaf.value == 0) {
                free(expr->knot.left);
                Expression* vsp = expr->knot.right;
                memcpy(expr, expr->knot.right, sizeof(*expr));
                free(vsp);
                return 1;
            }
        }
    }
    if (expr->type == e_knot_div) {
        if (expr->knot.right->type == e_leaf_const) {
            if (expr->knot.right->leaf.value == 1) {
                free(expr->knot.right);
                Expression* vsp = expr->knot.left;
                memcpy(expr, expr->knot.left, sizeof(*expr));
                free(vsp);
                return 1;
            }
        }
    }
    if (expr->type == e_knot_sub) {
        if (expr->knot.right->type == e_leaf_const) {
            expr->type = e_knot_add;
            expr->knot.right->leaf.value *= -1;
            return 1;
        }
    }
    return 0;
}

#define OPER(func) { \
    calculator_simplify_step(expr->knot.left);          \
    calculator_simplify_step(expr->knot.right);         \
    if (expr->knot.left->type == e_leaf_const &&        \
        expr->knot.right->type == e_leaf_const) {       \
        Type left = expr->knot.left->leaf.value;        \
        Type right = expr->knot.right->leaf.value;      \
        calclulator_destroy(expr->knot.left);           \
        calclulator_destroy(expr->knot.right);          \
        expr->type = e_leaf_const;                      \
        expr->leaf.value = func;                        \
        was_s |= 1;                                     \
    }                                                   \
    break;                                              \
}

int calculator_simplify_step(Expression *expr) {
    int was_s = 0;
    switch (expr->type) {
        case e_knot_add: OPER(left + right)
        case e_knot_sub: OPER(left - right)
        case e_knot_mul: OPER(left * right)
        case e_knot_div: OPER(left / right)
        case e_knot_step: OPER(pow(left, right))
        default: break;
    }
    was_s |= simplify_simple(expr);
    return was_s;
}

void calculator_simplify(Expression *expr) {
    while (calculator_simplify_step(expr));
}

// ===== // RESULT // ===== //

static int add_position(Expression* root, char* dest, size_t pos) {
    if (!root) return 0;
    if (root->type == e_leaf_const) {
        if ((double)(long)root->leaf.value == root->leaf.value) {
            int length = 0;
            sprintf(dest + pos, "%ld%n", (long)root->leaf.value, &length);
            pos += length;
            return length;
        }
        int length = 0;
        sprintf(dest + pos, "%lf%n", root->leaf.value, &length);
        pos += length;
        return length;
    }
    if (root->type == e_leaf_var) {
        sprintf(dest + pos, "%c", (char)root->leaf.value);
        pos += 1;
        return 1;
    }
    int delta = 1;
    if (root->type == e_knot_oper) {
        switch (root->knot.left->type) {
            case e_func_log:
                sprintf(dest + pos, "ln(");
                pos += 3;
                delta = add_position(root->knot.right, dest, pos);
                sprintf(dest + pos + delta, ")");
                pos += 1;
                return 4 + delta;
            default:
                printf("Error: don't know such function\n");
                return 0;
        }
    }
    dest[pos] = '(';
    delta += add_position(root->knot.left, dest, pos + delta);
    switch (root->type) {
        case e_knot_add: dest[pos + delta++] = '+'; break;
        case e_knot_mul: dest[pos + delta++] = '*'; break;
        case e_knot_sub: dest[pos + delta++] = '-'; break;
        case e_knot_div: dest[pos + delta++] = '/'; break;
        case e_knot_step: dest[pos + delta++] = '^'; break;
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
    if (result) add_position(expr, result, 0);
    return result;
}

// ===== // DIFFERENTIATION // ===== //

#define NEW calloc(1, sizeof(Expression))
#define C(tree) calculator_copy(tree)
#define D(tree) calculator_differentiate(tree, name)

Expression* calculator_differentiate(Expression* expr, char name) {
    if (expr->type == e_leaf_var && expr->leaf.value == name) {
        expr->type = e_leaf_const;
        expr->leaf.value = 1;
    }
    else if (expr->type == e_leaf_const || expr->type == e_leaf_var) {
        expr->type = e_leaf_const;
        expr->leaf.value = 0;
    }
    else if (expr->type == e_knot_add || expr->type == e_knot_sub) {
        calculator_differentiate(expr->knot.left, name);
        calculator_differentiate(expr->knot.right, name);
    }
    else if (expr->type == e_knot_mul) {
        Expression* left = expr->knot.left;
        Expression* right = expr->knot.right;
        expr->type = e_knot_add;

        expr->knot.left = NEW;
        expr->knot.right = NEW;
        if (!expr->knot.left || !expr->knot.right) {
            free(expr->knot.left);
            free(expr->knot.right);
            printf("Error: calloc failed\n");
            return expr;
        }
        expr->knot.left->type = expr->knot.right->type = e_knot_mul;

        expr->knot.left->knot.left = left;
        expr->knot.left->knot.right = D(C(right));
        expr->knot.right->knot.left = D(C(left));
        expr->knot.right->knot.right = right;
    }
    else if (expr->type == e_knot_div) {
        Expression* left = expr->knot.left;
        Expression* right = expr->knot.right;

        Expression* mul_1 = NEW;
        Expression* mul_2 = NEW;
        Expression* mul_d = NEW;
        Expression* sub = NEW;
        if (!mul_1 || !mul_2 || !mul_d || !sub) {
            free(mul_d);
            free(mul_1);
            free(mul_2);
            free(sub);
            printf("Error: calloc failed\n");
            return expr;
        }

        mul_1->type = mul_2->type = mul_d->type = e_knot_mul;
        sub->type = e_knot_sub;

        expr->knot.left = sub;
        sub->knot.left = mul_1;
        sub->knot.right = mul_2;
        expr->knot.right = mul_d;
        mul_1->knot.left = D(C(left));
        mul_1->knot.right = right;
        mul_2->knot.left = left;
        mul_2->knot.right = D(C(right));
        mul_d->knot.left = C(right);
        mul_d->knot.right = C(right);
    }
    else if (expr->type == e_knot_step) {
        Expression* left = expr->knot.left;
        Expression* right = expr->knot.right;
        
        Expression* new = NEW;
        Expression* func = NEW;
        Expression* ln = NEW;
        Expression* mul = NEW;
        if (!new || !ln || !mul || !func) {
            free(new);
            free(ln);
            free(mul);
            free(func);
            printf("Calloc error\n");
            return expr;
        }
        memcpy(new, expr, sizeof(*new));
        expr->type = e_knot_mul;
        expr->knot.left = new;
        expr->knot.right = mul;
        ln->type = e_func_log;
        func->type = e_knot_oper;
        func->knot.left = ln;
        func->knot.right = C(left);
        mul->type = e_knot_mul;
        mul->knot.left = func;
        mul->knot.right = C(right);
        calculator_differentiate(expr->knot.right, name);
    }
    else if (expr->type == e_knot_oper && expr->knot.left->type == e_func_log) {
        expr->type = e_knot_div;
        free(expr->knot.left);
        expr->knot.left = D(C(expr->knot.right));
    }
    else {
        printf("Error: cannot diffirenciate such function\n");
    }
    return expr;
}
