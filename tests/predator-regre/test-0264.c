/* Contributed by Anton Vasilyev. */

#include <stdlib.h>

struct A {
	unsigned char a;
	unsigned char b:2;
	unsigned char c:2;
	unsigned char d:4;
} __attribute__((packed));

int main(void)
{
	struct A *p;
	p = calloc(1, sizeof(struct A));
	if (p == NULL)
	    return 1;

	p->c = 3;
	if (p->b == 3) {
		free(p);
	}

	free(p);
}

/**
 * @file test-0264.c
 *
 * @brief modified test-0263, unsound for gcc plugin
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
