#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "akinator.h"
#include "akinator_replics.h"
#include "tree.h"
#include "strings.h"
#include "my_assert.h"
#include "file_processing.h"
#include "stack.h"
#include "flags.h"

const size_t MAX_STRING_SIZE = 256;
const char * AKINATOR_TRASH_VALUE = "nill";
const char * AKINATOR_FUNCTIONS = "QGMD";
const char * AKINATOR_SUPPORTED_ANSWERS = "YN";
const char * AKINATOR_DUMP_FILE_NAME = "./graphviz/akinator_dump";
const size_t BUFFER_EXPAND_COEFFICIENT = 2;

static AError_t create_akinator_nodes_recursive(Tree * tree, TreeNode * node, char * * buffer_ptr);
static bool is_open_braket(const char * buffer_ptr);
static bool is_close_braket(const char * buffer_ptr);
static void output_message(const char * message);
static char get_answer(const char * posible_answers, size_t size);
static AkinatorFunctions akinator_chose_function(void);
static AError_t akinator_guess(Tree * tree, char * * buffer, char * * buffer_ptr, size_t * buffer_size);
static AError_t recursive_guessing(Tree * tree, TreeNode * node, char * * buffer, char * * buffer_ptr, size_t * buffer_size);
static AkinatorAnswers akinator_get_answer(void);
static AError_t akinator_add_object(Tree * tree, TreeNode * node, const char * last_object_name,
                                    const char * new_object_name, size_t new_object_name_size,
                                    const char * difference, size_t difference_size,
                                    char * * buffer, char * * buffer_ptr, size_t * buffer_size);
static void akinator_print_tree_nodes(const TreeNode * node, FILE * fp);
static void akinator_print_tree_edges(const TreeNode * node, FILE * fp);
static AError_t find_object(const Tree * tree, Stack * stk, char * object_name, size_t object_name_size);
static AError_t find_object_recursive(const TreeNode * node, Stack * stk, char * object_name, size_t object_name_size, bool * is_founded);
static AError_t print_definition(const Tree * tree, Stack * stk);
static AError_t print_definition_recursive(const TreeNode * node, Stack * stk);
static AError_t define_object(const Tree * tree, char * object_name, size_t object_name_size);
static AError_t expand_new_objects_buffer(char * * buffer, char * * buffer_ptr, size_t * buffer_size);
static bool get_save_necessity(void);
static AError_t akinator_save_changes(const Tree * tree, char * data_file_name);
static AError_t akinator_save_changes_recursive(const TreeNode * node, FILE * fp, int * recursion_depth);
static void fprint_tabulation(FILE * fp, int recursion_depth);
static AError_t akinator_nodes_vtor(const TreeNode * node);


AError_t create_akinator_tree(Tree * tree, char * buffer)
{
    MY_ASSERT(tree);
    MY_ASSERT(buffer);

    char * buffer_ptr = buffer;
    AError_t aktor_errors = 0;

    buffer_ptr = skip_spaces(buffer_ptr);

    if (!is_open_braket(buffer_ptr))
    {
        aktor_errors |= AKINATOR_ERRORS_INVALID_SYNTAXIS;
        return aktor_errors;
    }
    buffer_ptr++;

    buffer_ptr = skip_spaces(buffer_ptr);

    if (aktor_errors = create_akinator_nodes_recursive(tree, tree->root, &buffer_ptr))
    {
        return aktor_errors;
    }

    return aktor_errors;
}


