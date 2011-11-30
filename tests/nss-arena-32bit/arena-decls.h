# 2 "arena-decls.h"
/* # 1 "arena.c" */
/* # 1 "<built-in>" */
/* # 1 "<command-line>" */
/* # 1 "arena.c" */
/* # 48 "arena.c" */
/* # 1 "base.h" 1 */
/* # 52 "base.h" */
/* # 1 "baset.h" 1 */
/* # 52 "baset.h" */
/* # 1 "nssbaset.h" 1 */
/* # 50 "nssbaset.h" */
/* # 1 "/usr/include/nspr4/nspr.h" 1 */
/* # 41 "/usr/include/nspr4/nspr.h" */
/* # 1 "/usr/include/nspr4/pratom.h" 1 */
/* # 46 "/usr/include/nspr4/pratom.h" */
/* # 1 "/usr/include/nspr4/prtypes.h" 1 */
/* # 58 "/usr/include/nspr4/prtypes.h" */
/* # 1 "/usr/include/nspr4/prcpucfg.h" 1 */
/* # 59 "/usr/include/nspr4/prtypes.h" 2 */


/* # 1 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stddef.h" 1 3 4 */
/* # 150 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stddef.h" 3 4 */
typedef int ptrdiff_t;
/* # 212 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stddef.h" 3 4 */
typedef unsigned int size_t;
/* # 324 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stddef.h" 3 4 */
typedef int wchar_t;
/* # 62 "/usr/include/nspr4/prtypes.h" 2 */
/* # 238 "/usr/include/nspr4/prtypes.h" */

/* # 248 "/usr/include/nspr4/prtypes.h" */
typedef unsigned char PRUint8;
/* # 262 "/usr/include/nspr4/prtypes.h" */
typedef signed char PRInt8;
/* # 287 "/usr/include/nspr4/prtypes.h" */
typedef unsigned short PRUint16;
typedef short PRInt16;
/* # 312 "/usr/include/nspr4/prtypes.h" */
typedef unsigned int PRUint32;
typedef int PRInt32;
/* # 376 "/usr/include/nspr4/prtypes.h" */
typedef long long PRInt64;
typedef unsigned long long PRUint64;
/* # 402 "/usr/include/nspr4/prtypes.h" */
typedef int PRIntn;
typedef unsigned int PRUintn;
/* # 413 "/usr/include/nspr4/prtypes.h" */
typedef double PRFloat64;






typedef size_t PRSize;







typedef PRInt32 PROffset32;
typedef PRInt64 PROffset64;







typedef ptrdiff_t PRPtrdiff;
/* # 448 "/usr/include/nspr4/prtypes.h" */
typedef unsigned long PRUptrdiff;
/* # 459 "/usr/include/nspr4/prtypes.h" */
typedef PRIntn PRBool;
/* # 469 "/usr/include/nspr4/prtypes.h" */
typedef PRUint8 PRPackedBool;





typedef enum { PR_FAILURE = -1, PR_SUCCESS = 0 } PRStatus;






typedef PRUint16 PRUnichar;
/* # 501 "/usr/include/nspr4/prtypes.h" */
typedef long PRWord;
typedef unsigned long PRUword;
/* # 541 "/usr/include/nspr4/prtypes.h" */
/* # 1 "/usr/include/nspr4/obsolete/protypes.h" 1 */
/* # 48 "/usr/include/nspr4/obsolete/protypes.h" */
typedef PRUintn uintn;

typedef PRIntn intn;
/* # 79 "/usr/include/nspr4/obsolete/protypes.h" */
/* # 1 "/usr/include/sys/types.h" 1 3 4 */
/* # 26 "/usr/include/sys/types.h" 3 4 */
/* # 1 "/usr/include/features.h" 1 3 4 */
/* # 363 "/usr/include/features.h" 3 4 */
/* # 1 "/usr/include/sys/cdefs.h" 1 3 4 */
/* # 372 "/usr/include/sys/cdefs.h" 3 4 */
/* # 1 "/usr/include/bits/wordsize.h" 1 3 4 */
/* # 373 "/usr/include/sys/cdefs.h" 2 3 4 */
/* # 364 "/usr/include/features.h" 2 3 4 */
/* # 387 "/usr/include/features.h" 3 4 */
/* # 1 "/usr/include/gnu/stubs.h" 1 3 4 */



/* # 1 "/usr/include/bits/wordsize.h" 1 3 4 */
/* # 5 "/usr/include/gnu/stubs.h" 2 3 4 */


/* # 1 "/usr/include/gnu/stubs-32.h" 1 3 4 */
/* # 8 "/usr/include/gnu/stubs.h" 2 3 4 */
/* # 388 "/usr/include/features.h" 2 3 4 */
/* # 27 "/usr/include/sys/types.h" 2 3 4 */



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
/* # 61 "/usr/include/sys/types.h" 3 4 */
typedef __dev_t dev_t;




typedef __gid_t gid_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;




typedef __uid_t uid_t;





typedef __off_t off_t;
/* # 99 "/usr/include/sys/types.h" 3 4 */
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





/* # 1 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stddef.h" 1 3 4 */
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
/* # 31 "/usr/include/bits/time.h" 3 4 */
struct timeval
  {
    __time_t tv_sec;
    __suseconds_t tv_usec;
  };
/* # 47 "/usr/include/sys/select.h" 2 3 4 */
/* # 55 "/usr/include/sys/select.h" 3 4 */
typedef long int __fd_mask;
/* # 65 "/usr/include/sys/select.h" 3 4 */
typedef struct
  {



    __fd_mask fds_bits[1024 / (8 * (int) sizeof (__fd_mask))];





  } fd_set;






typedef __fd_mask fd_mask;
/* # 97 "/usr/include/sys/select.h" 3 4 */

/* # 107 "/usr/include/sys/select.h" 3 4 */
extern int select (int __nfds, fd_set *__restrict __readfds,
     fd_set *__restrict __writefds,
     fd_set *__restrict __exceptfds,
     struct timeval *__restrict __timeout);
/* # 132 "/usr/include/sys/select.h" 3 4 */

/* # 221 "/usr/include/sys/types.h" 2 3 4 */


/* # 1 "/usr/include/sys/sysmacros.h" 1 3 4 */
/* # 30 "/usr/include/sys/sysmacros.h" 3 4 */


__extension__
extern unsigned int gnu_dev_major (unsigned long long int __dev)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__));
__extension__
extern unsigned int gnu_dev_minor (unsigned long long int __dev)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__));
__extension__
extern unsigned long long int gnu_dev_makedev (unsigned int __major,
            unsigned int __minor)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__));


__extension__ extern __inline __attribute__ ((__const__)) unsigned int
__attribute__ ((__nothrow__)) gnu_dev_major (unsigned long long int __dev)
{
  return ((__dev >> 8) & 0xfff) | ((unsigned int) (__dev >> 32) & ~0xfff);
}

__extension__ extern __inline __attribute__ ((__const__)) unsigned int
__attribute__ ((__nothrow__)) gnu_dev_minor (unsigned long long int __dev)
{
  return (__dev & 0xff) | ((unsigned int) (__dev >> 12) & ~0xff);
}

__extension__ extern __inline __attribute__ ((__const__)) unsigned long long int
__attribute__ ((__nothrow__)) gnu_dev_makedev (unsigned int __major, unsigned int __minor)
{
  return ((__minor & 0xff) | ((__major & 0xfff) << 8)
   | (((unsigned long long int) (__minor & ~0xff)) << 12)
   | (((unsigned long long int) (__major & ~0xfff)) << 32));
}


/* # 224 "/usr/include/sys/types.h" 2 3 4 */
/* # 236 "/usr/include/sys/types.h" 3 4 */
typedef __blkcnt_t blkcnt_t;



typedef __fsblkcnt_t fsblkcnt_t;



typedef __fsfilcnt_t fsfilcnt_t;
/* # 274 "/usr/include/sys/types.h" 3 4 */

/* # 80 "/usr/include/nspr4/obsolete/protypes.h" 2 */
/* # 100 "/usr/include/nspr4/obsolete/protypes.h" */
typedef PRUint64 uint64;
/* # 109 "/usr/include/nspr4/obsolete/protypes.h" */
typedef PRUint32 uint32;
/* # 120 "/usr/include/nspr4/obsolete/protypes.h" */
typedef PRUint16 uint16;







typedef PRUint8 uint8;







typedef PRInt64 int64;
/* # 146 "/usr/include/nspr4/obsolete/protypes.h" */
typedef PRInt32 int32;
/* # 158 "/usr/include/nspr4/obsolete/protypes.h" */
typedef PRInt16 int16;
/* # 167 "/usr/include/nspr4/obsolete/protypes.h" */
typedef PRInt8 int8;


typedef PRFloat64 float64;
typedef PRUptrdiff uptrdiff_t;
typedef PRUword uprword_t;
typedef PRWord prword_t;
/* # 542 "/usr/include/nspr4/prtypes.h" 2 */
/* # 554 "/usr/include/nspr4/prtypes.h" */

/* # 47 "/usr/include/nspr4/pratom.h" 2 */
/* # 1 "/usr/include/nspr4/prlock.h" 1 */
/* # 53 "/usr/include/nspr4/prlock.h" */

/* # 66 "/usr/include/nspr4/prlock.h" */
typedef struct PRLock PRLock;
/* # 83 "/usr/include/nspr4/prlock.h" */
extern __attribute__((visibility("default"))) PRLock* PR_NewLock(void);
/* # 94 "/usr/include/nspr4/prlock.h" */
extern __attribute__((visibility("default"))) void PR_DestroyLock(PRLock *lock);
/* # 105 "/usr/include/nspr4/prlock.h" */
extern __attribute__((visibility("default"))) void PR_Lock(PRLock *lock);
/* # 117 "/usr/include/nspr4/prlock.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Unlock(PRLock *lock);
/* # 137 "/usr/include/nspr4/prlock.h" */
extern __attribute__((visibility("default"))) void PR_AssertCurrentThreadOwnsLock(PRLock *lock);


/* # 48 "/usr/include/nspr4/pratom.h" 2 */


/* # 60 "/usr/include/nspr4/pratom.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_AtomicIncrement(PRInt32 *val);
/* # 71 "/usr/include/nspr4/pratom.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_AtomicDecrement(PRInt32 *val);
/* # 83 "/usr/include/nspr4/pratom.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_AtomicSet(PRInt32 *val, PRInt32 newval);
/* # 95 "/usr/include/nspr4/pratom.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_AtomicAdd(PRInt32 *ptr, PRInt32 val);
/* # 170 "/usr/include/nspr4/pratom.h" */
typedef struct PRStackElemStr PRStackElem;

struct PRStackElemStr {
    PRStackElem *prstk_elem_next;

};

typedef struct PRStackStr PRStack;
/* # 188 "/usr/include/nspr4/pratom.h" */
extern __attribute__((visibility("default"))) PRStack * PR_CreateStack(const char *stack_name);
/* # 200 "/usr/include/nspr4/pratom.h" */
extern __attribute__((visibility("default"))) void PR_StackPush(PRStack *stack, PRStackElem *stack_elem);
/* # 213 "/usr/include/nspr4/pratom.h" */
extern __attribute__((visibility("default"))) PRStackElem * PR_StackPop(PRStack *stack);
/* # 227 "/usr/include/nspr4/pratom.h" */
extern __attribute__((visibility("default"))) PRStatus PR_DestroyStack(PRStack *stack);


/* # 42 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prbit.h" 1 */
/* # 42 "/usr/include/nspr4/prbit.h" */

/* # 74 "/usr/include/nspr4/prbit.h" */
typedef unsigned long prbitmap_t;
/* # 86 "/usr/include/nspr4/prbit.h" */
extern __attribute__((visibility("default"))) PRIntn PR_CeilingLog2(PRUint32 i);




extern __attribute__((visibility("default"))) PRIntn PR_FloorLog2(PRUint32 i);
/* # 178 "/usr/include/nspr4/prbit.h" */

/* # 43 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prclist.h" 1 */
/* # 43 "/usr/include/nspr4/prclist.h" */
typedef struct PRCListStr PRCList;




struct PRCListStr {
    PRCList *next;
    PRCList *prev;
};
/* # 44 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prcmon.h" 1 */
/* # 51 "/usr/include/nspr4/prcmon.h" */
/* # 1 "/usr/include/nspr4/prmon.h" 1 */
/* # 42 "/usr/include/nspr4/prmon.h" */
/* # 1 "/usr/include/nspr4/prinrval.h" 1 */
/* # 55 "/usr/include/nspr4/prinrval.h" */






typedef PRUint32 PRIntervalTime;
/* # 111 "/usr/include/nspr4/prinrval.h" */
extern __attribute__((visibility("default"))) PRIntervalTime PR_IntervalNow(void);
/* # 129 "/usr/include/nspr4/prinrval.h" */
extern __attribute__((visibility("default"))) PRUint32 PR_TicksPerSecond(void);
/* # 148 "/usr/include/nspr4/prinrval.h" */
extern __attribute__((visibility("default"))) PRIntervalTime PR_SecondsToInterval(PRUint32 seconds);
extern __attribute__((visibility("default"))) PRIntervalTime PR_MillisecondsToInterval(PRUint32 milli);
extern __attribute__((visibility("default"))) PRIntervalTime PR_MicrosecondsToInterval(PRUint32 micro);
/* # 169 "/usr/include/nspr4/prinrval.h" */
extern __attribute__((visibility("default"))) PRUint32 PR_IntervalToSeconds(PRIntervalTime ticks);
extern __attribute__((visibility("default"))) PRUint32 PR_IntervalToMilliseconds(PRIntervalTime ticks);
extern __attribute__((visibility("default"))) PRUint32 PR_IntervalToMicroseconds(PRIntervalTime ticks);


/* # 43 "/usr/include/nspr4/prmon.h" 2 */



typedef struct PRMonitor PRMonitor;
/* # 55 "/usr/include/nspr4/prmon.h" */
extern __attribute__((visibility("default"))) PRMonitor* PR_NewMonitor(void);







extern __attribute__((visibility("default"))) void PR_DestroyMonitor(PRMonitor *mon);






extern __attribute__((visibility("default"))) void PR_EnterMonitor(PRMonitor *mon);






extern __attribute__((visibility("default"))) PRStatus PR_ExitMonitor(PRMonitor *mon);
/* # 95 "/usr/include/nspr4/prmon.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Wait(PRMonitor *mon, PRIntervalTime ticks);






extern __attribute__((visibility("default"))) PRStatus PR_Notify(PRMonitor *mon);






extern __attribute__((visibility("default"))) PRStatus PR_NotifyAll(PRMonitor *mon);
/* # 124 "/usr/include/nspr4/prmon.h" */
extern __attribute__((visibility("default"))) void PR_AssertCurrentThreadInMonitor(PRMonitor *mon);


/* # 52 "/usr/include/nspr4/prcmon.h" 2 */



/* # 64 "/usr/include/nspr4/prcmon.h" */
extern __attribute__((visibility("default"))) PRMonitor* PR_CEnterMonitor(void *address);





extern __attribute__((visibility("default"))) PRStatus PR_CExitMonitor(void *address);





extern __attribute__((visibility("default"))) PRStatus PR_CWait(void *address, PRIntervalTime timeout);





extern __attribute__((visibility("default"))) PRStatus PR_CNotify(void *address);





extern __attribute__((visibility("default"))) PRStatus PR_CNotifyAll(void *address);





extern __attribute__((visibility("default"))) void PR_CSetOnMonitorRecycle(void ( *callback)(void *address));


/* # 45 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prcvar.h" 1 */
/* # 44 "/usr/include/nspr4/prcvar.h" */


typedef struct PRCondVar PRCondVar;
/* # 59 "/usr/include/nspr4/prcvar.h" */
extern __attribute__((visibility("default"))) PRCondVar* PR_NewCondVar(PRLock *lock);







extern __attribute__((visibility("default"))) void PR_DestroyCondVar(PRCondVar *cvar);
/* # 97 "/usr/include/nspr4/prcvar.h" */
extern __attribute__((visibility("default"))) PRStatus PR_WaitCondVar(PRCondVar *cvar, PRIntervalTime timeout);
/* # 112 "/usr/include/nspr4/prcvar.h" */
extern __attribute__((visibility("default"))) PRStatus PR_NotifyCondVar(PRCondVar *cvar);
/* # 122 "/usr/include/nspr4/prcvar.h" */
extern __attribute__((visibility("default"))) PRStatus PR_NotifyAllCondVar(PRCondVar *cvar);


/* # 46 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prdtoa.h" 1 */
/* # 43 "/usr/include/nspr4/prdtoa.h" */

/* # 56 "/usr/include/nspr4/prdtoa.h" */
extern __attribute__((visibility("default"))) PRFloat64
PR_strtod(const char *s00, char **se);







extern __attribute__((visibility("default"))) void PR_cnvtf(char *buf, PRIntn bufsz, PRIntn prcsn, PRFloat64 fval);
/* # 78 "/usr/include/nspr4/prdtoa.h" */
extern __attribute__((visibility("default"))) PRStatus PR_dtoa(PRFloat64 d, PRIntn mode, PRIntn ndigits,
 PRIntn *decpt, PRIntn *sign, char **rve, char *buf, PRSize bufsize);


/* # 47 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prenv.h" 1 */
/* # 49 "/usr/include/nspr4/prenv.h" */

