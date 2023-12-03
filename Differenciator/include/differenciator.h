#ifndef DIFFERENCIATOR_H
    #define DIFFERENCIATOR_H

    #include "tree.h"

    typedef int DError_t;

    enum DifferenciatorErrors {
        DIFFERENCIATOR_ERRORS_CANT_CONVERT_TEXT_FILE = 1 << 0,
        DIFFERENCIATOR_ERRORS_INVALID_SYNTAXIS       = 1 << 1,
        DIFFERENCIATOR_ERRORS_TREE_ERROR             = 1 << 2,
        DIFFERENCIATOR_ERRORS_UNSUPPORTED_OPERATION  = 1 << 3,
    };

    DError_t create_dftr_tree(Tree * tree, char * buffer);
    void dftr_dump(Tree * tree);
    DError_t dftr_eval(const Tree * dftr_tree, double * answer);

#endif
