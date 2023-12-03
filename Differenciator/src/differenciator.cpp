#include <string.h>
#include <ctype.h>

#include "differenciator.h"
#include "my_assert.h"
#include "tree.h"
#include "strings.h"
#include "file_processing.h"
#include "math_operations.h"

const char * DIFFERENCIATOR_DUMP_FILE_NAME = "./graphviz/differenciator_dump";
const size_t MAX_FILE_NAME_SIZE = 64;

static DError_t create_dftr_nodes_recursive(Tree * tree, TreeNode * node, char * * buffer_ptr);
static bool is_open_braket(const char * buffer_ptr);
static bool is_close_braket(const char * buffer_ptr);
static bool try_get_number(char * buffer_ptr, Tree_t * val, int * token_size);
static bool try_get_string(char * buffer_ptr, Tree_t * val, int * token_size);
static void dftr_print_tree_nodes(const TreeNode * node, FILE * fp);
static void dftr_print_tree_edges(const TreeNode * node, FILE * fp);
static DError_t dftr_eval_recursive(const TreeNode * node, double * answer);


DError_t create_dftr_tree(Tree * tree, char * buffer)
{
    MY_ASSERT(tree);
    MY_ASSERT(buffer);

    char * buffer_ptr = buffer;
    DError_t dftr_errors = 0;

    buffer_ptr = skip_spaces(buffer_ptr);

    if (!is_open_braket(buffer_ptr))
    {
        dftr_errors |= DIFFERENCIATOR_ERRORS_INVALID_SYNTAXIS;
        return dftr_errors;
    }
    buffer_ptr++;

    buffer_ptr = skip_spaces(buffer_ptr);

    if (dftr_errors = create_dftr_nodes_recursive(tree, tree->root, &buffer_ptr))
    {
        return dftr_errors;
    }

    return dftr_errors;
}


static DError_t create_dftr_nodes_recursive(Tree * tree, TreeNode * node, char * * buffer_ptr)
{
    MY_ASSERT(buffer_ptr);
    MY_ASSERT(node);

    DError_t dftr_errors = 0;
    TError_t tree_errors  = 0;

    *buffer_ptr = skip_spaces(*buffer_ptr);

    if (is_open_braket(*buffer_ptr))
    {
        (*buffer_ptr)++;

        if (tree_errors = tree_insert(tree, node, TREE_NODE_BRANCH_LEFT, TREE_NULL))
        // if (tree_errors = tree_insert(tree, node, TREE_NODE_BRANCH_LEFT, 0))
        {
            tree_dump(tree);
            dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
            return dftr_errors;
        }

        if (dftr_errors = create_dftr_nodes_recursive(tree, node->left, buffer_ptr))
        {
            return dftr_errors;
        }
    }

    *buffer_ptr = skip_spaces(*buffer_ptr);

    if (is_open_braket(*buffer_ptr) || is_close_braket(*buffer_ptr))
    {
        dftr_errors |= DIFFERENCIATOR_ERRORS_INVALID_SYNTAXIS;
        return dftr_errors;
    }

    int token_size = 0;
    bool is_success_read = try_get_number(*buffer_ptr, &(node->value), &token_size);
    // bool is_success_read = try_get_number(*buffer_ptr, &(node->value), &token_size);

    if (!is_success_read)
        is_success_read = try_get_string(*buffer_ptr, &(node->value), &token_size);

    if (!is_success_read)
    {
        dftr_errors |= DIFFERENCIATOR_ERRORS_INVALID_SYNTAXIS;
        return dftr_errors;
    }

    *buffer_ptr += token_size;

    *buffer_ptr = skip_spaces(*buffer_ptr);

    if (is_open_braket(*buffer_ptr))
    {
        (*buffer_ptr)++;

        // if (tree_errors = tree_insert(tree, node, TREE_NODE_BRANCH_RIGHT, 0))
        if (tree_errors = tree_insert(tree, node, TREE_NODE_BRANCH_RIGHT, TREE_NULL))
        {
            tree_dump(tree);
            dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
            return dftr_errors;
        }

        if (dftr_errors = create_dftr_nodes_recursive(tree, node->right, buffer_ptr))
        {
            return dftr_errors;
        }
    }

    *buffer_ptr = skip_spaces(*buffer_ptr);

    if (!is_close_braket(*buffer_ptr))
    {
        dftr_errors |= DIFFERENCIATOR_ERRORS_INVALID_SYNTAXIS;
        return dftr_errors;
    }
    (*buffer_ptr)++;

    return dftr_errors;
}


