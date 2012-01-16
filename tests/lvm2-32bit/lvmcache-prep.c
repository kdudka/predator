/* # 1 "cache/lvmcache.c" */
/* # 1 "<built-in>" */
/* # 1 "<command-line>" */
/* # 1 "cache/lvmcache.c" */
/* # 16 "cache/lvmcache.c" */
/* # 1 "../include/lib.h" 1 */
/* # 22 "../include/lib.h" */
/* # 1 "../include/configure.h" 1 */
/* # 23 "../include/lib.h" 2 */





/* # 1 "../include/intl.h" 1 */
/* # 29 "../include/lib.h" 2 */
/* # 1 "../include/libdevmapper.h" 1 */
/* # 19 "../include/libdevmapper.h" */
/* # 1 "/usr/include/inttypes.h" 1 3 4 */
/* # 26 "/usr/include/inttypes.h" 3 4 */
/* # 1 "/usr/include/features.h" 1 3 4 */
/* # 357 "/usr/include/features.h" 3 4 */
/* # 1 "/usr/include/sys/cdefs.h" 1 3 4 */
/* # 353 "/usr/include/sys/cdefs.h" 3 4 */
/* # 1 "/usr/include/bits/wordsize.h" 1 3 4 */
/* # 354 "/usr/include/sys/cdefs.h" 2 3 4 */
/* # 358 "/usr/include/features.h" 2 3 4 */
/* # 381 "/usr/include/features.h" 3 4 */
/* # 1 "/usr/include/gnu/stubs.h" 1 3 4 */



/* # 1 "/usr/include/bits/wordsize.h" 1 3 4 */
/* # 5 "/usr/include/gnu/stubs.h" 2 3 4 */


/* # 1 "/usr/include/gnu/stubs-32.h" 1 3 4 */
/* # 8 "/usr/include/gnu/stubs.h" 2 3 4 */
/* # 382 "/usr/include/features.h" 2 3 4 */
/* # 27 "/usr/include/inttypes.h" 2 3 4 */

/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stdint.h" 1 3 4 */


/* # 1 "/usr/include/stdint.h" 1 3 4 */
/* # 27 "/usr/include/stdint.h" 3 4 */
/* # 1 "/usr/include/bits/wchar.h" 1 3 4 */
/* # 28 "/usr/include/stdint.h" 2 3 4 */
/* # 1 "/usr/include/bits/wordsize.h" 1 3 4 */
/* # 29 "/usr/include/stdint.h" 2 3 4 */
/* # 37 "/usr/include/stdint.h" 3 4 */
typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;



__extension__
typedef long long int int64_t;




typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;

typedef unsigned int uint32_t;





__extension__
typedef unsigned long long int uint64_t;






typedef signed char int_least8_t;
typedef short int int_least16_t;
typedef int int_least32_t;



__extension__
typedef long long int int_least64_t;



typedef unsigned char uint_least8_t;
typedef unsigned short int uint_least16_t;
typedef unsigned int uint_least32_t;



__extension__
typedef unsigned long long int uint_least64_t;






typedef signed char int_fast8_t;





typedef int int_fast16_t;
typedef int int_fast32_t;
__extension__
typedef long long int int_fast64_t;



typedef unsigned char uint_fast8_t;





typedef unsigned int uint_fast16_t;
typedef unsigned int uint_fast32_t;
__extension__
typedef unsigned long long int uint_fast64_t;
/* # 126 "/usr/include/stdint.h" 3 4 */
typedef int intptr_t;


typedef unsigned int uintptr_t;
/* # 138 "/usr/include/stdint.h" 3 4 */
__extension__
typedef long long int intmax_t;
__extension__
typedef unsigned long long int uintmax_t;
/* # 4 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stdint.h" 2 3 4 */
/* # 29 "/usr/include/inttypes.h" 2 3 4 */






typedef int __gwchar_t;
/* # 274 "/usr/include/inttypes.h" 3 4 */

/* # 288 "/usr/include/inttypes.h" 3 4 */
typedef struct
  {
    long long int quot;
    long long int rem;
  } imaxdiv_t;





extern intmax_t imaxabs (intmax_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern imaxdiv_t imaxdiv (intmax_t __numer, intmax_t __denom)
      __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern intmax_t strtoimax (__const char *__restrict __nptr,
      char **__restrict __endptr, int __base) __attribute__ ((__nothrow__));


extern uintmax_t strtoumax (__const char *__restrict __nptr,
       char ** __restrict __endptr, int __base) __attribute__ ((__nothrow__));


extern intmax_t wcstoimax (__const __gwchar_t *__restrict __nptr,
      __gwchar_t **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__));


extern uintmax_t wcstoumax (__const __gwchar_t *__restrict __nptr,
       __gwchar_t ** __restrict __endptr, int __base)
     __attribute__ ((__nothrow__));
/* # 379 "/usr/include/inttypes.h" 3 4 */
__extension__
extern long long int __strtoll_internal (__const char *__restrict __nptr,
      char **__restrict __endptr,
      int __base, int __group)
  __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

extern __inline intmax_t
__attribute__ ((__nothrow__)) strtoimax (__const char *__restrict nptr, char **__restrict endptr, int base)

{
  return __strtoll_internal (nptr, endptr, base, 0);
}

__extension__
extern unsigned long long int __strtoull_internal (__const char *
         __restrict __nptr,
         char **
         __restrict __endptr,
         int __base,
         int __group)
  __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

extern __inline uintmax_t
__attribute__ ((__nothrow__)) strtoumax (__const char *__restrict nptr, char **__restrict endptr, int base)

{
  return __strtoull_internal (nptr, endptr, base, 0);
}

__extension__
extern long long int __wcstoll_internal (__const __gwchar_t *
      __restrict __nptr,
      __gwchar_t **__restrict __endptr,
      int __base, int __group)
  __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

extern __inline intmax_t
__attribute__ ((__nothrow__)) wcstoimax (__const __gwchar_t *__restrict nptr, __gwchar_t **__restrict endptr, int base)

{
  return __wcstoll_internal (nptr, endptr, base, 0);
}


__extension__
extern unsigned long long int __wcstoull_internal (__const __gwchar_t *
         __restrict __nptr,
         __gwchar_t **
         __restrict __endptr,
         int __base,
         int __group)
  __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

extern __inline uintmax_t
__attribute__ ((__nothrow__)) wcstoumax (__const __gwchar_t *__restrict nptr, __gwchar_t **__restrict endptr, int base)

{
  return __wcstoull_internal (nptr, endptr, base, 0);
}





/* # 20 "../include/libdevmapper.h" 2 */
/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stdarg.h" 1 3 4 */
/* # 40 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stdarg.h" 3 4 */
typedef __builtin_va_list __gnuc_va_list;
/* # 102 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stdarg.h" 3 4 */
typedef __gnuc_va_list va_list;
/* # 21 "../include/libdevmapper.h" 2 */
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





typedef __ino64_t ino_t;




typedef __ino64_t ino64_t;




typedef __dev_t dev_t;




typedef __gid_t gid_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;




typedef __uid_t uid_t;







typedef __off64_t off_t;




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





/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stddef.h" 1 3 4 */
/* # 212 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stddef.h" 3 4 */
typedef unsigned int size_t;
/* # 148 "/usr/include/sys/types.h" 2 3 4 */



typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
/* # 201 "/usr/include/sys/types.h" 3 4 */
typedef unsigned int u_int8_t __attribute__ ((__mode__ (__QI__)));
typedef unsigned int u_int16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int u_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int u_int64_t __attribute__ ((__mode__ (__DI__)));

typedef int register_t __attribute__ ((__mode__ (__word__)));
/* # 217 "/usr/include/sys/types.h" 3 4 */
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
/* # 218 "/usr/include/sys/types.h" 2 3 4 */


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


__extension__ extern __inline unsigned int
__attribute__ ((__nothrow__)) gnu_dev_major (unsigned long long int __dev)
{
  return ((__dev >> 8) & 0xfff) | ((unsigned int) (__dev >> 32) & ~0xfff);
}

__extension__ extern __inline unsigned int
__attribute__ ((__nothrow__)) gnu_dev_minor (unsigned long long int __dev)
{
  return (__dev & 0xff) | ((unsigned int) (__dev >> 12) & ~0xff);
}

__extension__ extern __inline unsigned long long int
__attribute__ ((__nothrow__)) gnu_dev_makedev (unsigned int __major, unsigned int __minor)
{
  return ((__minor & 0xff) | ((__major & 0xfff) << 8)
   | (((unsigned long long int) (__minor & ~0xff)) << 12)
   | (((unsigned long long int) (__major & ~0xfff)) << 32));
}
/* # 224 "/usr/include/sys/types.h" 2 3 4 */





typedef __blksize_t blksize_t;
/* # 249 "/usr/include/sys/types.h" 3 4 */
typedef __blkcnt64_t blkcnt_t;



typedef __fsblkcnt64_t fsblkcnt_t;



typedef __fsfilcnt64_t fsfilcnt_t;





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



/* # 22 "../include/libdevmapper.h" 2 */


/* # 1 "/usr/include/linux/types.h" 1 3 4 */



/* # 1 "/usr/include/asm/types.h" 1 3 4 */



/* # 1 "/usr/include/asm-generic/types.h" 1 3 4 */






/* # 1 "/usr/include/asm-generic/int-ll64.h" 1 3 4 */
/* # 11 "/usr/include/asm-generic/int-ll64.h" 3 4 */
/* # 1 "/usr/include/asm/bitsperlong.h" 1 3 4 */
/* # 10 "/usr/include/asm/bitsperlong.h" 3 4 */
/* # 1 "/usr/include/asm-generic/bitsperlong.h" 1 3 4 */
/* # 11 "/usr/include/asm/bitsperlong.h" 2 3 4 */
/* # 12 "/usr/include/asm-generic/int-ll64.h" 2 3 4 */







typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;


__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
/* # 8 "/usr/include/asm-generic/types.h" 2 3 4 */



typedef unsigned short umode_t;
/* # 5 "/usr/include/asm/types.h" 2 3 4 */
/* # 5 "/usr/include/linux/types.h" 2 3 4 */



/* # 1 "/usr/include/linux/posix_types.h" 1 3 4 */



/* # 1 "/usr/include/linux/stddef.h" 1 3 4 */
/* # 14 "/usr/include/linux/stddef.h" 3 4 */
/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stddef.h" 1 3 4 */
/* # 150 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stddef.h" 3 4 */
typedef int ptrdiff_t;
/* # 324 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stddef.h" 3 4 */
typedef int wchar_t;
/* # 15 "/usr/include/linux/stddef.h" 2 3 4 */
/* # 5 "/usr/include/linux/posix_types.h" 2 3 4 */
/* # 36 "/usr/include/linux/posix_types.h" 3 4 */
typedef struct {
 unsigned long fds_bits [(1024/(8 * sizeof(unsigned long)))];
} __kernel_fd_set;


typedef void (*__kernel_sighandler_t)(int);


typedef int __kernel_key_t;
typedef int __kernel_mqd_t;

/* # 1 "/usr/include/asm/posix_types.h" 1 3 4 */

/* # 1 "/usr/include/asm/posix_types_32.h" 1 3 4 */
/* # 10 "/usr/include/asm/posix_types_32.h" 3 4 */
typedef unsigned long __kernel_ino_t;
typedef unsigned short __kernel_mode_t;
typedef unsigned short __kernel_nlink_t;
typedef long __kernel_off_t;
typedef int __kernel_pid_t;
typedef unsigned short __kernel_ipc_pid_t;
typedef unsigned short __kernel_uid_t;
typedef unsigned short __kernel_gid_t;
typedef unsigned int __kernel_size_t;
typedef int __kernel_ssize_t;
typedef int __kernel_ptrdiff_t;
typedef long __kernel_time_t;
typedef long __kernel_suseconds_t;
typedef long __kernel_clock_t;
typedef int __kernel_timer_t;
typedef int __kernel_clockid_t;
typedef int __kernel_daddr_t;
typedef char * __kernel_caddr_t;
typedef unsigned short __kernel_uid16_t;
typedef unsigned short __kernel_gid16_t;
typedef unsigned int __kernel_uid32_t;
typedef unsigned int __kernel_gid32_t;

typedef unsigned short __kernel_old_uid_t;
typedef unsigned short __kernel_old_gid_t;
typedef unsigned short __kernel_old_dev_t;


typedef long long __kernel_loff_t;


typedef struct {
 int val[2];
} __kernel_fsid_t;
/* # 3 "/usr/include/asm/posix_types.h" 2 3 4 */
/* # 48 "/usr/include/linux/posix_types.h" 2 3 4 */
/* # 9 "/usr/include/linux/types.h" 2 3 4 */
/* # 27 "/usr/include/linux/types.h" 3 4 */
typedef __u16 __le16;
typedef __u16 __be16;
typedef __u32 __le32;
typedef __u32 __be32;
typedef __u64 __le64;
typedef __u64 __be64;

typedef __u16 __sum16;
typedef __u32 __wsum;
/* # 25 "../include/libdevmapper.h" 2 */


/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include-fixed/limits.h" 1 3 4 */
/* # 34 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include-fixed/limits.h" 3 4 */
/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include-fixed/syslimits.h" 1 3 4 */






/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include-fixed/limits.h" 1 3 4 */
/* # 169 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include-fixed/limits.h" 3 4 */
/* # 1 "/usr/include/limits.h" 1 3 4 */
/* # 145 "/usr/include/limits.h" 3 4 */
/* # 1 "/usr/include/bits/posix1_lim.h" 1 3 4 */
/* # 157 "/usr/include/bits/posix1_lim.h" 3 4 */
/* # 1 "/usr/include/bits/local_lim.h" 1 3 4 */
/* # 39 "/usr/include/bits/local_lim.h" 3 4 */
/* # 1 "/usr/include/linux/limits.h" 1 3 4 */
/* # 40 "/usr/include/bits/local_lim.h" 2 3 4 */
/* # 158 "/usr/include/bits/posix1_lim.h" 2 3 4 */
/* # 146 "/usr/include/limits.h" 2 3 4 */



/* # 1 "/usr/include/bits/posix2_lim.h" 1 3 4 */
/* # 150 "/usr/include/limits.h" 2 3 4 */



/* # 1 "/usr/include/bits/xopen_lim.h" 1 3 4 */
/* # 34 "/usr/include/bits/xopen_lim.h" 3 4 */
/* # 1 "/usr/include/bits/stdio_lim.h" 1 3 4 */
/* # 35 "/usr/include/bits/xopen_lim.h" 2 3 4 */
/* # 154 "/usr/include/limits.h" 2 3 4 */
/* # 170 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include-fixed/limits.h" 2 3 4 */
/* # 8 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include-fixed/syslimits.h" 2 3 4 */
/* # 35 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include-fixed/limits.h" 2 3 4 */
/* # 28 "../include/libdevmapper.h" 2 */
/* # 1 "/usr/include/string.h" 1 3 4 */
/* # 29 "/usr/include/string.h" 3 4 */





/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stddef.h" 1 3 4 */
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
/* # 163 "/usr/include/string.h" 2 3 4 */


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
/* # 634 "/usr/include/string.h" 3 4 */
/* # 1 "/usr/include/bits/string.h" 1 3 4 */
/* # 635 "/usr/include/string.h" 2 3 4 */


/* # 1 "/usr/include/bits/string2.h" 1 3 4 */
/* # 394 "/usr/include/bits/string2.h" 3 4 */
extern void *__rawmemchr (const void *__s, int __c);
/* # 969 "/usr/include/bits/string2.h" 3 4 */
extern __inline size_t __strcspn_c1 (__const char *__s, int __reject);
extern __inline size_t
__strcspn_c1 (__const char *__s, int __reject)
{
  register size_t __result = 0;
  while (__s[__result] != '\0' && __s[__result] != __reject)
    ++__result;
  return __result;
}

extern __inline size_t __strcspn_c2 (__const char *__s, int __reject1,
         int __reject2);
extern __inline size_t
__strcspn_c2 (__const char *__s, int __reject1, int __reject2)
{
  register size_t __result = 0;
  while (__s[__result] != '\0' && __s[__result] != __reject1
  && __s[__result] != __reject2)
    ++__result;
  return __result;
}

extern __inline size_t __strcspn_c3 (__const char *__s, int __reject1,
         int __reject2, int __reject3);
extern __inline size_t
__strcspn_c3 (__const char *__s, int __reject1, int __reject2,
       int __reject3)
{
  register size_t __result = 0;
  while (__s[__result] != '\0' && __s[__result] != __reject1
  && __s[__result] != __reject2 && __s[__result] != __reject3)
    ++__result;
  return __result;
}
/* # 1045 "/usr/include/bits/string2.h" 3 4 */
extern __inline size_t __strspn_c1 (__const char *__s, int __accept);
extern __inline size_t
__strspn_c1 (__const char *__s, int __accept)
{
  register size_t __result = 0;

  while (__s[__result] == __accept)
    ++__result;
  return __result;
}

extern __inline size_t __strspn_c2 (__const char *__s, int __accept1,
        int __accept2);
extern __inline size_t
__strspn_c2 (__const char *__s, int __accept1, int __accept2)
{
  register size_t __result = 0;

  while (__s[__result] == __accept1 || __s[__result] == __accept2)
    ++__result;
  return __result;
}

extern __inline size_t __strspn_c3 (__const char *__s, int __accept1,
        int __accept2, int __accept3);
extern __inline size_t
__strspn_c3 (__const char *__s, int __accept1, int __accept2, int __accept3)
{
  register size_t __result = 0;

  while (__s[__result] == __accept1 || __s[__result] == __accept2
  || __s[__result] == __accept3)
    ++__result;
  return __result;
}
/* # 1121 "/usr/include/bits/string2.h" 3 4 */
extern __inline char *__strpbrk_c2 (__const char *__s, int __accept1,
         int __accept2);
extern __inline char *
__strpbrk_c2 (__const char *__s, int __accept1, int __accept2)
{

  while (*__s != '\0' && *__s != __accept1 && *__s != __accept2)
    ++__s;
  return *__s == '\0' ? ((void *)0) : (char *) (size_t) __s;
}

extern __inline char *__strpbrk_c3 (__const char *__s, int __accept1,
         int __accept2, int __accept3);
extern __inline char *
__strpbrk_c3 (__const char *__s, int __accept1, int __accept2,
       int __accept3)
{

  while (*__s != '\0' && *__s != __accept1 && *__s != __accept2
  && *__s != __accept3)
    ++__s;
  return *__s == '\0' ? ((void *)0) : (char *) (size_t) __s;
}
/* # 1172 "/usr/include/bits/string2.h" 3 4 */
extern __inline char *__strtok_r_1c (char *__s, char __sep, char **__nextp);
extern __inline char *
__strtok_r_1c (char *__s, char __sep, char **__nextp)
{
  char *__result;
  if (__s == ((void *)0))
    __s = *__nextp;
  while (*__s == __sep)
    ++__s;
  __result = ((void *)0);
  if (*__s != '\0')
    {
      __result = __s++;
      while (*__s != '\0')
 if (*__s++ == __sep)
   {
     __s[-1] = '\0';
     break;
   }
    }
  *__nextp = __s;
  return __result;
}
/* # 1204 "/usr/include/bits/string2.h" 3 4 */
extern char *__strsep_g (char **__stringp, __const char *__delim);
/* # 1222 "/usr/include/bits/string2.h" 3 4 */
extern __inline char *__strsep_1c (char **__s, char __reject);
extern __inline char *
__strsep_1c (char **__s, char __reject)
{
  register char *__retval = *__s;
  if (__retval != ((void *)0) && (*__s = (__extension__ (__builtin_constant_p (__reject) && !__builtin_constant_p (__retval) && (__reject) == '\0' ? (char *) __rawmemchr (__retval, __reject) : __builtin_strchr (__retval, __reject)))) != ((void *)0))
    *(*__s)++ = '\0';
  return __retval;
}

extern __inline char *__strsep_2c (char **__s, char __reject1, char __reject2);
extern __inline char *
__strsep_2c (char **__s, char __reject1, char __reject2)
{
  register char *__retval = *__s;
  if (__retval != ((void *)0))
    {
      register char *__cp = __retval;
      while (1)
 {
   if (*__cp == '\0')
     {
       __cp = ((void *)0);
   break;
     }
   if (*__cp == __reject1 || *__cp == __reject2)
     {
       *__cp++ = '\0';
       break;
     }
   ++__cp;
 }
      *__s = __cp;
    }
  return __retval;
}

extern __inline char *__strsep_3c (char **__s, char __reject1, char __reject2,
       char __reject3);
extern __inline char *
__strsep_3c (char **__s, char __reject1, char __reject2, char __reject3)
{
  register char *__retval = *__s;
  if (__retval != ((void *)0))
    {
      register char *__cp = __retval;
      while (1)
 {
   if (*__cp == '\0')
     {
       __cp = ((void *)0);
   break;
     }
   if (*__cp == __reject1 || *__cp == __reject2 || *__cp == __reject3)
     {
       *__cp++ = '\0';
       break;
     }
   ++__cp;
 }
      *__s = __cp;
    }
  return __retval;
}
/* # 1298 "/usr/include/bits/string2.h" 3 4 */
/* # 1 "/usr/include/stdlib.h" 1 3 4 */
/* # 33 "/usr/include/stdlib.h" 3 4 */
/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stddef.h" 1 3 4 */
/* # 34 "/usr/include/stdlib.h" 2 3 4 */


/* # 469 "/usr/include/stdlib.h" 3 4 */


extern void *malloc (size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__));

extern void *calloc (size_t __nmemb, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__));

/* # 964 "/usr/include/stdlib.h" 3 4 */

/* # 1299 "/usr/include/bits/string2.h" 2 3 4 */




extern char *__strdup (__const char *__string) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__));
/* # 1322 "/usr/include/bits/string2.h" 3 4 */
extern char *__strndup (__const char *__string, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__));
/* # 638 "/usr/include/string.h" 2 3 4 */




/* # 1 "/usr/include/bits/string3.h" 1 3 4 */
/* # 23 "/usr/include/bits/string3.h" 3 4 */
extern void __warn_memset_zero_len (void) __attribute__((__warning__ ("memset used with constant zero length parameter; this could be due to transposed parameters")))
                                                                                                   ;
/* # 48 "/usr/include/bits/string3.h" 3 4 */
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) void *
__attribute__ ((__nothrow__)) memcpy (void *__restrict __dest, __const void *__restrict __src, size_t __len)

{
  return __builtin___memcpy_chk (__dest, __src, __len, __builtin_object_size (__dest, 0));
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) void *
__attribute__ ((__nothrow__)) memmove (void *__dest, __const void *__src, size_t __len)
{
  return __builtin___memmove_chk (__dest, __src, __len, __builtin_object_size (__dest, 0));
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) void *
__attribute__ ((__nothrow__)) mempcpy (void *__restrict __dest, __const void *__restrict __src, size_t __len)

{
  return __builtin___mempcpy_chk (__dest, __src, __len, __builtin_object_size (__dest, 0));
}
/* # 76 "/usr/include/bits/string3.h" 3 4 */
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) void *
__attribute__ ((__nothrow__)) memset (void *__dest, int __ch, size_t __len)
{
  if (__builtin_constant_p (__len) && __len == 0
      && (!__builtin_constant_p (__ch) || __ch != 0))
    {
      __warn_memset_zero_len ();
      return __dest;
    }
  return __builtin___memset_chk (__dest, __ch, __len, __builtin_object_size (__dest, 0));
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) void
__attribute__ ((__nothrow__)) bcopy (__const void *__src, void *__dest, size_t __len)
{
  (void) __builtin___memmove_chk (__dest, __src, __len, __builtin_object_size (__dest, 0));
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) void
__attribute__ ((__nothrow__)) bzero (void *__dest, size_t __len)
{
  (void) __builtin___memset_chk (__dest, '\0', __len, __builtin_object_size (__dest, 0));
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__)) strcpy (char *__restrict __dest, __const char *__restrict __src)
{
  return __builtin___strcpy_chk (__dest, __src, __builtin_object_size (__dest, 2 > 1));
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__)) stpcpy (char *__restrict __dest, __const char *__restrict __src)
{
  return __builtin___stpcpy_chk (__dest, __src, __builtin_object_size (__dest, 2 > 1));
}



extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__)) strncpy (char *__restrict __dest, __const char *__restrict __src, size_t __len)

{
  return __builtin___strncpy_chk (__dest, __src, __len, __builtin_object_size (__dest, 2 > 1));
}


extern char *__stpncpy_chk (char *__dest, __const char *__src, size_t __n,
       size_t __destlen) __attribute__ ((__nothrow__));
extern char *__stpncpy_alias (char *__dest, __const char *__src, size_t __n) __asm__ ("" "stpncpy") __attribute__ ((__nothrow__))

                                 ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__)) stpncpy (char *__dest, __const char *__src, size_t __n)
{
  if (__builtin_object_size (__dest, 2 > 1) != (size_t) -1
      && (!__builtin_constant_p (__n) || __n <= __builtin_object_size (__dest, 2 > 1)))
    return __stpncpy_chk (__dest, __src, __n, __builtin_object_size (__dest, 2 > 1));
  return __stpncpy_alias (__dest, __src, __n);
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__)) strcat (char *__restrict __dest, __const char *__restrict __src)
{
  return __builtin___strcat_chk (__dest, __src, __builtin_object_size (__dest, 2 > 1));
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__)) strncat (char *__restrict __dest, __const char *__restrict __src, size_t __len)

{
  return __builtin___strncat_chk (__dest, __src, __len, __builtin_object_size (__dest, 2 > 1));
}
/* # 643 "/usr/include/string.h" 2 3 4 */




/* # 29 "../include/libdevmapper.h" 2 */
/* # 1 "/usr/include/stdlib.h" 1 3 4 */
/* # 33 "/usr/include/stdlib.h" 3 4 */
/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stddef.h" 1 3 4 */
/* # 34 "/usr/include/stdlib.h" 2 3 4 */








/* # 1 "/usr/include/bits/waitflags.h" 1 3 4 */
/* # 43 "/usr/include/stdlib.h" 2 3 4 */
/* # 1 "/usr/include/bits/waitstatus.h" 1 3 4 */
/* # 67 "/usr/include/bits/waitstatus.h" 3 4 */
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
extern size_t __ctype_get_mb_cur_max (void) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));




extern double atof (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

extern int atoi (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

extern long int atol (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





__extension__ extern long long int atoll (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern double strtod (__const char *__restrict __nptr,
        char **__restrict __endptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern float strtof (__const char *__restrict __nptr,
       char **__restrict __endptr) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

extern long double strtold (__const char *__restrict __nptr,
       char **__restrict __endptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern long int strtol (__const char *__restrict __nptr,
   char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

extern unsigned long int strtoul (__const char *__restrict __nptr,
      char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));




__extension__
extern long long int strtoq (__const char *__restrict __nptr,
        char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

__extension__
extern unsigned long long int strtouq (__const char *__restrict __nptr,
           char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





__extension__
extern long long int strtoll (__const char *__restrict __nptr,
         char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

__extension__
extern unsigned long long int strtoull (__const char *__restrict __nptr,
     char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

/* # 240 "/usr/include/stdlib.h" 3 4 */
extern long int strtol_l (__const char *__restrict __nptr,
     char **__restrict __endptr, int __base,
     __locale_t __loc) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 4))) __attribute__ ((__warn_unused_result__));

extern unsigned long int strtoul_l (__const char *__restrict __nptr,
        char **__restrict __endptr,
        int __base, __locale_t __loc)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 4))) __attribute__ ((__warn_unused_result__));

__extension__
extern long long int strtoll_l (__const char *__restrict __nptr,
    char **__restrict __endptr, int __base,
    __locale_t __loc)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 4))) __attribute__ ((__warn_unused_result__));

__extension__
extern unsigned long long int strtoull_l (__const char *__restrict __nptr,
       char **__restrict __endptr,
       int __base, __locale_t __loc)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 4))) __attribute__ ((__warn_unused_result__));

extern double strtod_l (__const char *__restrict __nptr,
   char **__restrict __endptr, __locale_t __loc)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 3))) __attribute__ ((__warn_unused_result__));

extern float strtof_l (__const char *__restrict __nptr,
         char **__restrict __endptr, __locale_t __loc)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 3))) __attribute__ ((__warn_unused_result__));

extern long double strtold_l (__const char *__restrict __nptr,
         char **__restrict __endptr,
         __locale_t __loc)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 3))) __attribute__ ((__warn_unused_result__));





extern __inline double
__attribute__ ((__nothrow__)) atof (__const char *__nptr)
{
  return strtod (__nptr, (char **) ((void *)0));
}
extern __inline int
__attribute__ ((__nothrow__)) atoi (__const char *__nptr)
{
  return (int) strtol (__nptr, (char **) ((void *)0), 10);
}
extern __inline long int
__attribute__ ((__nothrow__)) atol (__const char *__nptr)
{
  return strtol (__nptr, (char **) ((void *)0), 10);
}




__extension__ extern __inline long long int
__attribute__ ((__nothrow__)) atoll (__const char *__nptr)
{
  return strtoll (__nptr, (char **) ((void *)0), 10);
}

/* # 311 "/usr/include/stdlib.h" 3 4 */
extern char *l64a (long int __n) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));


extern long int a64l (__const char *__s)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
/* # 327 "/usr/include/stdlib.h" 3 4 */
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
/* # 479 "/usr/include/stdlib.h" 3 4 */






extern void *realloc (void *__ptr, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));

extern void free (void *__ptr) __attribute__ ((__nothrow__));




extern void cfree (void *__ptr) __attribute__ ((__nothrow__));



/* # 1 "/usr/include/alloca.h" 1 3 4 */
/* # 25 "/usr/include/alloca.h" 3 4 */
/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stddef.h" 1 3 4 */
/* # 26 "/usr/include/alloca.h" 2 3 4 */







extern void *alloca (size_t __size) __attribute__ ((__nothrow__));






/* # 498 "/usr/include/stdlib.h" 2 3 4 */





extern void *valloc (size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__));




extern int posix_memalign (void **__memptr, size_t __alignment, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));




extern void abort (void) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));



extern int atexit (void (*__func) (void)) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
/* # 528 "/usr/include/stdlib.h" 3 4 */
extern int at_quick_exit (void (*__func) (void)) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));







extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern void exit (int __status) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));







extern void quick_exit (int __status) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));







extern void _Exit (int __status) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));






extern char *getenv (__const char *__name) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));




extern char *__secure_getenv (__const char *__name)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern int putenv (char *__string) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int setenv (__const char *__name, __const char *__value, int __replace)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));


extern int unsetenv (__const char *__name) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern int clearenv (void) __attribute__ ((__nothrow__));
/* # 606 "/usr/include/stdlib.h" 3 4 */
extern char *mktemp (char *__template) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
/* # 623 "/usr/include/stdlib.h" 3 4 */
extern int mkstemp (char *__template) __asm__ ("" "mkstemp64")
     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern int mkstemp64 (char *__template) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
/* # 645 "/usr/include/stdlib.h" 3 4 */
extern int mkstemps (char *__template, int __suffixlen) __asm__ ("" "mkstemps64")
                     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern int mkstemps64 (char *__template, int __suffixlen)
     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
/* # 663 "/usr/include/stdlib.h" 3 4 */
extern char *mkdtemp (char *__template) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
/* # 677 "/usr/include/stdlib.h" 3 4 */
extern int mkostemp (char *__template, int __flags) __asm__ ("" "mkostemp64")
     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern int mkostemp64 (char *__template, int __flags) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
/* # 698 "/usr/include/stdlib.h" 3 4 */
extern int mkostemps (char *__template, int __suffixlen, int __flags) __asm__ ("" "mkostemps64")

     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern int mkostemps64 (char *__template, int __suffixlen, int __flags)
     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));









extern int system (__const char *__command) __attribute__ ((__warn_unused_result__));






extern char *canonicalize_file_name (__const char *__name)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
/* # 734 "/usr/include/stdlib.h" 3 4 */
extern char *realpath (__const char *__restrict __name,
         char *__restrict __resolved) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));






typedef int (*__compar_fn_t) (__const void *, __const void *);


typedef __compar_fn_t comparison_fn_t;



typedef int (*__compar_d_fn_t) (__const void *, __const void *, void *);





extern void *bsearch (__const void *__key, __const void *__base,
        size_t __nmemb, size_t __size, __compar_fn_t __compar)
     __attribute__ ((__nonnull__ (1, 2, 5))) __attribute__ ((__warn_unused_result__));



extern void qsort (void *__base, size_t __nmemb, size_t __size,
     __compar_fn_t __compar) __attribute__ ((__nonnull__ (1, 4)));

