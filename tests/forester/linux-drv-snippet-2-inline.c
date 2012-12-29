/*
 * Linux driver snippet
 *
 * boxes:
 */

#include <stdlib.h>
#include <verifier-builtins.h>

struct node_top {
    struct node_top *next;
    struct node_low *data;
};

struct node_low {
    struct node_low *next;
};

int main()
{
    struct node_top *top = malloc(sizeof *top);
    struct node_top *now = top;
    top->next = NULL;
    top->data = NULL;
    if (__VERIFIER_nondet_int()) {
        struct node_low *ptr = malloc(sizeof *ptr);
        ptr->next = NULL;
        top->data = ptr;
    }

    while (__VERIFIER_nondet_int()) {
        struct node_top *pi = malloc(sizeof *pi);
        pi->next = NULL;
        pi->data = NULL;

      if (__VERIFIER_nondet_int()) {
        struct node_low *ptr = malloc(sizeof *ptr);
        ptr->next = NULL;
        pi->data = ptr;
      }
	
      now->next = pi;
      now = now->next;
    }

    while (top) {
      now = top;
      top = top->next;
      if (now->data) {
        free(now->data);
      }
      free(now);
    }

    return 0;
}
