# 2 "plarena-decls.h"
/* # 1 "../../../mozilla/nsprpub/lib/ds/plarena.c" */
/* # 1 "/home/kdudka/cvs/target.debug/lib/ds//" */
/* # 1 "<built-in>" */
/* # 1 "<command-line>" */
/* # 1 "../../../mozilla/nsprpub/lib/ds/plarena.c" */
/* # 44 "../../../mozilla/nsprpub/lib/ds/plarena.c" */
/* # 1 "/usr/include/stdlib.h" 1 3 4 */
/* # 25 "/usr/include/stdlib.h" 3 4 */
/* # 1 "/usr/include/features.h" 1 3 4 */
/* # 356 "/usr/include/features.h" 3 4 */
/* # 1 "/usr/include/sys/cdefs.h" 1 3 4 */
/* # 353 "/usr/include/sys/cdefs.h" 3 4 */
/* # 1 "/usr/include/bits/wordsize.h" 1 3 4 */
/* # 354 "/usr/include/sys/cdefs.h" 2 3 4 */
/* # 357 "/usr/include/features.h" 2 3 4 */
/* # 380 "/usr/include/features.h" 3 4 */
/* # 1 "/usr/include/gnu/stubs.h" 1 3 4 */



/* # 1 "/usr/include/bits/wordsize.h" 1 3 4 */
/* # 5 "/usr/include/gnu/stubs.h" 2 3 4 */


/* # 1 "/usr/include/gnu/stubs-32.h" 1 3 4 */
/* # 8 "/usr/include/gnu/stubs.h" 2 3 4 */
/* # 381 "/usr/include/features.h" 2 3 4 */
/* # 26 "/usr/include/stdlib.h" 2 3 4 */







/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.1/include/stddef.h" 1 3 4 */
/* # 212 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.1/include/stddef.h" 3 4 */
typedef unsigned int size_t;
/* # 324 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.1/include/stddef.h" 3 4 */
typedef int wchar_t;
/* # 34 "/usr/include/stdlib.h" 2 3 4 */








/* # 1 "/usr/include/bits/waitflags.h" 1 3 4 */
/* # 43 "/usr/include/stdlib.h" 2 3 4 */
/* # 1 "/usr/include/bits/waitstatus.h" 1 3 4 */
/* # 65 "/usr/include/bits/waitstatus.h" 3 4 */
/* # 1 "/usr/include/endian.h" 1 3 4 */
/* # 37 "/usr/include/endian.h" 3 4 */
/* # 1 "/usr/include/bits/endian.h" 1 3 4 */
/* # 38 "/usr/include/endian.h" 2 3 4 */
/* # 61 "/usr/include/endian.h" 3 4 */
/* # 1 "/usr/include/bits/byteswap.h" 1 3 4 */
/* # 28 "/usr/include/bits/byteswap.h" 3 4 */
/* # 1 "/usr/include/bits/wordsize.h" 1 3 4 */
/* # 29 "/usr/include/bits/byteswap.h" 2 3 4 */
/* # 62 "/usr/include/endian.h" 2 3 4 */
/* # 66 "/usr/include/bits/waitstatus.h" 2 3 4 */

union wait
  {
    int w_status;
    struct
      {

 unsigned int __w_termsig:7;
 unsigned int __w_coredump:1;
 unsigned int __w_retcode:8;
 unsigned int:16;







      } __wait_terminated;
    struct
      {

 unsigned int __w_stopval:8;
 unsigned int __w_stopsig:8;
 unsigned int:16;






      } __wait_stopped;
  };
/* # 44 "/usr/include/stdlib.h" 2 3 4 */
/* # 68 "/usr/include/stdlib.h" 3 4 */
typedef union
  {
    union wait *__uptr;
    int *__iptr;
  } __WAIT_STATUS __attribute__ ((__transparent_union__));
/* # 96 "/usr/include/stdlib.h" 3 4 */


typedef struct
  {
    int quot;
    int rem;
  } div_t;



typedef struct
  {
    long int quot;
    long int rem;
  } ldiv_t;







__extension__ typedef struct
  {
    long long int quot;
    long long int rem;
  } lldiv_t;


/* # 140 "/usr/include/stdlib.h" 3 4 */
extern size_t __ctype_get_mb_cur_max (void) __attribute__ ((__nothrow__)) ;




extern double atof (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;

extern int atoi (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;

extern long int atol (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;





__extension__ extern long long int atoll (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;





extern double strtod (__const char *__restrict __nptr,
        char **__restrict __endptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;





extern float strtof (__const char *__restrict __nptr,
       char **__restrict __endptr) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

extern long double strtold (__const char *__restrict __nptr,
       char **__restrict __endptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;





extern long int strtol (__const char *__restrict __nptr,
   char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

extern unsigned long int strtoul (__const char *__restrict __nptr,
      char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;




__extension__
extern long long int strtoq (__const char *__restrict __nptr,
        char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

__extension__
extern unsigned long long int strtouq (__const char *__restrict __nptr,
           char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;





__extension__
extern long long int strtoll (__const char *__restrict __nptr,
         char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

__extension__
extern unsigned long long int strtoull (__const char *__restrict __nptr,
     char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

/* # 236 "/usr/include/stdlib.h" 3 4 */
/* # 1 "/usr/include/xlocale.h" 1 3 4 */
/* # 28 "/usr/include/xlocale.h" 3 4 */
typedef struct __locale_struct
{

  struct __locale_data *__locales[13];


  const unsigned short int *__ctype_b;
  const int *__ctype_tolower;
  const int *__ctype_toupper;


  const char *__names[13];
} *__locale_t;


typedef __locale_t locale_t;
/* # 237 "/usr/include/stdlib.h" 2 3 4 */



extern long int strtol_l (__const char *__restrict __nptr,
     char **__restrict __endptr, int __base,
     __locale_t __loc) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 4))) ;

extern unsigned long int strtoul_l (__const char *__restrict __nptr,
        char **__restrict __endptr,
        int __base, __locale_t __loc)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 4))) ;

__extension__
extern long long int strtoll_l (__const char *__restrict __nptr,
    char **__restrict __endptr, int __base,
    __locale_t __loc)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 4))) ;

__extension__
extern unsigned long long int strtoull_l (__const char *__restrict __nptr,
       char **__restrict __endptr,
       int __base, __locale_t __loc)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 4))) ;

extern double strtod_l (__const char *__restrict __nptr,
   char **__restrict __endptr, __locale_t __loc)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 3))) ;

extern float strtof_l (__const char *__restrict __nptr,
         char **__restrict __endptr, __locale_t __loc)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 3))) ;

extern long double strtold_l (__const char *__restrict __nptr,
         char **__restrict __endptr,
         __locale_t __loc)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 3))) ;
/* # 311 "/usr/include/stdlib.h" 3 4 */
extern char *l64a (long int __n) __attribute__ ((__nothrow__)) ;


extern long int a64l (__const char *__s)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;




/* # 1 "/usr/include/sys/types.h" 1 3 4 */
/* # 28 "/usr/include/sys/types.h" 3 4 */


/* # 1 "/usr/include/bits/types.h" 1 3 4 */
/* # 28 "/usr/include/bits/types.h" 3 4 */
/* # 1 "/usr/include/bits/wordsize.h" 1 3 4 */
/* # 29 "/usr/include/bits/types.h" 2 3 4 */


typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;


typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;




__extension__ typedef signed long long int __int64_t;
__extension__ typedef unsigned long long int __uint64_t;







__extension__ typedef long long int __quad_t;
__extension__ typedef unsigned long long int __u_quad_t;
/* # 131 "/usr/include/bits/types.h" 3 4 */
/* # 1 "/usr/include/bits/typesizes.h" 1 3 4 */
/* # 132 "/usr/include/bits/types.h" 2 3 4 */


