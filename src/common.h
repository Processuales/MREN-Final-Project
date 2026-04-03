#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

#define MAX_STRUCTURES 20
#define MAX_NAME_LEN 32
#define MAX_FRAMES 400
#define MAX_FRAME_LINES 40
#define MAX_LINE_LEN 120
#define STACK_CAP 100
#define QUEUE_CAP 100
#define DEQUE_CAP 100
#define TREE_QUEUE_CAP 256
#define INPUT_LINE_LEN 256

typedef struct {
    int data[STACK_CAP];
    int top;
} Stack;

typedef struct {
    int data[QUEUE_CAP];
    int front;
    int rear;
    int size;
} Queue;

typedef struct {
    int data[DEQUE_CAP];
    int front;
    int rear;
    int size;
} Deque;

typedef struct AVLNode {
    int key;
    int height;
    struct AVLNode *left;
    struct AVLNode *right;
} AVLNode;

typedef struct {
    AVLNode *root;
} AVLTree;

typedef enum {
    TYPE_NONE = 0,
    TYPE_STACK,
    TYPE_QUEUE,
    TYPE_DEQUE,
    TYPE_AVL
} StructureType;

typedef struct {
    char name[MAX_NAME_LEN];
    StructureType type;
    int watched;
    Stack stack;
    Queue queue;
    Deque deque;
    AVLTree avl;
} Structure;

typedef struct {
    int step_number;
    char structure_name[MAX_NAME_LEN];
    char operation[80];
    char note[120];
    char lines[MAX_FRAME_LINES][MAX_LINE_LEN];
    int line_count;
} Frame;

typedef struct {
    Structure structures[MAX_STRUCTURES];
    int structure_count;
    Frame frames[MAX_FRAMES];
    int frame_count;
    char last_error[200];
    int last_peek_value;
    int last_search_found;
} ProgramState;

void init_program_state(ProgramState *state);
Structure *find_structure(ProgramState *state, const char *name);
Structure *add_structure(ProgramState *state, const char *name, StructureType type);
void clear_screen(void);
void free_all_trees(ProgramState *state);

int parse_file_and_build_frames(ProgramState *state, const char *path);

void init_stack(Stack *s);
int push_stack(Stack *s, int value);
int pop_stack(Stack *s, int *out_value);
int peek_stack(Stack *s, int *out_value);
void build_stack_frame(ProgramState *state, Structure *st, const char *operation, const char *note);

void init_queue(Queue *q);
int enqueue_queue(Queue *q, int value);
int dequeue_queue(Queue *q, int *out_value);
int front_queue(Queue *q, int *out_value);
void build_queue_frame(ProgramState *state, Structure *st, const char *operation, const char *note);

void init_deque(Deque *d);
int push_front_deque(Deque *d, int value);
int push_back_deque(Deque *d, int value);
int pop_front_deque(Deque *d, int *out_value);
int pop_back_deque(Deque *d, int *out_value);
void build_deque_frame(ProgramState *state, Structure *st, const char *operation, const char *note);

void init_avl(AVLTree *tree);
int avl_insert_value(AVLTree *tree, int value, char *note, int note_size);
int avl_search_value(AVLTree *tree, int value);
void free_avl_nodes(AVLNode *node);
void build_avl_frame(ProgramState *state, Structure *st, const char *operation, const char *note);

#endif