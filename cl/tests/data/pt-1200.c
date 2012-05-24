#include "include/pt.h"

void third_level(int *ptrThirdL) {
    int *ptrThirdCopy = ptrThirdL;

    ___cl_pt_points_loc_n("ptrThirdCopy", "data");
}

void second_level(int *ptrSecondL) {
    int *ptrSecondCopy = ptrSecondL;

    ___cl_pt_points_loc_y("ptrSecondCopy", "data");
}

void first_level(int *ptrFirstL) {
    int *ptrFirstCopy = ptrFirstL;
    second_level(ptrFirstL);

    ___cl_pt_points_loc_y("ptrFirstCopy", "data");
}

int main()
{
    int data = 0;
    int *ptrOrigin = &data;

    first_level(ptrOrigin);
}