/* # 123 "/usr/include/nspr4/prenv.h" */
extern __attribute__((visibility("default"))) char* PR_GetEnv(const char *var);
/* # 146 "/usr/include/nspr4/prenv.h" */
extern __attribute__((visibility("default"))) PRStatus PR_SetEnv(const char *string);


/* # 48 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prerror.h" 1 */
/* # 43 "/usr/include/nspr4/prerror.h" */


typedef PRInt32 PRErrorCode;



/* # 1 "/usr/include/nspr4/prerr.h" 1 */
/* # 278 "/usr/include/nspr4/prerr.h" */
extern void nspr_InitializePRErrorTable(void);
/* # 50 "/usr/include/nspr4/prerror.h" 2 */







extern __attribute__((visibility("default"))) void PR_SetError(PRErrorCode errorCode, PRInt32 oserr);
/* # 68 "/usr/include/nspr4/prerror.h" */
extern __attribute__((visibility("default"))) void PR_SetErrorText(
    PRIntn textLength, const char *text);




extern __attribute__((visibility("default"))) PRErrorCode PR_GetError(void);





extern __attribute__((visibility("default"))) PRInt32 PR_GetOSError(void);






extern __attribute__((visibility("default"))) PRInt32 PR_GetErrorTextLength(void);






extern __attribute__((visibility("default"))) PRInt32 PR_GetErrorText(char *text);
/* # 177 "/usr/include/nspr4/prerror.h" */
typedef PRUint32 PRLanguageCode;
/* # 186 "/usr/include/nspr4/prerror.h" */
struct PRErrorMessage {
    const char * name;
    const char * en_text;
};






struct PRErrorTable {
    const struct PRErrorMessage * msgs;
    const char *name;
    PRErrorCode base;
    int n_msgs;
};






struct PRErrorCallbackPrivate;







struct PRErrorCallbackTablePrivate;
/* # 228 "/usr/include/nspr4/prerror.h" */
typedef const char *
PRErrorCallbackLookupFn(PRErrorCode code, PRLanguageCode language,
     const struct PRErrorTable *table,
     struct PRErrorCallbackPrivate *cb_private,
     struct PRErrorCallbackTablePrivate *table_private);
/* # 244 "/usr/include/nspr4/prerror.h" */
typedef struct PRErrorCallbackTablePrivate *
PRErrorCallbackNewTableFn(const struct PRErrorTable *table,
   struct PRErrorCallbackPrivate *cb_private);
/* # 262 "/usr/include/nspr4/prerror.h" */
extern __attribute__((visibility("default"))) const char * PR_ErrorToString(PRErrorCode code,
    PRLanguageCode language);
/* # 276 "/usr/include/nspr4/prerror.h" */
extern __attribute__((visibility("default"))) const char * PR_ErrorToName(PRErrorCode code);
/* # 290 "/usr/include/nspr4/prerror.h" */
extern __attribute__((visibility("default"))) const char * const * PR_ErrorLanguages(void);
/* # 303 "/usr/include/nspr4/prerror.h" */
extern __attribute__((visibility("default"))) PRErrorCode PR_ErrorInstallTable(const struct PRErrorTable *table);
/* # 319 "/usr/include/nspr4/prerror.h" */
extern __attribute__((visibility("default"))) void PR_ErrorInstallCallback(const char * const * languages,
         PRErrorCallbackLookupFn *lookup,
         PRErrorCallbackNewTableFn *newtable,
         struct PRErrorCallbackPrivate *cb_private);


/* # 49 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prinet.h" 1 */
/* # 69 "/usr/include/nspr4/prinet.h" */
/* # 1 "/usr/include/sys/socket.h" 1 3 4 */
/* # 26 "/usr/include/sys/socket.h" 3 4 */


/* # 1 "/usr/include/sys/uio.h" 1 3 4 */
/* # 26 "/usr/include/sys/uio.h" 3 4 */



/* # 1 "/usr/include/bits/uio.h" 1 3 4 */
/* # 44 "/usr/include/bits/uio.h" 3 4 */
struct iovec
  {
    void *iov_base;
    size_t iov_len;
  };
/* # 30 "/usr/include/sys/uio.h" 2 3 4 */
/* # 40 "/usr/include/sys/uio.h" 3 4 */
extern ssize_t readv (int __fd, __const struct iovec *__iovec, int __count)
  ;
/* # 51 "/usr/include/sys/uio.h" 3 4 */
extern ssize_t writev (int __fd, __const struct iovec *__iovec, int __count)
  ;
/* # 66 "/usr/include/sys/uio.h" 3 4 */
extern ssize_t preadv (int __fd, __const struct iovec *__iovec, int __count,
         __off_t __offset) ;
/* # 78 "/usr/include/sys/uio.h" 3 4 */
extern ssize_t pwritev (int __fd, __const struct iovec *__iovec, int __count,
   __off_t __offset) ;
/* # 121 "/usr/include/sys/uio.h" 3 4 */

/* # 29 "/usr/include/sys/socket.h" 2 3 4 */

/* # 1 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stddef.h" 1 3 4 */
/* # 31 "/usr/include/sys/socket.h" 2 3 4 */
/* # 40 "/usr/include/sys/socket.h" 3 4 */
/* # 1 "/usr/include/bits/socket.h" 1 3 4 */
/* # 29 "/usr/include/bits/socket.h" 3 4 */
/* # 1 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stddef.h" 1 3 4 */
/* # 30 "/usr/include/bits/socket.h" 2 3 4 */





typedef __socklen_t socklen_t;




enum __socket_type
{
  SOCK_STREAM = 1,


  SOCK_DGRAM = 2,


  SOCK_RAW = 3,

  SOCK_RDM = 4,

  SOCK_SEQPACKET = 5,


  SOCK_DCCP = 6,

  SOCK_PACKET = 10,







  SOCK_CLOEXEC = 02000000,


  SOCK_NONBLOCK = 04000


};
/* # 177 "/usr/include/bits/socket.h" 3 4 */
/* # 1 "/usr/include/bits/sockaddr.h" 1 3 4 */
/* # 29 "/usr/include/bits/sockaddr.h" 3 4 */
typedef unsigned short int sa_family_t;
/* # 178 "/usr/include/bits/socket.h" 2 3 4 */


struct sockaddr
  {
    sa_family_t sa_family;
    char sa_data[14];
  };
/* # 193 "/usr/include/bits/socket.h" 3 4 */
struct sockaddr_storage
  {
    sa_family_t ss_family;
    unsigned long int __ss_align;
    char __ss_padding[(128 - (2 * sizeof (unsigned long int)))];
  };



enum
  {
    MSG_OOB = 0x01,

    MSG_PEEK = 0x02,

    MSG_DONTROUTE = 0x04,






    MSG_CTRUNC = 0x08,

    MSG_PROXY = 0x10,

    MSG_TRUNC = 0x20,

    MSG_DONTWAIT = 0x40,

    MSG_EOR = 0x80,

    MSG_WAITALL = 0x100,

    MSG_FIN = 0x200,

    MSG_SYN = 0x400,

    MSG_CONFIRM = 0x800,

    MSG_RST = 0x1000,

    MSG_ERRQUEUE = 0x2000,

    MSG_NOSIGNAL = 0x4000,

    MSG_MORE = 0x8000,

    MSG_WAITFORONE = 0x10000,


    MSG_CMSG_CLOEXEC = 0x40000000



  };




struct msghdr
  {
    void *msg_name;
    socklen_t msg_namelen;

    struct iovec *msg_iov;
    size_t msg_iovlen;

    void *msg_control;
    size_t msg_controllen;




    int msg_flags;
  };
/* # 280 "/usr/include/bits/socket.h" 3 4 */
struct cmsghdr
  {
    size_t cmsg_len;




    int cmsg_level;
    int cmsg_type;



  };
/* # 310 "/usr/include/bits/socket.h" 3 4 */
extern struct cmsghdr *__cmsg_nxthdr (struct msghdr *__mhdr,
          struct cmsghdr *__cmsg) __attribute__ ((__nothrow__));




extern __inline struct cmsghdr *
__attribute__ ((__nothrow__)) __cmsg_nxthdr (struct msghdr *__mhdr, struct cmsghdr *__cmsg)
{
  if ((size_t) __cmsg->cmsg_len < sizeof (struct cmsghdr))

    return (struct cmsghdr *) 0;

  __cmsg = (struct cmsghdr *) ((unsigned char *) __cmsg
          + (((__cmsg->cmsg_len) + sizeof (size_t) - 1) & (size_t) ~(sizeof (size_t) - 1)));
  if ((unsigned char *) (__cmsg + 1) > ((unsigned char *) __mhdr->msg_control
     + __mhdr->msg_controllen)
      || ((unsigned char *) __cmsg + (((__cmsg->cmsg_len) + sizeof (size_t) - 1) & (size_t) ~(sizeof (size_t) - 1))
   > ((unsigned char *) __mhdr->msg_control + __mhdr->msg_controllen)))

    return (struct cmsghdr *) 0;
  return __cmsg;
}




enum
  {
    SCM_RIGHTS = 0x01





  };
/* # 383 "/usr/include/bits/socket.h" 3 4 */
/* # 1 "/usr/include/asm/socket.h" 1 3 4 */
/* # 1 "/usr/include/asm-generic/socket.h" 1 3 4 */



/* # 1 "/usr/include/asm/sockios.h" 1 3 4 */
/* # 1 "/usr/include/asm-generic/sockios.h" 1 3 4 */
/* # 1 "/usr/include/asm/sockios.h" 2 3 4 */
/* # 5 "/usr/include/asm-generic/socket.h" 2 3 4 */
/* # 1 "/usr/include/asm/socket.h" 2 3 4 */
/* # 384 "/usr/include/bits/socket.h" 2 3 4 */
/* # 417 "/usr/include/bits/socket.h" 3 4 */
struct linger
  {
    int l_onoff;
    int l_linger;
  };







/* mmsghdr is not declared (in other words, using glibc from rawhide is fun) */
#if 0
extern int recvmmsg (int __fd, struct mmsghdr *__vmessages,
       unsigned int __vlen, int __flags,
       __const struct timespec *__tmo);





extern int sendmmsg (int __fd, struct mmsghdr *__vmessages,
       unsigned int __vlen, int __flags);
#endif

/* # 41 "/usr/include/sys/socket.h" 2 3 4 */




struct osockaddr
  {
    unsigned short int sa_family;
    unsigned char sa_data[14];
  };




enum
{
  SHUT_RD = 0,

  SHUT_WR,

  SHUT_RDWR

};
/* # 105 "/usr/include/sys/socket.h" 3 4 */
extern int socket (int __domain, int __type, int __protocol) __attribute__ ((__nothrow__));





extern int socketpair (int __domain, int __type, int __protocol,
         int __fds[2]) __attribute__ ((__nothrow__));


extern int bind (int __fd, __const struct sockaddr * __addr, socklen_t __len)
     __attribute__ ((__nothrow__));


extern int getsockname (int __fd, struct sockaddr *__restrict __addr,
   socklen_t *__restrict __len) __attribute__ ((__nothrow__));
/* # 129 "/usr/include/sys/socket.h" 3 4 */
extern int connect (int __fd, __const struct sockaddr * __addr, socklen_t __len);



extern int getpeername (int __fd, struct sockaddr *__restrict __addr,
   socklen_t *__restrict __len) __attribute__ ((__nothrow__));






extern ssize_t send (int __fd, __const void *__buf, size_t __n, int __flags);






extern ssize_t recv (int __fd, void *__buf, size_t __n, int __flags);






extern ssize_t sendto (int __fd, __const void *__buf, size_t __n,
         int __flags, __const struct sockaddr * __addr,
         socklen_t __addr_len);
/* # 166 "/usr/include/sys/socket.h" 3 4 */
extern ssize_t recvfrom (int __fd, void *__restrict __buf, size_t __n,
    int __flags, struct sockaddr *__restrict __addr,
    socklen_t *__restrict __addr_len);







extern ssize_t sendmsg (int __fd, __const struct msghdr *__message,
   int __flags);






extern ssize_t recvmsg (int __fd, struct msghdr *__message, int __flags);





extern int getsockopt (int __fd, int __level, int __optname,
         void *__restrict __optval,
         socklen_t *__restrict __optlen) __attribute__ ((__nothrow__));




extern int setsockopt (int __fd, int __level, int __optname,
         __const void *__optval, socklen_t __optlen) __attribute__ ((__nothrow__));





extern int listen (int __fd, int __n) __attribute__ ((__nothrow__));
/* # 214 "/usr/include/sys/socket.h" 3 4 */
extern int accept (int __fd, struct sockaddr *__restrict __addr,
     socklen_t *__restrict __addr_len);
/* # 232 "/usr/include/sys/socket.h" 3 4 */
extern int shutdown (int __fd, int __how) __attribute__ ((__nothrow__));
/* # 245 "/usr/include/sys/socket.h" 3 4 */
extern int isfdtype (int __fd, int __fdtype) __attribute__ ((__nothrow__));
/* # 254 "/usr/include/sys/socket.h" 3 4 */

/* # 70 "/usr/include/nspr4/prinet.h" 2 */
/* # 1 "/usr/include/netinet/in.h" 1 3 4 */
/* # 24 "/usr/include/netinet/in.h" 3 4 */
/* # 1 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stdint.h" 1 3 4 */


/* # 1 "/usr/include/stdint.h" 1 3 4 */
/* # 27 "/usr/include/stdint.h" 3 4 */
/* # 1 "/usr/include/bits/wchar.h" 1 3 4 */
/* # 28 "/usr/include/stdint.h" 2 3 4 */
/* # 1 "/usr/include/bits/wordsize.h" 1 3 4 */
/* # 29 "/usr/include/stdint.h" 2 3 4 */
/* # 49 "/usr/include/stdint.h" 3 4 */
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
/* # 4 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stdint.h" 2 3 4 */
/* # 25 "/usr/include/netinet/in.h" 2 3 4 */







enum
  {
    IPPROTO_IP = 0,

    IPPROTO_HOPOPTS = 0,

    IPPROTO_ICMP = 1,

    IPPROTO_IGMP = 2,

    IPPROTO_IPIP = 4,

    IPPROTO_TCP = 6,

    IPPROTO_EGP = 8,

    IPPROTO_PUP = 12,

    IPPROTO_UDP = 17,

    IPPROTO_IDP = 22,

    IPPROTO_TP = 29,

    IPPROTO_DCCP = 33,

    IPPROTO_IPV6 = 41,

    IPPROTO_ROUTING = 43,

    IPPROTO_FRAGMENT = 44,

    IPPROTO_RSVP = 46,

    IPPROTO_GRE = 47,

    IPPROTO_ESP = 50,

    IPPROTO_AH = 51,

    IPPROTO_ICMPV6 = 58,

    IPPROTO_NONE = 59,

    IPPROTO_DSTOPTS = 60,

    IPPROTO_MTP = 92,

    IPPROTO_ENCAP = 98,

    IPPROTO_PIM = 103,

    IPPROTO_COMP = 108,

    IPPROTO_SCTP = 132,

    IPPROTO_UDPLITE = 136,

    IPPROTO_RAW = 255,

    IPPROTO_MAX
  };



typedef uint16_t in_port_t;


enum
  {
    IPPORT_ECHO = 7,
    IPPORT_DISCARD = 9,
    IPPORT_SYSTAT = 11,
    IPPORT_DAYTIME = 13,
    IPPORT_NETSTAT = 15,
    IPPORT_FTP = 21,
    IPPORT_TELNET = 23,
    IPPORT_SMTP = 25,
    IPPORT_TIMESERVER = 37,
    IPPORT_NAMESERVER = 42,
    IPPORT_WHOIS = 43,
    IPPORT_MTP = 57,

    IPPORT_TFTP = 69,
    IPPORT_RJE = 77,
    IPPORT_FINGER = 79,
    IPPORT_TTYLINK = 87,
    IPPORT_SUPDUP = 95,


    IPPORT_EXECSERVER = 512,
    IPPORT_LOGINSERVER = 513,
    IPPORT_CMDSERVER = 514,
    IPPORT_EFSSERVER = 520,


    IPPORT_BIFFUDP = 512,
    IPPORT_WHOSERVER = 513,
    IPPORT_ROUTESERVER = 520,


    IPPORT_RESERVED = 1024,


    IPPORT_USERRESERVED = 5000
  };



typedef uint32_t in_addr_t;
struct in_addr
  {
    in_addr_t s_addr;
  };
/* # 198 "/usr/include/netinet/in.h" 3 4 */
struct in6_addr
  {
    union
      {
 uint8_t __u6_addr8[16];

 uint16_t __u6_addr16[8];
 uint32_t __u6_addr32[4];

      } __in6_u;





  };

extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;
/* # 225 "/usr/include/netinet/in.h" 3 4 */
struct sockaddr_in
  {
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;


    unsigned char sin_zero[sizeof (struct sockaddr) -
      (sizeof (unsigned short int)) -
      sizeof (in_port_t) -
      sizeof (struct in_addr)];
  };


struct sockaddr_in6
  {
    sa_family_t sin6_family;
    in_port_t sin6_port;
    uint32_t sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t sin6_scope_id;
  };




struct ip_mreq
  {

    struct in_addr imr_multiaddr;


    struct in_addr imr_interface;
  };