extern void qsort_r (void *__base, size_t __nmemb, size_t __size,
       __compar_d_fn_t __compar, void *__arg)
  __attribute__ ((__nonnull__ (1, 4)));




extern int abs (int __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));
extern long int labs (long int __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));



__extension__ extern long long int llabs (long long int __x)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));







extern div_t div (int __numer, int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));
extern ldiv_t ldiv (long int __numer, long int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));




__extension__ extern lldiv_t lldiv (long long int __numer,
        long long int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));

/* # 808 "/usr/include/stdlib.h" 3 4 */
extern char *ecvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) __attribute__ ((__warn_unused_result__));




extern char *fcvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) __attribute__ ((__warn_unused_result__));




extern char *gcvt (double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3))) __attribute__ ((__warn_unused_result__));




extern char *qecvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) __attribute__ ((__warn_unused_result__));
extern char *qfcvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) __attribute__ ((__warn_unused_result__));
extern char *qgcvt (long double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3))) __attribute__ ((__warn_unused_result__));




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







extern int mblen (__const char *__s, size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));


extern int mbtowc (wchar_t *__restrict __pwc,
     __const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));


extern int wctomb (char *__s, wchar_t __wchar) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));



extern size_t mbstowcs (wchar_t *__restrict __pwcs,
   __const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__));

extern size_t wcstombs (char *__restrict __s,
   __const wchar_t *__restrict __pwcs, size_t __n)
     __attribute__ ((__nothrow__));








extern int rpmatch (__const char *__response) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
/* # 896 "/usr/include/stdlib.h" 3 4 */
extern int getsubopt (char **__restrict __optionp,
        char *__const *__restrict __tokens,
        char **__restrict __valuep)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2, 3))) __attribute__ ((__warn_unused_result__));





extern void setkey (__const char *__key) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));







extern int posix_openpt (int __oflag) __attribute__ ((__warn_unused_result__));







extern int grantpt (int __fd) __attribute__ ((__nothrow__));



extern int unlockpt (int __fd) __attribute__ ((__nothrow__));




extern char *ptsname (int __fd) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));






extern int ptsname_r (int __fd, char *__buf, size_t __buflen)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));


extern int getpt (void);






extern int getloadavg (double __loadavg[], int __nelem)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





/* # 1 "/usr/include/bits/stdlib.h" 1 3 4 */
/* # 24 "/usr/include/bits/stdlib.h" 3 4 */
extern char *__realpath_chk (__const char *__restrict __name,
        char *__restrict __resolved,
        size_t __resolvedlen) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));
extern char *__realpath_alias (__const char *__restrict __name, char *__restrict __resolved) __asm__ ("" "realpath") __attribute__ ((__nothrow__))

                                                 __attribute__ ((__warn_unused_result__));
extern char *__realpath_chk_warn (__const char *__restrict __name, char *__restrict __resolved, size_t __resolvedlen) __asm__ ("" "__realpath_chk") __attribute__ ((__nothrow__))


                                                __attribute__ ((__warn_unused_result__))
     __attribute__((__warning__ ("second argument of realpath must be either NULL or at " "least PATH_MAX bytes long buffer")))
                                      ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) char *
__attribute__ ((__nothrow__)) realpath (__const char *__restrict __name, char *__restrict __resolved)
{
  if (__builtin_object_size (__resolved, 2 > 1) != (size_t) -1)
    {

      if (__builtin_object_size (__resolved, 2 > 1) < 4096)
 return __realpath_chk_warn (__name, __resolved, __builtin_object_size (__resolved, 2 > 1));

      return __realpath_chk (__name, __resolved, __builtin_object_size (__resolved, 2 > 1));
    }

  return __realpath_alias (__name, __resolved);
}


extern int __ptsname_r_chk (int __fd, char *__buf, size_t __buflen,
       size_t __nreal) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));
extern int __ptsname_r_alias (int __fd, char *__buf, size_t __buflen) __asm__ ("" "ptsname_r") __attribute__ ((__nothrow__))

     __attribute__ ((__nonnull__ (2)));
extern int __ptsname_r_chk_warn (int __fd, char *__buf, size_t __buflen, size_t __nreal) __asm__ ("" "__ptsname_r_chk") __attribute__ ((__nothrow__))


     __attribute__ ((__nonnull__ (2))) __attribute__((__warning__ ("ptsname_r called with buflen bigger than " "size of buf")))
                   ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__)) ptsname_r (int __fd, char *__buf, size_t __buflen)
{
  if (__builtin_object_size (__buf, 2 > 1) != (size_t) -1)
    {
      if (!__builtin_constant_p (__buflen))
 return __ptsname_r_chk (__fd, __buf, __buflen, __builtin_object_size (__buf, 2 > 1));
      if (__buflen > __builtin_object_size (__buf, 2 > 1))
 return __ptsname_r_chk_warn (__fd, __buf, __buflen, __builtin_object_size (__buf, 2 > 1));
    }
  return __ptsname_r_alias (__fd, __buf, __buflen);
}


extern int __wctomb_chk (char *__s, wchar_t __wchar, size_t __buflen)
  __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));
extern int __wctomb_alias (char *__s, wchar_t __wchar) __asm__ ("" "wctomb") __attribute__ ((__nothrow__))
              __attribute__ ((__warn_unused_result__));

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) int
__attribute__ ((__nothrow__)) wctomb (char *__s, wchar_t __wchar)
{







  if (__builtin_object_size (__s, 2 > 1) != (size_t) -1 && 16 > __builtin_object_size (__s, 2 > 1))
    return __wctomb_chk (__s, __wchar, __builtin_object_size (__s, 2 > 1));
  return __wctomb_alias (__s, __wchar);
}


extern size_t __mbstowcs_chk (wchar_t *__restrict __dst,
         __const char *__restrict __src,
         size_t __len, size_t __dstlen) __attribute__ ((__nothrow__));
extern size_t __mbstowcs_alias (wchar_t *__restrict __dst, __const char *__restrict __src, size_t __len) __asm__ ("" "mbstowcs") __attribute__ ((__nothrow__))


                                  ;
extern size_t __mbstowcs_chk_warn (wchar_t *__restrict __dst, __const char *__restrict __src, size_t __len, size_t __dstlen) __asm__ ("" "__mbstowcs_chk") __attribute__ ((__nothrow__))



     __attribute__((__warning__ ("mbstowcs called with dst buffer smaller than len " "* sizeof (wchar_t)")))
                        ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) size_t
__attribute__ ((__nothrow__)) mbstowcs (wchar_t *__restrict __dst, __const char *__restrict __src, size_t __len)

{
  if (__builtin_object_size (__dst, 2 > 1) != (size_t) -1)
    {
      if (!__builtin_constant_p (__len))
 return __mbstowcs_chk (__dst, __src, __len,
          __builtin_object_size (__dst, 2 > 1) / sizeof (wchar_t));

      if (__len > __builtin_object_size (__dst, 2 > 1) / sizeof (wchar_t))
 return __mbstowcs_chk_warn (__dst, __src, __len,
         __builtin_object_size (__dst, 2 > 1) / sizeof (wchar_t));
    }
  return __mbstowcs_alias (__dst, __src, __len);
}


extern size_t __wcstombs_chk (char *__restrict __dst,
         __const wchar_t *__restrict __src,
         size_t __len, size_t __dstlen) __attribute__ ((__nothrow__));
extern size_t __wcstombs_alias (char *__restrict __dst, __const wchar_t *__restrict __src, size_t __len) __asm__ ("" "wcstombs") __attribute__ ((__nothrow__))


                                  ;
extern size_t __wcstombs_chk_warn (char *__restrict __dst, __const wchar_t *__restrict __src, size_t __len, size_t __dstlen) __asm__ ("" "__wcstombs_chk") __attribute__ ((__nothrow__))



     __attribute__((__warning__ ("wcstombs called with dst buffer smaller than len")));

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) size_t
__attribute__ ((__nothrow__)) wcstombs (char *__restrict __dst, __const wchar_t *__restrict __src, size_t __len)

{
  if (__builtin_object_size (__dst, 2 > 1) != (size_t) -1)
    {
      if (!__builtin_constant_p (__len))
 return __wcstombs_chk (__dst, __src, __len, __builtin_object_size (__dst, 2 > 1));
      if (__len > __builtin_object_size (__dst, 2 > 1))
 return __wcstombs_chk_warn (__dst, __src, __len, __builtin_object_size (__dst, 2 > 1));
    }
  return __wcstombs_alias (__dst, __src, __len);
}
/* # 956 "/usr/include/stdlib.h" 2 3 4 */
/* # 964 "/usr/include/stdlib.h" 3 4 */

/* # 30 "../include/libdevmapper.h" 2 */
/* # 1 "/usr/include/stdio.h" 1 3 4 */
/* # 30 "/usr/include/stdio.h" 3 4 */




/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stddef.h" 1 3 4 */
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
/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stddef.h" 1 3 4 */
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
/* # 109 "/usr/include/stdio.h" 3 4 */




typedef _G_fpos64_t fpos_t;



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



/* # 195 "/usr/include/stdio.h" 3 4 */
extern FILE *tmpfile (void) __asm__ ("" "tmpfile64") __attribute__ ((__warn_unused_result__));






extern FILE *tmpfile64 (void) __attribute__ ((__warn_unused_result__));



extern char *tmpnam (char *__s) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));





extern char *tmpnam_r (char *__s) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));
/* # 224 "/usr/include/stdio.h" 3 4 */
extern char *tempnam (__const char *__dir, __const char *__pfx)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__));








extern int fclose (FILE *__stream);




extern int fflush (FILE *__stream);

/* # 249 "/usr/include/stdio.h" 3 4 */
extern int fflush_unlocked (FILE *__stream);
/* # 259 "/usr/include/stdio.h" 3 4 */
extern int fcloseall (void);




/* # 280 "/usr/include/stdio.h" 3 4 */
extern FILE *fopen (__const char *__restrict __filename, __const char *__restrict __modes) __asm__ ("" "fopen64")

  __attribute__ ((__warn_unused_result__));
extern FILE *freopen (__const char *__restrict __filename, __const char *__restrict __modes, FILE *__restrict __stream) __asm__ ("" "freopen64")


  __attribute__ ((__warn_unused_result__));







extern FILE *fopen64 (__const char *__restrict __filename,
        __const char *__restrict __modes) __attribute__ ((__warn_unused_result__));
extern FILE *freopen64 (__const char *__restrict __filename,
   __const char *__restrict __modes,
   FILE *__restrict __stream) __attribute__ ((__warn_unused_result__));




extern FILE *fdopen (int __fd, __const char *__modes) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));





extern FILE *fopencookie (void *__restrict __magic_cookie,
     __const char *__restrict __modes,
     _IO_cookie_io_functions_t __io_funcs) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));




extern FILE *fmemopen (void *__s, size_t __len, __const char *__modes)
  __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));




extern FILE *open_memstream (char **__bufloc, size_t *__sizeloc) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));






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
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 2, 0))) __attribute__ ((__warn_unused_result__));
extern int __asprintf (char **__restrict __ptr,
         __const char *__restrict __fmt, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 2, 3))) __attribute__ ((__warn_unused_result__));
extern int asprintf (char **__restrict __ptr,
       __const char *__restrict __fmt, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 2, 3))) __attribute__ ((__warn_unused_result__));
/* # 414 "/usr/include/stdio.h" 3 4 */
extern int vdprintf (int __fd, __const char *__restrict __fmt,
       __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 2, 0)));
extern int dprintf (int __fd, __const char *__restrict __fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));








extern int fscanf (FILE *__restrict __stream,
     __const char *__restrict __format, ...) __attribute__ ((__warn_unused_result__));




extern int scanf (__const char *__restrict __format, ...) __attribute__ ((__warn_unused_result__));

extern int sscanf (__const char *__restrict __s,
     __const char *__restrict __format, ...) __attribute__ ((__nothrow__));
/* # 465 "/usr/include/stdio.h" 3 4 */








extern int vfscanf (FILE *__restrict __s, __const char *__restrict __format,
      __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0))) __attribute__ ((__warn_unused_result__));





extern int vscanf (__const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 1, 0))) __attribute__ ((__warn_unused_result__));


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
     __attribute__ ((__warn_unused_result__));






extern char *gets (char *__s) __attribute__ ((__warn_unused_result__));

/* # 642 "/usr/include/stdio.h" 3 4 */
extern char *fgets_unlocked (char *__restrict __s, int __n,
        FILE *__restrict __stream) __attribute__ ((__warn_unused_result__));
/* # 658 "/usr/include/stdio.h" 3 4 */
extern __ssize_t __getdelim (char **__restrict __lineptr,
          size_t *__restrict __n, int __delimiter,
          FILE *__restrict __stream) __attribute__ ((__warn_unused_result__));
extern __ssize_t getdelim (char **__restrict __lineptr,
        size_t *__restrict __n, int __delimiter,
        FILE *__restrict __stream) __attribute__ ((__warn_unused_result__));







extern __ssize_t getline (char **__restrict __lineptr,
       size_t *__restrict __n,
       FILE *__restrict __stream) __attribute__ ((__warn_unused_result__));








extern int fputs (__const char *__restrict __s, FILE *__restrict __stream);





extern int puts (__const char *__s);






extern int ungetc (int __c, FILE *__stream);






extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream) __attribute__ ((__warn_unused_result__));




extern size_t fwrite (__const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s) __attribute__ ((__warn_unused_result__));

/* # 719 "/usr/include/stdio.h" 3 4 */
extern int fputs_unlocked (__const char *__restrict __s,
      FILE *__restrict __stream);
/* # 730 "/usr/include/stdio.h" 3 4 */
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
         size_t __n, FILE *__restrict __stream) __attribute__ ((__warn_unused_result__));
extern size_t fwrite_unlocked (__const void *__restrict __ptr, size_t __size,
          size_t __n, FILE *__restrict __stream) __attribute__ ((__warn_unused_result__));








extern int fseek (FILE *__stream, long int __off, int __whence);




extern long int ftell (FILE *__stream) __attribute__ ((__warn_unused_result__));




extern void rewind (FILE *__stream);

/* # 774 "/usr/include/stdio.h" 3 4 */
extern int fseeko (FILE *__stream, __off64_t __off, int __whence) __asm__ ("" "fseeko64")

                  ;
extern __off64_t ftello (FILE *__stream) __asm__ ("" "ftello64");








/* # 799 "/usr/include/stdio.h" 3 4 */
extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos) __asm__ ("" "fgetpos64")
                                          ;
extern int fsetpos (FILE *__stream, __const fpos_t *__pos) __asm__ ("" "fsetpos64")
                                                            ;








extern int fseeko64 (FILE *__stream, __off64_t __off, int __whence);
extern __off64_t ftello64 (FILE *__stream) __attribute__ ((__warn_unused_result__));
extern int fgetpos64 (FILE *__restrict __stream, fpos64_t *__restrict __pos);
extern int fsetpos64 (FILE *__stream, __const fpos64_t *__pos);




extern void clearerr (FILE *__stream) __attribute__ ((__nothrow__));

extern int feof (FILE *__stream) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));

extern int ferror (FILE *__stream) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));




extern void clearerr_unlocked (FILE *__stream) __attribute__ ((__nothrow__));
extern int feof_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));
extern int ferror_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));








extern void perror (__const char *__s);






/* # 1 "/usr/include/bits/sys_errlist.h" 1 3 4 */
/* # 27 "/usr/include/bits/sys_errlist.h" 3 4 */
extern int sys_nerr;
extern __const char *__const sys_errlist[];


extern int _sys_nerr;
extern __const char *__const _sys_errlist[];
/* # 847 "/usr/include/stdio.h" 2 3 4 */




extern int fileno (FILE *__stream) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));




extern int fileno_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));
/* # 866 "/usr/include/stdio.h" 3 4 */
extern FILE *popen (__const char *__command, __const char *__modes) __attribute__ ((__warn_unused_result__));





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



extern int ftrylockfile (FILE *__stream) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));


extern void funlockfile (FILE *__stream) __attribute__ ((__nothrow__));
/* # 927 "/usr/include/stdio.h" 3 4 */
/* # 1 "/usr/include/bits/stdio.h" 1 3 4 */
/* # 44 "/usr/include/bits/stdio.h" 3 4 */
extern __inline int
getchar (void)
{
  return _IO_getc (stdin);
}




extern __inline int
fgetc_unlocked (FILE *__fp)
{
  return (__builtin_expect (((__fp)->_IO_read_ptr >= (__fp)->_IO_read_end), 0) ? __uflow (__fp) : *(unsigned char *) (__fp)->_IO_read_ptr++);
}





extern __inline int
getc_unlocked (FILE *__fp)
{
  return (__builtin_expect (((__fp)->_IO_read_ptr >= (__fp)->_IO_read_end), 0) ? __uflow (__fp) : *(unsigned char *) (__fp)->_IO_read_ptr++);
}


extern __inline int
getchar_unlocked (void)
{
  return (__builtin_expect (((stdin)->_IO_read_ptr >= (stdin)->_IO_read_end), 0) ? __uflow (stdin) : *(unsigned char *) (stdin)->_IO_read_ptr++);
}




extern __inline int
putchar (int __c)
{
  return _IO_putc (__c, stdout);
}




extern __inline int
fputc_unlocked (int __c, FILE *__stream)
{
  return (__builtin_expect (((__stream)->_IO_write_ptr >= (__stream)->_IO_write_end), 0) ? __overflow (__stream, (unsigned char) (__c)) : (unsigned char) (*(__stream)->_IO_write_ptr++ = (__c)));
}





extern __inline int
putc_unlocked (int __c, FILE *__stream)
{
  return (__builtin_expect (((__stream)->_IO_write_ptr >= (__stream)->_IO_write_end), 0) ? __overflow (__stream, (unsigned char) (__c)) : (unsigned char) (*(__stream)->_IO_write_ptr++ = (__c)));
}


extern __inline int
putchar_unlocked (int __c)
{
  return (__builtin_expect (((stdout)->_IO_write_ptr >= (stdout)->_IO_write_end), 0) ? __overflow (stdout, (unsigned char) (__c)) : (unsigned char) (*(stdout)->_IO_write_ptr++ = (__c)));
}





extern __inline __ssize_t
getline (char **__lineptr, size_t *__n, FILE *__stream)
{
  return __getdelim (__lineptr, __n, '\n', __stream);
}





extern __inline int
__attribute__ ((__nothrow__)) feof_unlocked (FILE *__stream)
{
  return (((__stream)->_flags & 0x10) != 0);
}


extern __inline int
__attribute__ ((__nothrow__)) ferror_unlocked (FILE *__stream)
{
  return (((__stream)->_flags & 0x20) != 0);
}
/* # 928 "/usr/include/stdio.h" 2 3 4 */


/* # 1 "/usr/include/bits/stdio2.h" 1 3 4 */
/* # 24 "/usr/include/bits/stdio2.h" 3 4 */
extern int __sprintf_chk (char *__restrict __s, int __flag, size_t __slen,
     __const char *__restrict __format, ...) __attribute__ ((__nothrow__));
extern int __vsprintf_chk (char *__restrict __s, int __flag, size_t __slen,
      __const char *__restrict __format,
      __gnuc_va_list __ap) __attribute__ ((__nothrow__));


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__)) sprintf (char *__restrict __s, __const char *__restrict __fmt, ...)
{
  return __builtin___sprintf_chk (__s, 2 - 1,
      __builtin_object_size (__s, 2 > 1), __fmt, __builtin_va_arg_pack ());
}






extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__)) vsprintf (char *__restrict __s, __const char *__restrict __fmt, __gnuc_va_list __ap)

{
  return __builtin___vsprintf_chk (__s, 2 - 1,
       __builtin_object_size (__s, 2 > 1), __fmt, __ap);
}



extern int __snprintf_chk (char *__restrict __s, size_t __n, int __flag,
      size_t __slen, __const char *__restrict __format,
      ...) __attribute__ ((__nothrow__));
extern int __vsnprintf_chk (char *__restrict __s, size_t __n, int __flag,
       size_t __slen, __const char *__restrict __format,
       __gnuc_va_list __ap) __attribute__ ((__nothrow__));


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__)) snprintf (char *__restrict __s, size_t __n, __const char *__restrict __fmt, ...)

{
  return __builtin___snprintf_chk (__s, __n, 2 - 1,
       __builtin_object_size (__s, 2 > 1), __fmt, __builtin_va_arg_pack ());
}






extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__)) vsnprintf (char *__restrict __s, size_t __n, __const char *__restrict __fmt, __gnuc_va_list __ap)

{
  return __builtin___vsnprintf_chk (__s, __n, 2 - 1,
        __builtin_object_size (__s, 2 > 1), __fmt, __ap);
}





extern int __fprintf_chk (FILE *__restrict __stream, int __flag,
     __const char *__restrict __format, ...);
extern int __printf_chk (int __flag, __const char *__restrict __format, ...);
extern int __vfprintf_chk (FILE *__restrict __stream, int __flag,
      __const char *__restrict __format, __gnuc_va_list __ap);
extern int __vprintf_chk (int __flag, __const char *__restrict __format,
     __gnuc_va_list __ap);


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
fprintf (FILE *__restrict __stream, __const char *__restrict __fmt, ...)
{
  return __fprintf_chk (__stream, 2 - 1, __fmt,
   __builtin_va_arg_pack ());
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
printf (__const char *__restrict __fmt, ...)
{
  return __printf_chk (2 - 1, __fmt, __builtin_va_arg_pack ());
}







extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
vprintf (__const char *__restrict __fmt, __gnuc_va_list __ap)
{

  return __vfprintf_chk (stdout, 2 - 1, __fmt, __ap);



}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
vfprintf (FILE *__restrict __stream,
   __const char *__restrict __fmt, __gnuc_va_list __ap)
{
  return __vfprintf_chk (__stream, 2 - 1, __fmt, __ap);
}



extern int __asprintf_chk (char **__restrict __ptr, int __flag,
      __const char *__restrict __fmt, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 4))) __attribute__ ((__warn_unused_result__));
extern int __vasprintf_chk (char **__restrict __ptr, int __flag,
       __const char *__restrict __fmt, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 0))) __attribute__ ((__warn_unused_result__));
extern int __dprintf_chk (int __fd, int __flag, __const char *__restrict __fmt,
     ...) __attribute__ ((__format__ (__printf__, 3, 4)));
extern int __vdprintf_chk (int __fd, int __flag,
      __const char *__restrict __fmt, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 3, 0)));
extern int __obstack_printf_chk (struct obstack *__restrict __obstack,
     int __flag, __const char *__restrict __format,
     ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 4)));
extern int __obstack_vprintf_chk (struct obstack *__restrict __obstack,
      int __flag,
      __const char *__restrict __format,
      __gnuc_va_list __args)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 0)));


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__)) asprintf (char **__restrict __ptr, __const char *__restrict __fmt, ...)
{
  return __asprintf_chk (__ptr, 2 - 1, __fmt,
    __builtin_va_arg_pack ());
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__)) __asprintf (char **__restrict __ptr, __const char *__restrict __fmt, ...)

{
  return __asprintf_chk (__ptr, 2 - 1, __fmt,
    __builtin_va_arg_pack ());
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
dprintf (int __fd, __const char *__restrict __fmt, ...)
{
  return __dprintf_chk (__fd, 2 - 1, __fmt,
   __builtin_va_arg_pack ());
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__)) obstack_printf (struct obstack *__restrict __obstack, __const char *__restrict __fmt, ...)

{
  return __obstack_printf_chk (__obstack, 2 - 1, __fmt,
          __builtin_va_arg_pack ());
}
/* # 195 "/usr/include/bits/stdio2.h" 3 4 */
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__)) vasprintf (char **__restrict __ptr, __const char *__restrict __fmt, __gnuc_va_list __ap)

{
  return __vasprintf_chk (__ptr, 2 - 1, __fmt, __ap);
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
vdprintf (int __fd, __const char *__restrict __fmt, __gnuc_va_list __ap)
{
  return __vdprintf_chk (__fd, 2 - 1, __fmt, __ap);
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__)) obstack_vprintf (struct obstack *__restrict __obstack, __const char *__restrict __fmt, __gnuc_va_list __ap)

{
  return __obstack_vprintf_chk (__obstack, 2 - 1, __fmt,
    __ap);
}





extern char *__gets_chk (char *__str, size_t) __attribute__ ((__warn_unused_result__));
extern char *__gets_warn (char *__str) __asm__ ("" "gets")
     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("please use fgets or getline instead, gets can't " "specify buffer size")))
                               ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) char *
gets (char *__str)
{
  if (__builtin_object_size (__str, 2 > 1) != (size_t) -1)
    return __gets_chk (__str, __builtin_object_size (__str, 2 > 1));
  return __gets_warn (__str);
}

extern char *__fgets_chk (char *__restrict __s, size_t __size, int __n,
     FILE *__restrict __stream) __attribute__ ((__warn_unused_result__));
extern char *__fgets_alias (char *__restrict __s, int __n, FILE *__restrict __stream) __asm__ ("" "fgets")

                                        __attribute__ ((__warn_unused_result__));
extern char *__fgets_chk_warn (char *__restrict __s, size_t __size, int __n, FILE *__restrict __stream) __asm__ ("" "__fgets_chk")


     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("fgets called with bigger size than length " "of destination buffer")))
                                 ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) char *
fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
{
  if (__builtin_object_size (__s, 2 > 1) != (size_t) -1)
    {
      if (!__builtin_constant_p (__n) || __n <= 0)
 return __fgets_chk (__s, __builtin_object_size (__s, 2 > 1), __n, __stream);

      if ((size_t) __n > __builtin_object_size (__s, 2 > 1))
 return __fgets_chk_warn (__s, __builtin_object_size (__s, 2 > 1), __n, __stream);
    }
  return __fgets_alias (__s, __n, __stream);
}

extern size_t __fread_chk (void *__restrict __ptr, size_t __ptrlen,
      size_t __size, size_t __n,
      FILE *__restrict __stream) __attribute__ ((__warn_unused_result__));
extern size_t __fread_alias (void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream) __asm__ ("" "fread")


            __attribute__ ((__warn_unused_result__));
extern size_t __fread_chk_warn (void *__restrict __ptr, size_t __ptrlen, size_t __size, size_t __n, FILE *__restrict __stream) __asm__ ("" "__fread_chk")




     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("fread called with bigger size * nmemb than length " "of destination buffer")))
                                 ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) size_t
fread (void *__restrict __ptr, size_t __size, size_t __n,
       FILE *__restrict __stream)
{
  if (__builtin_object_size (__ptr, 0) != (size_t) -1)
    {
      if (!__builtin_constant_p (__size)
   || !__builtin_constant_p (__n)
   || (__size | __n) >= (((size_t) 1) << (8 * sizeof (size_t) / 2)))
 return __fread_chk (__ptr, __builtin_object_size (__ptr, 0), __size, __n, __stream);

      if (__size * __n > __builtin_object_size (__ptr, 0))
 return __fread_chk_warn (__ptr, __builtin_object_size (__ptr, 0), __size, __n, __stream);
    }
  return __fread_alias (__ptr, __size, __n, __stream);
}


extern char *__fgets_unlocked_chk (char *__restrict __s, size_t __size,
       int __n, FILE *__restrict __stream) __attribute__ ((__warn_unused_result__));
extern char *__fgets_unlocked_alias (char *__restrict __s, int __n, FILE *__restrict __stream) __asm__ ("" "fgets_unlocked")

                                                 __attribute__ ((__warn_unused_result__));
extern char *__fgets_unlocked_chk_warn (char *__restrict __s, size_t __size, int __n, FILE *__restrict __stream) __asm__ ("" "__fgets_unlocked_chk")


     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("fgets_unlocked called with bigger size than length " "of destination buffer")))
                                 ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) char *
fgets_unlocked (char *__restrict __s, int __n, FILE *__restrict __stream)
{
  if (__builtin_object_size (__s, 2 > 1) != (size_t) -1)
    {
      if (!__builtin_constant_p (__n) || __n <= 0)
 return __fgets_unlocked_chk (__s, __builtin_object_size (__s, 2 > 1), __n, __stream);

      if ((size_t) __n > __builtin_object_size (__s, 2 > 1))
 return __fgets_unlocked_chk_warn (__s, __builtin_object_size (__s, 2 > 1), __n, __stream);
    }
  return __fgets_unlocked_alias (__s, __n, __stream);
}




extern size_t __fread_unlocked_chk (void *__restrict __ptr, size_t __ptrlen,
        size_t __size, size_t __n,
        FILE *__restrict __stream) __attribute__ ((__warn_unused_result__));
extern size_t __fread_unlocked_alias (void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream) __asm__ ("" "fread_unlocked")


                     __attribute__ ((__warn_unused_result__));
extern size_t __fread_unlocked_chk_warn (void *__restrict __ptr, size_t __ptrlen, size_t __size, size_t __n, FILE *__restrict __stream) __asm__ ("" "__fread_unlocked_chk")




     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("fread_unlocked called with bigger size * nmemb than " "length of destination buffer")))
                                        ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) size_t
fread_unlocked (void *__restrict __ptr, size_t __size, size_t __n,
  FILE *__restrict __stream)
{
  if (__builtin_object_size (__ptr, 0) != (size_t) -1)
    {
      if (!__builtin_constant_p (__size)
   || !__builtin_constant_p (__n)
   || (__size | __n) >= (((size_t) 1) << (8 * sizeof (size_t) / 2)))
 return __fread_unlocked_chk (__ptr, __builtin_object_size (__ptr, 0), __size, __n,
         __stream);

      if (__size * __n > __builtin_object_size (__ptr, 0))
 return __fread_unlocked_chk_warn (__ptr, __builtin_object_size (__ptr, 0), __size, __n,
       __stream);
    }


  if (__builtin_constant_p (__size)
      && __builtin_constant_p (__n)
      && (__size | __n) < (((size_t) 1) << (8 * sizeof (size_t) / 2))
      && __size * __n <= 8)
    {
      size_t __cnt = __size * __n;
      char *__cptr = (char *) __ptr;
      if (__cnt == 0)
 return 0;

      for (; __cnt > 0; --__cnt)
 {
   int __c = (__builtin_expect (((__stream)->_IO_read_ptr >= (__stream)->_IO_read_end), 0) ? __uflow (__stream) : *(unsigned char *) (__stream)->_IO_read_ptr++);
   if (__c == (-1))
     break;
   *__cptr++ = __c;
 }
      return (__cptr - (char *) __ptr) / __size;
    }

  return __fread_unlocked_alias (__ptr, __size, __n, __stream);
}
/* # 931 "/usr/include/stdio.h" 2 3 4 */






/* # 31 "../include/libdevmapper.h" 2 */
/* # 53 "../include/libdevmapper.h" */
typedef void (*dm_log_with_errno_fn) (int level, const char *file, int line,
          int dm_errno, const char *f, ...)
    __attribute__ ((format(printf, 5, 6)));

void dm_log_with_errno_init(dm_log_with_errno_fn fn);
void dm_log_init_verbose(int level);







typedef void (*dm_log_fn) (int level, const char *file, int line,
      const char *f, ...)
    __attribute__ ((format(printf, 4, 5)));

void dm_log_init(dm_log_fn fn);




int dm_log_is_non_default(void);

enum {
 DM_DEVICE_CREATE,
 DM_DEVICE_RELOAD,
 DM_DEVICE_REMOVE,
 DM_DEVICE_REMOVE_ALL,

 DM_DEVICE_SUSPEND,
 DM_DEVICE_RESUME,

 DM_DEVICE_INFO,
 DM_DEVICE_DEPS,
 DM_DEVICE_RENAME,

 DM_DEVICE_VERSION,

 DM_DEVICE_STATUS,
 DM_DEVICE_TABLE,
 DM_DEVICE_WAITEVENT,

 DM_DEVICE_LIST,

 DM_DEVICE_CLEAR,

 DM_DEVICE_MKNODES,

 DM_DEVICE_LIST_VERSIONS,

 DM_DEVICE_TARGET_MSG,

 DM_DEVICE_SET_GEOMETRY
};






struct dm_task;

struct dm_task *dm_task_create(int type);
void dm_task_destroy(struct dm_task *dmt);

int dm_task_set_name(struct dm_task *dmt, const char *name);
int dm_task_set_uuid(struct dm_task *dmt, const char *uuid);




struct dm_info {
 int exists;
 int suspended;
 int live_table;
 int inactive_table;
 int32_t open_count;
 uint32_t event_nr;
 uint32_t major;
 uint32_t minor;
 int read_only;

 int32_t target_count;
};

struct dm_deps {
 uint32_t count;
 uint32_t filler;
 uint64_t device[0];
};

