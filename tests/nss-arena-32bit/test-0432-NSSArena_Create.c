# 2 "test-0432.c"
#include "arena-decls.h"
#include "arena-harness.h"

/* # 59 "arena.c" 2 */
/* # 89 "arena.c" */
struct NSSArenaStr {
  PLArenaPool pool;
  PRLock *lock;
/* # 101 "arena.c" */
};







struct nssArenaMarkStr {
  PRUint32 magic;
  void *mark;







};
/* # 379 "arena.c" */
 NSSArena *
NSSArena_Create
(
  void
)
{
  nss_ClearErrorStack();
  return nssArena_Create();
}
/* # 404 "arena.c" */
 NSSArena *
nssArena_Create
(
  void
)
{
  NSSArena *rv = (NSSArena *)((void *)0);

  rv = ((NSSArena *)nss_ZAlloc(((NSSArena *)((void *)0)), sizeof(NSSArena)));
  if( (NSSArena *)((void *)0) == rv ) {
    nss_SetError(NSS_ERROR_NO_MEMORY);
    return (NSSArena *)((void *)0);
  }

  rv->lock = PR_NewLock();
  if( (PRLock *)((void *)0) == rv->lock ) {
    (void)nss_ZFreeIf(rv);
    nss_SetError(NSS_ERROR_NO_MEMORY);
    return (NSSArena *)((void *)0);
  }
/* # 442 "arena.c" */
  PL_InitArenaPool(&rv->pool, "NSS", 2048, sizeof(double));
/* # 457 "arena.c" */
  return rv;
}
/* # 476 "arena.c" */
 PRStatus
NSSArena_Destroy
(
  NSSArena *arena
)
{
  nss_ClearErrorStack();







  return nssArena_Destroy(arena);
}
/* # 509 "arena.c" */
 PRStatus
nssArena_Destroy
(
  NSSArena *arena
)
{
  PRLock *lock;







  if( (PRLock *)((void *)0) == arena->lock ) {

    nss_SetError(NSS_ERROR_INVALID_ARENA);
    return PR_FAILURE;
  }
  PR_Lock(arena->lock);
/* # 542 "arena.c" */
  PL_FinishArenaPool(&arena->pool);
  lock = arena->lock;
  arena->lock = (PRLock *)((void *)0);
  PR_Unlock(lock);
  PR_DestroyLock(lock);
  (void)nss_ZFreeIf(arena);
  return PR_SUCCESS;
}

static void *nss_zalloc_arena_locked(NSSArena *arena, PRUint32 size);
/* # 574 "arena.c" */
 nssArenaMark *
nssArena_Mark
(
  NSSArena *arena
)
{
  nssArenaMark *rv;
  void *p;







  if( (PRLock *)((void *)0) == arena->lock ) {

    nss_SetError(NSS_ERROR_INVALID_ARENA);
    return (nssArenaMark *)((void *)0);
  }
  PR_Lock(arena->lock);
/* # 611 "arena.c" */
  p = ((void *) (&arena->pool)->current->avail);



  rv = (nssArenaMark *)nss_zalloc_arena_locked(arena, sizeof(nssArenaMark));
  if( (nssArenaMark *)((void *)0) == rv ) {
    PR_Unlock(arena->lock);
    nss_SetError(NSS_ERROR_NO_MEMORY);
    return (nssArenaMark *)((void *)0);
  }
/* # 632 "arena.c" */
  rv->mark = p;
  rv->magic = 0x4d41524b;





  PR_Unlock(arena->lock);

  return rv;
}
/* # 651 "arena.c" */
static PRStatus
nss_arena_unmark_release
(
  NSSArena *arena,
  nssArenaMark *arenaMark,
  PRBool release
)
{
  void *inner_mark;







  if( 0x4d41524b != arenaMark->magic ) {
    nss_SetError(NSS_ERROR_INVALID_ARENA_MARK);
    return PR_FAILURE;
  }

  if( (PRLock *)((void *)0) == arena->lock ) {

    nss_SetError(NSS_ERROR_INVALID_ARENA);
    return PR_FAILURE;
  }
  PR_Lock(arena->lock);
/* # 689 "arena.c" */
  if( 0x4d41524b != arenaMark->magic ) {

    PR_Unlock(arena->lock);
    nss_SetError(NSS_ERROR_INVALID_ARENA_MARK);
    return PR_FAILURE;
  }

  arenaMark->magic = 0;
  inner_mark = arenaMark->mark;
/* # 729 "arena.c" */
  if( release ) {
/* # 740 "arena.c" */
    do { char *_m = (char *)(inner_mark); PLArena *_a = (&arena->pool)->current; if (((PRUword)(_m) - (PRUword)(_a->base)) <= ((PRUword)(_a->avail) - (PRUword)(_a->base))) { _a->avail = (PRUword)(((PRUword)(_m) + (&arena->pool)->mask) & ~(&arena->pool)->mask); ; ; } else { PL_ArenaRelease(&arena->pool, _m); } ; } while (0);

  }

  PR_Unlock(arena->lock);
  return PR_SUCCESS;
}
/* # 767 "arena.c" */
 PRStatus
