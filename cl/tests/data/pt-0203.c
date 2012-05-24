#include "include/pt.h"

/**
 * list (dynamic structures)
 */

struct list {
    struct list *next;
    void *data;
};

int main()
{
    int data;
    struct list *a, *b, *c;

    b->next = c;
    a->next = b;

    c->data = &data;

    void    *p = &a;

    ___cl_pt_is_pointed_y(a);
    ___cl_pt_is_pointed_n(b);
    ___cl_pt_is_pointed_n(c);
    ___cl_pt_is_pointed_y(data);

    ___cl_pt_points_glob_n(a, b);
    ___cl_pt_points_glob_n(b, c);

    ___cl_pt_points_glob_y(a, data);
    ___cl_pt_points_glob_y(b, data);
    ___cl_pt_points_glob_y(c, data);
    ___cl_pt_points_glob_y(p, data);
}
