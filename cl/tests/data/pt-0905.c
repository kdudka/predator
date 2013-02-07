#include "include/pt.h"

/**
 * passing struct as an parameter to called function
 */

struct list {
    struct list *next;
};

void joinTarget(struct list l1, struct list l2)
{
    l1.next = l2.next;
}

int main(int argc, char **argv)
{
    struct list l1, l2, l3;
    l1.next = &l3;

    // after this, l2 should point to l3 also
    joinTarget(l1, l2);

    ___cl_pt_points_glob_y(l2, l3);
}