nssArena_Release
(
  NSSArena *arena,
  nssArenaMark *arenaMark
)
{
  return nss_arena_unmark_release(arena, arenaMark, 1);
}
/* # 799 "arena.c" */
 PRStatus
nssArena_Unmark
(
  NSSArena *arena,
  nssArenaMark *arenaMark
)
{
  return nss_arena_unmark_release(arena, arenaMark, 0);
}
/* # 817 "arena.c" */
struct pointer_header {
  NSSArena *arena;
  PRUint32 size;
};

static void *
nss_zalloc_arena_locked
(
  NSSArena *arena,
  PRUint32 size
)
{
  void *p;
  void *rv;
  struct pointer_header *h;
  PRUint32 my_size = size + sizeof(struct pointer_header);
  do { PLArena *_a = (&arena->pool)->current; PRUint32 _nb = (((PRUword)(my_size) + (&arena->pool)->mask) & ~(&arena->pool)->mask); PRUword _p = _a->avail; PRUword _q = _p + _nb; if (_q > _a->limit) _p = (PRUword)PL_ArenaAllocate(&arena->pool, _nb); else _a->avail = _q; p = (void *)_p; ; } while (0);
  if( (void *)((void *)0) == p ) {
    nss_SetError(NSS_ERROR_NO_MEMORY);
    return (void *)((void *)0);
  }





  h = (struct pointer_header *)p;
  h->arena = arena;
  h->size = size;
  rv = (void *)((char *)h + sizeof(struct pointer_header));
  (void)nsslibc_memset(rv, 0, size);
  return rv;
}
/* # 874 "arena.c" */
 void *
nss_ZAlloc
(
  NSSArena *arenaOpt,
  PRUint32 size
)
{
  struct pointer_header *h;
  PRUint32 my_size = size + sizeof(struct pointer_header);

  if( my_size < sizeof(struct pointer_header) ) {

    nss_SetError(NSS_ERROR_NO_MEMORY);
    return (void *)((void *)0);
  }

  if( (NSSArena *)((void *)0) == arenaOpt ) {

    h = (struct pointer_header *)PR_Calloc(1, my_size);
    if( (struct pointer_header *)((void *)0) == h ) {
      nss_SetError(NSS_ERROR_NO_MEMORY);
      return (void *)((void *)0);
    }

    h->arena = (NSSArena *)((void *)0);
    h->size = size;


    return (void *)((char *)h + sizeof(struct pointer_header));
  } else {
    void *rv;







    if( (PRLock *)((void *)0) == arenaOpt->lock ) {

      nss_SetError(NSS_ERROR_INVALID_ARENA);
      return (void *)((void *)0);
    }
    PR_Lock(arenaOpt->lock);
/* # 929 "arena.c" */
    rv = nss_zalloc_arena_locked(arenaOpt, size);

    PR_Unlock(arenaOpt->lock);
    return rv;
  }

}
/* # 955 "arena.c" */
 PRStatus
