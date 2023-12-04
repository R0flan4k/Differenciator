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
const DifferenciatorVariable SUPPORTED_VARIABLES[] = {
    {.name = "x", .value = 0},
};
size_t SUPPORTED_VARIABLES_NUMBER = sizeof(SUPPORTED_VARIABLES) / sizeof(SUPPORTED_VARIABLES[0]);

static DError_t create_dftr_nodes_recursive(Tree * tree, TreeNode * node, char * * buffer_ptr);
static bool is_open_braket(const char * buffer_ptr);
static bool is_close_braket(const char * buffer_ptr);
static bool try_get_number(char * buffer_ptr, Tree_t * val, int * token_size);
static bool try_get_string(char * buffer_ptr, Tree_t * val, int * token_size);
static void dftr_print_tree_nodes(const TreeNode * node, FILE * fp);
static void dftr_print_tree_edges(const TreeNode * node, FILE * fp);
static DError_t dftr_eval_recursive(const TreeNode * node, double * answer);
static DError_t get_node_answer(const TreeNode * node, DifferenciatorInput input_type, double * answer, size_t i);
static DifferenciatorInput get_node_input_type(const TreeNode * node, size_t * i);
static DError_t dftr_create_diff_node(const TreeNode * node, Tree * d_tree, TreeNode * d_node);


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

    size_t i = 0;
    DifferenciatorInput input_type = get_node_input_type(node, &i);

    dftr_errors |= get_node_answer(node, input_type, answer, i);

    return dftr_errors;
}


static DifferenciatorInput get_node_input_type(const TreeNode * node, size_t * i)
{
    MY_ASSERT(node);

    DifferenciatorInput input_type = DIFFERENCIATOR_INPUT_INVALID;
    bool is_supported_input = false;

    switch (node->value.type)
    {
        case TREE_NODE_TYPES_NUMBER:
            input_type = DIFFERENCIATOR_INPUT_NUMBER;
            break;

        case TREE_NODE_TYPES_STRING:
            for (*i = 0; *i < MATH_OPERATIONS_ARRAY_SIZE; (*i)++)
            {
                if (!strcmp(node->value.value.string, MATH_OPERATIONS_ARRAY[*i].name))
                {
                    is_supported_input = true;
                    input_type = DIFFERENCIATOR_INPUT_OPERATION;
                    break;
                }
            }

            if (!is_supported_input)
            {
                for (*i = 0; *i < SUPPORTED_VARIABLES_NUMBER; (*i)++)
                {
                    if (!strcmp(node->value.value.string, SUPPORTED_VARIABLES[*i].name))
                    {
                        is_supported_input = true;
                        input_type = DIFFERENCIATOR_INPUT_VARIABLE;
                    }
                }
            }
            break;

        case TREE_NODE_TYPES_NO_TYPE:
        default:
            MY_ASSERT(0 && "UNREACHABLE");
            break;
    }

    return input_type;
}


static DError_t get_node_answer(const TreeNode * node, DifferenciatorInput input_type, double * answer, size_t i)
{
    MY_ASSERT(node);
    MY_ASSERT(answer);

    DError_t dftr_errors = 0;
    double left = 0, right = 0;

    switch (input_type)
    {
        case DIFFERENCIATOR_INPUT_NUMBER:
            *answer = node->value.value.number;
            break;

        case DIFFERENCIATOR_INPUT_OPERATION:
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

            *answer = MATH_OPERATIONS_ARRAY[i].operation(left, right);
            break;

        case DIFFERENCIATOR_INPUT_VARIABLE:
            *answer = SUPPORTED_VARIABLES[i].value;
            break;

        case DIFFERENCIATOR_INPUT_INVALID:
            dftr_errors |= DIFFERENCIATOR_ERRORS_INVALID_INPUT;
            break;

        default:
            MY_ASSERT(0 && "UNREACHABLE");
            break;
    }

    return dftr_errors;
}


DError_t dftr_create_diff_tree(const Tree * tree, Tree * d_tree)
{
    MY_ASSERT(tree);
    MY_ASSERT(d_tree);

    return dftr_create_diff_node(tree->root, d_tree, d_tree->root);
}


