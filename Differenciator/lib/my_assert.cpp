#include <stdio.h>

#include "my_assert.h"
#ifndef NDEBUG
    void my_func_assert(const bool expr, const char * expr_string, const char * date,
                        const int line, const char * file)
    {
        if (!expr)
        {
            printf("Assert error: %s, line %d, %s, file %s\n",
                    expr_string, line, date, file);
            exit(EXIT_FAILURE);
        }
    }
#endif // NDEBUG
