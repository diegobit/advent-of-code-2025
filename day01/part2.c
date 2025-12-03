#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct dial {
    int tick;
    int len;
} dial;


/* Simulate the click over the dial. Click logic is somewhat separated from
 * rotation. */
int click(dial *d) {
    if (d->tick == 0) return 1;
    return 0;
}

/* Rotate the dial by 'rotation' number of clicks. Simulate the dial by doing
 * the rotation one position at a time.
 * Click logic is separated from rotation. */
int rotate(dial *d, int rotation) {
    int turns = 0;

    int dir = rotation >= 0 ? +1 : -1;
    rotation = abs(rotation);
    while (rotation > 0) {
        d->tick += dir;
        if (d->tick >= d->len) d->tick-=d->len;
        if (d->tick < 0)       d->tick+=d->len;

        turns += click(d);

        rotation--;
    }
    return turns;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Open file
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("File does not exist.");
        return 1;
    }

    // Allocate malloc
    dial *d = malloc(sizeof(*d));
    d->tick = 50;
    d->len = 100;
    int zero_count = 0;

    // Read file line by line
    char buf[8192];
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (buf[0] == 'L') buf[0] = '-';
        if (buf[0] == 'R') buf[0] = '+';
        char *end;
        int rotation = strtol(buf, &end, 10);

        // Rotate
        // sleep(1);
        int start_count = zero_count;
        int start_tick = d->tick;

        int turns = rotate(d, rotation);
        zero_count += turns;

        printf("ROT: %d, Dial: %d->%d, Zero count: %d->%d\n", rotation, start_tick, d->tick, start_count, zero_count);
    }

    printf("END: Dial: %d, Len: %d, Zero count: %d\n", d->tick, d->len, zero_count);

    free(d);
}