nss_ZFreeIf
(
  void *pointer
)
{
  struct pointer_header *h;

  if( (void *)((void *)0) == pointer ) {
    return PR_SUCCESS;
  }

  h = (struct pointer_header *)((char *)pointer
    - sizeof(struct pointer_header));



  if( (NSSArena *)((void *)0) == h->arena ) {

    (void)nsslibc_memset(pointer, 0, h->size);
    PR_Free(h);
    return PR_SUCCESS;
  } else {







    if( (PRLock *)((void *)0) == h->arena->lock ) {

      nss_SetError(NSS_ERROR_INVALID_POINTER);
      return PR_FAILURE;
    }
    PR_Lock(h->arena->lock);

    (void)nsslibc_memset(pointer, 0, h->size);



    PR_Unlock(h->arena->lock);
    return PR_SUCCESS;
  }

}
/* # 1022 "arena.c" */
extern void *
nss_ZRealloc
(
  void *pointer,
  PRUint32 newSize
)
{
  NSSArena *arena;
  struct pointer_header *h, *new_h;
  PRUint32 my_newSize = newSize + sizeof(struct pointer_header);
  void *rv;

  if( my_newSize < sizeof(struct pointer_header) ) {

    nss_SetError(NSS_ERROR_NO_MEMORY);
    return (void *)((void *)0);
  }

  if( (void *)((void *)0) == pointer ) {
    nss_SetError(NSS_ERROR_INVALID_POINTER);
    return (void *)((void *)0);
  }

  h = (struct pointer_header *)((char *)pointer
    - sizeof(struct pointer_header));



  if( newSize == h->size ) {

    return pointer;
  }

  arena = h->arena;
  if (!arena) {

    new_h = (struct pointer_header *)PR_Calloc(1, my_newSize);
    if( (struct pointer_header *)((void *)0) == new_h ) {
      nss_SetError(NSS_ERROR_NO_MEMORY);
      return (void *)((void *)0);
    }

    new_h->arena = (NSSArena *)((void *)0);
    new_h->size = newSize;
    rv = (void *)((char *)new_h + sizeof(struct pointer_header));

    if( newSize > h->size ) {
      (void)nsslibc_memcpy(rv, pointer, h->size);
      (void)nsslibc_memset(&((char *)rv)[ h->size ],
                           0, (newSize - h->size));
    } else {
      (void)nsslibc_memcpy(rv, pointer, newSize);
    }

    (void)nsslibc_memset(pointer, 0, h->size);
    h->size = 0;
    PR_Free(h);

    return rv;
  } else {
    void *p;







    if (!arena->lock) {

      nss_SetError(NSS_ERROR_INVALID_POINTER);
      return (void *)((void *)0);
    }
    PR_Lock(arena->lock);
/* # 1107 "arena.c" */
    if( newSize < h->size ) {
/* # 1120 "arena.c" */
      char *extra = &((char *)pointer)[ newSize ];
      (void)nsslibc_memset(extra, 0, (h->size - newSize));
      PR_Unlock(arena->lock);
      return pointer;
    }

    do { PLArena *_a = (&arena->pool)->current; PRUint32 _nb = (((PRUword)(my_newSize) + (&arena->pool)->mask) & ~(&arena->pool)->mask); PRUword _p = _a->avail; PRUword _q = _p + _nb; if (_q > _a->limit) _p = (PRUword)PL_ArenaAllocate(&arena->pool, _nb); else _a->avail = _q; p = (void *)_p; ; } while (0);
    if( (void *)((void *)0) == p ) {
      PR_Unlock(arena->lock);
      nss_SetError(NSS_ERROR_NO_MEMORY);
      return (void *)((void *)0);
    }

    new_h = (struct pointer_header *)p;
    new_h->arena = arena;
    new_h->size = newSize;
    rv = (void *)((char *)new_h + sizeof(struct pointer_header));
    if (rv != pointer) {
 (void)nsslibc_memcpy(rv, pointer, h->size);
 (void)nsslibc_memset(pointer, 0, h->size);
    }
    (void)nsslibc_memset(&((char *)rv)[ h->size ], 0, (newSize - h->size));
    h->arena = (NSSArena *)((void *)0);
    h->size = 0;
    PR_Unlock(arena->lock);
    return rv;
  }

}

PRStatus
nssArena_Shutdown(void)
{
  PRStatus rv = PR_SUCCESS;



  return rv;
}

int main()
{
    NSSArena *arena = NSSArena_Create();
    __VERIFIER_plot("NSSArena_Create", &arena);

    return 0;
}
