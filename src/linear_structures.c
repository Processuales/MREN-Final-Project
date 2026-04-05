#include "common.h"

#include <stdio.h>
#include <string.h>

void init_stack(StackInfo *stack)
{
    stack->top = -1;
}


int push_stack(StackInfo *stack, int value)
{
    if (stack->top >= LINEAR_CAP - 1)
    {
        return 0;
    }

    stack->top++;
    stack->data[stack->top] = value;
    return 1;
}

int pop_stack(StackInfo *stack, int *out_value)
{
    if (stack->top < 0)
    {
        return 0;
    }

    *out_value = stack->data[stack->top];
    stack->top--;
    return 1;
}

int peek_stack(StackInfo *stack, int *out_value)
{
    if (stack->top < 0)
    {
        return 0;
    }

    *out_value = stack->data[stack->top];
    return 1;
}

void build_stack_frame(ProgramState *state, StackInfo *stack, const char *operation, const char *note)
{
    Frame *frame;
    char line[MAX_LINE_TEXT];
    int i;

    // Only watched structures create operation frames
    if (!stack->watched)
    {
        return;
    }

    frame = start_frame(state, stack->name, operation, note);

    if (frame == NULL)
    {
        return;
    }

    add_line_to_frame(frame, "Stack state:");

    if (stack->top < 0)
    {
        add_line_to_frame(frame, "[empty]");
        return;
    }

    // print from top to bottom
    for (i = stack->top; i >= 0; i--)
    {
        if (i == stack->top)
        {
            snprintf(line, sizeof(line), "[%d]  <- top", stack->data[i]);
        }
        else
        {
            snprintf(line, sizeof(line), "[%d]", stack->data[i]);
        }

        add_line_to_frame(frame, line);
    }
}

void init_queue(QueueInfo *queue)
{
    queue->front = 0;
    queue->rear = 0;
    queue->size = 0;
}

int enqueue_queue(QueueInfo *queue, int value)
{
    if (queue->size >= LINEAR_CAP)
    {
        return 0;
    }

    queue->data[queue->rear] = value;
    queue->rear = (queue->rear + 1) % LINEAR_CAP;
    queue->size++;
    return 1;
}

int dequeue_queue(QueueInfo *queue, int *out_value)
{
    if (queue->size <= 0)
    {
        return 0;
    }

    *out_value = queue->data[queue->front];
    queue->front = (queue->front + 1) % LINEAR_CAP;
    queue->size--;
    return 1;
}

int front_queue(QueueInfo *queue, int *out_value)
{
    if (queue->size <= 0)
    {
        return 0;
    }

    *out_value = queue->data[queue->front];
    return 1;
}

void build_queue_frame(ProgramState *state, QueueInfo *queue, const char *operation, const char *note)
{
    Frame *frame;
    char line[MAX_LINE_TEXT];
    char values_line[MAX_LINE_TEXT];
    int i;
    int index;

    if (!queue->watched)
    {
        return;
    }

    frame = start_frame(state, queue->name, operation, note);

    if (frame == NULL)
    {
        return;
    }

    add_line_to_frame(frame, "Queue state:");

    if (queue->size == 0)
    {
        add_line_to_frame(frame, "[empty]");
        return;
    }

    values_line[0] = '\0';

    for (i = 0; i < queue->size; i++)
    {
        index = (queue->front + i) % LINEAR_CAP;

        if (i == 0)
        {
            snprintf(line, sizeof(line), "[%d]", queue->data[index]);
        }
        else
        {
            snprintf(line, sizeof(line), " -> [%d]", queue->data[index]);
        }

        strncat(values_line, line, sizeof(values_line) - strlen(values_line) - 1);
    }

    add_line_to_frame(frame, values_line);

    snprintf(line, sizeof(line), "front = %d", queue->data[queue->front]);
    add_line_to_frame(frame, line);

    index = (queue->rear - 1 + LINEAR_CAP) % LINEAR_CAP;
    snprintf(line, sizeof(line), "rear  = %d", queue->data[index]);
    add_line_to_frame(frame, line);
}

void init_deque(DequeInfo *deque)
{
    deque->front = 0;
    deque->rear = 0;
    deque->size = 0;
}

int push_front_deque(DequeInfo *deque, int value)
{
    if (deque->size >= LINEAR_CAP)
    {
        return 0;
    }

    deque->front = (deque->front - 1 + LINEAR_CAP) % LINEAR_CAP;
    deque->data[deque->front] = value;
    deque->size++;
    return 1;
}

int push_back_deque(DequeInfo *deque, int value)
{
    if (deque->size >= LINEAR_CAP)
    {
        return 0;
    }

    deque->data[deque->rear] = value;
    deque->rear = (deque->rear + 1) % LINEAR_CAP;
    deque->size++;
    return 1;
}

int pop_front_deque(DequeInfo *deque, int *out_value)
{
    if (deque->size <= 0)
    {
        return 0;
    }

    *out_value = deque->data[deque->front];
    deque->front = (deque->front + 1) % LINEAR_CAP;
    deque->size--;
    return 1;
}

int pop_back_deque(DequeInfo *deque, int *out_value)
{
    int index;

    if (deque->size <= 0)
    {
        return 0;
    }

    index = (deque->rear - 1 + LINEAR_CAP) % LINEAR_CAP;
    *out_value = deque->data[index];
    deque->rear = index;
    deque->size--;
    return 1;
}

void build_deque_frame(ProgramState *state, DequeInfo *deque, const char *operation, const char *note)
{
    Frame *frame;
    char line[MAX_LINE_TEXT];
    char values_line[MAX_LINE_TEXT];
    int i;
    int index;

    if (!deque->watched)
    {
        return;
    }

    frame = start_frame(state, deque->name, operation, note);

    if (frame == NULL)
    {
        return;
    }

    add_line_to_frame(frame, "Deque state:");

    if (deque->size == 0)
    {
        add_line_to_frame(frame, "[empty]");
        return;
    }

    values_line[0] = '\0';

    for (i = 0; i < deque->size; i++)
    {
        index = (deque->front + i) % LINEAR_CAP;

        if (i == 0)
        {
            snprintf(line, sizeof(line), "[%d]", deque->data[index]);
        }
        else
        {
            snprintf(line, sizeof(line), " <-> [%d]", deque->data[index]);
        }

        strncat(values_line, line, sizeof(values_line) - strlen(values_line) - 1);
    }

    add_line_to_frame(frame, values_line);

    snprintf(line, sizeof(line), "front = %d", deque->data[deque->front]);
    add_line_to_frame(frame, line);

    index = (deque->rear - 1 + LINEAR_CAP) % LINEAR_CAP;
    snprintf(line, sizeof(line), "rear  = %d", deque->data[index]);
    add_line_to_frame(frame, line);
}