struct dm_names {
 uint64_t dev;
 uint32_t next;
 char name[0];
};

struct dm_versions {
 uint32_t next;
 uint32_t version[3];

 char name[0];
};

int dm_get_library_version(char *version, size_t size);
int dm_task_get_driver_version(struct dm_task *dmt, char *version, size_t size);
int dm_task_get_info(struct dm_task *dmt, struct dm_info *dmi);
const char *dm_task_get_name(const struct dm_task *dmt);
const char *dm_task_get_uuid(const struct dm_task *dmt);

struct dm_deps *dm_task_get_deps(struct dm_task *dmt);
struct dm_names *dm_task_get_names(struct dm_task *dmt);
struct dm_versions *dm_task_get_versions(struct dm_task *dmt);

int dm_task_set_ro(struct dm_task *dmt);
int dm_task_set_newname(struct dm_task *dmt, const char *newname);
int dm_task_set_newuuid(struct dm_task *dmt, const char *newuuid);
int dm_task_set_minor(struct dm_task *dmt, int minor);
int dm_task_set_major(struct dm_task *dmt, int major);
int dm_task_set_major_minor(struct dm_task *dmt, int major, int minor, int allow_default_major_fallback);
int dm_task_set_uid(struct dm_task *dmt, uid_t uid);
int dm_task_set_gid(struct dm_task *dmt, gid_t gid);
int dm_task_set_mode(struct dm_task *dmt, mode_t mode);
int dm_task_set_cookie(struct dm_task *dmt, uint32_t *cookie, uint16_t flags);
int dm_task_set_event_nr(struct dm_task *dmt, uint32_t event_nr);
int dm_task_set_geometry(struct dm_task *dmt, const char *cylinders, const char *heads, const char *sectors, const char *start);
int dm_task_set_message(struct dm_task *dmt, const char *message);
int dm_task_set_sector(struct dm_task *dmt, uint64_t sector);
int dm_task_no_flush(struct dm_task *dmt);
int dm_task_no_open_count(struct dm_task *dmt);
int dm_task_skip_lockfs(struct dm_task *dmt);
int dm_task_query_inactive_table(struct dm_task *dmt);
int dm_task_suppress_identical_reload(struct dm_task *dmt);
int dm_task_secure_data(struct dm_task *dmt);
typedef enum {
 DM_ADD_NODE_ON_RESUME,
 DM_ADD_NODE_ON_CREATE
} dm_add_node_t;
int dm_task_set_add_node(struct dm_task *dmt, dm_add_node_t add_node);
/* # 205 "../include/libdevmapper.h" */
int dm_task_set_read_ahead(struct dm_task *dmt, uint32_t read_ahead,
      uint32_t read_ahead_flags);
uint32_t dm_task_get_read_ahead(const struct dm_task *dmt,
    uint32_t *read_ahead);




int dm_task_add_target(struct dm_task *dmt,
         uint64_t start,
         uint64_t size, const char *ttype, const char *params);





int dm_format_dev(char *buf, int bufsize, uint32_t dev_major, uint32_t dev_minor);


void *dm_get_next_target(struct dm_task *dmt,
    void *next, uint64_t *start, uint64_t *length,
    char **target_type, char **params);




int dm_task_run(struct dm_task *dmt);





void dm_task_update_nodes(void);




int dm_set_dev_dir(const char *dir);
const char *dm_dir(void);




int dm_is_dm_major(uint32_t major);




void dm_lib_release(void);
void dm_lib_exit(void) __attribute__((destructor));




int dm_mknodes(const char *name);
int dm_driver_version(char *version, size_t size);




struct dm_tree;
struct dm_tree_node;
/* # 279 "../include/libdevmapper.h" */
struct dm_tree *dm_tree_create(void);
void dm_tree_free(struct dm_tree *tree);




int dm_tree_add_dev(struct dm_tree *tree, uint32_t major, uint32_t minor);
int dm_tree_add_dev_with_udev_flags(struct dm_tree *tree, uint32_t major,
        uint32_t minor, uint16_t udev_flags);




struct dm_tree_node *dm_tree_add_new_dev(struct dm_tree *tree,
      const char *name,
      const char *uuid,
      uint32_t major, uint32_t minor,
      int read_only,
      int clear_inactive,
      void *context);
struct dm_tree_node *dm_tree_add_new_dev_with_udev_flags(struct dm_tree *tree,
        const char *name,
        const char *uuid,
        uint32_t major,
        uint32_t minor,
        int read_only,
        int clear_inactive,
        void *context,
        uint16_t udev_flags);





struct dm_tree_node *dm_tree_find_node(struct dm_tree *tree,
       uint32_t major,
       uint32_t minor);
struct dm_tree_node *dm_tree_find_node_by_uuid(struct dm_tree *tree,
        const char *uuid);







struct dm_tree_node *dm_tree_next_child(void **handle,
     const struct dm_tree_node *parent,
     uint32_t inverted);




const char *dm_tree_node_get_name(const struct dm_tree_node *node);
const char *dm_tree_node_get_uuid(const struct dm_tree_node *node);
const struct dm_info *dm_tree_node_get_info(const struct dm_tree_node *node);
void *dm_tree_node_get_context(const struct dm_tree_node *node);
int dm_tree_node_size_changed(const struct dm_tree_node *dnode);





int dm_tree_node_num_children(const struct dm_tree_node *node, uint32_t inverted);





int dm_tree_deactivate_children(struct dm_tree_node *dnode,
       const char *uuid_prefix,
       size_t uuid_prefix_len);




int dm_tree_preload_children(struct dm_tree_node *dnode,
        const char *uuid_prefix,
        size_t uuid_prefix_len);





int dm_tree_activate_children(struct dm_tree_node *dnode,
         const char *uuid_prefix,
         size_t uuid_prefix_len);





int dm_tree_suspend_children(struct dm_tree_node *dnode,
       const char *uuid_prefix,
       size_t uuid_prefix_len);






void dm_tree_skip_lockfs(struct dm_tree_node *dnode);
/* # 390 "../include/libdevmapper.h" */
void dm_tree_use_no_flush_suspend(struct dm_tree_node *dnode);






int dm_tree_children_use_uuid(struct dm_tree_node *dnode,
     const char *uuid_prefix,
     size_t uuid_prefix_len);




int dm_tree_node_add_snapshot_origin_target(struct dm_tree_node *dnode,
            uint64_t size,
            const char *origin_uuid);
int dm_tree_node_add_snapshot_target(struct dm_tree_node *node,
     uint64_t size,
     const char *origin_uuid,
     const char *cow_uuid,
     int persistent,
     uint32_t chunk_size);
int dm_tree_node_add_snapshot_merge_target(struct dm_tree_node *node,
          uint64_t size,
          const char *origin_uuid,
          const char *cow_uuid,
          const char *merge_uuid,
          uint32_t chunk_size);
int dm_tree_node_add_error_target(struct dm_tree_node *node,
         uint64_t size);
int dm_tree_node_add_zero_target(struct dm_tree_node *node,
        uint64_t size);
int dm_tree_node_add_linear_target(struct dm_tree_node *node,
          uint64_t size);
int dm_tree_node_add_striped_target(struct dm_tree_node *node,
           uint64_t size,
           uint32_t stripe_size);
/* # 436 "../include/libdevmapper.h" */
int dm_tree_node_add_crypt_target(struct dm_tree_node *node,
      uint64_t size,
      const char *cipher,
      const char *chainmode,
      const char *iv,
      uint64_t iv_offset,
      const char *key);
int dm_tree_node_add_mirror_target(struct dm_tree_node *node,
          uint64_t size);







int dm_tree_node_add_mirror_target_log(struct dm_tree_node *node,
       uint32_t region_size,
       unsigned clustered,
       const char *log_uuid,
       unsigned area_count,
       uint32_t flags);





typedef enum {
 DM_REPLICATOR_SYNC,
 DM_REPLICATOR_ASYNC_WARN,
 DM_REPLICATOR_ASYNC_STALL,
 DM_REPLICATOR_ASYNC_DROP,
 DM_REPLICATOR_ASYNC_FAIL,
 NUM_DM_REPLICATOR_MODES
} dm_replicator_mode_t;

int dm_tree_node_add_replicator_target(struct dm_tree_node *node,
           uint64_t size,
           const char *rlog_uuid,
           const char *rlog_type,
           unsigned rsite_index,
           dm_replicator_mode_t mode,
           uint32_t async_timeout,
           uint64_t fall_behind_data,
           uint32_t fall_behind_ios);

int dm_tree_node_add_replicator_dev_target(struct dm_tree_node *node,
        uint64_t size,
        const char *replicator_uuid,
        uint64_t rdevice_index,
        const char *rdev_uuid,
        unsigned rsite_index,
        const char *slog_uuid,
        uint32_t slog_flags,
        uint32_t slog_region_size);


void dm_tree_node_set_presuspend_node(struct dm_tree_node *node,
          struct dm_tree_node *presuspend_node);

int dm_tree_node_add_target_area(struct dm_tree_node *node,
        const char *dev_name,
        const char *dlid,
        uint64_t offset);




void dm_tree_node_set_read_ahead(struct dm_tree_node *dnode,
     uint32_t read_ahead,
     uint32_t read_ahead_flags);

void dm_tree_set_cookie(struct dm_tree_node *node, uint32_t cookie);
uint32_t dm_tree_get_cookie(struct dm_tree_node *node);
/* # 519 "../include/libdevmapper.h" */
void *dm_malloc_aux(size_t s, const char *file, int line);
void *dm_malloc_aux_debug(size_t s, const char *file, int line);
void *dm_zalloc_aux(size_t s, const char *file, int line);
void *dm_zalloc_aux_debug(size_t s, const char *file, int line);
char *dm_strdup_aux(const char *str, const char *file, int line);
void dm_free_aux(void *p);
void *dm_realloc_aux(void *p, unsigned int s, const char *file, int line);
int dm_dump_memory_debug(void);
void dm_bounds_check_debug(void);
/* # 585 "../include/libdevmapper.h" */
struct dm_pool;


struct dm_pool *dm_pool_create(const char *name, size_t chunk_hint);
void dm_pool_destroy(struct dm_pool *p);


void *dm_pool_alloc(struct dm_pool *p, size_t s);
void *dm_pool_alloc_aligned(struct dm_pool *p, size_t s, unsigned alignment);
void dm_pool_empty(struct dm_pool *p);
void dm_pool_free(struct dm_pool *p, void *ptr);
/* # 642 "../include/libdevmapper.h" */
int dm_pool_begin_object(struct dm_pool *p, size_t hint);
int dm_pool_grow_object(struct dm_pool *p, const void *extra, size_t delta);
void *dm_pool_end_object(struct dm_pool *p);
void dm_pool_abandon_object(struct dm_pool *p);


char *dm_pool_strdup(struct dm_pool *p, const char *str);
char *dm_pool_strndup(struct dm_pool *p, const char *str, size_t n);
void *dm_pool_zalloc(struct dm_pool *p, size_t s);





typedef uint32_t *dm_bitset_t;

dm_bitset_t dm_bitset_create(struct dm_pool *mem, unsigned num_bits);
void dm_bitset_destroy(dm_bitset_t bs);

int dm_bitset_equal(dm_bitset_t in1, dm_bitset_t in2);

void dm_bit_and(dm_bitset_t out, dm_bitset_t in1, dm_bitset_t in2);
void dm_bit_union(dm_bitset_t out, dm_bitset_t in1, dm_bitset_t in2);
int dm_bit_get_first(dm_bitset_t bs);
int dm_bit_get_next(dm_bitset_t bs, int last_bit);
/* # 689 "../include/libdevmapper.h" */
static inline unsigned hweight32(uint32_t i)
{
 unsigned r = (i & 0x55555555) + ((i >> 1) & 0x55555555);

 r = (r & 0x33333333) + ((r >> 2) & 0x33333333);
 r = (r & 0x0F0F0F0F) + ((r >> 4) & 0x0F0F0F0F);
 r = (r & 0x00FF00FF) + ((r >> 8) & 0x00FF00FF);
 return (r & 0x0000FFFF) + ((r >> 16) & 0x0000FFFF);
}





struct dm_hash_table;
struct dm_hash_node;

typedef void (*dm_hash_iterate_fn) (void *data);

struct dm_hash_table *dm_hash_create(unsigned size_hint);
void dm_hash_destroy(struct dm_hash_table *t);
void dm_hash_wipe(struct dm_hash_table *t);

void *dm_hash_lookup(struct dm_hash_table *t, const char *key);
int dm_hash_insert(struct dm_hash_table *t, const char *key, void *data);
void dm_hash_remove(struct dm_hash_table *t, const char *key);

void *dm_hash_lookup_binary(struct dm_hash_table *t, const void *key, uint32_t len);
int dm_hash_insert_binary(struct dm_hash_table *t, const void *key, uint32_t len,
         void *data);
void dm_hash_remove_binary(struct dm_hash_table *t, const void *key, uint32_t len);

unsigned dm_hash_get_num_entries(struct dm_hash_table *t);
void dm_hash_iter(struct dm_hash_table *t, dm_hash_iterate_fn f);

char *dm_hash_get_key(struct dm_hash_table *t, struct dm_hash_node *n);
void *dm_hash_get_data(struct dm_hash_table *t, struct dm_hash_node *n);
struct dm_hash_node *dm_hash_get_first(struct dm_hash_table *t);
struct dm_hash_node *dm_hash_get_next(struct dm_hash_table *t, struct dm_hash_node *n);
/* # 743 "../include/libdevmapper.h" */
struct dm_list {
 struct dm_list *n, *p;
};






void dm_list_init(struct dm_list *head);





void dm_list_add(struct dm_list *head, struct dm_list *elem);





void dm_list_add_h(struct dm_list *head, struct dm_list *elem);






void dm_list_del(struct dm_list *elem);




void dm_list_move(struct dm_list *head, struct dm_list *elem);




void dm_list_splice(struct dm_list *head, struct dm_list *head1);




int dm_list_empty(const struct dm_list *head);




int dm_list_start(const struct dm_list *head, const struct dm_list *elem);




int dm_list_end(const struct dm_list *head, const struct dm_list *elem);




struct dm_list *dm_list_first(const struct dm_list *head);




struct dm_list *dm_list_last(const struct dm_list *head);




struct dm_list *dm_list_prev(const struct dm_list *head, const struct dm_list *elem);




struct dm_list *dm_list_next(const struct dm_list *head, const struct dm_list *elem);
/* # 927 "../include/libdevmapper.h" */
unsigned int dm_list_size(const struct dm_list *head);
/* # 939 "../include/libdevmapper.h" */
int dm_prepare_selinux_context(const char *path, mode_t mode);



int dm_set_selinux_context(const char *path, mode_t mode);
/* # 955 "../include/libdevmapper.h" */
int dm_split_lvm_name(struct dm_pool *mem, const char *dmname,
        char **vgname, char **lvname, char **layer);





int dm_split_words(char *buffer, unsigned max,
     unsigned ignore_comments,
     char **argv);




int dm_snprintf(char *buf, size_t bufsize, const char *format, ...)
    __attribute__ ((format(printf, 3, 4)));




const char *dm_basename(const char *path);
/* # 985 "../include/libdevmapper.h" */
int dm_create_dir(const char *dir);
/* # 995 "../include/libdevmapper.h" */
int dm_fclose(FILE *stream);






int dm_asprintf(char **buf, const char *format, ...)
    __attribute__ ((format(printf, 2, 3)));







int dm_create_lockfile(const char* lockfile);






int dm_daemon_is_running(const char* lockfile);




struct dm_regex;





struct dm_regex *dm_regex_create(struct dm_pool *mem, const char * const *patterns,
     unsigned num_patterns);






int dm_regex_match(struct dm_regex *regex, const char *s);
/* # 1047 "../include/libdevmapper.h" */
uint32_t dm_regex_fingerprint(struct dm_regex *regex);





struct dm_report_object_type {
 uint32_t id;
 const char *desc;
 const char *prefix;
 void *(*data_fn)(void *object);
};

struct dm_report_field;
/* # 1076 "../include/libdevmapper.h" */
struct dm_report;
struct dm_report_field_type {
 uint32_t type;
 uint32_t flags;
 uint32_t offset;
 int32_t width;

 const char id[32];

 const char heading[32];
 int (*report_fn)(struct dm_report *rh, struct dm_pool *mem,
    struct dm_report_field *field, const void *data,
    void *private_data);
 const char *desc;
};
/* # 1103 "../include/libdevmapper.h" */
struct dm_report *dm_report_init(uint32_t *report_types,
     const struct dm_report_object_type *types,
     const struct dm_report_field_type *fields,
     const char *output_fields,
     const char *output_separator,
     uint32_t output_flags,
     const char *sort_keys,
     void *private_data);
int dm_report_object(struct dm_report *rh, void *object);
int dm_report_output(struct dm_report *rh);
void dm_report_free(struct dm_report *rh);




int dm_report_set_output_field_name_prefix(struct dm_report *rh,
        const char *report_prefix);





int dm_report_field_string(struct dm_report *rh, struct dm_report_field *field,
      const char *const *data);
int dm_report_field_int32(struct dm_report *rh, struct dm_report_field *field,
     const int32_t *data);
int dm_report_field_uint32(struct dm_report *rh, struct dm_report_field *field,
      const uint32_t *data);
int dm_report_field_int(struct dm_report *rh, struct dm_report_field *field,
   const int *data);
int dm_report_field_uint64(struct dm_report *rh, struct dm_report_field *field,
      const uint64_t *data);






void dm_report_field_set_value(struct dm_report_field *field, const void *value,
          const void *sortvalue);
/* # 1214 "../include/libdevmapper.h" */
int dm_cookie_supported(void);




void dm_udev_set_sync_support(int sync_with_udev);
int dm_udev_get_sync_support(void);
void dm_udev_set_checking(int checking);
int dm_udev_get_checking(void);





int dm_udev_create_cookie(uint32_t *cookie);
int dm_udev_complete(uint32_t cookie);
int dm_udev_wait(uint32_t cookie);
/* # 30 "../include/lib.h" 2 */
/* # 1 "../include/lvm-globals.h" 1 */
/* # 23 "../include/lvm-globals.h" */
void init_verbose(int level);
void init_test(int level);
void init_md_filtering(int level);
void init_pvmove(int level);
void init_full_scan_done(int level);
void init_trust_cache(int trustcache);
void init_debug(int level);
void init_cmd_name(int status);
void init_ignorelockingfailure(int level);
void init_lockingfailed(int level);
void init_security_level(int level);
void init_mirror_in_sync(int in_sync);
void init_dmeventd_monitor(int reg);
void init_background_polling(int polling);
void init_ignore_suspended_devices(int ignore);
void init_error_message_produced(int produced);
void init_is_static(unsigned value);
void init_udev_checking(int checking);
void init_dev_disable_after_error_count(int value);
void init_pv_min_size(uint64_t sectors);

void set_cmd_name(const char *cmd_name);
void set_sysfs_dir_path(const char *path);

int test_mode(void);
int md_filtering(void);
int pvmove_mode(void);
int full_scan_done(void);
int trust_cache(void);
int verbose_level(void);
int debug_level(void);
int ignorelockingfailure(void);
int lockingfailed(void);
int security_level(void);
int mirror_in_sync(void);
int background_polling(void);
int ignore_suspended_devices(void);
const char *log_command_name(void);
unsigned is_static(void);
int udev_checking(void);
const char *sysfs_dir_path(void);
uint64_t pv_min_size(void);


int dmeventd_monitor_mode(void);


int dev_disable_after_error_count(void);
/* # 31 "../include/lib.h" 2 */
/* # 1 "../include/lvm-wrappers.h" 1 */
/* # 19 "../include/lvm-wrappers.h" */
int lvm_getpagesize(void);




int read_urandom(void *buf, size_t len);
/* # 32 "../include/lib.h" 2 */
/* # 1 "../include/lvm-types.h" 1 */
/* # 27 "../include/lvm-types.h" */
struct str_list {
 struct dm_list list;
 const char *str;
};
/* # 33 "../include/lib.h" 2 */
/* # 1 "../include/util.h" 1 */
/* # 34 "../include/lib.h" 2 */




/* # 1 "../include/lvm-logging.h" 1 */
/* # 19 "../include/lvm-logging.h" */
/* XXX void print_log(int level, const char *file, int line, int dm_errno,
        const char *format, ...)
    __attribute__ ((format(printf, 5, 6))); */







/* # 1 "../include/log.h" 1 */
/* # 42 "../include/log.h" */
/* # 1 "/usr/include/errno.h" 1 3 4 */
/* # 32 "/usr/include/errno.h" 3 4 */




/* # 1 "/usr/include/bits/errno.h" 1 3 4 */
/* # 25 "/usr/include/bits/errno.h" 3 4 */
/* # 1 "/usr/include/linux/errno.h" 1 3 4 */



/* # 1 "/usr/include/asm/errno.h" 1 3 4 */
/* # 1 "/usr/include/asm-generic/errno.h" 1 3 4 */



/* # 1 "/usr/include/asm-generic/errno-base.h" 1 3 4 */
/* # 5 "/usr/include/asm-generic/errno.h" 2 3 4 */
/* # 1 "/usr/include/asm/errno.h" 2 3 4 */
/* # 5 "/usr/include/linux/errno.h" 2 3 4 */
/* # 26 "/usr/include/bits/errno.h" 2 3 4 */
/* # 47 "/usr/include/bits/errno.h" 3 4 */
extern int *__errno_location (void) __attribute__ ((__nothrow__)) __attribute__ ((__const__));
/* # 37 "/usr/include/errno.h" 2 3 4 */
/* # 55 "/usr/include/errno.h" 3 4 */
extern char *program_invocation_name, *program_invocation_short_name;




/* # 69 "/usr/include/errno.h" 3 4 */
typedef int error_t;
/* # 43 "../include/log.h" 2 */
/* # 30 "../include/lvm-logging.h" 2 */

typedef void (*lvm2_log_fn_t) (int level, const char *file, int line,
          int dm_errno, const char *message);

void init_log_fn(lvm2_log_fn_t log_fn);

void init_indent(int indent);
void init_msg_prefix(const char *prefix);

void init_log_file(const char *log_file, int append);
void init_log_direct(const char *log_file, int append);
void init_log_while_suspended(int log_while_suspended);
void init_abort_on_internal_errors(int fatal);

void fin_log(void);
void release_log_memory(void);
void reset_log_duplicated(void);

void init_syslog(int facility);
void fin_syslog(void);

int error_message_produced(void);
void reset_lvm_errno(int store_errmsg);
int stored_errno(void);
const char *stored_errmsg(void);



int log_suppress(int suppress);


void syslog_suppress(int suppress);
/* # 39 "../include/lib.h" 2 */


/* # 1 "/usr/include/errno.h" 1 3 4 */
/* # 42 "../include/lib.h" 2 */



/* # 1 "/usr/include/unistd.h" 1 3 4 */
/* # 28 "/usr/include/unistd.h" 3 4 */

/* # 203 "/usr/include/unistd.h" 3 4 */
/* # 1 "/usr/include/bits/posix_opt.h" 1 3 4 */
/* # 204 "/usr/include/unistd.h" 2 3 4 */



/* # 1 "/usr/include/bits/environments.h" 1 3 4 */
/* # 23 "/usr/include/bits/environments.h" 3 4 */
/* # 1 "/usr/include/bits/wordsize.h" 1 3 4 */
/* # 24 "/usr/include/bits/environments.h" 2 3 4 */
/* # 208 "/usr/include/unistd.h" 2 3 4 */
/* # 227 "/usr/include/unistd.h" 3 4 */
/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include/stddef.h" 1 3 4 */
/* # 228 "/usr/include/unistd.h" 2 3 4 */
/* # 275 "/usr/include/unistd.h" 3 4 */
typedef __socklen_t socklen_t;
/* # 288 "/usr/include/unistd.h" 3 4 */
extern int access (__const char *__name, int __type) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));




extern int euidaccess (__const char *__name, int __type)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int eaccess (__const char *__name, int __type)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern int faccessat (int __fd, __const char *__file, int __type, int __flag)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2))) __attribute__ ((__warn_unused_result__));
/* # 334 "/usr/include/unistd.h" 3 4 */
extern __off64_t lseek (int __fd, __off64_t __offset, int __whence) __asm__ ("" "lseek64") __attribute__ ((__nothrow__))

             ;





extern __off64_t lseek64 (int __fd, __off64_t __offset, int __whence)
     __attribute__ ((__nothrow__));






extern int close (int __fd);






extern ssize_t read (int __fd, void *__buf, size_t __nbytes) __attribute__ ((__warn_unused_result__));





extern ssize_t write (int __fd, __const void *__buf, size_t __n) __attribute__ ((__warn_unused_result__));
/* # 385 "/usr/include/unistd.h" 3 4 */
extern ssize_t pread (int __fd, void *__buf, size_t __nbytes, __off64_t __offset) __asm__ ("" "pread64")

               __attribute__ ((__warn_unused_result__));
extern ssize_t pwrite (int __fd, __const void *__buf, size_t __nbytes, __off64_t __offset) __asm__ ("" "pwrite64")

                __attribute__ ((__warn_unused_result__));
/* # 401 "/usr/include/unistd.h" 3 4 */
extern ssize_t pread64 (int __fd, void *__buf, size_t __nbytes,
   __off64_t __offset) __attribute__ ((__warn_unused_result__));


extern ssize_t pwrite64 (int __fd, __const void *__buf, size_t __n,
    __off64_t __offset) __attribute__ ((__warn_unused_result__));







extern int pipe (int __pipedes[2]) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));




extern int pipe2 (int __pipedes[2], int __flags) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));
/* # 429 "/usr/include/unistd.h" 3 4 */
extern unsigned int alarm (unsigned int __seconds) __attribute__ ((__nothrow__));
/* # 441 "/usr/include/unistd.h" 3 4 */
extern unsigned int sleep (unsigned int __seconds);







extern __useconds_t ualarm (__useconds_t __value, __useconds_t __interval)
     __attribute__ ((__nothrow__));






extern int usleep (__useconds_t __useconds);
/* # 466 "/usr/include/unistd.h" 3 4 */
extern int pause (void);



extern int chown (__const char *__file, __uid_t __owner, __gid_t __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));



extern int fchown (int __fd, __uid_t __owner, __gid_t __group) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));




extern int lchown (__const char *__file, __uid_t __owner, __gid_t __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));






extern int fchownat (int __fd, __const char *__file, __uid_t __owner,
       __gid_t __group, int __flag)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2))) __attribute__ ((__warn_unused_result__));



extern int chdir (__const char *__path) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));



extern int fchdir (int __fd) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));
/* # 508 "/usr/include/unistd.h" 3 4 */
extern char *getcwd (char *__buf, size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));





extern char *get_current_dir_name (void) __attribute__ ((__nothrow__));







extern char *getwd (char *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__deprecated__)) __attribute__ ((__warn_unused_result__));




extern int dup (int __fd) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));


extern int dup2 (int __fd, int __fd2) __attribute__ ((__nothrow__));




extern int dup3 (int __fd, int __fd2, int __flags) __attribute__ ((__nothrow__));



extern char **__environ;

extern char **environ;





extern int execve (__const char *__path, char *__const __argv[],
     char *__const __envp[]) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));




extern int fexecve (int __fd, char *__const __argv[], char *__const __envp[])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));




extern int execv (__const char *__path, char *__const __argv[])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));



extern int execle (__const char *__path, __const char *__arg, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));



extern int execl (__const char *__path, __const char *__arg, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));



extern int execvp (__const char *__file, char *__const __argv[])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));




extern int execlp (__const char *__file, __const char *__arg, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));




extern int execvpe (__const char *__file, char *__const __argv[],
      char *__const __envp[])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));





extern int nice (int __inc) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));




extern void _exit (int __status) __attribute__ ((__noreturn__));





/* # 1 "/usr/include/bits/confname.h" 1 3 4 */
/* # 26 "/usr/include/bits/confname.h" 3 4 */
enum
  {
    _PC_LINK_MAX,

    _PC_MAX_CANON,

    _PC_MAX_INPUT,

    _PC_NAME_MAX,

    _PC_PATH_MAX,

    _PC_PIPE_BUF,

    _PC_CHOWN_RESTRICTED,

    _PC_NO_TRUNC,

    _PC_VDISABLE,

    _PC_SYNC_IO,

    _PC_ASYNC_IO,

    _PC_PRIO_IO,

    _PC_SOCK_MAXBUF,

    _PC_FILESIZEBITS,

    _PC_REC_INCR_XFER_SIZE,

    _PC_REC_MAX_XFER_SIZE,

    _PC_REC_MIN_XFER_SIZE,

    _PC_REC_XFER_ALIGN,

    _PC_ALLOC_SIZE_MIN,

    _PC_SYMLINK_MAX,

    _PC_2_SYMLINKS

  };


