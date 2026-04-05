#ifndef DATA_STRUCTURE_VIEWER_COMMON_H
#define DATA_STRUCTURE_VIEWER_COMMON_H

// I wanna avoid dynamic memory allocations and instead use fixed-sized arrays cuz its easier
#define MAX_STRUCTURES 20
#define MAX_NAME_LEN 32
#define INPUT_LINE_LEN 256

#define LINEAR_CAP 50
#define MAX_FRAMES 400
#define MAX_FRAME_LINES 30
#define MAX_FRAME_TEXT 120
#define MAX_LINE_TEXT 120

typedef struct AVLNode {
    int key;
    int height;
    struct AVLNode *left;
    struct AVLNode *right;
} AVLNode;

typedef struct {
    char name[MAX_NAME_LEN];
    int watched;
    int data[LINEAR_CAP];
    int top;
} StackInfo;

typedef struct {
    char name[MAX_NAME_LEN];
    int watched;
    int data[LINEAR_CAP];
    int front;
    int rear;
    int size;
} QueueInfo;

typedef struct {
    char name[MAX_NAME_LEN];
    int watched;
    int data[LINEAR_CAP];
    int front;
    int rear;
    int size;
} DequeInfo;

typedef struct {
    char name[MAX_NAME_LEN];
    int watched;
    AVLNode *root;
} AVLInfo;

// Every frame is one full snapshot that can be shown in terminal
typedef struct {
    int step_number;
    char structure_name[MAX_NAME_LEN];
    char operation[40];
    char note[MAX_FRAME_TEXT];
    char lines[MAX_FRAME_LINES][MAX_LINE_TEXT];
    int line_count;
} Frame;

typedef struct {
    // Stacks
    StackInfo stacks[MAX_STRUCTURES];
    int stack_count;

    // Queues
    QueueInfo queues[MAX_STRUCTURES];
    int queue_count;

    // Deques
    DequeInfo deques[MAX_STRUCTURES];
    int deque_count;

    // AVL trees
    AVLInfo avls[MAX_STRUCTURES];
    int avl_count;

    // Frames
    Frame frames[MAX_FRAMES];
    int frame_count;

    char last_error[200];
} ProgramState;

// ========= PARSER / STATE =========

void init_program_state(ProgramState *state);
int parse_file(ProgramState *state, const char *path);
void free_all_trees(ProgramState *state);

// ========= FRAME HELPERS =========

Frame *start_frame(ProgramState *state, const char *structure_name, const char *operation, const char *note);

void add_line_to_frame(Frame *frame, const char *text);

// ========= LINEAR STRUCTURES =========

void init_stack(StackInfo *stack);
int push_stack(StackInfo *stack, int value);
int pop_stack(StackInfo *stack, int *out_value);
int peek_stack(StackInfo *stack, int *out_value);
void build_stack_frame(ProgramState *state, StackInfo *stack, const char *operation, const char *note);

void init_queue(QueueInfo *queue);
int enqueue_queue(QueueInfo *queue, int value);
int dequeue_queue(QueueInfo *queue, int *out_value);
int front_queue(QueueInfo *queue, int *out_value);
void build_queue_frame(ProgramState *state, QueueInfo *queue, const char *operation, const char *note);

void init_deque(DequeInfo *deque);
int push_front_deque(DequeInfo *deque, int value);
int push_back_deque(DequeInfo *deque, int value);
int pop_front_deque(DequeInfo *deque, int *out_value);
int pop_back_deque(DequeInfo *deque, int *out_value);
void build_deque_frame(ProgramState *state, DequeInfo *deque, const char *operation, const char *note);

// ========= TREE STRUCTURES =========

void init_avl(AVLInfo *tree);
int avl_insert_value(AVLInfo *tree, int value, char *note, int note_size);
int avl_search_value(AVLInfo *tree, int value);
void build_avl_frame(ProgramState *state, AVLInfo *tree, const char *operation, const char *note);
void free_avl_nodes(AVLNode *node);

#endif // DATA_STRUCTURE_VIEWER_COMMON_H