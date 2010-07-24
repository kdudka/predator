/*
 Jonathan Cederberg:  example based on the eternally confuzzled web tutorial 
 */

#include "../sl.h"

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

        ___sl_plot("01");
	
	delete_list(the_list);

        ___sl_plot("02");
	
	return EXIT_SUCCESS;
}
