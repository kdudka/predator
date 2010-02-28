#include <stdlib.h>

typedef struct item { struct item *lptr; struct item *rptr; } *item_t;
item_t alloc_item(void) { return (item_t) malloc(sizeof(struct item)); }
item_t alloc_triple(void) {
    item_t root, lptr, rptr = NULL;
    if (NULL == (lptr = alloc_item()))
        goto out_of_memory;
    if (NULL == (rptr = alloc_item()))
        goto out_of_memory;
    if (NULL == (root = alloc_item()))
        goto out_of_memory;
    root->lptr = lptr;
    root->rptr = rptr;
    return root;

out_of_memory:
    free(root);
    free(lptr);
    free(rptr);
    return NULL;
}

int main() {
    item_t item = alloc_triple();
    if (item) {
        free(item->lptr);
        free(item->rptr);
        free(item);
    }
    return 0;
}
