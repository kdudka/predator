#include "include/pt.h"

/**
 * Basic pointer operations with static/dynamic structures
 */

struct list {
    struct list *next;
    int data;
};

int main()
{
    void *p;
    struct list staticList;
    struct list *dynamicList;

    dynamicList->next = &staticList;
    p = &dynamicList->data; // this does not taking pointer to dynamicList!

    ___cl_pt_is_pointed_n(dynamicList);
    ___cl_pt_is_pointed_y(staticList);

    ___cl_pt_points_glob_y(p,               staticList);
    ___cl_pt_points_glob_y(dynamicList,     staticList);
    ___cl_pt_points_glob_n(staticList,      dynamicList);
}
