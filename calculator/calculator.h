#ifndef HEADER_CALCULATOR
#define HEADER_CALCULATOR

#include <stdio.h>

typedef double Type;

struct Expression_;
typedef struct Expression_ Expression;

/**
 * @brief generate Expression from string
 */
Expression *calculator_parse(const char *expression);

/**
 * @brief set the value of variable 'name' to 'func'
 */
void calculator_define_variable(Expression *expr, char name, Expression* func);

/**
 * @brief single part of simplifying
 * @return bool was simplifyied
 */
int calculator_simplify_step(Expression *expr);

/**
 * @brief simplify (calculate) 'expr'
 */
void calculator_simplify(Expression *expr);

/**
 * @brief get string form of 'expr'
 */
char *calculator_result(Expression *expr);

/**
 * @brief destructor
 */
void calclulator_destroy(Expression* expr);

/**
 * @brief get copy of tree
 */
Expression* calculator_copy(Expression* expr);

/**
 * @brief just differentiate function by variable with name
 * @return expr (the same as input)
 */
Expression* calculator_differentiate(Expression* expr, char name);

/**
 * @brief print text for graphviz, describing expr, to file
 */
void calculator_graph(Expression* expr, FILE* file);

/**
 * @brief print text for latex, describing expr, to file
 */
void calculator_latex(Expression* expr, FILE* file);

#define STACK_DEPTH 0x100

#endif