static AError_t create_akinator_nodes_recursive(Tree * tree, TreeNode * node, char * * buffer_ptr)
{
    MY_ASSERT(buffer_ptr);
    MY_ASSERT(node);

    AError_t aktor_errors = 0;
    TError_t tree_errors  = 0;

    *buffer_ptr = skip_spaces(*buffer_ptr);

    if (is_open_braket(*buffer_ptr))
    {
        (*buffer_ptr)++;

        if (tree_errors = tree_insert(tree, node, TREE_NODE_BRANCH_LEFT, NULL))
        {
            tree_dump(tree);
            aktor_errors |= AKINATOR_ERRORS_TREE_ERROR;
            return aktor_errors;
        }

        if (aktor_errors = create_akinator_nodes_recursive(tree, node->left, buffer_ptr))
        {
            return aktor_errors;
        }
    }

    *buffer_ptr = skip_spaces(*buffer_ptr);

    if (is_open_braket(*buffer_ptr) || is_close_braket(*buffer_ptr))
    {
        aktor_errors |= AKINATOR_ERRORS_INVALID_SYNTAXIS;
        return aktor_errors;
    }

    node->value = *buffer_ptr;
    *buffer_ptr = skip_no_spaces(*buffer_ptr);

    char * tmp_buffer_ptr = *buffer_ptr;
    tmp_buffer_ptr = skip_spaces(tmp_buffer_ptr);
    while (!is_open_braket(tmp_buffer_ptr) && !is_close_braket(tmp_buffer_ptr))
    {
        tmp_buffer_ptr = skip_no_spaces(tmp_buffer_ptr);
        *buffer_ptr = tmp_buffer_ptr;
        tmp_buffer_ptr = skip_spaces(tmp_buffer_ptr);
    }
    **buffer_ptr = '\0';
    (*buffer_ptr)++;

    *buffer_ptr = skip_spaces(*buffer_ptr);

    if (is_open_braket(*buffer_ptr))
    {
        (*buffer_ptr)++;

        if (tree_errors = tree_insert(tree, node, TREE_NODE_BRANCH_RIGHT, NULL))
        {
            tree_dump(tree);
            aktor_errors |= AKINATOR_ERRORS_TREE_ERROR;
            return aktor_errors;
        }

        if (aktor_errors = create_akinator_nodes_recursive(tree, node->right, buffer_ptr))
        {
            return aktor_errors;
        }
    }

    *buffer_ptr = skip_spaces(*buffer_ptr);

    if (!is_close_braket(*buffer_ptr))
    {
        printf("%d\n", **buffer_ptr);
        aktor_errors |= AKINATOR_ERRORS_INVALID_SYNTAXIS;
        return aktor_errors;
    }
    (*buffer_ptr)++;

    return aktor_errors;
}


static bool is_open_braket(const char * buffer_ptr)
{
    return *buffer_ptr == '{';
}


static bool is_close_braket(const char * buffer_ptr)
{
    return *buffer_ptr == '}';
}


AError_t akinator_start_game(Tree * tree)
{
    MY_ASSERT(tree);

    AError_t aktor_errors = 0;
    char object_name[MAX_STR_SIZE] = "";
    size_t object_name_size = 0;

    if (aktor_errors = akinator_vtor(tree))
    {
        return aktor_errors;
    }

    output_message(ENG_AKINATOR_REPLICS.say_hello);
    output_message(ENG_AKINATOR_REPLICS.show_menu);

    AkinatorFunctions choosen_function = AKINATOR_FUNCTIONS_NOTHING;

    char * new_objects_buffer = NULL;
    size_t new_objects_buffer_size = MAX_STRING_SIZE;
    if (!(new_objects_buffer = (char *) calloc(MAX_STRING_SIZE, sizeof(char))))
    {
        aktor_errors |= AKINATOR_ERRORS_CANT_ALLOCATE_MEMORY;
        return aktor_errors;
    }
    char * new_objects_buffer_ptr = new_objects_buffer;


    while ((choosen_function = akinator_chose_function()) != AKINATOR_FUNCTIONS_QUIT)
    {

        switch (choosen_function)
        {
            case AKINATOR_FUNCTIONS_GUESS:
                output_message(ENG_AKINATOR_REPLICS.start_guessing);

                if (aktor_errors = akinator_guess(tree, &new_objects_buffer,
                                                  &new_objects_buffer_ptr, &new_objects_buffer_size))
                {
                    free(new_objects_buffer);
                    return aktor_errors;
                }

                break;

            case AKINATOR_FUNCTIONS_MAKE_DUMP:
                akinator_dump(tree);
                output_message(ENG_AKINATOR_REPLICS.dump_maked);

                break;

            case AKINATOR_FUNCTIONS_DEFINITION:
                output_message(ENG_AKINATOR_REPLICS.ask_definition_name);
                memset(object_name, 0, MAX_STR_SIZE);
                object_name_size = get_input(object_name, MAX_STR_SIZE);

                aktor_errors = define_object(tree, object_name, object_name_size);
                aktor_errors &= ~AKINATOR_ERRORS_INVALID_OBJECT;

                if (aktor_errors)
                {
                    free(new_objects_buffer);
                    return aktor_errors;
                }


                break;


            case AKINATOR_FUNCTIONS_NOTHING:
                break;

            case AKINATOR_FUNCTIONS_QUIT:
                MY_ASSERT(0 && "UNREACHABLE AKINATOR_FUNCTIONS_QUIT");
                break;

            default:
                MY_ASSERT(0 && "UNREACHABLE");
                break;
        }

        output_message(ENG_AKINATOR_REPLICS.show_menu);
    }

    output_message(ENG_AKINATOR_REPLICS.ask_save_necessity);

    if (get_save_necessity())
    {
        if (aktor_errors = akinator_save_changes(tree, SOURCE_FILE_NAME))
        {
            return aktor_errors;
        }

        output_message(ENG_AKINATOR_REPLICS.save_success);
    }

    output_message(ENG_AKINATOR_REPLICS.say_goodbye);

    free(new_objects_buffer);

    return aktor_errors;
}


