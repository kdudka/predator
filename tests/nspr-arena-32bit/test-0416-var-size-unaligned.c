# 2 "test-0416.c"
#include "plarena-decls.h"
#include "plarena-harness.h"
#include <verifier-builtins.h>

/* # 52 "../../../mozilla/nsprpub/lib/ds/plarena.c" 2 */

static PLArena *arena_freelist;
/* # 65 "../../../mozilla/nsprpub/lib/ds/plarena.c" */
static PRLock *arenaLock;
static PRCallOnceType once;
static const PRCallOnceType pristineCallOnce;
/* # 83 "../../../mozilla/nsprpub/lib/ds/plarena.c" */
static PRStatus InitializeArenas( void )
{
    ((arenaLock == ((void *)0))?((void)0):PR_Assert("arenaLock == NULL","../../../mozilla/nsprpub/lib/ds/plarena.c",85));
    arenaLock = PR_NewLock();
    if ( arenaLock == ((void *)0) )
        return PR_FAILURE;
    else
        return PR_SUCCESS;
}

static PRStatus LockArena( void )
{
    PRStatus rc = PR_CallOnce( &once, InitializeArenas );

    if ( PR_FAILURE != rc )
        PR_Lock( arenaLock );
    return(rc);
}

static void UnlockArena( void )
{
    PR_Unlock( arenaLock );
    return;
}

__attribute__((visibility("default"))) void PL_InitArenaPool(
    PLArenaPool *pool, const char *name, PRUint32 size, PRUint32 align)
{




    static const PRUint8 pmasks[33] = {
         0,
         0, 1, 3, 3, 7, 7, 7, 7,15,15,15,15,15,15,15,15,
        31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31};

    if (align == 0)
        align = sizeof(double);

    if (align < sizeof(pmasks)/sizeof(pmasks[0]))
        pool->mask = pmasks[align];
    else
        pool->mask = (((PRUint32)1 << (PR_CeilingLog2(align))) - 1);

    pool->first.next = ((void *)0);
    pool->first.base = pool->first.avail = pool->first.limit =
        (PRUword)(((PRUword)(&pool->first + 1) + (pool)->mask) /* & ~(pool)->mask */);
    pool->current = &pool->first;
    pool->arenasize = size;






}
/* # 168 "../../../mozilla/nsprpub/lib/ds/plarena.c" */
__attribute__((visibility("default"))) void * PL_ArenaAllocate(PLArenaPool *pool, PRUint32 nb)
{
    PLArena *a;
    char *rp;

    (((nb & pool->mask) == 0)?((void)0):PR_Assert("(nb & pool->mask) == 0","../../../mozilla/nsprpub/lib/ds/plarena.c",173));

    nb = (PRUword)(((PRUword)(nb) + (pool)->mask) & ~(pool)->mask);


    {
        a = pool->current;
        do {
            if ( a->avail +nb <= a->limit ) {
                pool->current = a;
                rp = (char *)a->avail;
                a->avail += nb;
                return rp;
            }
        } while( ((void *)0) != (a = a->next) );
    }


    {
        PLArena *p;


        if ( PR_FAILURE == LockArena())
            return(0);

        for ( a = arena_freelist, p = ((void *)0); a != ((void *)0) ; p = a, a = a->next ) {
            if ( a->base +nb <= a->limit ) {
                if ( p == ((void *)0) )
                    arena_freelist = a->next;
                else
                    p->next = a->next;
                UnlockArena();
                a->avail = a->base;
                rp = (char *)a->avail;
                a->avail += nb;


                a->next = pool->current->next;
                pool->current->next = a;
                pool->current = a;
                if ( ((void *)0) == pool->first.next )
                    pool->first.next = a;
                return(rp);
            }
        }
        UnlockArena();
    }


    {
        PRUint32 sz = ((pool->arenasize)>(nb)?(pool->arenasize):(nb));
        sz += sizeof *a + pool->mask;
        a = (PLArena*)(PR_Malloc((sz)));
        if ( ((void *)0) != a ) {
            a->limit = (PRUword)a + sz;
            a->base = a->avail = (PRUword)(((PRUword)(a + 1) + (pool)->mask) /*& ~(pool)->mask*/);
            rp = (char *)a->avail;
            a->avail += nb;


            a->next = pool->current->next;
            pool->current->next = a;
            pool->current = a;
            if ( ((void *)0) == pool->first.next )
                pool->first.next = a;
            ;
            ;
            return(rp);
        }
    }


    return(((void *)0));
}

