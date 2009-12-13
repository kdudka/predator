#include <stdlib.h>

int main() {
    void *undef;
    void *val = NULL;
    void *ptr = &val;

    free(undef);
    free(val);
    free(ptr);

    void *item = malloc(sizeof(void *));
    free(item);
    free(item);

    return 0;
}
