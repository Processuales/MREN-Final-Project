#ifndef DATA_STRUCTURE_VIEWER_PARSER_H
#define DATA_STRUCTURE_VIEWER_PARSER_H

#include "common.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

// ========= HELPER FUNCTIONS =========
// Source: https://stackoverflow.com/questions/8880548/ignore-byte-order-marks-in-c-reading-from-a-stream
static void skip_bom(FILE *file)
{
    unsigned char bom[3];

    bom[0] = fgetc(file);
    bom[1] = fgetc(file);
    bom[2] = fgetc(file);

    if (!(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF))
    {
        ungetc(bom[2], file);
        ungetc(bom[1], file);
        ungetc(bom[0], file);
    }
}

// Trim newline and carriage return characters from the end of the line
static void trim_newline(char *line)
{
    int i = 0;

    while (line[i] != '\0')
    {
        if (line[i] == '\n' || line[i] == '\r')
        {
            line[i] = '\0';
            return;
        }
        i++;
    }
}

// Trim spaces from the beginning and end of the line
static void trim_spaces(char *line)
{
    int start = 0;
    int end;
    int i;

    // Step 1: Find where the actual text starts
    while (line[start] != '\0' && isspace((unsigned char)line[start]))
    {
        start++;
    }

    // Step 2: Move the text to the beginning of the string
    i = 0;

    while (line[start] != '\0')
    {
        line[i] = line[start];
        i++;
        start++;
    }

    line[i] = '\0';

    // Step 3: Find where the text ends
    end = (int)strlen(line) - 1;

    // Step 4: Remove spaces at the end
    while (end >= 0 && isspace((unsigned char)line[end]))
    {
        line[end] = '\0';
        end--;
    }
}

// Split a line into words based on spaces and tabs. Returns the number of words found.
static int split_words(char *line, char words[][MAX_NAME_LEN], int max_words)
{
    int word_count = 0;
    char *token = strtok(line, " \t");

    // Keep getting words until no more words left
    while (token != NULL && word_count < max_words)
    {
        strncpy(words[word_count], token, MAX_NAME_LEN - 1);
        words[word_count][MAX_NAME_LEN - 1] = '\0';
        word_count++;

        token = strtok(NULL, " \t");
    }

    return word_count;
}

// Frees all AVL nodes in the tree recursively
void free_all_trees(ProgramState *state)
{
    int i;

    for (i = 0; i < state->avl_count; i++)
    {
        free_avl_nodes(state->avls[i].root);
        state->avls[i].root = NULL;
    }
}

#endif // DATA_STRUCTURE_VIEWER_PARSER_H