static void output_message(const char * message)
{
    MY_ASSERT(message);

    printf("%s", message);

    return;
}


static char get_answer(const char * posible_answers, size_t size)
{
    MY_ASSERT(posible_answers);

    char ans = 0;
    char extra_char = 0;

    while ((ans = (char) getchar()) != EOF && isspace(ans))
        continue;

    ans = (char) toupper(ans);

    while ((extra_char = (char) getchar()) != '\n' && extra_char != EOF)
    {
        if (!isspace(extra_char))
            return (char) NULL;
    }

    for (size_t i = 0; i < size; i++)
    {
        if (ans == posible_answers[i])
            return ans;
    }

    return (char) NULL;
}


static AkinatorFunctions akinator_chose_function(void)
{
    char character = get_answer(AKINATOR_FUNCTIONS, strlen(AKINATOR_FUNCTIONS));

    switch (character)
    {
        case 'Q':
            return AKINATOR_FUNCTIONS_QUIT;
            break;

        case 'G':
            return AKINATOR_FUNCTIONS_GUESS;
            break;

        case 'M':
            return AKINATOR_FUNCTIONS_MAKE_DUMP;
            break;

        case 'D':
            return AKINATOR_FUNCTIONS_DEFINITION;
            break;

        case (char) NULL:
            return AKINATOR_FUNCTIONS_NOTHING;
            break;

        default:
            MY_ASSERT(0 && "UNREACHABLE");
            break;
    }

    return AKINATOR_FUNCTIONS_NOTHING;
}


static AError_t akinator_guess(Tree * tree, char * * buffer, char * * buffer_ptr, size_t * buffer_size)
{
    MY_ASSERT(tree);
    MY_ASSERT(buffer);
    MY_ASSERT(*buffer);
    MY_ASSERT(buffer_ptr);
    MY_ASSERT(*buffer_ptr);
    MY_ASSERT(buffer_size);

    AError_t aktor_errors = 0;

    if (aktor_errors = recursive_guessing(tree, tree->root, buffer, buffer_ptr, buffer_size))
    {
        return aktor_errors;
    }

    return aktor_errors;
}


