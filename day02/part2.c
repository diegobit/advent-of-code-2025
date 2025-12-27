#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "pls.h"
#include <limits.h>

#define VERBOSE 0
#if VERBOSE
#define VPRINTF(...) printf(__VA_ARGS__)
#else
#define VPRINTF(...) ((void)0)
#endif

/* ========================== Allocation wrappers ========================== */

void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory allocating %zu bytes\n", size);
        exit(1);
    }
    return ptr;
}

void *xcalloc(size_t count, size_t size) {
    void *ptr = calloc(count, size);
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory allocating %zu bytes\n", size);
        exit(1);
    }
    return ptr;
}

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
    range *r = xcalloc(1, sizeof(*r));
    if (!r) return NULL;

    char buf[1024];
    int ptr = 0;

    int found = 0;
    int c;
    while(!found && (c = getc(fp)) != EOF) {
        if (c == '-') {
            r->start = xmalloc(ptr);
            memcpy(r->start, buf, ptr);

            char *ps = ps_create(r->start, ptr); // ptr is len!
            free(r->start);
            r->start = ps;

            r->start[ptr] = 0;
            ptr = 0;
        } else if (c == ',' || c == '\n') {
            r->end = xmalloc(ptr);
            memcpy(r->end, buf, ptr);

            char *ps = ps_create(r->end, ptr); // ptr is len!
            free(r->end);
            r->end = ps;

            r->end[ptr] = 0;
            ptr = 0;
            found = 1;
        } else {
            if (ptr >= (int)sizeof(buf)) {
                printf("Trying to write out of bounds for buf. Increase buf size.\n");
                freeRange(r);
                return NULL;
            }
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

/* ======================= Get dividers of an integer ====================== */

int compare_ints(const void *a, const void *b) {
    const int al = *(int*)a;
    const int bl = *(int*)b;
    int cmp = (al > bl) - (al < bl);
    // Descending order
    return -cmp;
}

/* Find the dividers of integer n and return a list with the numbers in
 * decreasing order.
 * Returns a newly allocated array of length 'width', which should be the
 * maximum number of dividers.
 *
 * Intuition: loop up to sqrt(n) and use pairs.
 * 
 * For each i from 1 to floor(sqrt(n)):
 *   If n % i == 0, then i is a divisor and n / i is also a divisor.
 *   If i * i == n, add i.
 */
int *getDividers(int n, int width) {
    if (width <= 0 || n <= 0) {
        printf("WARN: getDividers args wrong. width=%d, n=%d", width, n);
        return NULL;
    }
    // Not worth it to have a dynamically allocated array;
    int *divs = xcalloc((size_t)width, sizeof(int));
    int curr = 0;
    for (int i = 1; i <= floor(sqrt(n)); i++) {
        if (n % i == 0) {
            assert(curr < width);
            divs[curr++] = i;
            assert(curr < width);
            if (i != n/i) divs[curr++] = n/i;
        }
    }
    qsort(divs, width, sizeof(int), compare_ints);
    return divs;
}

/* ===================== Exercise logic over ranges ====================== */

/* Returns the invalid Id as a int, if found, otherwise returns 0. */
long getInvalidId(char *pls) {
    uint32_t *lenptr = (uint32_t*)(pls-4);
    if (pls[0] == '0') {
        VPRINTF("  Eval: %12s  starts with 0\n", pls);
        return 0;
    } else {
        VPRINTF("  Eval: %12s\n", pls);
    }

    // Given the number of digits, get the possible 'splits', eg. 123456 has
    // length 6, so 3 and 2 because I can split 123456 into 123-456 or 12-34-56
    int max_num_dividers = (int)floor(sqrt((double)*lenptr)) * 2;
    if (max_num_dividers < 2) return 0;
    int *divs = getDividers(*lenptr, max_num_dividers);
    VPRINTF("    Dividers: ");
    for (int i = 0; i < max_num_dividers; i++) {
        VPRINTF("%d,", divs[i]);
    }
    VPRINTF("\n");

    for (int i = 0; i < max_num_dividers; i++) {
        int restart = 0;
        uint32_t currlen = divs[i]; // the divider is the current length!
        int ci = (currlen > INT_MAX) ? INT_MAX : (int)currlen;
        if (currlen == 0) continue;
        if (*lenptr == currlen) continue;
        int num_parts = *lenptr / currlen;
        VPRINTF("    num_parts: %d\n", num_parts);

        // Compare the first part with each other parts, eg. given 123456
        // if currlen = 3 -> compare 123 with 456, or
        // if currlen = 2 -> compare 12 with 34; then 12 with 56.
        char *first_part_ptr = pls;
        for (int j = 1; j < num_parts; j++) {
            char *curr_part_ptr = pls + j*currlen;
            VPRINTF("    comparing: %.*s with %.*s\n", ci, first_part_ptr, ci, curr_part_ptr);
            int cmp = memcmp(first_part_ptr, curr_part_ptr, currlen);
            if (cmp != 0) { // different, move to next divider
                VPRINTF("  (%.*s", ci, first_part_ptr);
                VPRINTF(" %.*s)\n", ci, curr_part_ptr);
                restart = 1;
                break;
            }
        }
        if (restart) continue;

        printf("  (%.*s) %d times", ci, first_part_ptr, num_parts);
        printf(" --> Not valid\n");
        long val = strtol(pls, NULL, 10);
        free(divs);
        return val;
    }
    free(divs);
    return 0;

}

/* Returns the sum of the invalid ids in range 'r'. */
long sumValidIds(range *r) {
    long sum = 0;
    sum += getInvalidId(r->start);
    sum += getInvalidId(r->end);

    long start = strtol(r->start, NULL, 10);
    long end = strtol(r->end, NULL, 10);
    for (long i = start+1; i < end; i++) {
        char *s = xcalloc(32, sizeof(char));
        assert(s != NULL);
        snprintf(s, 32, "%ld", i);
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
    fclose(fp);

    printf("sum: %ld\n", sum);
}
