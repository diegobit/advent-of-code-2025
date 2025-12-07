#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "pls.h"

#define VERBOSE 0

/* =========== Range definition / allocation / read / deallocation ============ */

typedef struct range {
    char *start;
    char *end;
} range;

void freeRange(range *r) {
    if (r == NULL) return;
    if (r->start != NULL) ps_free(r->start);
    if (r->end != NULL) ps_free(r->end);
    free(r);
}

/* Read one range from file, advancing the pointer. Returns the range if 
 * found, NULL otherwise. Assumes the file content is valid: only one line
 * and all ranges are wellformed.
*/
range *readRange(FILE *fp) {
    range *r = calloc(1, sizeof(*r));

    char buf[1024];
    int ptr = 0;

    int found = 0;
    int c;
    while(!found && (c = getc(fp)) != EOF) {
        if (c == '-') {
            r->start = malloc(ptr);
            memcpy(r->start, &buf, ptr);

            char *ps = ps_create(r->start, ptr); // ptr is len!
            free(r->start);
            r->start = ps;

            r->start[ptr] = 0;
            ptr = 0;
        } else if (c == ',' || c == '\n') {
            r->end = malloc(ptr);
            memcpy(r->end, &buf, ptr);

            char *ps = ps_create(r->end, ptr); // ptr is len!
            free(r->end);
            r->end = ps;

            r->end[ptr] = 0;
            ptr = 0;
            found = 1;
        } else {
            buf[ptr] = c;
            ptr++;
        }
    }

    if (!found) {
        freeRange(r);
        return NULL;
    }

    return r;
}

/* ===================== Exercise logic over ranges ====================== */

/* Returns the invalid Id as a long, if found, otherwise returns 0. */
long getInvalidId(char *pls) {
    uint32_t *lenptr = (uint32_t*)(pls-4);
    if (*lenptr %2 != 0) {
#if VERBOSE
        printf("  Eval: %12s  is odd\n", pls);
#endif
        return 0;
    }
    if (pls[0] == '0') {
#if VERBOSE
        printf("  Eval: %12s  starts with 0\n", pls);
#endif
        return 0;
    } else {
#if VERBOSE
        printf("  Eval: %12s", pls);
#endif
    }
    uint32_t halflen = *lenptr / 2;

    char *first_half_ptr = pls;
    char *second_half_ptr = pls + halflen;

    // Using memcmp instead of strncmp because strncmp would stop at \0
    int cmp = memcmp(first_half_ptr, second_half_ptr, halflen) != 0;
    if (cmp == 0) {
        printf("  (%.*s", halflen, first_half_ptr);
        printf(" %.*s)", halflen, second_half_ptr);
        printf(" --> Not valid\n");
        long val = strtol(pls, NULL, 10);
        return val;
    } else {
#if VERBOSE
        printf("  (%.*s", halflen, first_half_ptr);
        printf(" %.*s)\n", halflen, second_half_ptr);
#endif
        return 0;
    }
}

/* Returns the sum of the invalid ids in range 'r'. */
long sumValidIds(range *r) {
    long sum = 0;
    sum += getInvalidId(r->start);
    sum += getInvalidId(r->end);

    long start = strtol(r->start, NULL, 10);
    long end = strtol(r->end, NULL, 10);
    for (long i = start+1; i < end; i++) {
        char *s = calloc(20, sizeof(char));
        assert(s != NULL);
        snprintf(s, 20, "%ld", i);
        // printf("  Eval: %s\n", s);
        char *pls = ps_create(s, strlen(s));
        sum += getInvalidId(pls);
        ps_free(pls);
        free(s);
    }
    printf("  Sum invalid IDs in this range: %ld\n", sum);

    return sum;
}


/* ================================= Main ================================== */

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

    // Read file and 
    range *r = NULL;
    long sum = 0;
    do {
        freeRange(r);
        r = readRange(fp);
        if (r == NULL) break;
        printf("%s-%s\n", r->start, r->end);
        sum += sumValidIds(r);
    } while(r);
    printf("\n");
    freeRange(r);

    printf("sum: %ld\n", sum);
}