struct ip_mreq_source
  {

    struct in_addr imr_multiaddr;


    struct in_addr imr_interface;


    struct in_addr imr_sourceaddr;
  };




struct ipv6_mreq
  {

    struct in6_addr ipv6mr_multiaddr;


    unsigned int ipv6mr_interface;
  };




struct group_req
  {

    uint32_t gr_interface;


    struct sockaddr_storage gr_group;
  };

struct group_source_req
  {

    uint32_t gsr_interface;


    struct sockaddr_storage gsr_group;


    struct sockaddr_storage gsr_source;
  };



struct ip_msfilter
  {

    struct in_addr imsf_multiaddr;


    struct in_addr imsf_interface;


    uint32_t imsf_fmode;


    uint32_t imsf_numsrc;

    struct in_addr imsf_slist[1];
  };





struct group_filter
  {

    uint32_t gf_interface;


    struct sockaddr_storage gf_group;


    uint32_t gf_fmode;


    uint32_t gf_numsrc;

    struct sockaddr_storage gf_slist[1];
};
/* # 356 "/usr/include/netinet/in.h" 3 4 */
/* # 1 "/usr/include/bits/in.h" 1 3 4 */
/* # 99 "/usr/include/bits/in.h" 3 4 */
struct ip_opts
  {
    struct in_addr ip_dst;
    char ip_opts[40];
  };


struct ip_mreqn
  {
    struct in_addr imr_multiaddr;
    struct in_addr imr_address;
    int imr_ifindex;
  };


struct in_pktinfo
  {
    int ipi_ifindex;
    struct in_addr ipi_spec_dst;
    struct in_addr ipi_addr;
  };
/* # 357 "/usr/include/netinet/in.h" 2 3 4 */
/* # 365 "/usr/include/netinet/in.h" 3 4 */
extern uint32_t ntohl (uint32_t __netlong) __attribute__ ((__nothrow__)) __attribute__ ((__const__));
extern uint16_t ntohs (uint16_t __netshort)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__));
extern uint32_t htonl (uint32_t __hostlong)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__));
extern uint16_t htons (uint16_t __hostshort)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__));




/* # 1 "/usr/include/bits/byteswap.h" 1 3 4 */
/* # 377 "/usr/include/netinet/in.h" 2 3 4 */
/* # 440 "/usr/include/netinet/in.h" 3 4 */
extern int bindresvport (int __sockfd, struct sockaddr_in *__sock_in) __attribute__ ((__nothrow__));


extern int bindresvport6 (int __sockfd, struct sockaddr_in6 *__sock_in)
     __attribute__ ((__nothrow__));
/* # 565 "/usr/include/netinet/in.h" 3 4 */

/* # 71 "/usr/include/nspr4/prinet.h" 2 */
/* # 85 "/usr/include/nspr4/prinet.h" */
/* # 1 "/usr/include/arpa/inet.h" 1 3 4 */
/* # 31 "/usr/include/arpa/inet.h" 3 4 */




extern in_addr_t inet_addr (__const char *__cp) __attribute__ ((__nothrow__));


extern in_addr_t inet_lnaof (struct in_addr __in) __attribute__ ((__nothrow__));



extern struct in_addr inet_makeaddr (in_addr_t __net, in_addr_t __host)
     __attribute__ ((__nothrow__));


extern in_addr_t inet_netof (struct in_addr __in) __attribute__ ((__nothrow__));



extern in_addr_t inet_network (__const char *__cp) __attribute__ ((__nothrow__));



extern char *inet_ntoa (struct in_addr __in) __attribute__ ((__nothrow__));




extern int inet_pton (int __af, __const char *__restrict __cp,
        void *__restrict __buf) __attribute__ ((__nothrow__));




extern __const char *inet_ntop (int __af, __const void *__restrict __cp,
    char *__restrict __buf, socklen_t __len)
     __attribute__ ((__nothrow__));






extern int inet_aton (__const char *__cp, struct in_addr *__inp) __attribute__ ((__nothrow__));



extern char *inet_neta (in_addr_t __net, char *__buf, size_t __len) __attribute__ ((__nothrow__));




extern char *inet_net_ntop (int __af, __const void *__cp, int __bits,
       char *__buf, size_t __len) __attribute__ ((__nothrow__));




extern int inet_net_pton (int __af, __const char *__cp,
     void *__buf, size_t __len) __attribute__ ((__nothrow__));




extern unsigned int inet_nsap_addr (__const char *__cp,
        unsigned char *__buf, int __len) __attribute__ ((__nothrow__));



extern char *inet_nsap_ntoa (int __len, __const unsigned char *__cp,
        char *__buf) __attribute__ ((__nothrow__));



/* # 86 "/usr/include/nspr4/prinet.h" 2 */

/* # 1 "/usr/include/netdb.h" 1 3 4 */
/* # 33 "/usr/include/netdb.h" 3 4 */
/* # 1 "/usr/include/rpc/netdb.h" 1 3 4 */
/* # 42 "/usr/include/rpc/netdb.h" 3 4 */
/* # 1 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stddef.h" 1 3 4 */
/* # 43 "/usr/include/rpc/netdb.h" 2 3 4 */



struct rpcent
{
  char *r_name;
  char **r_aliases;
  int r_number;
};

extern void setrpcent (int __stayopen) __attribute__ ((__nothrow__));
extern void endrpcent (void) __attribute__ ((__nothrow__));
extern struct rpcent *getrpcbyname (__const char *__name) __attribute__ ((__nothrow__));
extern struct rpcent *getrpcbynumber (int __number) __attribute__ ((__nothrow__));
extern struct rpcent *getrpcent (void) __attribute__ ((__nothrow__));


extern int getrpcbyname_r (__const char *__name, struct rpcent *__result_buf,
      char *__buffer, size_t __buflen,
      struct rpcent **__result) __attribute__ ((__nothrow__));

extern int getrpcbynumber_r (int __number, struct rpcent *__result_buf,
        char *__buffer, size_t __buflen,
        struct rpcent **__result) __attribute__ ((__nothrow__));

extern int getrpcent_r (struct rpcent *__result_buf, char *__buffer,
   size_t __buflen, struct rpcent **__result) __attribute__ ((__nothrow__));



/* # 34 "/usr/include/netdb.h" 2 3 4 */
/* # 43 "/usr/include/netdb.h" 3 4 */
/* # 1 "/usr/include/bits/netdb.h" 1 3 4 */
/* # 27 "/usr/include/bits/netdb.h" 3 4 */
struct netent
{
  char *n_name;
  char **n_aliases;
  int n_addrtype;
  uint32_t n_net;
};
/* # 44 "/usr/include/netdb.h" 2 3 4 */
/* # 54 "/usr/include/netdb.h" 3 4 */








extern int *__h_errno_location (void) __attribute__ ((__nothrow__)) __attribute__ ((__const__));
/* # 93 "/usr/include/netdb.h" 3 4 */
extern void herror (__const char *__str) __attribute__ ((__nothrow__));


extern __const char *hstrerror (int __err_num) __attribute__ ((__nothrow__));




struct hostent
{
  char *h_name;
  char **h_aliases;
  int h_addrtype;
  int h_length;
  char **h_addr_list;



};






extern void sethostent (int __stay_open);





extern void endhostent (void);






extern struct hostent *gethostent (void);






extern struct hostent *gethostbyaddr (__const void *__addr, __socklen_t __len,
          int __type);





extern struct hostent *gethostbyname (__const char *__name);
/* # 156 "/usr/include/netdb.h" 3 4 */
extern struct hostent *gethostbyname2 (__const char *__name, int __af);
/* # 168 "/usr/include/netdb.h" 3 4 */
extern int gethostent_r (struct hostent *__restrict __result_buf,
    char *__restrict __buf, size_t __buflen,
    struct hostent **__restrict __result,
    int *__restrict __h_errnop);

extern int gethostbyaddr_r (__const void *__restrict __addr, __socklen_t __len,
       int __type,
       struct hostent *__restrict __result_buf,
       char *__restrict __buf, size_t __buflen,
       struct hostent **__restrict __result,
       int *__restrict __h_errnop);

extern int gethostbyname_r (__const char *__restrict __name,
       struct hostent *__restrict __result_buf,
       char *__restrict __buf, size_t __buflen,
       struct hostent **__restrict __result,
       int *__restrict __h_errnop);

extern int gethostbyname2_r (__const char *__restrict __name, int __af,
        struct hostent *__restrict __result_buf,
        char *__restrict __buf, size_t __buflen,
        struct hostent **__restrict __result,
        int *__restrict __h_errnop);
/* # 199 "/usr/include/netdb.h" 3 4 */
extern void setnetent (int __stay_open);





extern void endnetent (void);






extern struct netent *getnetent (void);






extern struct netent *getnetbyaddr (uint32_t __net, int __type);





extern struct netent *getnetbyname (__const char *__name);
/* # 238 "/usr/include/netdb.h" 3 4 */
extern int getnetent_r (struct netent *__restrict __result_buf,
   char *__restrict __buf, size_t __buflen,
   struct netent **__restrict __result,
   int *__restrict __h_errnop);

extern int getnetbyaddr_r (uint32_t __net, int __type,
      struct netent *__restrict __result_buf,
      char *__restrict __buf, size_t __buflen,
      struct netent **__restrict __result,
      int *__restrict __h_errnop);

extern int getnetbyname_r (__const char *__restrict __name,
      struct netent *__restrict __result_buf,
      char *__restrict __buf, size_t __buflen,
      struct netent **__restrict __result,
      int *__restrict __h_errnop);




struct servent
{
  char *s_name;
  char **s_aliases;
  int s_port;
  char *s_proto;
};






extern void setservent (int __stay_open);





extern void endservent (void);






extern struct servent *getservent (void);






extern struct servent *getservbyname (__const char *__name,
          __const char *__proto);






extern struct servent *getservbyport (int __port, __const char *__proto);
/* # 310 "/usr/include/netdb.h" 3 4 */
extern int getservent_r (struct servent *__restrict __result_buf,
    char *__restrict __buf, size_t __buflen,
    struct servent **__restrict __result);

extern int getservbyname_r (__const char *__restrict __name,
       __const char *__restrict __proto,
       struct servent *__restrict __result_buf,
       char *__restrict __buf, size_t __buflen,
       struct servent **__restrict __result);

extern int getservbyport_r (int __port, __const char *__restrict __proto,
       struct servent *__restrict __result_buf,
       char *__restrict __buf, size_t __buflen,
       struct servent **__restrict __result);




struct protoent
{
  char *p_name;
  char **p_aliases;
  int p_proto;
};






extern void setprotoent (int __stay_open);





extern void endprotoent (void);






extern struct protoent *getprotoent (void);





extern struct protoent *getprotobyname (__const char *__name);





extern struct protoent *getprotobynumber (int __proto);
/* # 376 "/usr/include/netdb.h" 3 4 */
extern int getprotoent_r (struct protoent *__restrict __result_buf,
     char *__restrict __buf, size_t __buflen,
     struct protoent **__restrict __result);

extern int getprotobyname_r (__const char *__restrict __name,
        struct protoent *__restrict __result_buf,
        char *__restrict __buf, size_t __buflen,
        struct protoent **__restrict __result);

extern int getprotobynumber_r (int __proto,
          struct protoent *__restrict __result_buf,
          char *__restrict __buf, size_t __buflen,
          struct protoent **__restrict __result);
/* # 397 "/usr/include/netdb.h" 3 4 */
extern int setnetgrent (__const char *__netgroup);







extern void endnetgrent (void);
/* # 414 "/usr/include/netdb.h" 3 4 */
extern int getnetgrent (char **__restrict __hostp,
   char **__restrict __userp,
   char **__restrict __domainp);
/* # 425 "/usr/include/netdb.h" 3 4 */
extern int innetgr (__const char *__netgroup, __const char *__host,
      __const char *__user, __const char *__domain);







extern int getnetgrent_r (char **__restrict __hostp,
     char **__restrict __userp,
     char **__restrict __domainp,
     char *__restrict __buffer, size_t __buflen);
/* # 453 "/usr/include/netdb.h" 3 4 */
extern int rcmd (char **__restrict __ahost, unsigned short int __rport,
   __const char *__restrict __locuser,
   __const char *__restrict __remuser,
   __const char *__restrict __cmd, int *__restrict __fd2p);
/* # 465 "/usr/include/netdb.h" 3 4 */
extern int rcmd_af (char **__restrict __ahost, unsigned short int __rport,
      __const char *__restrict __locuser,
      __const char *__restrict __remuser,
      __const char *__restrict __cmd, int *__restrict __fd2p,
      sa_family_t __af);
/* # 481 "/usr/include/netdb.h" 3 4 */
extern int rexec (char **__restrict __ahost, int __rport,
    __const char *__restrict __name,
    __const char *__restrict __pass,
    __const char *__restrict __cmd, int *__restrict __fd2p);
/* # 493 "/usr/include/netdb.h" 3 4 */
extern int rexec_af (char **__restrict __ahost, int __rport,
       __const char *__restrict __name,
       __const char *__restrict __pass,
       __const char *__restrict __cmd, int *__restrict __fd2p,
       sa_family_t __af);
/* # 507 "/usr/include/netdb.h" 3 4 */
extern int ruserok (__const char *__rhost, int __suser,
      __const char *__remuser, __const char *__locuser);
/* # 517 "/usr/include/netdb.h" 3 4 */
extern int ruserok_af (__const char *__rhost, int __suser,
         __const char *__remuser, __const char *__locuser,
         sa_family_t __af);
/* # 530 "/usr/include/netdb.h" 3 4 */
extern int iruserok (uint32_t __raddr, int __suser,
       __const char *__remuser, __const char *__locuser);
/* # 541 "/usr/include/netdb.h" 3 4 */
extern int iruserok_af (__const void *__raddr, int __suser,
   __const char *__remuser, __const char *__locuser,
   sa_family_t __af);
/* # 553 "/usr/include/netdb.h" 3 4 */
extern int rresvport (int *__alport);
/* # 562 "/usr/include/netdb.h" 3 4 */
extern int rresvport_af (int *__alport, sa_family_t __af);






struct addrinfo
{
  int ai_flags;
  int ai_family;
  int ai_socktype;
  int ai_protocol;
  socklen_t ai_addrlen;
  struct sockaddr *ai_addr;
  char *ai_canonname;
  struct addrinfo *ai_next;
};
/* # 664 "/usr/include/netdb.h" 3 4 */
extern int getaddrinfo (__const char *__restrict __name,
   __const char *__restrict __service,
   __const struct addrinfo *__restrict __req,
   struct addrinfo **__restrict __pai);


extern void freeaddrinfo (struct addrinfo *__ai) __attribute__ ((__nothrow__));


extern __const char *gai_strerror (int __ecode) __attribute__ ((__nothrow__));





extern int getnameinfo (__const struct sockaddr *__restrict __sa,
   socklen_t __salen, char *__restrict __host,
   socklen_t __hostlen, char *__restrict __serv,
   socklen_t __servlen, int __flags);
/* # 715 "/usr/include/netdb.h" 3 4 */

/* # 88 "/usr/include/nspr4/prinet.h" 2 */
/* # 50 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prinit.h" 1 */
/* # 41 "/usr/include/nspr4/prinit.h" */
/* # 1 "/usr/include/nspr4/prthread.h" 1 */
/* # 81 "/usr/include/nspr4/prthread.h" */


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
/* # 137 "/usr/include/nspr4/prthread.h" */
extern __attribute__((visibility("default"))) PRThread* PR_CreateThread(PRThreadType type,
                     void ( *start)(void *arg),
                     void *arg,
                     PRThreadPriority priority,
                     PRThreadScope scope,
                     PRThreadState state,
                     PRUint32 stackSize);
/* # 158 "/usr/include/nspr4/prthread.h" */
extern __attribute__((visibility("default"))) PRStatus PR_JoinThread(PRThread *thread);





extern __attribute__((visibility("default"))) PRThread* PR_GetCurrentThread(void);







extern __attribute__((visibility("default"))) PRThreadPriority PR_GetThreadPriority(const PRThread *thread);




extern __attribute__((visibility("default"))) void PR_SetThreadPriority(PRThread *thread, PRThreadPriority priority);
/* # 203 "/usr/include/nspr4/prthread.h" */
typedef void ( *PRThreadPrivateDTOR)(void *priv);

extern __attribute__((visibility("default"))) PRStatus PR_NewThreadPrivateIndex(
    PRUintn *newIndex, PRThreadPrivateDTOR destructor);
/* # 219 "/usr/include/nspr4/prthread.h" */
extern __attribute__((visibility("default"))) PRStatus PR_SetThreadPrivate(PRUintn tpdIndex, void *priv);
/* # 230 "/usr/include/nspr4/prthread.h" */
extern __attribute__((visibility("default"))) void* PR_GetThreadPrivate(PRUintn tpdIndex);
/* # 242 "/usr/include/nspr4/prthread.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Interrupt(PRThread *thread);





extern __attribute__((visibility("default"))) void PR_ClearInterrupt(void);




extern __attribute__((visibility("default"))) void PR_BlockInterrupt(void);




extern __attribute__((visibility("default"))) void PR_UnblockInterrupt(void);
/* # 267 "/usr/include/nspr4/prthread.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Sleep(PRIntervalTime ticks);




extern __attribute__((visibility("default"))) PRThreadScope PR_GetThreadScope(const PRThread *thread);




extern __attribute__((visibility("default"))) PRThreadType PR_GetThreadType(const PRThread *thread);




extern __attribute__((visibility("default"))) PRThreadState PR_GetThreadState(const PRThread *thread);


/* # 42 "/usr/include/nspr4/prinit.h" 2 */