enum
  {
    _SC_ARG_MAX,

    _SC_CHILD_MAX,

    _SC_CLK_TCK,

    _SC_NGROUPS_MAX,

    _SC_OPEN_MAX,

    _SC_STREAM_MAX,

    _SC_TZNAME_MAX,

    _SC_JOB_CONTROL,

    _SC_SAVED_IDS,

    _SC_REALTIME_SIGNALS,

    _SC_PRIORITY_SCHEDULING,

    _SC_TIMERS,

    _SC_ASYNCHRONOUS_IO,

    _SC_PRIORITIZED_IO,

    _SC_SYNCHRONIZED_IO,

    _SC_FSYNC,

    _SC_MAPPED_FILES,

    _SC_MEMLOCK,

    _SC_MEMLOCK_RANGE,

    _SC_MEMORY_PROTECTION,

    _SC_MESSAGE_PASSING,

    _SC_SEMAPHORES,

    _SC_SHARED_MEMORY_OBJECTS,

    _SC_AIO_LISTIO_MAX,

    _SC_AIO_MAX,

    _SC_AIO_PRIO_DELTA_MAX,

    _SC_DELAYTIMER_MAX,

    _SC_MQ_OPEN_MAX,

    _SC_MQ_PRIO_MAX,

    _SC_VERSION,

    _SC_PAGESIZE,


    _SC_RTSIG_MAX,

    _SC_SEM_NSEMS_MAX,

    _SC_SEM_VALUE_MAX,

    _SC_SIGQUEUE_MAX,

    _SC_TIMER_MAX,




    _SC_BC_BASE_MAX,

    _SC_BC_DIM_MAX,

    _SC_BC_SCALE_MAX,

    _SC_BC_STRING_MAX,

    _SC_COLL_WEIGHTS_MAX,

    _SC_EQUIV_CLASS_MAX,

    _SC_EXPR_NEST_MAX,

    _SC_LINE_MAX,

    _SC_RE_DUP_MAX,

    _SC_CHARCLASS_NAME_MAX,


    _SC_2_VERSION,

    _SC_2_C_BIND,

    _SC_2_C_DEV,

    _SC_2_FORT_DEV,

    _SC_2_FORT_RUN,

    _SC_2_SW_DEV,

    _SC_2_LOCALEDEF,


    _SC_PII,

    _SC_PII_XTI,

    _SC_PII_SOCKET,

    _SC_PII_INTERNET,

    _SC_PII_OSI,

    _SC_POLL,

    _SC_SELECT,

    _SC_UIO_MAXIOV,

    _SC_IOV_MAX = _SC_UIO_MAXIOV,

    _SC_PII_INTERNET_STREAM,

    _SC_PII_INTERNET_DGRAM,

    _SC_PII_OSI_COTS,

    _SC_PII_OSI_CLTS,

    _SC_PII_OSI_M,

    _SC_T_IOV_MAX,



    _SC_THREADS,

    _SC_THREAD_SAFE_FUNCTIONS,

    _SC_GETGR_R_SIZE_MAX,

    _SC_GETPW_R_SIZE_MAX,

    _SC_LOGIN_NAME_MAX,

    _SC_TTY_NAME_MAX,

    _SC_THREAD_DESTRUCTOR_ITERATIONS,

    _SC_THREAD_KEYS_MAX,

    _SC_THREAD_STACK_MIN,

    _SC_THREAD_THREADS_MAX,

    _SC_THREAD_ATTR_STACKADDR,

    _SC_THREAD_ATTR_STACKSIZE,

    _SC_THREAD_PRIORITY_SCHEDULING,

    _SC_THREAD_PRIO_INHERIT,

    _SC_THREAD_PRIO_PROTECT,

    _SC_THREAD_PROCESS_SHARED,


    _SC_NPROCESSORS_CONF,

    _SC_NPROCESSORS_ONLN,

    _SC_PHYS_PAGES,

    _SC_AVPHYS_PAGES,

    _SC_ATEXIT_MAX,

    _SC_PASS_MAX,


    _SC_XOPEN_VERSION,

    _SC_XOPEN_XCU_VERSION,

    _SC_XOPEN_UNIX,

    _SC_XOPEN_CRYPT,

    _SC_XOPEN_ENH_I18N,

    _SC_XOPEN_SHM,


    _SC_2_CHAR_TERM,

    _SC_2_C_VERSION,

    _SC_2_UPE,


    _SC_XOPEN_XPG2,

    _SC_XOPEN_XPG3,

    _SC_XOPEN_XPG4,


    _SC_CHAR_BIT,

    _SC_CHAR_MAX,

    _SC_CHAR_MIN,

    _SC_INT_MAX,

    _SC_INT_MIN,

    _SC_LONG_BIT,

    _SC_WORD_BIT,

    _SC_MB_LEN_MAX,

    _SC_NZERO,

    _SC_SSIZE_MAX,

    _SC_SCHAR_MAX,

    _SC_SCHAR_MIN,

    _SC_SHRT_MAX,

    _SC_SHRT_MIN,

    _SC_UCHAR_MAX,

    _SC_UINT_MAX,

    _SC_ULONG_MAX,

    _SC_USHRT_MAX,


    _SC_NL_ARGMAX,

    _SC_NL_LANGMAX,

    _SC_NL_MSGMAX,

    _SC_NL_NMAX,

    _SC_NL_SETMAX,

    _SC_NL_TEXTMAX,


    _SC_XBS5_ILP32_OFF32,

    _SC_XBS5_ILP32_OFFBIG,

    _SC_XBS5_LP64_OFF64,

    _SC_XBS5_LPBIG_OFFBIG,


    _SC_XOPEN_LEGACY,

    _SC_XOPEN_REALTIME,

    _SC_XOPEN_REALTIME_THREADS,


    _SC_ADVISORY_INFO,

    _SC_BARRIERS,

    _SC_BASE,

    _SC_C_LANG_SUPPORT,

    _SC_C_LANG_SUPPORT_R,

    _SC_CLOCK_SELECTION,

    _SC_CPUTIME,

    _SC_THREAD_CPUTIME,

    _SC_DEVICE_IO,

    _SC_DEVICE_SPECIFIC,

    _SC_DEVICE_SPECIFIC_R,

    _SC_FD_MGMT,

    _SC_FIFO,

    _SC_PIPE,

    _SC_FILE_ATTRIBUTES,

    _SC_FILE_LOCKING,

    _SC_FILE_SYSTEM,

    _SC_MONOTONIC_CLOCK,

    _SC_MULTI_PROCESS,

    _SC_SINGLE_PROCESS,

    _SC_NETWORKING,

    _SC_READER_WRITER_LOCKS,

    _SC_SPIN_LOCKS,

    _SC_REGEXP,

    _SC_REGEX_VERSION,

    _SC_SHELL,

    _SC_SIGNALS,

    _SC_SPAWN,

    _SC_SPORADIC_SERVER,

    _SC_THREAD_SPORADIC_SERVER,

    _SC_SYSTEM_DATABASE,

    _SC_SYSTEM_DATABASE_R,

    _SC_TIMEOUTS,

    _SC_TYPED_MEMORY_OBJECTS,

    _SC_USER_GROUPS,

    _SC_USER_GROUPS_R,

    _SC_2_PBS,

    _SC_2_PBS_ACCOUNTING,

    _SC_2_PBS_LOCATE,

    _SC_2_PBS_MESSAGE,

    _SC_2_PBS_TRACK,

    _SC_SYMLOOP_MAX,

    _SC_STREAMS,

    _SC_2_PBS_CHECKPOINT,


    _SC_V6_ILP32_OFF32,

    _SC_V6_ILP32_OFFBIG,

    _SC_V6_LP64_OFF64,

    _SC_V6_LPBIG_OFFBIG,


    _SC_HOST_NAME_MAX,

    _SC_TRACE,

    _SC_TRACE_EVENT_FILTER,

    _SC_TRACE_INHERIT,

    _SC_TRACE_LOG,


    _SC_LEVEL1_ICACHE_SIZE,

    _SC_LEVEL1_ICACHE_ASSOC,

    _SC_LEVEL1_ICACHE_LINESIZE,

    _SC_LEVEL1_DCACHE_SIZE,

    _SC_LEVEL1_DCACHE_ASSOC,

    _SC_LEVEL1_DCACHE_LINESIZE,

    _SC_LEVEL2_CACHE_SIZE,

    _SC_LEVEL2_CACHE_ASSOC,

    _SC_LEVEL2_CACHE_LINESIZE,

    _SC_LEVEL3_CACHE_SIZE,

    _SC_LEVEL3_CACHE_ASSOC,

    _SC_LEVEL3_CACHE_LINESIZE,

    _SC_LEVEL4_CACHE_SIZE,

    _SC_LEVEL4_CACHE_ASSOC,

    _SC_LEVEL4_CACHE_LINESIZE,



    _SC_IPV6 = _SC_LEVEL1_ICACHE_SIZE + 50,

    _SC_RAW_SOCKETS,


    _SC_V7_ILP32_OFF32,

    _SC_V7_ILP32_OFFBIG,

    _SC_V7_LP64_OFF64,

    _SC_V7_LPBIG_OFFBIG,


    _SC_SS_REPL_MAX,


    _SC_TRACE_EVENT_NAME_MAX,

    _SC_TRACE_NAME_MAX,

    _SC_TRACE_SYS_MAX,

    _SC_TRACE_USER_EVENT_MAX,


    _SC_XOPEN_STREAMS,


    _SC_THREAD_ROBUST_PRIO_INHERIT,

    _SC_THREAD_ROBUST_PRIO_PROTECT

  };


enum
  {
    _CS_PATH,


    _CS_V6_WIDTH_RESTRICTED_ENVS,



    _CS_GNU_LIBC_VERSION,

    _CS_GNU_LIBPTHREAD_VERSION,


    _CS_V5_WIDTH_RESTRICTED_ENVS,



    _CS_V7_WIDTH_RESTRICTED_ENVS,



    _CS_LFS_CFLAGS = 1000,

    _CS_LFS_LDFLAGS,

    _CS_LFS_LIBS,

    _CS_LFS_LINTFLAGS,

    _CS_LFS64_CFLAGS,

    _CS_LFS64_LDFLAGS,

    _CS_LFS64_LIBS,

    _CS_LFS64_LINTFLAGS,


    _CS_XBS5_ILP32_OFF32_CFLAGS = 1100,

    _CS_XBS5_ILP32_OFF32_LDFLAGS,

    _CS_XBS5_ILP32_OFF32_LIBS,

    _CS_XBS5_ILP32_OFF32_LINTFLAGS,

    _CS_XBS5_ILP32_OFFBIG_CFLAGS,

    _CS_XBS5_ILP32_OFFBIG_LDFLAGS,

    _CS_XBS5_ILP32_OFFBIG_LIBS,

    _CS_XBS5_ILP32_OFFBIG_LINTFLAGS,

    _CS_XBS5_LP64_OFF64_CFLAGS,

    _CS_XBS5_LP64_OFF64_LDFLAGS,

    _CS_XBS5_LP64_OFF64_LIBS,

    _CS_XBS5_LP64_OFF64_LINTFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_CFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_LDFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_LIBS,

    _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS,


    _CS_POSIX_V6_ILP32_OFF32_CFLAGS,

    _CS_POSIX_V6_ILP32_OFF32_LDFLAGS,

    _CS_POSIX_V6_ILP32_OFF32_LIBS,

    _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_LIBS,

    _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS,

    _CS_POSIX_V6_LP64_OFF64_CFLAGS,

    _CS_POSIX_V6_LP64_OFF64_LDFLAGS,

    _CS_POSIX_V6_LP64_OFF64_LIBS,

    _CS_POSIX_V6_LP64_OFF64_LINTFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LIBS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS,


    _CS_POSIX_V7_ILP32_OFF32_CFLAGS,

    _CS_POSIX_V7_ILP32_OFF32_LDFLAGS,

    _CS_POSIX_V7_ILP32_OFF32_LIBS,

    _CS_POSIX_V7_ILP32_OFF32_LINTFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_LIBS,

    _CS_POSIX_V7_ILP32_OFFBIG_LINTFLAGS,

    _CS_POSIX_V7_LP64_OFF64_CFLAGS,

    _CS_POSIX_V7_LP64_OFF64_LDFLAGS,

    _CS_POSIX_V7_LP64_OFF64_LIBS,

    _CS_POSIX_V7_LP64_OFF64_LINTFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LIBS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LINTFLAGS,


    _CS_V6_ENV,

    _CS_V7_ENV

  };
/* # 607 "/usr/include/unistd.h" 2 3 4 */


extern long int pathconf (__const char *__path, int __name)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern long int fpathconf (int __fd, int __name) __attribute__ ((__nothrow__));


extern long int sysconf (int __name) __attribute__ ((__nothrow__));



extern size_t confstr (int __name, char *__buf, size_t __len) __attribute__ ((__nothrow__));




extern __pid_t getpid (void) __attribute__ ((__nothrow__));


extern __pid_t getppid (void) __attribute__ ((__nothrow__));




extern __pid_t getpgrp (void) __attribute__ ((__nothrow__));
/* # 643 "/usr/include/unistd.h" 3 4 */
extern __pid_t __getpgid (__pid_t __pid) __attribute__ ((__nothrow__));

extern __pid_t getpgid (__pid_t __pid) __attribute__ ((__nothrow__));






extern int setpgid (__pid_t __pid, __pid_t __pgid) __attribute__ ((__nothrow__));
/* # 669 "/usr/include/unistd.h" 3 4 */
extern int setpgrp (void) __attribute__ ((__nothrow__));
/* # 686 "/usr/include/unistd.h" 3 4 */
extern __pid_t setsid (void) __attribute__ ((__nothrow__));



extern __pid_t getsid (__pid_t __pid) __attribute__ ((__nothrow__));



extern __uid_t getuid (void) __attribute__ ((__nothrow__));


extern __uid_t geteuid (void) __attribute__ ((__nothrow__));


extern __gid_t getgid (void) __attribute__ ((__nothrow__));


extern __gid_t getegid (void) __attribute__ ((__nothrow__));




extern int getgroups (int __size, __gid_t __list[]) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));



extern int group_member (__gid_t __gid) __attribute__ ((__nothrow__));






extern int setuid (__uid_t __uid) __attribute__ ((__nothrow__));




extern int setreuid (__uid_t __ruid, __uid_t __euid) __attribute__ ((__nothrow__));




extern int seteuid (__uid_t __uid) __attribute__ ((__nothrow__));






extern int setgid (__gid_t __gid) __attribute__ ((__nothrow__));




extern int setregid (__gid_t __rgid, __gid_t __egid) __attribute__ ((__nothrow__));




extern int setegid (__gid_t __gid) __attribute__ ((__nothrow__));





extern int getresuid (__uid_t *__ruid, __uid_t *__euid, __uid_t *__suid)
     __attribute__ ((__nothrow__));



extern int getresgid (__gid_t *__rgid, __gid_t *__egid, __gid_t *__sgid)
     __attribute__ ((__nothrow__));



extern int setresuid (__uid_t __ruid, __uid_t __euid, __uid_t __suid)
     __attribute__ ((__nothrow__));



extern int setresgid (__gid_t __rgid, __gid_t __egid, __gid_t __sgid)
     __attribute__ ((__nothrow__));






extern __pid_t fork (void) __attribute__ ((__nothrow__));







extern __pid_t vfork (void) __attribute__ ((__nothrow__));





extern char *ttyname (int __fd) __attribute__ ((__nothrow__));



extern int ttyname_r (int __fd, char *__buf, size_t __buflen)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2))) __attribute__ ((__warn_unused_result__));



extern int isatty (int __fd) __attribute__ ((__nothrow__));





extern int ttyslot (void) __attribute__ ((__nothrow__));




extern int link (__const char *__from, __const char *__to)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__warn_unused_result__));




extern int linkat (int __fromfd, __const char *__from, int __tofd,
     __const char *__to, int __flags)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 4))) __attribute__ ((__warn_unused_result__));




extern int symlink (__const char *__from, __const char *__to)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__warn_unused_result__));




extern ssize_t readlink (__const char *__restrict __path,
    char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__warn_unused_result__));




extern int symlinkat (__const char *__from, int __tofd,
        __const char *__to) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 3))) __attribute__ ((__warn_unused_result__));


extern ssize_t readlinkat (int __fd, __const char *__restrict __path,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 3))) __attribute__ ((__warn_unused_result__));



extern int unlink (__const char *__name) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int unlinkat (int __fd, __const char *__name, int __flag)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));



extern int rmdir (__const char *__path) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern __pid_t tcgetpgrp (int __fd) __attribute__ ((__nothrow__));


extern int tcsetpgrp (int __fd, __pid_t __pgrp_id) __attribute__ ((__nothrow__));






extern char *getlogin (void);







extern int getlogin_r (char *__name, size_t __name_len) __attribute__ ((__nonnull__ (1)));




extern int setlogin (__const char *__name) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
/* # 890 "/usr/include/unistd.h" 3 4 */
/* # 1 "/usr/include/getopt.h" 1 3 4 */
/* # 59 "/usr/include/getopt.h" 3 4 */
extern char *optarg;
/* # 73 "/usr/include/getopt.h" 3 4 */
extern int optind;




extern int opterr;



extern int optopt;
/* # 152 "/usr/include/getopt.h" 3 4 */
extern int getopt (int ___argc, char *const *___argv, const char *__shortopts)
       __attribute__ ((__nothrow__));
/* # 891 "/usr/include/unistd.h" 2 3 4 */







extern int gethostname (char *__name, size_t __len) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern int sethostname (__const char *__name, size_t __len)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));



extern int sethostid (long int __id) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));





extern int getdomainname (char *__name, size_t __len)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
extern int setdomainname (__const char *__name, size_t __len)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern int vhangup (void) __attribute__ ((__nothrow__));


extern int revoke (__const char *__file) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));







extern int profil (unsigned short int *__sample_buffer, size_t __size,
     size_t __offset, unsigned int __scale)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int acct (__const char *__name) __attribute__ ((__nothrow__));



extern char *getusershell (void) __attribute__ ((__nothrow__));
extern void endusershell (void) __attribute__ ((__nothrow__));
extern void setusershell (void) __attribute__ ((__nothrow__));





extern int daemon (int __nochdir, int __noclose) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));






extern int chroot (__const char *__path) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));



extern char *getpass (__const char *__prompt) __attribute__ ((__nonnull__ (1)));
/* # 976 "/usr/include/unistd.h" 3 4 */
extern int fsync (int __fd);






extern long int gethostid (void);


extern void sync (void) __attribute__ ((__nothrow__));





extern int getpagesize (void) __attribute__ ((__nothrow__)) __attribute__ ((__const__));




extern int getdtablesize (void) __attribute__ ((__nothrow__));
/* # 1011 "/usr/include/unistd.h" 3 4 */
extern int truncate (__const char *__file, __off64_t __length) __asm__ ("" "truncate64") __attribute__ ((__nothrow__))

                  __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));





extern int truncate64 (__const char *__file, __off64_t __length)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
/* # 1032 "/usr/include/unistd.h" 3 4 */
extern int ftruncate (int __fd, __off64_t __length) __asm__ ("" "ftruncate64") __attribute__ ((__nothrow__))
                   __attribute__ ((__warn_unused_result__));





extern int ftruncate64 (int __fd, __off64_t __length) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));
/* # 1050 "/usr/include/unistd.h" 3 4 */
extern int brk (void *__addr) __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));





extern void *sbrk (intptr_t __delta) __attribute__ ((__nothrow__));
/* # 1071 "/usr/include/unistd.h" 3 4 */
extern long int syscall (long int __sysno, ...) __attribute__ ((__nothrow__));
/* # 1097 "/usr/include/unistd.h" 3 4 */
extern int lockf (int __fd, int __cmd, __off64_t __len) __asm__ ("" "lockf64")
                  __attribute__ ((__warn_unused_result__));





extern int lockf64 (int __fd, int __cmd, __off64_t __len) __attribute__ ((__warn_unused_result__));
/* # 1125 "/usr/include/unistd.h" 3 4 */
extern int fdatasync (int __fildes);







extern char *crypt (__const char *__key, __const char *__salt)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));



extern void encrypt (char *__block, int __edflag) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern void swab (__const void *__restrict __from, void *__restrict __to,
    ssize_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));







extern char *ctermid (char *__s) __attribute__ ((__nothrow__));





/* # 1 "/usr/include/bits/unistd.h" 1 3 4 */
/* # 24 "/usr/include/bits/unistd.h" 3 4 */
extern ssize_t __read_chk (int __fd, void *__buf, size_t __nbytes,
      size_t __buflen) __attribute__ ((__warn_unused_result__));
extern ssize_t __read_alias (int __fd, void *__buf, size_t __nbytes) __asm__ ("" "read")
                               __attribute__ ((__warn_unused_result__));
extern ssize_t __read_chk_warn (int __fd, void *__buf, size_t __nbytes, size_t __buflen) __asm__ ("" "__read_chk")


     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("read called with bigger length than size of " "the destination buffer")))
                                  ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) ssize_t
read (int __fd, void *__buf, size_t __nbytes)
{
  if (__builtin_object_size (__buf, 0) != (size_t) -1)
    {
      if (!__builtin_constant_p (__nbytes))
 return __read_chk (__fd, __buf, __nbytes, __builtin_object_size (__buf, 0));

      if (__nbytes > __builtin_object_size (__buf, 0))
 return __read_chk_warn (__fd, __buf, __nbytes, __builtin_object_size (__buf, 0));
    }
  return __read_alias (__fd, __buf, __nbytes);
}


extern ssize_t __pread_chk (int __fd, void *__buf, size_t __nbytes,
       __off_t __offset, size_t __bufsize) __attribute__ ((__warn_unused_result__));
extern ssize_t __pread64_chk (int __fd, void *__buf, size_t __nbytes,
         __off64_t __offset, size_t __bufsize) __attribute__ ((__warn_unused_result__));
extern ssize_t __pread_alias (int __fd, void *__buf, size_t __nbytes, __off_t __offset) __asm__ ("" "pread")

                                 __attribute__ ((__warn_unused_result__));
extern ssize_t __pread64_alias (int __fd, void *__buf, size_t __nbytes, __off64_t __offset) __asm__ ("" "pread64")

                                     __attribute__ ((__warn_unused_result__));
extern ssize_t __pread_chk_warn (int __fd, void *__buf, size_t __nbytes, __off_t __offset, size_t __bufsize) __asm__ ("" "__pread_chk")


     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("pread called with bigger length than size of " "the destination buffer")))
                                  ;
extern ssize_t __pread64_chk_warn (int __fd, void *__buf, size_t __nbytes, __off64_t __offset, size_t __bufsize) __asm__ ("" "__pread64_chk")



     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("pread64 called with bigger length than size of " "the destination buffer")))
                                  ;
/* # 87 "/usr/include/bits/unistd.h" 3 4 */
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) ssize_t
pread (int __fd, void *__buf, size_t __nbytes, __off64_t __offset)
{
  if (__builtin_object_size (__buf, 0) != (size_t) -1)
    {
      if (!__builtin_constant_p (__nbytes))
 return __pread64_chk (__fd, __buf, __nbytes, __offset, __builtin_object_size (__buf, 0));

      if ( __nbytes > __builtin_object_size (__buf, 0))
 return __pread64_chk_warn (__fd, __buf, __nbytes, __offset,
       __builtin_object_size (__buf, 0));
    }

  return __pread64_alias (__fd, __buf, __nbytes, __offset);
}



extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) ssize_t
pread64 (int __fd, void *__buf, size_t __nbytes, __off64_t __offset)
{
  if (__builtin_object_size (__buf, 0) != (size_t) -1)
    {
      if (!__builtin_constant_p (__nbytes))
 return __pread64_chk (__fd, __buf, __nbytes, __offset, __builtin_object_size (__buf, 0));

      if ( __nbytes > __builtin_object_size (__buf, 0))
 return __pread64_chk_warn (__fd, __buf, __nbytes, __offset,
       __builtin_object_size (__buf, 0));
    }

  return __pread64_alias (__fd, __buf, __nbytes, __offset);
}




extern ssize_t __readlink_chk (__const char *__restrict __path,
          char *__restrict __buf, size_t __len,
          size_t __buflen)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__warn_unused_result__));
extern ssize_t __readlink_alias (__const char *__restrict __path, char *__restrict __buf, size_t __len) __asm__ ("" "readlink") __attribute__ ((__nothrow__))


     __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__warn_unused_result__));
extern ssize_t __readlink_chk_warn (__const char *__restrict __path, char *__restrict __buf, size_t __len, size_t __buflen) __asm__ ("" "__readlink_chk") __attribute__ ((__nothrow__))



     __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("readlink called with bigger length " "than size of destination buffer")))
                                         ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__warn_unused_result__)) ssize_t
__attribute__ ((__nothrow__)) readlink (__const char *__restrict __path, char *__restrict __buf, size_t __len)

{
  if (__builtin_object_size (__buf, 2 > 1) != (size_t) -1)
    {
      if (!__builtin_constant_p (__len))
 return __readlink_chk (__path, __buf, __len, __builtin_object_size (__buf, 2 > 1));

      if ( __len > __builtin_object_size (__buf, 2 > 1))
 return __readlink_chk_warn (__path, __buf, __len, __builtin_object_size (__buf, 2 > 1));
    }
  return __readlink_alias (__path, __buf, __len);
}



extern ssize_t __readlinkat_chk (int __fd, __const char *__restrict __path,
     char *__restrict __buf, size_t __len,
     size_t __buflen)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 3))) __attribute__ ((__warn_unused_result__));
extern ssize_t __readlinkat_alias (int __fd, __const char *__restrict __path, char *__restrict __buf, size_t __len) __asm__ ("" "readlinkat") __attribute__ ((__nothrow__))



     __attribute__ ((__nonnull__ (2, 3))) __attribute__ ((__warn_unused_result__));
extern ssize_t __readlinkat_chk_warn (int __fd, __const char *__restrict __path, char *__restrict __buf, size_t __len, size_t __buflen) __asm__ ("" "__readlinkat_chk") __attribute__ ((__nothrow__))



     __attribute__ ((__nonnull__ (2, 3))) __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("readlinkat called with bigger " "length than size of destination " "buffer")))

                ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__nonnull__ (2, 3))) __attribute__ ((__warn_unused_result__)) ssize_t
__attribute__ ((__nothrow__)) readlinkat (int __fd, __const char *__restrict __path, char *__restrict __buf, size_t __len)

{
  if (__builtin_object_size (__buf, 2 > 1) != (size_t) -1)
    {
      if (!__builtin_constant_p (__len))
 return __readlinkat_chk (__fd, __path, __buf, __len, __builtin_object_size (__buf, 2 > 1));

      if (__len > __builtin_object_size (__buf, 2 > 1))
 return __readlinkat_chk_warn (__fd, __path, __buf, __len,
          __builtin_object_size (__buf, 2 > 1));
    }
  return __readlinkat_alias (__fd, __path, __buf, __len);
}


extern char *__getcwd_chk (char *__buf, size_t __size, size_t __buflen)
     __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));
extern char *__getcwd_alias (char *__buf, size_t __size) __asm__ ("" "getcwd") __attribute__ ((__nothrow__))
                                              __attribute__ ((__warn_unused_result__));
extern char *__getcwd_chk_warn (char *__buf, size_t __size, size_t __buflen) __asm__ ("" "__getcwd_chk") __attribute__ ((__nothrow__))


     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("getcwd caller with bigger length than size of " "destination buffer")))
                              ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) char *
__attribute__ ((__nothrow__)) getcwd (char *__buf, size_t __size)
{
  if (__builtin_object_size (__buf, 2 > 1) != (size_t) -1)
    {
      if (!__builtin_constant_p (__size))
 return __getcwd_chk (__buf, __size, __builtin_object_size (__buf, 2 > 1));

      if (__size > __builtin_object_size (__buf, 2 > 1))
 return __getcwd_chk_warn (__buf, __size, __builtin_object_size (__buf, 2 > 1));
    }
  return __getcwd_alias (__buf, __size);
}


extern char *__getwd_chk (char *__buf, size_t buflen)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
extern char *__getwd_warn (char *__buf) __asm__ ("" "getwd") __attribute__ ((__nothrow__))
     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("please use getcwd instead, as getwd " "doesn't specify buffer size")))
                                         ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__deprecated__)) __attribute__ ((__warn_unused_result__)) char *
__attribute__ ((__nothrow__)) getwd (char *__buf)
{
  if (__builtin_object_size (__buf, 2 > 1) != (size_t) -1)
    return __getwd_chk (__buf, __builtin_object_size (__buf, 2 > 1));
  return __getwd_warn (__buf);
}


extern size_t __confstr_chk (int __name, char *__buf, size_t __len,
        size_t __buflen) __attribute__ ((__nothrow__));
extern size_t __confstr_alias (int __name, char *__buf, size_t __len) __asm__ ("" "confstr") __attribute__ ((__nothrow__))
                             ;
extern size_t __confstr_chk_warn (int __name, char *__buf, size_t __len, size_t __buflen) __asm__ ("" "__confstr_chk") __attribute__ ((__nothrow__))


     __attribute__((__warning__ ("confstr called with bigger length than size of destination " "buffer")))
            ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) size_t
__attribute__ ((__nothrow__)) confstr (int __name, char *__buf, size_t __len)
{
  if (__builtin_object_size (__buf, 2 > 1) != (size_t) -1)
    {
      if (!__builtin_constant_p (__len))
 return __confstr_chk (__name, __buf, __len, __builtin_object_size (__buf, 2 > 1));

      if (__builtin_object_size (__buf, 2 > 1) < __len)
 return __confstr_chk_warn (__name, __buf, __len, __builtin_object_size (__buf, 2 > 1));
    }
  return __confstr_alias (__name, __buf, __len);
}


extern int __getgroups_chk (int __size, __gid_t __list[], size_t __listlen)
     __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));
extern int __getgroups_alias (int __size, __gid_t __list[]) __asm__ ("" "getgroups") __attribute__ ((__nothrow__))
                 __attribute__ ((__warn_unused_result__));
extern int __getgroups_chk_warn (int __size, __gid_t __list[], size_t __listlen) __asm__ ("" "__getgroups_chk") __attribute__ ((__nothrow__))


     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("getgroups called with bigger group count than what " "can fit into destination buffer")))
                                           ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__)) getgroups (int __size, __gid_t __list[])
{
  if (__builtin_object_size (__list, 2 > 1) != (size_t) -1)
    {
      if (!__builtin_constant_p (__size) || __size < 0)
 return __getgroups_chk (__size, __list, __builtin_object_size (__list, 2 > 1));

      if (__size * sizeof (__gid_t) > __builtin_object_size (__list, 2 > 1))
 return __getgroups_chk_warn (__size, __list, __builtin_object_size (__list, 2 > 1));
    }
  return __getgroups_alias (__size, __list);
}


extern int __ttyname_r_chk (int __fd, char *__buf, size_t __buflen,
       size_t __nreal) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));
extern int __ttyname_r_alias (int __fd, char *__buf, size_t __buflen) __asm__ ("" "ttyname_r") __attribute__ ((__nothrow__))

     __attribute__ ((__nonnull__ (2)));
extern int __ttyname_r_chk_warn (int __fd, char *__buf, size_t __buflen, size_t __nreal) __asm__ ("" "__ttyname_r_chk") __attribute__ ((__nothrow__))


     __attribute__ ((__nonnull__ (2))) __attribute__((__warning__ ("ttyname_r called with bigger buflen than " "size of destination buffer")))
                                  ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__)) ttyname_r (int __fd, char *__buf, size_t __buflen)
{
  if (__builtin_object_size (__buf, 2 > 1) != (size_t) -1)
    {
      if (!__builtin_constant_p (__buflen))
 return __ttyname_r_chk (__fd, __buf, __buflen, __builtin_object_size (__buf, 2 > 1));

      if (__buflen > __builtin_object_size (__buf, 2 > 1))
 return __ttyname_r_chk_warn (__fd, __buf, __buflen, __builtin_object_size (__buf, 2 > 1));
    }
  return __ttyname_r_alias (__fd, __buf, __buflen);
}



extern int __getlogin_r_chk (char *__buf, size_t __buflen, size_t __nreal)
     __attribute__ ((__nonnull__ (1)));
extern int __getlogin_r_alias (char *__buf, size_t __buflen) __asm__ ("" "getlogin_r")
                     __attribute__ ((__nonnull__ (1)));
extern int __getlogin_r_chk_warn (char *__buf, size_t __buflen, size_t __nreal) __asm__ ("" "__getlogin_r_chk")


     __attribute__ ((__nonnull__ (1))) __attribute__((__warning__ ("getlogin_r called with bigger buflen than " "size of destination buffer")))
                                  ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
getlogin_r (char *__buf, size_t __buflen)
{
  if (__builtin_object_size (__buf, 2 > 1) != (size_t) -1)
    {
      if (!__builtin_constant_p (__buflen))
 return __getlogin_r_chk (__buf, __buflen, __builtin_object_size (__buf, 2 > 1));

      if (__buflen > __builtin_object_size (__buf, 2 > 1))
 return __getlogin_r_chk_warn (__buf, __buflen, __builtin_object_size (__buf, 2 > 1));
    }
  return __getlogin_r_alias (__buf, __buflen);
}




extern int __gethostname_chk (char *__buf, size_t __buflen, size_t __nreal)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
extern int __gethostname_alias (char *__buf, size_t __buflen) __asm__ ("" "gethostname") __attribute__ ((__nothrow__))
                   __attribute__ ((__nonnull__ (1)));
extern int __gethostname_chk_warn (char *__buf, size_t __buflen, size_t __nreal) __asm__ ("" "__gethostname_chk") __attribute__ ((__nothrow__))


     __attribute__ ((__nonnull__ (1))) __attribute__((__warning__ ("gethostname called with bigger buflen than " "size of destination buffer")))
                                  ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__)) gethostname (char *__buf, size_t __buflen)
{
  if (__builtin_object_size (__buf, 2 > 1) != (size_t) -1)
    {
      if (!__builtin_constant_p (__buflen))
 return __gethostname_chk (__buf, __buflen, __builtin_object_size (__buf, 2 > 1));

      if (__buflen > __builtin_object_size (__buf, 2 > 1))
 return __gethostname_chk_warn (__buf, __buflen, __builtin_object_size (__buf, 2 > 1));
    }
  return __gethostname_alias (__buf, __buflen);
}




extern int __getdomainname_chk (char *__buf, size_t __buflen, size_t __nreal)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
extern int __getdomainname_alias (char *__buf, size_t __buflen) __asm__ ("" "getdomainname") __attribute__ ((__nothrow__))

                     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
extern int __getdomainname_chk_warn (char *__buf, size_t __buflen, size_t __nreal) __asm__ ("" "__getdomainname_chk") __attribute__ ((__nothrow__))


     __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("getdomainname called with bigger " "buflen than size of destination " "buffer")))

                    ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__)) getdomainname (char *__buf, size_t __buflen)
{
  if (__builtin_object_size (__buf, 2 > 1) != (size_t) -1)
    {
      if (!__builtin_constant_p (__buflen))
 return __getdomainname_chk (__buf, __buflen, __builtin_object_size (__buf, 2 > 1));

      if (__buflen > __builtin_object_size (__buf, 2 > 1))
 return __getdomainname_chk_warn (__buf, __buflen, __builtin_object_size (__buf, 2 > 1));
    }
  return __getdomainname_alias (__buf, __buflen);
}
/* # 1161 "/usr/include/unistd.h" 2 3 4 */



/* # 46 "../include/lib.h" 2 */
/* # 1 "/usr/include/sys/stat.h" 1 3 4 */
/* # 39 "/usr/include/sys/stat.h" 3 4 */
/* # 1 "/usr/include/time.h" 1 3 4 */
/* # 40 "/usr/include/sys/stat.h" 2 3 4 */
/* # 105 "/usr/include/sys/stat.h" 3 4 */


/* # 1 "/usr/include/bits/stat.h" 1 3 4 */
/* # 46 "/usr/include/bits/stat.h" 3 4 */
struct stat
  {
    __dev_t st_dev;

    unsigned short int __pad1;




    __ino_t __st_ino;


    __mode_t st_mode;
    __nlink_t st_nlink;




    __uid_t st_uid;
    __gid_t st_gid;



    __dev_t st_rdev;

    unsigned short int __pad2;




    __off64_t st_size;

    __blksize_t st_blksize;



    __blkcnt64_t st_blocks;
/* # 91 "/usr/include/bits/stat.h" 3 4 */
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
/* # 112 "/usr/include/bits/stat.h" 3 4 */
    __ino64_t st_ino;


  };



