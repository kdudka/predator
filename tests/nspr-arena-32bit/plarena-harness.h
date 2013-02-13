#include <verifier-builtins.h>

#define PR_Assert(msg, file, line) ___sl_error(msg)

__attribute__((visibility("default"))) void * PR_Malloc(PRUint32 size)
{
    return malloc(size);
}

__attribute__((visibility("default"))) void * PR_Calloc(PRUint32 nelem,
        PRUint32 elsize)
{
    return calloc(nelem, elsize);
}

__attribute__((visibility("default"))) void * PR_Realloc(void *ptr,
        PRUint32 size)
{
    return realloc(ptr, size);
}

__attribute__((visibility("default"))) void PR_Free(void *ptr)
{
    free(ptr);
}

/* ATTENTION: do not use this simplified model for concurrency analyzers */
__attribute__((visibility("default"))) PRStatus PR_CallOnce(
    PRCallOnceType *once,
    PRCallOnceFN func)
{
    if (once->initialized)
        return once->status;

    // call the function we are requested to call
    const PRStatus status = func();

    // update state info
    once->initialized = 1;
    once->status = status;

    return status;
}

/* ATTENTION: do not use this simplified model for concurrency analyzers */
struct PRLock {
    int locked;
};

/* ATTENTION: do not use this simplified model for concurrency analyzers */
__attribute__((visibility("default"))) PRLock* PR_NewLock(void)
{
    return PR_Calloc(1, sizeof(PRLock));
}

/* ATTENTION: do not use this simplified model for concurrency analyzers */
__attribute__((visibility("default"))) void PR_DestroyLock(PRLock *lock)
{
    PR_Free(lock);
}

/* ATTENTION: do not use this simplified model for concurrency analyzers */
__attribute__((visibility("default"))) void PR_Lock(PRLock *lock)
{
    __VERIFIER_assert(!lock->locked);
    lock->locked = 1;
}

/* ATTENTION: do not use this simplified model for concurrency analyzers */
__attribute__((visibility("default"))) PRStatus PR_Unlock(PRLock *lock)
{
    if (!lock->locked)
        return PR_FAILURE;

    lock->locked = 0;
    return PR_SUCCESS;
}
