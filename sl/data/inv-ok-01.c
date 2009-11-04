#include <stdlib.h>

typedef struct list_lower_s {
    struct list_lower_s *next_low;
} list_lower_t;

typedef struct list_upper_s {
    struct list_upper_s *next_up;
    struct list_lower_s list_low;
} list_upper_t;

int main() {
    list_upper_t *a;
    list_lower_t *al, *bl, *cl;
    int result;

    a=(list_upper_t *) malloc(sizeof(list_upper_t));
    a->next_up=a;

    al=&(a->list_low);	

    bl=(list_lower_t *) malloc(sizeof(list_lower_t));

    bl->next_low=al;
    al->next_low=bl; 
    cl = (bl->next_low)->next_low;

    if (cl==bl) result=1;   // Correct: one should go here.
    else result=0;          // Wrong: one should not get here.

    free(a);
    free(bl);
    return result;
}
