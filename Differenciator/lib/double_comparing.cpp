#include <math.h>

#include "double_comparing.h"

const double EPSILON = 1.0e-6;

bool is_equal_double(double val1, double val2)
{
    return (abs(val1 - val2) < EPSILON);
}