__extension__ typedef __u_quad_t __dev_t;
__extension__ typedef unsigned int __uid_t;
__extension__ typedef unsigned int __gid_t;
__extension__ typedef unsigned long int __ino_t;
__extension__ typedef __u_quad_t __ino64_t;
__extension__ typedef unsigned int __mode_t;
__extension__ typedef unsigned int __nlink_t;
__extension__ typedef long int __off_t;
__extension__ typedef __quad_t __off64_t;
__extension__ typedef int __pid_t;
__extension__ typedef struct { int __val[2]; } __fsid_t;
__extension__ typedef long int __clock_t;
__extension__ typedef unsigned long int __rlim_t;
__extension__ typedef __u_quad_t __rlim64_t;
__extension__ typedef unsigned int __id_t;
__extension__ typedef long int __time_t;
__extension__ typedef unsigned int __useconds_t;
__extension__ typedef long int __suseconds_t;

__extension__ typedef int __daddr_t;
__extension__ typedef long int __swblk_t;
__extension__ typedef int __key_t;


__extension__ typedef int __clockid_t;


__extension__ typedef void * __timer_t;


__extension__ typedef long int __blksize_t;




__extension__ typedef long int __blkcnt_t;
__extension__ typedef __quad_t __blkcnt64_t;


__extension__ typedef unsigned long int __fsblkcnt_t;
__extension__ typedef __u_quad_t __fsblkcnt64_t;


__extension__ typedef unsigned long int __fsfilcnt_t;
__extension__ typedef __u_quad_t __fsfilcnt64_t;

__extension__ typedef int __ssize_t;



typedef __off64_t __loff_t;
typedef __quad_t *__qaddr_t;
typedef char *__caddr_t;


__extension__ typedef int __intptr_t;


__extension__ typedef unsigned int __socklen_t;
/* # 31 "/usr/include/sys/types.h" 2 3 4 */



typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;




typedef __loff_t loff_t;



typedef __ino_t ino_t;






typedef __ino64_t ino64_t;




typedef __dev_t dev_t;




typedef __gid_t gid_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;




typedef __uid_t uid_t;





typedef __off_t off_t;






typedef __off64_t off64_t;




typedef __pid_t pid_t;





typedef __id_t id_t;




typedef __ssize_t ssize_t;





typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;





typedef __key_t key_t;
/* # 133 "/usr/include/sys/types.h" 3 4 */
/* # 1 "/usr/include/time.h" 1 3 4 */
/* # 58 "/usr/include/time.h" 3 4 */


typedef __clock_t clock_t;



/* # 74 "/usr/include/time.h" 3 4 */


typedef __time_t time_t;



/* # 92 "/usr/include/time.h" 3 4 */
typedef __clockid_t clockid_t;
/* # 104 "/usr/include/time.h" 3 4 */
typedef __timer_t timer_t;
/* # 134 "/usr/include/sys/types.h" 2 3 4 */



typedef __useconds_t useconds_t;



typedef __suseconds_t suseconds_t;





/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.1/include/stddef.h" 1 3 4 */
/* # 148 "/usr/include/sys/types.h" 2 3 4 */



typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
/* # 195 "/usr/include/sys/types.h" 3 4 */
typedef int int8_t __attribute__ ((__mode__ (__QI__)));
typedef int int16_t __attribute__ ((__mode__ (__HI__)));
typedef int int32_t __attribute__ ((__mode__ (__SI__)));
typedef int int64_t __attribute__ ((__mode__ (__DI__)));


typedef unsigned int u_int8_t __attribute__ ((__mode__ (__QI__)));
typedef unsigned int u_int16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int u_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int u_int64_t __attribute__ ((__mode__ (__DI__)));

typedef int register_t __attribute__ ((__mode__ (__word__)));
/* # 220 "/usr/include/sys/types.h" 3 4 */
/* # 1 "/usr/include/sys/select.h" 1 3 4 */
/* # 31 "/usr/include/sys/select.h" 3 4 */
/* # 1 "/usr/include/bits/select.h" 1 3 4 */
/* # 23 "/usr/include/bits/select.h" 3 4 */
/* # 1 "/usr/include/bits/wordsize.h" 1 3 4 */
/* # 24 "/usr/include/bits/select.h" 2 3 4 */
/* # 32 "/usr/include/sys/select.h" 2 3 4 */


/* # 1 "/usr/include/bits/sigset.h" 1 3 4 */
/* # 24 "/usr/include/bits/sigset.h" 3 4 */
typedef int __sig_atomic_t;




typedef struct
  {
    unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
  } __sigset_t;
/* # 35 "/usr/include/sys/select.h" 2 3 4 */



typedef __sigset_t sigset_t;





/* # 1 "/usr/include/time.h" 1 3 4 */
/* # 120 "/usr/include/time.h" 3 4 */
struct timespec
  {
    __time_t tv_sec;
    long int tv_nsec;
  };
/* # 45 "/usr/include/sys/select.h" 2 3 4 */

/* # 1 "/usr/include/bits/time.h" 1 3 4 */
/* # 75 "/usr/include/bits/time.h" 3 4 */
struct timeval
  {
    __time_t tv_sec;
    __suseconds_t tv_usec;
  };
/* # 47 "/usr/include/sys/select.h" 2 3 4 */
/* # 55 "/usr/include/sys/select.h" 3 4 */
typedef long int __fd_mask;
/* # 67 "/usr/include/sys/select.h" 3 4 */
typedef struct
  {



    __fd_mask fds_bits[1024 / (8 * (int) sizeof (__fd_mask))];





  } fd_set;






typedef __fd_mask fd_mask;
/* # 99 "/usr/include/sys/select.h" 3 4 */

/* # 109 "/usr/include/sys/select.h" 3 4 */
extern int select (int __nfds, fd_set *__restrict __readfds,
     fd_set *__restrict __writefds,
     fd_set *__restrict __exceptfds,
     struct timeval *__restrict __timeout);
/* # 121 "/usr/include/sys/select.h" 3 4 */
extern int pselect (int __nfds, fd_set *__restrict __readfds,
      fd_set *__restrict __writefds,
      fd_set *__restrict __exceptfds,
      const struct timespec *__restrict __timeout,
      const __sigset_t *__restrict __sigmask);



/* # 221 "/usr/include/sys/types.h" 2 3 4 */


/* # 1 "/usr/include/sys/sysmacros.h" 1 3 4 */
/* # 30 "/usr/include/sys/sysmacros.h" 3 4 */
__extension__
extern unsigned int gnu_dev_major (unsigned long long int __dev)
     __attribute__ ((__nothrow__));
__extension__
extern unsigned int gnu_dev_minor (unsigned long long int __dev)
     __attribute__ ((__nothrow__));
__extension__
extern unsigned long long int gnu_dev_makedev (unsigned int __major,
            unsigned int __minor)
     __attribute__ ((__nothrow__));
/* # 224 "/usr/include/sys/types.h" 2 3 4 */





typedef __blksize_t blksize_t;






typedef __blkcnt_t blkcnt_t;



typedef __fsblkcnt_t fsblkcnt_t;



typedef __fsfilcnt_t fsfilcnt_t;
/* # 263 "/usr/include/sys/types.h" 3 4 */
typedef __blkcnt64_t blkcnt64_t;
typedef __fsblkcnt64_t fsblkcnt64_t;
typedef __fsfilcnt64_t fsfilcnt64_t;





/* # 1 "/usr/include/bits/pthreadtypes.h" 1 3 4 */
/* # 23 "/usr/include/bits/pthreadtypes.h" 3 4 */
/* # 1 "/usr/include/bits/wordsize.h" 1 3 4 */
/* # 24 "/usr/include/bits/pthreadtypes.h" 2 3 4 */
/* # 50 "/usr/include/bits/pthreadtypes.h" 3 4 */
typedef unsigned long int pthread_t;


typedef union
{
  char __size[36];
  long int __align;
} pthread_attr_t;
/* # 67 "/usr/include/bits/pthreadtypes.h" 3 4 */
typedef struct __pthread_internal_slist
{
  struct __pthread_internal_slist *__next;
} __pthread_slist_t;





typedef union
{
  struct __pthread_mutex_s
  {
    int __lock;
    unsigned int __count;
    int __owner;





    int __kind;





    unsigned int __nusers;
    __extension__ union
    {
      int __spins;
      __pthread_slist_t __list;
    };

  } __data;
  char __size[24];
  long int __align;
} pthread_mutex_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_mutexattr_t;