/* # 1 "/usr/include/nspr4/prwin16.h" 1 */
/* # 44 "/usr/include/nspr4/prinit.h" 2 */
/* # 1 "/usr/include/stdio.h" 1 3 4 */
/* # 30 "/usr/include/stdio.h" 3 4 */




/* # 1 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stddef.h" 1 3 4 */
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
/* # 1 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stddef.h" 1 3 4 */
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
/* # 1 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stdarg.h" 1 3 4 */
/* # 40 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stdarg.h" 3 4 */
typedef __builtin_va_list __gnuc_va_list;
/* # 54 "/usr/include/libio.h" 2 3 4 */
/* # 172 "/usr/include/libio.h" 3 4 */
struct _IO_jump_t; struct _IO_FILE;
/* # 182 "/usr/include/libio.h" 3 4 */
typedef void _IO_lock_t;





struct _IO_marker {
  struct _IO_marker *_next;
  struct _IO_FILE *_sbuf;



  int _pos;
/* # 205 "/usr/include/libio.h" 3 4 */
};


enum __codecvt_result
{
  __codecvt_ok,
  __codecvt_partial,
  __codecvt_error,
  __codecvt_noconv
};
/* # 273 "/usr/include/libio.h" 3 4 */
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
/* # 321 "/usr/include/libio.h" 3 4 */
  __off64_t _offset;
/* # 330 "/usr/include/libio.h" 3 4 */
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
/* # 366 "/usr/include/libio.h" 3 4 */
typedef __ssize_t __io_read_fn (void *__cookie, char *__buf, size_t __nbytes);







typedef __ssize_t __io_write_fn (void *__cookie, __const char *__buf,
     size_t __n);







typedef int __io_seek_fn (void *__cookie, __off64_t *__pos, int __w);


typedef int __io_close_fn (void *__cookie);
/* # 418 "/usr/include/libio.h" 3 4 */
extern int __underflow (_IO_FILE *);
extern int __uflow (_IO_FILE *);
extern int __overflow (_IO_FILE *, int);
/* # 462 "/usr/include/libio.h" 3 4 */
extern int _IO_getc (_IO_FILE *__fp);
extern int _IO_putc (int __c, _IO_FILE *__fp);
extern int _IO_feof (_IO_FILE *__fp) __attribute__ ((__nothrow__));
extern int _IO_ferror (_IO_FILE *__fp) __attribute__ ((__nothrow__));

extern int _IO_peekc_locked (_IO_FILE *__fp);





extern void _IO_flockfile (_IO_FILE *) __attribute__ ((__nothrow__));
extern void _IO_funlockfile (_IO_FILE *) __attribute__ ((__nothrow__));
extern int _IO_ftrylockfile (_IO_FILE *) __attribute__ ((__nothrow__));
/* # 492 "/usr/include/libio.h" 3 4 */
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




/* # 165 "/usr/include/stdio.h" 3 4 */
/* # 1 "/usr/include/bits/stdio_lim.h" 1 3 4 */
/* # 166 "/usr/include/stdio.h" 2 3 4 */



extern struct _IO_FILE *stdin;
extern struct _IO_FILE *stdout;
extern struct _IO_FILE *stderr;









extern int remove (__const char *__filename) __attribute__ ((__nothrow__));

extern int rename (__const char *__old, __const char *__new) __attribute__ ((__nothrow__));














extern FILE *tmpfile (void) ;
/* # 212 "/usr/include/stdio.h" 3 4 */
extern char *tmpnam (char *__s) __attribute__ ((__nothrow__)) ;





extern char *tmpnam_r (char *__s) __attribute__ ((__nothrow__)) ;
/* # 230 "/usr/include/stdio.h" 3 4 */
extern char *tempnam (__const char *__dir, __const char *__pfx)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;








extern int fclose (FILE *__stream);




extern int fflush (FILE *__stream);

/* # 255 "/usr/include/stdio.h" 3 4 */
extern int fflush_unlocked (FILE *__stream);
/* # 269 "/usr/include/stdio.h" 3 4 */






extern FILE *fopen (__const char *__restrict __filename,
      __const char *__restrict __modes) ;




extern FILE *freopen (__const char *__restrict __filename,
        __const char *__restrict __modes,
        FILE *__restrict __stream) ;
/* # 298 "/usr/include/stdio.h" 3 4 */

/* # 309 "/usr/include/stdio.h" 3 4 */
extern FILE *fdopen (int __fd, __const char *__modes) __attribute__ ((__nothrow__)) ;
/* # 332 "/usr/include/stdio.h" 3 4 */



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

/* # 428 "/usr/include/stdio.h" 3 4 */





extern int fscanf (FILE *__restrict __stream,
     __const char *__restrict __format, ...) ;




extern int scanf (__const char *__restrict __format, ...) ;

extern int sscanf (__const char *__restrict __s,
     __const char *__restrict __format, ...) __attribute__ ((__nothrow__));
/* # 471 "/usr/include/stdio.h" 3 4 */

/* # 534 "/usr/include/stdio.h" 3 4 */





extern int fgetc (FILE *__stream);
extern int getc (FILE *__stream);





extern int getchar (void);

/* # 558 "/usr/include/stdio.h" 3 4 */
extern int getc_unlocked (FILE *__stream);
extern int getchar_unlocked (void);
/* # 569 "/usr/include/stdio.h" 3 4 */
extern int fgetc_unlocked (FILE *__stream);











extern int fputc (int __c, FILE *__stream);
extern int putc (int __c, FILE *__stream);





extern int putchar (int __c);

/* # 602 "/usr/include/stdio.h" 3 4 */
extern int fputc_unlocked (int __c, FILE *__stream);







extern int putc_unlocked (int __c, FILE *__stream);
extern int putchar_unlocked (int __c);






extern int getw (FILE *__stream);


extern int putw (int __w, FILE *__stream);








extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
     ;






extern char *gets (char *__s) ;

/* # 683 "/usr/include/stdio.h" 3 4 */





extern int fputs (__const char *__restrict __s, FILE *__restrict __stream);





extern int puts (__const char *__s);






extern int ungetc (int __c, FILE *__stream);






extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream) ;




extern size_t fwrite (__const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s) ;

/* # 736 "/usr/include/stdio.h" 3 4 */
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
         size_t __n, FILE *__restrict __stream) ;
extern size_t fwrite_unlocked (__const void *__restrict __ptr, size_t __size,
          size_t __n, FILE *__restrict __stream) ;








extern int fseek (FILE *__stream, long int __off, int __whence);




extern long int ftell (FILE *__stream) ;




extern void rewind (FILE *__stream);

/* # 791 "/usr/include/stdio.h" 3 4 */






extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos);




extern int fsetpos (FILE *__stream, __const fpos_t *__pos);
/* # 814 "/usr/include/stdio.h" 3 4 */

/* # 823 "/usr/include/stdio.h" 3 4 */


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
/* # 853 "/usr/include/stdio.h" 2 3 4 */




extern int fileno (FILE *__stream) __attribute__ ((__nothrow__)) ;




extern int fileno_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) ;
/* # 872 "/usr/include/stdio.h" 3 4 */
extern FILE *popen (__const char *__command, __const char *__modes) ;





extern int pclose (FILE *__stream);





extern char *ctermid (char *__s) __attribute__ ((__nothrow__));





extern char *cuserid (char *__s);
/* # 912 "/usr/include/stdio.h" 3 4 */
extern void flockfile (FILE *__stream) __attribute__ ((__nothrow__));



extern int ftrylockfile (FILE *__stream) __attribute__ ((__nothrow__)) ;


extern void funlockfile (FILE *__stream) __attribute__ ((__nothrow__));







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







  extern int getopt (int ___argc, char *const *___argv, const char *__shortopts) __asm__ ("" "__posix_getopt") __attribute__ ((__nothrow__))

                       ;
/* # 928 "/usr/include/stdio.h" 2 3 4 */





/* # 1 "/usr/include/bits/stdio.h" 1 3 4 */
/* # 36 "/usr/include/bits/stdio.h" 3 4 */
extern __inline int
vprintf (__const char *__restrict __fmt, __gnuc_va_list __arg)
{
  return vfprintf (stdout, __fmt, __arg);
}



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
/* # 125 "/usr/include/bits/stdio.h" 3 4 */
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
/* # 934 "/usr/include/stdio.h" 2 3 4 */
/* # 942 "/usr/include/stdio.h" 3 4 */

/* # 45 "/usr/include/nspr4/prinit.h" 2 */


/* # 88 "/usr/include/nspr4/prinit.h" */
typedef PRBool (*PRVersionCheck)(const char*);
/* # 98 "/usr/include/nspr4/prinit.h" */
extern __attribute__((visibility("default"))) PRBool PR_VersionCheck(const char *importedVersion);




extern __attribute__((visibility("default"))) const char* PR_GetVersion(void);
/* # 116 "/usr/include/nspr4/prinit.h" */
extern __attribute__((visibility("default"))) void PR_Init(
    PRThreadType type, PRThreadPriority priority, PRUintn maxPTDs);
/* # 137 "/usr/include/nspr4/prinit.h" */
typedef PRIntn ( *PRPrimordialFn)(PRIntn argc, char **argv);

extern __attribute__((visibility("default"))) PRIntn PR_Initialize(
    PRPrimordialFn prmain, PRIntn argc, char **argv, PRUintn maxPTDs);




extern __attribute__((visibility("default"))) PRBool PR_Initialized(void);
/* # 163 "/usr/include/nspr4/prinit.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Cleanup(void);





extern __attribute__((visibility("default"))) void PR_DisableClockInterrupts(void);





extern __attribute__((visibility("default"))) void PR_EnableClockInterrupts(void);





extern __attribute__((visibility("default"))) void PR_BlockClockInterrupts(void);





extern __attribute__((visibility("default"))) void PR_UnblockClockInterrupts(void);




extern __attribute__((visibility("default"))) void PR_SetConcurrency(PRUintn numCPUs);






extern __attribute__((visibility("default"))) PRStatus PR_SetFDCacheSize(PRIntn low, PRIntn high);






extern __attribute__((visibility("default"))) void PR_ProcessExit(PRIntn status);






extern __attribute__((visibility("default"))) void PR_Abort(void);
/* # 223 "/usr/include/nspr4/prinit.h" */
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



/* # 51 "/usr/include/nspr4/nspr.h" 2 */

/* # 1 "/usr/include/nspr4/prio.h" 1 */
/* # 48 "/usr/include/nspr4/prio.h" */
/* # 1 "/usr/include/nspr4/prlong.h" 1 */
/* # 51 "/usr/include/nspr4/prlong.h" */

/* # 62 "/usr/include/nspr4/prlong.h" */
extern __attribute__((visibility("default"))) PRInt64 LL_MaxInt(void);
extern __attribute__((visibility("default"))) PRInt64 LL_MinInt(void);
extern __attribute__((visibility("default"))) PRInt64 LL_Zero(void);
extern __attribute__((visibility("default"))) PRUint64 LL_MaxUint(void);
/* # 433 "/usr/include/nspr4/prlong.h" */

/* # 49 "/usr/include/nspr4/prio.h" 2 */
/* # 1 "/usr/include/nspr4/prtime.h" 1 */
/* # 53 "/usr/include/nspr4/prtime.h" */

/* # 80 "/usr/include/nspr4/prtime.h" */
typedef PRInt64 PRTime;






typedef struct PRTimeParameters {
    PRInt32 tp_gmt_offset;
    PRInt32 tp_dst_offset;
} PRTimeParameters;
/* # 115 "/usr/include/nspr4/prtime.h" */
typedef struct PRExplodedTime {
    PRInt32 tm_usec;
    PRInt32 tm_sec;

    PRInt32 tm_min;
    PRInt32 tm_hour;
    PRInt32 tm_mday;

    PRInt32 tm_month;
    PRInt16 tm_year;


    PRInt8 tm_wday;

    PRInt16 tm_yday;


    PRTimeParameters tm_params;
} PRExplodedTime;
/* # 158 "/usr/include/nspr4/prtime.h" */
typedef PRTimeParameters ( *PRTimeParamFn)(const PRExplodedTime *gmt);
/* # 177 "/usr/include/nspr4/prtime.h" */
extern __attribute__((visibility("default"))) PRTime
PR_Now(void);
/* # 190 "/usr/include/nspr4/prtime.h" */
extern __attribute__((visibility("default"))) void PR_ExplodeTime(
    PRTime usecs, PRTimeParamFn params, PRExplodedTime *exploded);


extern __attribute__((visibility("default"))) PRTime
PR_ImplodeTime(const PRExplodedTime *exploded);
/* # 209 "/usr/include/nspr4/prtime.h" */
extern __attribute__((visibility("default"))) void PR_NormalizeTime(
    PRExplodedTime *exploded, PRTimeParamFn params);






extern __attribute__((visibility("default"))) PRTimeParameters PR_LocalTimeParameters(const PRExplodedTime *gmt);


extern __attribute__((visibility("default"))) PRTimeParameters PR_GMTParameters(const PRExplodedTime *gmt);





extern __attribute__((visibility("default"))) PRTimeParameters PR_USPacificTimeParameters(const PRExplodedTime *gmt);
/* # 263 "/usr/include/nspr4/prtime.h" */
extern __attribute__((visibility("default"))) PRStatus PR_ParseTimeStringToExplodedTime (
        const char *string,
        PRBool default_to_gmt,
        PRExplodedTime *result);
/* # 276 "/usr/include/nspr4/prtime.h" */
extern __attribute__((visibility("default"))) PRStatus PR_ParseTimeString (
 const char *string,
 PRBool default_to_gmt,
 PRTime *result);
/* # 293 "/usr/include/nspr4/prtime.h" */
extern __attribute__((visibility("default"))) PRUint32 PR_FormatTime(char *buf, int buflen, const char *fmt,
                                           const PRExplodedTime *tm);




extern __attribute__((visibility("default"))) PRUint32
PR_FormatTimeUSEnglish( char* buf, PRUint32 bufSize,
                        const char* format, const PRExplodedTime* tm );




/* # 50 "/usr/include/nspr4/prio.h" 2 */






typedef struct PRDir PRDir;
typedef struct PRDirEntry PRDirEntry;




typedef struct PRFileDesc PRFileDesc;
typedef struct PRFileInfo PRFileInfo;
typedef struct PRFileInfo64 PRFileInfo64;
typedef union PRNetAddr PRNetAddr;
typedef struct PRIOMethods PRIOMethods;
typedef struct PRPollDesc PRPollDesc;
typedef struct PRFilePrivate PRFilePrivate;
typedef struct PRSendFileData PRSendFileData;
/* # 82 "/usr/include/nspr4/prio.h" */
typedef PRIntn PRDescIdentity;

struct PRFileDesc {
    const PRIOMethods *methods;
    PRFilePrivate *secret;
    PRFileDesc *lower, *higher;
    void ( *dtor)(PRFileDesc *fd);

    PRDescIdentity identity;
};
/* # 102 "/usr/include/nspr4/prio.h" */
typedef enum PRTransmitFileFlags {
    PR_TRANSMITFILE_KEEP_OPEN = 0,

    PR_TRANSMITFILE_CLOSE_SOCKET = 1

} PRTransmitFileFlags;
/* # 162 "/usr/include/nspr4/prio.h" */
struct PRIPv6Addr {
 union {
  PRUint8 _S6_u8[16];
  PRUint16 _S6_u16[8];
  PRUint32 _S6_u32[4];
  PRUint64 _S6_u64[2];
 } _S6_un;
};





typedef struct PRIPv6Addr PRIPv6Addr;

union PRNetAddr {
    struct {
        PRUint16 family;



        char data[14];

    } raw;
    struct {
        PRUint16 family;
        PRUint16 port;
        PRUint32 ip;



        char pad[8];

    } inet;
    struct {
        PRUint16 family;
        PRUint16 port;
        PRUint32 flowinfo;
        PRIPv6Addr ip;
        PRUint32 scope_id;
    } ipv6;

    struct {
        PRUint16 family;




        char path[104];

    } local;

};
/* # 225 "/usr/include/nspr4/prio.h" */
typedef enum PRSockOption
{
    PR_SockOpt_Nonblocking,
    PR_SockOpt_Linger,
    PR_SockOpt_Reuseaddr,
    PR_SockOpt_Keepalive,
    PR_SockOpt_RecvBufferSize,
    PR_SockOpt_SendBufferSize,

    PR_SockOpt_IpTimeToLive,
    PR_SockOpt_IpTypeOfService,

    PR_SockOpt_AddMember,
    PR_SockOpt_DropMember,
    PR_SockOpt_McastInterface,
    PR_SockOpt_McastTimeToLive,
    PR_SockOpt_McastLoopback,

    PR_SockOpt_NoDelay,
    PR_SockOpt_MaxSegment,
    PR_SockOpt_Broadcast,
    PR_SockOpt_Last
} PRSockOption;

typedef struct PRLinger {
 PRBool polarity;
 PRIntervalTime linger;
} PRLinger;

