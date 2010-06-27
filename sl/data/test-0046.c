#include "../sl.h"

int main()
{
    int a = 1;
    int b = 2;

    if (a == b)
        return 1;

    // check if Neq predicates are still traversed and plotted correctly
    ___sl_plot_by_ptr(&a, "test-0046-a");
    ___sl_plot_by_ptr(&b, "test-0046-b");

    return 0;
}
