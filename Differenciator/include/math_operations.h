#ifndef MATH_OPERATIONS_H
    #define MATH_OPERATIONS_H

    enum MathOperationTypes {
        MATH_OPERATION_TYPES_UNARY  = 1,
        MATH_OPERATION_TYPES_BINARY = 2,
    };

    enum MathOperations {
        MATH_OPERATIONS_ADDITION       = 1,
        MATH_OPERATIONS_SUBTRACTION    = 2,
        MATH_OPERATIONS_MULTIPLICATION = 3,
        MATH_OPERATIONS_DIVISION       = 4,
        MATH_OPERATIONS_POWER          = 5,
        MATH_OPERATIONS_SINUS          = 6,
        MATH_OPERATIONS_COSINUS        = 7,
    };

    struct MathOperation {
        const char * name;
        MathOperationTypes type;
        MathOperations id;
        double (*operation)(const double, const double);
    };

    extern MathOperation MATH_OPERATIONS_ARRAY[];
    extern size_t MATH_OPERATIONS_ARRAY_SIZE;

    double math_op_addition       (const double val1, const double val2);
    double math_op_subtraction    (const double val1, const double val2);
    double math_op_multiplication (const double val1, const double val2);
    double math_op_division       (const double val1, const double val2);
    double math_op_power          (const double val1, const double val2);
    double math_op_sinus          (const double val,  const double trash);
    double math_op_cosinus        (const double val,  const double trash);

#endif