typedef struct PRMcastRequest {
 PRNetAddr mcaddr;
 PRNetAddr ifaddr;
} PRMcastRequest;

typedef struct PRSocketOptionData
{
    PRSockOption option;
    union
    {
        PRUintn ip_ttl;
        PRUintn mcast_ttl;
        PRUintn tos;
        PRBool non_blocking;
        PRBool reuse_addr;
        PRBool keep_alive;
        PRBool mcast_loopback;
        PRBool no_delay;
        PRBool broadcast;
        PRSize max_segment;
        PRSize recv_buffer_size;
        PRSize send_buffer_size;
        PRLinger linger;
        PRMcastRequest add_member;
        PRMcastRequest drop_member;
        PRNetAddr mcast_if;
    } value;
} PRSocketOptionData;
/* # 291 "/usr/include/nspr4/prio.h" */
typedef struct PRIOVec {
    char *iov_base;
    int iov_len;
} PRIOVec;






typedef enum PRDescType
{
    PR_DESC_FILE = 1,
    PR_DESC_SOCKET_TCP = 2,
    PR_DESC_SOCKET_UDP = 3,
    PR_DESC_LAYERED = 4,
    PR_DESC_PIPE = 5
} PRDescType;

typedef enum PRSeekWhence {
    PR_SEEK_SET = 0,
    PR_SEEK_CUR = 1,
    PR_SEEK_END = 2
} PRSeekWhence;

extern __attribute__((visibility("default"))) PRDescType PR_GetDescType(PRFileDesc *file);
/* # 334 "/usr/include/nspr4/prio.h" */
typedef PRStatus ( *PRCloseFN)(PRFileDesc *fd);
typedef PRInt32 ( *PRReadFN)(PRFileDesc *fd, void *buf, PRInt32 amount);
typedef PRInt32 ( *PRWriteFN)(PRFileDesc *fd, const void *buf, PRInt32 amount);
typedef PRInt32 ( *PRAvailableFN)(PRFileDesc *fd);
typedef PRInt64 ( *PRAvailable64FN)(PRFileDesc *fd);
typedef PRStatus ( *PRFsyncFN)(PRFileDesc *fd);
typedef PROffset32 ( *PRSeekFN)(PRFileDesc *fd, PROffset32 offset, PRSeekWhence how);
typedef PROffset64 ( *PRSeek64FN)(PRFileDesc *fd, PROffset64 offset, PRSeekWhence how);
typedef PRStatus ( *PRFileInfoFN)(PRFileDesc *fd, PRFileInfo *info);
typedef PRStatus ( *PRFileInfo64FN)(PRFileDesc *fd, PRFileInfo64 *info);
typedef PRInt32 ( *PRWritevFN)(
    PRFileDesc *fd, const PRIOVec *iov, PRInt32 iov_size,
    PRIntervalTime timeout);
typedef PRStatus ( *PRConnectFN)(
    PRFileDesc *fd, const PRNetAddr *addr, PRIntervalTime timeout);
typedef PRFileDesc* ( *PRAcceptFN) (
    PRFileDesc *fd, PRNetAddr *addr, PRIntervalTime timeout);
typedef PRStatus ( *PRBindFN)(PRFileDesc *fd, const PRNetAddr *addr);
typedef PRStatus ( *PRListenFN)(PRFileDesc *fd, PRIntn backlog);
typedef PRStatus ( *PRShutdownFN)(PRFileDesc *fd, PRIntn how);
typedef PRInt32 ( *PRRecvFN)(
    PRFileDesc *fd, void *buf, PRInt32 amount,
    PRIntn flags, PRIntervalTime timeout);
typedef PRInt32 ( *PRSendFN) (
    PRFileDesc *fd, const void *buf, PRInt32 amount,
    PRIntn flags, PRIntervalTime timeout);
typedef PRInt32 ( *PRRecvfromFN)(
    PRFileDesc *fd, void *buf, PRInt32 amount,
    PRIntn flags, PRNetAddr *addr, PRIntervalTime timeout);
typedef PRInt32 ( *PRSendtoFN)(
    PRFileDesc *fd, const void *buf, PRInt32 amount,
    PRIntn flags, const PRNetAddr *addr, PRIntervalTime timeout);
typedef PRInt16 ( *PRPollFN)(
    PRFileDesc *fd, PRInt16 in_flags, PRInt16 *out_flags);
typedef PRInt32 ( *PRAcceptreadFN)(
    PRFileDesc *sd, PRFileDesc **nd, PRNetAddr **raddr,
    void *buf, PRInt32 amount, PRIntervalTime t);
typedef PRInt32 ( *PRTransmitfileFN)(
     PRFileDesc *sd, PRFileDesc *fd, const void *headers,
     PRInt32 hlen, PRTransmitFileFlags flags, PRIntervalTime t);
typedef PRStatus ( *PRGetsocknameFN)(PRFileDesc *fd, PRNetAddr *addr);
typedef PRStatus ( *PRGetpeernameFN)(PRFileDesc *fd, PRNetAddr *addr);
typedef PRStatus ( *PRGetsocketoptionFN)(
    PRFileDesc *fd, PRSocketOptionData *data);
typedef PRStatus ( *PRSetsocketoptionFN)(
    PRFileDesc *fd, const PRSocketOptionData *data);
typedef PRInt32 ( *PRSendfileFN)(
 PRFileDesc *networkSocket, PRSendFileData *sendData,
 PRTransmitFileFlags flags, PRIntervalTime timeout);
typedef PRStatus ( *PRConnectcontinueFN)(
    PRFileDesc *fd, PRInt16 out_flags);
typedef PRIntn ( *PRReservedFN)(PRFileDesc *fd);

struct PRIOMethods {
    PRDescType file_type;
    PRCloseFN close;
    PRReadFN read;
    PRWriteFN write;
    PRAvailableFN available;
    PRAvailable64FN available64;
    PRFsyncFN fsync;
    PRSeekFN seek;
    PRSeek64FN seek64;
    PRFileInfoFN fileInfo;
    PRFileInfo64FN fileInfo64;
    PRWritevFN writev;
    PRConnectFN connect;
    PRAcceptFN accept;
    PRBindFN bind;
    PRListenFN listen;
    PRShutdownFN shutdown;
    PRRecvFN recv;
    PRSendFN send;
    PRRecvfromFN recvfrom;
    PRSendtoFN sendto;
    PRPollFN poll;
    PRAcceptreadFN acceptread;
    PRTransmitfileFN transmitfile;
    PRGetsocknameFN getsockname;
    PRGetpeernameFN getpeername;
    PRReservedFN reserved_fn_6;
    PRReservedFN reserved_fn_5;
    PRGetsocketoptionFN getsocketoption;

    PRSetsocketoptionFN setsocketoption;

    PRSendfileFN sendfile;
    PRConnectcontinueFN connectcontinue;

    PRReservedFN reserved_fn_3;
    PRReservedFN reserved_fn_2;
    PRReservedFN reserved_fn_1;
    PRReservedFN reserved_fn_0;
};
/* # 448 "/usr/include/nspr4/prio.h" */
typedef enum PRSpecialFD
{
    PR_StandardInput,
    PR_StandardOutput,
    PR_StandardError
} PRSpecialFD;

extern __attribute__((visibility("default"))) PRFileDesc* PR_GetSpecialFD(PRSpecialFD id);
/* # 495 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRDescIdentity PR_GetUniqueIdentity(const char *layer_name);
extern __attribute__((visibility("default"))) const char* PR_GetNameForIdentity(PRDescIdentity ident);
extern __attribute__((visibility("default"))) PRDescIdentity PR_GetLayersIdentity(PRFileDesc* fd);
extern __attribute__((visibility("default"))) PRFileDesc* PR_GetIdentitiesLayer(PRFileDesc* fd_stack, PRDescIdentity id);
/* # 508 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) const PRIOMethods * PR_GetDefaultIOMethods(void);
/* # 519 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRFileDesc* PR_CreateIOLayerStub(
    PRDescIdentity ident, const PRIOMethods *methods);
/* # 536 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRFileDesc* PR_CreateIOLayer(PRFileDesc* fd);
/* # 552 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_PushIOLayer(
    PRFileDesc *fd_stack, PRDescIdentity id, PRFileDesc *layer);
/* # 569 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRFileDesc* PR_PopIOLayer(PRFileDesc *fd_stack, PRDescIdentity id);
/* # 643 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRFileDesc* PR_Open(const char *name, PRIntn flags, PRIntn mode);
/* # 669 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRFileDesc* PR_OpenFile(
    const char *name, PRIntn flags, PRIntn mode);
/* # 700 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Close(PRFileDesc *fd);
/* # 735 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_Read(PRFileDesc *fd, void *buf, PRInt32 amount);
/* # 759 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_Write(PRFileDesc *fd,const void *buf,PRInt32 amount);
/* # 793 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_Writev(
    PRFileDesc *fd, const PRIOVec *iov, PRInt32 iov_size,
    PRIntervalTime timeout);
/* # 814 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Delete(const char *name);



typedef enum PRFileType
{
    PR_FILE_FILE = 1,
    PR_FILE_DIRECTORY = 2,
    PR_FILE_OTHER = 3
} PRFileType;

struct PRFileInfo {
    PRFileType type;
    PROffset32 size;
    PRTime creationTime;
    PRTime modifyTime;
};

struct PRFileInfo64 {
    PRFileType type;
    PROffset64 size;
    PRTime creationTime;
    PRTime modifyTime;
};
/* # 857 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_GetFileInfo(const char *fn, PRFileInfo *info);
extern __attribute__((visibility("default"))) PRStatus PR_GetFileInfo64(const char *fn, PRFileInfo64 *info);
/* # 884 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_GetOpenFileInfo(PRFileDesc *fd, PRFileInfo *info);
extern __attribute__((visibility("default"))) PRStatus PR_GetOpenFileInfo64(PRFileDesc *fd, PRFileInfo64 *info);
/* # 903 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Rename(const char *from, const char *to);
/* # 929 "/usr/include/nspr4/prio.h" */
typedef enum PRAccessHow {
    PR_ACCESS_EXISTS = 1,
    PR_ACCESS_WRITE_OK = 2,
    PR_ACCESS_READ_OK = 3
} PRAccessHow;

extern __attribute__((visibility("default"))) PRStatus PR_Access(const char *name, PRAccessHow how);
/* # 970 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PROffset32 PR_Seek(PRFileDesc *fd, PROffset32 offset, PRSeekWhence whence);
extern __attribute__((visibility("default"))) PROffset64 PR_Seek64(PRFileDesc *fd, PROffset64 offset, PRSeekWhence whence);
/* # 993 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_Available(PRFileDesc *fd);
extern __attribute__((visibility("default"))) PRInt64 PR_Available64(PRFileDesc *fd);
/* # 1013 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Sync(PRFileDesc *fd);



struct PRDirEntry {
    const char *name;
};
/* # 1052 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRDir* PR_OpenDir(const char *name);
/* # 1084 "/usr/include/nspr4/prio.h" */
typedef enum PRDirFlags {
    PR_SKIP_NONE = 0x0,
    PR_SKIP_DOT = 0x1,
    PR_SKIP_DOT_DOT = 0x2,
    PR_SKIP_BOTH = 0x3,
    PR_SKIP_HIDDEN = 0x4
} PRDirFlags;

extern __attribute__((visibility("default"))) PRDirEntry* PR_ReadDir(PRDir *dir, PRDirFlags flags);
/* # 1118 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_CloseDir(PRDir *dir);
/* # 1147 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_MkDir(const char *name, PRIntn mode);
/* # 1159 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_MakeDir(const char *name, PRIntn mode);
/* # 1179 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_RmDir(const char *name);
/* # 1198 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRFileDesc* PR_NewUDPSocket(void);
/* # 1217 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRFileDesc* PR_NewTCPSocket(void);
/* # 1237 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRFileDesc* PR_OpenUDPSocket(PRIntn af);
/* # 1257 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRFileDesc* PR_OpenTCPSocket(PRIntn af);
/* # 1285 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Connect(
    PRFileDesc *fd, const PRNetAddr *addr, PRIntervalTime timeout);
/* # 1320 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_ConnectContinue(PRFileDesc *fd, PRInt16 out_flags);
/* # 1350 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_GetConnectStatus(const PRPollDesc *pd);
/* # 1374 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRFileDesc* PR_Accept(
    PRFileDesc *fd, PRNetAddr *addr, PRIntervalTime timeout);
/* # 1396 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Bind(PRFileDesc *fd, const PRNetAddr *addr);
/* # 1418 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_Listen(PRFileDesc *fd, PRIntn backlog);
/* # 1442 "/usr/include/nspr4/prio.h" */
typedef enum PRShutdownHow
{
    PR_SHUTDOWN_RCV = 0,
    PR_SHUTDOWN_SEND = 1,
    PR_SHUTDOWN_BOTH = 2
} PRShutdownHow;

extern __attribute__((visibility("default"))) PRStatus PR_Shutdown(PRFileDesc *fd, PRShutdownHow how);
/* # 1482 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_Recv(PRFileDesc *fd, void *buf, PRInt32 amount,
                PRIntn flags, PRIntervalTime timeout);
/* # 1513 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_Send(PRFileDesc *fd, const void *buf, PRInt32 amount,
                                PRIntn flags, PRIntervalTime timeout);
/* # 1548 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_RecvFrom(
    PRFileDesc *fd, void *buf, PRInt32 amount, PRIntn flags,
    PRNetAddr *addr, PRIntervalTime timeout);
/* # 1581 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_SendTo(
    PRFileDesc *fd, const void *buf, PRInt32 amount, PRIntn flags,
    const PRNetAddr *addr, PRIntervalTime timeout);
/* # 1621 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_TransmitFile(
    PRFileDesc *networkSocket, PRFileDesc *sourceFile,
    const void *headers, PRInt32 hlen, PRTransmitFileFlags flags,
    PRIntervalTime timeout);
/* # 1660 "/usr/include/nspr4/prio.h" */
struct PRSendFileData {
 PRFileDesc *fd;
 PRUint32 file_offset;
 PRSize file_nbytes;


 const void *header;
 PRInt32 hlen;
 const void *trailer;
 PRInt32 tlen;
};


extern __attribute__((visibility("default"))) PRInt32 PR_SendFile(
    PRFileDesc *networkSocket, PRSendFileData *sendData,
 PRTransmitFileFlags flags, PRIntervalTime timeout);
/* # 1727 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_AcceptRead(
    PRFileDesc *listenSock, PRFileDesc **acceptedSock,
    PRNetAddr **peerAddr, void *buf, PRInt32 amount, PRIntervalTime timeout);
/* # 1751 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_NewTCPSocketPair(PRFileDesc *fds[2]);
/* # 1771 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_GetSockName(PRFileDesc *fd, PRNetAddr *addr);
/* # 1793 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_GetPeerName(PRFileDesc *fd, PRNetAddr *addr);

extern __attribute__((visibility("default"))) PRStatus PR_GetSocketOption(
    PRFileDesc *fd, PRSocketOptionData *data);

extern __attribute__((visibility("default"))) PRStatus PR_SetSocketOption(
    PRFileDesc *fd, const PRSocketOptionData *data);
/* # 1828 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_SetFDInheritable(
    PRFileDesc *fd,
    PRBool inheritable);
/* # 1848 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRFileDesc * PR_GetInheritedFD(const char *name);
/* # 1858 "/usr/include/nspr4/prio.h" */
typedef struct PRFileMap PRFileMap;




typedef enum PRFileMapProtect {
    PR_PROT_READONLY,
    PR_PROT_READWRITE,
    PR_PROT_WRITECOPY
} PRFileMapProtect;

extern __attribute__((visibility("default"))) PRFileMap * PR_CreateFileMap(
    PRFileDesc *fd,
    PRInt64 size,
    PRFileMapProtect prot);




extern __attribute__((visibility("default"))) PRInt32 PR_GetMemMapAlignment(void);

extern __attribute__((visibility("default"))) void * PR_MemMap(
    PRFileMap *fmap,
    PROffset64 offset,

    PRUint32 len);

extern __attribute__((visibility("default"))) PRStatus PR_MemUnmap(void *addr, PRUint32 len);

extern __attribute__((visibility("default"))) PRStatus PR_CloseFileMap(PRFileMap *fmap);
/* # 1902 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRStatus PR_CreatePipe(
    PRFileDesc **readPipe,
    PRFileDesc **writePipe
);





struct PRPollDesc {
    PRFileDesc* fd;
    PRInt16 in_flags;
    PRInt16 out_flags;
};
/* # 1983 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_Poll(
    PRPollDesc *pds, PRIntn npds, PRIntervalTime timeout);
/* # 2027 "/usr/include/nspr4/prio.h" */
extern __attribute__((visibility("default"))) PRFileDesc * PR_NewPollableEvent(void);

extern __attribute__((visibility("default"))) PRStatus PR_DestroyPollableEvent(PRFileDesc *event);

extern __attribute__((visibility("default"))) PRStatus PR_SetPollableEvent(PRFileDesc *event);

extern __attribute__((visibility("default"))) PRStatus PR_WaitForPollableEvent(PRFileDesc *event);


/* # 53 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/pripcsem.h" 1 */
/* # 62 "/usr/include/nspr4/pripcsem.h" */