static AError_t recursive_guessing(Tree * tree, TreeNode * node, char * * buffer, char * * buffer_ptr, size_t * buffer_size)
{
    MY_ASSERT(tree);
    MY_ASSERT(node);
    MY_ASSERT(buffer);
    MY_ASSERT(*buffer);
    MY_ASSERT(buffer_ptr);
    MY_ASSERT(*buffer_ptr);
    MY_ASSERT(buffer_size);

    AError_t aktor_errors = 0;
    char message[MAX_STRING_SIZE] = "";

    sprintf(message, "%s %s? ",
            ENG_AKINATOR_REPLICS.ask, node->value);

    output_message(message);
    output_message(ENG_AKINATOR_REPLICS.answer_help);

    AkinatorAnswers ans = AKINATOR_ANSWERS_INVALID;

    while ((ans = akinator_get_answer()) == AKINATOR_ANSWERS_INVALID)
        output_message(ENG_AKINATOR_REPLICS.answer_help);

    switch (ans)
    {
        case AKINATOR_ANSWERS_YES:
            if (!node->left)
            {
                output_message(ENG_AKINATOR_REPLICS.success);
                return aktor_errors;
            }

            if (aktor_errors = recursive_guessing(tree, node->left, buffer,
                                                  buffer_ptr, buffer_size))
            {
                return aktor_errors;
            }

            break;

        case AKINATOR_ANSWERS_NO:
            if (!node->right)
            {
                output_message(ENG_AKINATOR_REPLICS.failure);

                MY_ASSERT(node->value);
                Tree_t last_object_name = node->value;

                char new_object_name[MAX_STR_SIZE] = "";
                size_t new_object_name_size = get_input(new_object_name, MAX_STR_SIZE);

                char difference_message[MAX_STRING_SIZE] = "";
                sprintf(difference_message, "%s %s %s %s?\n",
                        ENG_AKINATOR_REPLICS.get_difference, new_object_name,
                        ENG_AKINATOR_REPLICS.and_word, last_object_name);
                output_message(difference_message);

                char difference[MAX_STR_SIZE] = "";
                size_t difference_size = get_input(difference, MAX_STR_SIZE);

                if (aktor_errors = akinator_add_object(tree, node, last_object_name,
                                                       new_object_name, new_object_name_size,
                                                       difference, difference_size,
                                                       buffer, buffer_ptr, buffer_size))
                {
                    return aktor_errors;
                }

                output_message(ENG_AKINATOR_REPLICS.next_time);

                return aktor_errors;
            }

            if (aktor_errors = recursive_guessing(tree, node->right, buffer,
                                                  buffer_ptr, buffer_size))
            {
                return aktor_errors;
            }

            break;

        case AKINATOR_ANSWERS_INVALID:
            MY_ASSERT(0 && "UNREACHABLE AKINATOR_ANSWER_INVALID");
            break;

        default:
            MY_ASSERT(0 && "UNREACHABLE");
            break;
    }

    return aktor_errors;
}


static AkinatorAnswers akinator_get_answer(void)
{
    char character = get_answer(AKINATOR_SUPPORTED_ANSWERS, strlen(AKINATOR_SUPPORTED_ANSWERS));

    switch (character)
    {
        case 'Y':
            return AKINATOR_ANSWERS_YES;
            break;

        case 'N':
            return AKINATOR_ANSWERS_NO;
            break;

        case (char) NULL:
            return AKINATOR_ANSWERS_INVALID;
            break;

        default:
            MY_ASSERT(0 && "UNREACHABLE");
            break;
    }

    return AKINATOR_ANSWERS_INVALID;
}


static AError_t akinator_add_object(Tree * tree, TreeNode * node, const char * last_object_name,
                                    const char * new_object_name, size_t new_object_name_size,
                                    const char * difference, size_t difference_size,
                                    char * * buffer, char * * buffer_ptr, size_t * buffer_size)
{
    MY_ASSERT(tree);
    MY_ASSERT(node);
    MY_ASSERT(new_object_name);
    MY_ASSERT(last_object_name);
    MY_ASSERT(difference);
    MY_ASSERT(buffer);
    MY_ASSERT(*buffer);
    MY_ASSERT(buffer_ptr);
    MY_ASSERT(*buffer_ptr);
    MY_ASSERT(buffer_size);

    TError_t tree_errors = 0;
    AError_t aktor_errors = 0;

    if (*buffer_size - 2 < *buffer_ptr - *buffer + difference_size + new_object_name_size)/////////////////////////////////////
        aktor_errors = expand_new_objects_buffer(buffer, buffer_ptr, buffer_size);

    if (aktor_errors)
        return aktor_errors;

    sprintf(*buffer_ptr, "%s", new_object_name);
    if (tree_errors = tree_insert(tree, node, TREE_NODE_BRANCH_LEFT, *buffer_ptr))
    {
        aktor_errors |= AKINATOR_ERRORS_TREE_ERROR;
        return aktor_errors;
    }
    *buffer_ptr += new_object_name_size;
    **buffer_ptr = '\0';
    (*buffer_ptr)++;

    sprintf(*buffer_ptr, "%s", difference);
    node->value = *buffer_ptr;
    *buffer_ptr += difference_size;
    **buffer_ptr = '\0';
    (*buffer_ptr)++;

    if (tree_errors = tree_insert(tree, node, TREE_NODE_BRANCH_RIGHT, last_object_name))
    {
        aktor_errors |= AKINATOR_ERRORS_TREE_ERROR;
        return aktor_errors;
    }

    return aktor_errors;
}