struct stat64
  {
    __dev_t st_dev;





    unsigned int __pad1;
    __ino_t __st_ino;
    __mode_t st_mode;
    __nlink_t st_nlink;

    __uid_t st_uid;
    __gid_t st_gid;





    __dev_t st_rdev;
    unsigned int __pad2;
    __off64_t st_size;

    __blksize_t st_blksize;
    __blkcnt64_t st_blocks;







    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
/* # 169 "/usr/include/bits/stat.h" 3 4 */
    __ino64_t st_ino;

  };
/* # 108 "/usr/include/sys/stat.h" 2 3 4 */
/* # 219 "/usr/include/sys/stat.h" 3 4 */
extern int stat (__const char *__restrict __file, struct stat *__restrict __buf) __asm__ ("" "stat64") __attribute__ ((__nothrow__))

     __attribute__ ((__nonnull__ (1, 2)));
extern int fstat (int __fd, struct stat *__buf) __asm__ ("" "fstat64") __attribute__ ((__nothrow__))
     __attribute__ ((__nonnull__ (2)));






extern int stat64 (__const char *__restrict __file,
     struct stat64 *__restrict __buf) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int fstat64 (int __fd, struct stat64 *__buf) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));
/* # 245 "/usr/include/sys/stat.h" 3 4 */
extern int fstatat (int __fd, __const char *__restrict __file, struct stat *__restrict __buf, int __flag) __asm__ ("" "fstatat64") __attribute__ ((__nothrow__))


                 __attribute__ ((__nonnull__ (2, 3)));






extern int fstatat64 (int __fd, __const char *__restrict __file,
        struct stat64 *__restrict __buf, int __flag)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 3)));
/* # 269 "/usr/include/sys/stat.h" 3 4 */
extern int lstat (__const char *__restrict __file, struct stat *__restrict __buf) __asm__ ("" "lstat64") __attribute__ ((__nothrow__))


     __attribute__ ((__nonnull__ (1, 2)));





extern int lstat64 (__const char *__restrict __file,
      struct stat64 *__restrict __buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));





extern int chmod (__const char *__file, __mode_t __mode)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int lchmod (__const char *__file, __mode_t __mode)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));




extern int fchmod (int __fd, __mode_t __mode) __attribute__ ((__nothrow__));





extern int fchmodat (int __fd, __const char *__file, __mode_t __mode,
       int __flag)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2))) __attribute__ ((__warn_unused_result__));






extern __mode_t umask (__mode_t __mask) __attribute__ ((__nothrow__));




extern __mode_t getumask (void) __attribute__ ((__nothrow__));



extern int mkdir (__const char *__path, __mode_t __mode)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int mkdirat (int __fd, __const char *__path, __mode_t __mode)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));






extern int mknod (__const char *__path, __mode_t __mode, __dev_t __dev)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int mknodat (int __fd, __const char *__path, __mode_t __mode,
      __dev_t __dev) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));





extern int mkfifo (__const char *__path, __mode_t __mode)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int mkfifoat (int __fd, __const char *__path, __mode_t __mode)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));





extern int utimensat (int __fd, __const char *__path,
        __const struct timespec __times[2],
        int __flags)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));




extern int futimens (int __fd, __const struct timespec __times[2]) __attribute__ ((__nothrow__));
/* # 412 "/usr/include/sys/stat.h" 3 4 */
extern int __fxstat (int __ver, int __fildes, struct stat *__stat_buf) __asm__ ("" "__fxstat64") __attribute__ ((__nothrow__))

     __attribute__ ((__nonnull__ (3)));
extern int __xstat (int __ver, __const char *__filename, struct stat *__stat_buf) __asm__ ("" "__xstat64") __attribute__ ((__nothrow__))

     __attribute__ ((__nonnull__ (2, 3)));
extern int __lxstat (int __ver, __const char *__filename, struct stat *__stat_buf) __asm__ ("" "__lxstat64") __attribute__ ((__nothrow__))

     __attribute__ ((__nonnull__ (2, 3)));
extern int __fxstatat (int __ver, int __fildes, __const char *__filename, struct stat *__stat_buf, int __flag) __asm__ ("" "__fxstatat64") __attribute__ ((__nothrow__))


                    __attribute__ ((__nonnull__ (3, 4)));
/* # 434 "/usr/include/sys/stat.h" 3 4 */
extern int __fxstat64 (int __ver, int __fildes, struct stat64 *__stat_buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3)));
extern int __xstat64 (int __ver, __const char *__filename,
        struct stat64 *__stat_buf) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 3)));
extern int __lxstat64 (int __ver, __const char *__filename,
         struct stat64 *__stat_buf) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 3)));
extern int __fxstatat64 (int __ver, int __fildes, __const char *__filename,
    struct stat64 *__stat_buf, int __flag)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4)));

extern int __xmknod (int __ver, __const char *__path, __mode_t __mode,
       __dev_t *__dev) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 4)));

extern int __xmknodat (int __ver, int __fd, __const char *__path,
         __mode_t __mode, __dev_t *__dev)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 5)));




extern __inline int
__attribute__ ((__nothrow__)) stat (__const char *__path, struct stat *__statbuf)
{
  return __xstat (3, __path, __statbuf);
}


extern __inline int
__attribute__ ((__nothrow__)) lstat (__const char *__path, struct stat *__statbuf)
{
  return __lxstat (3, __path, __statbuf);
}


extern __inline int
__attribute__ ((__nothrow__)) fstat (int __fd, struct stat *__statbuf)
{
  return __fxstat (3, __fd, __statbuf);
}


extern __inline int
__attribute__ ((__nothrow__)) fstatat (int __fd, __const char *__filename, struct stat *__statbuf, int __flag)

{
  return __fxstatat (3, __fd, __filename, __statbuf, __flag);
}



extern __inline int
__attribute__ ((__nothrow__)) mknod (__const char *__path, __mode_t __mode, __dev_t __dev)
{
  return __xmknod (1, __path, __mode, &__dev);
}



extern __inline int
__attribute__ ((__nothrow__)) mknodat (int __fd, __const char *__path, __mode_t __mode, __dev_t __dev)

{
  return __xmknodat (1, __fd, __path, __mode, &__dev);
}





extern __inline int
__attribute__ ((__nothrow__)) stat64 (__const char *__path, struct stat64 *__statbuf)
{
  return __xstat64 (3, __path, __statbuf);
}


extern __inline int
__attribute__ ((__nothrow__)) lstat64 (__const char *__path, struct stat64 *__statbuf)
{
  return __lxstat64 (3, __path, __statbuf);
}


extern __inline int
__attribute__ ((__nothrow__)) fstat64 (int __fd, struct stat64 *__statbuf)
{
  return __fxstat64 (3, __fd, __statbuf);
}


extern __inline int
__attribute__ ((__nothrow__)) fstatat64 (int __fd, __const char *__filename, struct stat64 *__statbuf, int __flag)

{
  return __fxstatat64 (3, __fd, __filename, __statbuf, __flag);
}







/* # 47 "../include/lib.h" 2 */
/* # 17 "cache/lvmcache.c" 2 */
/* # 1 "cache/lvmcache.h" 1 */
/* # 19 "cache/lvmcache.h" */
/* # 1 "../include/dev-cache.h" 1 */
/* # 19 "../include/dev-cache.h" */
/* # 1 "../include/device.h" 1 */
/* # 19 "../include/device.h" */
/* # 1 "../include/uuid.h" 1 */
/* # 22 "../include/uuid.h" */
struct id {
 int8_t uuid[32];
};





union lvid {
 struct id id[2];
 char s[2 * sizeof(struct id) + 1 + 7];
};

int lvid_from_lvnum(union lvid *lvid, struct id *vgid, uint32_t lv_num);
int lvnum_from_lvid(union lvid *lvid);
int lvid_in_restricted_range(union lvid *lvid);

void uuid_from_num(char *uuid, uint32_t num);

int lvid_create(union lvid *lvid, struct id *vgid);
int id_create(struct id *id);
int id_valid(struct id *id);
int id_equal(const struct id *lhs, const struct id *rhs);





int id_write_format(const struct id *id, char *buffer, size_t size);




int id_read_format(struct id *id, const char *buffer);

char *id_format_and_copy(struct dm_pool *mem, const struct id *id);
/* # 20 "../include/device.h" 2 */

/* # 1 "/usr/include/fcntl.h" 1 3 4 */
/* # 30 "/usr/include/fcntl.h" 3 4 */




/* # 1 "/usr/include/bits/fcntl.h" 1 3 4 */
/* # 26 "/usr/include/bits/fcntl.h" 3 4 */
/* # 1 "/usr/include/bits/wordsize.h" 1 3 4 */
/* # 27 "/usr/include/bits/fcntl.h" 2 3 4 */

/* # 1 "/usr/include/bits/uio.h" 1 3 4 */
/* # 44 "/usr/include/bits/uio.h" 3 4 */
struct iovec
  {
    void *iov_base;
    size_t iov_len;
  };
/* # 29 "/usr/include/bits/fcntl.h" 2 3 4 */
/* # 167 "/usr/include/bits/fcntl.h" 3 4 */
struct flock
  {
    short int l_type;
    short int l_whence;




    __off64_t l_start;
    __off64_t l_len;

    __pid_t l_pid;
  };


struct flock64
  {
    short int l_type;
    short int l_whence;
    __off64_t l_start;
    __off64_t l_len;
    __pid_t l_pid;
  };




enum __pid_type
  {
    F_OWNER_TID = 0,
    F_OWNER_PID,
    F_OWNER_PGRP,
    F_OWNER_GID = F_OWNER_PGRP
  };


struct f_owner_ex
  {
    enum __pid_type type;
    __pid_t pid;
  };
/* # 252 "/usr/include/bits/fcntl.h" 3 4 */





extern ssize_t readahead (int __fd, __off64_t __offset, size_t __count)
    __attribute__ ((__nothrow__));



extern int sync_file_range (int __fd, __off64_t __offset, __off64_t __count,
       unsigned int __flags);



extern ssize_t vmsplice (int __fdout, const struct iovec *__iov,
    size_t __count, unsigned int __flags);


extern ssize_t splice (int __fdin, __off64_t *__offin, int __fdout,
         __off64_t *__offout, size_t __len,
         unsigned int __flags);


extern ssize_t tee (int __fdin, int __fdout, size_t __len,
      unsigned int __flags);






extern int fallocate (int __fd, int __mode, __off64_t __offset, __off64_t __len) __asm__ ("" "fallocate64")

                     ;





extern int fallocate64 (int __fd, int __mode, __off64_t __offset,
   __off64_t __len);




/* # 35 "/usr/include/fcntl.h" 2 3 4 */





/* # 1 "/usr/include/time.h" 1 3 4 */
/* # 41 "/usr/include/fcntl.h" 2 3 4 */
/* # 1 "/usr/include/bits/stat.h" 1 3 4 */
/* # 42 "/usr/include/fcntl.h" 2 3 4 */
/* # 124 "/usr/include/fcntl.h" 3 4 */
extern int fcntl (int __fd, int __cmd, ...);
/* # 136 "/usr/include/fcntl.h" 3 4 */
extern int open (__const char *__file, int __oflag, ...) __asm__ ("" "open64")
     __attribute__ ((__nonnull__ (1)));





extern int open64 (__const char *__file, int __oflag, ...) __attribute__ ((__nonnull__ (1)));
/* # 161 "/usr/include/fcntl.h" 3 4 */
extern int openat (int __fd, __const char *__file, int __oflag, ...) __asm__ ("" "openat64")
                    __attribute__ ((__nonnull__ (2)));





extern int openat64 (int __fd, __const char *__file, int __oflag, ...)
     __attribute__ ((__nonnull__ (2)));
/* # 182 "/usr/include/fcntl.h" 3 4 */
extern int creat (__const char *__file, __mode_t __mode) __asm__ ("" "creat64")
                  __attribute__ ((__nonnull__ (1)));





extern int creat64 (__const char *__file, __mode_t __mode) __attribute__ ((__nonnull__ (1)));
/* # 229 "/usr/include/fcntl.h" 3 4 */
extern int posix_fadvise (int __fd, __off64_t __offset, __off64_t __len, int __advise) __asm__ ("" "posix_fadvise64") __attribute__ ((__nothrow__))

                      ;





extern int posix_fadvise64 (int __fd, __off64_t __offset, __off64_t __len,
       int __advise) __attribute__ ((__nothrow__));
/* # 250 "/usr/include/fcntl.h" 3 4 */
extern int posix_fallocate (int __fd, __off64_t __offset, __off64_t __len) __asm__ ("" "posix_fallocate64")

                           ;





extern int posix_fallocate64 (int __fd, __off64_t __offset, __off64_t __len);







/* # 1 "/usr/include/bits/fcntl2.h" 1 3 4 */
/* # 31 "/usr/include/bits/fcntl2.h" 3 4 */
extern int __open_2 (__const char *__path, int __oflag) __asm__ ("" "__open64_2")
                     __attribute__ ((__nonnull__ (1)));
extern int __open_alias (__const char *__path, int __oflag, ...) __asm__ ("" "open64")
                 __attribute__ ((__nonnull__ (1)));

extern void __open_too_many_args (void) __attribute__((__error__ ("open can be called either with 2 or 3 arguments, not more")))
                                                                  ;
extern void __open_missing_mode (void) __attribute__((__error__ ("open with O_CREAT in second argument needs 3 arguments")))
                                                               ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
open (__const char *__path, int __oflag, ...)
{
  if (__builtin_va_arg_pack_len () > 1)
    __open_too_many_args ();

  if (__builtin_constant_p (__oflag))
    {
      if ((__oflag & 0100) != 0 && __builtin_va_arg_pack_len () < 1)
 {
   __open_missing_mode ();
   return __open_2 (__path, __oflag);
 }
      return __open_alias (__path, __oflag, __builtin_va_arg_pack ());
    }

  if (__builtin_va_arg_pack_len () < 1)
    return __open_2 (__path, __oflag);

  return __open_alias (__path, __oflag, __builtin_va_arg_pack ());
}



extern int __open64_2 (__const char *__path, int __oflag) __attribute__ ((__nonnull__ (1)));
extern int __open64_alias (__const char *__path, int __oflag, ...) __asm__ ("" "open64")
                   __attribute__ ((__nonnull__ (1)));
extern void __open64_too_many_args (void) __attribute__((__error__ ("open64 can be called either with 2 or 3 arguments, not more")))
                                                                    ;
extern void __open64_missing_mode (void) __attribute__((__error__ ("open64 with O_CREAT in second argument needs 3 arguments")))
                                                                 ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
open64 (__const char *__path, int __oflag, ...)
{
  if (__builtin_va_arg_pack_len () > 1)
    __open64_too_many_args ();

  if (__builtin_constant_p (__oflag))
    {
      if ((__oflag & 0100) != 0 && __builtin_va_arg_pack_len () < 1)
 {
   __open64_missing_mode ();
   return __open64_2 (__path, __oflag);
 }
      return __open64_alias (__path, __oflag, __builtin_va_arg_pack ());
    }

  if (__builtin_va_arg_pack_len () < 1)
    return __open64_2 (__path, __oflag);

  return __open64_alias (__path, __oflag, __builtin_va_arg_pack ());
}
/* # 105 "/usr/include/bits/fcntl2.h" 3 4 */
extern int __openat_2 (int __fd, __const char *__path, int __oflag) __asm__ ("" "__openat64_2")

     __attribute__ ((__nonnull__ (2)));
extern int __openat_alias (int __fd, __const char *__path, int __oflag, ...) __asm__ ("" "openat64")

     __attribute__ ((__nonnull__ (2)));

extern void __openat_too_many_args (void) __attribute__((__error__ ("openat can be called either with 3 or 4 arguments, not more")))
                                                                    ;
extern void __openat_missing_mode (void) __attribute__((__error__ ("openat with O_CREAT in third argument needs 4 arguments")))
                                                                ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
openat (int __fd, __const char *__path, int __oflag, ...)
{
  if (__builtin_va_arg_pack_len () > 1)
    __openat_too_many_args ();

  if (__builtin_constant_p (__oflag))
    {
      if ((__oflag & 0100) != 0 && __builtin_va_arg_pack_len () < 1)
 {
   __openat_missing_mode ();
   return __openat_2 (__fd, __path, __oflag);
 }
      return __openat_alias (__fd, __path, __oflag, __builtin_va_arg_pack ());
    }

  if (__builtin_va_arg_pack_len () < 1)
    return __openat_2 (__fd, __path, __oflag);

  return __openat_alias (__fd, __path, __oflag, __builtin_va_arg_pack ());
}



extern int __openat64_2 (int __fd, __const char *__path, int __oflag)
     __attribute__ ((__nonnull__ (2)));
extern int __openat64_alias (int __fd, __const char *__path, int __oflag, ...) __asm__ ("" "openat64")

     __attribute__ ((__nonnull__ (2)));
extern void __openat64_too_many_args (void) __attribute__((__error__ ("openat64 can be called either with 3 or 4 arguments, not more")))
                                                                      ;
extern void __openat64_missing_mode (void) __attribute__((__error__ ("openat64 with O_CREAT in third argument needs 4 arguments")))
                                                                  ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__artificial__)) int
openat64 (int __fd, __const char *__path, int __oflag, ...)
{
  if (__builtin_va_arg_pack_len () > 1)
    __openat64_too_many_args ();

  if (__builtin_constant_p (__oflag))
    {
      if ((__oflag & 0100) != 0 && __builtin_va_arg_pack_len () < 1)
 {
   __openat64_missing_mode ();
   return __openat64_2 (__fd, __path, __oflag);
 }
      return __openat64_alias (__fd, __path, __oflag, __builtin_va_arg_pack ());
    }

  if (__builtin_va_arg_pack_len () < 1)
    return __openat64_2 (__fd, __path, __oflag);

  return __openat64_alias (__fd, __path, __oflag, __builtin_va_arg_pack ());
}
/* # 267 "/usr/include/fcntl.h" 2 3 4 */



/* # 22 "../include/device.h" 2 */
/* # 35 "../include/device.h" */
struct device {
 struct dm_list aliases;
 dev_t dev;


 int fd;
 int open_count;
 int error_count;
 int max_error_count;
 int block_size;
 int read_ahead;
 uint32_t flags;
 uint64_t end;
 struct dm_list open_list;

 char pvid[32 + 1];
 char _padding[7];
};

struct device_list {
 struct dm_list list;
 struct device *dev;
};

struct device_area {
 struct device *dev;
 uint64_t start;
 uint64_t size;
};




int dev_get_size(const struct device *dev, uint64_t *size);
int dev_get_sectsize(struct device *dev, uint32_t *size);
int dev_get_read_ahead(struct device *dev, uint32_t *read_ahead);


int dev_open(struct device *dev);
int dev_open_quiet(struct device *dev);
int dev_open_flags(struct device *dev, int flags, int direct, int quiet);
int dev_close(struct device *dev);
int dev_close_immediate(struct device *dev);
void dev_close_all(void);
int dev_test_excl(struct device *dev);

int dev_fd(struct device *dev);
const char *dev_name(const struct device *dev);

int dev_read(struct device *dev, uint64_t offset, size_t len, void *buffer);
int dev_read_circular(struct device *dev, uint64_t offset, size_t len,
        uint64_t offset2, size_t len2, char *buf);
int dev_write(struct device *dev, uint64_t offset, size_t len, void *buffer);
int dev_append(struct device *dev, size_t len, char *buffer);
int dev_set(struct device *dev, uint64_t offset, size_t len, int value);
void dev_flush(struct device *dev);

struct device *dev_create_file(const char *filename, struct device *dev,
          struct str_list *alias, int use_malloc);


const char *dev_name_confirmed(struct device *dev, int quiet);


int dev_is_md(struct device *dev, uint64_t *sb);
int dev_is_swap(struct device *dev, uint64_t *signature);
int dev_is_luks(struct device *dev, uint64_t *signature);
unsigned long dev_md_stripe_width(const char *sysfs_dir, struct device *dev);

int is_partitioned_dev(struct device *dev);

int get_primary_dev(const char *sysfs_dir,
      const struct device *dev, dev_t *result);

unsigned long dev_alignment_offset(const char *sysfs_dir,
       struct device *dev);

unsigned long dev_minimum_io_size(const char *sysfs_dir,
      struct device *dev);

unsigned long dev_optimal_io_size(const char *sysfs_dir,
      struct device *dev);
/* # 20 "../include/dev-cache.h" 2 */




struct dev_filter {
 int (*passes_filter) (struct dev_filter * f, struct device * dev);
 void (*destroy) (struct dev_filter * f);
 unsigned use_count;
 void *private;
};




struct cmd_context;
int dev_cache_init(struct cmd_context *cmd);
void dev_cache_exit(void);


void dev_cache_scan(int do_scan);
int dev_cache_has_scanned(void);

int dev_cache_add_dir(const char *path);
int dev_cache_add_loopfile(const char *path);
struct device *dev_cache_get(const char *name, struct dev_filter *f);

void dev_set_preferred_name(struct str_list *sl, struct device *dev);




struct dev_iter;
struct dev_iter *dev_iter_create(struct dev_filter *f, int dev_scan);
void dev_iter_destroy(struct dev_iter *iter);
struct device *dev_iter_get(struct dev_iter *iter);

void dev_reset_error_count(struct cmd_context *cmd);
/* # 20 "cache/lvmcache.h" 2 */
/* # 1 "../include/uuid.h" 1 */
/* # 21 "cache/lvmcache.h" 2 */
/* # 1 "../include/label.h" 1 */
/* # 27 "../include/label.h" */
struct labeller;


struct label_header {
 int8_t id[8];
 uint64_t sector_xl;
 uint32_t crc_xl;
 uint32_t offset_xl;
 int8_t type[8];
} __attribute__ ((packed));


struct label {
 char type[8];
 uint64_t sector;
 struct labeller *labeller;
 void *info;
};

struct labeller;

struct label_ops {



 int (*can_handle) (struct labeller * l, void *buf, uint64_t sector);




 int (*write) (struct label * label, void *buf);




 int (*read) (struct labeller * l, struct device * dev,
       void *buf, struct label ** label);




 int (*verify) (struct labeller * l, void *buf, uint64_t sector);




 int (*initialise_label) (struct labeller * l, struct label * label);




 void (*destroy_label) (struct labeller * l, struct label * label);




 void (*destroy) (struct labeller * l);
};

struct labeller {
 struct label_ops *ops;
 const void *private;
};

int label_init(void);
void label_exit(void);

int label_register_handler(const char *name, struct labeller *handler);

struct labeller *label_get_handler(const char *name);

int label_remove(struct device *dev);
int label_read(struct device *dev, struct label **result,
  uint64_t scan_sector);
int label_write(struct device *dev, struct label *label);
int label_verify(struct device *dev);
struct label *label_create(struct labeller *labeller);
void label_destroy(struct label *label);
/* # 22 "cache/lvmcache.h" 2 */
/* # 1 "../include/locking.h" 1 */
/* # 20 "../include/locking.h" */
/* # 1 "../include/config.h" 1 */
/* # 21 "../include/config.h" */
struct device;
struct cmd_context;

enum {
 CFG_STRING,
 CFG_FLOAT,
 CFG_INT,
 CFG_EMPTY_ARRAY
};

struct config_value {
 int type;
 union {
  int64_t i;
  float r;
  const char *str;
 } v;
 struct config_value *next;
};

struct config_node {
 const char *key;
 struct config_node *parent, *sib, *child;
 struct config_value *v;
};

struct config_tree {
 struct config_node *root;
};

struct config_tree_list {
 struct dm_list list;
 struct config_tree *cft;
};

struct config_tree *create_config_tree(const char *filename, int keep_open);
struct config_tree *create_config_tree_from_string(struct cmd_context *cmd,
         const char *config_settings);
int override_config_tree_from_string(struct cmd_context *cmd,
         const char *config_settings);
void destroy_config_tree(struct config_tree *cft);

typedef uint32_t (*checksum_fn_t) (uint32_t initial, const uint8_t *buf, uint32_t size);

int read_config_fd(struct config_tree *cft, struct device *dev,
     off_t offset, size_t size, off_t offset2, size_t size2,
     checksum_fn_t checksum_fn, uint32_t checksum);

int read_config_file(struct config_tree *cft);
int write_config_file(struct config_tree *cft, const char *file,
        int argc, char **argv);

typedef int (*putline_fn)(const char *line, void *baton);
int write_config_node(const struct config_node *cn, putline_fn putline, void *baton);

time_t config_file_timestamp(struct config_tree *cft);
int config_file_changed(struct config_tree *cft);
int merge_config_tree(struct cmd_context *cmd, struct config_tree *cft,
        struct config_tree *newdata);

const struct config_node *find_config_node(const struct config_node *cn,
        const char *path);
const char *find_config_str(const struct config_node *cn, const char *path,
       const char *fail);
int find_config_int(const struct config_node *cn, const char *path, int fail);
float find_config_float(const struct config_node *cn, const char *path,
   float fail);




const struct config_node *find_config_tree_node(struct cmd_context *cmd,
      const char *path);
const char *find_config_tree_str(struct cmd_context *cmd,
     const char *path, const char *fail);
int find_config_tree_int(struct cmd_context *cmd, const char *path,
    int fail);
int64_t find_config_tree_int64(struct cmd_context *cmd, const char *path,
        int64_t fail);
float find_config_tree_float(struct cmd_context *cmd, const char *path,
        float fail);





int find_config_bool(const struct config_node *cn, const char *path, int fail);
int find_config_tree_bool(struct cmd_context *cmd, const char *path, int fail);

int get_config_uint32(const struct config_node *cn, const char *path,
        uint32_t *result);

int get_config_uint64(const struct config_node *cn, const char *path,
        uint64_t *result);

int get_config_str(const struct config_node *cn, const char *path,
     const char **result);

unsigned maybe_config_section(const char *str, unsigned len);

const char *config_parent_name(const struct config_node *n);

struct config_node *clone_config_node(struct dm_pool *mem, const struct config_node *cn,
          int siblings);
/* # 21 "../include/locking.h" 2 */

int init_locking(int type, struct cmd_context *cmd, int suppress_messages);
void fin_locking(void);
void reset_locking(void);
int vg_write_lock_held(void);
int locking_is_clustered(void);

int remote_lock_held(const char *vol, int *exclusive);
/* # 46 "../include/locking.h" */
int lock_vol(struct cmd_context *cmd, const char *vol, uint32_t flags);
/* # 60 "../include/locking.h" */
int check_lvm1_vg_inactive(struct cmd_context *cmd, const char *vgname);
/* # 184 "../include/locking.h" */
int sync_local_dev_names(struct cmd_context* cmd);
int sync_dev_names(struct cmd_context* cmd);


int suspend_lvs(struct cmd_context *cmd, struct dm_list *lvs);
int resume_lvs(struct cmd_context *cmd, struct dm_list *lvs);
int activate_lvs(struct cmd_context *cmd, struct dm_list *lvs, unsigned exclusive);


void sigint_clear(void);
void sigint_allow(void);
void sigint_restore(void);
int sigint_caught(void);
/* # 23 "cache/lvmcache.h" 2 */
/* # 33 "cache/lvmcache.h" */
struct cmd_context;
struct format_type;
struct volume_group;
struct config_tree;


struct lvmcache_vginfo {
 struct dm_list list;
 struct dm_list infos;
 const struct format_type *fmt;
 char *vgname;
 uint32_t status;
 char vgid[32 + 1];
 char _padding[7];
 struct lvmcache_vginfo *next;
 char *creation_host;
 char *vgmetadata;
 struct config_tree *cft;

 unsigned precommitted;
};


struct lvmcache_info {
 struct dm_list list;
 struct dm_list mdas;
 struct dm_list das;
 struct lvmcache_vginfo *vginfo;
 struct label *label;
 const struct format_type *fmt;
 struct device *dev;
 uint64_t device_size;
 uint32_t status;
};

int lvmcache_init(void);
void lvmcache_destroy(struct cmd_context *cmd, int retain_orphans);



int lvmcache_label_scan(struct cmd_context *cmd, int full_scan);


struct lvmcache_info *lvmcache_add(struct labeller *labeller, const char *pvid,
       struct device *dev,
       const char *vgname, const char *vgid,
       uint32_t vgstatus);
int lvmcache_add_orphan_vginfo(const char *vgname, struct format_type *fmt);
void lvmcache_del(struct lvmcache_info *info);


int lvmcache_update_vgname_and_id(struct lvmcache_info *info,
      const char *vgname, const char *vgid,
      uint32_t vgstatus, const char *hostname);
int lvmcache_update_vg(struct volume_group *vg, unsigned precommitted);

void lvmcache_lock_vgname(const char *vgname, int read_only);
void lvmcache_unlock_vgname(const char *vgname);
int lvmcache_verify_lock_order(const char *vgname);


const struct format_type *fmt_from_vgname(const char *vgname, const char *vgid, unsigned revalidate_labels);
struct lvmcache_vginfo *vginfo_from_vgname(const char *vgname,
        const char *vgid);
struct lvmcache_vginfo *vginfo_from_vgid(const char *vgid);
struct lvmcache_info *info_from_pvid(const char *pvid, int valid_only);
const char *vgname_from_vgid(struct dm_pool *mem, const char *vgid);
struct device *device_from_pvid(struct cmd_context *cmd, const struct id *pvid,
    unsigned *scan_done_once);
const char *pvid_from_devname(struct cmd_context *cmd,
         const char *dev_name);
char *lvmcache_vgname_from_pvid(struct cmd_context *cmd, const char *pvid);
int vgs_locked(void);
int vgname_is_locked(const char *vgname);



struct dm_list *lvmcache_get_vgnames(struct cmd_context *cmd,
         int include_internal);



struct dm_list *lvmcache_get_vgids(struct cmd_context *cmd,
       int include_internal);


struct dm_list *lvmcache_get_pvids(struct cmd_context *cmd, const char *vgname,
    const char *vgid);


struct volume_group *lvmcache_get_vg(const char *vgid, unsigned precommitted);
void lvmcache_drop_metadata(const char *vgname, int drop_precommitted);
void lvmcache_commit_metadata(const char *vgname);
/* # 18 "cache/lvmcache.c" 2 */
/* # 1 "../include/toolcontext.h" 1 */
/* # 19 "../include/toolcontext.h" */
/* # 1 "../include/dev-cache.h" 1 */
/* # 20 "../include/toolcontext.h" 2 */


/* # 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include-fixed/limits.h" 1 3 4 */
/* # 23 "../include/toolcontext.h" 2 */




struct config_info {
 int debug;
 int verbose;
 int test;
 int syslog;
 int activation;
 int suffix;
 int archive;
 int backup;
 int read_ahead;
 int udev_rules;
 int udev_sync;
 int cache_vgmetadata;
 const char *msg_prefix;
 const char *fmt_name;
 uint64_t unit_factor;
 int cmd_name;
 mode_t umask;
 char unit_type;
 char _padding[1];
};

struct config_tree;
struct archive_params;
struct backup_params;
struct arg_values;



struct cmd_context {
 struct dm_pool *libmem;
 struct dm_pool *mem;

 const struct format_type *fmt;
 struct format_type *fmt_backup;

 struct dm_list formats;
 struct dm_list segtypes;
 const char *hostname;
 const char *kernel_vsn;

 unsigned rand_seed;
 const char *cmd_line;
 struct command *command;
 char **argv;
 struct arg_values *arg_values;
 struct dm_list arg_value_groups;
 unsigned is_long_lived:1;
 unsigned handles_missing_pvs:1;
 unsigned handles_unknown_segments:1;
 unsigned partial_activation:1;
 unsigned si_unit_consistency:1;
 unsigned metadata_read_only:1;

 unsigned independent_metadata_areas:1;

 struct dev_filter *filter;
 int dump_filter;

 struct dm_list config_files;
 int config_valid;
 struct config_tree *cft;
 struct config_tree *cft_override;
 struct config_info default_settings;
 struct config_info current_settings;

 struct archive_params *archive_params;
 struct backup_params *backup_params;
 const char *stripe_filler;


 struct dm_list tags;
 int hosttags;

 char system_dir[4096];
 char dev_dir[4096];
 char proc_dir[4096];
 char sysfs_dir[4096];
};





struct cmd_context *create_toolcontext(unsigned is_long_lived,
           const char *system_dir);
void destroy_toolcontext(struct cmd_context *cmd);
int refresh_toolcontext(struct cmd_context *cmd);
int refresh_filters(struct cmd_context *cmd);
int config_files_changed(struct cmd_context *cmd);
int init_lvmcache_orphans(struct cmd_context *cmd);

