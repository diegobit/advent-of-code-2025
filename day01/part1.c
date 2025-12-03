#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dial {
    int tick;
    int len;
} dial;


void rotate(dial *d, int rotation) {
    d->tick = d->tick + rotation;
    while (d->tick < 0) d->tick = d->tick + d->len;
    while (d->tick >= d->len) d->tick = d->tick - d->len;
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
    char buf[1024];
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (buf[0] == 'L') buf[0] = '-';
        if (buf[0] == 'R') buf[0] = '+';
        char *end;
        int rotation = strtol(buf, &end, 10);

        // Rotate
        // printf("ROT %d, c=%d, FROM %d to ", rotation, zero_count, d->tick);
        rotate(d, rotation);
        // printf("%d\n", d->tick);
        if (d->tick == 0) zero_count++;
    }

    printf("END d->tick=%d, d->len=%d, zero_count=%d\n", d->tick, d->len, zero_count);

    free(d);
}
