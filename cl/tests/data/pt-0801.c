/**
 * global variable incorporated
 */

#include "include/pt.h"

int *glob;

int main() {
    int * p;
    int   d;

    p = &d;
    p = glob;

    ___cl_pt_points_glob_y(glob, d);
}
