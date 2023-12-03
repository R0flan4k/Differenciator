#include <stdlib.h>

#include "differenciator.h"
#include "cmd_input.h"
#include "flags.h"
#include "file_processing.h"
#include "my_assert.h"

int main(int argc, char * argv[])
{
    if (!check_cmd_input(argc, argv))
    {
        return 1;
    }

    DError_t dftr_errors = 0;
    char * buffer = NULL;

    long buffer_size = 0;
    if (!(buffer_size = text_file_to_buffer(SOURCE_FILE_NAME, &buffer)))
    {
        dftr_errors |= DIFFERENCIATOR_ERRORS_CANT_CONVERT_TEXT_FILE;
        return dftr_errors;
    }

    Tree dftr_tree = {};

    op_new_tree(&dftr_tree, TREE_NULL);
    // op_new_tree(&dftr_tree, 0);

    if (dftr_errors = create_dftr_tree(&dftr_tree, buffer))
    {
        printf("Syntaxis error.\n");
        tree_dump(&dftr_tree);
        return dftr_errors;
    }

    double answer = 0;

    dftr_errors |= dftr_eval(&dftr_tree, &answer);
    if (dftr_errors)
        return dftr_errors;
    printf("Answer = %.2lf\n", answer);

    dftr_dump(&dftr_tree);
    tree_dump(&dftr_tree);

    free(buffer);
    op_delete_tree(&dftr_tree);

    return 0;
}
