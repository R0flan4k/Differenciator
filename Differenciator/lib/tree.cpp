#include <string.h>

#include "tree.h"
#include "my_assert.h"
#include "file_processing.h"
#include "strings.h"

const char * TREE_DUMP_FILE_NAME = "./graphviz/tree_dump";

const size_t TRASH_VALUE = 0xAB1BA5;
const size_t BUFFER_SIZE = 256;

static size_t tree_free(TreeNode * * main_node);
static size_t tree_free_iternal(TreeNode * * node, size_t * count);
static void print_tree_nodes(const TreeNode * node, FILE * fp);
static void print_tree_edges(const TreeNode * node, FILE * fp);
static TError_t tree_create_node(Tree * tree, TreeNode * * node_ptr);
static void print_text_nodes(const TreeNode * main_node);


TError_t op_new_tree(Tree * tree, const Tree_t root_value)
{
    MY_ASSERT(tree);

    TError_t errors = 0;

    if (!tree_vtor(tree))
    {
        errors |= TREE_ERRORS_ALREADY_CONSTRUCTED;
        return errors;
    }

    if (!(tree->root = (TreeNode *) calloc(1, sizeof(TreeNode))))
    {
        errors |= TREE_ERRORS_CANT_ALLOCATE_MEMORY;
        return errors;
    }
    tree->size = 1;

    tree->root->left = NULL;
    tree->root->right = NULL;

    tree->root->value = root_value;

    return errors;
}


static size_t tree_free(TreeNode * * main_node)
{
    MY_ASSERT(main_node);

    size_t count = 0;

    return tree_free_iternal(main_node, &count);
}


static size_t tree_free_iternal(TreeNode * * node, size_t * count)
{
    MY_ASSERT(node);
    MY_ASSERT(*node);
    MY_ASSERT(count);

    if ((*node)->left)
    {
        tree_free_iternal(&(*node)->left, count);
    }

    if ((*node)->right)
    {
        tree_free_iternal(&(*node)->right, count);
    }

    free(*node);
    *node = NULL;
    (*count)++;

    return *count;
}


TError_t op_delete_tree(Tree * tree)
{
    MY_ASSERT(tree);

    TError_t errors = 0;

    if (!tree->root)
    {
        errors |= TREE_ERRORS_ALREADY_DESTRUCTED;
        return errors;
    }

    if (tree_free(&(tree->root)) != tree->size)
        errors |= TREE_ERRORS_INVALID_SIZE;
    tree->size = TRASH_VALUE;

    return errors;
}


TError_t tree_vtor(const Tree * tree)
{
    MY_ASSERT(tree);

    TError_t errors = 0;

    if (tree->size == 0)
        errors |= TREE_ERRORS_INVALID_SIZE;

    if (!tree->root)
        errors |= TREE_ERRORS_INVALID_ROOT;

    return errors;
}


static TError_t tree_create_node(Tree * tree, TreeNode * * node_ptr)
{
    MY_ASSERT(tree);
    MY_ASSERT(!*node_ptr);

    TError_t errors = 0;

    if (!(*node_ptr = (TreeNode *) calloc(1, sizeof(TreeNode))))
    {
        errors |= TREE_ERRORS_CANT_ALLOCATE_MEMORY;
        return errors;
    }

    (*node_ptr)->left = NULL;
    (*node_ptr)->right = NULL;

    (*node_ptr)->value = TREE_NULL;

    tree->size++;

    return errors;
}


TError_t tree_insert(Tree * tree, TreeNode * node, TreeNodeBranches mode, const Tree_t value)
{
    MY_ASSERT(node);

    TError_t errors = 0;

    if (errors = tree_vtor(tree))
    {
        return errors;
    }

    switch (mode)
    {
        case TREE_NODE_BRANCH_LEFT:
            if (node->left)
            {
                errors |= TREE_ERRORS_INVALID_NODE;
                return errors;
            }

            if (errors = tree_create_node(tree, &node->left))
                return errors;

            node->left->value = value;

            break;

        case TREE_NODE_BRANCH_RIGHT:
            if (node->right)
            {
                errors |= TREE_ERRORS_INVALID_NODE;
                return errors;
            }

            if (errors = tree_create_node(tree, &node->right))
                return errors;

            node->right->value = value;

            break;

        default:
            MY_ASSERT(0 && "UNREACHABLE");
            break;
    }

    return errors;
}


TError_t tree_delete_branch(Tree * tree, TreeNode * * node)
{
    MY_ASSERT(tree);
    MY_ASSERT(node);
    MY_ASSERT(*node);

    TError_t errors = 0;

    if (errors = tree_vtor(tree))
    {
        return errors;
    }

    size_t deleting_nodes_count = tree_free(node);
    *node = NULL;

    if (tree->size < deleting_nodes_count)
    {
        errors |= TREE_ERRORS_INVALID_SIZE;
        return errors;
    }

    tree->size -= deleting_nodes_count;

    return errors;
}


