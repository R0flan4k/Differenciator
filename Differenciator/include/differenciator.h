#ifndef DIFFERENCIATOR_H
    #define DIFFERENCIATOR_H

    #include "tree.h"

    typedef int DError_t;

    enum DifferenciatorErrors {
        DIFFERENCIATOR_ERRORS_CANT_CONVERT_TEXT_FILE = 1 << 0,
        DIFFERENCIATOR_ERRORS_INVALID_SYNTAXIS       = 1 << 1,
        DIFFERENCIATOR_ERRORS_TREE_ERROR             = 1 << 2,
        DIFFERENCIATOR_ERRORS_INVALID_INPUT          = 1 << 3,
    };

    enum DifferenciatorInput {
        DIFFERENCIATOR_INPUT_INVALID   = 0,
        DIFFERENCIATOR_INPUT_NUMBER    = 1,
        DIFFERENCIATOR_INPUT_OPERATION = 2,
        DIFFERENCIATOR_INPUT_VARIABLE  = 3,
    };

    struct DifferenciatorVariable {
        const char * name;
        double value;
    };

    DError_t create_dftr_tree(Tree * tree, char * buffer);
    void dftr_dump(Tree * tree);
    DError_t dftr_eval(const Tree * dftr_tree, double * answer);
    DError_t dftr_create_diff_tree(const Tree * tree, Tree * d_tree);
    void dftr_latex(const Tree * tree, const Tree * d_tree);

#endif
