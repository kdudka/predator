#include <stdlib.h>

#define TRIGGER_INV_BUG 1

int main()
{
    typedef struct sll_item sll_item_t;
    struct sll_item {
        sll_item_t *next;
    };

    sll_item_t array_static[7];
    sll_item_t *array_dynamic = (sll_item_t *) malloc(7 * sizeof(sll_item_t));

    {
        int i;
#if TRIGGER_INV_BUG
        for (i = 0; i /* off-by-one error */ <= 7; ++i)
#else
        for (i = 0; i < 7; ++i)
#endif
        {
            array_static[i].next = NULL; 
            array_dynamic[i].next = NULL; 
        }
    }

    free(array_dynamic);

    return 0;
}
