#include "../sl.h"
#include <stdlib.h>

int main() {
    struct item {
        struct item *next;
    } *ptr = NULL;

    for(;;) {
        void *data = ptr;
        ptr = malloc(sizeof *ptr);
        if (!ptr)
            // OOM
            return -1;

        ptr->next = data;
        ___sl_plot("test-0001-snapshot");
    }

    return 0;
}