typedef union
{
  struct
  {
    int __lock;
    unsigned int __futex;
    __extension__ unsigned long long int __total_seq;
    __extension__ unsigned long long int __wakeup_seq;
    __extension__ unsigned long long int __woken_seq;
    void *__mutex;
    unsigned int __nwaiters;
    unsigned int __broadcast_seq;
  } __data;
  char __size[48];
  __extension__ long long int __align;
} pthread_cond_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_condattr_t;



typedef unsigned int pthread_key_t;



typedef int pthread_once_t;





typedef union
{
/* # 170 "/usr/include/bits/pthreadtypes.h" 3 4 */
  struct
  {
    int __lock;
    unsigned int __nr_readers;
    unsigned int __readers_wakeup;
    unsigned int __writer_wakeup;
    unsigned int __nr_readers_queued;
    unsigned int __nr_writers_queued;


    unsigned char __flags;
    unsigned char __shared;
    unsigned char __pad1;
    unsigned char __pad2;
    int __writer;
  } __data;

  char __size[32];
  long int __align;
} pthread_rwlock_t;

typedef union
{
  char __size[8];
  long int __align;
} pthread_rwlockattr_t;





typedef volatile int pthread_spinlock_t;




typedef union
{
  char __size[20];
  long int __align;
} pthread_barrier_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_barrierattr_t;
/* # 272 "/usr/include/sys/types.h" 2 3 4 */



/* # 321 "/usr/include/stdlib.h" 2 3 4 */






extern long int random (void) __attribute__ ((__nothrow__));


extern void srandom (unsigned int __seed) __attribute__ ((__nothrow__));





extern char *initstate (unsigned int __seed, char *__statebuf,
   size_t __statelen) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));



extern char *setstate (char *__statebuf) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));







struct random_data
  {
    int32_t *fptr;
    int32_t *rptr;
    int32_t *state;
    int rand_type;
    int rand_deg;
    int rand_sep;
    int32_t *end_ptr;
  };

extern int random_r (struct random_data *__restrict __buf,
       int32_t *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern int srandom_r (unsigned int __seed, struct random_data *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));

extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
   size_t __statelen,
   struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 4)));

extern int setstate_r (char *__restrict __statebuf,
         struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));






extern int rand (void) __attribute__ ((__nothrow__));

extern void srand (unsigned int __seed) __attribute__ ((__nothrow__));




extern int rand_r (unsigned int *__seed) __attribute__ ((__nothrow__));







extern double drand48 (void) __attribute__ ((__nothrow__));
extern double erand48 (unsigned short int __xsubi[3]) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern long int lrand48 (void) __attribute__ ((__nothrow__));
extern long int nrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern long int mrand48 (void) __attribute__ ((__nothrow__));
extern long int jrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern void srand48 (long int __seedval) __attribute__ ((__nothrow__));
extern unsigned short int *seed48 (unsigned short int __seed16v[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
extern void lcong48 (unsigned short int __param[7]) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





struct drand48_data
  {
    unsigned short int __x[3];
    unsigned short int __old_x[3];
    unsigned short int __c;
    unsigned short int __init;
    unsigned long long int __a;
  };


extern int drand48_r (struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int erand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int lrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int nrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int mrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int jrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int srand48_r (long int __seedval, struct drand48_data *__buffer)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));

extern int seed48_r (unsigned short int __seed16v[3],
       struct drand48_data *__buffer) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern int lcong48_r (unsigned short int __param[7],
        struct drand48_data *__buffer)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));









extern void *malloc (size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;

extern void *calloc (size_t __nmemb, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;










extern void *realloc (void *__ptr, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));

extern void free (void *__ptr) __attribute__ ((__nothrow__));




extern void cfree (void *__ptr) __attribute__ ((__nothrow__));



/* # 1 "/usr/include/alloca.h" 1 3 4 */
/* # 25 "/usr/include/alloca.h" 3 4 */
/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.1/include/stddef.h" 1 3 4 */
/* # 26 "/usr/include/alloca.h" 2 3 4 */







extern void *alloca (size_t __size) __attribute__ ((__nothrow__));






/* # 498 "/usr/include/stdlib.h" 2 3 4 */





extern void *valloc (size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;




extern int posix_memalign (void **__memptr, size_t __alignment, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;




extern void abort (void) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));



extern int atexit (void (*__func) (void)) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
/* # 528 "/usr/include/stdlib.h" 3 4 */
extern int at_quick_exit (void (*__func) (void)) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));







extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern void exit (int __status) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));







extern void quick_exit (int __status) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));







extern void _Exit (int __status) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));






extern char *getenv (__const char *__name) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;




extern char *__secure_getenv (__const char *__name)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;





extern int putenv (char *__string) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int setenv (__const char *__name, __const char *__value, int __replace)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));


extern int unsetenv (__const char *__name) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern int clearenv (void) __attribute__ ((__nothrow__));
/* # 606 "/usr/include/stdlib.h" 3 4 */
extern char *mktemp (char *__template) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
/* # 620 "/usr/include/stdlib.h" 3 4 */
extern int mkstemp (char *__template) __attribute__ ((__nonnull__ (1))) ;
/* # 630 "/usr/include/stdlib.h" 3 4 */
extern int mkstemp64 (char *__template) __attribute__ ((__nonnull__ (1))) ;
/* # 642 "/usr/include/stdlib.h" 3 4 */
extern int mkstemps (char *__template, int __suffixlen) __attribute__ ((__nonnull__ (1))) ;
/* # 652 "/usr/include/stdlib.h" 3 4 */
extern int mkstemps64 (char *__template, int __suffixlen)
     __attribute__ ((__nonnull__ (1))) ;
/* # 663 "/usr/include/stdlib.h" 3 4 */
extern char *mkdtemp (char *__template) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
/* # 674 "/usr/include/stdlib.h" 3 4 */
extern int mkostemp (char *__template, int __flags) __attribute__ ((__nonnull__ (1))) ;
/* # 684 "/usr/include/stdlib.h" 3 4 */
extern int mkostemp64 (char *__template, int __flags) __attribute__ ((__nonnull__ (1))) ;
/* # 694 "/usr/include/stdlib.h" 3 4 */
extern int mkostemps (char *__template, int __suffixlen, int __flags)
     __attribute__ ((__nonnull__ (1))) ;
/* # 706 "/usr/include/stdlib.h" 3 4 */
extern int mkostemps64 (char *__template, int __suffixlen, int __flags)
     __attribute__ ((__nonnull__ (1))) ;









extern int system (__const char *__command) ;






extern char *canonicalize_file_name (__const char *__name)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
/* # 734 "/usr/include/stdlib.h" 3 4 */
extern char *realpath (__const char *__restrict __name,
         char *__restrict __resolved) __attribute__ ((__nothrow__)) ;






typedef int (*__compar_fn_t) (__const void *, __const void *);


typedef __compar_fn_t comparison_fn_t;



typedef int (*__compar_d_fn_t) (__const void *, __const void *, void *);





extern void *bsearch (__const void *__key, __const void *__base,
        size_t __nmemb, size_t __size, __compar_fn_t __compar)
     __attribute__ ((__nonnull__ (1, 2, 5))) ;



extern void qsort (void *__base, size_t __nmemb, size_t __size,
     __compar_fn_t __compar) __attribute__ ((__nonnull__ (1, 4)));

extern void qsort_r (void *__base, size_t __nmemb, size_t __size,
       __compar_d_fn_t __compar, void *__arg)
  __attribute__ ((__nonnull__ (1, 4)));




extern int abs (int __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;
extern long int labs (long int __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;



__extension__ extern long long int llabs (long long int __x)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;







extern div_t div (int __numer, int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;
extern ldiv_t ldiv (long int __numer, long int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;




__extension__ extern lldiv_t lldiv (long long int __numer,
        long long int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;

/* # 808 "/usr/include/stdlib.h" 3 4 */
extern char *ecvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) ;




extern char *fcvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) ;




extern char *gcvt (double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3))) ;




extern char *qecvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *qfcvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *qgcvt (long double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3))) ;




extern int ecvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int fcvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));

extern int qecvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int qfcvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));







extern int mblen (__const char *__s, size_t __n) __attribute__ ((__nothrow__)) ;