static bool is_open_braket(const char * buffer_ptr)
{
    MY_ASSERT(buffer_ptr);

    return *buffer_ptr == '{';
}


static bool is_close_braket(const char * buffer_ptr)
{
    MY_ASSERT(buffer_ptr);

    return *buffer_ptr == '}';
}


static bool try_get_number(char * buffer_ptr, Tree_t * val, int * token_size)
{
    MY_ASSERT(buffer_ptr);
    MY_ASSERT(val);
    MY_ASSERT(token_size);

    double tmp_val = 0;
    int tmp_token_size = 0;

    if (!sscanf(buffer_ptr, "%lf%n", &tmp_val, &tmp_token_size))
    {
        return false;
    }

    printf("In try_get_number(): %lf\n", tmp_val);

    val->value.number = tmp_val;
    val->type = TREE_NODE_TYPES_NUMBER;
    *token_size = tmp_token_size;

    return true;
}


static bool try_get_string(char * buffer_ptr, Tree_t * val, int * token_size)
{
    MY_ASSERT(buffer_ptr);
    MY_ASSERT(val);

    val->value.string = buffer_ptr;
    val->type = TREE_NODE_TYPES_STRING;
    buffer_ptr = skip_no_spaces(buffer_ptr);

    char * tmp_buffer_ptr = buffer_ptr;
    tmp_buffer_ptr = skip_spaces(tmp_buffer_ptr);
    while (!is_open_braket(tmp_buffer_ptr) && !is_close_braket(tmp_buffer_ptr))
    {
        tmp_buffer_ptr = skip_no_spaces(tmp_buffer_ptr);
        buffer_ptr = tmp_buffer_ptr;
        tmp_buffer_ptr = skip_spaces(tmp_buffer_ptr);
    }
    *buffer_ptr = '\0';

    printf("In try_get_string(): %s\n", val->value.string);

    *token_size = (int) (buffer_ptr - val->value.string + 1);

    return true;
}


void dftr_dump(Tree * tree)
{
    MY_ASSERT(tree);

    FILE * fp = NULL;

    char dot_file_name[MAX_FILE_NAME_SIZE] = "";
    make_file_extension(dot_file_name, DIFFERENCIATOR_DUMP_FILE_NAME, ".dot");

    if (!(fp = file_open(dot_file_name, "wb")))
    {
        return;
    }

    fprintf(fp, "digraph G\n"
                "{\n"
                "    graph [dpi = 150]\n"
                "    ranksep = 0.6;\n"
                "    bgcolor = \"#f0faf0\"\n"
                "    splines = curved;\n"
                "    edge[minlen = 3];\n"
                "    node[shape = record, style = \"rounded\", color = \"#f58eb4\",\n"
                "        fixedsize = true, height = 1, width = 3, fontsize = 20];\n"
                "    {rank = min;\n"
                "        inv_min [style = invis];\n"
                "    }\n");

    if (tree->root)
    {
        dftr_print_tree_nodes(tree->root, fp);
        dftr_print_tree_edges(tree->root, fp);
    }

    fprintf(fp, "}");

    fclose(fp);

    static size_t dftr_dumps_count = 0;
    char png_dump_file_name[MAX_FILE_NAME_SIZE] = "";
    char command_string[MAX_STR_SIZE] = "";
    char extension_string[MAX_FILE_NAME_SIZE] = "";

    sprintf(extension_string, "%zd.png", dftr_dumps_count);
    make_file_extension(png_dump_file_name, DIFFERENCIATOR_DUMP_FILE_NAME, extension_string);
    sprintf(command_string, "dot %s -T png -o %s", dot_file_name, png_dump_file_name);
    system(command_string);

    dftr_dumps_count++;
}


