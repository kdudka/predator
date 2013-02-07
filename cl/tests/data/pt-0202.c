#include "include/pt.h"

/**
 * list (static structures)
 */

struct list {
    struct list *next;
    int data;
};

int main()
{
    struct list a, b, c;

    a.next = &b;
    b.next = &c;

    void *p = &a;

    ___cl_pt_points_glob_y(a, b);
    ___cl_pt_points_glob_y(a, c);
    ___cl_pt_points_glob_y(b, c);
    ___cl_pt_points_glob_y(p, c);

    ___cl_pt_points_glob_n(c, b);
    ___cl_pt_points_glob_n(b, a);

    ___cl_pt_is_pointed_n(p);
    ___cl_pt_is_pointed_y(a);
    ___cl_pt_is_pointed_y(b);
    ___cl_pt_is_pointed_y(c);
}
