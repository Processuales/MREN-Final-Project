#include "common.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void remove_bom(char *line) {
    unsigned char *p = (unsigned char *)line;

    if (p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF) {
        memmove(line, line + 3, strlen(line + 3) + 1);
    }
}


static void trim_newline(char *line) {
    size_t len = strlen(line);

    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[len - 1] = '\0';
        len--;
    }
}

static void trim_spaces(char *line) {
    int start = 0;
    int i;
    int len = (int)strlen(line);

    while (line[start] != '\0' && isspace((unsigned char)line[start])) {
        start++;
    }

    if (start > 0) {
        for (i = 0; line[start + i] != '\0'; i++) {
            line[i] = line[start + i];
        }
        line[i] = '\0';
    }

    len = (int)strlen(line);
    while (len > 0 && isspace((unsigned char)line[len - 1])) {
        line[len - 1] = '\0';
        len--;
    }
}

static int split_words(char *line, char words[][MAX_NAME_LEN], int max_words) {
    int count = 0;
    char *token = strtok(line, " \t");

    while (token != NULL && count < max_words) {
        strncpy(words[count], token, MAX_NAME_LEN - 1);
        words[count][MAX_NAME_LEN - 1] = '\0';
        count++;
        token = strtok(NULL, " \t");
    }

    return count;
}

void init_program_state(ProgramState *state) {
    int i;

    state->structure_count = 0;
    state->frame_count = 0;
    state->last_error[0] = '\0';
    state->last_peek_value = 0;
    state->last_search_found = 0;

    for (i = 0; i < MAX_STRUCTURES; i++) {
        state->structures[i].name[0] = '\0';
        state->structures[i].type = TYPE_NONE;
        state->structures[i].watched = 0;
    }
}

Structure *find_structure(ProgramState *state, const char *name) {
    int i;

    for (i = 0; i < state->structure_count; i++) {
        if (strcmp(state->structures[i].name, name) == 0) {
            return &state->structures[i];
        }
    }

    return NULL;
}

Structure *add_structure(ProgramState *state, const char *name, StructureType type) {
    Structure *st;

    if (state->structure_count >= MAX_STRUCTURES) {
        snprintf(state->last_error, sizeof(state->last_error), "Too many structures");
        return NULL;
    }

    if (find_structure(state, name) != NULL) {
        snprintf(state->last_error, sizeof(state->last_error), "Structure %s already exists", name);
        return NULL;
    }

    st = &state->structures[state->structure_count];
    state->structure_count++;

    strncpy(st->name, name, MAX_NAME_LEN - 1);
    st->name[MAX_NAME_LEN - 1] = '\0';
    st->type = type;
    st->watched = 0;

    if (type == TYPE_STACK) {
        init_stack(&st->stack);
    } else if (type == TYPE_QUEUE) {
        init_queue(&st->queue);
    } else if (type == TYPE_DEQUE) {
        init_deque(&st->deque);
    } else if (type == TYPE_AVL) {
        init_avl(&st->avl);
    }

    return st;
}

void free_all_trees(ProgramState *state) {
    int i;

    for (i = 0; i < state->structure_count; i++) {
        if (state->structures[i].type == TYPE_AVL) {
            free_avl_nodes(state->structures[i].avl.root);
            state->structures[i].avl.root = NULL;
        }
    }
}

static int parse_int_value(const char *text, int *out_value) {
    char *end_ptr;
    long value = strtol(text, &end_ptr, 10);

    if (*text == '\0' || *end_ptr != '\0') {
        return 0;
    }

    *out_value = (int)value;
    return 1;
}

static int handle_create(ProgramState *state, char words[][MAX_NAME_LEN], int count) {
    StructureType type;
    Structure *st;

    if (count != 3) {
        snprintf(state->last_error, sizeof(state->last_error), "create needs 2 arguments");
        return 0;
    }

    type = TYPE_NONE;

    if (strcmp(words[1], "stack") == 0) {
        type = TYPE_STACK;
    } else if (strcmp(words[1], "queue") == 0) {
        type = TYPE_QUEUE;
    } else if (strcmp(words[1], "deque") == 0) {
        type = TYPE_DEQUE;
    } else if (strcmp(words[1], "avl") == 0) {
        type = TYPE_AVL;
    }

    if (type == TYPE_NONE) {
        snprintf(state->last_error, sizeof(state->last_error), "Unknown structure type %s", words[1]);
        return 0;
    }

    st = add_structure(state, words[2], type);
    if (st == NULL) {
        return 0;
    }

    return 1;
}

