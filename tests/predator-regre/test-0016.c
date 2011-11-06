#include <stdlib.h>

#define DLL_NEW(type) \
    ((type *) malloc(sizeof(type)))

typedef struct sll_item sll_item_t;
struct sll_item {
    sll_item_t *next;
};

static sll_item_t* create_item(void)
{
    sll_item_t *item = DLL_NEW(sll_item_t);
    if (!item)
        abort();

    return item;
}

int main()
{
    sll_item_t *item = create_item();

    /* The following line kills our garbage collector and the analysis ends up
     * in an infinite loop */
#if 0
    sll_item_t *ptr = item;
#endif

    int i;
    for (i = 0; i < 128; ++i) {
        item->next = create_item();
        item = item->next;
    }

    return 0;
}
