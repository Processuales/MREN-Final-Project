#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    AVLNode *items[TREE_QUEUE_CAP];
    int front;
    int rear;
} NodeQueue;

static void nq_init(NodeQueue *q) {
    q->front = 0;
    q->rear = 0;
}

static int nq_is_empty(NodeQueue *q) {
    return q->front == q->rear;
}

static int nq_push(NodeQueue *q, AVLNode *node) {
    int next = (q->rear + 1) % TREE_QUEUE_CAP;

    if (next == q->front) {
        return 0;
    }

    q->items[q->rear] = node;
    q->rear = next;
    return 1;
}

static AVLNode *nq_pop(NodeQueue *q) {
    AVLNode *node;

    if (nq_is_empty(q)) {
        return NULL;
    }

    node = q->items[q->front];
    q->front = (q->front + 1) % TREE_QUEUE_CAP;
    return node;
}

static int nq_size(NodeQueue *q) {
    if (q->rear >= q->front) {
        return q->rear - q->front;
    }

    return TREE_QUEUE_CAP - q->front + q->rear;
}

static int height(AVLNode *node) {
    if (node == NULL) {
        return 0;
    }

    return node->height;
}

static int max_int(int a, int b) {
    if (a > b) {
        return a;
    }

    return b;
}

static AVLNode *new_node(int value) {
    AVLNode *node = (AVLNode *)malloc(sizeof(AVLNode));

    if (node == NULL) {
        return NULL;
    }

    node->key = value;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    return node;
}

static void update_height(AVLNode *node) {
    if (node == NULL) {
        return;
    }

    node->height = 1 + max_int(height(node->left), height(node->right));
}

static int get_balance(AVLNode *node) {
    if (node == NULL) {
        return 0;
    }

    return height(node->left) - height(node->right);
}

static AVLNode *rotate_right(AVLNode *y) {
    AVLNode *x = y->left;
    AVLNode *t2 = x->right;

    x->right = y;
    y->left = t2;

    update_height(y);
    update_height(x);
    return x;
}

static AVLNode *rotate_left(AVLNode *x) {
    AVLNode *y = x->right;
    AVLNode *t2 = y->left;

    y->left = x;
    x->right = t2;

    update_height(x);
    update_height(y);
    return y;
}

static AVLNode *insert_node(AVLNode *node, int value, char *note, int note_size, int *ok) {
    int balance;

    if (node == NULL) {
        AVLNode *created = new_node(value);

        if (created == NULL) {
            *ok = 0;
            snprintf(note, note_size, "Could not allocate AVL node");
        } else {
            snprintf(note, note_size, "Inserted %d as a new node", value);
        }

        return created;
    }

    if (value < node->key) {
        node->left = insert_node(node->left, value, note, note_size, ok);
    } else if (value > node->key) {
        node->right = insert_node(node->right, value, note, note_size, ok);
    } else {
        snprintf(note, note_size, "Value %d is already in the tree", value);
        return node;
    }

    if (!*ok) {
        return node;
    }

    update_height(node);
    balance = get_balance(node);

    if (balance > 1 && value < node->left->key) {
        snprintf(note, note_size, "LL imbalance at %d, rotate right", node->key);
        return rotate_right(node);
    }

    if (balance < -1 && value > node->right->key) {
        snprintf(note, note_size, "RR imbalance at %d, rotate left", node->key);
        return rotate_left(node);
    }

    if (balance > 1 && value > node->left->key) {
        snprintf(note, note_size, "LR imbalance at %d, rotate left then right", node->key);
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }

    if (balance < -1 && value < node->right->key) {
        snprintf(note, note_size, "RL imbalance at %d, rotate right then left", node->key);
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }

    snprintf(note, note_size, "Inserted %d, no rotation needed", value);
    return node;
}

void init_avl(AVLTree *tree) {
    tree->root = NULL;
}

int avl_insert_value(AVLTree *tree, int value, char *note, int note_size) {
    int ok = 1;
    tree->root = insert_node(tree->root, value, note, note_size, &ok);
    return ok;
}

int avl_search_value(AVLTree *tree, int value) {
    AVLNode *current = tree->root;

    while (current != NULL) {
        if (value == current->key) {
            return 1;
        }

        if (value < current->key) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    return 0;
}

void free_avl_nodes(AVLNode *node) {
    if (node == NULL) {
        return;
    }

    free_avl_nodes(node->left);
    free_avl_nodes(node->right);
    free(node);
}

void build_avl_frame(ProgramState *state, Structure *st, const char *operation, const char *note) {
    Frame *frame;
    char line[MAX_LINE_LEN];
    NodeQueue q;
    int level;
    int nodes_this_level;
    int i;
    int has_real_next_level;

    if (!st->watched || state->frame_count >= MAX_FRAMES) {
        return;
    }

    frame = &state->frames[state->frame_count];
    frame->step_number = state->frame_count + 1;

    strncpy(frame->structure_name, st->name, MAX_NAME_LEN - 1);
    frame->structure_name[MAX_NAME_LEN - 1] = '\0';

    strncpy(frame->operation, operation, sizeof(frame->operation) - 1);
    frame->operation[sizeof(frame->operation) - 1] = '\0';

    strncpy(frame->note, note, sizeof(frame->note) - 1);
    frame->note[sizeof(frame->note) - 1] = '\0';

    frame->line_count = 0;

    snprintf(line, sizeof(line), "AVL tree: %s", st->name);
    strncpy(frame->lines[frame->line_count++], line, MAX_LINE_LEN - 1);

    if (st->avl.root == NULL) {
        strncpy(frame->lines[frame->line_count++], "Level 0: .", MAX_LINE_LEN - 1);
        state->frame_count++;
        return;
    }

    nq_init(&q);
    nq_push(&q, st->avl.root);
    level = 0;

    while (!nq_is_empty(&q) && frame->line_count < MAX_FRAME_LINES) {
        char temp[MAX_LINE_LEN];
        temp[0] = '\0';

        snprintf(line, sizeof(line), "Level %d: ", level);
        strncat(temp, line, sizeof(temp) - strlen(temp) - 1);

        nodes_this_level = nq_size(&q);
        has_real_next_level = 0;

        for (i = 0; i < nodes_this_level; i++) {
            AVLNode *node = nq_pop(&q);

            if (node == NULL) {
                strncat(temp, ". ", sizeof(temp) - strlen(temp) - 1);
                nq_push(&q, NULL);
                nq_push(&q, NULL);
            } else {
                snprintf(line, sizeof(line), "%d ", node->key);
                strncat(temp, line, sizeof(temp) - strlen(temp) - 1);

                nq_push(&q, node->left);
                nq_push(&q, node->right);

                if (node->left != NULL || node->right != NULL) {
                    has_real_next_level = 1;
                }
            }
        }

        strncpy(frame->lines[frame->line_count], temp, MAX_LINE_LEN - 1);
        frame->lines[frame->line_count][MAX_LINE_LEN - 1] = '\0';
        frame->line_count++;

        level++;

        if (!has_real_next_level) {
            break;
        }
    }

    state->frame_count++;
}