static AError_t expand_new_objects_buffer(char * * buffer, char * * buffer_ptr, size_t * buffer_size)
{
    MY_ASSERT(buffer);
    MY_ASSERT(*buffer);
    MY_ASSERT(buffer_ptr);
    MY_ASSERT(*buffer_ptr);
    MY_ASSERT(buffer_size);

    AError_t aktor_errors = 0;

    size_t last_position = *buffer_ptr - *buffer;
    char * pointer = *buffer;

    if (!(pointer = (char *) realloc(buffer, *buffer_size * BUFFER_EXPAND_COEFFICIENT)))
    {
        aktor_errors |= AKINATOR_ERRORS_CANT_ALLOCATE_MEMORY;
        return aktor_errors;
    }
    *buffer = pointer;
    *buffer_ptr = *buffer + last_position;
    *buffer_size *= BUFFER_EXPAND_COEFFICIENT;

    return aktor_errors;
}


void akinator_dump(Tree * tree)
{
    MY_ASSERT(tree);

    FILE * fp = NULL;

    char dot_file_name[MAX_STR_SIZE] = "";
    make_file_extension(dot_file_name, AKINATOR_DUMP_FILE_NAME, ".dot");

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
                "        fixedsize = true, height = 1, width = 6, fontsize = 15];\n"
                "    {rank = min;\n"
                "        inv_min [style = invis];\n"
                "    }\n");

    if (tree->root)
    {
        akinator_print_tree_nodes(tree->root, fp);
        akinator_print_tree_edges(tree->root, fp);
    }

    fprintf(fp, "}");

    fclose(fp);

    static size_t akinator_dumps_count = 0;
    char png_dump_file_name[MAX_STR_SIZE] = "";
    char command_string[MAX_STRING_SIZE] = "";
    char extension_string[MAX_STRING_SIZE] = "";

    sprintf(extension_string, "%zd.png", akinator_dumps_count);
    make_file_extension(png_dump_file_name, AKINATOR_DUMP_FILE_NAME, extension_string);
    sprintf(command_string, "dot %s -T png -o %s", dot_file_name, png_dump_file_name);
    system(command_string);

    akinator_dumps_count++;
}


static void akinator_print_tree_nodes(const TreeNode * node, FILE * fp)
{
    MY_ASSERT(node);
    MY_ASSERT(node->value);

    if (node->left && node->right)
    {
        fprintf(fp, "    node%p [ label = \"{ %s? | { <l> YES | NO  }}\" ]\n",
                node, node->value);
    }
    else
    {
        fprintf(fp, "    node%p [ label = \"{ %s }\", color = green ]\n",
                node, node->value);
    }

    if (node->right)
    {
        akinator_print_tree_nodes(node->right, fp);
    }

    if (node->left)
    {
        akinator_print_tree_nodes(node->left, fp);
    }

    return;
}

static void akinator_print_tree_edges(const TreeNode * node, FILE * fp)
{
    MY_ASSERT(node);

    if (node->left)
    {
        fprintf(fp, "    node%p:<l> -> node%p;\n", node, node->left);
        akinator_print_tree_edges(node->left, fp);
    }

    if (node->right)
    {
        fprintf(fp, "    node%p:<r> -> node%p;\n", node, node->right);
        akinator_print_tree_edges(node->right, fp);
    }

    return;
}


