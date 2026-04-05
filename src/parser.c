#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_program_state(ProgramState *state)
{
    int i;

    state->stack_count = 0;
    state->queue_count = 0;
    state->deque_count = 0;
    state->avl_count = 0;
    state->frame_count = 0;
    state->last_error[0] = '\0';

    // Start every structure as not watched
    for (i = 0; i < MAX_STRUCTURES; i++)
    {
        state->stacks[i].name[0] = '\0';
        state->stacks[i].watched = 0;
        state->stacks[i].top = -1;

        state->queues[i].name[0] = '\0';
        state->queues[i].watched = 0;
        state->queues[i].front = 0;
        state->queues[i].rear = 0;
        state->queues[i].size = 0;

        state->deques[i].name[0] = '\0';
        state->deques[i].watched = 0;
        state->deques[i].front = 0;
        state->deques[i].rear = 0;
        state->deques[i].size = 0;

        state->avls[i].name[0] = '\0';
        state->avls[i].watched = 0;
        state->avls[i].root = NULL;
    }
}

// ========= FRAME HELPERS =========

Frame *start_frame(ProgramState *state,
                   const char *structure_name,
                   const char *operation,
                   const char *note)
{
    Frame *frame;

    // If we run out of frame space, just stop adding more
    if (state->frame_count >= MAX_FRAMES)
    {
        return NULL;
    }

    frame = &state->frames[state->frame_count];

    frame->step_number = state->frame_count + 1;

    strncpy(frame->structure_name, structure_name, MAX_NAME_LEN - 1);
    frame->structure_name[MAX_NAME_LEN - 1] = '\0';

    strncpy(frame->operation, operation, sizeof(frame->operation) - 1);
    frame->operation[sizeof(frame->operation) - 1] = '\0';

    strncpy(frame->note, note, sizeof(frame->note) - 1);
    frame->note[sizeof(frame->note) - 1] = '\0';

    frame->line_count = 0;

    state->frame_count++;

    return frame;
}

void add_line_to_frame(Frame *frame, const char *text)
{
    if (frame == NULL)
    {
        return;
    }

    if (frame->line_count >= MAX_FRAME_LINES)
    {
        return;
    }

    strncpy(frame->lines[frame->line_count], text, MAX_LINE_TEXT - 1);
    frame->lines[frame->line_count][MAX_LINE_TEXT - 1] = '\0';
    frame->line_count++;
}

// ========= LOOKUP HELPERS =========

int name_exists_anywhere(ProgramState *state, const char *name)
{
    int i;

    for (i = 0; i < state->stack_count; i++)
    {
        if (strcmp(state->stacks[i].name, name) == 0)
            return 1;
    }

    for (i = 0; i < state->queue_count; i++)
    {
        if (strcmp(state->queues[i].name, name) == 0)
            return 1;
    }

    for (i = 0; i < state->deque_count; i++)
    {
        if (strcmp(state->deques[i].name, name) == 0)
            return 1;
    }

    for (i = 0; i < state->avl_count; i++)
    {
        if (strcmp(state->avls[i].name, name) == 0)
            return 1;
    }

    return 0;
}

StackInfo *find_stack(ProgramState *state, const char *name)
{
    int i;

    for (i = 0; i < state->stack_count; i++)
    {
        if (strcmp(state->stacks[i].name, name) == 0)
        {
            return &state->stacks[i];
        }
    }

    return NULL;
}

QueueInfo *find_queue(ProgramState *state, const char *name)
{
    int i;

    for (i = 0; i < state->queue_count; i++)
    {
        if (strcmp(state->queues[i].name, name) == 0)
        {
            return &state->queues[i];
        }
    }

    return NULL;
}

DequeInfo *find_deque(ProgramState *state, const char *name)
{
    int i;

    for (i = 0; i < state->deque_count; i++)
    {
        if (strcmp(state->deques[i].name, name) == 0)
        {
            return &state->deques[i];
        }
    }

    return NULL;
}

AVLInfo *find_avl(ProgramState *state, const char *name)
{
    int i;

    for (i = 0; i < state->avl_count; i++)
    {
        if (strcmp(state->avls[i].name, name) == 0)
        {
            return &state->avls[i];
        }
    }

    return NULL;
}

// Transform a string into an int
int parse_int_value(const char *text, int *out_value)
{
    char *end_ptr;
    long value = strtol(text, &end_ptr, 10);

    // checks that input is actually an integer
    if (*text == '\0' || *end_ptr != '\0')
    {
        return 0;
    }

    *out_value = (int) value;
    return 1;
}

// ========= CREATE HELPERS =========

