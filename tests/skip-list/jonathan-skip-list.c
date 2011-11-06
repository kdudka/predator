/*
 *  skiplist.c
 *  
 *
 *  Created by Jonathan Cederberg on 2010-05-19.
 *  Copyright 2010 Uppsala Universitet. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX 100
#define DEBUG 1

struct SL {
	
	int data;
	struct SL *next;
	struct SL *down;
};

struct jsw_skip {
	
	struct SL *head;
	struct SL *bottom;
	int height;
};

struct SL *new_node(int data, struct SL *next, struct SL *down) // my code
{
	struct SL *res = (struct SL *) malloc(sizeof(struct SL));
	
	res->data = data;
	res->down = down;
	res->next = next;

	return res;
}

struct jsw_skip *new_skip() // my code
{
	struct jsw_skip *res = (struct jsw_skip *) malloc(sizeof(struct jsw_skip));
	struct SL *_node = new_node(-1, NULL, NULL);

	res->height = 1;
	res->head = _node;
	res->bottom = _node;
	
	return res;
}

int is_empty(struct jsw_skip *to_test) // my code
{
	if(to_test->head == NULL)
	{
		return 1;
	}
	return 0;
}

/*
 int insert_into_empty(struct jsw_skip *skip, int key)
{
	
	struct SL *new_node = (struct SL *) malloc(sizeof(struct SL));
	
	skip->head = new_node;
	skip->bottom = new_node;
	new_node->next = NULL;
	new_node->down = NULL;
	new_node->data = key;
	(skip->height)++;
	return 1;
}
 */

int rheight ( int max )
{
	int h = 1; /* Never return 0 */
	
	while ( h < max && rand() < RAND_MAX / 2 )
		++h;
	
	return h;
}

int jsw_find ( struct jsw_skip *skip, int key )
{
	if(DEBUG) printf("Calling insert with key %d\n", key);

	struct SL *it = skip->head;
	
	while ( it->down != NULL ) {
		if(DEBUG) puts("Blabla5");
		while ( it->next != NULL && key > it->next->data )
			it = it->next;
		it = it->down;
	}
	
	return it->next ? it->next->data : -1;
}

int jsw_insert ( struct jsw_skip *skip, int key )
{
	if(DEBUG) printf("Calling insert with key %d\n", key);
	
	if ( jsw_find ( skip, key ) == key )
	{
		if(DEBUG) puts("key already there!");
		return 0;
		
	} else {
		
		struct SL *it, *save = NULL;
		int h = rheight ( MAX );
		int i;
		
		if(DEBUG) printf("Calling insert with key %d\n", key);

		if ( h > skip->height ) {
			skip->head = new_node ( -1, NULL, skip->head );
			++skip->height;
		}
		
		it = skip->head;
		
		for ( i = skip->height; it != NULL; i-- ) {
			while ( it->next != NULL && key > it->next->data )
				it = it->next;
			
			if ( i <= h ) {
				if ( it->next == NULL || key != it->next->data )
					it->next = new_node ( key, it->next, NULL );
				
				if ( save != NULL )
					save->down = it->next;
				
				save = it->next;
			}
			
			it = it->down;
		}
	}
	return 1;
}

int jsw_remove ( struct jsw_skip *skip, int key )
{
	if ( jsw_find ( skip, key ) != key )
		return 0;
	else {
		struct SL *it = skip->head;
		struct SL *save = NULL;
		
		while ( it != NULL ) {
			while ( it->next != NULL && key > it->next->data )
				it = it->next;
			
			if ( it->next != NULL && key == it->next->data ) {
				save = it->next;
				it->next = save->next;
				free ( save );
			}
			
			it = it->down;
		}
	}
	
	return 1;
}

struct jsw_skip *make_list(int *vals, int n_vals) // my code
{
	
	struct jsw_skip *res = new_skip();
	if(DEBUG) puts("Blabla2");
	
	while(n_vals>0)
	{
		if(DEBUG) puts("Blabla3");
		jsw_insert(res, vals[n_vals-1]);
		if(DEBUG) puts("Blabla4");
		
		n_vals--;
	}	
	
	return res;
}

struct SL *find_second_level(struct jsw_skip *the_list) // my code
{
	// assumes there are at least 2 levels in the list
	
	struct SL *tmp = the_list->head;
	
	while(tmp != the_list->bottom)
	{
		tmp = tmp->down;
	}
	
	return tmp;
}

void destroy_list(struct jsw_skip *to_destroy) // my code
{
	struct SL *tmp, *tmp2;
	struct SL *next = to_destroy->head;
	
	while(next!=NULL)
	{
		tmp = next;
		next = next->down;
		
		while(tmp!=NULL)
		{
			tmp2 = tmp->next;
			free(tmp);
			tmp = tmp2;
		}
	}
}

void print_list(struct jsw_skip *the_list) // my code
{
	
	// prints list
	if(DEBUG) printf("Head data: %d\n", the_list->head->data);

	struct SL *tmp;
	struct SL *next_level = the_list->head;
	
	while(next_level!=NULL) {
		
		tmp=next_level;
		while(tmp != NULL) {
			
			printf("%d -> ", tmp->data);
			tmp=tmp->next;
		}
		printf("#\n");
		next_level = next_level->down;
	}
}

int main(void) // my code
{
	
	puts("Starting program");
	
	int elems = 5;
	
	int numbers[] = {1,5,6,23,7};
	
	struct jsw_skip *my_list  = make_list(numbers, elems);
	
	puts("List created:");
	
	printf("head: %d\n", (int) my_list);
	
	print_list(my_list);
	
	puts("Destroying list:");
	
	destroy_list(my_list);
	
	puts("List destroyed:");
	
	printf("head: %d\n", (int) my_list);
}