extern int mbtowc (wchar_t *__restrict __pwc,
     __const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__)) ;


extern int wctomb (char *__s, wchar_t __wchar) __attribute__ ((__nothrow__)) ;



extern size_t mbstowcs (wchar_t *__restrict __pwcs,
   __const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__));

extern size_t wcstombs (char *__restrict __s,
   __const wchar_t *__restrict __pwcs, size_t __n)
     __attribute__ ((__nothrow__));








extern int rpmatch (__const char *__response) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
/* # 896 "/usr/include/stdlib.h" 3 4 */
extern int getsubopt (char **__restrict __optionp,
        char *__const *__restrict __tokens,
        char **__restrict __valuep)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2, 3))) ;





extern void setkey (__const char *__key) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));







extern int posix_openpt (int __oflag) ;







extern int grantpt (int __fd) __attribute__ ((__nothrow__));



extern int unlockpt (int __fd) __attribute__ ((__nothrow__));




extern char *ptsname (int __fd) __attribute__ ((__nothrow__)) ;






extern int ptsname_r (int __fd, char *__buf, size_t __buflen)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));


extern int getpt (void);






extern int getloadavg (double __loadavg[], int __nelem)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
/* # 964 "/usr/include/stdlib.h" 3 4 */

/* # 45 "../../../mozilla/nsprpub/lib/ds/plarena.c" 2 */
/* # 1 "/usr/include/string.h" 1 3 4 */
/* # 29 "/usr/include/string.h" 3 4 */





/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.1/include/stddef.h" 1 3 4 */
/* # 35 "/usr/include/string.h" 2 3 4 */









extern void *memcpy (void *__restrict __dest,
       __const void *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern void *memmove (void *__dest, __const void *__src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));






extern void *memccpy (void *__restrict __dest, __const void *__restrict __src,
        int __c, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));





extern void *memset (void *__s, int __c, size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int memcmp (__const void *__s1, __const void *__s2, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
/* # 95 "/usr/include/string.h" 3 4 */
extern void *memchr (__const void *__s, int __c, size_t __n)
      __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));


/* # 109 "/usr/include/string.h" 3 4 */
extern void *rawmemchr (__const void *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
/* # 120 "/usr/include/string.h" 3 4 */
extern void *memrchr (__const void *__s, int __c, size_t __n)
      __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));






extern char *strcpy (char *__restrict __dest, __const char *__restrict __src)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncpy (char *__restrict __dest,
        __const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern char *strcat (char *__restrict __dest, __const char *__restrict __src)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncat (char *__restrict __dest, __const char *__restrict __src,
        size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcmp (__const char *__s1, __const char *__s2)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern int strncmp (__const char *__s1, __const char *__s2, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcoll (__const char *__s1, __const char *__s2)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern size_t strxfrm (char *__restrict __dest,
         __const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));

/* # 165 "/usr/include/string.h" 3 4 */
extern int strcoll_l (__const char *__s1, __const char *__s2, __locale_t __l)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 3)));

extern size_t strxfrm_l (char *__dest, __const char *__src, size_t __n,
    __locale_t __l) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 4)));





extern char *strdup (__const char *__s)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));






extern char *strndup (__const char *__string, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));
/* # 210 "/usr/include/string.h" 3 4 */

/* # 235 "/usr/include/string.h" 3 4 */
extern char *strchr (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
/* # 262 "/usr/include/string.h" 3 4 */
extern char *strrchr (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));


/* # 276 "/usr/include/string.h" 3 4 */
extern char *strchrnul (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));






extern size_t strcspn (__const char *__s, __const char *__reject)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern size_t strspn (__const char *__s, __const char *__accept)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
/* # 314 "/usr/include/string.h" 3 4 */
extern char *strpbrk (__const char *__s, __const char *__accept)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
/* # 342 "/usr/include/string.h" 3 4 */
extern char *strstr (__const char *__haystack, __const char *__needle)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));




extern char *strtok (char *__restrict __s, __const char *__restrict __delim)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));




extern char *__strtok_r (char *__restrict __s,
    __const char *__restrict __delim,
    char **__restrict __save_ptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 3)));

extern char *strtok_r (char *__restrict __s, __const char *__restrict __delim,
         char **__restrict __save_ptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 3)));
/* # 373 "/usr/include/string.h" 3 4 */
extern char *strcasestr (__const char *__haystack, __const char *__needle)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));







extern void *memmem (__const void *__haystack, size_t __haystacklen,
       __const void *__needle, size_t __needlelen)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 3)));



extern void *__mempcpy (void *__restrict __dest,
   __const void *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern void *mempcpy (void *__restrict __dest,
        __const void *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));





extern size_t strlen (__const char *__s)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));





extern size_t strnlen (__const char *__string, size_t __maxlen)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));





extern char *strerror (int __errnum) __attribute__ ((__nothrow__));

/* # 438 "/usr/include/string.h" 3 4 */
extern char *strerror_r (int __errnum, char *__buf, size_t __buflen)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));





extern char *strerror_l (int __errnum, __locale_t __l) __attribute__ ((__nothrow__));





extern void __bzero (void *__s, size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern void bcopy (__const void *__src, void *__dest, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern void bzero (void *__s, size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int bcmp (__const void *__s1, __const void *__s2, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
/* # 489 "/usr/include/string.h" 3 4 */
extern char *index (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
/* # 517 "/usr/include/string.h" 3 4 */
extern char *rindex (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));




extern int ffs (int __i) __attribute__ ((__nothrow__)) __attribute__ ((__const__));




extern int ffsl (long int __l) __attribute__ ((__nothrow__)) __attribute__ ((__const__));

__extension__ extern int ffsll (long long int __ll)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__));




extern int strcasecmp (__const char *__s1, __const char *__s2)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strncasecmp (__const char *__s1, __const char *__s2, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));





extern int strcasecmp_l (__const char *__s1, __const char *__s2,
    __locale_t __loc)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 3)));

extern int strncasecmp_l (__const char *__s1, __const char *__s2,
     size_t __n, __locale_t __loc)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 4)));





extern char *strsep (char **__restrict __stringp,
       __const char *__restrict __delim)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));




extern char *strsignal (int __sig) __attribute__ ((__nothrow__));


extern char *__stpcpy (char *__restrict __dest, __const char *__restrict __src)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *stpcpy (char *__restrict __dest, __const char *__restrict __src)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));



extern char *__stpncpy (char *__restrict __dest,
   __const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *stpncpy (char *__restrict __dest,
        __const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));




