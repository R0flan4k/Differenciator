#include <math.h>

#include "math_operations.h"
#include "my_assert.h"

static MathOperation create_math_operation(const char * op_name, MathOperationTypes op_type,
                                           MathOperations id, double (*operation)(const double, const double));

MathOperation MATH_OPERATIONS_ARRAY[] = {
    create_math_operation("+", MATH_OPERATION_TYPES_BINARY, MATH_OPERATIONS_ADDITION, math_op_addition),
    create_math_operation("-", MATH_OPERATION_TYPES_BINARY, MATH_OPERATIONS_SUBTRACTION, math_op_subtraction),
    create_math_operation("*", MATH_OPERATION_TYPES_BINARY, MATH_OPERATIONS_MULTIPLICATION, math_op_multiplication),
    create_math_operation("/", MATH_OPERATION_TYPES_BINARY, MATH_OPERATIONS_DIVISION, math_op_division),
    create_math_operation("^", MATH_OPERATION_TYPES_BINARY, MATH_OPERATIONS_POWER, math_op_power),
    create_math_operation("sin", MATH_OPERATION_TYPES_UNARY, MATH_OPERATIONS_SINUS, math_op_sinus),
    create_math_operation("cos", MATH_OPERATION_TYPES_UNARY, MATH_OPERATIONS_COSINUS, math_op_cosinus),
};
size_t MATH_OPERATIONS_ARRAY_SIZE = sizeof(MATH_OPERATIONS_ARRAY) / sizeof(MATH_OPERATIONS_ARRAY[0]);


static MathOperation create_math_operation(const char * op_name, MathOperationTypes op_type,
                                           MathOperations op_id, double (*operation)(const double, const double))
{
    MY_ASSERT(op_name);

    MathOperation op = {
        .name = op_name,
        .type = op_type,
        .id   = op_id,
        .operation = operation,
    };

    return op;
}


double math_op_addition(const double val1, const double val2)
{
    return val1 + val2;
}


double math_op_subtraction(const double val1, const double val2)
{
    return val1 - val2;
}


double math_op_multiplication(const double val1, const double val2)
{
    return val1 * val2;
}


double math_op_division(const double val1, const double val2)
{
    return val1 / val2;
}


double math_op_power(const double val1, const double val2)
{
    return pow(val1, val2);
}


double math_op_sinus(const double val, const double)
{
    return sin(val);
}


double math_op_cosinus(const double val, const double)
{
    return cos(val);
}
