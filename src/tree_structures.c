#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TREE_QUEUE_CAP 256

typedef struct {
    AVLNode *items[TREE_QUEUE_CAP];
    int front;
    int rear;
} NodeQueue;

static void nq_init(NodeQueue *queue)
{
    queue->front = 0;
    queue->rear = 0;
}

static int nq_is_empty(NodeQueue *queue)
{
    return queue->front == queue->rear;
}

static int nq_size(NodeQueue *queue)
{
    if (queue->rear >= queue->front)
    {
        return queue->rear - queue->front;
    }

    return TREE_QUEUE_CAP - queue->front + queue->rear;
}

static void nq_push(NodeQueue *queue, AVLNode *node)
{
    int next = (queue->rear + 1) % TREE_QUEUE_CAP;

    if (next == queue->front)
    {
        return;
    }

    queue->items[queue->rear] = node;
    queue->rear = next;
}

static AVLNode *nq_pop(NodeQueue *queue)
{
    AVLNode *node;

    if (nq_is_empty(queue))
    {
        return NULL;
    }

    node = queue->items[queue->front];
    queue->front = (queue->front + 1) % TREE_QUEUE_CAP;
    return node;
}

static int height(AVLNode *node)
{
    if (node == NULL)
    {
        return 0;
    }

    return node->height;
}

static int max_int(int a, int b)
{
    if (a > b)
    {
        return a;
    }

    return b;
}

static AVLNode *create_node(int value)
{
    AVLNode *node = (AVLNode *)malloc(sizeof(AVLNode));

    if (node == NULL)
    {
        return NULL;
    }

    node->key = value;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;

    return node;
}

static void update_height(AVLNode *node)
{
    if (node == NULL)
    {
        return;
    }

    node->height = 1 + max_int(height(node->left), height(node->right));
}

static int get_balance(AVLNode *node)
{
    if (node == NULL)
    {
        return 0;
    }

    return height(node->left) - height(node->right);
}

static AVLNode *rotate_right(AVLNode *y)
{
    AVLNode *x = y->left;
    AVLNode *t2 = x->right;

    x->right = y;
    y->left = t2;

    update_height(y);
    update_height(x);

    return x;
}

static AVLNode *rotate_left(AVLNode *x)
{
    AVLNode *y = x->right;
    AVLNode *t2 = y->left;

    y->left = x;
    x->right = t2;

    update_height(x);
    update_height(y);

    return y;
}

static AVLNode *insert_node(AVLNode *node,
                            int value,
                            char *note,
                            int note_size,
                            int *ok)
{
    int balance;

    if (node == NULL)
    {
        AVLNode *new_node = create_node(value);

        if (new_node == NULL)
        {
            *ok = 0;
            snprintf(note, note_size, "Could not allocate AVL node");
            return NULL;
        }

        snprintf(note, note_size, "Inserted %d as a new node", value);
        return new_node;
    }

    if (value < node->key)
    {
        node->left = insert_node(node->left, value, note, note_size, ok);
    }
    else if (value > node->key)
    {
        node->right = insert_node(node->right, value, note, note_size, ok);
    }
    else
    {
        snprintf(note, note_size, "Value %d is already in the tree", value);
        return node;
    }

    if (!*ok)
    {
        return node;
    }

    update_height(node);
    balance = get_balance(node);

    if (balance > 1 && value < node->left->key)
    {
        snprintf(note, note_size, "LL imbalance at %d, rotate right", node->key);
        return rotate_right(node);
    }

    if (balance < -1 && value > node->right->key)
    {
        snprintf(note, note_size, "RR imbalance at %d, rotate left", node->key);
        return rotate_left(node);
    }

    if (balance > 1 && value > node->left->key)
    {
        snprintf(note, note_size, "LR imbalance at %d, rotate left then right", node->key);
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }

    if (balance < -1 && value < node->right->key)
    {
        snprintf(note, note_size, "RL imbalance at %d, rotate right then left", node->key);
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }

    snprintf(note, note_size, "Inserted %d, no rotation needed", value);
    return node;
}

void init_avl(AVLInfo *tree)
{
    tree->root = NULL;
}

int avl_insert_value(AVLInfo *tree, int value, char *note, int note_size)
{
    int ok = 1;

    tree->root = insert_node(tree->root, value, note, note_size, &ok);

    return ok;
}

int avl_search_value(AVLInfo *tree, int value)
{
    AVLNode *current = tree->root;

    while (current != NULL)
    {
        if (value == current->key)
        {
            return 1;
        }

        if (value < current->key)
        {
            current = current->left;
        }
        else
        {
            current = current->right;
        }
    }

    return 0;
}

void build_avl_frame(ProgramState *state, AVLInfo *tree, const char *operation, const char *note)
{
    Frame *frame;
    NodeQueue queue;
    char line[MAX_LINE_TEXT];
    int level;

    if (!tree->watched)
    {
        return;
    }

    frame = start_frame(state, tree->name, operation, note);

    if (frame == NULL)
    {
        return;
    }

    add_line_to_frame(frame, "AVL tree levels:");

    if (tree->root == NULL)
    {
        add_line_to_frame(frame, "Level 0: .");
        return;
    }

    nq_init(&queue);
    nq_push(&queue, tree->root);
    level = 0;

    while (!nq_is_empty(&queue))
    {
        int nodes_this_level;
        int i;
        int has_real_next_level;
        char temp[MAX_LINE_TEXT];

        nodes_this_level = nq_size(&queue);
        has_real_next_level = 0;

        snprintf(temp, sizeof(temp), "Level %d: ", level);

        for (i = 0; i < nodes_this_level; i++)
        {
            AVLNode *node = nq_pop(&queue);

            if (node == NULL)
            {
                strncat(temp, ". ", sizeof(temp) - strlen(temp) - 1);
            }
            else
            {
                snprintf(line, sizeof(line), "%d ", node->key);
                strncat(temp, line, sizeof(temp) - strlen(temp) - 1);

                nq_push(&queue, node->left);
                nq_push(&queue, node->right);

                if (node->left != NULL || node->right != NULL)
                {
                    has_real_next_level = 1;
                }
            }
        }

        add_line_to_frame(frame, temp);
        level++;

        // Stop after the last level that leads to real children
        if (!has_real_next_level)
        {
            break;
        }
    }
}

void free_avl_nodes(AVLNode *node)
{
    if (node == NULL)
    {
        return;
    }

    free_avl_nodes(node->left);
    free_avl_nodes(node->right);
    free(node);
}