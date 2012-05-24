#include "include/pt.h"

int *third_level(int *ptrThirdL) {
    int *ptrThirdCopy = ptrThirdL;

    ___cl_pt_exists_local_n("data");

    return 0;
}

int *second_level(int *ptrSecondL) {
    int *ptrSecondCopy = ptrSecondL;

    ___cl_pt_exists_local_y("data");
    ___cl_pt_points_loc_y("ptrSecondCopy", "data");

    return 0;
}

int *first_level(int *ptrFirstL) {
    int *ptrFirstCopy = ptrFirstL;
    second_level(ptrFirstL);

    ___cl_pt_points_loc_y("ptrFirstCopy", "data");
    ___cl_pt_points_loc_y("ptrFirstL", "data");

    ___cl_pt_exists_local_n("ptrReturned");

    return 0;
}

int main()
{
    int data = 0;
    int *ptrOrigin = &data;
    int *ptrReturned = first_level(ptrOrigin);
}