extern int strverscmp (__const char *__s1, __const char *__s2)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern char *strfry (char *__string) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern void *memfrob (void *__s, size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
/* # 606 "/usr/include/string.h" 3 4 */
extern char *basename (__const char *__filename) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
/* # 646 "/usr/include/string.h" 3 4 */

/* # 46 "../../../mozilla/nsprpub/lib/ds/plarena.c" 2 */
/* # 1 "../../../mozilla/nsprpub/lib/ds/plarena.h" 1 */
/* # 48 "../../../mozilla/nsprpub/lib/ds/plarena.h" */
/* # 1 "../../dist/include/nspr/prtypes.h" 1 */
/* # 58 "../../dist/include/nspr/prtypes.h" */
/* # 1 "../../dist/include/nspr/prcpucfg.h" 1 */
/* # 59 "../../dist/include/nspr/prtypes.h" 2 */


/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.1/include/stddef.h" 1 3 4 */
/* # 150 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.1/include/stddef.h" 3 4 */
typedef int ptrdiff_t;
/* # 62 "../../dist/include/nspr/prtypes.h" 2 */
/* # 231 "../../dist/include/nspr/prtypes.h" */

/* # 241 "../../dist/include/nspr/prtypes.h" */
typedef unsigned char PRUint8;
/* # 255 "../../dist/include/nspr/prtypes.h" */
typedef signed char PRInt8;
/* # 280 "../../dist/include/nspr/prtypes.h" */
typedef unsigned short PRUint16;
typedef short PRInt16;
/* # 305 "../../dist/include/nspr/prtypes.h" */
typedef unsigned int PRUint32;
typedef int PRInt32;
/* # 354 "../../dist/include/nspr/prtypes.h" */
typedef long long PRInt64;
typedef unsigned long long PRUint64;
/* # 378 "../../dist/include/nspr/prtypes.h" */
typedef int PRIntn;
typedef unsigned int PRUintn;
/* # 389 "../../dist/include/nspr/prtypes.h" */
typedef double PRFloat64;






typedef size_t PRSize;







typedef PRInt32 PROffset32;
typedef PRInt64 PROffset64;







typedef ptrdiff_t PRPtrdiff;
/* # 424 "../../dist/include/nspr/prtypes.h" */
typedef unsigned long PRUptrdiff;
/* # 435 "../../dist/include/nspr/prtypes.h" */
typedef PRIntn PRBool;
/* # 445 "../../dist/include/nspr/prtypes.h" */
typedef PRUint8 PRPackedBool;





typedef enum { PR_FAILURE = -1, PR_SUCCESS = 0 } PRStatus;






typedef PRUint16 PRUnichar;
/* # 477 "../../dist/include/nspr/prtypes.h" */
typedef long PRWord;
typedef unsigned long PRUword;
/* # 517 "../../dist/include/nspr/prtypes.h" */
/* # 1 "../../dist/include/nspr/obsolete/protypes.h" 1 */
/* # 48 "../../dist/include/nspr/obsolete/protypes.h" */
typedef PRUintn uintn;

typedef PRIntn intn;
/* # 100 "../../dist/include/nspr/obsolete/protypes.h" */
typedef PRUint64 uint64;
/* # 109 "../../dist/include/nspr/obsolete/protypes.h" */
typedef PRUint32 uint32;
/* # 120 "../../dist/include/nspr/obsolete/protypes.h" */
typedef PRUint16 uint16;







typedef PRUint8 uint8;







typedef PRInt64 int64;
/* # 146 "../../dist/include/nspr/obsolete/protypes.h" */
typedef PRInt32 int32;
/* # 158 "../../dist/include/nspr/obsolete/protypes.h" */
typedef PRInt16 int16;
/* # 167 "../../dist/include/nspr/obsolete/protypes.h" */
typedef PRInt8 int8;


typedef PRFloat64 float64;
typedef PRUptrdiff uptrdiff_t;
typedef PRUword uprword_t;
typedef PRWord prword_t;
/* # 518 "../../dist/include/nspr/prtypes.h" 2 */





/* # 49 "../../../mozilla/nsprpub/lib/ds/plarena.h" 2 */
/* # 1 "../../../mozilla/nsprpub/lib/ds/plarenas.h" 1 */
/* # 42 "../../../mozilla/nsprpub/lib/ds/plarenas.h" */


typedef struct PLArenaPool PLArenaPool;
/* # 76 "../../../mozilla/nsprpub/lib/ds/plarenas.h" */
extern __attribute__((visibility("default"))) void PL_InitArenaPool(
    PLArenaPool *pool, const char *name, PRUint32 size, PRUint32 align);




extern __attribute__((visibility("default"))) void PL_ArenaFinish(void);






extern __attribute__((visibility("default"))) void PL_FreeArenaPool(PLArenaPool *pool);




extern __attribute__((visibility("default"))) void PL_FinishArenaPool(PLArenaPool *pool);




extern __attribute__((visibility("default"))) void PL_CompactArenaPool(PLArenaPool *pool);




extern __attribute__((visibility("default"))) void * PL_ArenaAllocate(PLArenaPool *pool, PRUint32 nb);

extern __attribute__((visibility("default"))) void * PL_ArenaGrow(
    PLArenaPool *pool, void *p, PRUint32 size, PRUint32 incr);

extern __attribute__((visibility("default"))) void PL_ArenaRelease(PLArenaPool *pool, char *mark);


/* # 50 "../../../mozilla/nsprpub/lib/ds/plarena.h" 2 */



typedef struct PLArena PLArena;

struct PLArena {
    PLArena *next;
    PRUword base;
    PRUword limit;
    PRUword avail;
};
/* # 83 "../../../mozilla/nsprpub/lib/ds/plarena.h" */
struct PLArenaPool {
    PLArena first;
    PLArena *current;
    PRUint32 arenasize;
    PRUword mask;



};
/* # 211 "../../../mozilla/nsprpub/lib/ds/plarena.h" */

/* # 47 "../../../mozilla/nsprpub/lib/ds/plarena.c" 2 */
/* # 1 "../../dist/include/nspr/prmem.h" 1 */
/* # 46 "../../dist/include/nspr/prmem.h" */
/* # 1 "../../dist/include/nspr/prtypes.h" 1 */
/* # 47 "../../dist/include/nspr/prmem.h" 2 */



/* # 66 "../../dist/include/nspr/prmem.h" */
extern __attribute__((visibility("default"))) void * PR_Malloc(PRUint32 size);

extern __attribute__((visibility("default"))) void * PR_Calloc(PRUint32 nelem, PRUint32 elsize);

extern __attribute__((visibility("default"))) void * PR_Realloc(void *ptr, PRUint32 size);

extern __attribute__((visibility("default"))) void PR_Free(void *ptr);
/* # 156 "../../dist/include/nspr/prmem.h" */

/* # 48 "../../../mozilla/nsprpub/lib/ds/plarena.c" 2 */
/* # 1 "../../dist/include/nspr/prbit.h" 1 */
/* # 42 "../../dist/include/nspr/prbit.h" */





typedef unsigned long prbitmap_t;
/* # 59 "../../dist/include/nspr/prbit.h" */
extern __attribute__((visibility("default"))) PRIntn PR_CeilingLog2(PRUint32 i);




extern __attribute__((visibility("default"))) PRIntn PR_FloorLog2(PRUint32 i);
/* # 135 "../../dist/include/nspr/prbit.h" */

/* # 49 "../../../mozilla/nsprpub/lib/ds/plarena.c" 2 */
/* # 1 "../../dist/include/nspr/prlog.h" 1 */
/* # 43 "../../dist/include/nspr/prlog.h" */

/* # 138 "../../dist/include/nspr/prlog.h" */
typedef enum PRLogModuleLevel {
    PR_LOG_NONE = 0,
    PR_LOG_ALWAYS = 1,
    PR_LOG_ERROR = 2,
    PR_LOG_WARNING = 3,
    PR_LOG_DEBUG = 4,

    PR_LOG_NOTICE = PR_LOG_DEBUG,
    PR_LOG_WARN = PR_LOG_WARNING,
    PR_LOG_MIN = PR_LOG_DEBUG,
    PR_LOG_MAX = PR_LOG_DEBUG
} PRLogModuleLevel;






typedef struct PRLogModuleInfo {
    const char *name;
    PRLogModuleLevel level;
    struct PRLogModuleInfo *next;
} PRLogModuleInfo;




extern __attribute__((visibility("default"))) PRLogModuleInfo* PR_NewLogModule(const char *name);





extern __attribute__((visibility("default"))) PRBool PR_SetLogFile(const char *name);





extern __attribute__((visibility("default"))) void PR_SetLogBuffering(PRIntn buffer_size);







extern __attribute__((visibility("default"))) void PR_LogPrint(const char *fmt, ...);




extern __attribute__((visibility("default"))) void PR_LogFlush(void);
/* # 236 "../../dist/include/nspr/prlog.h" */
extern __attribute__((visibility("default"))) void PR_Assert(const char *s, const char *file, PRIntn ln);
/* # 258 "../../dist/include/nspr/prlog.h" */

/* # 50 "../../../mozilla/nsprpub/lib/ds/plarena.c" 2 */
/* # 1 "../../dist/include/nspr/prlock.h" 1 */
/* # 53 "../../dist/include/nspr/prlock.h" */

/* # 66 "../../dist/include/nspr/prlock.h" */
typedef struct PRLock PRLock;
/* # 83 "../../dist/include/nspr/prlock.h" */
extern __attribute__((visibility("default"))) PRLock* PR_NewLock(void);
/* # 94 "../../dist/include/nspr/prlock.h" */
extern __attribute__((visibility("default"))) void PR_DestroyLock(PRLock *lock);
/* # 105 "../../dist/include/nspr/prlock.h" */
extern __attribute__((visibility("default"))) void PR_Lock(PRLock *lock);
/* # 117 "../../dist/include/nspr/prlock.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Unlock(PRLock *lock);
/* # 137 "../../dist/include/nspr/prlock.h" */
extern __attribute__((visibility("default"))) void PR_AssertCurrentThreadOwnsLock(PRLock *lock);


/* # 51 "../../../mozilla/nsprpub/lib/ds/plarena.c" 2 */
/* # 1 "../../dist/include/nspr/prinit.h" 1 */
/* # 41 "../../dist/include/nspr/prinit.h" */
/* # 1 "../../dist/include/nspr/prthread.h" 1 */
/* # 79 "../../dist/include/nspr/prthread.h" */
/* # 1 "../../dist/include/nspr/prinrval.h" 1 */
/* # 55 "../../dist/include/nspr/prinrval.h" */






typedef PRUint32 PRIntervalTime;
/* # 111 "../../dist/include/nspr/prinrval.h" */
extern __attribute__((visibility("default"))) PRIntervalTime PR_IntervalNow(void);
/* # 129 "../../dist/include/nspr/prinrval.h" */
extern __attribute__((visibility("default"))) PRUint32 PR_TicksPerSecond(void);
/* # 148 "../../dist/include/nspr/prinrval.h" */
extern __attribute__((visibility("default"))) PRIntervalTime PR_SecondsToInterval(PRUint32 seconds);
extern __attribute__((visibility("default"))) PRIntervalTime PR_MillisecondsToInterval(PRUint32 milli);
extern __attribute__((visibility("default"))) PRIntervalTime PR_MicrosecondsToInterval(PRUint32 micro);
/* # 169 "../../dist/include/nspr/prinrval.h" */
extern __attribute__((visibility("default"))) PRUint32 PR_IntervalToSeconds(PRIntervalTime ticks);
extern __attribute__((visibility("default"))) PRUint32 PR_IntervalToMilliseconds(PRIntervalTime ticks);
extern __attribute__((visibility("default"))) PRUint32 PR_IntervalToMicroseconds(PRIntervalTime ticks);


/* # 80 "../../dist/include/nspr/prthread.h" 2 */



typedef struct PRThread PRThread;
typedef struct PRThreadStack PRThreadStack;

typedef enum PRThreadType {
    PR_USER_THREAD,
    PR_SYSTEM_THREAD
} PRThreadType;

typedef enum PRThreadScope {
    PR_LOCAL_THREAD,
    PR_GLOBAL_THREAD,
    PR_GLOBAL_BOUND_THREAD
} PRThreadScope;

typedef enum PRThreadState {
    PR_JOINABLE_THREAD,
    PR_UNJOINABLE_THREAD
} PRThreadState;

typedef enum PRThreadPriority
{
    PR_PRIORITY_FIRST = 0,
    PR_PRIORITY_LOW = 0,
    PR_PRIORITY_NORMAL = 1,
    PR_PRIORITY_HIGH = 2,
    PR_PRIORITY_URGENT = 3,
    PR_PRIORITY_LAST = 3
} PRThreadPriority;
/* # 137 "../../dist/include/nspr/prthread.h" */
extern __attribute__((visibility("default"))) PRThread* PR_CreateThread(PRThreadType type,
                     void ( *start)(void *arg),
                     void *arg,
                     PRThreadPriority priority,
                     PRThreadScope scope,
                     PRThreadState state,
                     PRUint32 stackSize);
/* # 158 "../../dist/include/nspr/prthread.h" */
extern __attribute__((visibility("default"))) PRStatus PR_JoinThread(PRThread *thread);





extern __attribute__((visibility("default"))) PRThread* PR_GetCurrentThread(void);







extern __attribute__((visibility("default"))) PRThreadPriority PR_GetThreadPriority(const PRThread *thread);




extern __attribute__((visibility("default"))) void PR_SetThreadPriority(PRThread *thread, PRThreadPriority priority);
/* # 203 "../../dist/include/nspr/prthread.h" */
typedef void ( *PRThreadPrivateDTOR)(void *priv);

extern __attribute__((visibility("default"))) PRStatus PR_NewThreadPrivateIndex(
    PRUintn *newIndex, PRThreadPrivateDTOR destructor);
/* # 219 "../../dist/include/nspr/prthread.h" */
extern __attribute__((visibility("default"))) PRStatus PR_SetThreadPrivate(PRUintn tpdIndex, void *priv);
/* # 230 "../../dist/include/nspr/prthread.h" */
extern __attribute__((visibility("default"))) void* PR_GetThreadPrivate(PRUintn tpdIndex);
/* # 242 "../../dist/include/nspr/prthread.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Interrupt(PRThread *thread);





extern __attribute__((visibility("default"))) void PR_ClearInterrupt(void);




extern __attribute__((visibility("default"))) void PR_BlockInterrupt(void);




extern __attribute__((visibility("default"))) void PR_UnblockInterrupt(void);
/* # 267 "../../dist/include/nspr/prthread.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Sleep(PRIntervalTime ticks);




extern __attribute__((visibility("default"))) PRThreadScope PR_GetThreadScope(const PRThread *thread);




extern __attribute__((visibility("default"))) PRThreadType PR_GetThreadType(const PRThread *thread);




extern __attribute__((visibility("default"))) PRThreadState PR_GetThreadState(const PRThread *thread);


/* # 42 "../../dist/include/nspr/prinit.h" 2 */

/* # 1 "../../dist/include/nspr/prwin16.h" 1 */
/* # 44 "../../dist/include/nspr/prinit.h" 2 */
/* # 1 "/usr/include/stdio.h" 1 3 4 */
/* # 30 "/usr/include/stdio.h" 3 4 */




/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.1/include/stddef.h" 1 3 4 */
/* # 35 "/usr/include/stdio.h" 2 3 4 */
/* # 45 "/usr/include/stdio.h" 3 4 */
struct _IO_FILE;



typedef struct _IO_FILE FILE;





/* # 65 "/usr/include/stdio.h" 3 4 */
typedef struct _IO_FILE __FILE;
/* # 75 "/usr/include/stdio.h" 3 4 */
/* # 1 "/usr/include/libio.h" 1 3 4 */
/* # 32 "/usr/include/libio.h" 3 4 */
/* # 1 "/usr/include/_G_config.h" 1 3 4 */
/* # 15 "/usr/include/_G_config.h" 3 4 */
/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.1/include/stddef.h" 1 3 4 */
/* # 16 "/usr/include/_G_config.h" 2 3 4 */




/* # 1 "/usr/include/wchar.h" 1 3 4 */
/* # 83 "/usr/include/wchar.h" 3 4 */
typedef struct
{
  int __count;
  union
  {

    unsigned int __wch;



    char __wchb[4];
  } __value;
} __mbstate_t;
/* # 21 "/usr/include/_G_config.h" 2 3 4 */

typedef struct
{
  __off_t __pos;
  __mbstate_t __state;
} _G_fpos_t;
typedef struct
{
  __off64_t __pos;
  __mbstate_t __state;
} _G_fpos64_t;
/* # 53 "/usr/include/_G_config.h" 3 4 */
typedef int _G_int16_t __attribute__ ((__mode__ (__HI__)));
typedef int _G_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int _G_uint16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int _G_uint32_t __attribute__ ((__mode__ (__SI__)));
/* # 33 "/usr/include/libio.h" 2 3 4 */
/* # 53 "/usr/include/libio.h" 3 4 */
/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.1/include/stdarg.h" 1 3 4 */
/* # 40 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.1/include/stdarg.h" 3 4 */
typedef __builtin_va_list __gnuc_va_list;
/* # 54 "/usr/include/libio.h" 2 3 4 */
/* # 170 "/usr/include/libio.h" 3 4 */
struct _IO_jump_t; struct _IO_FILE;
/* # 180 "/usr/include/libio.h" 3 4 */
typedef void _IO_lock_t;





struct _IO_marker {
  struct _IO_marker *_next;
  struct _IO_FILE *_sbuf;



  int _pos;
/* # 203 "/usr/include/libio.h" 3 4 */
};


enum __codecvt_result
{
  __codecvt_ok,
  __codecvt_partial,
  __codecvt_error,
  __codecvt_noconv
};
/* # 271 "/usr/include/libio.h" 3 4 */
struct _IO_FILE {
  int _flags;




  char* _IO_read_ptr;
  char* _IO_read_end;
  char* _IO_read_base;
  char* _IO_write_base;
  char* _IO_write_ptr;
  char* _IO_write_end;
  char* _IO_buf_base;
  char* _IO_buf_end;

  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;

  struct _IO_marker *_markers;

  struct _IO_FILE *_chain;

  int _fileno;



  int _flags2;

  __off_t _old_offset;



  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];



  _IO_lock_t *_lock;
/* # 319 "/usr/include/libio.h" 3 4 */
  __off64_t _offset;
/* # 328 "/usr/include/libio.h" 3 4 */
  void *__pad1;
  void *__pad2;
  void *__pad3;
  void *__pad4;
  size_t __pad5;

  int _mode;

  char _unused2[15 * sizeof (int) - 4 * sizeof (void *) - sizeof (size_t)];

};