void tree_dump_iternal(const Tree * tree,
                       const char * tree_name, const char * func,
                       const int line, const char * file)
{
    MY_ASSERT(tree);
    MY_ASSERT(tree_name);
    MY_ASSERT(func);
    MY_ASSERT(file);

    FILE * fp = NULL;

    char dot_file_name[64] = "";
    make_file_extension(dot_file_name, TREE_DUMP_FILE_NAME, ".dot");

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
                "    }\n"
                "    {rank = max;\n"
                "        info_node [label = \" %s[%p] \\n from %s \\n %s:%d \", color = green];\n"
                "    }\n", tree_name, tree, file, func, line);

    if (tree->root)
    {
        print_tree_nodes(tree->root, fp);
        print_tree_edges(tree->root, fp);

        fprintf(fp, "info_node -> node%p [style = invis];\n", tree->root);
    }

    fprintf(fp, "}");

    fclose(fp);

    static size_t dumps_count = 0;
    char png_dump_file_name[64] = "";
    char command_string[BUFFER_SIZE] = "";
    char extension_string[BUFFER_SIZE] = "";

    sprintf(extension_string, "%zd.png", dumps_count);
    make_file_extension(png_dump_file_name, TREE_DUMP_FILE_NAME, extension_string);
    sprintf(command_string, "dot %s -T png -o %s", dot_file_name, png_dump_file_name);
    system(command_string);

    dumps_count++;
}


static void print_tree_nodes(const TreeNode * node, FILE * fp)
{
    MY_ASSERT(node);
    MY_ASSERT(fp);

    fprintf(fp, "    node%p [ label = \"{[%p] ",
            node, node);

    switch (node->value.type)
    {
        case TREE_NODE_TYPES_NO_TYPE:
            fprintf(fp, "%lX", TRASH_VALUE);
            break;

        case TREE_NODE_TYPES_STRING:
            fprintf(fp, "%s", node->value.value.string);
            break;

        case TREE_NODE_TYPES_NUMBER:
            fprintf(fp, "%.2lf", node->value.value.number);
            break;

        default:
            MY_ASSERT(0 && "UNREACHABLE");
            break;
    }

    // fprintf(fp, "%lf", node->value);

    fprintf(fp, " | { <l> left[%p] | right[%p]  }}\" ]\n",
            node->left, node->right);

    if (node->right)
    {
        print_tree_nodes(node->right, fp);
    }

    if (node->left)
    {
        print_tree_nodes(node->left, fp);
    }
}


static void print_tree_edges(const TreeNode * node, FILE * fp)
{
    MY_ASSERT(node);

    if (node->left)
    {
        fprintf(fp, "    node%p:<l> -> node%p;\n", node, node->left);
        print_tree_edges(node->left, fp);
    }

    if (node->right)
    {
        fprintf(fp, "    node%p:<r> -> node%p;\n", node, node->right);
        print_tree_edges(node->right, fp);
    }
}


void tree_text_dump(const Tree * tree)
{
    MY_ASSERT(tree);

    printf("---------TREE_DUMP----------\n"
           "Tree[%p]:\n", tree);
    print_text_nodes(tree->root);
    printf("\r\tsize = %zd\n", tree->size);
    printf("----------------------------\n");

}


static void print_text_nodes(const TreeNode * main_node)
{
    MY_ASSERT(main_node);

    printf("\tNode[%p]\n", main_node);
    if (main_node->left)
    {
        printf("\r\tLeft:\n\t");
        print_text_nodes(main_node->left);
    }
    if (main_node->right)
    {
        printf("\r\tRight:\n\t");
        print_text_nodes(main_node->right);
    }

    return;
}


TError_t tree_copy_branch(Tree * dst_tree, TreeNode * dst_node, const TreeNode * src_node)
{
    MY_ASSERT(dst_tree);
    MY_ASSERT(dst_node);
    MY_ASSERT(src_node);

    TError_t errors = 0;

    dst_node->value = src_node->value;

    if (src_node->left)
    {
       errors |= tree_insert(dst_tree, dst_node, TREE_NODE_BRANCH_LEFT, TREE_NULL);
       tree_copy_branch(dst_tree, dst_node->left, src_node->left);
    }

    if (src_node->right)
    {
       errors |= tree_insert(dst_tree, dst_node, TREE_NODE_BRANCH_RIGHT, TREE_NULL);
       tree_copy_branch(dst_tree, dst_node->right, src_node->right);
    }

    return errors;
}