struct format_type *get_format_by_name(struct cmd_context *cmd, const char *format);
/* # 19 "cache/lvmcache.c" 2 */


/* # 1 "../include/metadata.h" 1 */
/* # 24 "../include/metadata.h" */
/* # 1 "/usr/include/ctype.h" 1 3 4 */
/* # 30 "/usr/include/ctype.h" 3 4 */

/* # 48 "/usr/include/ctype.h" 3 4 */
enum
{
  _ISupper = ((0) < 8 ? ((1 << (0)) << 8) : ((1 << (0)) >> 8)),
  _ISlower = ((1) < 8 ? ((1 << (1)) << 8) : ((1 << (1)) >> 8)),
  _ISalpha = ((2) < 8 ? ((1 << (2)) << 8) : ((1 << (2)) >> 8)),
  _ISdigit = ((3) < 8 ? ((1 << (3)) << 8) : ((1 << (3)) >> 8)),
  _ISxdigit = ((4) < 8 ? ((1 << (4)) << 8) : ((1 << (4)) >> 8)),
  _ISspace = ((5) < 8 ? ((1 << (5)) << 8) : ((1 << (5)) >> 8)),
  _ISprint = ((6) < 8 ? ((1 << (6)) << 8) : ((1 << (6)) >> 8)),
  _ISgraph = ((7) < 8 ? ((1 << (7)) << 8) : ((1 << (7)) >> 8)),
  _ISblank = ((8) < 8 ? ((1 << (8)) << 8) : ((1 << (8)) >> 8)),
  _IScntrl = ((9) < 8 ? ((1 << (9)) << 8) : ((1 << (9)) >> 8)),
  _ISpunct = ((10) < 8 ? ((1 << (10)) << 8) : ((1 << (10)) >> 8)),
  _ISalnum = ((11) < 8 ? ((1 << (11)) << 8) : ((1 << (11)) >> 8))
};
/* # 81 "/usr/include/ctype.h" 3 4 */
extern __const unsigned short int **__ctype_b_loc (void)
     __attribute__ ((__nothrow__)) __attribute__ ((__const));
extern __const __int32_t **__ctype_tolower_loc (void)
     __attribute__ ((__nothrow__)) __attribute__ ((__const));
extern __const __int32_t **__ctype_toupper_loc (void)
     __attribute__ ((__nothrow__)) __attribute__ ((__const));
/* # 96 "/usr/include/ctype.h" 3 4 */






extern int isalnum (int) __attribute__ ((__nothrow__));
extern int isalpha (int) __attribute__ ((__nothrow__));
extern int iscntrl (int) __attribute__ ((__nothrow__));
extern int isdigit (int) __attribute__ ((__nothrow__));
extern int islower (int) __attribute__ ((__nothrow__));
extern int isgraph (int) __attribute__ ((__nothrow__));
extern int isprint (int) __attribute__ ((__nothrow__));
extern int ispunct (int) __attribute__ ((__nothrow__));
extern int isspace (int) __attribute__ ((__nothrow__));
extern int isupper (int) __attribute__ ((__nothrow__));
extern int isxdigit (int) __attribute__ ((__nothrow__));



extern int tolower (int __c) __attribute__ ((__nothrow__));


extern int toupper (int __c) __attribute__ ((__nothrow__));








extern int isblank (int) __attribute__ ((__nothrow__));






extern int isctype (int __c, int __mask) __attribute__ ((__nothrow__));






extern int isascii (int __c) __attribute__ ((__nothrow__));



extern int toascii (int __c) __attribute__ ((__nothrow__));



extern int _toupper (int) __attribute__ ((__nothrow__));
extern int _tolower (int) __attribute__ ((__nothrow__));
/* # 190 "/usr/include/ctype.h" 3 4 */
extern __inline int
__attribute__ ((__nothrow__)) tolower (int __c)
{
  return __c >= -128 && __c < 256 ? (*__ctype_tolower_loc ())[__c] : __c;
}

extern __inline int
__attribute__ ((__nothrow__)) toupper (int __c)
{
  return __c >= -128 && __c < 256 ? (*__ctype_toupper_loc ())[__c] : __c;
}
/* # 247 "/usr/include/ctype.h" 3 4 */
extern int isalnum_l (int, __locale_t) __attribute__ ((__nothrow__));
extern int isalpha_l (int, __locale_t) __attribute__ ((__nothrow__));
extern int iscntrl_l (int, __locale_t) __attribute__ ((__nothrow__));
extern int isdigit_l (int, __locale_t) __attribute__ ((__nothrow__));
extern int islower_l (int, __locale_t) __attribute__ ((__nothrow__));
extern int isgraph_l (int, __locale_t) __attribute__ ((__nothrow__));
extern int isprint_l (int, __locale_t) __attribute__ ((__nothrow__));
extern int ispunct_l (int, __locale_t) __attribute__ ((__nothrow__));
extern int isspace_l (int, __locale_t) __attribute__ ((__nothrow__));
extern int isupper_l (int, __locale_t) __attribute__ ((__nothrow__));
extern int isxdigit_l (int, __locale_t) __attribute__ ((__nothrow__));

extern int isblank_l (int, __locale_t) __attribute__ ((__nothrow__));



extern int __tolower_l (int __c, __locale_t __l) __attribute__ ((__nothrow__));
extern int tolower_l (int __c, __locale_t __l) __attribute__ ((__nothrow__));


extern int __toupper_l (int __c, __locale_t __l) __attribute__ ((__nothrow__));
extern int toupper_l (int __c, __locale_t __l) __attribute__ ((__nothrow__));
/* # 323 "/usr/include/ctype.h" 3 4 */

/* # 25 "../include/metadata.h" 2 */

/* # 1 "../include/lvm-string.h" 1 */
/* # 25 "../include/lvm-string.h" */
struct pool;

int emit_to_buffer(char **buffer, size_t *size, const char *fmt, ...)
  __attribute__ ((format(printf, 3, 4)));

char *build_dm_name(struct dm_pool *mem, const char *vg,
                    const char *lv, const char *layer);
char *build_dm_uuid(struct dm_pool *mem, const char *lvid,
      const char *layer);

int validate_name(const char *n);
int validate_tag(const char *n);

int apply_lvname_restrictions(const char *name);
int is_reserved_lvname(const char *name);




unsigned count_chars(const char *str, size_t len, const int c);




size_t escaped_len(const char *str);







char *escape_double_quotes(char *out, const char *src);




void unescape_double_quotes(char *src);






void unescape_colons_and_at_signs(char *src,
      char **substr_first_unquoted_colon,
      char **substr_first_unquoted_at_sign);
/* # 27 "../include/metadata.h" 2 */
/* # 1 "../include/metadata-exported.h" 1 */
/* # 25 "../include/metadata-exported.h" */
/* # 1 "../include/pv.h" 1 */
/* # 18 "../include/pv.h" */
struct id;
struct device;
struct format_type;
struct volume_group;

struct physical_volume {
 struct id id;
 struct id old_id;
 struct device *dev;
 const struct format_type *fmt;
 struct format_instance *fid;





 const char *vg_name;
 struct id vgid;





 struct volume_group *vg;

 uint64_t status;
 uint64_t size;


 uint32_t pe_size;
 uint64_t pe_start;
 uint32_t pe_count;
 uint32_t pe_alloc_count;
 unsigned long pe_align;
 unsigned long pe_align_offset;

 struct dm_list segments;
 struct dm_list tags;
};

char *pv_fmt_dup(const struct physical_volume *pv);
char *pv_name_dup(const struct physical_volume *pv);
struct device *pv_dev(const struct physical_volume *pv);
const char *pv_vg_name(const struct physical_volume *pv);
char *pv_attr_dup(struct dm_pool *mem, const struct physical_volume *pv);
const char *pv_dev_name(const struct physical_volume *pv);
char *pv_uuid_dup(const struct physical_volume *pv);
char *pv_tags_dup(const struct physical_volume *pv);
uint64_t pv_size(const struct physical_volume *pv);
uint64_t pv_size_field(const struct physical_volume *pv);
uint64_t pv_dev_size(const struct physical_volume *pv);
uint64_t pv_free(const struct physical_volume *pv);
uint64_t pv_status(const struct physical_volume *pv);
uint32_t pv_pe_size(const struct physical_volume *pv);
uint64_t pv_pe_start(const struct physical_volume *pv);
uint32_t pv_pe_count(const struct physical_volume *pv);
uint32_t pv_pe_alloc_count(const struct physical_volume *pv);
uint64_t pv_mda_size(const struct physical_volume *pv);
uint64_t pv_mda_free(const struct physical_volume *pv);
uint64_t pv_used(const struct physical_volume *pv);
uint32_t pv_mda_count(const struct physical_volume *pv);
uint32_t pv_mda_used_count(const struct physical_volume *pv);
unsigned pv_mda_set_ignored(const struct physical_volume *pv, unsigned ignored);
int is_orphan(const struct physical_volume *pv);
int is_missing_pv(const struct physical_volume *pv);
int is_pv(const struct physical_volume *pv);
/* # 26 "../include/metadata-exported.h" 2 */
/* # 1 "../include/vg.h" 1 */
/* # 18 "../include/vg.h" */
struct cmd_context;
struct dm_pool;
struct format_instance;
struct dm_list;
struct id;

typedef enum {
 ALLOC_INVALID,
 ALLOC_CONTIGUOUS,
 ALLOC_CLING,
 ALLOC_CLING_BY_TAGS,
 ALLOC_NORMAL,
 ALLOC_ANYWHERE,
 ALLOC_INHERIT
} alloc_policy_t;

struct volume_group {
 struct cmd_context *cmd;
 struct dm_pool *vgmem;
 struct format_instance *fid;
 struct dm_list *cmd_vgs;
 uint32_t cmd_missing_vgs;
 uint32_t seqno;

 alloc_policy_t alloc;
 uint64_t status;

 struct id id;
 const char *name;
 const char *old_name;
 char *system_id;

 uint32_t extent_size;
 uint32_t extent_count;
 uint32_t free_count;

 uint32_t max_lv;
 uint32_t max_pv;


 uint32_t pv_count;
 struct dm_list pvs;
/* # 75 "../include/vg.h" */
 struct dm_list lvs;

 struct dm_list tags;
/* # 87 "../include/vg.h" */
 struct dm_list removed_pvs;
 uint32_t open_mode;





 uint32_t read_status;
 uint32_t mda_copies;
};

struct volume_group *alloc_vg(const char *pool_name, struct cmd_context *cmd,
         const char *vg_name);

char *vg_fmt_dup(const struct volume_group *vg);
char *vg_name_dup(const struct volume_group *vg);
char *vg_system_id_dup(const struct volume_group *vg);
uint32_t vg_seqno(const struct volume_group *vg);
uint64_t vg_status(const struct volume_group *vg);
int vg_set_alloc_policy(struct volume_group *vg, alloc_policy_t alloc);
int vg_set_clustered(struct volume_group *vg, int clustered);
uint64_t vg_size(const struct volume_group *vg);
uint64_t vg_free(const struct volume_group *vg);
uint64_t vg_extent_size(const struct volume_group *vg);
int vg_set_extent_size(struct volume_group *vg, uint32_t new_extent_size);
uint64_t vg_extent_count(const struct volume_group *vg);
uint64_t vg_free_count(const struct volume_group *vg);
uint64_t vg_pv_count(const struct volume_group *vg);
uint64_t vg_max_pv(const struct volume_group *vg);
int vg_set_max_pv(struct volume_group *vg, uint32_t max_pv);
uint64_t vg_max_lv(const struct volume_group *vg);
int vg_set_max_lv(struct volume_group *vg, uint32_t max_lv);
uint32_t vg_mda_count(const struct volume_group *vg);
uint32_t vg_mda_used_count(const struct volume_group *vg);
uint32_t vg_mda_copies(const struct volume_group *vg);
int vg_set_mda_copies(struct volume_group *vg, uint32_t mda_copies);



unsigned vg_visible_lvs(const struct volume_group *vg);



unsigned snapshot_count(const struct volume_group *vg);

uint64_t vg_mda_size(const struct volume_group *vg);
uint64_t vg_mda_free(const struct volume_group *vg);
char *vg_attr_dup(struct dm_pool *mem, const struct volume_group *vg);
char *vg_uuid_dup(const struct volume_group *vg);
char *vg_tags_dup(const struct volume_group *vg);
/* # 27 "../include/metadata-exported.h" 2 */
/* # 1 "../include/lv.h" 1 */
/* # 18 "../include/lv.h" */
union lvid;
struct volume_group;
struct dm_list;
struct lv_segment;
struct replicator_device;

struct logical_volume {
 union lvid lvid;
 const char *name;

 struct volume_group *vg;

 uint64_t status;
 alloc_policy_t alloc;
 uint32_t read_ahead;
 int32_t major;
 int32_t minor;

 uint64_t size;
 uint32_t le_count;

 uint32_t origin_count;
 struct dm_list snapshot_segs;
 struct lv_segment *snapshot;

 struct replicator_device *rdevice;
 struct dm_list rsites;

 struct dm_list segments;
 struct dm_list tags;
 struct dm_list segs_using_this_lv;
};

uint64_t lv_size(const struct logical_volume *lv);
char *lv_attr_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_uuid_dup(const struct logical_volume *lv);
char *lv_tags_dup(const struct logical_volume *lv);
char *lv_path_dup(struct dm_pool *mem, const struct logical_volume *lv);
uint64_t lv_origin_size(const struct logical_volume *lv);
char *lv_move_pv_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_convert_lv_dup(struct dm_pool *mem, const struct logical_volume *lv);
int lv_kernel_major(const struct logical_volume *lv);
int lv_kernel_minor(const struct logical_volume *lv);
char *lv_mirror_log_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_modules_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_name_dup(struct dm_pool *mem, const struct logical_volume *lv);
char *lv_origin_dup(struct dm_pool *mem, const struct logical_volume *lv);
uint32_t lv_kernel_read_ahead(const struct logical_volume *lv);
uint64_t lvseg_start(const struct lv_segment *seg);
uint64_t lvseg_size(const struct lv_segment *seg);
uint64_t lvseg_chunksize(const struct lv_segment *seg);
char *lvseg_segtype_dup(struct dm_pool *mem, const struct lv_segment *seg);
char *lvseg_tags_dup(const struct lv_segment *seg);
/* # 28 "../include/metadata-exported.h" 2 */
/* # 1 "../include/lvm-percent.h" 1 */
/* # 31 "../include/lvm-percent.h" */
typedef int32_t percent_t;

typedef enum {
 PERCENT_0 = 0,
 PERCENT_1 = 1000000,
 PERCENT_100 = 100 * PERCENT_1,
 PERCENT_INVALID = -1
} percent_range_t;

float percent_to_float(percent_t v);
percent_t make_percent(uint64_t numerator, uint64_t denominator);
/* # 29 "../include/metadata-exported.h" 2 */
/* # 128 "../include/metadata-exported.h" */
typedef enum {
 AREA_UNASSIGNED,
 AREA_PV,
 AREA_LV
} area_type_t;




typedef enum {
 PROMPT = 0,
 DONT_PROMPT = 1,
 DONT_PROMPT_OVERRIDE = 2
} force_t;

struct cmd_context;
struct format_handler;
struct labeller;

struct format_type {
 struct dm_list list;
 struct cmd_context *cmd;
 struct format_handler *ops;
 struct labeller *labeller;
 const char *name;
 const char *alias;
 const char *orphan_vg_name;
 uint32_t features;
 void *library;
 void *private;
};

struct pv_segment {
 struct dm_list list;


 struct physical_volume *pv;
 uint32_t pe;
 uint32_t len;

 struct lv_segment *lvseg;
 uint32_t lv_area;
};
/* # 198 "../include/metadata-exported.h" */
struct format_instance {
 unsigned ref_count;
 struct dm_pool *mem;

 uint32_t type;
 const struct format_type *fmt;
/* # 212 "../include/metadata-exported.h" */
 struct dm_list metadata_areas_in_use;
 struct dm_list metadata_areas_ignored;
 union {
  struct metadata_area **array;
  struct dm_hash_table *hash;
 } metadata_areas_index;

 void *private;
};


struct lv_segment_area {
 area_type_t type;
 union {
  struct {
   struct pv_segment *pvseg;
  } pv;
  struct {
   struct logical_volume *lv;
   uint32_t le;
  } lv;
 } u;
};

struct segment_type;


struct cmd_vg {
 struct dm_list list;
 const char *vg_name;
 const char *vgid;
 uint32_t flags;
 struct volume_group *vg;
};


typedef enum {
 REPLICATOR_STATE_PASSIVE,
 REPLICATOR_STATE_ACTIVE,
 NUM_REPLICATOR_STATE
} replicator_state_t;

struct replicator_site {
 struct dm_list list;
 struct dm_list rdevices;

 struct logical_volume *replicator;

 const char *name;
 const char *vg_name;
 struct volume_group *vg;
 unsigned site_index;
 replicator_state_t state;
 dm_replicator_mode_t op_mode;
 uint64_t fall_behind_data;
 uint32_t fall_behind_ios;
 uint32_t fall_behind_timeout;
};

struct replicator_device {
 struct dm_list list;

 struct lv_segment *replicator_dev;
 struct replicator_site *rsite;

 uint64_t device_index;
 const char *name;
 struct logical_volume *lv;
 struct logical_volume *slog;
 const char *slog_name;
};


struct lv_segment {
 struct dm_list list;
 struct logical_volume *lv;

 const struct segment_type *segtype;
 uint32_t le;
 uint32_t len;

 uint64_t status;


 uint32_t stripe_size;
 uint32_t area_count;
 uint32_t area_len;
 uint32_t chunk_size;
 struct logical_volume *origin;
 struct logical_volume *cow;
 struct dm_list origin_list;
 uint32_t region_size;
 uint32_t extents_copied;
 struct logical_volume *log_lv;
 struct lv_segment *pvmove_source_seg;
 void *segtype_private;

 struct dm_list tags;

 struct lv_segment_area *areas;

 struct logical_volume *replicator;
 struct logical_volume *rlog_lv;
 const char *rlog_type;
 uint64_t rdevice_index_highest;
 unsigned rsite_index_highest;
};





struct pe_range {
 struct dm_list list;
 uint32_t start;
 uint32_t count;
};

struct pv_list {
 struct dm_list list;
 struct physical_volume *pv;
 struct dm_list *mdas;
 struct dm_list *pe_ranges;
};

struct lv_list {
 struct dm_list list;
 struct logical_volume *lv;
};

struct pvcreate_params {
 int zero;
 uint64_t size;
 uint64_t data_alignment;
 uint64_t data_alignment_offset;
 int pvmetadatacopies;
 uint64_t pvmetadatasize;
 int64_t labelsector;
 struct id id;
 struct id *idp;
 uint64_t pe_start;
 uint32_t extent_count;
 uint32_t extent_size;
 const char *restorefile;
 force_t force;
 unsigned yes;
 unsigned metadataignore;
};

struct physical_volume *pvcreate_single(struct cmd_context *cmd,
     const char *pv_name,
     struct pvcreate_params *pp);
void pvcreate_params_set_defaults(struct pvcreate_params *pp);




int vg_write(struct volume_group *vg);
int vg_commit(struct volume_group *vg);
int vg_revert(struct volume_group *vg);
struct volume_group *vg_read_internal(struct cmd_context *cmd, const char *vg_name,
        const char *vgid, int warnings, int *consistent);
struct physical_volume *pv_read(struct cmd_context *cmd, const char *pv_name,
    uint64_t *label_sector, int warnings,
    int scan_label_only);
struct dm_list *get_pvs(struct cmd_context *cmd);




int link_lv_to_vg(struct volume_group *vg, struct logical_volume *lv);
int unlink_lv_from_vg(struct logical_volume *lv);
void lv_set_visible(struct logical_volume *lv);
void lv_set_hidden(struct logical_volume *lv);

struct dm_list *get_vgnames(struct cmd_context *cmd, int include_internal);
struct dm_list *get_vgids(struct cmd_context *cmd, int include_internal);
int scan_vgs_for_pvs(struct cmd_context *cmd, int warnings);

int pv_write(struct cmd_context *cmd, struct physical_volume *pv, int allow_non_orphan);
int move_pv(struct volume_group *vg_from, struct volume_group *vg_to,
     const char *pv_name);
int move_pvs_used_by_lv(struct volume_group *vg_from,
   struct volume_group *vg_to,
   const char *lv_name);
int is_global_vg(const char *vg_name);
int is_orphan_vg(const char *vg_name);
int is_real_vg(const char *vg_name);
int vg_missing_pv_count(const struct volume_group *vg);
int vgs_are_compatible(struct cmd_context *cmd,
         struct volume_group *vg_from,
         struct volume_group *vg_to);
uint32_t vg_lock_newname(struct cmd_context *cmd, const char *vgname);




struct volume_group *vg_read(struct cmd_context *cmd, const char *vg_name,
              const char *vgid, uint32_t flags);
struct volume_group *vg_read_for_update(struct cmd_context *cmd, const char *vg_name,
                         const char *vgid, uint32_t flags);




uint32_t vg_read_error(struct volume_group *vg_handle);



struct physical_volume *pv_create(const struct cmd_context *cmd,
      struct device *dev,
      struct id *id,
      uint64_t size,
      unsigned long data_alignment,
      unsigned long data_alignment_offset,
      uint64_t pe_start,
      uint32_t existing_extent_count,
      uint32_t existing_extent_size,
      uint64_t label_sector,
      int pvmetadatacopies,
      uint64_t pvmetadatasize,
      unsigned metadataignore);
int pv_resize(struct physical_volume *pv, struct volume_group *vg,
             uint64_t size);
int pv_analyze(struct cmd_context *cmd, const char *pv_name,
        uint64_t label_sector);


uint32_t pv_list_extents_free(const struct dm_list *pvh);

struct volume_group *vg_create(struct cmd_context *cmd, const char *vg_name);
int vg_remove_mdas(struct volume_group *vg);
int vg_remove_check(struct volume_group *vg);
void vg_remove_pvs(struct volume_group *vg);
int vg_remove(struct volume_group *vg);
int vg_rename(struct cmd_context *cmd, struct volume_group *vg,
       const char *new_name);
int vg_extend(struct volume_group *vg, int pv_count, const char *const *pv_names,
       struct pvcreate_params *pp);
int vg_reduce(struct volume_group *vg, const char *pv_name);
int vg_change_tag(struct volume_group *vg, const char *tag, int add_tag);
int vg_split_mdas(struct cmd_context *cmd, struct volume_group *vg_from,
    struct volume_group *vg_to);

void add_pvl_to_vgs(struct volume_group *vg, struct pv_list *pvl);
void del_pvl_from_vgs(struct volume_group *vg, struct pv_list *pvl);


int remove_lvs_in_vg(struct cmd_context *cmd,
       struct volume_group *vg,
       force_t force);






void free_pv_fid(struct physical_volume *pv);





void free_vg(struct volume_group *vg);


struct logical_volume *lv_create_empty(const char *name,
           union lvid *lvid,
           uint64_t status,
           alloc_policy_t alloc,
           struct volume_group *vg);


int set_lv(struct cmd_context *cmd, struct logical_volume *lv,
           uint64_t sectors, int value);

int lv_change_tag(struct logical_volume *lv, const char *tag, int add_tag);


int lv_reduce(struct logical_volume *lv, uint32_t extents);


int lv_empty(struct logical_volume *lv);


int replace_lv_with_error_segment(struct logical_volume *lv);


int lv_extend(struct logical_volume *lv,
       const struct segment_type *segtype,
       uint32_t stripes, uint32_t stripe_size,
       uint32_t mirrors, uint32_t extents,
       struct physical_volume *mirrored_pv, uint32_t mirrored_pe,
       uint64_t status, struct dm_list *allocatable_pvs,
       alloc_policy_t alloc);


int lv_remove(struct logical_volume *lv);

int lv_remove_single(struct cmd_context *cmd, struct logical_volume *lv,
       force_t force);

int lv_remove_with_dependencies(struct cmd_context *cmd, struct logical_volume *lv,
    force_t force, unsigned level);

int lv_rename(struct cmd_context *cmd, struct logical_volume *lv,
       const char *new_name);

uint64_t extents_from_size(struct cmd_context *cmd, uint64_t size,
      uint32_t extent_size);


struct lvcreate_params {

 int snapshot;
 int zero;
 int major;
 int minor;
 int log_count;
 int nosync;
 int activation_monitoring;

 char *origin;
 const char *vg_name;
 const char *lv_name;

 uint32_t stripes;
 uint32_t stripe_size;
 uint32_t chunk_size;
 uint32_t region_size;

 uint32_t mirrors;

 const struct segment_type *segtype;


 uint32_t extents;
 uint32_t voriginextents;
 uint64_t voriginsize;
 struct dm_list *pvh;

 uint32_t permission;
 uint32_t read_ahead;
 alloc_policy_t alloc;

 struct dm_list tags;
};

int lv_create_single(struct volume_group *vg,
       struct lvcreate_params *lp);




int insert_layer_for_segments_on_pv(struct cmd_context *cmd,
        struct logical_volume *lv_where,
        struct logical_volume *layer_lv,
        uint64_t status,
        struct pv_list *pv,
        struct dm_list *lvs_changed);
int remove_layers_for_segments(struct cmd_context *cmd,
          struct logical_volume *lv,
          struct logical_volume *layer_lv,
          uint64_t status_mask, struct dm_list *lvs_changed);
int remove_layers_for_segments_all(struct cmd_context *cmd,
       struct logical_volume *layer_lv,
       uint64_t status_mask,
       struct dm_list *lvs_changed);
int split_parent_segments_for_layer(struct cmd_context *cmd,
        struct logical_volume *layer_lv);
int remove_layer_from_lv(struct logical_volume *lv,
    struct logical_volume *layer_lv);
struct logical_volume *insert_layer_for_lv(struct cmd_context *cmd,
        struct logical_volume *lv_where,
        uint64_t status,
        const char *layer_suffix);


struct pv_list *find_pv_in_vg(const struct volume_group *vg,
         const char *pv_name);
struct pv_list *find_pv_in_vg_by_uuid(const struct volume_group *vg,
          const struct id *id);


struct lv_list *find_lv_in_vg(const struct volume_group *vg,
         const char *lv_name);


struct logical_volume *find_lv(const struct volume_group *vg,
          const char *lv_name);
struct physical_volume *find_pv_by_name(struct cmd_context *cmd,
     const char *pv_name);

const char *find_vgname_from_pvname(struct cmd_context *cmd,
        const char *pvname);
const char *find_vgname_from_pvid(struct cmd_context *cmd,
      const char *pvid);

struct lv_segment *first_seg(const struct logical_volume *lv);





int lv_is_origin(const struct logical_volume *lv);
int lv_is_virtual_origin(const struct logical_volume *lv);
int lv_is_cow(const struct logical_volume *lv);
int lv_is_merging_origin(const struct logical_volume *origin);
int lv_is_merging_cow(const struct logical_volume *snapshot);


int lv_is_visible(const struct logical_volume *lv);

int pv_is_in_vg(struct volume_group *vg, struct physical_volume *pv);

struct lv_segment *find_merging_cow(const struct logical_volume *origin);


struct lv_segment *find_cow(const struct logical_volume *lv);


struct logical_volume *origin_from_cow(const struct logical_volume *lv);

void init_snapshot_seg(struct lv_segment *seg, struct logical_volume *origin,
         struct logical_volume *cow, uint32_t chunk_size, int merge);

void init_snapshot_merge(struct lv_segment *cow_seg, struct logical_volume *origin);

void clear_snapshot_merge(struct logical_volume *origin);

int vg_add_snapshot(struct logical_volume *origin, struct logical_volume *cow,
      union lvid *lvid, uint32_t extent_count,
      uint32_t chunk_size);

int vg_remove_snapshot(struct logical_volume *cow);

int vg_check_status(const struct volume_group *vg, uint64_t status);





int vg_max_lv_reached(struct volume_group *vg);




struct lv_segment *find_mirror_seg(struct lv_segment *seg);
int lv_add_mirrors(struct cmd_context *cmd, struct logical_volume *lv,
     uint32_t mirrors, uint32_t stripes, uint32_t stripe_size,
     uint32_t region_size, uint32_t log_count,
     struct dm_list *pvs, alloc_policy_t alloc, uint32_t flags);
int lv_split_mirror_images(struct logical_volume *lv, const char *split_lv_name,
      uint32_t split_count, struct dm_list *removable_pvs);
int lv_remove_mirrors(struct cmd_context *cmd, struct logical_volume *lv,
        uint32_t mirrors, uint32_t log_count,
        int (*is_removable)(struct logical_volume *, void *),
        void *removable_baton, uint64_t status_mask);

int is_temporary_mirror_layer(const struct logical_volume *lv);
struct logical_volume * find_temporary_mirror(const struct logical_volume *lv);
int lv_is_mirrored(const struct logical_volume *lv);
uint32_t lv_mirror_count(const struct logical_volume *lv);
uint32_t adjusted_mirror_region_size(uint32_t extent_size, uint32_t extents,
                                    uint32_t region_size);
int remove_mirrors_from_segments(struct logical_volume *lv,
     uint32_t new_mirrors, uint64_t status_mask);
int add_mirrors_to_segments(struct cmd_context *cmd, struct logical_volume *lv,
       uint32_t mirrors, uint32_t region_size,
       struct dm_list *allocatable_pvs, alloc_policy_t alloc);

int remove_mirror_images(struct logical_volume *lv, uint32_t num_mirrors,
    int (*is_removable)(struct logical_volume *, void *),
    void *removable_baton, unsigned remove_log);
int add_mirror_images(struct cmd_context *cmd, struct logical_volume *lv,
        uint32_t mirrors, uint32_t stripes, uint32_t stripe_size, uint32_t region_size,
        struct dm_list *allocatable_pvs, alloc_policy_t alloc,
        uint32_t log_count);
struct logical_volume *detach_mirror_log(struct lv_segment *seg);
int attach_mirror_log(struct lv_segment *seg, struct logical_volume *lv);
int remove_mirror_log(struct cmd_context *cmd, struct logical_volume *lv,
        struct dm_list *removable_pvs, int force);
int add_mirror_log(struct cmd_context *cmd, struct logical_volume *lv,
     uint32_t log_count, uint32_t region_size,
     struct dm_list *allocatable_pvs, alloc_policy_t alloc);

int reconfigure_mirror_images(struct lv_segment *mirrored_seg, uint32_t num_mirrors,
         struct dm_list *removable_pvs, unsigned remove_log);
int collapse_mirrored_lv(struct logical_volume *lv);
int shift_mirror_images(struct lv_segment *mirrored_seg, unsigned mimage);


int replicator_add_replicator_dev(struct logical_volume *replicator_lv,
      struct lv_segment *rdev_seg);
struct logical_volume *replicator_remove_replicator_dev(struct lv_segment *rdev_seg);
int replicator_add_rlog(struct lv_segment *replicator_seg, struct logical_volume *rlog_lv);
struct logical_volume *replicator_remove_rlog(struct lv_segment *replicator_seg);

int replicator_dev_add_slog(struct replicator_device *rdev, struct logical_volume *slog_lv);
struct logical_volume *replicator_dev_remove_slog(struct replicator_device *rdev);
int replicator_dev_add_rimage(struct replicator_device *rdev, struct logical_volume *lv);
struct logical_volume *replicator_dev_remove_rimage(struct replicator_device *rdev);

int lv_is_active_replicator_dev(const struct logical_volume *lv);
int lv_is_replicator(const struct logical_volume *lv);
int lv_is_replicator_dev(const struct logical_volume *lv);
int lv_is_rimage(const struct logical_volume *lv);
int lv_is_rlog(const struct logical_volume *lv);
int lv_is_slog(const struct logical_volume *lv);
struct logical_volume *first_replicator_dev(const struct logical_volume *lv);

struct cmd_vg *cmd_vg_add(struct dm_pool *mem, struct dm_list *cmd_vgs,
     const char *vg_name, const char *vgid,
     uint32_t flags);
struct cmd_vg *cmd_vg_lookup(struct dm_list *cmd_vgs,
        const char *vg_name, const char *vgid);
int cmd_vg_read(struct cmd_context *cmd, struct dm_list *cmd_vgs);
void free_cmd_vgs(struct dm_list *cmd_vgs);

int find_replicator_vgs(struct logical_volume *lv);

int lv_read_replicator_vgs(struct logical_volume *lv);
void lv_release_replicator_vgs(struct logical_volume *lv);

struct logical_volume *find_pvmove_lv(struct volume_group *vg,
          struct device *dev, uint32_t lv_type);
