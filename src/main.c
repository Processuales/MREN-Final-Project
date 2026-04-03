#include "common.h"

#include <stdio.h>
#include <string.h>

static void print_frame(Frame *frame, int current_index, int total_frames) {
    int i;

    printf("========================================\n");
    printf("Frame %d / %d\n", current_index + 1, total_frames);
    printf("Structure: %s\n", frame->structure_name);
    printf("Operation: %s\n", frame->operation);
    printf("Note: %s\n", frame->note);
    printf("========================================\n\n");

    for (i = 0; i < frame->line_count; i++) {
        printf("%s\n", frame->lines[i]);
    }

    printf("\n[a] previous   [d] next   [q] quit\n");
}

static void run_frame_viewer(ProgramState *state) {
    int current = 0;
    char input[32];

    if (state->frame_count == 0) {
        printf("No frames were created. Maybe nothing was watched.\n");
        return;
    }

    while (1) {
        clear_screen();
        print_frame(&state->frames[current], current, state->frame_count);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        if (input[0] == 'q' || input[0] == 'Q') {
            break;
        } else if (input[0] == 'a' || input[0] == 'A') {
            if (current > 0) {
                current--;
            }
        } else if (input[0] == 'd' || input[0] == 'D') {
            if (current < state->frame_count - 1) {
                current++;
            }
        }
    }
}

int main(void) {
    ProgramState state;
    char path[INPUT_LINE_LEN];

    init_program_state(&state);

    printf("Enter path to .dsa file:\n");
    if (fgets(path, sizeof(path), stdin) == NULL) {
        printf("Could not read path.\n");
        return 1;
    }

    path[strcspn(path, "\r\n")] = '\0';

    if (!parse_file_and_build_frames(&state, path)) {
        printf("Error: %s\n", state.last_error);
        free_all_trees(&state);
        return 1;
    }

    run_frame_viewer(&state);
    free_all_trees(&state);
    return 0;
}