#ifndef HEADER_CALCULATOR
#define HEADER_CALCULATOR

enum Types_expression
{
    e_type_const,
    e_type_var,
    e_type_add,
    e_type_mul,
    e_type_div,
    e_type_sub,
    e_type_step,
    e_type_comp,
    e_type_func,
};

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
void calculator_define_variable(Expression *expr, const char *name, Type value);

/**
 * @brief simplify (calculate) 'expr'
 */
void calculator_simplify(Expression *expr);

/**
 * @brief get string form of 'expr'
 */
const char *calculator_result(Expression *expr);

#endif
