#include <verifier-builtins.h>

typedef enum {
    LIST_BEG,
    LIST_END
} end_point_t;

end_point_t rand_end_point(void)
{
    return !!__VERIFIER_nondet_int();
}

int main()
{
    const end_point_t ep = rand_end_point();
    __VERIFIER_plot((void *) 0);

    switch (ep) {
        case LIST_BEG:
        case LIST_END:
            return 0;
    }

    ___sl_error("TODO: plesse improve the handling of (VO_UNKNOWN != 0) expr");
    return 0;
}