typedef struct PRSem PRSem;
/* # 92 "/usr/include/nspr4/pripcsem.h" */
extern __attribute__((visibility("default"))) PRSem * PR_OpenSemaphore(
    const char *name, PRIntn flags, PRIntn mode, PRUintn value);
/* # 105 "/usr/include/nspr4/pripcsem.h" */
extern __attribute__((visibility("default"))) PRStatus PR_WaitSemaphore(PRSem *sem);







extern __attribute__((visibility("default"))) PRStatus PR_PostSemaphore(PRSem *sem);







extern __attribute__((visibility("default"))) PRStatus PR_CloseSemaphore(PRSem *sem);







extern __attribute__((visibility("default"))) PRStatus PR_DeleteSemaphore(const char *name);


/* # 54 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prlink.h" 1 */
/* # 46 "/usr/include/nspr4/prlink.h" */


typedef struct PRLibrary PRLibrary;

typedef struct PRStaticLinkTable {
    const char *name;
    void (*fp)(void);
} PRStaticLinkTable;
/* # 62 "/usr/include/nspr4/prlink.h" */
extern __attribute__((visibility("default"))) PRStatus PR_SetLibraryPath(const char *path);
/* # 72 "/usr/include/nspr4/prlink.h" */
extern __attribute__((visibility("default"))) char* PR_GetLibraryPath(void);
/* # 85 "/usr/include/nspr4/prlink.h" */
extern __attribute__((visibility("default"))) char* PR_GetLibraryName(const char *dir, const char *lib);





extern __attribute__((visibility("default"))) void PR_FreeLibraryName(char *mem);
/* # 105 "/usr/include/nspr4/prlink.h" */
extern __attribute__((visibility("default"))) PRLibrary* PR_LoadLibrary(const char *name);
/* # 126 "/usr/include/nspr4/prlink.h" */
typedef enum PRLibSpecType {
    PR_LibSpec_Pathname,
    PR_LibSpec_MacNamedFragment,
    PR_LibSpec_MacIndexedFragment,
    PR_LibSpec_PathnameU
} PRLibSpecType;

struct FSSpec;

typedef struct PRLibSpec {
    PRLibSpecType type;
    union {

        const char *pathname;


        struct {
            const struct FSSpec *fsspec;
            const char *name;
        } mac_named_fragment;


        struct {
            const struct FSSpec *fsspec;
            PRUint32 index;
        } mac_indexed_fragment;


        const PRUnichar *pathname_u;
    } value;
} PRLibSpec;
/* # 176 "/usr/include/nspr4/prlink.h" */
extern __attribute__((visibility("default"))) PRLibrary *
PR_LoadLibraryWithFlags(
    PRLibSpec libSpec,
    PRIntn flags
);
/* # 191 "/usr/include/nspr4/prlink.h" */
extern __attribute__((visibility("default"))) PRStatus PR_UnloadLibrary(PRLibrary *lib);
/* # 201 "/usr/include/nspr4/prlink.h" */
extern __attribute__((visibility("default"))) void* PR_FindSymbol(PRLibrary *lib, const char *name);
/* # 211 "/usr/include/nspr4/prlink.h" */
typedef void (*PRFuncPtr)(void);
extern __attribute__((visibility("default"))) PRFuncPtr PR_FindFunctionSymbol(PRLibrary *lib, const char *name);
/* # 224 "/usr/include/nspr4/prlink.h" */
extern __attribute__((visibility("default"))) void* PR_FindSymbolAndLibrary(const char *name,
            PRLibrary* *lib);
/* # 235 "/usr/include/nspr4/prlink.h" */
extern __attribute__((visibility("default"))) PRFuncPtr PR_FindFunctionSymbolAndLibrary(const char *name,
            PRLibrary* *lib);
/* # 249 "/usr/include/nspr4/prlink.h" */
extern __attribute__((visibility("default"))) PRLibrary* PR_LoadStaticLibrary(
    const char *name, const PRStaticLinkTable *table);







extern __attribute__((visibility("default"))) char * PR_GetLibraryFilePathname(const char *name, PRFuncPtr addr);


/* # 55 "/usr/include/nspr4/nspr.h" 2 */

/* # 1 "/usr/include/nspr4/prlog.h" 1 */
/* # 43 "/usr/include/nspr4/prlog.h" */

/* # 138 "/usr/include/nspr4/prlog.h" */
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

extern __attribute__((visibility("default"))) void PR_Assert(const char *s, const char *file, PRIntn ln);
/* # 251 "/usr/include/nspr4/prlog.h" */

/* # 57 "/usr/include/nspr4/nspr.h" 2 */

/* # 1 "/usr/include/nspr4/prmem.h" 1 */
/* # 47 "/usr/include/nspr4/prmem.h" */
/* # 1 "/usr/include/stdlib.h" 1 3 4 */
/* # 33 "/usr/include/stdlib.h" 3 4 */
/* # 1 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stddef.h" 1 3 4 */
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

/* # 182 "/usr/include/stdlib.h" 3 4 */


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

/* # 277 "/usr/include/stdlib.h" 3 4 */

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









extern void *malloc (size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;

extern void *calloc (size_t __nmemb, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;










extern void *realloc (void *__ptr, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__warn_unused_result__));

extern void free (void *__ptr) __attribute__ ((__nothrow__));




extern void cfree (void *__ptr) __attribute__ ((__nothrow__));



/* # 1 "/usr/include/alloca.h" 1 3 4 */
/* # 25 "/usr/include/alloca.h" 3 4 */
/* # 1 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stddef.h" 1 3 4 */
/* # 26 "/usr/include/alloca.h" 2 3 4 */







extern void *alloca (size_t __size) __attribute__ ((__nothrow__));






/* # 498 "/usr/include/stdlib.h" 2 3 4 */





extern void *valloc (size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;
/* # 512 "/usr/include/stdlib.h" 3 4 */


extern void abort (void) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));



extern int atexit (void (*__func) (void)) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
/* # 531 "/usr/include/stdlib.h" 3 4 */





extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern void exit (int __status) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));
/* # 554 "/usr/include/stdlib.h" 3 4 */

/* # 565 "/usr/include/stdlib.h" 3 4 */


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
/* # 642 "/usr/include/stdlib.h" 3 4 */
extern int mkstemps (char *__template, int __suffixlen) __attribute__ ((__nonnull__ (1))) ;
/* # 663 "/usr/include/stdlib.h" 3 4 */
extern char *mkdtemp (char *__template) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
/* # 712 "/usr/include/stdlib.h" 3 4 */





extern int system (__const char *__command) ;

/* # 734 "/usr/include/stdlib.h" 3 4 */
extern char *realpath (__const char *__restrict __name,
         char *__restrict __resolved) __attribute__ ((__nothrow__)) ;






typedef int (*__compar_fn_t) (__const void *, __const void *);
/* # 752 "/usr/include/stdlib.h" 3 4 */



extern void *bsearch (__const void *__key, __const void *__base,
        size_t __nmemb, size_t __size, __compar_fn_t __compar)
     __attribute__ ((__nonnull__ (1, 2, 5))) ;



extern void qsort (void *__base, size_t __nmemb, size_t __size,
     __compar_fn_t __compar) __attribute__ ((__nonnull__ (1, 4)));
