#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <verifier-builtins.h>

typedef struct {
  unsigned int item_count;
  struct stack_item {
    int item;
    struct stack_item *next;
  } *head;
} stack_t;

void stack_init(stack_t * stack)
{
  stack->head = NULL;
  stack->item_count = 0;
}

bool stack_empty(stack_t const * stack)
{
  return stack->item_count == 0;
}

bool stack_pop(stack_t * stack)
{
  if (stack->item_count == 0){
      return false;
  }

  struct stack_item *new_stack_item = stack->head;
  stack->head = stack->head->next;
  free(new_stack_item);
  stack->item_count--;
  return true;
}

bool stack_push(stack_t * stack, int item)
{
  if (stack->item_count == 0) {
    stack->head = malloc(sizeof(struct stack_item));
    if (!stack->head){
      return false;
    }

    stack->head->item = item;
    stack->head->next = NULL;
    stack->item_count = 1;
  }
  else {
    struct stack_item *new_stack_item = malloc(sizeof(struct stack_item));
    if (!new_stack_item) {
      return false;
    }

    new_stack_item->item = item;
    new_stack_item->next = stack->head;
    stack->head = new_stack_item;
    stack->item_count++;
  }
  return true;
}

unsigned int stack_size(stack_t const *stack)
{
  return stack->item_count;
}

int stack_top(stack_t const * stack)
{
  return stack->head->item;
}

void stack_clear(stack_t *stack)
{
  struct stack_item *stack_item;
  while ((stack_item = stack->head) != NULL) {
    // The following two line should be swapped. Error here!!!
    free(stack_item);
    stack->head = stack->head->next;
    stack->item_count--;
  }
}

int main(void)
{
  stack_t stack;
  stack_init(&stack);

  printf("Pushing items: ");
  int i;
  for (i = 0; i < 10; i++) {
    if (!stack_push(&stack, i)) {
      fprintf(stderr, "Can't push another item!\n");
      return EXIT_FAILURE;
    }
    printf("%d ", stack_top(&stack));
  }

  if (__VERIFIER_nondet_int()) {
    printf("\nPoping items: ");
    while (!stack_empty(&stack)){
      printf("%d ", stack_top(&stack));
      stack_pop(&stack);
    }
    printf("\n");
  }

  stack_clear(&stack);
  return 0;
}

/**
 * @file test-0199.c
 *
 * @brief a stack implementation (contributed by Daniela Ďuričeková)
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