struct logical_volume *find_pvmove_lv_from_pvname(struct cmd_context *cmd,
        struct volume_group *vg,
        const char *name,
        const char *uuid,
        uint32_t lv_type);
const char *get_pvmove_pvname_from_lv(struct logical_volume *lv);
const char *get_pvmove_pvname_from_lv_mirr(struct logical_volume *lv_mirr);
percent_t copy_percent(const struct logical_volume *lv_mirr);
struct dm_list *lvs_using_lv(struct cmd_context *cmd, struct volume_group *vg,
     struct logical_volume *lv);

uint32_t find_free_lvnum(struct logical_volume *lv);
char *generate_lv_name(struct volume_group *vg, const char *format,
         char *buffer, size_t len);




int pv_change_metadataignore(struct physical_volume *pv, uint32_t mda_ignore);


int vg_check_write_mode(struct volume_group *vg);




int lv_has_unknown_segments(const struct logical_volume *lv);
int vg_has_unknown_segments(const struct volume_group *vg);

struct vgcreate_params {
 const char *vg_name;
 uint32_t extent_size;
 size_t max_pv;
 size_t max_lv;
 alloc_policy_t alloc;
 int clustered;
 uint32_t vgmetadatacopies;
};

int vgcreate_params_validate(struct cmd_context *cmd,
        struct vgcreate_params *vp);

int validate_vg_rename_params(struct cmd_context *cmd,
         const char *vg_name_old,
         const char *vg_name_new);
/* # 28 "../include/metadata.h" 2 */
/* # 103 "../include/metadata.h" */
struct config_tree;
struct metadata_area;


struct metadata_area_ops {
 struct volume_group *(*vg_read) (struct format_instance * fi,
      const char *vg_name,
      struct metadata_area * mda);
 struct volume_group *(*vg_read_precommit) (struct format_instance * fi,
      const char *vg_name,
      struct metadata_area * mda);
/* # 128 "../include/metadata.h" */
 int (*vg_write) (struct format_instance * fid, struct volume_group * vg,
    struct metadata_area * mda);
 int (*vg_precommit) (struct format_instance * fid,
        struct volume_group * vg,
        struct metadata_area * mda);
 int (*vg_commit) (struct format_instance * fid,
     struct volume_group * vg, struct metadata_area * mda);
 int (*vg_revert) (struct format_instance * fid,
     struct volume_group * vg, struct metadata_area * mda);
 int (*vg_remove) (struct format_instance * fi, struct volume_group * vg,
     struct metadata_area * mda);




 void *(*mda_metadata_locn_copy) (struct dm_pool *mem, void *metadata_locn);




 const char *(*mda_metadata_locn_name) (void *metadata_locn);
 uint64_t (*mda_metadata_locn_offset) (void *metadata_locn);




 uint64_t (*mda_free_sectors) (struct metadata_area *mda);




 uint64_t (*mda_total_sectors) (struct metadata_area *mda);




 int (*mda_in_vg) (struct format_instance * fi,
       struct volume_group * vg, struct metadata_area *mda);



 int (*pv_analyze_mda) (const struct format_type * fmt,
          struct metadata_area *mda);





 unsigned (*mda_locns_match)(struct metadata_area *mda1,
        struct metadata_area *mda2);
};



struct metadata_area {
 struct dm_list list;
 struct metadata_area_ops *ops;
 void *metadata_locn;
 uint32_t status;
};
struct metadata_area *mda_copy(struct dm_pool *mem,
          struct metadata_area *mda);

unsigned mda_is_ignored(struct metadata_area *mda);
void mda_set_ignored(struct metadata_area *mda, unsigned ignored);
unsigned mda_locns_match(struct metadata_area *mda1, struct metadata_area *mda2);

struct format_instance_ctx {
 uint32_t type;
 union {
  const char *pv_id;
  struct {
   const char *vg_name;
   const char *vg_id;
  } vg_ref;
  void *private;
 } context;
};

struct format_instance *alloc_fid(const struct format_type *fmt,
      const struct format_instance_ctx *fic);






void pv_set_fid(struct physical_volume *pv, struct format_instance *fid);
void vg_set_fid(struct volume_group *vg, struct format_instance *fid);


int fid_add_mda(struct format_instance *fid, struct metadata_area *mda,
  const char *key, size_t key_len, const unsigned sub_key);
int fid_add_mdas(struct format_instance *fid, struct dm_list *mdas,
   const char *key, size_t key_len);
int fid_remove_mda(struct format_instance *fid, struct metadata_area *mda,
     const char *key, size_t key_len, const unsigned sub_key);
struct metadata_area *fid_get_mda_indexed(struct format_instance *fid,
  const char *key, size_t key_len, const unsigned sub_key);
int mdas_empty_or_ignored(struct dm_list *mdas);






struct name_list {
 struct dm_list list;
 char *name;
};

struct mda_list {
 struct dm_list list;
 struct device_area mda;
};

struct peg_list {
 struct dm_list list;
 struct pv_segment *peg;
};

struct seg_list {
 struct dm_list list;
 unsigned count;
 struct lv_segment *seg;
};




struct format_handler {



 int (*scan) (const struct format_type * fmt, const char *vgname);




 int (*pv_read) (const struct format_type * fmt, const char *pv_name,
   struct physical_volume * pv, int scan_label_only);




 int (*pv_initialise) (const struct format_type * fmt,
         int64_t label_sector,
         uint64_t pe_start,
         uint32_t extent_count,
         uint32_t extent_size,
         unsigned long data_alignment,
         unsigned long data_alignment_offset,
         struct physical_volume * pv);





 int (*pv_setup) (const struct format_type * fmt,
    struct physical_volume * pv,
    struct volume_group * vg);




 int (*pv_add_metadata_area) (const struct format_type * fmt,
         struct physical_volume * pv,
         int pe_start_locked,
         unsigned metadata_index,
         uint64_t metadata_size,
         unsigned metadata_ignored);




 int (*pv_remove_metadata_area) (const struct format_type *fmt,
     struct physical_volume *pv,
     unsigned metadata_index);




 int (*pv_resize) (const struct format_type *fmt,
     struct physical_volume *pv,
     struct volume_group *vg,
     uint64_t size);





 int (*pv_write) (const struct format_type * fmt,
    struct physical_volume * pv);





 int (*lv_setup) (struct format_instance * fi,
    struct logical_volume * lv);





 int (*vg_setup) (struct format_instance * fi, struct volume_group * vg);




 int (*segtype_supported) (struct format_instance *fid,
      const struct segment_type *segtype);




 struct format_instance *(*create_instance) (const struct format_type *fmt,
          const struct format_instance_ctx *fic);




 void (*destroy_instance) (struct format_instance * fid);




 void (*destroy) (struct format_type * fmt);
};




unsigned long set_pe_align(struct physical_volume *pv, unsigned long data_alignment);
unsigned long set_pe_align_offset(struct physical_volume *pv,
      unsigned long data_alignment_offset);
int vg_validate(struct volume_group *vg);

int pv_write_orphan(struct cmd_context *cmd, struct physical_volume *pv);


int pv_add(struct volume_group *vg, struct physical_volume *pv);
int pv_remove(struct volume_group *vg, struct physical_volume *pv);
struct physical_volume *pv_find(struct volume_group *vg, const char *pv_name);


int get_pv_from_vg_by_id(const struct format_type *fmt, const char *vg_name,
    const char *vgid, const char *pvid,
    struct physical_volume *pv);

struct lv_list *find_lv_in_vg_by_lvid(struct volume_group *vg,
          const union lvid *lvid);

struct lv_list *find_lv_in_lv_list(const struct dm_list *ll,
       const struct logical_volume *lv);



struct volume_group *find_vg_with_lv(const char *lv_name);


struct logical_volume *lv_from_lvid(struct cmd_context *cmd,
        const char *lvid_s,
        unsigned precommitted);


struct physical_volume *find_pv(struct volume_group *vg, struct device *dev);

struct pv_list *find_pv_in_pv_list(const struct dm_list *pl,
       const struct physical_volume *pv);


struct lv_segment *find_seg_by_le(const struct logical_volume *lv, uint32_t le);




const char *strip_dir(const char *vg_name, const char *dir);

struct logical_volume *alloc_lv(struct dm_pool *mem);





int check_lv_segments(struct logical_volume *lv, int complete_vg);





int check_replicator_segment(const struct lv_segment *replicator_seg);






int lv_merge_segments(struct logical_volume *lv);




int lv_split_segment(struct logical_volume *lv, uint32_t le);





int add_seg_to_segs_using_this_lv(struct logical_volume *lv, struct lv_segment *seg);
int remove_seg_from_segs_using_this_lv(struct logical_volume *lv, struct lv_segment *seg);
struct lv_segment *get_only_segment_using_this_lv(struct logical_volume *lv);




void lv_calculate_readahead(const struct logical_volume *lv, uint32_t *read_ahead);




int export_vg_to_buffer(struct volume_group *vg, char **buf);
struct volume_group *import_vg_from_buffer(const char *buf,
        struct format_instance *fid);
struct volume_group *import_vg_from_config_tree(const struct config_tree *cft,
      struct format_instance *fid);
/* # 462 "../include/metadata.h" */
int fixup_imported_mirrors(struct volume_group *vg);




struct id pv_id(const struct physical_volume *pv);
const struct format_type *pv_format_type(const struct physical_volume *pv);
struct id pv_vgid(const struct physical_volume *pv);

struct physical_volume *pv_by_path(struct cmd_context *cmd, const char *pv_name);
int add_pv_to_vg(struct volume_group *vg, const char *pv_name,
   struct physical_volume *pv);
int vg_mark_partial_lvs(struct volume_group *vg);
int is_mirror_image_removable(struct logical_volume *mimage_lv, void *baton);

uint64_t find_min_mda_size(struct dm_list *mdas);
char *tags_format_and_copy(struct dm_pool *mem, const struct dm_list *tags);
/* # 22 "cache/lvmcache.c" 2 */
/* # 1 "../include/filter.h" 1 */
/* # 33 "../include/filter.h" */
struct dev_filter *lvm_type_filter_create(const char *proc,
       const struct config_node *cn);

void lvm_type_filter_destroy(struct dev_filter *f);

int dm_major(void);
int md_major(void);
int blkext_major(void);
int max_partitions(int major);

int dev_subsystem_part_major(const struct device *dev);
const char *dev_subsystem_name(const struct device *dev);
/* # 23 "cache/lvmcache.c" 2 */
/* # 1 "../include/filter-persistent.h" 1 */
/* # 21 "../include/filter-persistent.h" */
struct dev_filter *persistent_filter_create(struct dev_filter *f,
         const char *file);

int persistent_filter_wipe(struct dev_filter *f);
int persistent_filter_load(struct dev_filter *f, struct config_tree **cft_out);
int persistent_filter_dump(struct dev_filter *f, int merge_existing);
/* # 24 "cache/lvmcache.c" 2 */
/* # 1 "../include/memlock.h" 1 */
/* # 19 "../include/memlock.h" */
struct cmd_context;

void critical_section_inc(struct cmd_context *cmd);
void critical_section_dec(struct cmd_context *cmd);
int critical_section(void);
void memlock_inc_daemon(struct cmd_context *cmd);
void memlock_dec_daemon(struct cmd_context *cmd);
void memlock_init(struct cmd_context *cmd);
void memlock_reset(void);
void memlock_unlock(struct cmd_context *cmd);
/* # 25 "cache/lvmcache.c" 2 */
/* # 1 "../include/str_list.h" 1 */
/* # 19 "../include/str_list.h" */
struct dm_list *str_list_create(struct dm_pool *mem);
int str_list_add(struct dm_pool *mem, struct dm_list *sll, const char *str);
int str_list_del(struct dm_list *sll, const char *str);
int str_list_match_item(const struct dm_list *sll, const char *str);
int str_list_match_list(const struct dm_list *sll, const struct dm_list *sll2, const char **tag_matched);
int str_list_lists_equal(const struct dm_list *sll, const struct dm_list *sll2);
int str_list_dup(struct dm_pool *mem, struct dm_list *sllnew,
   const struct dm_list *sllold);
/* # 26 "cache/lvmcache.c" 2 */
/* # 1 "../include/format-text.h" 1 */
/* # 20 "../include/format-text.h" */
/* # 1 "../include/metadata.h" 1 */
/* # 21 "../include/format-text.h" 2 */
/* # 33 "../include/format-text.h" */
int archive_vg(struct volume_group *vg,
        const char *dir,
        const char *desc, uint32_t retain_days, uint32_t min_archive);




int archive_list(struct cmd_context *cmd, const char *dir, const char *vgname);
int archive_list_file(struct cmd_context *cmd, const char *file);
int backup_list(struct cmd_context *cmd, const char *dir, const char *vgname);




struct text_context {
 const char *path_live;
 const char *path_edit;
 const char *desc;
};
struct format_type *create_text_format(struct cmd_context *cmd);

struct labeller *text_labeller_create(const struct format_type *fmt);

int pvhdr_read(struct device *dev, char *buf);

int add_da(struct dm_pool *mem, struct dm_list *das,
    uint64_t start, uint64_t size);
void del_das(struct dm_list *das);
int add_mda(const struct format_type *fmt, struct dm_pool *mem, struct dm_list *mdas,
     struct device *dev, uint64_t start, uint64_t size, unsigned ignored);
void del_mdas(struct dm_list *mdas);
/* # 27 "cache/lvmcache.c" 2 */
/* # 1 "../include/format_pool.h" 1 */
/* # 25 "../include/format_pool.h" */
struct format_type *init_pool_format(struct cmd_context *cmd);
/* # 28 "cache/lvmcache.c" 2 */
/* # 1 "../include/format1.h" 1 */
/* # 20 "../include/format1.h" */
/* # 1 "../include/lvmcache.h" 1 */
/* # 21 "../include/format1.h" 2 */





struct format_type *init_lvm1_format(struct cmd_context *cmd);
/* # 29 "cache/lvmcache.c" 2 */
/* # 1 "../include/config.h" 1 */
/* # 30 "cache/lvmcache.c" 2 */

static struct dm_hash_table *_pvid_hash = ((void *)0);
static struct dm_hash_table *_vgid_hash = ((void *)0);
static struct dm_hash_table *_vgname_hash = ((void *)0);
static struct dm_hash_table *_lock_hash = ((void *)0);
static struct dm_list _vginfos = { &(_vginfos), &(_vginfos) };
static int _scanning_in_progress = 0;
static int _has_scanned = 0;
static int _vgs_locked = 0;
static int _vg_global_lock_held = 0;

int lvmcache_init(void)
{




 _vgs_locked = 0;

 dm_list_init(&_vginfos);

 if (!(_vgname_hash = dm_hash_create(128)))
  return 0;

 if (!(_vgid_hash = dm_hash_create(128)))
  return 0;

 if (!(_pvid_hash = dm_hash_create(128)))
  return 0;

 if (!(_lock_hash = dm_hash_create(128)))
  return 0;






 if (_vg_global_lock_held) {
  lvmcache_lock_vgname("#global", 0);
  _vg_global_lock_held = 0;
 }

 return 1;
}


static void _free_cached_vgmetadata(struct lvmcache_vginfo *vginfo)
{
 if (!vginfo || !vginfo->vgmetadata)
  return;

 free(vginfo->vgmetadata);

 vginfo->vgmetadata = ((void *)0);


 if (vginfo->cft) {
  destroy_config_tree(vginfo->cft);
  vginfo->cft = ((void *)0);
 }

 print_log(7, "cache/lvmcache.c", 92 , 0,"Metadata cache: VG %s wiped.", vginfo->vgname);
}




static void _store_metadata(struct volume_group *vg, unsigned precommitted)
{
 char uuid[64] __attribute__((aligned(8)));
 struct lvmcache_vginfo *vginfo;
 int size;

 if (!(vginfo = vginfo_from_vgid((const char *)&vg->id))) {
  print_log(7, "cache/lvmcache.c", 105 , 0,"<backtrace>");
  return;
 }

 if (vginfo->vgmetadata)
  _free_cached_vgmetadata(vginfo);

 if (!(size = export_vg_to_buffer(vg, &vginfo->vgmetadata))) {
  print_log(7, "cache/lvmcache.c", 113 , 0,"<backtrace>");
  return;
 }

 vginfo->precommitted = precommitted;

 if (!id_write_format((const struct id *)vginfo->vgid, uuid, sizeof(uuid))) {
  print_log(7, "cache/lvmcache.c", 120 , 0,"<backtrace>");
  return;
 }

 print_log(7,

 "cache/lvmcache.c"
/* # 124 "cache/lvmcache.c" */
 ,

 126
/* # 124 "cache/lvmcache.c" */
 , 0,"Metadata cache: VG %s (%s) stored (%d bytes%s).", vginfo->vgname, uuid, size, precommitted ? ", precommitted" : "")

                                         ;
}

static void _update_cache_info_lock_state(struct lvmcache_info *info,
       int locked,
       int *cached_vgmetadata_valid)
{
 int was_locked = (info->status & 0x00000002) ? 1 : 0;





 if (!vgname_is_locked("#global") && (was_locked != locked)) {
  info->status |= 0x00000001;
  *cached_vgmetadata_valid = 0;
 }

 if (locked)
  info->status |= 0x00000002;
 else
  info->status &= ~0x00000002;
}

static void _update_cache_vginfo_lock_state(struct lvmcache_vginfo *vginfo,
         int locked)
{
 struct lvmcache_info *info;
 int cached_vgmetadata_valid = 1;

 for (info = ((__typeof__(*info) *)((const char *)(((&vginfo->infos))->n) - (const char *)&((__typeof__(*info) *) 0)->list)); &info->list != ((&vginfo->infos)); info = ((__typeof__(*info) *)((const char *)(info->list.n) - (const char *)&((__typeof__(*info) *) 0)->list)))
  _update_cache_info_lock_state(info, locked,
           &cached_vgmetadata_valid);

 if (!cached_vgmetadata_valid)
  _free_cached_vgmetadata(vginfo);
}

static void _update_cache_lock_state(const char *vgname, int locked)
{
 struct lvmcache_vginfo *vginfo;

 if (!(vginfo = vginfo_from_vgname(vgname, ((void *)0))))
  return;

 _update_cache_vginfo_lock_state(vginfo, locked);
}

static void _drop_metadata(const char *vgname, int drop_precommitted)
{
 struct lvmcache_vginfo *vginfo;
 struct lvmcache_info *info;

 if (!(vginfo = vginfo_from_vgname(vgname, ((void *)0))))
  return;







 if (!drop_precommitted && vginfo->precommitted && !vginfo->vgmetadata)
  print_log(3,
 "cache/lvmcache.c"
/* # 189 "cache/lvmcache.c" */
  ,
 190
/* # 189 "cache/lvmcache.c" */
  , -1,"Internal error: " "metadata commit (or revert) missing before " "dropping metadata from cache.")
                                     ;

 if (drop_precommitted || !vginfo->precommitted)
  for (info = ((__typeof__(*info) *)((const char *)(((&vginfo->infos))->n) - (const char *)&((__typeof__(*info) *) 0)->list)); &info->list != ((&vginfo->infos)); info = ((__typeof__(*info) *)((const char *)(info->list.n) - (const char *)&((__typeof__(*info) *) 0)->list)))
   info->status |= 0x00000001;

 _free_cached_vgmetadata(vginfo);
}






void lvmcache_commit_metadata(const char *vgname)
{
 struct lvmcache_vginfo *vginfo;

 if (!(vginfo = vginfo_from_vgname(vgname, ((void *)0))))
  return;

 if (vginfo->precommitted) {
  print_log(7,
 "cache/lvmcache.c"
/* # 212 "cache/lvmcache.c" */
  ,
 213
/* # 212 "cache/lvmcache.c" */
  , 0,"Precommitted metadata cache: VG %s upgraded to committed.", vginfo->vgname)
                    ;
  vginfo->precommitted = 0;
 }
}

void lvmcache_drop_metadata(const char *vgname, int drop_precommitted)
{

 if (!strcmp(vgname, "#orphans")) {
  _drop_metadata("#orphans" "_" "lvm2", 0);
  _drop_metadata("#orphans" "_" "lvm1", 0);
  _drop_metadata("#orphans" "_" "pool", 0);


  init_full_scan_done(0);
 } else if (!vgname_is_locked("#global"))
  _drop_metadata(vgname, drop_precommitted);
}






static int _vgname_order_correct(const char *vgname1, const char *vgname2)
{
 if (is_global_vg(vgname1))
  return 1;

 if (is_global_vg(vgname2))
  return 0;

 if (is_orphan_vg(vgname1))
  return 0;

 if (is_orphan_vg(vgname2))
  return 1;

 if (strcmp(vgname1, vgname2) < 0)
  return 1;

 return 0;
}




int lvmcache_verify_lock_order(const char *vgname)
{
 struct dm_hash_node *n;
 const char *vgname2;

 if (!_lock_hash)
  do { print_log(7, "cache/lvmcache.c", 266 , 0,"<backtrace>"); return 0; } while (0);

 for (n = dm_hash_get_first((_lock_hash)); n; n = dm_hash_get_next((_lock_hash), n)) {
  if (!dm_hash_get_data(_lock_hash, n))
   do { print_log(7, "cache/lvmcache.c", 270 , 0,"<backtrace>"); return 0; } while (0);

  vgname2 = dm_hash_get_key(_lock_hash, n);

  if (!_vgname_order_correct(vgname2, vgname)) {
   print_log(3,

 "cache/lvmcache.c"
/* # 275 "cache/lvmcache.c" */
   ,

 277
/* # 275 "cache/lvmcache.c" */
   , 35,"Internal error: " "VG lock %s must " "be requested before %s, not after.", vgname, vgname2)

                      ;
   do { print_log(7, "cache/lvmcache.c", 278 , 0,"<backtrace>"); return 0; } while (0);
  }
 }

 return 1;
}

void lvmcache_lock_vgname(const char *vgname, int read_only __attribute__((unused)))
{
 if (!_lock_hash && !lvmcache_init()) {
  print_log(3, "cache/lvmcache.c", 288 , -1,"Internal cache initialisation failed");
  return;
 }

 if (dm_hash_lookup(_lock_hash, vgname))
  print_log(3,
 "cache/lvmcache.c"
/* # 293 "cache/lvmcache.c" */
  ,
 294
/* # 293 "cache/lvmcache.c" */
  , -1,"Internal error: " "Nested locking attempted on VG %s.", vgname)
            ;

 if (!dm_hash_insert(_lock_hash, vgname, (void *) 1))
  print_log(3, "cache/lvmcache.c", 297 , -1,"Cache locking failure for %s", vgname);

 _update_cache_lock_state(vgname, 1);

 if (strcmp(vgname, "#global"))
  _vgs_locked++;
}

int vgname_is_locked(const char *vgname)
{
 if (!_lock_hash)
  return 0;

 return dm_hash_lookup(_lock_hash, is_orphan_vg(vgname) ? "#orphans" : vgname) ? 1 : 0;
}

void lvmcache_unlock_vgname(const char *vgname)
{
 if (!dm_hash_lookup(_lock_hash, vgname))
  print_log(3,
 "cache/lvmcache.c"
/* # 316 "cache/lvmcache.c" */
  ,
 317
/* # 316 "cache/lvmcache.c" */
  , -1,"Internal error: " "Attempt to unlock unlocked VG %s.", vgname)
            ;

 _update_cache_lock_state(vgname, 0);

 dm_hash_remove(_lock_hash, vgname);


 if (strcmp(vgname, "#global") && !--_vgs_locked)
  dev_close_all();
}

int vgs_locked(void)
{
 return _vgs_locked;
}

static void _vginfo_attach_info(struct lvmcache_vginfo *vginfo,
    struct lvmcache_info *info)
{
 if (!vginfo)
  return;

 info->vginfo = vginfo;
 dm_list_add(&vginfo->infos, &info->list);
}

static void _vginfo_detach_info(struct lvmcache_info *info)
{
 if (!dm_list_empty(&info->list)) {
  dm_list_del(&info->list);
  dm_list_init(&info->list);
 }

 info->vginfo = ((void *)0);
}


struct lvmcache_vginfo *vginfo_from_vgname(const char *vgname, const char *vgid)
{
 struct lvmcache_vginfo *vginfo;

 if (!vgname)
  return vginfo_from_vgid(vgid);

 if (!_vgname_hash)
  return ((void *)0);

 if (!(vginfo = dm_hash_lookup(_vgname_hash, vgname)))
  return ((void *)0);

 if (vgid)
  do
   if (!strncmp(vgid, vginfo->vgid, 32))
    return vginfo;
  while ((vginfo = vginfo->next));

 return vginfo;
}

const struct format_type *fmt_from_vgname(const char *vgname, const char *vgid, unsigned revalidate_labels)
{
 struct lvmcache_vginfo *vginfo;
 struct lvmcache_info *info;
 struct label *label;
 struct dm_list *devh, *tmp;
 struct dm_list devs;
 struct device_list *devl;
 char vgid_found[32 + 1] __attribute__((aligned(8)));

 if (!(vginfo = vginfo_from_vgname(vgname, vgid)))
  return ((void *)0);




 if (!revalidate_labels)
  goto out;





 dm_list_init(&devs);
 for (info = ((__typeof__(*info) *)((const char *)(((&vginfo->infos))->n) - (const char *)&((__typeof__(*info) *) 0)->list)); &info->list != ((&vginfo->infos)); info = ((__typeof__(*info) *)((const char *)(info->list.n) - (const char *)&((__typeof__(*info) *) 0)->list))) {
  if (!(devl = dm_malloc_aux((sizeof(*devl)), "cache/lvmcache.c", 401))) {
   print_log(3, "cache/lvmcache.c", 402 , -1,"device_list element allocation failed");
   return ((void *)0);
  }
  devl->dev = info->dev;
  dm_list_add(&devs, &devl->list);
 }

 memcpy(vgid_found, vginfo->vgid, sizeof(vgid_found));

 for (devh = (&devs)->n, tmp = devh->n; devh != &devs; devh = tmp, tmp = devh->n) {
  devl = ((struct device_list *)((const char *)((devh)) - (const char *)&((struct device_list *) 0)->list));
  label_read(devl->dev, &label, 0ULL);
  dm_list_del(&devl->list);
  free(devl);
 }


 if (!(vginfo = vginfo_from_vgname(vgname, vgid_found)) ||
     strncmp(vginfo->vgid, vgid_found, 32))
  return ((void *)0);

out:
 return vginfo->fmt;
}

struct lvmcache_vginfo *vginfo_from_vgid(const char *vgid)
{
 struct lvmcache_vginfo *vginfo;
 char id[32 + 1] __attribute__((aligned(8)));

 if (!_vgid_hash || !vgid)
  return ((void *)0);


 strncpy(&id[0], vgid, 32);
 id[32] = '\0';

 if (!(vginfo = dm_hash_lookup(_vgid_hash, id)))
  return ((void *)0);

 return vginfo;
}

const char *vgname_from_vgid(struct dm_pool *mem, const char *vgid)
{
 struct lvmcache_vginfo *vginfo;
 const char *vgname = ((void *)0);

 if ((vginfo = vginfo_from_vgid(vgid)))
  vgname = vginfo->vgname;

 if (mem && vgname)
  return dm_pool_strdup(mem, vgname);

 return vgname;
}

static int _info_is_valid(struct lvmcache_info *info)
{
 if (info->status & 0x00000001)
  return 0;
/* # 471 "cache/lvmcache.c" */
 if (info->vginfo && !vgname_is_locked(info->vginfo->vgname))
  return 1;

 if (!(info->status & 0x00000002))
  return 0;

 return 1;
}

static int _vginfo_is_valid(struct lvmcache_vginfo *vginfo)
{
 struct lvmcache_info *info;


 for (info = ((__typeof__(*info) *)((const char *)(((&vginfo->infos))->n) - (const char *)&((__typeof__(*info) *) 0)->list)); &info->list != ((&vginfo->infos)); info = ((__typeof__(*info) *)((const char *)(info->list.n) - (const char *)&((__typeof__(*info) *) 0)->list)))
  if (!_info_is_valid(info))
   return 0;

 return 1;
}


static int _vginfo_is_invalid(struct lvmcache_vginfo *vginfo)
{
 struct lvmcache_info *info;

 for (info = ((__typeof__(*info) *)((const char *)(((&vginfo->infos))->n) - (const char *)&((__typeof__(*info) *) 0)->list)); &info->list != ((&vginfo->infos)); info = ((__typeof__(*info) *)((const char *)(info->list.n) - (const char *)&((__typeof__(*info) *) 0)->list)))
  if (_info_is_valid(info))
   return 0;

 return 1;
}





struct lvmcache_info *info_from_pvid(const char *pvid, int valid_only)
{
 struct lvmcache_info *info;
 char id[32 + 1] __attribute__((aligned(8)));

 if (!_pvid_hash || !pvid)
  return ((void *)0);

 strncpy(&id[0], pvid, 32);
 id[32] = '\0';

 if (!(info = dm_hash_lookup(_pvid_hash, id)))
  return ((void *)0);

 if (valid_only && !_info_is_valid(info))
  return ((void *)0);

 return info;
}

char *lvmcache_vgname_from_pvid(struct cmd_context *cmd, const char *pvid)
{
 struct lvmcache_info *info;
 char *vgname;

 if (!device_from_pvid(cmd, (const struct id *)pvid, ((void *)0))) {
  print_log(3, "cache/lvmcache.c", 534 , -1,"Couldn't find device with uuid %s.", pvid);
  return ((void *)0);
 }

 info = info_from_pvid(pvid, 0);
 if (!info)
  do { print_log(7, "cache/lvmcache.c", 540 , 0,"<backtrace>"); return ((void *)0); } while (0);

 if (!(vgname = dm_pool_strdup(cmd->mem, info->vginfo->vgname))) {
  print_log(3, "cache/lvmcache.c", 543 , 12, "vgname allocation failed");
  return ((void *)0);
 }
 return vgname;
}

static void _rescan_entry(struct lvmcache_info *info)
{
 struct label *label;

 if (info->status & 0x00000001)
  label_read(info->dev, &label, 0ULL);
}

static int _scan_invalid(void)
{
 dm_hash_iter(_pvid_hash, (dm_hash_iterate_fn) _rescan_entry);

 return 1;
}

int lvmcache_label_scan(struct cmd_context *cmd, int full_scan)
{
 struct label *label;
 struct dev_iter *iter;
 struct device *dev;
 struct format_type *fmt;

 int r = 0;


 if (_scanning_in_progress)
  return 0;

 _scanning_in_progress = 1;

 if (!_vgname_hash && !lvmcache_init()) {
  print_log(3, "cache/lvmcache.c", 580 , -1,"Internal cache initialisation failed");
  goto out;
 }

 if (_has_scanned && !full_scan) {
  r = _scan_invalid();
  goto out;
 }

 if (full_scan == 2 && !cmd->filter->use_count && !refresh_filters(cmd)) {
  print_log(3, "cache/lvmcache.c", 590 , -1,"refresh filters failed");
  goto out;
 }

 if (!(iter = dev_iter_create(cmd->filter, (full_scan == 2) ? 1 : 0))) {
  print_log(3, "cache/lvmcache.c", 595 , -1,"dev_iter creation failed");
  goto out;
 }

 while ((dev = dev_iter_get(iter)))
  label_read(dev, &label, 0ULL);

 dev_iter_destroy(iter);

 _has_scanned = 1;


 if (cmd->independent_metadata_areas)
  for (fmt = ((__typeof__(*fmt) *)((const char *)(((&cmd->formats))->n) - (const char *)&((__typeof__(*fmt) *) 0)->list)); &fmt->list != ((&cmd->formats)); fmt = ((__typeof__(*fmt) *)((const char *)(fmt->list.n) - (const char *)&((__typeof__(*fmt) *) 0)->list)))
   if (fmt->ops->scan && !fmt->ops->scan(fmt, ((void *)0)))
    goto out;





 if (full_scan == 2 && cmd->is_long_lived && cmd->dump_filter)
  persistent_filter_dump(cmd->filter, 0);

 r = 1;

      out:
 _scanning_in_progress = 0;

 return r;
}

struct volume_group *lvmcache_get_vg(const char *vgid, unsigned precommitted)
{
 struct lvmcache_vginfo *vginfo;
 struct volume_group *vg = ((void *)0);
 struct format_instance *fid;
 struct format_instance_ctx fic;

 if (!vgid || !(vginfo = vginfo_from_vgid(vgid)) || !vginfo->vgmetadata)
  return ((void *)0);