/* # 771 "/usr/include/stdlib.h" 3 4 */
extern int abs (int __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;
extern long int labs (long int __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;












extern div_t div (int __numer, int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;
extern ldiv_t ldiv (long int __numer, long int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;

/* # 857 "/usr/include/stdlib.h" 3 4 */



extern int mblen (__const char *__s, size_t __n) __attribute__ ((__nothrow__)) ;


extern int mbtowc (wchar_t *__restrict __pwc,
     __const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__)) ;


extern int wctomb (char *__s, wchar_t __wchar) __attribute__ ((__nothrow__)) ;



extern size_t mbstowcs (wchar_t *__restrict __pwcs,
   __const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__));

extern size_t wcstombs (char *__restrict __s,
   __const wchar_t *__restrict __pwcs, size_t __n)
     __attribute__ ((__nothrow__));

/* # 905 "/usr/include/stdlib.h" 3 4 */
extern void setkey (__const char *__key) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
/* # 921 "/usr/include/stdlib.h" 3 4 */
extern int grantpt (int __fd) __attribute__ ((__nothrow__));



extern int unlockpt (int __fd) __attribute__ ((__nothrow__));




extern char *ptsname (int __fd) __attribute__ ((__nothrow__)) ;
/* # 948 "/usr/include/stdlib.h" 3 4 */
extern int getloadavg (double __loadavg[], int __nelem)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
/* # 964 "/usr/include/stdlib.h" 3 4 */

/* # 48 "/usr/include/nspr4/prmem.h" 2 */


/* # 66 "/usr/include/nspr4/prmem.h" */
extern __attribute__((visibility("default"))) void * PR_Malloc(PRUint32 size);

extern __attribute__((visibility("default"))) void * PR_Calloc(PRUint32 nelem, PRUint32 elsize);

extern __attribute__((visibility("default"))) void * PR_Realloc(void *ptr, PRUint32 size);

extern __attribute__((visibility("default"))) void PR_Free(void *ptr);
/* # 156 "/usr/include/nspr4/prmem.h" */

/* # 59 "/usr/include/nspr4/nspr.h" 2 */

/* # 1 "/usr/include/nspr4/prmwait.h" 1 */
/* # 46 "/usr/include/nspr4/prmwait.h" */

/* # 69 "/usr/include/nspr4/prmwait.h" */
typedef struct PRWaitGroup PRWaitGroup;
/* # 93 "/usr/include/nspr4/prmwait.h" */
typedef enum PRMWStatus
{
    PR_MW_PENDING = 1,
    PR_MW_SUCCESS = 0,
    PR_MW_FAILURE = -1,
    PR_MW_TIMEOUT = -2,
    PR_MW_INTERRUPT = -3
} PRMWStatus;
/* # 109 "/usr/include/nspr4/prmwait.h" */
typedef struct PRMemoryDescriptor
{
    void *start;
    PRSize length;
} PRMemoryDescriptor;
/* # 122 "/usr/include/nspr4/prmwait.h" */
typedef struct PRMWaitClientData PRMWaitClientData;
/* # 140 "/usr/include/nspr4/prmwait.h" */
typedef struct PRRecvWait
{
    PRCList internal;

    PRFileDesc *fd;
    PRMWStatus outcome;
    PRIntervalTime timeout;

    PRInt32 bytesRecv;
    PRMemoryDescriptor buffer;
    PRMWaitClientData *client;
} PRRecvWait;
/* # 162 "/usr/include/nspr4/prmwait.h" */
typedef struct PRMWaitEnumerator PRMWaitEnumerator;
/* # 196 "/usr/include/nspr4/prmwait.h" */
extern __attribute__((visibility("default"))) PRStatus PR_AddWaitFileDesc(PRWaitGroup *group, PRRecvWait *desc);
/* # 230 "/usr/include/nspr4/prmwait.h" */
extern __attribute__((visibility("default"))) PRRecvWait* PR_WaitRecvReady(PRWaitGroup *group);
/* # 265 "/usr/include/nspr4/prmwait.h" */
extern __attribute__((visibility("default"))) PRStatus PR_CancelWaitFileDesc(PRWaitGroup *group, PRRecvWait *desc);
/* # 292 "/usr/include/nspr4/prmwait.h" */
extern __attribute__((visibility("default"))) PRRecvWait* PR_CancelWaitGroup(PRWaitGroup *group);
/* # 319 "/usr/include/nspr4/prmwait.h" */
extern __attribute__((visibility("default"))) PRWaitGroup* PR_CreateWaitGroup(PRInt32 size);
/* # 342 "/usr/include/nspr4/prmwait.h" */
extern __attribute__((visibility("default"))) PRStatus PR_DestroyWaitGroup(PRWaitGroup *group);
/* # 364 "/usr/include/nspr4/prmwait.h" */
extern __attribute__((visibility("default"))) PRMWaitEnumerator* PR_CreateMWaitEnumerator(PRWaitGroup *group);
/* # 382 "/usr/include/nspr4/prmwait.h" */
extern __attribute__((visibility("default"))) PRStatus PR_DestroyMWaitEnumerator(PRMWaitEnumerator* enumerator);
/* # 405 "/usr/include/nspr4/prmwait.h" */
extern __attribute__((visibility("default"))) PRRecvWait* PR_EnumerateWaitGroup(
    PRMWaitEnumerator *enumerator, const PRRecvWait *previous);


/* # 61 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prnetdb.h" 1 */
/* # 44 "/usr/include/nspr4/prnetdb.h" */








extern __attribute__((visibility("default"))) PRStatus PR_StringToNetAddr(
    const char *string, PRNetAddr *addr);

extern __attribute__((visibility("default"))) PRStatus PR_NetAddrToString(
    const PRNetAddr *addr, char *string, PRUint32 size);
/* # 68 "/usr/include/nspr4/prnetdb.h" */
typedef struct PRHostEnt {
    char *h_name;
    char **h_aliases;




    PRInt32 h_addrtype;
    PRInt32 h_length;

    char **h_addr_list;
} PRHostEnt;
/* # 109 "/usr/include/nspr4/prnetdb.h" */
extern __attribute__((visibility("default"))) PRStatus PR_GetHostByName(
    const char *hostname, char *buf, PRIntn bufsize, PRHostEnt *hostentry);
/* # 146 "/usr/include/nspr4/prnetdb.h" */
extern __attribute__((visibility("default"))) PRStatus PR_GetIPNodeByName(
    const char *hostname,
    PRUint16 af,
    PRIntn flags,
    char *buf,
    PRIntn bufsize,
    PRHostEnt *hostentry);
/* # 175 "/usr/include/nspr4/prnetdb.h" */
extern __attribute__((visibility("default"))) PRStatus PR_GetHostByAddr(
    const PRNetAddr *hostaddr, char *buf, PRIntn bufsize, PRHostEnt *hostentry);
/* # 209 "/usr/include/nspr4/prnetdb.h" */
extern __attribute__((visibility("default"))) PRIntn PR_EnumerateHostEnt(
    PRIntn enumIndex, const PRHostEnt *hostEnt, PRUint16 port, PRNetAddr *address);
/* # 234 "/usr/include/nspr4/prnetdb.h" */
typedef enum PRNetAddrValue
{
    PR_IpAddrNull,
    PR_IpAddrAny,
    PR_IpAddrLoopback,
    PR_IpAddrV4Mapped
} PRNetAddrValue;

extern __attribute__((visibility("default"))) PRStatus PR_InitializeNetAddr(
    PRNetAddrValue val, PRUint16 port, PRNetAddr *addr);
/* # 270 "/usr/include/nspr4/prnetdb.h" */
extern __attribute__((visibility("default"))) PRStatus PR_SetNetAddr(
    PRNetAddrValue val, PRUint16 af, PRUint16 port, PRNetAddr *addr);
/* # 286 "/usr/include/nspr4/prnetdb.h" */
extern __attribute__((visibility("default"))) PRBool PR_IsNetAddrType(const PRNetAddr *addr, PRNetAddrValue val);
/* # 303 "/usr/include/nspr4/prnetdb.h" */
extern __attribute__((visibility("default"))) void PR_ConvertIPv4AddrToIPv6(PRUint32 v4addr, PRIPv6Addr *v6addr);
/* # 354 "/usr/include/nspr4/prnetdb.h" */
typedef struct PRProtoEnt {
    char *p_name;
    char **p_aliases;



    PRInt32 p_num;

} PRProtoEnt;

extern __attribute__((visibility("default"))) PRStatus PR_GetProtoByName(
    const char* protocolname, char* buffer, PRInt32 bufsize, PRProtoEnt* result);
/* # 389 "/usr/include/nspr4/prnetdb.h" */
extern __attribute__((visibility("default"))) PRStatus PR_GetProtoByNumber(
    PRInt32 protocolnumber, char* buffer, PRInt32 bufsize, PRProtoEnt* result);
/* # 413 "/usr/include/nspr4/prnetdb.h" */
typedef struct PRAddrInfo PRAddrInfo;

extern __attribute__((visibility("default"))) PRAddrInfo* PR_GetAddrInfoByName(
    const char *hostname, PRUint16 af, PRIntn flags);
/* # 430 "/usr/include/nspr4/prnetdb.h" */
extern __attribute__((visibility("default"))) void PR_FreeAddrInfo(PRAddrInfo *addrInfo);
/* # 455 "/usr/include/nspr4/prnetdb.h" */
extern __attribute__((visibility("default"))) void * PR_EnumerateAddrInfo(
    void *enumPtr, const PRAddrInfo *addrInfo, PRUint16 port, PRNetAddr *result);
/* # 474 "/usr/include/nspr4/prnetdb.h" */
extern __attribute__((visibility("default"))) const char * PR_GetCanonNameFromAddrInfo(
    const PRAddrInfo *addrInfo);
/* # 490 "/usr/include/nspr4/prnetdb.h" */
extern __attribute__((visibility("default"))) PRUint16 PR_ntohs(PRUint16);
extern __attribute__((visibility("default"))) PRUint32 PR_ntohl(PRUint32);
extern __attribute__((visibility("default"))) PRUint64 PR_ntohll(PRUint64);
extern __attribute__((visibility("default"))) PRUint16 PR_htons(PRUint16);
extern __attribute__((visibility("default"))) PRUint32 PR_htonl(PRUint32);
extern __attribute__((visibility("default"))) PRUint64 PR_htonll(PRUint64);


/* # 62 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prprf.h" 1 */
/* # 60 "/usr/include/nspr4/prprf.h" */
/* # 1 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stdarg.h" 1 3 4 */
/* # 61 "/usr/include/nspr4/prprf.h" 2 */








extern __attribute__((visibility("default"))) PRUint32 PR_snprintf(char *out, PRUint32 outlen, const char *fmt, ...);






extern __attribute__((visibility("default"))) char* PR_smprintf(const char *fmt, ...);




extern __attribute__((visibility("default"))) void PR_smprintf_free(char *mem);
/* # 90 "/usr/include/nspr4/prprf.h" */
extern __attribute__((visibility("default"))) char* PR_sprintf_append(char *last, const char *fmt, ...);
/* # 99 "/usr/include/nspr4/prprf.h" */
typedef PRIntn (*PRStuffFunc)(void *arg, const char *s, PRUint32 slen);

extern __attribute__((visibility("default"))) PRUint32 PR_sxprintf(PRStuffFunc f, void *arg, const char *fmt, ...);




extern __attribute__((visibility("default"))) PRUint32 PR_fprintf(struct PRFileDesc* fd, const char *fmt, ...);




extern __attribute__((visibility("default"))) PRUint32 PR_vsnprintf(char *out, PRUint32 outlen, const char *fmt, va_list ap);
extern __attribute__((visibility("default"))) char* PR_vsmprintf(const char *fmt, va_list ap);
extern __attribute__((visibility("default"))) char* PR_vsprintf_append(char *last, const char *fmt, va_list ap);
extern __attribute__((visibility("default"))) PRUint32 PR_vsxprintf(PRStuffFunc f, void *arg, const char *fmt, va_list ap);
extern __attribute__((visibility("default"))) PRUint32 PR_vfprintf(struct PRFileDesc* fd, const char *fmt, va_list ap);
/* # 150 "/usr/include/nspr4/prprf.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_sscanf(const char *buf, const char *fmt, ...);


/* # 63 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prproces.h" 1 */
/* # 44 "/usr/include/nspr4/prproces.h" */






typedef struct PRProcess PRProcess;
typedef struct PRProcessAttr PRProcessAttr;

extern __attribute__((visibility("default"))) PRProcessAttr * PR_NewProcessAttr(void);

extern __attribute__((visibility("default"))) void PR_ResetProcessAttr(PRProcessAttr *attr);

extern __attribute__((visibility("default"))) void PR_DestroyProcessAttr(PRProcessAttr *attr);

extern __attribute__((visibility("default"))) void PR_ProcessAttrSetStdioRedirect(
    PRProcessAttr *attr,
    PRSpecialFD stdioFd,
    PRFileDesc *redirectFd
);




extern __attribute__((visibility("default"))) void PR_SetStdioRedirect(
    PRProcessAttr *attr,
    PRSpecialFD stdioFd,
    PRFileDesc *redirectFd
);

extern __attribute__((visibility("default"))) PRStatus PR_ProcessAttrSetCurrentDirectory(
    PRProcessAttr *attr,
    const char *dir
);

extern __attribute__((visibility("default"))) PRStatus PR_ProcessAttrSetInheritableFD(
    PRProcessAttr *attr,
    PRFileDesc *fd,
    const char *name
);
/* # 98 "/usr/include/nspr4/prproces.h" */
extern __attribute__((visibility("default"))) PRProcess* PR_CreateProcess(
    const char *path,
    char *const *argv,
    char *const *envp,
    const PRProcessAttr *attr);

extern __attribute__((visibility("default"))) PRStatus PR_CreateProcessDetached(
    const char *path,
    char *const *argv,
    char *const *envp,
    const PRProcessAttr *attr);

extern __attribute__((visibility("default"))) PRStatus PR_DetachProcess(PRProcess *process);

extern __attribute__((visibility("default"))) PRStatus PR_WaitProcess(PRProcess *process, PRInt32 *exitCode);

extern __attribute__((visibility("default"))) PRStatus PR_KillProcess(PRProcess *process);


/* # 64 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prrng.h" 1 */
/* # 51 "/usr/include/nspr4/prrng.h" */

/* # 99 "/usr/include/nspr4/prrng.h" */
extern __attribute__((visibility("default"))) PRSize PR_GetRandomNoise(
    void *buf,
    PRSize size
);


/* # 65 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prrwlock.h" 1 */
/* # 49 "/usr/include/nspr4/prrwlock.h" */

/* # 59 "/usr/include/nspr4/prrwlock.h" */
typedef struct PRRWLock PRRWLock;
/* # 76 "/usr/include/nspr4/prrwlock.h" */
extern __attribute__((visibility("default"))) PRRWLock* PR_NewRWLock(PRUint32 lock_rank, const char *lock_name);
/* # 86 "/usr/include/nspr4/prrwlock.h" */
extern __attribute__((visibility("default"))) void PR_DestroyRWLock(PRRWLock *lock);
/* # 96 "/usr/include/nspr4/prrwlock.h" */
extern __attribute__((visibility("default"))) void PR_RWLock_Rlock(PRRWLock *lock);
/* # 106 "/usr/include/nspr4/prrwlock.h" */
extern __attribute__((visibility("default"))) void PR_RWLock_Wlock(PRRWLock *lock);
/* # 116 "/usr/include/nspr4/prrwlock.h" */
extern __attribute__((visibility("default"))) void PR_RWLock_Unlock(PRRWLock *lock);


/* # 66 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prshm.h" 1 */
/* # 137 "/usr/include/nspr4/prshm.h" */





typedef struct PRSharedMemory PRSharedMemory;
/* # 179 "/usr/include/nspr4/prshm.h" */
extern __attribute__((visibility("default"))) PRSharedMemory *
    PR_OpenSharedMemory(
        const char *name,
        PRSize size,
        PRIntn flags,
        PRIntn mode
);
/* # 213 "/usr/include/nspr4/prshm.h" */
extern __attribute__((visibility("default"))) void *
    PR_AttachSharedMemory(
        PRSharedMemory *shm,
        PRIntn flags
);
/* # 239 "/usr/include/nspr4/prshm.h" */
extern __attribute__((visibility("default"))) PRStatus
    PR_DetachSharedMemory(
        PRSharedMemory *shm,
        void *addr
);
/* # 261 "/usr/include/nspr4/prshm.h" */
extern __attribute__((visibility("default"))) PRStatus
    PR_CloseSharedMemory(
        PRSharedMemory *shm
);
/* # 282 "/usr/include/nspr4/prshm.h" */
extern __attribute__((visibility("default"))) PRStatus
    PR_DeleteSharedMemory(
        const char *name
);


/* # 67 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prshma.h" 1 */
/* # 138 "/usr/include/nspr4/prshma.h" */

/* # 166 "/usr/include/nspr4/prshma.h" */
extern __attribute__((visibility("default"))) PRFileMap *
PR_OpenAnonFileMap(
    const char *dirName,
    PRSize size,
    PRFileMapProtect prot
);
/* # 194 "/usr/include/nspr4/prshma.h" */
extern __attribute__((visibility("default"))) PRStatus
PR_ProcessAttrSetInheritableFileMap(
    PRProcessAttr *attr,
    PRFileMap *fm,
    const char *shmname
);
/* # 219 "/usr/include/nspr4/prshma.h" */
extern __attribute__((visibility("default"))) PRFileMap *
PR_GetInheritedFileMap(
    const char *shmname
);
/* # 243 "/usr/include/nspr4/prshma.h" */
extern __attribute__((visibility("default"))) PRStatus
PR_ExportFileMapAsString(
    PRFileMap *fm,
    PRSize bufsize,
    char *buf
);
/* # 265 "/usr/include/nspr4/prshma.h" */
extern __attribute__((visibility("default"))) PRFileMap *
PR_ImportFileMapFromString(
    const char *fmstring
);


/* # 68 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prsystem.h" 1 */
/* # 46 "/usr/include/nspr4/prsystem.h" */








extern __attribute__((visibility("default"))) char PR_GetDirectorySeparator(void);






extern __attribute__((visibility("default"))) char PR_GetDirectorySepartor(void);







extern __attribute__((visibility("default"))) char PR_GetPathSeparator(void);


typedef enum {
    PR_SI_HOSTNAME,

    PR_SI_SYSNAME,
    PR_SI_RELEASE,
    PR_SI_ARCHITECTURE,
    PR_SI_HOSTNAME_UNTRUNCATED

} PRSysInfo;
/* # 94 "/usr/include/nspr4/prsystem.h" */
extern __attribute__((visibility("default"))) PRStatus PR_GetSystemInfo(PRSysInfo cmd, char *buf, PRUint32 buflen);




extern __attribute__((visibility("default"))) PRInt32 PR_GetPageSize(void);




extern __attribute__((visibility("default"))) PRInt32 PR_GetPageShift(void);
/* # 120 "/usr/include/nspr4/prsystem.h" */
extern __attribute__((visibility("default"))) PRInt32 PR_GetNumberOfProcessors( void );
/* # 136 "/usr/include/nspr4/prsystem.h" */
extern __attribute__((visibility("default"))) PRUint64 PR_GetPhysicalMemorySize(void);


/* # 69 "/usr/include/nspr4/nspr.h" 2 */


/* # 1 "/usr/include/nspr4/prtpool.h" 1 */
/* # 52 "/usr/include/nspr4/prtpool.h" */


typedef struct PRJobIoDesc {
    PRFileDesc *socket;
    PRErrorCode error;
    PRIntervalTime timeout;
} PRJobIoDesc;

typedef struct PRThreadPool PRThreadPool;
typedef struct PRJob PRJob;
typedef void ( *PRJobFn) (void *arg);


extern __attribute__((visibility("default"))) PRThreadPool *
PR_CreateThreadPool(PRInt32 initial_threads, PRInt32 max_threads,
                          PRUint32 stacksize);


extern __attribute__((visibility("default"))) PRJob *
PR_QueueJob(PRThreadPool *tpool, PRJobFn fn, void *arg, PRBool joinable);


extern __attribute__((visibility("default"))) PRJob *
PR_QueueJob_Read(PRThreadPool *tpool, PRJobIoDesc *iod,
       PRJobFn fn, void * arg, PRBool joinable);


extern __attribute__((visibility("default"))) PRJob *
PR_QueueJob_Write(PRThreadPool *tpool, PRJobIoDesc *iod,
        PRJobFn fn, void * arg, PRBool joinable);


extern __attribute__((visibility("default"))) PRJob *
PR_QueueJob_Accept(PRThreadPool *tpool, PRJobIoDesc *iod,
         PRJobFn fn, void * arg, PRBool joinable);


extern __attribute__((visibility("default"))) PRJob *
PR_QueueJob_Connect(PRThreadPool *tpool, PRJobIoDesc *iod,
   const PRNetAddr *addr, PRJobFn fn, void * arg, PRBool joinable);


extern __attribute__((visibility("default"))) PRJob *
PR_QueueJob_Timer(PRThreadPool *tpool, PRIntervalTime timeout,
        PRJobFn fn, void * arg, PRBool joinable);

extern __attribute__((visibility("default"))) PRStatus
PR_CancelJob(PRJob *job);


extern __attribute__((visibility("default"))) PRStatus
PR_JoinJob(PRJob *job);


extern __attribute__((visibility("default"))) PRStatus
PR_ShutdownThreadPool(PRThreadPool *tpool);


extern __attribute__((visibility("default"))) PRStatus
PR_JoinThreadPool(PRThreadPool *tpool);


/* # 72 "/usr/include/nspr4/nspr.h" 2 */
/* # 1 "/usr/include/nspr4/prtrace.h" 1 */
/* # 100 "/usr/include/nspr4/prtrace.h" */







typedef void * PRTraceHandle;






typedef struct PRTraceEntry
{
    PRThread *thread;
    PRTraceHandle handle;
    PRTime time;
    PRUint32 userData[8];
} PRTraceEntry;






typedef enum PRTraceOption
{
    PRTraceBufSize,
    PRTraceEnable,
    PRTraceDisable,
    PRTraceSuspend,
    PRTraceResume,
    PRTraceSuspendRecording,
    PRTraceResumeRecording,
    PRTraceLockHandles,
    PRTraceUnLockHandles,
    PRTraceStopRecording
} PRTraceOption;
/* # 208 "/usr/include/nspr4/prtrace.h" */
extern __attribute__((visibility("default"))) PRTraceHandle
 PR_CreateTrace(
     const char *qName,
     const char *rName,
     const char *description
);
/* # 242 "/usr/include/nspr4/prtrace.h" */
extern __attribute__((visibility("default"))) void
 PR_DestroyTrace(
  PRTraceHandle handle
);
/* # 283 "/usr/include/nspr4/prtrace.h" */
extern __attribute__((visibility("default"))) void
 PR_Trace(
     PRTraceHandle handle,
     PRUint32 userData0,
     PRUint32 userData1,
     PRUint32 userData2,
     PRUint32 userData3,
     PRUint32 userData4,
     PRUint32 userData5,
     PRUint32 userData6,
     PRUint32 userData7
);
/* # 370 "/usr/include/nspr4/prtrace.h" */
extern __attribute__((visibility("default"))) void
 PR_SetTraceOption(
     PRTraceOption command,
     void *value
);
/* # 408 "/usr/include/nspr4/prtrace.h" */
extern __attribute__((visibility("default"))) void
 PR_GetTraceOption(
     PRTraceOption command,
     void *value
);
/* # 444 "/usr/include/nspr4/prtrace.h" */
extern __attribute__((visibility("default"))) PRTraceHandle
 PR_GetTraceHandleFromName(
     const char *qName,
        const char *rName
);
/* # 476 "/usr/include/nspr4/prtrace.h" */
extern __attribute__((visibility("default"))) void
 PR_GetTraceNameFromHandle(
     PRTraceHandle handle,
     const char **qName,
     const char **rName,
     const char **description
);
/* # 518 "/usr/include/nspr4/prtrace.h" */
extern __attribute__((visibility("default"))) PRTraceHandle
 PR_FindNextTraceQname(
        PRTraceHandle handle
);
/* # 560 "/usr/include/nspr4/prtrace.h" */
extern __attribute__((visibility("default"))) PRTraceHandle
 PR_FindNextTraceRname(
        PRTraceHandle rhandle,
        PRTraceHandle qhandle
);
/* # 616 "/usr/include/nspr4/prtrace.h" */
extern __attribute__((visibility("default"))) void
 PR_RecordTraceEntries(
        void
);
/* # 668 "/usr/include/nspr4/prtrace.h" */
extern __attribute__((visibility("default"))) PRIntn
    PR_GetTraceEntries(
        PRTraceEntry *buffer,
        PRInt32 count,
        PRInt32 *found
);


/* # 73 "/usr/include/nspr4/nspr.h" 2 */
/* # 51 "nssbaset.h" 2 */
/* # 1 "../../../../dist/public/nss/nssilock.h" 1 */
/* # 136 "../../../../dist/public/nss/nssilock.h" */
/* # 1 "../../../../dist/public/nss/utilrename.h" 1 */
/* # 137 "../../../../dist/public/nss/nssilock.h" 2 */
/* # 1 "/usr/include/nspr4/prtypes.h" 1 */
/* # 138 "../../../../dist/public/nss/nssilock.h" 2 */
/* # 1 "/usr/include/nspr4/prmon.h" 1 */
/* # 139 "../../../../dist/public/nss/nssilock.h" 2 */
/* # 1 "/usr/include/nspr4/prlock.h" 1 */
/* # 140 "../../../../dist/public/nss/nssilock.h" 2 */
/* # 1 "/usr/include/nspr4/prcvar.h" 1 */
/* # 141 "../../../../dist/public/nss/nssilock.h" 2 */

/* # 1 "../../../../dist/public/nss/nssilckt.h" 1 */
/* # 142 "../../../../dist/public/nss/nssilckt.h" */
typedef enum {
    nssILockArena = 0,
    nssILockSession = 1,
    nssILockObject = 2,
    nssILockRefLock = 3,
    nssILockCert = 4,
    nssILockCertDB = 5,
    nssILockDBM = 6,
    nssILockCache = 7,
    nssILockSSL = 8,
    nssILockList = 9,
    nssILockSlot = 10,
    nssILockFreelist = 11,
    nssILockOID = 12,
    nssILockAttribute = 13,
    nssILockPK11cxt = 14,
    nssILockRWLock = 15,
    nssILockOther = 16,
    nssILockSelfServ = 17,
    nssILockKeyDB = 18,
    nssILockLast
} nssILockType;
/* # 143 "../../../../dist/public/nss/nssilock.h" 2 */


/* # 319 "../../../../dist/public/nss/nssilock.h" */

/* # 52 "nssbaset.h" 2 */
/* # 68 "nssbaset.h" */

/* # 79 "nssbaset.h" */
typedef PRInt32 NSSError;
/* # 92 "nssbaset.h" */
struct NSSArenaStr;
typedef struct NSSArenaStr NSSArena;
/* # 102 "nssbaset.h" */
struct NSSItemStr {
  void *data;
  PRUint32 size;
};
typedef struct NSSItemStr NSSItem;
/* # 115 "nssbaset.h" */
typedef NSSItem NSSBER;
/* # 124 "nssbaset.h" */
typedef NSSBER NSSDER;
/* # 135 "nssbaset.h" */
typedef NSSItem NSSBitString;







typedef char NSSUTF8;







typedef char NSSASCII7;


/* # 53 "baset.h" 2 */


/* # 1 "/usr/include/nspr4/plhash.h" 1 */
/* # 46 "/usr/include/nspr4/plhash.h" */


typedef struct PLHashEntry PLHashEntry;
typedef struct PLHashTable PLHashTable;
typedef PRUint32 PLHashNumber;

typedef PLHashNumber ( *PLHashFunction)(const void *key);
typedef PRIntn ( *PLHashComparator)(const void *v1, const void *v2);

typedef PRIntn ( *PLHashEnumerator)(PLHashEntry *he, PRIntn i, void *arg);







typedef struct PLHashAllocOps {
    void * ( *allocTable)(void *pool, PRSize size);
    void ( *freeTable)(void *pool, void *item);
    PLHashEntry * ( *allocEntry)(void *pool, const void *key);
    void ( *freeEntry)(void *pool, PLHashEntry *he, PRUintn flag);
} PLHashAllocOps;




struct PLHashEntry {
    PLHashEntry *next;
    PLHashNumber keyHash;
    const void *key;
    void *value;
};

struct PLHashTable {
    PLHashEntry **buckets;
    PRUint32 nentries;
    PRUint32 shift;
    PLHashFunction keyHash;
    PLHashComparator keyCompare;
    PLHashComparator valueCompare;
    const PLHashAllocOps *allocOps;
    void *allocPriv;






};





extern __attribute__((visibility("default"))) PLHashTable *
PL_NewHashTable(PRUint32 numBuckets, PLHashFunction keyHash,
                PLHashComparator keyCompare, PLHashComparator valueCompare,
                const PLHashAllocOps *allocOps, void *allocPriv);

extern __attribute__((visibility("default"))) void
PL_HashTableDestroy(PLHashTable *ht);


extern __attribute__((visibility("default"))) PLHashEntry *
PL_HashTableAdd(PLHashTable *ht, const void *key, void *value);

extern __attribute__((visibility("default"))) PRBool
PL_HashTableRemove(PLHashTable *ht, const void *key);

extern __attribute__((visibility("default"))) void *
PL_HashTableLookup(PLHashTable *ht, const void *key);

extern __attribute__((visibility("default"))) void *
PL_HashTableLookupConst(PLHashTable *ht, const void *key);

extern __attribute__((visibility("default"))) PRIntn
PL_HashTableEnumerateEntries(PLHashTable *ht, PLHashEnumerator f, void *arg);


extern __attribute__((visibility("default"))) PLHashNumber
PL_HashString(const void *key);


extern __attribute__((visibility("default"))) PRIntn
PL_CompareStrings(const void *v1, const void *v2);


extern __attribute__((visibility("default"))) PRIntn
PL_CompareValues(const void *v1, const void *v2);


extern __attribute__((visibility("default"))) PLHashEntry **
PL_HashTableRawLookup(PLHashTable *ht, PLHashNumber keyHash, const void *key);

extern __attribute__((visibility("default"))) PLHashEntry **
PL_HashTableRawLookupConst(PLHashTable *ht, PLHashNumber keyHash,
                           const void *key);

extern __attribute__((visibility("default"))) PLHashEntry *
PL_HashTableRawAdd(PLHashTable *ht, PLHashEntry **hep, PLHashNumber keyHash,
                   const void *key, void *value);

extern __attribute__((visibility("default"))) void
PL_HashTableRawRemove(PLHashTable *ht, PLHashEntry **hep, PLHashEntry *he);


extern __attribute__((visibility("default"))) PRIntn
PL_HashTableDump(PLHashTable *ht, PLHashEnumerator dump, FILE *fp);


/* # 56 "baset.h" 2 */









struct nssArenaMarkStr;
typedef struct nssArenaMarkStr nssArenaMark;
/* # 105 "baset.h" */
typedef struct nssListStr nssList;
typedef struct nssListIteratorStr nssListIterator;
typedef PRBool (* nssListCompareFunc)(void *a, void *b);
typedef PRIntn (* nssListSortFunc)(void *a, void *b);
typedef void (* nssListElementDestructorFunc)(void *el);

typedef struct nssHashStr nssHash;
typedef void ( *nssHashIterator)(const void *key,
                                            void *value,
                                            void *arg);
/* # 145 "baset.h" */
enum nssStringTypeEnum {
  nssStringType_DirectoryString,
  nssStringType_TeletexString,
  nssStringType_PrintableString,
  nssStringType_UniversalString,
  nssStringType_BMPString,
  nssStringType_UTF8String,
  nssStringType_PHGString,
  nssStringType_GeneralString,

  nssStringType_Unknown = -1
};
typedef enum nssStringTypeEnum nssStringType;


/* # 53 "base.h" 2 */



/* # 1 "nssbase.h" 1 */
/* # 55 "nssbase.h" */

/* # 80 "nssbase.h" */
extern NSSArena *
NSSArena_Create
(
  void
);

extern const NSSError NSS_ERROR_NO_MEMORY;
/* # 104 "nssbase.h" */
extern PRStatus
NSSArena_Destroy
(
  NSSArena *arena
);

extern const NSSError NSS_ERROR_INVALID_ARENA;
/* # 136 "nssbase.h" */
extern NSSError
NSS_GetError
(
  void
);

extern const NSSError NSS_ERROR_NO_ERROR;
/* # 162 "nssbase.h" */
extern NSSError *
NSS_GetErrorStack
(
  void
);


/* # 57 "base.h" 2 */





/* # 122 "base.h" */
extern NSSArena *
nssArena_Create
(
  void
);

extern const NSSError NSS_ERROR_NO_MEMORY;
/* # 146 "base.h" */
extern PRStatus
nssArena_Destroy
(
  NSSArena *arena
);

extern const NSSError NSS_ERROR_INVALID_ARENA;
/* # 175 "base.h" */
extern nssArenaMark *
nssArena_Mark
(
  NSSArena *arena
);

extern const NSSError NSS_ERROR_INVALID_ARENA;
extern const NSSError NSS_ERROR_NO_MEMORY;
extern const NSSError NSS_ERROR_ARENA_MARKED_BY_ANOTHER_THREAD;
/* # 204 "base.h" */
extern PRStatus
nssArena_Release
(
  NSSArena *arena,
  nssArenaMark *arenaMark
);

extern const NSSError NSS_ERROR_INVALID_ARENA;
extern const NSSError NSS_ERROR_INVALID_ARENA_MARK;
/* # 236 "base.h" */
extern PRStatus
nssArena_Unmark
(
  NSSArena *arena,
  nssArenaMark *arenaMark
);

extern const NSSError NSS_ERROR_INVALID_ARENA;
extern const NSSError NSS_ERROR_INVALID_ARENA_MARK;
extern const NSSError NSS_ERROR_ARENA_MARKED_BY_ANOTHER_THREAD;
/* # 342 "base.h" */
extern void *
nss_ZAlloc
(
  NSSArena *arenaOpt,
  PRUint32 size
);

extern const NSSError NSS_ERROR_INVALID_ARENA;
extern const NSSError NSS_ERROR_NO_MEMORY;
extern const NSSError NSS_ERROR_ARENA_MARKED_BY_ANOTHER_THREAD;
/* # 371 "base.h" */
extern PRStatus
nss_ZFreeIf
(
  void *pointer
);

extern const NSSError NSS_ERROR_INVALID_POINTER;
/* # 399 "base.h" */
extern void *
nss_ZRealloc
(
  void *pointer,
  PRUint32 newSize
);

extern const NSSError NSS_ERROR_INVALID_POINTER;
extern const NSSError NSS_ERROR_NO_MEMORY;
extern const NSSError NSS_ERROR_ARENA_MARKED_BY_ANOTHER_THREAD;
/* # 526 "base.h" */
extern PRStatus
nssArena_Shutdown(void);
/* # 540 "base.h" */
extern PLHashAllocOps nssArenaHashAllocOps;
/* # 559 "base.h" */
extern void
nss_SetError
(
  PRUint32 error
);







extern void
nss_ClearErrorStack
(
  void
);







extern void
nss_DestroyErrorStack
(
  void
);
/* # 597 "base.h" */
extern NSSItem *
nssItem_Create
(
  NSSArena *arenaOpt,
  NSSItem *rvOpt,
  PRUint32 length,
  const void *data
);

extern void
nssItem_Destroy
(
  NSSItem *item
);

extern NSSItem *
nssItem_Duplicate
(
  NSSItem *obj,
  NSSArena *arenaOpt,
  NSSItem *rvOpt
);

extern PRBool
nssItem_Equal
(
  const NSSItem *one,
  const NSSItem *two,
  PRStatus *statusOpt
);
/* # 653 "base.h" */
extern PRBool
nssUTF8_CaseIgnoreMatch
(
  const NSSUTF8 *a,
  const NSSUTF8 *b,
  PRStatus *statusOpt
);
/* # 678 "base.h" */
extern NSSUTF8 *
nssUTF8_Duplicate
(
  const NSSUTF8 *s,
  NSSArena *arenaOpt
);
/* # 707 "base.h" */
extern PRBool
nssUTF8_PrintableMatch
(
  const NSSUTF8 *a,
  const NSSUTF8 *b,
  PRStatus *statusOpt
);
/* # 731 "base.h" */
extern PRUint32
nssUTF8_Size
(
  const NSSUTF8 *s,
  PRStatus *statusOpt
);

extern const NSSError NSS_ERROR_INVALID_POINTER;
extern const NSSError NSS_ERROR_VALUE_TOO_LARGE;
/* # 758 "base.h" */
extern PRUint32
nssUTF8_Length
(
  const NSSUTF8 *s,
  PRStatus *statusOpt
);

extern const NSSError NSS_ERROR_INVALID_POINTER;
extern const NSSError NSS_ERROR_VALUE_TOO_LARGE;
extern const NSSError NSS_ERROR_INVALID_STRING;
/* # 792 "base.h" */
extern NSSUTF8 *
nssUTF8_Create
(
  NSSArena *arenaOpt,
  nssStringType type,
  const void *inputString,
  PRUint32 size
);

extern const NSSError NSS_ERROR_INVALID_POINTER;
extern const NSSError NSS_ERROR_NO_MEMORY;
extern const NSSError NSS_ERROR_UNSUPPORTED_TYPE;

extern NSSItem *
nssUTF8_GetEncoding
(
  NSSArena *arenaOpt,
  NSSItem *rvOpt,
  nssStringType type,
  NSSUTF8 *string
);
/* # 825 "base.h" */
extern const NSSError NSS_ERROR_INVALID_POINTER;
extern const NSSError NSS_ERROR_INVALID_ARGUMENT;

extern PRStatus
nssUTF8_CopyIntoFixedBuffer
(
  NSSUTF8 *string,
  char *buffer,
  PRUint32 bufferSize,
  char pad
);






extern PRBool
nssUTF8_Equal
(
  const NSSUTF8 *a,
  const NSSUTF8 *b,
  PRStatus *statusOpt
);
/* # 865 "base.h" */
extern nssList *
nssList_Create
(
  NSSArena *arenaOpt,
  PRBool threadSafe
);




extern PRStatus
nssList_Destroy
(
  nssList *list
);

extern void
nssList_Clear
(
  nssList *list,
  nssListElementDestructorFunc destructor
);
/* # 895 "base.h" */
extern void
nssList_SetCompareFunction
(
  nssList *list,
  nssListCompareFunc compareFunc
);






extern void
nssList_SetSortFunction
(
  nssList *list,
  nssListSortFunc sortFunc
);




extern PRStatus
nssList_Add
(
  nssList *list,
  void *data
);







extern PRStatus
nssList_AddUnique
(
  nssList *list,
  void *data
);






extern PRStatus
nssList_Remove(nssList *list, void *data);







extern void *
nssList_Get
(
  nssList *list,
  void *data
);




extern PRUint32
nssList_Count
(
  nssList *list
);







extern PRStatus
nssList_GetArray
(
  nssList *list,
  void **rvArray,
  PRUint32 maxElements
);






extern nssListIterator *
nssList_CreateIterator
(
  nssList *list
);

extern nssList *
nssList_Clone
(
  nssList *list
);




extern void
nssListIterator_Destroy
(
  nssListIterator *iter
);







extern void *
nssListIterator_Start
(
  nssListIterator *iter
);






extern void *
nssListIterator_Next
(
  nssListIterator *iter
);







extern PRStatus
nssListIterator_Finish
(
  nssListIterator *iter
);
/* # 1060 "base.h" */
extern nssHash *
nssHash_Create
(
  NSSArena *arenaOpt,
  PRUint32 numBuckets,
  PLHashFunction keyHash,
  PLHashComparator keyCompare,
  PLHashComparator valueCompare
);

extern nssHash *
nssHash_CreatePointer
(
  NSSArena *arenaOpt,
  PRUint32 numBuckets
);

extern nssHash *
nssHash_CreateString
(
  NSSArena *arenaOpt,
  PRUint32 numBuckets
);

extern nssHash *
nssHash_CreateItem
(
  NSSArena *arenaOpt,
  PRUint32 numBuckets
);





extern void
nssHash_Destroy
(
  nssHash *hash
);






extern const NSSError NSS_ERROR_HASH_COLLISION;

extern PRStatus
nssHash_Add
(
  nssHash *hash,
  const void *key,
  const void *value
);





extern void
nssHash_Remove
(
  nssHash *hash,
  const void *it
);





extern PRUint32
nssHash_Count
(
  nssHash *hash
);





extern PRBool
nssHash_Exists
(
  nssHash *hash,
  const void *it
);





extern void *
nssHash_Lookup
(
  nssHash *hash,
  const void *it
);





extern void
nssHash_Iterate
(
  nssHash *hash,
  nssHashIterator fcn,
  void *closure
);
/* # 1372 "base.h" */
extern void *
nsslibc_memcpy
(
  void *dest,
  const void *source,
  PRUint32 n
);

extern const NSSError NSS_ERROR_INVALID_POINTER;
/* # 1393 "base.h" */
extern void *
nsslibc_memset
(
  void *dest,
  PRUint8 byte,
  PRUint32 n
);

extern const NSSError NSS_ERROR_INVALID_POINTER;
/* # 1415 "base.h" */
extern PRBool
nsslibc_memequal
(
  const void *a,
  const void *b,
  PRUint32 len,
  PRStatus *statusOpt
);

extern const NSSError NSS_ERROR_INVALID_POINTER;




/* # 49 "arena.c" 2 */







/* # 1 "/usr/include/nspr4/plarena.h" 1 */
/* # 49 "/usr/include/nspr4/plarena.h" */
/* # 1 "/usr/include/nspr4/plarenas.h" 1 */
/* # 41 "/usr/include/nspr4/plarenas.h" */


typedef struct PLArenaPool PLArenaPool;
/* # 54 "/usr/include/nspr4/plarenas.h" */
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




extern __attribute__((visibility("default"))) void PL_ClearArenaPool(PLArenaPool *pool, PRInt32 pattern);


/* # 50 "/usr/include/nspr4/plarena.h" 2 */



typedef struct PLArena PLArena;

struct PLArena {
    PLArena *next;
    PRUword base;
    PRUword limit;
    PRUword avail;
};
/* # 83 "/usr/include/nspr4/plarena.h" */
struct PLArenaPool {
    PLArena first;
    PLArena *current;
    PRUint32 arenasize;
    PRUword mask;



};
/* # 212 "/usr/include/nspr4/plarena.h" */

/* # 57 "arena.c" 2 */

/* # 1 "/usr/include/string.h" 1 3 4 */
/* # 29 "/usr/include/string.h" 3 4 */





/* # 1 "/usr/lib/gcc/x86_64-redhat-linux/4.6.2/include/stddef.h" 1 3 4 */
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


/* # 126 "/usr/include/string.h" 3 4 */


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

/* # 175 "/usr/include/string.h" 3 4 */
extern char *strdup (__const char *__s)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));
/* # 210 "/usr/include/string.h" 3 4 */

/* # 235 "/usr/include/string.h" 3 4 */
extern char *strchr (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
/* # 262 "/usr/include/string.h" 3 4 */
extern char *strrchr (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));


/* # 281 "/usr/include/string.h" 3 4 */



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
/* # 397 "/usr/include/string.h" 3 4 */


extern size_t strlen (__const char *__s)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));

/* # 411 "/usr/include/string.h" 3 4 */


extern char *strerror (int __errnum) __attribute__ ((__nothrow__));

/* # 438 "/usr/include/string.h" 3 4 */
extern char *strerror_r (int __errnum, char *__buf, size_t __buflen)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));
/* # 451 "/usr/include/string.h" 3 4 */
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
/* # 536 "/usr/include/string.h" 3 4 */
extern int strcasecmp (__const char *__s1, __const char *__s2)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strncasecmp (__const char *__s1, __const char *__s2, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
/* # 559 "/usr/include/string.h" 3 4 */
extern char *strsep (char **__restrict __stringp,
       __const char *__restrict __delim)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
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
/* # 1303 "/usr/include/bits/string2.h" 3 4 */
extern char *__strdup (__const char *__string) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__));
/* # 1322 "/usr/include/bits/string2.h" 3 4 */
extern char *__strndup (__const char *__string, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__));
