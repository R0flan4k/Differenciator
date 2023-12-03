#ifndef AKINATOR_H
    #define AKINATOR_H

    #include "tree.h"

    typedef int AError_t;

    enum AkinatorErrorsMasks {
        AKINATOR_ERRORS_CANT_CONVERT_TEXT_FILE = 1 << 0,
        AKINATOR_ERRORS_INVALID_SYNTAXIS       = 1 << 1,
        AKINATOR_ERRORS_TREE_ERROR             = 1 << 2,
        AKINATOR_ERRORS_INVALID_INPUT          = 1 << 3,
        AKINATOR_ERRORS_STACK_ERROR            = 1 << 4,
        AKINATOR_ERRORS_INVALID_OBJECT         = 1 << 5,
        AKINATOR_ERRORS_CANT_ALLOCATE_MEMORY   = 1 << 6,
        AKINATOR_ERRORS_CANT_OPEN_DATA_FILE    = 1 << 7,
    };

    enum AkinatorFunctions {
        AKINATOR_FUNCTIONS_NOTHING    = 0,
        AKINATOR_FUNCTIONS_GUESS      = 1,
        AKINATOR_FUNCTIONS_QUIT       = 2,
        AKINATOR_FUNCTIONS_MAKE_DUMP  = 3,
        AKINATOR_FUNCTIONS_DEFINITION = 4,
    };

    enum AkinatorAnswers {
        AKINATOR_ANSWERS_INVALID = 0,
        AKINATOR_ANSWERS_YES     = 1,
        AKINATOR_ANSWERS_NO      = 2,
    };

    AError_t create_akinator_tree(Tree * tree, char * buffer);
    AError_t akinator_start_game(Tree * tree);
    void akinator_dump(Tree * tree);
    AError_t akinator_vtor(const Tree * tree);

#endif
