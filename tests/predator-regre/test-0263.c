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

	p->a = 1;
	if (p->a != 1) {
		free(p);
	}
	p->b = 2;
	if (p->b != 2) {
		free(p);
	}
	p->c = 3;
	if (p->c != 3) {
		free(p);
	}
	p->d = 4;
	if (p->d != 4) {
		free(p);
	}

	free(p);
}

/**
 * @file test-0263.c
 *
 * @brief a test for bitfields
 *
 * - based on https://sv-comp.sosy-lab.org/2020/results/sv-benchmarks/c/ldv-memsafety-bitfields/test-bitfields-1-1.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