static AError_t find_object(const Tree * tree, Stack * stk, char * object_name, size_t object_name_size)
{
    MY_ASSERT(tree);
    MY_ASSERT(stk);
    MY_ASSERT(object_name);

    AError_t aktor_errors = 0;
    bool is_founded = false;

    aktor_errors = find_object_recursive(tree->root, stk, object_name, object_name_size, &is_founded);

    if (!is_founded)
        aktor_errors |= AKINATOR_ERRORS_INVALID_OBJECT;

    return aktor_errors;
}


static AError_t find_object_recursive(const TreeNode * node, Stack * stk, char * object_name, size_t object_name_size, bool * is_founded)
{
    MY_ASSERT(node);
    MY_ASSERT(stk);
    MY_ASSERT(object_name);
    MY_ASSERT(is_founded);

    AError_t aktor_errors = 0;
    Error_t stk_errors = 0;

    if (!(*is_founded) && !strncmp(node->value, object_name, object_name_size) &&
        !node->left && !node->right)
    {
        *is_founded = true;
        strcpy(object_name, node->value);
        return aktor_errors;
    }


    if (node->left && !(*is_founded))
    {
        if (aktor_errors = find_object_recursive(node->left, stk, object_name, object_name_size, is_founded))
            return aktor_errors;

        if (*is_founded)
        {
            if (stack_push(stk, (Elem_t) TREE_NODE_BRANCH_LEFT))
            {
                aktor_errors |= AKINATOR_ERRORS_STACK_ERROR;
                show_dump(stk, &stk_errors);
                return aktor_errors;
            }
        }
    }

    if (node->right && !(*is_founded))
    {
        if (aktor_errors = find_object_recursive(node->right, stk, object_name, object_name_size, is_founded))
            return aktor_errors;

        if (*is_founded)
        {
            if (stk_errors = stack_push(stk, (Elem_t) TREE_NODE_BRANCH_RIGHT))
            {
                aktor_errors |= AKINATOR_ERRORS_STACK_ERROR;
                show_dump(stk, &stk_errors);
                return aktor_errors;
            }
        }
    }

    return aktor_errors;
}


static AError_t print_definition(const Tree * tree, Stack * stk)
{
    MY_ASSERT(tree);
    MY_ASSERT(stk);

    return print_definition_recursive(tree->root, stk);
}


static AError_t print_definition_recursive(const TreeNode * node, Stack * stk)
{
    MY_ASSERT(node);
    MY_ASSERT(stk);

    AError_t aktor_errors = 0;
    Elem_t val = 0;
    Error_t stk_errors = 0;

    if (stk->size == 0)
    {
        output_message(".\n\n");
        return aktor_errors;
    }

    if (stk_errors = stack_pop(stk, &val))
    {
        aktor_errors |= AKINATOR_ERRORS_STACK_ERROR;
        show_dump(stk, &stk_errors);
        return aktor_errors;
    }

    if (val == (Elem_t) TREE_NODE_BRANCH_RIGHT)
        output_message("not ");

    output_message(node->value);

    if (stk->size >= 1)
        output_message(", ");

    switch ((TreeNodeBranches) val)
    {
        case TREE_NODE_BRANCH_LEFT:
            print_definition_recursive(node->left, stk);
            break;

        case TREE_NODE_BRANCH_RIGHT:
            print_definition_recursive(node->right, stk);
            break;

        default:
            MY_ASSERT(0 && "UNREACHABLE");
            break;
    }

    return aktor_errors;
}


static AError_t define_object(const Tree * tree, char * object_name, size_t object_name_size)
{
    MY_ASSERT(tree);
    MY_ASSERT(object_name);

    AError_t aktor_errors = 0;
    Error_t stk_errors = 0;
    Stack stk = {};

    if (stk_errors = stack_ctor(&stk))
    {
        aktor_errors |= AKINATOR_ERRORS_STACK_ERROR;
        show_dump(&stk, &stk_errors);
        return aktor_errors;
    }

    aktor_errors = find_object(tree, &stk, object_name, object_name_size);

    if (!(aktor_errors & AKINATOR_ERRORS_INVALID_OBJECT))
    {
        output_message(object_name);
        output_message(ENG_AKINATOR_REPLICS.definition);
        print_definition(tree, &stk);
    }
    else
    {
        output_message(ENG_AKINATOR_REPLICS.invalid_object);
    }

    if (stk_errors = stack_dtor(&stk))
    {
        aktor_errors |= AKINATOR_ERRORS_STACK_ERROR;
        show_dump(&stk, &stk_errors);
        return aktor_errors;
    }

    return aktor_errors;
}


