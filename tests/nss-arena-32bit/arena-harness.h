#include <verifier-builtins.h>
#include "../nspr-arena-32bit/plarena-harness.h"
#include "../nspr-arena-32bit/plarena-bare.c"

void* nsslibc_memset(void *dest, PRUint8 byte, PRUint32 n)
{
    return memset(dest, (int)byte, (size_t)n);
}

typedef struct error_stack_str error_stack;

void nss_ClearErrorStack(void)
{
    /* TODO */
#if 0
    error_stack *es = error_get_my_stack();
    if( (error_stack *)((void *)0) == es )
        return;

    es->header.count = 0;
    es->stack[0] = 0;
#endif
}

void nss_SetError(PRUint32 error)
{
    error_stack *es;

    if( 0 == error ) {
        nss_ClearErrorStack();
        return;
    }

    /* TODO */
#if 0
    es = error_get_my_stack();
    if( (error_stack *)((void *)0) == es )
        return;

    if (es->header.count < es->header.space) {
        es->stack[ es->header.count++ ] = error;
    } else {
        memmove(es->stack, es->stack + 1,
                (es->header.space - 1) * (sizeof es->stack[0]));
        es->stack[ es->header.space - 1 ] = error;
    }
#endif
}
