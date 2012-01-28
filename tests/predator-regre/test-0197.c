#include <verifier-builtins.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long TIntAddr;

static const size_t N = 0x1000;

void* do_memset(
        const size_t size_to_alloc,
        const size_t addr_alignment,
        const ssize_t shift_beg,
        const ssize_t shift_end)
{
    TIntAddr addr = (TIntAddr) malloc(size_to_alloc);
    TIntAddr limit = addr + size_to_alloc;
    void *beg = (void *) addr;

    addr += (addr_alignment - 1) + shift_beg;
    addr &= ~(addr_alignment - 1);

    ssize_t size = (limit - addr);
    size += shift_end;
    ___sl_plot("00-before-memset");

    memset((void *) addr, 0, size);
    ___sl_plot("01-after-memset");

    return beg;
}

int main()
{
    char *ptr;

    switch (___sl_get_nondet_int()) {
        case 0:
            // SAFE (without alignment)
            ptr = do_memset(
                    /* size_to_alloc    */ N,
                    /* addr_alignment   */ 1,
                    /* shift_beg        */ 0,
                    /* shift_end        */ 0);
            break;

        case 1:
            // SAFE
            ptr = do_memset(
                    /* size_to_alloc    */ N,
                    /* addr_alignment   */ sizeof(double),
                    /* shift_beg        */ 0,
                    /* shift_end        */ 0);
            break;

        case 2:
            // SAFE, but will trigger assertion failure afterwards
            ptr = do_memset(
                    /* size_to_alloc    */ N,
                    /* addr_alignment   */ sizeof(double),
                    /* shift_beg        */ 1,
                    /* shift_end        */ 0);
            break;

        case 3:
            // SAFE, but will trigger assertion failure afterwards
            ptr = do_memset(
                    /* size_to_alloc    */ N,
                    /* addr_alignment   */ sizeof(double),
                    /* shift_beg        */ 0,
                    /* shift_end        */ -1);
            break;

        case 4:
            // above the allocated area by 1B (without alignment)
            ptr = do_memset(
                    /* size_to_alloc    */ N,
                    /* addr_alignment   */ 1,
                    /* shift_beg        */ -1,
                    /* shift_end        */ -1);
            break;

        case 5:
            // byond the allocated area by 1B (without alignment)
            ptr = do_memset(
                    /* size_to_alloc    */ N,
                    /* addr_alignment   */ 1,
                    /* shift_beg        */ 0,
                    /* shift_end        */ 1);
            break;

        case 6:
            // above the allocated area by 1B
            ptr = do_memset(
                    /* size_to_alloc    */ N,
                    /* addr_alignment   */ sizeof(double),
                    /* shift_beg        */ -1,
                    /* shift_end        */ -1);
            break;

        case 7:
            // byond the allocated area by 1B
            ptr = do_memset(
                    /* size_to_alloc    */ N,
                    /* addr_alignment   */ sizeof(double),
                    /* shift_beg        */ 0,
                    /* shift_end        */ 1);
            break;

        default:
            return 1;
    }

    // NOTE: this triggers some false positives if join is enabled
    ___SL_ASSERT(!ptr[sizeof(double) - 1]);
    ___SL_ASSERT(!ptr[N - sizeof(double)]);
    ___sl_plot("end-of-main-reached", &ptr);

    return 0;
}

/**
 * @file test-0197.c
 *
 * @brief a regression test for execution of memset() on ranges
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
