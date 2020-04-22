/* Contributed by Anton Vasilyev. */

#include <stdlib.h>
#include <string.h>

struct A {
	unsigned char a;
	unsigned char b:2;
	unsigned char c:2;
	unsigned char d:4;
	unsigned char e:4;
	unsigned char pad;
	unsigned int f;
} __attribute__((packed));

struct B {
	unsigned char a;
	unsigned char b:2;
	unsigned char c:2;
	unsigned char d:4;
	unsigned char e;
	unsigned char f;
	unsigned char f1;
	unsigned char f2;
	unsigned char f3;
	unsigned char f4;
} __attribute__((packed));

struct B d = {.c = 3, 4, .b = 2};
int main(void)
{
	struct A *p;
	p = malloc(sizeof(struct A));
	memcpy(p, &d, sizeof(struct A));
	if (p->a != 0) {
		free(p);
	}
	if (p->b != 2) {
		free(p);
	}
	if (p->c != 3) {
		free(p);
	}
	if (p->d != 4) {
		free(p);
	}
	if (p->e != 0) {
		free(p);
	}
	if (p->f != 0) {
		free(p);
	}
	free(p);
}

/**
 * @file test-0265.c
 *
 * @brief a test for bitfields
 *
 * - taken from https://sv-comp.sosy-lab.org/2020/results/sv-benchmarks/c/ldv-memsafety-bitfields/test-bitfields-3.1-1.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