 if (!_vginfo_is_valid(vginfo))
  return ((void *)0);
/* # 652 "cache/lvmcache.c" */
 if ((precommitted && !vginfo->precommitted) ||
     (!precommitted && vginfo->precommitted && !critical_section()))
  return ((void *)0);

 fic.type = 0x00000001U | 0x00000002U | 0x00000004U;
 fic.context.vg_ref.vg_name = vginfo->vgname;
 fic.context.vg_ref.vg_id = vgid;
 if (!(fid = vginfo->fmt->ops->create_instance(vginfo->fmt, &fic)))
  do { print_log(7, "cache/lvmcache.c", 660 , 0,"<backtrace>"); return ((void *)0); } while (0);


 if (!vginfo->cft &&
     !(vginfo->cft =
       create_config_tree_from_string(fid->fmt->cmd,
          vginfo->vgmetadata)))
  do { print_log(7, "cache/lvmcache.c", 667 , 0,"<backtrace>"); goto bad; } while (0);

 if (!(vg = import_vg_from_config_tree(vginfo->cft, fid)))
  do { print_log(7, "cache/lvmcache.c", 670 , 0,"<backtrace>"); goto bad; } while (0);

 print_log(7,
 "cache/lvmcache.c"
/* # 672 "cache/lvmcache.c" */
 ,
 673
/* # 672 "cache/lvmcache.c" */
 , 0,"Using cached %smetadata for VG %s.", vginfo->precommitted ? "pre-committed" : "", vginfo->vgname)
                                                                ;

 return vg;

bad:
 free_vg(vg);
 _free_cached_vgmetadata(vginfo);
 return ((void *)0);
}

struct dm_list *lvmcache_get_vgids(struct cmd_context *cmd,
       int include_internal)
{
 struct dm_list *vgids;
 struct lvmcache_vginfo *vginfo;

 lvmcache_label_scan(cmd, 0);

 if (!(vgids = str_list_create(cmd->mem))) {
  print_log(3, "cache/lvmcache.c", 692 , -1,"vgids list allocation failed");
  return ((void *)0);
 }

 for (vginfo = ((__typeof__(*vginfo) *)((const char *)(((&_vginfos))->n) - (const char *)&((__typeof__(*vginfo) *) 0)->list)); &vginfo->list != ((&_vginfos)); vginfo = ((__typeof__(*vginfo) *)((const char *)(vginfo->list.n) - (const char *)&((__typeof__(*vginfo) *) 0)->list))) {
  if (!include_internal && is_orphan_vg(vginfo->vgname))
   continue;

  if (!str_list_add(cmd->mem, vgids,
      dm_pool_strdup(cmd->mem, vginfo->vgid))) {
   print_log(3, "cache/lvmcache.c", 702 , -1,"strlist allocation failed");
   return ((void *)0);
  }
 }

 return vgids;
}

struct dm_list *lvmcache_get_vgnames(struct cmd_context *cmd,
         int include_internal)
{
 struct dm_list *vgnames;
 struct lvmcache_vginfo *vginfo;

 lvmcache_label_scan(cmd, 0);

 if (!(vgnames = str_list_create(cmd->mem))) {
  print_log(3, "cache/lvmcache.c", 719 , 12, "vgnames list allocation failed");
  return ((void *)0);
 }

 for (vginfo = ((__typeof__(*vginfo) *)((const char *)(((&_vginfos))->n) - (const char *)&((__typeof__(*vginfo) *) 0)->list)); &vginfo->list != ((&_vginfos)); vginfo = ((__typeof__(*vginfo) *)((const char *)(vginfo->list.n) - (const char *)&((__typeof__(*vginfo) *) 0)->list))) {
  if (!include_internal && is_orphan_vg(vginfo->vgname))
   continue;

  if (!str_list_add(cmd->mem, vgnames,
      dm_pool_strdup(cmd->mem, vginfo->vgname))) {
   print_log(3, "cache/lvmcache.c", 729 , 12, "strlist allocation failed");
   return ((void *)0);
  }
 }

 return vgnames;
}

struct dm_list *lvmcache_get_pvids(struct cmd_context *cmd, const char *vgname,
    const char *vgid)
{
 struct dm_list *pvids;
 struct lvmcache_vginfo *vginfo;
 struct lvmcache_info *info;

 if (!(pvids = str_list_create(cmd->mem))) {
  print_log(3, "cache/lvmcache.c", 745 , -1,"pvids list allocation failed");
  return ((void *)0);
 }

 if (!(vginfo = vginfo_from_vgname(vgname, vgid)))
  return pvids;

 for (info = ((__typeof__(*info) *)((const char *)(((&vginfo->infos))->n) - (const char *)&((__typeof__(*info) *) 0)->list)); &info->list != ((&vginfo->infos)); info = ((__typeof__(*info) *)((const char *)(info->list.n) - (const char *)&((__typeof__(*info) *) 0)->list))) {
  if (!str_list_add(cmd->mem, pvids,
      dm_pool_strdup(cmd->mem, info->dev->pvid))) {
   print_log(3, "cache/lvmcache.c", 755 , -1,"strlist allocation failed");
   return ((void *)0);
  }
 }

 return pvids;
}

struct device *device_from_pvid(struct cmd_context *cmd, const struct id *pvid,
    unsigned *scan_done_once)
{
 struct label *label;
 struct lvmcache_info *info;


 if ((info = info_from_pvid((const char *) pvid, 0))) {
  if (label_read(info->dev, &label, 0ULL)) {
   info = (struct lvmcache_info *) label->info;
   if (id_equal(pvid, (struct id *) &info->dev->pvid))
    return info->dev;
  }
 }

 lvmcache_label_scan(cmd, 0);


 if ((info = info_from_pvid((const char *) pvid, 0))) {
  if (label_read(info->dev, &label, 0ULL)) {
   info = (struct lvmcache_info *) label->info;
   if (id_equal(pvid, (struct id *) &info->dev->pvid))
    return info->dev;
  }
 }

 if (critical_section() || (scan_done_once && *scan_done_once))
  return ((void *)0);

 lvmcache_label_scan(cmd, 2);
 if (scan_done_once)
  *scan_done_once = 1;


 if ((info = info_from_pvid((const char *) pvid, 0))) {
  if (label_read(info->dev, &label, 0ULL)) {
   info = (struct lvmcache_info *) label->info;
   if (id_equal(pvid, (struct id *) &info->dev->pvid))
    return info->dev;
  }
 }

 return ((void *)0);
}

const char *pvid_from_devname(struct cmd_context *cmd,
         const char *devname)
{
 struct device *dev;
 struct label *label;

 if (!(dev = dev_cache_get(devname, cmd->filter))) {
  print_log(3,
 "cache/lvmcache.c"
/* # 815 "cache/lvmcache.c" */
  ,
 816
/* # 815 "cache/lvmcache.c" */
  , -1,"%s: Couldn't find device.  Check your filters?", devname)
             ;
  return ((void *)0);
 }

 if (!(label_read(dev, &label, 0ULL)))
  return ((void *)0);

 return dev->pvid;
}


static int _free_vginfo(struct lvmcache_vginfo *vginfo)
{
 struct lvmcache_vginfo *primary_vginfo, *vginfo2;
 int r = 1;

 _free_cached_vgmetadata(vginfo);

 vginfo2 = primary_vginfo = vginfo_from_vgname(vginfo->vgname, ((void *)0));

 if (vginfo == primary_vginfo) {
  dm_hash_remove(_vgname_hash, vginfo->vgname);
  if (vginfo->next && !dm_hash_insert(_vgname_hash, vginfo->vgname,
          vginfo->next)) {
   print_log(3,
 "cache/lvmcache.c"
/* # 840 "cache/lvmcache.c" */
   ,
 841
/* # 840 "cache/lvmcache.c" */
   , -1,"_vgname_hash re-insertion for %s failed", vginfo->vgname)
                     ;
   r = 0;
  }
 } else do
  if (vginfo2->next == vginfo) {
   vginfo2->next = vginfo->next;
   break;
  }
  while ((vginfo2 = vginfo2->next));

 if (vginfo->vgname)
  free(vginfo->vgname);

 if (vginfo->creation_host)
  free(vginfo->creation_host);

 if (*vginfo->vgid && _vgid_hash &&
     vginfo_from_vgid(vginfo->vgid) == vginfo)
  dm_hash_remove(_vgid_hash, vginfo->vgid);

 dm_list_del(&vginfo->list);

 free(vginfo);

 return r;
}




static int _drop_vginfo(struct lvmcache_info *info, struct lvmcache_vginfo *vginfo)
{
 if (info)
  _vginfo_detach_info(info);


 if (!vginfo || is_orphan_vg(vginfo->vgname) ||
     !dm_list_empty(&vginfo->infos))
  return 1;

 if (!_free_vginfo(vginfo))
  do { print_log(7, "cache/lvmcache.c", 882 , 0,"<backtrace>"); return 0; } while (0);

 return 1;
}
/* # 902 "cache/lvmcache.c" */
static int _lvmcache_update_pvid(struct lvmcache_info *info, const char *pvid)
{



 if (((dm_hash_lookup(_pvid_hash, pvid)) == info) &&
     !strcmp(info->dev->pvid, pvid))
  return 1;
 if (*info->dev->pvid)
  dm_hash_remove(_pvid_hash, info->dev->pvid);
 strncpy(info->dev->pvid, pvid, sizeof(info->dev->pvid));
 if (!dm_hash_insert(_pvid_hash, pvid, info)) {
  print_log(3, "cache/lvmcache.c", 914 , -1,"_lvmcache_update: pvid insertion failed: %s", pvid);
  return 0;
 }

 return 1;
}




static int _lvmcache_update_vgid(struct lvmcache_info *info,
     struct lvmcache_vginfo *vginfo,
     const char *vgid)
{
 if (!vgid || !vginfo ||
     !strncmp(vginfo->vgid, vgid, 32))
  return 1;

 if (vginfo && *vginfo->vgid)
  dm_hash_remove(_vgid_hash, vginfo->vgid);
 if (!vgid) {
  print_log(7, "cache/lvmcache.c", 935 , 0,"lvmcache: %s: clearing VGID", info ? dev_name(info->dev) : vginfo->vgname);
  return 1;
 }

 strncpy(vginfo->vgid, vgid, 32);
 vginfo->vgid[32] = '\0';
 if (!dm_hash_insert(_vgid_hash, vginfo->vgid, vginfo)) {
  print_log(3,
 "cache/lvmcache.c"
/* # 942 "cache/lvmcache.c" */
  ,
 943
/* # 942 "cache/lvmcache.c" */
  , -1,"_lvmcache_update: vgid hash insertion failed: %s", vginfo->vgid)
                  ;
  return 0;
 }

 if (/* TODO: !is_orphan_vg(vginfo->vgname) */ 0)
  print_log(7,

 "cache/lvmcache.c"
/* # 948 "cache/lvmcache.c" */
  ,

 950
/* # 948 "cache/lvmcache.c" */
  , 0,"lvmcache: %s: setting %s VGID to %s", dev_name(info->dev), vginfo->vgname, vginfo->vgid)

                  ;

 return 1;
}

static int _insert_vginfo(struct lvmcache_vginfo *new_vginfo, const char *vgid,
     uint32_t vgstatus, const char *creation_host,
     struct lvmcache_vginfo *primary_vginfo)
{
 struct lvmcache_vginfo *last_vginfo = primary_vginfo;
 char uuid_primary[64] __attribute__((aligned(8)));
 char uuid_new[64] __attribute__((aligned(8)));
 int use_new = 0;


 if (primary_vginfo) {
  if (!id_write_format((const struct id *)vgid, uuid_new, sizeof(uuid_new)))
   do { print_log(7, "cache/lvmcache.c", 967 , 0,"<backtrace>"); return 0; } while (0);

  if (!id_write_format((const struct id *)&primary_vginfo->vgid, uuid_primary,
         sizeof(uuid_primary)))
   do { print_log(7, "cache/lvmcache.c", 971 , 0,"<backtrace>"); return 0; } while (0);
/* # 981 "cache/lvmcache.c" */
  if (!(primary_vginfo->status & 0x00000002U) &&
      (vgstatus & 0x00000002U))
   print_log(4 | 128,


 "cache/lvmcache.c"
/* # 983 "cache/lvmcache.c" */
   ,


 986
/* # 983 "cache/lvmcache.c" */
   , 0,"WARNING: Duplicate VG name %s: " "Existing %s takes precedence over " "exported %s", new_vginfo->vgname, uuid_primary, uuid_new)


                            ;
  else if ((primary_vginfo->status & 0x00000002U) &&
      !(vgstatus & 0x00000002U)) {
   print_log(4 | 128,


 "cache/lvmcache.c"
/* # 989 "cache/lvmcache.c" */
   ,


 992
/* # 989 "cache/lvmcache.c" */
   , 0,"WARNING: Duplicate VG name %s: " "%s takes precedence over exported %s", new_vginfo->vgname, uuid_new, uuid_primary)


                  ;
   use_new = 1;
  } else if (primary_vginfo->creation_host &&
      !strcmp(primary_vginfo->creation_host,
       primary_vginfo->fmt->cmd->hostname))
   print_log(4 | 128,


 "cache/lvmcache.c"
/* # 997 "cache/lvmcache.c" */
   ,


 1000
/* # 997 "cache/lvmcache.c" */
   , 0,"WARNING: Duplicate VG name %s: " "Existing %s (created here) takes precedence " "over %s", new_vginfo->vgname, uuid_primary, uuid_new)


              ;
  else if (!primary_vginfo->creation_host && creation_host) {
   print_log(4 | 128,


 "cache/lvmcache.c"
/* # 1002 "cache/lvmcache.c" */
   ,


 1005
/* # 1002 "cache/lvmcache.c" */
   , 0,"WARNING: Duplicate VG name %s: " "%s (with creation_host) takes precedence over %s", new_vginfo->vgname, uuid_new, uuid_primary)


                  ;
   use_new = 1;
  } else if (creation_host &&
      !strcmp(creation_host,
       primary_vginfo->fmt->cmd->hostname)) {
   print_log(4 | 128,


 "cache/lvmcache.c"
/* # 1010 "cache/lvmcache.c" */
   ,


 1013
/* # 1010 "cache/lvmcache.c" */
   , 0,"WARNING: Duplicate VG name %s: " "%s (created here) takes precedence over %s", new_vginfo->vgname, uuid_new, uuid_primary)


                  ;
   use_new = 1;
  }

  if (!use_new) {
   while (last_vginfo->next)
    last_vginfo = last_vginfo->next;
   last_vginfo->next = new_vginfo;
   return 1;
  }

  dm_hash_remove(_vgname_hash, primary_vginfo->vgname);
 }

 if (!dm_hash_insert(_vgname_hash, new_vginfo->vgname, new_vginfo)) {
  print_log(3,
 "cache/lvmcache.c"
/* # 1028 "cache/lvmcache.c" */
  ,
 1029
/* # 1028 "cache/lvmcache.c" */
  , -1,"cache_update: vg hash insertion failed: %s", new_vginfo->vgname)
                        ;
  return 0;
 }

 if (primary_vginfo)
  new_vginfo->next = primary_vginfo;

 return 1;
}

static int _lvmcache_update_vgname(struct lvmcache_info *info,
       const char *vgname, const char *vgid,
       uint32_t vgstatus, const char *creation_host,
       const struct format_type *fmt)
{
 struct lvmcache_vginfo *vginfo, *primary_vginfo, *orphan_vginfo;
 struct lvmcache_info *info2, *info3;
 char mdabuf[32];


 if (!vgname || (info && info->vginfo && !strcmp(info->vginfo->vgname, vgname)))
  return 1;


 if (info)
  _drop_vginfo(info, info->vginfo);


 if (!(vginfo = vginfo_from_vgname(vgname, vgid))) {
/* # 1094 "cache/lvmcache.c" */
  if (!(vginfo = dm_zalloc_aux((sizeof(*vginfo)), "cache/lvmcache.c", 1094))) {
   print_log(3, "cache/lvmcache.c", 1095 , -1,"lvmcache_update_vgname: list alloc failed");
   return 0;
  }
  if (!(vginfo->vgname = (__extension__ (__builtin_constant_p (vgname) && ((size_t)(const void *)((vgname) + 1) - (size_t)(const void *)(vgname) == 1) ? (((__const char *) (vgname))[0] == '\0' ? (char *) calloc ((size_t) 1, (size_t) 1) : ({ size_t __len = strlen (vgname) + 1; char *__retval = (char *) malloc (__len); if (__retval != ((void *)0)) __retval = (char *) memcpy (__retval, vgname, __len); __retval; })) : __strdup (vgname))))) {
   free(vginfo);
   print_log(3, "cache/lvmcache.c", 1100 , -1,"cache vgname alloc failed for %s", vgname);
   return 0;
  }
  dm_list_init(&vginfo->infos);





  while ((primary_vginfo = vginfo_from_vgname(vgname, ((void *)0))) &&
         _scanning_in_progress && _vginfo_is_invalid(primary_vginfo))
   for (info2 = ((__typeof__(*info2) *)((const char *)(((&primary_vginfo->infos))->n) - (const char *)&((__typeof__(*info2) *) 0)->list)), info3 = ((__typeof__(*info2) *)((const char *)(info2->list.n) - (const char *)&((__typeof__(*info2) *) 0)->list)); &info2->list != ((&primary_vginfo->infos)); info2 = info3, info3 = ((__typeof__(*info2) *)((const char *)(info2->list.n) - (const char *)&((__typeof__(*info2) *) 0)->list))) {
    orphan_vginfo = vginfo_from_vgname(primary_vginfo->fmt->orphan_vg_name, ((void *)0));
    if (!orphan_vginfo) {
     print_log(3,
 "cache/lvmcache.c"
/* # 1114 "cache/lvmcache.c" */
     ,
 1115
/* # 1114 "cache/lvmcache.c" */
     , -1,"Internal error: " "Orphan vginfo %s lost from cache.", primary_vginfo->fmt->orphan_vg_name)
                                            ;
     free(vginfo->vgname);
     free(vginfo);
     return 0;
    }
    _drop_vginfo(info2, primary_vginfo);
    _vginfo_attach_info(orphan_vginfo, info2);
    if (info2->mdas.n)
     sprintf(mdabuf, " with %u mdas",
      dm_list_size(&info2->mdas));
    else
     mdabuf[0] = '\0';
    print_log(7,



 "cache/lvmcache.c"
/* # 1127 "cache/lvmcache.c" */
    ,



 1131
/* # 1127 "cache/lvmcache.c" */
    , 0,"lvmcache: %s: now in VG %s%s%s%s%s", dev_name(info2->dev), vgname, orphan_vginfo->vgid[0] ? " (" : "", orphan_vginfo->vgid[0] ? orphan_vginfo->vgid : "", orphan_vginfo->vgid[0] ? ")" : "", mdabuf)



                                                 ;
  }

  if (!_insert_vginfo(vginfo, vgid, vgstatus, creation_host,
        primary_vginfo)) {
   free(vginfo->vgname);
   free(vginfo);
   return 0;
  }

  if (is_orphan_vg(vgname))
   dm_list_add(&_vginfos, &vginfo->list);
  else
   dm_list_add_h(&_vginfos, &vginfo->list);



 }

 if (info)
  _vginfo_attach_info(vginfo, info);
 else if (!_lvmcache_update_vgid(((void *)0), vginfo, vgid))
  do { print_log(7, "cache/lvmcache.c", 1153 , 0,"<backtrace>"); return 0; } while (0);

 _update_cache_vginfo_lock_state(vginfo, vgname_is_locked(vgname));


 vginfo->fmt = fmt;

 if (info) {
  if (info->mdas.n)
   sprintf(mdabuf, " with %u mdas", dm_list_size(&info->mdas));
  else
   mdabuf[0] = '\0';
  print_log(7,



 "cache/lvmcache.c"
/* # 1165 "cache/lvmcache.c" */
  ,



 1169
/* # 1165 "cache/lvmcache.c" */
  , 0,"lvmcache: %s: now in VG %s%s%s%s%s", dev_name(info->dev), vgname, vginfo->vgid[0] ? " (" : "", vginfo->vgid[0] ? vginfo->vgid : "", vginfo->vgid[0] ? ")" : "", mdabuf)



                                        ;
 } else
  print_log(7, "cache/lvmcache.c", 1171 , 0,"lvmcache: initialised VG %s", vgname);

 return 1;
}

static int _lvmcache_update_vgstatus(struct lvmcache_info *info, uint32_t vgstatus,
         const char *creation_host)
{
 if (!info || !info->vginfo)
  return 1;

 if ((info->vginfo->status & 0x00000002U) != (vgstatus & 0x00000002U))
  print_log(7,

 "cache/lvmcache.c"
/* # 1183 "cache/lvmcache.c" */
  ,

 1185
/* # 1183 "cache/lvmcache.c" */
  , 0,"lvmcache: %s: VG %s %s exported", dev_name(info->dev), info->vginfo->vgname, vgstatus & 0x00000002U ? "now" : "no longer")

                                                  ;

 info->vginfo->status = vgstatus;

 if (!creation_host)
  return 1;

 if (info->vginfo->creation_host && !strcmp(creation_host,
         info->vginfo->creation_host))
  return 1;

 if (info->vginfo->creation_host)
  free(info->vginfo->creation_host);

 if (!(info->vginfo->creation_host = (__extension__ (__builtin_constant_p (creation_host) && ((size_t)(const void *)((creation_host) + 1) - (size_t)(const void *)(creation_host) == 1) ? (((__const char *) (creation_host))[0] == '\0' ? (char *) calloc ((size_t) 1, (size_t) 1) : ({ size_t __len = strlen (creation_host) + 1; char *__retval = (char *) malloc (__len); if (__retval != ((void *)0)) __retval = (char *) memcpy (__retval, creation_host, __len); __retval; })) : __strdup (creation_host))))) {
  print_log(3,
 "cache/lvmcache.c"
/* # 1200 "cache/lvmcache.c" */
  ,
 1201
/* # 1200 "cache/lvmcache.c" */
  , -1,"cache creation host alloc failed for %s", creation_host)
                   ;
  return 0;
 }

 print_log(7,
 "cache/lvmcache.c"
/* # 1205 "cache/lvmcache.c" */
 ,
 1206
/* # 1205 "cache/lvmcache.c" */
 , 0,"lvmcache: %s: VG %s: Set creation host to %s.", dev_name(info->dev), info->vginfo->vgname, creation_host)
                                                             ;

 return 1;
}

int lvmcache_add_orphan_vginfo(const char *vgname, struct format_type *fmt)
{
 if (!_lock_hash && !lvmcache_init()) {
  print_log(3, "cache/lvmcache.c", 1214 , -1,"Internal cache initialisation failed");
  return 0;
 }

 return _lvmcache_update_vgname(((void *)0), vgname, vgname, 0, "", fmt);
}

int lvmcache_update_vgname_and_id(struct lvmcache_info *info,
      const char *vgname, const char *vgid,
      uint32_t vgstatus, const char *creation_host)
{
 if (!vgname && !info->vginfo) {
  print_log(3, "cache/lvmcache.c", 1226 , -1,"Internal error: " "NULL vgname handed to cache");

  vgname = info->fmt->orphan_vg_name;
  vgid = vgname;
 }


 if (is_orphan_vg(vgname) && info->vginfo &&
     mdas_empty_or_ignored(&info->mdas) &&
     !is_orphan_vg(info->vginfo->vgname) && critical_section())
  return 1;


 if (!is_orphan_vg(vgname))
  info->status &= ~0x00000001;

 if (!_lvmcache_update_vgname(info, vgname, vgid, vgstatus,
         creation_host, info->fmt) ||
     !_lvmcache_update_vgid(info, info->vginfo, vgid) ||
     !_lvmcache_update_vgstatus(info, vgstatus, creation_host))
  do { print_log(7, "cache/lvmcache.c", 1246 , 0,"<backtrace>"); return 0; } while (0);

 return 1;
}

int lvmcache_update_vg(struct volume_group *vg, unsigned precommitted)
{
 struct pv_list *pvl;
 struct lvmcache_info *info;
 char pvid_s[32 + 1] __attribute__((aligned(8)));

 pvid_s[sizeof(pvid_s) - 1] = '\0';

 for (pvl = ((__typeof__(*pvl) *)((const char *)(((&vg->pvs))->n) - (const char *)&((__typeof__(*pvl) *) 0)->list)); &pvl->list != ((&vg->pvs)); pvl = ((__typeof__(*pvl) *)((const char *)(pvl->list.n) - (const char *)&((__typeof__(*pvl) *) 0)->list))) {
  strncpy(pvid_s, (char *) &pvl->pv->id, sizeof(pvid_s) - 1);

  if ((info = info_from_pvid(pvid_s, 0)) &&
      !lvmcache_update_vgname_and_id(info, vg->name,
         (char *) &vg->id,
         vg->status, ((void *)0)))
   do { print_log(7, "cache/lvmcache.c", 1266 , 0,"<backtrace>"); return 0; } while (0);
 }


 if (vg->cmd->current_settings.cache_vgmetadata)
  _store_metadata(vg, precommitted);

 return 1;
}

struct lvmcache_info *lvmcache_add(struct labeller *labeller, const char *pvid,
       struct device *dev,
       const char *vgname, const char *vgid,
       uint32_t vgstatus)
{
 struct label *label;
 struct lvmcache_info *existing, *info;
 char pvid_s[32 + 1] __attribute__((aligned(8)));

 if (!_vgname_hash && !lvmcache_init()) {
  print_log(3, "cache/lvmcache.c", 1286 , -1,"Internal cache initialisation failed");
  return ((void *)0);
 }

 strncpy(pvid_s, pvid, sizeof(pvid_s) - 1);
 pvid_s[sizeof(pvid_s) - 1] = '\0';

 if (!(existing = info_from_pvid(pvid_s, 0)) &&
     !(existing = info_from_pvid(dev->pvid, 0))) {
  if (!(label = label_create(labeller)))
   do { print_log(7, "cache/lvmcache.c", 1296 , 0,"<backtrace>"); return ((void *)0); } while (0);
  if (!(info = dm_zalloc_aux((sizeof(*info)), "cache/lvmcache.c", 1297))) {
   print_log(3, "cache/lvmcache.c", 1298 , -1,"lvmcache_info allocation failed");
   label_destroy(label);
   return ((void *)0);
  }

  label->info = info;
  info->label = label;
  dm_list_init(&info->list);
  info->dev = dev;
 } else {
  if (existing->dev != dev) {

   if (dev_subsystem_part_major(existing->dev) &&
       !dev_subsystem_part_major(dev)) {
    print_log(6,



 "cache/lvmcache.c"
/* # 1312 "cache/lvmcache.c" */
    ,



 1316
/* # 1312 "cache/lvmcache.c" */
    , 0,"Ignoring duplicate PV %s on " "%s - using %s %s", pvid, dev_name(dev), dev_subsystem_name(existing->dev), dev_name(existing->dev))



                               ;
    return ((void *)0);
   } else if (dm_is_dm_major(((existing->dev->dev & 0xfff00) >> 8)) &&
       !dm_is_dm_major(((dev->dev & 0xfff00) >> 8))) {
    print_log(6,


 "cache/lvmcache.c"
/* # 1320 "cache/lvmcache.c" */
    ,


 1323
/* # 1320 "cache/lvmcache.c" */
    , 0,"Ignoring duplicate PV %s on " "%s - using dm %s", pvid, dev_name(dev), dev_name(existing->dev))


                               ;
    return ((void *)0);
   } else if (!dev_subsystem_part_major(existing->dev) &&
       dev_subsystem_part_major(dev))
    print_log(6,



 "cache/lvmcache.c"
/* # 1327 "cache/lvmcache.c" */
    ,



 1331
/* # 1327 "cache/lvmcache.c" */
    , 0,"Duplicate PV %s on %s - " "using %s %s", pvid, dev_name(existing->dev), dev_subsystem_name(existing->dev), dev_name(dev))



                     ;
   else if (!dm_is_dm_major(((existing->dev->dev & 0xfff00) >> 8)) &&
     dm_is_dm_major(((dev->dev & 0xfff00) >> 8)))
    print_log(6,


 "cache/lvmcache.c"
/* # 1334 "cache/lvmcache.c" */
    ,


 1337
/* # 1334 "cache/lvmcache.c" */
    , 0,"Duplicate PV %s on %s - " "using dm %s", pvid, dev_name(existing->dev), dev_name(dev))


                     ;




   else if (!strcmp(pvid_s, existing->dev->pvid))
    print_log(3,

 "cache/lvmcache.c"
/* # 1343 "cache/lvmcache.c" */
    ,

 1345
/* # 1343 "cache/lvmcache.c" */
    , -1,"Found duplicate PV %s: using %s not " "%s", pvid, dev_name(dev), dev_name(existing->dev))

                               ;
  }
  if (strcmp(pvid_s, existing->dev->pvid))
   print_log(7,

 "cache/lvmcache.c"
/* # 1348 "cache/lvmcache.c" */
   ,

 1350
/* # 1348 "cache/lvmcache.c" */
   , 0,"Updating pvid cache to %s (%s) from %s (%s)", pvid_s, dev_name(dev), existing->dev->pvid, dev_name(existing->dev))

                                                   ;

  existing->dev = dev;
  info = existing;

  if (info->label->labeller != labeller) {
   label_destroy(info->label);
   if (!(info->label = label_create(labeller)))

    do { print_log(7, "cache/lvmcache.c", 1359 , 0,"<backtrace>"); return ((void *)0); } while (0);
   info->label->info = info;
  }
  label = info->label;
 }

 info->fmt = (const struct format_type *) labeller->private;
 info->status |= 0x00000001;

 if (!_lvmcache_update_pvid(info, pvid_s)) {
  if (!existing) {
   free(info);
   label_destroy(label);
  }
  return ((void *)0);
 }

 if (!lvmcache_update_vgname_and_id(info, vgname, vgid, vgstatus, ((void *)0))) {
  if (!existing) {
   dm_hash_remove(_pvid_hash, pvid_s);
   strcpy(info->dev->pvid, "");
   free(info);
   label_destroy(label);
  }
  return ((void *)0);
 }

 return info;
}

static void _lvmcache_destroy_entry(struct lvmcache_info *info)
{
 _vginfo_detach_info(info);
 strcpy(info->dev->pvid, "");
 label_destroy(info->label);
 free(info);
}

static void _lvmcache_destroy_vgnamelist(struct lvmcache_vginfo *vginfo)
{
 struct lvmcache_vginfo *next;

 do {
  next = vginfo->next;
  if (!_free_vginfo(vginfo))
   print_log(7, "cache/lvmcache.c", 1404 , 0,"<backtrace>");
 } while ((vginfo = next));
}

static void _lvmcache_destroy_lockname(struct dm_hash_node *n)
{
 char *vgname;

 if (!dm_hash_get_data(_lock_hash, n))
  return;

 vgname = dm_hash_get_key(_lock_hash, n);

 if (!strcmp(vgname, "#global"))
  _vg_global_lock_held = 1;
 else
  print_log(3,
 "cache/lvmcache.c"
/* # 1420 "cache/lvmcache.c" */
  ,
 1421
/* # 1420 "cache/lvmcache.c" */
  , -1,"Internal error: " "Volume Group %s was not unlocked", dm_hash_get_key(_lock_hash, n))
                                    ;
}

void lvmcache_destroy(struct cmd_context *cmd, int retain_orphans)
{
 struct dm_hash_node *n;
 print_log(5, "cache/lvmcache.c", 1427 , 0,"Wiping internal VG cache");

 _has_scanned = 0;

 if (_vgid_hash) {
  dm_hash_destroy(_vgid_hash);
  _vgid_hash = ((void *)0);
 }

 if (_pvid_hash) {
  dm_hash_iter(_pvid_hash, (dm_hash_iterate_fn) _lvmcache_destroy_entry);
  dm_hash_destroy(_pvid_hash);
  _pvid_hash = ((void *)0);
 }

 if (_vgname_hash) {
  dm_hash_iter(_vgname_hash,
     (dm_hash_iterate_fn) _lvmcache_destroy_vgnamelist);
  dm_hash_destroy(_vgname_hash);
  _vgname_hash = ((void *)0);
 }

 if (_lock_hash) {
  for (n = dm_hash_get_first((_lock_hash)); n; n = dm_hash_get_next((_lock_hash), n))
   _lvmcache_destroy_lockname(n);
  dm_hash_destroy(_lock_hash);
  _lock_hash = ((void *)0);
 }

 if (!dm_list_empty(&_vginfos))
  print_log(3, "cache/lvmcache.c", 1457 , -1,"Internal error: " "_vginfos list should be empty");
 dm_list_init(&_vginfos);

 if (retain_orphans)
  init_lvmcache_orphans(cmd);
}