static DError_t dftr_create_diff_node(const TreeNode * node, Tree * d_tree, TreeNode * d_node)
{
    MY_ASSERT(node);
    MY_ASSERT(d_node);

    DError_t dftr_errors = 0;
    TError_t tree_errors = 0;
    size_t i = 0;
    DifferenciatorInput input_type = get_node_input_type(node, &i);

    switch (input_type)
    {
        case DIFFERENCIATOR_INPUT_NUMBER:
            d_node->value.type = TREE_NODE_TYPES_NUMBER;
            d_node->value.value.number = 0;
            break;

        case DIFFERENCIATOR_INPUT_VARIABLE:
            d_node->value.type = TREE_NODE_TYPES_NUMBER;
            d_node->value.value.number = 1;
            break;

        case DIFFERENCIATOR_INPUT_OPERATION:
            switch (MATH_OPERATIONS_ARRAY[i].id)
            {
                case MATH_OPERATIONS_ADDITION:
                    d_node->value.type = TREE_NODE_TYPES_STRING;
                    d_node->value.value.string = "+";
                    tree_errors |= tree_insert(d_tree, d_node, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node, TREE_NODE_BRANCH_RIGHT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    dftr_errors |= dftr_create_diff_node(node->left, d_tree, d_node->left);
                    dftr_errors |= dftr_create_diff_node(node->right, d_tree, d_node->right);
                    break;

                case MATH_OPERATIONS_SUBTRACTION:
                    d_node->value.type = TREE_NODE_TYPES_STRING;
                    d_node->value.value.string = "-";
                    tree_errors |= tree_insert(d_tree, d_node, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node, TREE_NODE_BRANCH_RIGHT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    dftr_errors |= dftr_create_diff_node(node->left, d_tree, d_node->left);
                    dftr_errors |= dftr_create_diff_node(node->right, d_tree, d_node->right);
                    break;

                case MATH_OPERATIONS_MULTIPLICATION:
                    d_node->value.type = TREE_NODE_TYPES_STRING;
                    d_node->value.value.string = "+";

                    tree_errors |= tree_insert(d_tree, d_node, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node, TREE_NODE_BRANCH_RIGHT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    d_node->left->value.type = TREE_NODE_TYPES_STRING;
                    d_node->left->value.value.string = "*";
                    d_node->right->value.type = TREE_NODE_TYPES_STRING;
                    d_node->right->value.value.string = "*";

                    tree_errors |= tree_insert(d_tree, d_node->left, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node->left, TREE_NODE_BRANCH_RIGHT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node->right, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node->right, TREE_NODE_BRANCH_RIGHT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    // d_node->left->left->value.type = node->left->value.type;
                    // d_node->left->left->value.value = node->left->value.value;
                    tree_errors |= tree_copy_branch(d_tree, d_node->left->left, node->left);
                    dftr_errors |= dftr_create_diff_node(node->left, d_tree, d_node->left->right);
                    // d_node->right->left->value.type = node->right->value.type;
                    // d_node->right->left->value.value = node->right->value.value;
                    tree_errors |= tree_copy_branch(d_tree, d_node->right->left, node->right);
                    dftr_errors |= dftr_create_diff_node(node->right, d_tree, d_node->right->right);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    break;

                case MATH_OPERATIONS_DIVISION:
                    d_node->value.type = TREE_NODE_TYPES_STRING;
                    d_node->value.value.string = "/";

                    tree_errors |= tree_insert(d_tree, d_node, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node, TREE_NODE_BRANCH_RIGHT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    d_node->left->value.type = TREE_NODE_TYPES_STRING;
                    d_node->left->value.value.string = "-";
                    d_node->right->value.type = TREE_NODE_TYPES_STRING;
                    d_node->right->value.value.string = "*";

                    tree_errors |= tree_insert(d_tree, d_node->left, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node->left, TREE_NODE_BRANCH_RIGHT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node->right, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node->right, TREE_NODE_BRANCH_RIGHT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    d_node->left->left->value.type = TREE_NODE_TYPES_STRING;
                    d_node->left->left->value.value.string = "*";
                    d_node->left->right->value.type = TREE_NODE_TYPES_STRING;
                    d_node->left->right->value.value.string = "*";

                    // d_node->right->left->value.type = node->right->value.type;
                    // d_node->right->left->value.value = node->right->value.value;
                    tree_errors |= tree_copy_branch(d_tree, d_node->right->left, node->right);
                    // d_node->right->right->value.type = node->right->value.type;
                    // d_node->right->right->value.value = node->right->value.value;
                    tree_errors |= tree_copy_branch(d_tree, d_node->right->right, node->right);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    tree_errors |= tree_insert(d_tree, d_node->left, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node->left, TREE_NODE_BRANCH_RIGHT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node->right, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node->right, TREE_NODE_BRANCH_RIGHT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    // d_node->left->left->left->value.type = node->left->value.type;
                    // d_node->left->left->left->value.value = node->left->value.value;
                    tree_errors |= tree_copy_branch(d_tree, d_node->left->left->left, node->left);
                    dftr_errors |= dftr_create_diff_node(node->left, d_tree, d_node->left->left->right);
                    // d_node->left->right->left->value.type = node->right->value.type;
                    // d_node->left->right->left->value.value = node->right->value.value;
                    tree_errors |= tree_copy_branch(d_tree, d_node->left->right->left, node->right);
                    dftr_errors |= dftr_create_diff_node(node->right, d_tree, d_node->left->right->right);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    break;

                case MATH_OPERATIONS_POWER:
                    d_node->value.type = TREE_NODE_TYPES_STRING;
                    d_node->value.value.string = "*";

                    tree_errors |= tree_insert(d_tree, d_node, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node, TREE_NODE_BRANCH_RIGHT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    d_node->left->value.type = TREE_NODE_TYPES_STRING;
                    d_node->left->value.value.string = "*";
                    dftr_errors |= dftr_create_diff_node(node->left, d_tree, d_node->right);

                    tree_errors |= tree_insert(d_tree, d_node->left, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node->left, TREE_NODE_BRANCH_RIGHT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    d_node->left->left->value.type = TREE_NODE_TYPES_NUMBER;
                    d_node->left->left->value.value.number = node->right->value.value.number;
                    d_node->left->right->value.type = TREE_NODE_TYPES_STRING;
                    d_node->left->right->value.value.string = "^";

                    tree_errors |= tree_insert(d_tree, d_node->left->right, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node->left->right, TREE_NODE_BRANCH_RIGHT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    // d_node->left->right->left->value.type = node->left->value.type;
                    // d_node->left->right->left->value.value = node->left->value.value;
                    tree_errors |= tree_copy_branch(d_tree, d_node->left->right->left, node->left);
                    d_node->left->right->right->value.type = TREE_NODE_TYPES_NUMBER;
                    d_node->left->right->right->value.value.number = node->right->value.value.number - 1;

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    break;

                case MATH_OPERATIONS_SINUS:
                    d_node->value.type = TREE_NODE_TYPES_STRING;
                    d_node->value.value.string = "*";

                    tree_errors |= tree_insert(d_tree, d_node, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node, TREE_NODE_BRANCH_RIGHT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    d_node->left->value.type = TREE_NODE_TYPES_STRING;
                    d_node->left->value.value.string = "cos";
                    dftr_errors |= dftr_create_diff_node(node->left, d_tree, d_node->right);

                    tree_errors |= tree_insert(d_tree, d_node->left, TREE_NODE_BRANCH_LEFT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    // d_node->left->left->value.type = node->left->value.type;
                    // d_node->left->left->value.value = node->left->value.value;
                    tree_errors |= tree_copy_branch(d_tree, d_node->left->left, node->left);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    break;

                case MATH_OPERATIONS_COSINUS:
                    d_node->value.type = TREE_NODE_TYPES_STRING;
                    d_node->value.value.string = "*";

                    tree_errors |= tree_insert(d_tree, d_node, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node, TREE_NODE_BRANCH_RIGHT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    d_node->left->value.type = TREE_NODE_TYPES_STRING;
                    d_node->left->value.value.string = "*";
                    dftr_errors |= dftr_create_diff_node(node->left, d_tree, d_node->right);

                    tree_errors |= tree_insert(d_tree, d_node->left, TREE_NODE_BRANCH_LEFT, TREE_NULL);
                    tree_errors |= tree_insert(d_tree, d_node->left, TREE_NODE_BRANCH_RIGHT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    d_node->left->left->value.type = TREE_NODE_TYPES_NUMBER;
                    d_node->left->left->value.value.number = -1;
                    d_node->left->right->value.type = TREE_NODE_TYPES_STRING;
                    d_node->left->right->value.value.string = "sin";

                    tree_errors |= tree_insert(d_tree, d_node->left->right, TREE_NODE_BRANCH_LEFT, TREE_NULL);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    // d_node->left->right->left->value.type = node->left->value.type;
                    // d_node->left->right->left->value.value = node->left->value.value;
                    tree_errors |= tree_copy_branch(d_tree, d_node->left->right->left, node->left);

                    if (tree_errors)
                    {
                        dftr_errors |= DIFFERENCIATOR_ERRORS_TREE_ERROR;
                        return dftr_errors;
                    }

                    break;

                default:
                    MY_ASSERT(0 && "UNREACHABLE");
                    break;
            }
            break;

        case DIFFERENCIATOR_INPUT_INVALID:
        default:
            MY_ASSERT(0 && "UNREACHABLE");
            break;
    }

    return dftr_errors;
}
