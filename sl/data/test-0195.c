#include "../sl.h"

#include <stdbool.h>
#include <stdlib.h>

struct node {
    char *text;
    struct node *next;
};

static void insert_node(struct node **pn, char c)
{
    struct node *n = malloc(sizeof *n);
    if (c) {
        n->text = calloc(2, sizeof(char));
        n->text[0] = c;
    }
    else
        n->text = NULL;

    n->next = *pn;
    *pn = n;
}

int main()
{
    struct node *list = NULL;

    insert_node(&list, 'A');
    insert_node(&list, '\0');
    insert_node(&list, 'A');

    return 0;
}