static int handle_watch(ProgramState *state, char words[][MAX_NAME_LEN], int count) {
    Structure *st;

    if (count != 2) {
        snprintf(state->last_error, sizeof(state->last_error), "watch needs 1 argument");
        return 0;
    }

    st = find_structure(state, words[1]);
    if (st == NULL) {
        snprintf(state->last_error, sizeof(state->last_error), "Could not find structure %s", words[1]);
        return 0;
    }

    st->watched = 1;
    return 1;
}

static int handle_push(ProgramState *state, char words[][MAX_NAME_LEN], int count) {
    Structure *st;
    int value;
    char note[120];

    if (count != 3) {
        snprintf(state->last_error, sizeof(state->last_error), "push needs 2 arguments");
        return 0;
    }

    st = find_structure(state, words[1]);
    if (st == NULL || st->type != TYPE_STACK) {
        snprintf(state->last_error, sizeof(state->last_error), "%s is not a stack", words[1]);
        return 0;
    }

    if (!parse_int_value(words[2], &value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Bad integer %s", words[2]);
        return 0;
    }

    if (!push_stack(&st->stack, value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Stack %s is full", st->name);
        return 0;
    }

    snprintf(note, sizeof(note), "Pushed %d", value);
    build_stack_frame(state, st, "push", note);
    return 1;
}

static int handle_pop(ProgramState *state, char words[][MAX_NAME_LEN], int count) {
    Structure *st;
    int value;
    char note[120];

    if (count != 2) {
        snprintf(state->last_error, sizeof(state->last_error), "pop needs 1 argument");
        return 0;
    }

    st = find_structure(state, words[1]);
    if (st == NULL || st->type != TYPE_STACK) {
        snprintf(state->last_error, sizeof(state->last_error), "%s is not a stack", words[1]);
        return 0;
    }

    if (!pop_stack(&st->stack, &value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Stack %s is empty", st->name);
        return 0;
    }

    snprintf(note, sizeof(note), "Popped %d", value);
    build_stack_frame(state, st, "pop", note);
    return 1;
}

static int handle_peek(ProgramState *state, char words[][MAX_NAME_LEN], int count) {
    Structure *st;
    int value;
    char note[120];

    if (count != 2) {
        snprintf(state->last_error, sizeof(state->last_error), "peek needs 1 argument");
        return 0;
    }

    st = find_structure(state, words[1]);
    if (st == NULL || st->type != TYPE_STACK) {
        snprintf(state->last_error, sizeof(state->last_error), "%s is not a stack", words[1]);
        return 0;
    }

    if (!peek_stack(&st->stack, &value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Stack %s is empty", st->name);
        return 0;
    }

    state->last_peek_value = value;
    snprintf(note, sizeof(note), "Peeked top value %d", value);
    build_stack_frame(state, st, "peek", note);
    return 1;
}

static int handle_enqueue(ProgramState *state, char words[][MAX_NAME_LEN], int count) {
    Structure *st;
    int value;
    char note[120];

    if (count != 3) {
        snprintf(state->last_error, sizeof(state->last_error), "enqueue needs 2 arguments");
        return 0;
    }

    st = find_structure(state, words[1]);
    if (st == NULL || st->type != TYPE_QUEUE) {
        snprintf(state->last_error, sizeof(state->last_error), "%s is not a queue", words[1]);
        return 0;
    }

    if (!parse_int_value(words[2], &value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Bad integer %s", words[2]);
        return 0;
    }

    if (!enqueue_queue(&st->queue, value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Queue %s is full", st->name);
        return 0;
    }

    snprintf(note, sizeof(note), "Enqueued %d", value);
    build_queue_frame(state, st, "enqueue", note);
    return 1;
}

static int handle_dequeue(ProgramState *state, char words[][MAX_NAME_LEN], int count) {
    Structure *st;
    int value;
    char note[120];

    if (count != 2) {
        snprintf(state->last_error, sizeof(state->last_error), "dequeue needs 1 argument");
        return 0;
    }

    st = find_structure(state, words[1]);
    if (st == NULL || st->type != TYPE_QUEUE) {
        snprintf(state->last_error, sizeof(state->last_error), "%s is not a queue", words[1]);
        return 0;
    }

    if (!dequeue_queue(&st->queue, &value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Queue %s is empty", st->name);
        return 0;
    }

    snprintf(note, sizeof(note), "Dequeued %d", value);
    build_queue_frame(state, st, "dequeue", note);
    return 1;
}

static int handle_front(ProgramState *state, char words[][MAX_NAME_LEN], int count) {
    Structure *st;
    int value;
    char note[120];

    if (count != 2) {
        snprintf(state->last_error, sizeof(state->last_error), "front needs 1 argument");
        return 0;
    }

    st = find_structure(state, words[1]);
    if (st == NULL || st->type != TYPE_QUEUE) {
        snprintf(state->last_error, sizeof(state->last_error), "%s is not a queue", words[1]);
        return 0;
    }

    if (!front_queue(&st->queue, &value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Queue %s is empty", st->name);
        return 0;
    }

    snprintf(note, sizeof(note), "Front value is %d", value);
    build_queue_frame(state, st, "front", note);
    return 1;
}

static int handle_push_front(ProgramState *state, char words[][MAX_NAME_LEN], int count) {
    Structure *st;
    int value;
    char note[120];

    if (count != 3) {
        snprintf(state->last_error, sizeof(state->last_error), "push_front needs 2 arguments");
        return 0;
    }

    st = find_structure(state, words[1]);
    if (st == NULL || st->type != TYPE_DEQUE) {
        snprintf(state->last_error, sizeof(state->last_error), "%s is not a deque", words[1]);
        return 0;
    }

    if (!parse_int_value(words[2], &value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Bad integer %s", words[2]);
        return 0;
    }

    if (!push_front_deque(&st->deque, value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Deque %s is full", st->name);
        return 0;
    }

    snprintf(note, sizeof(note), "Pushed %d to front", value);
    build_deque_frame(state, st, "push_front", note);
    return 1;
}

static int handle_push_back(ProgramState *state, char words[][MAX_NAME_LEN], int count) {
    Structure *st;
    int value;
    char note[120];

    if (count != 3) {
        snprintf(state->last_error, sizeof(state->last_error), "push_back needs 2 arguments");
        return 0;
    }

    st = find_structure(state, words[1]);
    if (st == NULL || st->type != TYPE_DEQUE) {
        snprintf(state->last_error, sizeof(state->last_error), "%s is not a deque", words[1]);
        return 0;
    }

    if (!parse_int_value(words[2], &value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Bad integer %s", words[2]);
        return 0;
    }

    if (!push_back_deque(&st->deque, value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Deque %s is full", st->name);
        return 0;
    }

    snprintf(note, sizeof(note), "Pushed %d to back", value);
    build_deque_frame(state, st, "push_back", note);
    return 1;
}

static int handle_pop_front(ProgramState *state, char words[][MAX_NAME_LEN], int count) {
    Structure *st;
    int value;
    char note[120];

    if (count != 2) {
        snprintf(state->last_error, sizeof(state->last_error), "pop_front needs 1 argument");
        return 0;
    }

    st = find_structure(state, words[1]);
    if (st == NULL || st->type != TYPE_DEQUE) {
        snprintf(state->last_error, sizeof(state->last_error), "%s is not a deque", words[1]);
        return 0;
    }

    if (!pop_front_deque(&st->deque, &value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Deque %s is empty", st->name);
        return 0;
    }

    snprintf(note, sizeof(note), "Popped %d from front", value);
    build_deque_frame(state, st, "pop_front", note);
    return 1;
}

static int handle_pop_back(ProgramState *state, char words[][MAX_NAME_LEN], int count) {
    Structure *st;
    int value;
    char note[120];

    if (count != 2) {
        snprintf(state->last_error, sizeof(state->last_error), "pop_back needs 1 argument");
        return 0;
    }

    st = find_structure(state, words[1]);
    if (st == NULL || st->type != TYPE_DEQUE) {
        snprintf(state->last_error, sizeof(state->last_error), "%s is not a deque", words[1]);
        return 0;
    }

    if (!pop_back_deque(&st->deque, &value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Deque %s is empty", st->name);
        return 0;
    }

    snprintf(note, sizeof(note), "Popped %d from back", value);
    build_deque_frame(state, st, "pop_back", note);
    return 1;
}

static int handle_insert(ProgramState *state, char words[][MAX_NAME_LEN], int count) {
    Structure *st;
    int value;
    char note[120];

    if (count != 3) {
        snprintf(state->last_error, sizeof(state->last_error), "insert needs 2 arguments");
        return 0;
    }

    st = find_structure(state, words[1]);
    if (st == NULL || st->type != TYPE_AVL) {
        snprintf(state->last_error, sizeof(state->last_error), "%s is not an avl tree", words[1]);
        return 0;
    }

    if (!parse_int_value(words[2], &value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Bad integer %s", words[2]);
        return 0;
    }

    if (!avl_insert_value(&st->avl, value, note, sizeof(note))) {
        snprintf(state->last_error, sizeof(state->last_error), "Could not insert into AVL tree");
        return 0;
    }

    build_avl_frame(state, st, "insert", note);
    return 1;
}

static int handle_search(ProgramState *state, char words[][MAX_NAME_LEN], int count) {
    Structure *st;
    int value;
    char note[120];

    if (count != 3) {
        snprintf(state->last_error, sizeof(state->last_error), "search needs 2 arguments");
        return 0;
    }

    st = find_structure(state, words[1]);
    if (st == NULL || st->type != TYPE_AVL) {
        snprintf(state->last_error, sizeof(state->last_error), "%s is not an avl tree", words[1]);
        return 0;
    }

    if (!parse_int_value(words[2], &value)) {
        snprintf(state->last_error, sizeof(state->last_error), "Bad integer %s", words[2]);
        return 0;
    }

    state->last_search_found = avl_search_value(&st->avl, value);

    if (state->last_search_found) {
        snprintf(note, sizeof(note), "Found value %d", value);
    } else {
        snprintf(note, sizeof(note), "Did not find value %d", value);
    }

    build_avl_frame(state, st, "search", note);
    return 1;
}

int parse_file_and_build_frames(ProgramState *state, const char *path) {
    FILE *file;
    char line[INPUT_LINE_LEN];
    int line_number = 0;

    file = fopen(path, "r");
    if (file == NULL) {
        snprintf(state->last_error, sizeof(state->last_error), "Could not open file: %s", path);
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char original_line[INPUT_LINE_LEN];
        char words[6][MAX_NAME_LEN];
        int word_count;

        line_number++;

        strncpy(original_line, line, sizeof(original_line) - 1);
        original_line[sizeof(original_line) - 1] = '\0';

        remove_bom(line);
        trim_newline(line);
        trim_spaces(line);

        if (line[0] == '\0') {
            continue;
        }
        if (line[0] == '#') {
            continue;
        }

        word_count = split_words(line, words, 6);
        if (word_count <= 0) {
            continue;
        }

        if (strcmp(words[0], "create") == 0) {
            if (!handle_create(state, words, word_count)) {
                break;
            }
        } else if (strcmp(words[0], "watch") == 0) {
            if (!handle_watch(state, words, word_count)) {
                break;
            }
        } else if (strcmp(words[0], "push") == 0) {
            if (!handle_push(state, words, word_count)) {
                break;
            }
        } else if (strcmp(words[0], "pop") == 0) {
            if (!handle_pop(state, words, word_count)) {
                break;
            }
        } else if (strcmp(words[0], "peek") == 0) {
            if (!handle_peek(state, words, word_count)) {
                break;
            }
        } else if (strcmp(words[0], "enqueue") == 0) {
            if (!handle_enqueue(state, words, word_count)) {
                break;
            }
        } else if (strcmp(words[0], "dequeue") == 0) {
            if (!handle_dequeue(state, words, word_count)) {
                break;
            }
        } else if (strcmp(words[0], "front") == 0) {
            if (!handle_front(state, words, word_count)) {
                break;
            }
        } else if (strcmp(words[0], "push_front") == 0) {
            if (!handle_push_front(state, words, word_count)) {
                break;
            }
        } else if (strcmp(words[0], "push_back") == 0) {
            if (!handle_push_back(state, words, word_count)) {
                break;
            }
        } else if (strcmp(words[0], "pop_front") == 0) {
            if (!handle_pop_front(state, words, word_count)) {
                break;
            }
        } else if (strcmp(words[0], "pop_back") == 0) {
            if (!handle_pop_back(state, words, word_count)) {
                break;
            }
        } else if (strcmp(words[0], "insert") == 0) {
            if (!handle_insert(state, words, word_count)) {
                break;
            }
        } else if (strcmp(words[0], "search") == 0) {
            if (!handle_search(state, words, word_count)) {
                break;
            }
        } else {
            snprintf(state->last_error, sizeof(state->last_error), "Line %d: unknown command", line_number);
            break;
        }
    }

    fclose(file);

    if (state->last_error[0] != '\0') {
        return 0;
    }

    return 1;
}

void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}