int add_stack(ProgramState *state, const char *name)
{
    Frame *frame;

    if (state->stack_count >= MAX_STRUCTURES)
    {
        snprintf(state->last_error,
                 sizeof(state->last_error),
                 "Too many stacks");
        return 0;
    }

    if (name_exists_anywhere(state, name))
    {
        snprintf(state->last_error,
                 sizeof(state->last_error),
                 "Structure %s already exists",
                 name);
        return 0;
    }

    strncpy(state->stacks[state->stack_count].name, name, MAX_NAME_LEN - 1);
    state->stacks[state->stack_count].name[MAX_NAME_LEN - 1] = '\0';
    state->stacks[state->stack_count].watched = 0;
    init_stack(&state->stacks[state->stack_count]);
    state->stack_count++;

    frame = start_frame(state, name, "create", "Created stack");
    add_line_to_frame(frame, "Type: stack");
    add_line_to_frame(frame, "Watched: no");

    return 1;
}

int add_queue(ProgramState *state, const char *name)
{
    Frame *frame;

    if (state->queue_count >= MAX_STRUCTURES)
    {
        snprintf(state->last_error,
                 sizeof(state->last_error),
                 "Too many queues");
        return 0;
    }

    if (name_exists_anywhere(state, name))
    {
        snprintf(state->last_error,
                 sizeof(state->last_error),
                 "Structure %s already exists",
                 name);
        return 0;
    }

    strncpy(state->queues[state->queue_count].name, name, MAX_NAME_LEN - 1);
    state->queues[state->queue_count].name[MAX_NAME_LEN - 1] = '\0';
    state->queues[state->queue_count].watched = 0;
    init_queue(&state->queues[state->queue_count]);
    state->queue_count++;

    frame = start_frame(state, name, "create", "Created queue");
    add_line_to_frame(frame, "Type: queue");
    add_line_to_frame(frame, "Watched: no");

    return 1;
}

int add_deque(ProgramState *state, const char *name)
{
    Frame *frame;

    if (state->deque_count >= MAX_STRUCTURES)
    {
        snprintf(state->last_error,
                 sizeof(state->last_error),
                 "Too many deques");
        return 0;
    }

    if (name_exists_anywhere(state, name))
    {
        snprintf(state->last_error,
                 sizeof(state->last_error),
                 "Structure %s already exists",
                 name);
        return 0;
    }

    strncpy(state->deques[state->deque_count].name, name, MAX_NAME_LEN - 1);
    state->deques[state->deque_count].name[MAX_NAME_LEN - 1] = '\0';
    state->deques[state->deque_count].watched = 0;
    init_deque(&state->deques[state->deque_count]);
    state->deque_count++;

    frame = start_frame(state, name, "create", "Created deque");
    add_line_to_frame(frame, "Type: deque");
    add_line_to_frame(frame, "Watched: no");

    return 1;
}

int add_avl(ProgramState *state, const char *name)
{
    Frame *frame;

    if (state->avl_count >= MAX_STRUCTURES)
    {
        snprintf(state->last_error,
                 sizeof(state->last_error),
                 "Too many avl trees");
        return 0;
    }

    if (name_exists_anywhere(state, name))
    {
        snprintf(state->last_error,
                 sizeof(state->last_error),
                 "Structure %s already exists",
                 name);
        return 0;
    }

    strncpy(state->avls[state->avl_count].name, name, MAX_NAME_LEN - 1);
    state->avls[state->avl_count].name[MAX_NAME_LEN - 1] = '\0';
    state->avls[state->avl_count].watched = 0;
    init_avl(&state->avls[state->avl_count]);
    state->avl_count++;

    frame = start_frame(state, name, "create", "Created avl tree");
    add_line_to_frame(frame, "Type: avl");
    add_line_to_frame(frame, "Watched: no");

    return 1;
}

// ========= WATCH HELPER =========

int watch_structure(ProgramState *state, const char *name)
{
    int i;
    Frame *frame;

    // Just do a simple search through every structure list
    // Not the most optimal thing ever but very easy to understand
    for (i = 0; i < state->stack_count; i++)
    {
        if (strcmp(state->stacks[i].name, name) == 0)
        {
            state->stacks[i].watched = 1;

            frame = start_frame(state, name, "watch", "Started watching stack");
            add_line_to_frame(frame, "This stack will now create frames for operations.");

            return 1;
        }
    }

    for (i = 0; i < state->queue_count; i++)
    {
        if (strcmp(state->queues[i].name, name) == 0)
        {
            state->queues[i].watched = 1;

            frame = start_frame(state, name, "watch", "Started watching queue");
            add_line_to_frame(frame, "This queue will now create frames for operations.");

            return 1;
        }
    }

    for (i = 0; i < state->deque_count; i++)
    {
        if (strcmp(state->deques[i].name, name) == 0)
        {
            state->deques[i].watched = 1;

            frame = start_frame(state, name, "watch", "Started watching deque");
            add_line_to_frame(frame, "This deque will now create frames for operations.");

            return 1;
        }
    }

    for (i = 0; i < state->avl_count; i++)
    {
        if (strcmp(state->avls[i].name, name) == 0)
        {
            state->avls[i].watched = 1;

            frame = start_frame(state, name, "watch", "Started watching avl tree");
            add_line_to_frame(frame, "This AVL tree will now create frames for operations.");

            return 1;
        }
    }

    snprintf(state->last_error,
             sizeof(state->last_error),
             "Structure %s not found",
             name);

    return 0;
}