__attribute__((visibility("default"))) void * PL_ArenaGrow(
    PLArenaPool *pool, void *p, PRUint32 size, PRUint32 incr)
{
    void *newp;

    do { PLArena *_a = (pool)->current; PRUint32 _nb = (((PRUword)(size + incr) + (pool)->mask) & ~(pool)->mask); PRUword _p = _a->avail; PRUword _q = _p + _nb; if (_q > _a->limit) _p = (PRUword)PL_ArenaAllocate(pool, _nb); else _a->avail = _q; newp = (void *)_p; ; } while (0);
    if (newp)
        memcpy(newp, p, size);
    return newp;
}





static void FreeArenaList(PLArenaPool *pool, PLArena *head, PRBool reallyFree)
{
    PLArena **ap, *a;

    ap = &head->next;
    a = *ap;
    if (!a)
        return;


    do {
        ((a->base <= a->avail && a->avail <= a->limit)?((void)0):PR_Assert("a->base <= a->avail && a->avail <= a->limit","../../../mozilla/nsprpub/lib/ds/plarena.c",274));
        a->avail = a->base;
        ((((a)->avail <= (a)->limit)?((void)0):PR_Assert("(a)->avail <= (a)->limit","../../../mozilla/nsprpub/lib/ds/plarena.c",276)), memset((void*)(a)->avail, 0xDA, (a)->limit - (a)->avail));
    } while ((a = a->next) != 0);
    a = *ap;


    if (reallyFree) {
        do {
            *ap = a->next;
            memset((void*)(a), 0xDA, (a)->limit - (PRUword)(a));
            ;
            { PR_Free(a); (a) = ((void *)0); };
        } while ((a = *ap) != 0);
    } else {

        do {
            ap = &(*ap)->next;
        } while (*ap);
        LockArena();
        *ap = arena_freelist;
        arena_freelist = a;
        head->next = 0;
        UnlockArena();
    }

    pool->current = head;
}

__attribute__((visibility("default"))) void PL_ArenaRelease(PLArenaPool *pool, char *mark)
{
    PLArena *a;

    for (a = pool->first.next; a; a = a->next) {
        if (((PRUword)(mark) - (PRUword)(a->base)) < ((PRUword)(a->avail) - (PRUword)(a->base))) {
            a->avail = (PRUword)(((PRUword)(mark) + (pool)->mask) & ~(pool)->mask);
            FreeArenaList(pool, a, 0);
            return;
        }
    }
}

__attribute__((visibility("default"))) void PL_FreeArenaPool(PLArenaPool *pool)
{
    FreeArenaList(pool, &pool->first, 0);
    ;
}

__attribute__((visibility("default"))) void PL_FinishArenaPool(PLArenaPool *pool)
{
    FreeArenaList(pool, &pool->first, 1);
/* # 340 "../../../mozilla/nsprpub/lib/ds/plarena.c" */
}

__attribute__((visibility("default"))) void PL_CompactArenaPool(PLArenaPool *ap)
{
}

__attribute__((visibility("default"))) void PL_ArenaFinish(void)
{
    PLArena *a, *next;

    for (a = arena_freelist; a; a = next) {
        next = a->next;
        { PR_Free(a); (a) = ((void *)0); };
    }
    arena_freelist = ((void *)0);

    if (arenaLock) {
        PR_DestroyLock(arenaLock);
        arenaLock = ((void *)0);
    }
    once = pristineCallOnce;
}

void torture_arena(PLArenaPool *pool)
{
    while (__VERIFIER_nondet_int()) {
        size_t size = __VERIFIER_nondet_int();
        if (size < 0x100)
            abort();
        if (0x1000 < size)
            abort();

        size &= ~0x7;

        __VERIFIER_plot("01-torture_arena", &pool, &size, &arena_freelist);

        PL_ArenaAllocate(pool, size);

        while (__VERIFIER_nondet_int())
            PL_FreeArenaPool(pool);
    }
}

int main()
{
    while (__VERIFIER_nondet_int()) {
        PLArenaPool pool;

        while (__VERIFIER_nondet_int()) {
            // initialize arena pool
            PL_InitArenaPool(&pool, "cool pool", 0x1000, 0x10);

            torture_arena(&pool);

            PL_FreeArenaPool(&pool);
            PL_FinishArenaPool(&pool);
        }

        PL_ArenaFinish();
    }

    return 0;
}

/**
 * @file test-0416-var-size-unaligned.c
 *
 * @brief unaligned variant of test-0415-var-size.c
 *
 *
 * - unaligned allocation on aligned arena pool
 *
 * - assertion failure successfully caught by Predator
 *
 * @attention
 * This description is automatically imported from tests/nspr-arena-32bit/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