static void dftr_print_tree_nodes(const TreeNode * node, FILE * fp)
{
    MY_ASSERT(node);

    fprintf(fp, "   node%p [ label = ", node);

    switch (node->value.type)
    {
        case TREE_NODE_TYPES_NUMBER:
            fprintf(fp, "\"{ %.2lf }\", color = green ]\n", node->value.value.number);
            break;

        case TREE_NODE_TYPES_STRING:
            fprintf(fp, "\"{ %s }\", color = blue ]\n", node->value.value.string);
            break;

        case TREE_NODE_TYPES_NO_TYPE:
        default:
            MY_ASSERT(0 && "UNREACHABLE");
            break;
    }

    if (node->left)
    {
        dftr_print_tree_nodes(node->left, fp);
    }

    if (node->right)
    {
        dftr_print_tree_nodes(node->right, fp);
    }

    return;
}

static void dftr_print_tree_edges(const TreeNode * node, FILE * fp)
{
    MY_ASSERT(node);

    if (node->left)
    {
        fprintf(fp, "    node%p:<l> -> node%p;\n", node, node->left);
        dftr_print_tree_edges(node->left, fp);
    }

    if (node->right)
    {
        fprintf(fp, "    node%p:<r> -> node%p;\n", node, node->right);
        dftr_print_tree_edges(node->right, fp);
    }

    return;
}


DError_t dftr_eval(const Tree * dftr_tree, double * answer)
{
    MY_ASSERT(dftr_tree);
    MY_ASSERT(answer);

    return dftr_eval_recursive(dftr_tree->root, answer);
}


static DError_t dftr_eval_recursive(const TreeNode * node, double * answer)
{
    MY_ASSERT(node);
    MY_ASSERT(answer);

    DError_t dftr_errors = 0;

    double left = 0, right = 0;
    bool is_supported_operation = false;
    size_t i = 0;

    switch (node->value.type)
    {
        case TREE_NODE_TYPES_NUMBER:
            *answer = node->value.value.number;
            break;

        case TREE_NODE_TYPES_STRING:
            for (i = 0; i < MATH_OPERATIONS_ARRAY_SIZE; i++)
            {
                if (!strcmp(node->value.value.string, MATH_OPERATIONS_ARRAY[i].name))
                {
                    is_supported_operation = true;
                    break;
                }
            }

            if (!is_supported_operation)
            {
                dftr_errors |= DIFFERENCIATOR_ERRORS_UNSUPPORTED_OPERATION;
                return dftr_errors;
            }

            switch (MATH_OPERATIONS_ARRAY[i].type)
            {
                case MATH_OPERATION_TYPES_UNARY:
                    MY_ASSERT(node->left);
                    dftr_errors = dftr_eval_recursive(node->left, &left);

                    break;

                case MATH_OPERATION_TYPES_BINARY:
                    MY_ASSERT(node->left);
                    MY_ASSERT(node->right);
                    dftr_errors |= dftr_eval_recursive(node->left, &left);
                    dftr_errors |= dftr_eval_recursive(node->right, &right);

                    break;

                default:
                    MY_ASSERT(0 && "UNREACHABLE");
                    break;
            }

            if (dftr_errors)
            {
                return dftr_errors;
            }

            *answer = MATH_OPERATIONS_ARRAY[i].operation(left, right);
            printf("%s: %lf\n\tleft: %lf\n\tright: %lf\n", MATH_OPERATIONS_ARRAY[i].name, *answer, left, right);

            break;

        case TREE_NODE_TYPES_NO_TYPE:
        default:
            MY_ASSERT(0 && "UNREACHABLE");
            break;
    }

    return dftr_errors;
}
