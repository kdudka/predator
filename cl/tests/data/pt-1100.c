#include "include/pt.h"

extern int *j;
extern int *k;

extern void forceBlackHole();

void touch()
{
    int a, b;
    j = &a;
    k = &b;
}

int main()
{
    int i = 0;

    touch();

    // this is going to make black hole
    forceBlackHole();

    j = &i;

    ___cl_pt_points_loc_y("k", "i");
}

/**
 * note that after phase 2 there is not joined 'k' and 'j' in touch's graph --
 * that is correct adn don't worry about it.  Anyway we have to ask everytime
 * the global PT-graph also for PT info.
 */
