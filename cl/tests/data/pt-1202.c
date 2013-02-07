#include "include/pt.h"

int *first_level(int *);

int *third_level(int *ptrThirdL) {
    int *ptrThirdCopy = ptrThirdL;
    return first_level(ptrThirdL);

    ___cl_pt_points_loc_y("ptrThirdCopy", "secondLocal");
}

int *second_level(int *ptrSecondL) {
    int secondLocal = 0;
    int *ptrSecondCopy = ptrSecondL;
    // this should be propagated like:
    //      ==> third_level ==> first_level
    ptrSecondCopy = &secondLocal;

    return third_level(ptrSecondL);

    ___cl_pt_points_loc_y("ptrSecondCopy", "secondLocal");
}

int *first_level(int *ptrFirstL) {
    int *ptrFirstCopy = ptrFirstL;

    return second_level(ptrFirstL);

    ___cl_pt_points_loc_y("ptrFirstCopy", "secondLocal");
}

int main()
{
    int data = 0;
    int *ptrOrigin = &data;
    int *ptrReturned = first_level(ptrOrigin);

    return 0;
}