// ========= MAIN PARSER =========

int parse_file(ProgramState *state, const char *path)
{
    FILE *file;
    char line[INPUT_LINE_LEN];
    int line_number = 0;

    file = fopen(path, "r");

    if (file == NULL)
    {
        snprintf(state->last_error,
                 sizeof(state->last_error),
                 "Could not open file");
        return 0;
    }

    skip_bom(file);

    // Read file line by line
    while (fgets(line, sizeof(line), file))
    {
        char words[5][MAX_NAME_LEN];
        int word_count;
        int value;

        line_number++;

        trim_newline(line);
        trim_spaces(line);

        // Ignore empty lines and comments
        if (line[0] == '\0' || line[0] == '#')
            continue;

        word_count = split_words(line, words, 5);

        // create TYPE NAME
        if (word_count == 3 && strcmp(words[0], "create") == 0)
        {
            if (strcmp(words[1], "stack") == 0)
            {
                if (!add_stack(state, words[2]))
                {
                    fclose(file);
                    return 0;
                }
            } else if (strcmp(words[1], "queue") == 0)
            {
                if (!add_queue(state, words[2]))
                {
                    fclose(file);
                    return 0;
                }
            } else if (strcmp(words[1], "deque") == 0)
            {
                if (!add_deque(state, words[2]))
                {
                    fclose(file);
                    return 0;
                }
            } else if (strcmp(words[1], "avl") == 0)
            {
                if (!add_avl(state, words[2]))
                {
                    fclose(file);
                    return 0;
                }
            } else
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d invalid structure type",
                         line_number);
                fclose(file);
                return 0;
            }
        }
        // watch NAME
        else if (word_count == 2 && strcmp(words[0], "watch") == 0)
        {
            if (!watch_structure(state, words[1]))
            {
                fclose(file);
                return 0;
            }
        }
        // push STACK VALUE
        else if (word_count == 3 && strcmp(words[0], "push") == 0)
        {
            StackInfo *stack = find_stack(state, words[1]);

            if (stack == NULL)
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: %s is not a stack",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            if (!parse_int_value(words[2], &value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d bad integer",
                         line_number);
                fclose(file);
                return 0;
            }

            if (!push_stack(stack, value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: stack %s is full",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            {
                char note[MAX_FRAME_TEXT];
                snprintf(note, sizeof(note), "Pushed %d", value);
                build_stack_frame(state, stack, "push", note);
            }
        }
        // pop STACK
        else if (word_count == 2 && strcmp(words[0], "pop") == 0)
        {
            StackInfo *stack = find_stack(state, words[1]);
            int popped_value;

            if (stack == NULL)
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: %s is not a stack",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            if (!pop_stack(stack, &popped_value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: stack %s is empty",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            {
                char note[MAX_FRAME_TEXT];
                snprintf(note, sizeof(note), "Popped %d", popped_value);
                build_stack_frame(state, stack, "pop", note);
            }
        }
        // peek STACK
        else if (word_count == 2 && strcmp(words[0], "peek") == 0)
        {
            StackInfo *stack = find_stack(state, words[1]);
            int top_value;

            if (stack == NULL)
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: %s is not a stack",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            if (!peek_stack(stack, &top_value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: stack %s is empty",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            {
                char note[MAX_FRAME_TEXT];
                snprintf(note, sizeof(note), "Top value is %d", top_value);
                build_stack_frame(state, stack, "peek", note);
            }
        }
        // enqueue QUEUE VALUE
        else if (word_count == 3 && strcmp(words[0], "enqueue") == 0)
        {
            QueueInfo *queue = find_queue(state, words[1]);

            if (queue == NULL)
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: %s is not a queue",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            if (!parse_int_value(words[2], &value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d bad integer",
                         line_number);
                fclose(file);
                return 0;
            }

            if (!enqueue_queue(queue, value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: queue %s is full",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            {
                char note[MAX_FRAME_TEXT];
                snprintf(note, sizeof(note), "Enqueued %d", value);
                build_queue_frame(state, queue, "enqueue", note);
            }
        }
        // dequeue QUEUE
        else if (word_count == 2 && strcmp(words[0], "dequeue") == 0)
        {
            QueueInfo *queue = find_queue(state, words[1]);
            int removed_value;

            if (queue == NULL)
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: %s is not a queue",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            if (!dequeue_queue(queue, &removed_value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: queue %s is empty",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            {
                char note[MAX_FRAME_TEXT];
                snprintf(note, sizeof(note), "Dequeued %d", removed_value);
                build_queue_frame(state, queue, "dequeue", note);
            }
        }
        // front QUEUE
        else if (word_count == 2 && strcmp(words[0], "front") == 0)
        {
            QueueInfo *queue = find_queue(state, words[1]);
            int front_value;

            if (queue == NULL)
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: %s is not a queue",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            if (!front_queue(queue, &front_value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: queue %s is empty",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            {
                char note[MAX_FRAME_TEXT];
                snprintf(note, sizeof(note), "Front value is %d", front_value);
                build_queue_frame(state, queue, "front", note);
            }
        }
        // push_front DEQUE VALUE
        else if (word_count == 3 && strcmp(words[0], "push_front") == 0)
        {
            DequeInfo *deque = find_deque(state, words[1]);

            if (deque == NULL)
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: %s is not a deque",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            if (!parse_int_value(words[2], &value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d bad integer",
                         line_number);
                fclose(file);
                return 0;
            }

            if (!push_front_deque(deque, value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: deque %s is full",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            {
                char note[MAX_FRAME_TEXT];
                snprintf(note, sizeof(note), "Pushed %d to front", value);
                build_deque_frame(state, deque, "push_front", note);
            }
        }
        // push_back DEQUE VALUE
        else if (word_count == 3 && strcmp(words[0], "push_back") == 0)
        {
            DequeInfo *deque = find_deque(state, words[1]);

            if (deque == NULL)
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: %s is not a deque",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            if (!parse_int_value(words[2], &value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d bad integer",
                         line_number);
                fclose(file);
                return 0;
            }

            if (!push_back_deque(deque, value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: deque %s is full",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            {
                char note[MAX_FRAME_TEXT];
                snprintf(note, sizeof(note), "Pushed %d to back", value);
                build_deque_frame(state, deque, "push_back", note);
            }
        }
        // pop_front DEQUE
        else if (word_count == 2 && strcmp(words[0], "pop_front") == 0)
        {
            DequeInfo *deque = find_deque(state, words[1]);
            int removed_value;

            if (deque == NULL)
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: %s is not a deque",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            if (!pop_front_deque(deque, &removed_value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: deque %s is empty",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            {
                char note[MAX_FRAME_TEXT];
                snprintf(note, sizeof(note), "Popped %d from front", removed_value);
                build_deque_frame(state, deque, "pop_front", note);
            }
        }
        // pop_back DEQUE
        else if (word_count == 2 && strcmp(words[0], "pop_back") == 0)
        {
            DequeInfo *deque = find_deque(state, words[1]);
            int removed_value;

            if (deque == NULL)
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: %s is not a deque",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            if (!pop_back_deque(deque, &removed_value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: deque %s is empty",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            {
                char note[MAX_FRAME_TEXT];
                snprintf(note, sizeof(note), "Popped %d from back", removed_value);
                build_deque_frame(state, deque, "pop_back", note);
            }
        }
        // insert AVL VALUE
        else if (word_count == 3 && strcmp(words[0], "insert") == 0)
        {
            AVLInfo *tree = find_avl(state, words[1]);
            char note[MAX_FRAME_TEXT];

            if (tree == NULL)
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: %s is not an avl tree",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            if (!parse_int_value(words[2], &value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d bad integer",
                         line_number);
                fclose(file);
                return 0;
            }

            if (!avl_insert_value(tree, value, note, sizeof(note)))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: could not insert into avl",
                         line_number);
                fclose(file);
                return 0;
            }

            build_avl_frame(state, tree, "insert", note);
        }
        // search AVL VALUE
        else if (word_count == 3 && strcmp(words[0], "search") == 0)
        {
            AVLInfo *tree = find_avl(state, words[1]);
            int found;
            char note[MAX_FRAME_TEXT];

            if (tree == NULL)
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d: %s is not an avl tree",
                         line_number,
                         words[1]);
                fclose(file);
                return 0;
            }

            if (!parse_int_value(words[2], &value))
            {
                snprintf(state->last_error,
                         sizeof(state->last_error),
                         "Line %d bad integer",
                         line_number);
                fclose(file);
                return 0;
            }

            found = avl_search_value(tree, value);

            if (found)
            {
                snprintf(note, sizeof(note), "Found value %d", value);
            } else
            {
                snprintf(note, sizeof(note), "Did not find value %d", value);
            }

            build_avl_frame(state, tree, "search", note);
        } else
        {
            snprintf(state->last_error,
                     sizeof(state->last_error),
                     "Line %d invalid command",
                     line_number);

            fclose(file);
            return 0;
        }
    }

    fclose(file);
    return 1;
}