typedef struct _IO_FILE _IO_FILE;


struct _IO_FILE_plus;

extern struct _IO_FILE_plus _IO_2_1_stdin_;
extern struct _IO_FILE_plus _IO_2_1_stdout_;
extern struct _IO_FILE_plus _IO_2_1_stderr_;
/* # 364 "/usr/include/libio.h" 3 4 */
typedef __ssize_t __io_read_fn (void *__cookie, char *__buf, size_t __nbytes);







typedef __ssize_t __io_write_fn (void *__cookie, __const char *__buf,
     size_t __n);







typedef int __io_seek_fn (void *__cookie, __off64_t *__pos, int __w);


typedef int __io_close_fn (void *__cookie);




typedef __io_read_fn cookie_read_function_t;
typedef __io_write_fn cookie_write_function_t;
typedef __io_seek_fn cookie_seek_function_t;
typedef __io_close_fn cookie_close_function_t;


typedef struct
{
  __io_read_fn *read;
  __io_write_fn *write;
  __io_seek_fn *seek;
  __io_close_fn *close;
} _IO_cookie_io_functions_t;
typedef _IO_cookie_io_functions_t cookie_io_functions_t;

struct _IO_cookie_file;


extern void _IO_cookie_init (struct _IO_cookie_file *__cfile, int __read_write,
        void *__cookie, _IO_cookie_io_functions_t __fns);







