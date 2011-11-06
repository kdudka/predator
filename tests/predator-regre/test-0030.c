#include <stdlib.h>
#define NEW (item_t) malloc(sizeof(struct item))
typedef struct item {
    struct item *lptr; struct item *rptr;
} *item_t;

item_t alloc_triple(void) {
    item_t lptr, rptr, root = NULL;
    if (NULL == (root = NEW)) return NULL;
    if (NULL == (lptr = NEW)) goto out_of_memory;
    if (NULL == (rptr = NEW)) goto out_of_memory;
    root->lptr = lptr; root->rptr = rptr;
    return root;
out_of_memory:
    free(lptr);
    free(rptr);
    free(root);
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
