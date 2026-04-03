#include "common.h"

#include <stdio.h>
#include <string.h>

static void start_frame(Frame *frame, ProgramState *state, Structure *st, const char *operation, const char *note) {
    if (state->frame_count >= MAX_FRAMES) {
        return;
    }

    frame->step_number = state->frame_count + 1;

    strncpy(frame->structure_name, st->name, MAX_NAME_LEN - 1);
    frame->structure_name[MAX_NAME_LEN - 1] = '\0';

    strncpy(frame->operation, operation, sizeof(frame->operation) - 1);
    frame->operation[sizeof(frame->operation) - 1] = '\0';

    strncpy(frame->note, note, sizeof(frame->note) - 1);
    frame->note[sizeof(frame->note) - 1] = '\0';

    frame->line_count = 0;
}

static void add_line(Frame *frame, const char *text) {
    if (frame->line_count >= MAX_FRAME_LINES) {
        return;
    }

    strncpy(frame->lines[frame->line_count], text, MAX_LINE_LEN - 1);
    frame->lines[frame->line_count][MAX_LINE_LEN - 1] = '\0';
    frame->line_count++;
}

void init_stack(Stack *s) {
    s->top = -1;
}

int push_stack(Stack *s, int value) {
    if (s->top >= STACK_CAP - 1) {
        return 0;
    }

    s->top++;
    s->data[s->top] = value;
    return 1;
}

int pop_stack(Stack *s, int *out_value) {
    if (s->top < 0) {
        return 0;
    }

    *out_value = s->data[s->top];
    s->top--;
    return 1;
}

int peek_stack(Stack *s, int *out_value) {
    if (s->top < 0) {
        return 0;
    }

    *out_value = s->data[s->top];
    return 1;
}

void build_stack_frame(ProgramState *state, Structure *st, const char *operation, const char *note) {
    Frame *frame;
    char line[MAX_LINE_LEN];
    int i;

    if (!st->watched || state->frame_count >= MAX_FRAMES) {
        return;
    }

    frame = &state->frames[state->frame_count];
    start_frame(frame, state, st, operation, note);

    snprintf(line, sizeof(line), "Stack: %s", st->name);
    add_line(frame, line);

    if (st->stack.top < 0) {
        add_line(frame, "[empty]");
    } else {
        for (i = st->stack.top; i >= 0; i--) {
            if (i == st->stack.top) {
                snprintf(line, sizeof(line), "[%d]  <- top", st->stack.data[i]);
            } else {
                snprintf(line, sizeof(line), "[%d]", st->stack.data[i]);
            }
            add_line(frame, line);
        }
    }

    state->frame_count++;
}

void init_queue(Queue *q) {
    q->front = 0;
    q->rear = 0;
    q->size = 0;
}

int enqueue_queue(Queue *q, int value) {
    if (q->size >= QUEUE_CAP) {
        return 0;
    }

    q->data[q->rear] = value;
    q->rear = (q->rear + 1) % QUEUE_CAP;
    q->size++;
    return 1;
}

int dequeue_queue(Queue *q, int *out_value) {
    if (q->size <= 0) {
        return 0;
    }

    *out_value = q->data[q->front];
    q->front = (q->front + 1) % QUEUE_CAP;
    q->size--;
    return 1;
}

int front_queue(Queue *q, int *out_value) {
    if (q->size <= 0) {
        return 0;
    }

    *out_value = q->data[q->front];
    return 1;
}

void build_queue_frame(ProgramState *state, Structure *st, const char *operation, const char *note) {
    Frame *frame;
    char line[MAX_LINE_LEN];
    char temp[MAX_LINE_LEN];
    int i;
    int index;

    if (!st->watched || state->frame_count >= MAX_FRAMES) {
        return;
    }

    frame = &state->frames[state->frame_count];
    start_frame(frame, state, st, operation, note);

    snprintf(line, sizeof(line), "Queue: %s", st->name);
    add_line(frame, line);

    if (st->queue.size == 0) {
        add_line(frame, "[empty]");
    } else {
        temp[0] = '\0';

        for (i = 0; i < st->queue.size; i++) {
            index = (st->queue.front + i) % QUEUE_CAP;

            if (i == 0) {
                snprintf(line, sizeof(line), "[%d]", st->queue.data[index]);
            } else {
                snprintf(line, sizeof(line), " -> [%d]", st->queue.data[index]);
            }

            strncat(temp, line, sizeof(temp) - strlen(temp) - 1);
        }

        add_line(frame, temp);

        snprintf(line, sizeof(line), "front = %d", st->queue.data[st->queue.front]);
        add_line(frame, line);

        index = (st->queue.rear - 1 + QUEUE_CAP) % QUEUE_CAP;
        snprintf(line, sizeof(line), "rear  = %d", st->queue.data[index]);
        add_line(frame, line);
    }

    state->frame_count++;
}

void init_deque(Deque *d) {
    d->front = 0;
    d->rear = 0;
    d->size = 0;
}

int push_front_deque(Deque *d, int value) {
    if (d->size >= DEQUE_CAP) {
        return 0;
    }

    d->front = (d->front - 1 + DEQUE_CAP) % DEQUE_CAP;
    d->data[d->front] = value;
    d->size++;
    return 1;
}

int push_back_deque(Deque *d, int value) {
    if (d->size >= DEQUE_CAP) {
        return 0;
    }

    d->data[d->rear] = value;
    d->rear = (d->rear + 1) % DEQUE_CAP;
    d->size++;
    return 1;
}

int pop_front_deque(Deque *d, int *out_value) {
    if (d->size <= 0) {
        return 0;
    }

    *out_value = d->data[d->front];
    d->front = (d->front + 1) % DEQUE_CAP;
    d->size--;
    return 1;
}

int pop_back_deque(Deque *d, int *out_value) {
    int index;

    if (d->size <= 0) {
        return 0;
    }

    index = (d->rear - 1 + DEQUE_CAP) % DEQUE_CAP;
    *out_value = d->data[index];
    d->rear = index;
    d->size--;
    return 1;
}

void build_deque_frame(ProgramState *state, Structure *st, const char *operation, const char *note) {
    Frame *frame;
    char line[MAX_LINE_LEN];
    char temp[MAX_LINE_LEN];
    int i;
    int index;

    if (!st->watched || state->frame_count >= MAX_FRAMES) {
        return;
    }

    frame = &state->frames[state->frame_count];
    start_frame(frame, state, st, operation, note);

    snprintf(line, sizeof(line), "Deque: %s", st->name);
    add_line(frame, line);

    if (st->deque.size == 0) {
        add_line(frame, "[empty]");
    } else {
        temp[0] = '\0';

        for (i = 0; i < st->deque.size; i++) {
            index = (st->deque.front + i) % DEQUE_CAP;

            if (i == 0) {
                snprintf(line, sizeof(line), "[%d]", st->deque.data[index]);
            } else {
                snprintf(line, sizeof(line), " <-> [%d]", st->deque.data[index]);
            }

            strncat(temp, line, sizeof(temp) - strlen(temp) - 1);
        }

        add_line(frame, temp);

        snprintf(line, sizeof(line), "front = %d", st->deque.data[st->deque.front]);
        add_line(frame, line);

        index = (st->deque.rear - 1 + DEQUE_CAP) % DEQUE_CAP;
        snprintf(line, sizeof(line), "rear  = %d", st->deque.data[index]);
        add_line(frame, line);
    }

    state->frame_count++;
}