extern int __underflow (_IO_FILE *);
extern int __uflow (_IO_FILE *);
extern int __overflow (_IO_FILE *, int);
/* # 460 "/usr/include/libio.h" 3 4 */
extern int _IO_getc (_IO_FILE *__fp);
extern int _IO_putc (int __c, _IO_FILE *__fp);
extern int _IO_feof (_IO_FILE *__fp) __attribute__ ((__nothrow__));
extern int _IO_ferror (_IO_FILE *__fp) __attribute__ ((__nothrow__));

extern int _IO_peekc_locked (_IO_FILE *__fp);





extern void _IO_flockfile (_IO_FILE *) __attribute__ ((__nothrow__));
extern void _IO_funlockfile (_IO_FILE *) __attribute__ ((__nothrow__));
extern int _IO_ftrylockfile (_IO_FILE *) __attribute__ ((__nothrow__));
/* # 490 "/usr/include/libio.h" 3 4 */
extern int _IO_vfscanf (_IO_FILE * __restrict, const char * __restrict,
   __gnuc_va_list, int *__restrict);
extern int _IO_vfprintf (_IO_FILE *__restrict, const char *__restrict,
    __gnuc_va_list);
extern __ssize_t _IO_padn (_IO_FILE *, int, __ssize_t);
extern size_t _IO_sgetn (_IO_FILE *, void *, size_t);

extern __off64_t _IO_seekoff (_IO_FILE *, __off64_t, int, int);
extern __off64_t _IO_seekpos (_IO_FILE *, __off64_t, int);

extern void _IO_free_backup_area (_IO_FILE *) __attribute__ ((__nothrow__));
/* # 76 "/usr/include/stdio.h" 2 3 4 */




typedef __gnuc_va_list va_list;
/* # 109 "/usr/include/stdio.h" 3 4 */


typedef _G_fpos_t fpos_t;





typedef _G_fpos64_t fpos64_t;
/* # 161 "/usr/include/stdio.h" 3 4 */
/* # 1 "/usr/include/bits/stdio_lim.h" 1 3 4 */
/* # 162 "/usr/include/stdio.h" 2 3 4 */



extern struct _IO_FILE *stdin;
extern struct _IO_FILE *stdout;
extern struct _IO_FILE *stderr;







extern int remove (__const char *__filename) __attribute__ ((__nothrow__));

extern int rename (__const char *__old, __const char *__new) __attribute__ ((__nothrow__));




extern int renameat (int __oldfd, __const char *__old, int __newfd,
       __const char *__new) __attribute__ ((__nothrow__));








extern FILE *tmpfile (void) ;
/* # 202 "/usr/include/stdio.h" 3 4 */
extern FILE *tmpfile64 (void) ;



extern char *tmpnam (char *__s) __attribute__ ((__nothrow__)) ;





extern char *tmpnam_r (char *__s) __attribute__ ((__nothrow__)) ;
/* # 224 "/usr/include/stdio.h" 3 4 */
extern char *tempnam (__const char *__dir, __const char *__pfx)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;








extern int fclose (FILE *__stream);




extern int fflush (FILE *__stream);

/* # 249 "/usr/include/stdio.h" 3 4 */
extern int fflush_unlocked (FILE *__stream);
/* # 259 "/usr/include/stdio.h" 3 4 */
extern int fcloseall (void);









extern FILE *fopen (__const char *__restrict __filename,
      __const char *__restrict __modes) ;




extern FILE *freopen (__const char *__restrict __filename,
        __const char *__restrict __modes,
        FILE *__restrict __stream) ;
/* # 292 "/usr/include/stdio.h" 3 4 */


extern FILE *fopen64 (__const char *__restrict __filename,
        __const char *__restrict __modes) ;
extern FILE *freopen64 (__const char *__restrict __filename,
   __const char *__restrict __modes,
   FILE *__restrict __stream) ;




extern FILE *fdopen (int __fd, __const char *__modes) __attribute__ ((__nothrow__)) ;





extern FILE *fopencookie (void *__restrict __magic_cookie,
     __const char *__restrict __modes,
     _IO_cookie_io_functions_t __io_funcs) __attribute__ ((__nothrow__)) ;




extern FILE *fmemopen (void *__s, size_t __len, __const char *__modes)
  __attribute__ ((__nothrow__)) ;




extern FILE *open_memstream (char **__bufloc, size_t *__sizeloc) __attribute__ ((__nothrow__)) ;






extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) __attribute__ ((__nothrow__));



extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
      int __modes, size_t __n) __attribute__ ((__nothrow__));





extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
         size_t __size) __attribute__ ((__nothrow__));


extern void setlinebuf (FILE *__stream) __attribute__ ((__nothrow__));








extern int fprintf (FILE *__restrict __stream,
      __const char *__restrict __format, ...);




extern int printf (__const char *__restrict __format, ...);

extern int sprintf (char *__restrict __s,
      __const char *__restrict __format, ...) __attribute__ ((__nothrow__));





extern int vfprintf (FILE *__restrict __s, __const char *__restrict __format,
       __gnuc_va_list __arg);




extern int vprintf (__const char *__restrict __format, __gnuc_va_list __arg);

extern int vsprintf (char *__restrict __s, __const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nothrow__));





extern int snprintf (char *__restrict __s, size_t __maxlen,
       __const char *__restrict __format, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 4)));

extern int vsnprintf (char *__restrict __s, size_t __maxlen,
        __const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 0)));






extern int vasprintf (char **__restrict __ptr, __const char *__restrict __f,
        __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 2, 0))) ;
extern int __asprintf (char **__restrict __ptr,
         __const char *__restrict __fmt, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 2, 3))) ;
extern int asprintf (char **__restrict __ptr,
       __const char *__restrict __fmt, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 2, 3))) ;
