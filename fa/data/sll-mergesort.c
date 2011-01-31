#include <stdlib.h>

int __nondet();

struct DItem {
	struct DItem* next;
	int value;
};

struct TLItem {
	struct TLItem* next;
	struct DItem* data;	
};

int main() {

	struct TLItem* data = NULL;
	struct DItem* item, * item2;
	struct TLItem *lItem;

	while (__nondet()) {

          item = malloc(sizeof *item);
          item->next = NULL;
          item->value = __nondet();

          lItem = malloc(sizeof *lItem);
	  if (data) {
            lItem->next = data->next;
	    data->next = lItem;
	  } else {
	    lItem->next = lItem;
	    data = lItem;
	  }
          lItem->data = item;

	  item = NULL;
	  lItem = NULL;

        }
	
	if (!data)
	  return 0;

	while (data->next != data) {

	  item = data->data;
	  item2 = data->next->data;

	  lItem = data->next;
	  data->next = lItem->next;
	  free(lItem);

	  if (__nondet()) {  
	    data->data = item;
	    item = item->next;
	  } else {
	    data->data = item2;
	    item2 = item2->next;
	  }

	  struct DItem* dst = data->data;

	  while (item && item2) {

	    if (__nondet()) {
	      dst->next = item;
	      item = item->next;
	    } else {
	      dst->next = item2;
	      item2 = item2->next;
	    }

	    dst = dst->next;

	  }

	  if (item) {
	    dst->next = item;
	    item = NULL;
	  }

	  if (item2) {
	    dst->next = item2;
	    item2 = NULL;
	  }

	  dst = NULL;
	  data = data->next;

	}

	item = data->data;
	free(data);

	while (item) {
	  item2 = item;
	  item = item->next;
	  free(item2);
	}

	return 0;

}
