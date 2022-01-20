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
 * @brief set the value of variable 'name' to 'value'
 */
void calculator_define_variable(Expression *expr, char name, Type value);

/**
 * @brief single part of simplifying
 * @param voice bool should use printf for simplifying steps
 * @return bool was simplifyied
 */
int calculator_simplify_step(Expression *expr, int voice);

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

#endif
