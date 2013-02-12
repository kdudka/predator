/*
 Jonathan Cederberg:  example based on the eternally confuzzled web tutorial 
 */

#include <verifier-builtins.h>

#include <stdio.h>
#include <stdlib.h>

struct DLL {
	
  struct DLL *next;
  struct DLL *prev;
	int data;
	
};

/*
  Create a new list with an optional dummy head and tail
  Returns a pointer to the new list, or NULL on error
*/
struct DLL *new_list (int size)
{
  struct DLL *new_node = NULL;
  struct DLL *list = NULL;
  struct DLL *tail = NULL;

  while(size > 0)
  {
	  new_node = (struct DLL *) malloc(sizeof(struct DLL)); // TV: junk according to Predator???
	  if(list == NULL)
	  {

		  list = new_node;
		  tail = new_node;
		  new_node->prev = NULL;

	  } else {

		  tail->next = new_node;
		  new_node->prev = tail;
		  tail = new_node;          // TV: junk according to Predator???
	  }
	  new_node->next = NULL;
	  size--;

	  printf("a number: %d\n", size);
  }

  return list;
}

void print_list(struct DLL *list)
{

	struct DLL *tmp = list;
	while(!(tmp==NULL))
	{
		puts("a number");
		tmp = tmp->next;
	}
}

void delete_list(struct DLL *list)
{
	struct DLL *tmp;
	
	while(list != NULL) {
		
		tmp = list;
		list = list->next;
		free(tmp);
	}
}

int main(void) {
	
	int length = 4;
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */

	struct DLL *the_list = new_list(length);

	print_list(the_list);

        __VERIFIER_plot("01");
	
	delete_list(the_list);

        __VERIFIER_plot("02");
	
	return EXIT_SUCCESS;
}

/**
 * @file test-0082.c
 *
 * @brief a.k.a. jonathan-DLL.c
 *
 * - an example provided by Jonathan Cederberg
 * - seen infinite because we abstract out integral values
 * - does not care about the OOM state, what makes our analysis
 *   a bit complex
 *
 * - works better with -fplugin-arg-libsl-symexec-args=fast
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