/* # 414 "/usr/include/stdio.h" 3 4 */
extern int vdprintf (int __fd, __const char *__restrict __fmt,
       __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 2, 0)));
extern int dprintf (int __fd, __const char *__restrict __fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));








extern int fscanf (FILE *__restrict __stream,
     __const char *__restrict __format, ...) ;




extern int scanf (__const char *__restrict __format, ...) ;

extern int sscanf (__const char *__restrict __s,
     __const char *__restrict __format, ...) __attribute__ ((__nothrow__));
/* # 465 "/usr/include/stdio.h" 3 4 */








extern int vfscanf (FILE *__restrict __s, __const char *__restrict __format,
      __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0))) ;





extern int vscanf (__const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 1, 0))) ;


extern int vsscanf (__const char *__restrict __s,
      __const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__scanf__, 2, 0)));
/* # 524 "/usr/include/stdio.h" 3 4 */









extern int fgetc (FILE *__stream);
extern int getc (FILE *__stream);





extern int getchar (void);

/* # 552 "/usr/include/stdio.h" 3 4 */
extern int getc_unlocked (FILE *__stream);
extern int getchar_unlocked (void);
/* # 563 "/usr/include/stdio.h" 3 4 */
extern int fgetc_unlocked (FILE *__stream);











extern int fputc (int __c, FILE *__stream);
extern int putc (int __c, FILE *__stream);





extern int putchar (int __c);

/* # 596 "/usr/include/stdio.h" 3 4 */
extern int fputc_unlocked (int __c, FILE *__stream);







extern int putc_unlocked (int __c, FILE *__stream);
extern int putchar_unlocked (int __c);






extern int getw (FILE *__stream);


extern int putw (int __w, FILE *__stream);








extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
     ;






extern char *gets (char *__s) ;

/* # 642 "/usr/include/stdio.h" 3 4 */
extern char *fgets_unlocked (char *__restrict __s, int __n,
        FILE *__restrict __stream) ;
/* # 658 "/usr/include/stdio.h" 3 4 */
extern __ssize_t __getdelim (char **__restrict __lineptr,
          size_t *__restrict __n, int __delimiter,
          FILE *__restrict __stream) ;
extern __ssize_t getdelim (char **__restrict __lineptr,
        size_t *__restrict __n, int __delimiter,
        FILE *__restrict __stream) ;







extern __ssize_t getline (char **__restrict __lineptr,
       size_t *__restrict __n,
       FILE *__restrict __stream) ;








extern int fputs (__const char *__restrict __s, FILE *__restrict __stream);





extern int puts (__const char *__s);






extern int ungetc (int __c, FILE *__stream);






extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream) ;




extern size_t fwrite (__const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s) ;

/* # 719 "/usr/include/stdio.h" 3 4 */
extern int fputs_unlocked (__const char *__restrict __s,
      FILE *__restrict __stream);
/* # 730 "/usr/include/stdio.h" 3 4 */
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
         size_t __n, FILE *__restrict __stream) ;
extern size_t fwrite_unlocked (__const void *__restrict __ptr, size_t __size,
          size_t __n, FILE *__restrict __stream) ;








extern int fseek (FILE *__stream, long int __off, int __whence);




extern long int ftell (FILE *__stream) ;




extern void rewind (FILE *__stream);

/* # 766 "/usr/include/stdio.h" 3 4 */
extern int fseeko (FILE *__stream, __off_t __off, int __whence);




extern __off_t ftello (FILE *__stream) ;
/* # 785 "/usr/include/stdio.h" 3 4 */






extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos);




extern int fsetpos (FILE *__stream, __const fpos_t *__pos);
/* # 808 "/usr/include/stdio.h" 3 4 */



extern int fseeko64 (FILE *__stream, __off64_t __off, int __whence);
extern __off64_t ftello64 (FILE *__stream) ;
extern int fgetpos64 (FILE *__restrict __stream, fpos64_t *__restrict __pos);
extern int fsetpos64 (FILE *__stream, __const fpos64_t *__pos);




extern void clearerr (FILE *__stream) __attribute__ ((__nothrow__));

extern int feof (FILE *__stream) __attribute__ ((__nothrow__)) ;

extern int ferror (FILE *__stream) __attribute__ ((__nothrow__)) ;




extern void clearerr_unlocked (FILE *__stream) __attribute__ ((__nothrow__));
extern int feof_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) ;
extern int ferror_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) ;








extern void perror (__const char *__s);






/* # 1 "/usr/include/bits/sys_errlist.h" 1 3 4 */
/* # 27 "/usr/include/bits/sys_errlist.h" 3 4 */
extern int sys_nerr;
extern __const char *__const sys_errlist[];


extern int _sys_nerr;
extern __const char *__const _sys_errlist[];
/* # 847 "/usr/include/stdio.h" 2 3 4 */




extern int fileno (FILE *__stream) __attribute__ ((__nothrow__)) ;




extern int fileno_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) ;
/* # 866 "/usr/include/stdio.h" 3 4 */
extern FILE *popen (__const char *__command, __const char *__modes) ;





extern int pclose (FILE *__stream);





extern char *ctermid (char *__s) __attribute__ ((__nothrow__));





extern char *cuserid (char *__s);




struct obstack;


extern int obstack_printf (struct obstack *__restrict __obstack,
      __const char *__restrict __format, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 2, 3)));
extern int obstack_vprintf (struct obstack *__restrict __obstack,
       __const char *__restrict __format,
       __gnuc_va_list __args)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 2, 0)));







extern void flockfile (FILE *__stream) __attribute__ ((__nothrow__));



extern int ftrylockfile (FILE *__stream) __attribute__ ((__nothrow__)) ;


extern void funlockfile (FILE *__stream) __attribute__ ((__nothrow__));
/* # 936 "/usr/include/stdio.h" 3 4 */

/* # 45 "../../dist/include/nspr/prinit.h" 2 */


/* # 88 "../../dist/include/nspr/prinit.h" */
typedef PRBool (*PRVersionCheck)(const char*);
/* # 98 "../../dist/include/nspr/prinit.h" */
extern __attribute__((visibility("default"))) PRBool PR_VersionCheck(const char *importedVersion);
/* # 111 "../../dist/include/nspr/prinit.h" */
extern __attribute__((visibility("default"))) void PR_Init(
    PRThreadType type, PRThreadPriority priority, PRUintn maxPTDs);
/* # 132 "../../dist/include/nspr/prinit.h" */
typedef PRIntn ( *PRPrimordialFn)(PRIntn argc, char **argv);

extern __attribute__((visibility("default"))) PRIntn PR_Initialize(
    PRPrimordialFn prmain, PRIntn argc, char **argv, PRUintn maxPTDs);




extern __attribute__((visibility("default"))) PRBool PR_Initialized(void);
/* # 158 "../../dist/include/nspr/prinit.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Cleanup(void);





extern __attribute__((visibility("default"))) void PR_DisableClockInterrupts(void);





extern __attribute__((visibility("default"))) void PR_EnableClockInterrupts(void);





extern __attribute__((visibility("default"))) void PR_BlockClockInterrupts(void);





extern __attribute__((visibility("default"))) void PR_UnblockClockInterrupts(void);




extern __attribute__((visibility("default"))) void PR_SetConcurrency(PRUintn numCPUs);






extern __attribute__((visibility("default"))) PRStatus PR_SetFDCacheSize(PRIntn low, PRIntn high);






extern __attribute__((visibility("default"))) void PR_ProcessExit(PRIntn status);






extern __attribute__((visibility("default"))) void PR_Abort(void);
/* # 218 "../../dist/include/nspr/prinit.h" */
typedef struct PRCallOnceType {
    PRIntn initialized;
    PRInt32 inProgress;
    PRStatus status;
} PRCallOnceType;

typedef PRStatus ( *PRCallOnceFN)(void);

typedef PRStatus ( *PRCallOnceWithArgFN)(void *arg);

extern __attribute__((visibility("default"))) PRStatus PR_CallOnce(
    PRCallOnceType *once,
    PRCallOnceFN func
);

extern __attribute__((visibility("default"))) PRStatus PR_CallOnceWithArg(
    PRCallOnceType *once,
    PRCallOnceWithArgFN func,
    void *arg
);
