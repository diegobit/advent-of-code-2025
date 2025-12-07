#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Initialize a prefixed length string with the specified
 * string in 'init' of length 'len'.
 *
 * The created string has the following layout:
 *
 * +----+-----------\\\
 * |LLLL|My string here
 * +----+-----------\\\
 *
 * Where L is four unsigned bytes stating the total length of the string.
 * Thus this strings are binary safe: _zero_ bytes are permitted in the middle.
 *
 * Defensively, the string is null terminated to allow printf to not overflow.
 *
 * Returns the pointer to the allocated memory. */
char *ps_create(char *init, int len) {
    char *s = malloc(4+len+1);
    uint32_t *lenptr = (uint32_t*)s;
    *lenptr = len;

    s += 4;
    for (int j = 0; j < len; j++) {
        s[j] = init[j];
    }
    s[len] = 0;
    return s;
}

/* Display the string 's' on the screen */
void ps_print(char *s) {
    uint32_t *lenptr = (uint32_t*)(s-4);
    for (uint32_t j = 0; j < *lenptr; j++) {
        putchar(s[j]);
    }
    printf("\n");
}

/* Free a previously created PS string. */
void ps_free(char *s) {
    free(s-4);
}

uint32_t ps_len(char *s) {
    uint32_t *lenptr = (uint32_t*)(s-4);
    return *lenptr;
}