static bool get_save_necessity(void)
{
    output_message(ENG_AKINATOR_REPLICS.answer_help);

    AkinatorAnswers ans = AKINATOR_ANSWERS_INVALID;

    while ((ans = akinator_get_answer()) == AKINATOR_ANSWERS_INVALID)
        output_message(ENG_AKINATOR_REPLICS.answer_help);

    switch (ans)
    {
        case AKINATOR_ANSWERS_YES:
            return true;
            break;

        case AKINATOR_ANSWERS_NO:
            return false;
            break;

        case AKINATOR_ANSWERS_INVALID:
        default:
            MY_ASSERT(0 && "UNREACHABLE");
            break;
    }

    return false;
}


static AError_t akinator_save_changes(const Tree * tree, char * data_file_name)
{
    MY_ASSERT(tree);
    MY_ASSERT(data_file_name);

    AError_t aktor_errors = 0;
    FILE * fp = NULL;

    if (!(fp = file_open(data_file_name, "wb")))
    {
        aktor_errors |= AKINATOR_ERRORS_CANT_OPEN_DATA_FILE;
        return aktor_errors;
    }

    int recursion_depth = 0;

    if (aktor_errors = akinator_save_changes_recursive(tree->root, fp, &recursion_depth))
    {
        fclose(fp);
        return aktor_errors;
    }

    fclose(fp);

    return aktor_errors;
}


static AError_t akinator_save_changes_recursive(const TreeNode * node, FILE * fp, int * recursion_depth)
{
    MY_ASSERT(node);
    MY_ASSERT(node->value);
    MY_ASSERT(fp);

    AError_t aktor_errors = 0;

    (*recursion_depth)++;

    fprint_tabulation(fp, *recursion_depth - 1);
    fprintf(fp, "{\n");

    if (node->left)
    {
        if (aktor_errors = akinator_save_changes_recursive(node->left, fp,
                                                           recursion_depth))
        {
            return aktor_errors;
        }
    }

    fprint_tabulation(fp, *recursion_depth);
    fprintf(fp, "%s\n", node->value);

    if (node->right)
    {
        if (aktor_errors = akinator_save_changes_recursive(node->right, fp,
                                                           recursion_depth))
        {
            return aktor_errors;
        }
    }

    fprint_tabulation(fp, *recursion_depth - 1);
    fprintf(fp, "}\n");

    (*recursion_depth)--;

    return aktor_errors;
}


static void fprint_tabulation(FILE * fp, int recursion_depth)
{
    MY_ASSERT(fp);

    for (int i = 0; i < recursion_depth; i++)
    {
        fprintf(fp, "\t");
    }

    return;
}


AError_t akinator_vtor(const Tree * tree)
{
    MY_ASSERT(tree);

    AError_t aktor_errors = 0;

    if (tree_vtor(tree))
    {
        aktor_errors |= AKINATOR_ERRORS_TREE_ERROR;
        tree_dump(tree);
        return aktor_errors;
    }

    if (aktor_errors = akinator_nodes_vtor(tree->root))
    {
        return aktor_errors;
    }

    return aktor_errors;
}


static AError_t akinator_nodes_vtor(const TreeNode * node)
{
    MY_ASSERT(node);

    AError_t aktor_errors = 0;

    if (node->left)
    {
        if (aktor_errors = akinator_nodes_vtor(node->left))
        {
            return aktor_errors;
        }
    }
    else
    {
        if (node->right)
        {
            aktor_errors |= AKINATOR_ERRORS_INVALID_OBJECT;
            return aktor_errors;
        }
    }

    if (node->right)
    {
        if (aktor_errors = akinator_nodes_vtor(node->right))
        {
            return aktor_errors;
        }
    }
    else
    {
        if (node->left)
        {
            aktor_errors |= AKINATOR_ERRORS_INVALID_OBJECT;
            return aktor_errors;
        }
    }

    return aktor_errors;
}
