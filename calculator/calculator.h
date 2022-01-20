#ifndef HEADER_CALCULATOR
#define HEADER_CALCULATOR

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

#endif
