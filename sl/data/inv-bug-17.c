#include <stdio.h>
#include <stdlib.h>

#define top_level 10
#define back_level 10

#define NEW(type) \
    (type *) malloc(sizeof(type))

struct list_item {
    struct list_item *next_up;
    struct list_item *next_low;
    int seq;
};

void attach_lower(struct list_item *low, int seq) {
    int j;
    struct list_item *low_beg = low;
    for(j = 1; j < back_level; j++) {
        struct list_item *bl = NEW(struct list_item);
        bl->seq = seq + j;
        low->next_low = bl;
        low = bl;
    }
    low->next_low = low_beg;
}

void printelm_up(struct list_item **up, int go) {
    int i;
    printf("upper level: ");
    for (i = 1; i < go; i++){
        printf("%i:%i ", i, (*up)->seq);
        *up = (*up)->next_up;
    }
    printf("\n");
}

void printelm_low(struct list_item *low, int go) {
    int i;
    printf("lower level: ");
    for(i = 1; i < go; i++) {
        printf("%i:%i ", i, low->seq);
        low = low->next_low;
    }
    printf("\n");
}

void printelm(struct list_item *up, int go_up, int go_low) {
    printelm_up(&up, go_up);
    printelm_low(up->next_low, go_low);
}

int main() {
    int i;

    /* create first item */
    struct list_item *up_beg = NEW(struct list_item);
    struct list_item *up = up_beg;
    up->seq = 0;
    attach_lower(up, 1000);

    /* create other items */
    for(i = 1; i < top_level; i++) {
        struct list_item *low = NEW(struct list_item);
        low->seq = i;
        up->next_up = low;
        up = low;
        attach_lower(up, i * 1000);
    }

    /* make upper list cyclic */
    up->next_up = up_beg;

    /* print something */
    printelm(up_beg, 100, 20);

    /* destroy all allocated items */
    up = up_beg;
    do {
        struct list_item *up_next = up->next_up;

        /* destroy all items except the first from the lower list */
        struct list_item *low = up->next_low;
        while (low != up) {
            struct list_item *low_next = low->next_low;
            free(low);
            low = low_next;
        }

        free(up);
        up = up_next;
    } while (up != up_beg);

    return 0;
}
