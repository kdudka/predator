# 0 "arena-orig.c"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "arena-orig.c"
# 48 "arena-orig.c"
# 1 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h" 1
# 16 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
# 1 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/baset.h" 1
# 16 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/baset.h"
# 1 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbaset.h" 1
# 14 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbaset.h"
# 1 "/usr/include/nspr/nspr.h" 1
# 9 "/usr/include/nspr/nspr.h"
# 1 "/usr/include/nspr/pratom.h" 1
# 14 "/usr/include/nspr/pratom.h"
# 1 "/usr/include/nspr/prtypes.h" 1
# 26 "/usr/include/nspr/prtypes.h"
# 1 "/usr/include/nspr/prcpucfg.h" 1
# 27 "/usr/include/nspr/prtypes.h" 2


# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 1 3 4
# 145 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 3 4

# 145 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 3 4
typedef long int ptrdiff_t;
# 214 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 3 4
typedef long unsigned int size_t;
# 329 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 3 4
typedef int wchar_t;
# 30 "/usr/include/nspr/prtypes.h" 2
# 169 "/usr/include/nspr/prtypes.h"

# 209 "/usr/include/nspr/prtypes.h"

# 209 "/usr/include/nspr/prtypes.h"
typedef unsigned char PRUint8;
# 229 "/usr/include/nspr/prtypes.h"
typedef signed char PRInt8;
# 254 "/usr/include/nspr/prtypes.h"
typedef unsigned short PRUint16;
typedef short PRInt16;
# 279 "/usr/include/nspr/prtypes.h"
typedef unsigned int PRUint32;
typedef int PRInt32;
# 334 "/usr/include/nspr/prtypes.h"
typedef long PRInt64;
typedef unsigned long PRUint64;
# 379 "/usr/include/nspr/prtypes.h"
typedef int PRIntn;
typedef unsigned int PRUintn;
# 390 "/usr/include/nspr/prtypes.h"
typedef double PRFloat64;






typedef size_t PRSize;







typedef PRInt32 PROffset32;
typedef PRInt64 PROffset64;







typedef ptrdiff_t PRPtrdiff;
# 425 "/usr/include/nspr/prtypes.h"
typedef unsigned long PRUptrdiff;
# 436 "/usr/include/nspr/prtypes.h"
typedef PRIntn PRBool;
# 446 "/usr/include/nspr/prtypes.h"
typedef PRUint8 PRPackedBool;





typedef enum { PR_FAILURE = -1, PR_SUCCESS = 0 } PRStatus;






typedef PRUint16 PRUnichar;
# 478 "/usr/include/nspr/prtypes.h"
typedef long PRWord;
typedef unsigned long PRUword;
# 556 "/usr/include/nspr/prtypes.h"

# 15 "/usr/include/nspr/pratom.h" 2
# 1 "/usr/include/nspr/prlock.h" 1
# 21 "/usr/include/nspr/prlock.h"

# 34 "/usr/include/nspr/prlock.h"
typedef struct PRLock PRLock;
# 51 "/usr/include/nspr/prlock.h"
extern __attribute__((visibility("default"))) PRLock* PR_NewLock(void);
# 62 "/usr/include/nspr/prlock.h"
extern __attribute__((visibility("default"))) void PR_DestroyLock(PRLock *lock);
# 73 "/usr/include/nspr/prlock.h"
extern __attribute__((visibility("default"))) void PR_Lock(PRLock *lock);
# 85 "/usr/include/nspr/prlock.h"
extern __attribute__((visibility("default"))) PRStatus PR_Unlock(PRLock *lock);
# 105 "/usr/include/nspr/prlock.h"
extern __attribute__((visibility("default"))) void PR_AssertCurrentThreadOwnsLock(PRLock *lock);


# 16 "/usr/include/nspr/pratom.h" 2


# 28 "/usr/include/nspr/pratom.h"
extern __attribute__((visibility("default"))) PRInt32 PR_AtomicIncrement(PRInt32 *val);
# 39 "/usr/include/nspr/pratom.h"
extern __attribute__((visibility("default"))) PRInt32 PR_AtomicDecrement(PRInt32 *val);
# 51 "/usr/include/nspr/pratom.h"
extern __attribute__((visibility("default"))) PRInt32 PR_AtomicSet(PRInt32 *val, PRInt32 newval);
# 63 "/usr/include/nspr/pratom.h"
extern __attribute__((visibility("default"))) PRInt32 PR_AtomicAdd(PRInt32 *ptr, PRInt32 val);
# 137 "/usr/include/nspr/pratom.h"
typedef struct PRStackElemStr PRStackElem;

struct PRStackElemStr {
    PRStackElem *prstk_elem_next;

};

typedef struct PRStackStr PRStack;
# 155 "/usr/include/nspr/pratom.h"
extern __attribute__((visibility("default"))) PRStack * PR_CreateStack(const char *stack_name);
# 167 "/usr/include/nspr/pratom.h"
extern __attribute__((visibility("default"))) void PR_StackPush(PRStack *stack, PRStackElem *stack_elem);
# 180 "/usr/include/nspr/pratom.h"
extern __attribute__((visibility("default"))) PRStackElem * PR_StackPop(PRStack *stack);
# 194 "/usr/include/nspr/pratom.h"
extern __attribute__((visibility("default"))) PRStatus PR_DestroyStack(PRStack *stack);


# 10 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prbit.h" 1
# 10 "/usr/include/nspr/prbit.h"

# 47 "/usr/include/nspr/prbit.h"
typedef unsigned long prbitmap_t;
# 59 "/usr/include/nspr/prbit.h"
extern __attribute__((visibility("default"))) PRIntn PR_CeilingLog2(PRUint32 i);




extern __attribute__((visibility("default"))) PRIntn PR_FloorLog2(PRUint32 i);
# 151 "/usr/include/nspr/prbit.h"

# 11 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prclist.h" 1
# 11 "/usr/include/nspr/prclist.h"
typedef struct PRCListStr PRCList;




struct PRCListStr {
    PRCList *next;
    PRCList *prev;
};
# 12 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prcmon.h" 1
# 19 "/usr/include/nspr/prcmon.h"
# 1 "/usr/include/nspr/prmon.h" 1
# 10 "/usr/include/nspr/prmon.h"
# 1 "/usr/include/nspr/prinrval.h" 1
# 23 "/usr/include/nspr/prinrval.h"






typedef PRUint32 PRIntervalTime;
# 79 "/usr/include/nspr/prinrval.h"
extern __attribute__((visibility("default"))) PRIntervalTime PR_IntervalNow(void);
# 97 "/usr/include/nspr/prinrval.h"
extern __attribute__((visibility("default"))) PRUint32 PR_TicksPerSecond(void);
# 116 "/usr/include/nspr/prinrval.h"
extern __attribute__((visibility("default"))) PRIntervalTime PR_SecondsToInterval(PRUint32 seconds);
extern __attribute__((visibility("default"))) PRIntervalTime PR_MillisecondsToInterval(PRUint32 milli);
extern __attribute__((visibility("default"))) PRIntervalTime PR_MicrosecondsToInterval(PRUint32 micro);
# 137 "/usr/include/nspr/prinrval.h"
extern __attribute__((visibility("default"))) PRUint32 PR_IntervalToSeconds(PRIntervalTime ticks);
extern __attribute__((visibility("default"))) PRUint32 PR_IntervalToMilliseconds(PRIntervalTime ticks);
extern __attribute__((visibility("default"))) PRUint32 PR_IntervalToMicroseconds(PRIntervalTime ticks);


# 11 "/usr/include/nspr/prmon.h" 2



typedef struct PRMonitor PRMonitor;
# 23 "/usr/include/nspr/prmon.h"
extern __attribute__((visibility("default"))) PRMonitor* PR_NewMonitor(void);







extern __attribute__((visibility("default"))) void PR_DestroyMonitor(PRMonitor *mon);






extern __attribute__((visibility("default"))) void PR_EnterMonitor(PRMonitor *mon);






extern __attribute__((visibility("default"))) PRStatus PR_ExitMonitor(PRMonitor *mon);
# 63 "/usr/include/nspr/prmon.h"
extern __attribute__((visibility("default"))) PRStatus PR_Wait(PRMonitor *mon, PRIntervalTime ticks);






extern __attribute__((visibility("default"))) PRStatus PR_Notify(PRMonitor *mon);






extern __attribute__((visibility("default"))) PRStatus PR_NotifyAll(PRMonitor *mon);
# 92 "/usr/include/nspr/prmon.h"
extern __attribute__((visibility("default"))) void PR_AssertCurrentThreadInMonitor(PRMonitor *mon);


# 20 "/usr/include/nspr/prcmon.h" 2



# 32 "/usr/include/nspr/prcmon.h"
extern __attribute__((visibility("default"))) PRMonitor* PR_CEnterMonitor(void *address);





extern __attribute__((visibility("default"))) PRStatus PR_CExitMonitor(void *address);





extern __attribute__((visibility("default"))) PRStatus PR_CWait(void *address, PRIntervalTime timeout);





extern __attribute__((visibility("default"))) PRStatus PR_CNotify(void *address);





extern __attribute__((visibility("default"))) PRStatus PR_CNotifyAll(void *address);





extern __attribute__((visibility("default"))) void PR_CSetOnMonitorRecycle(void ( *callback)(void *address));


# 13 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prcvar.h" 1
# 12 "/usr/include/nspr/prcvar.h"


typedef struct PRCondVar PRCondVar;
# 27 "/usr/include/nspr/prcvar.h"
extern __attribute__((visibility("default"))) PRCondVar* PR_NewCondVar(PRLock *lock);







extern __attribute__((visibility("default"))) void PR_DestroyCondVar(PRCondVar *cvar);
# 65 "/usr/include/nspr/prcvar.h"
extern __attribute__((visibility("default"))) PRStatus PR_WaitCondVar(PRCondVar *cvar, PRIntervalTime timeout);
# 80 "/usr/include/nspr/prcvar.h"
extern __attribute__((visibility("default"))) PRStatus PR_NotifyCondVar(PRCondVar *cvar);
# 90 "/usr/include/nspr/prcvar.h"
extern __attribute__((visibility("default"))) PRStatus PR_NotifyAllCondVar(PRCondVar *cvar);


# 14 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prdtoa.h" 1
# 11 "/usr/include/nspr/prdtoa.h"

# 24 "/usr/include/nspr/prdtoa.h"
extern __attribute__((visibility("default"))) PRFloat64
PR_strtod(const char *s00, char **se);







extern __attribute__((visibility("default"))) void PR_cnvtf(char *buf, PRIntn bufsz, PRIntn prcsn, PRFloat64 fval);
# 46 "/usr/include/nspr/prdtoa.h"
extern __attribute__((visibility("default"))) PRStatus PR_dtoa(PRFloat64 d, PRIntn mode, PRIntn ndigits,
                           PRIntn *decpt, PRIntn *sign, char **rve, char *buf, PRSize bufsize);


# 15 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prenv.h" 1
# 17 "/usr/include/nspr/prenv.h"

# 91 "/usr/include/nspr/prenv.h"
extern __attribute__((visibility("default"))) char* PR_GetEnv(const char *var);
# 105 "/usr/include/nspr/prenv.h"
extern __attribute__((visibility("default"))) char* PR_GetEnvSecure(const char *var);
# 128 "/usr/include/nspr/prenv.h"
extern __attribute__((visibility("default"))) PRStatus PR_SetEnv(const char *string);
# 158 "/usr/include/nspr/prenv.h"
extern __attribute__((visibility("default"))) char ** PR_DuplicateEnvironment(void);


# 16 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prerror.h" 1
# 11 "/usr/include/nspr/prerror.h"


typedef PRInt32 PRErrorCode;



# 1 "/usr/include/nspr/prerr.h" 1
# 246 "/usr/include/nspr/prerr.h"
extern void nspr_InitializePRErrorTable(void);
# 18 "/usr/include/nspr/prerror.h" 2







extern __attribute__((visibility("default"))) void PR_SetError(PRErrorCode errorCode, PRInt32 oserr);
# 36 "/usr/include/nspr/prerror.h"
extern __attribute__((visibility("default"))) void PR_SetErrorText(
    PRIntn textLength, const char *text);




extern __attribute__((visibility("default"))) PRErrorCode PR_GetError(void);





extern __attribute__((visibility("default"))) PRInt32 PR_GetOSError(void);






extern __attribute__((visibility("default"))) PRInt32 PR_GetErrorTextLength(void);






extern __attribute__((visibility("default"))) PRInt32 PR_GetErrorText(char *text);
# 145 "/usr/include/nspr/prerror.h"
typedef PRUint32 PRLanguageCode;
# 154 "/usr/include/nspr/prerror.h"
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
# 196 "/usr/include/nspr/prerror.h"
typedef const char *
PRErrorCallbackLookupFn(PRErrorCode code, PRLanguageCode language,
                        const struct PRErrorTable *table,
                        struct PRErrorCallbackPrivate *cb_private,
                        struct PRErrorCallbackTablePrivate *table_private);
# 212 "/usr/include/nspr/prerror.h"
typedef struct PRErrorCallbackTablePrivate *
PRErrorCallbackNewTableFn(const struct PRErrorTable *table,
                          struct PRErrorCallbackPrivate *cb_private);
# 230 "/usr/include/nspr/prerror.h"
extern __attribute__((visibility("default"))) const char * PR_ErrorToString(PRErrorCode code,
                                        PRLanguageCode language);
# 244 "/usr/include/nspr/prerror.h"
extern __attribute__((visibility("default"))) const char * PR_ErrorToName(PRErrorCode code);
# 258 "/usr/include/nspr/prerror.h"
extern __attribute__((visibility("default"))) const char * const * PR_ErrorLanguages(void);
# 271 "/usr/include/nspr/prerror.h"
extern __attribute__((visibility("default"))) PRErrorCode PR_ErrorInstallTable(const struct PRErrorTable *table);
# 287 "/usr/include/nspr/prerror.h"
extern __attribute__((visibility("default"))) void PR_ErrorInstallCallback(const char * const * languages,
                                       PRErrorCallbackLookupFn *lookup,
                                       PRErrorCallbackNewTableFn *newtable,
                                       struct PRErrorCallbackPrivate *cb_private);


# 17 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prinet.h" 1
# 36 "/usr/include/nspr/prinet.h"
# 1 "/usr/include/sys/types.h" 1 3 4
# 25 "/usr/include/sys/types.h" 3 4
# 1 "/usr/include/features.h" 1 3 4
# 402 "/usr/include/features.h" 3 4
# 1 "/usr/include/features-time64.h" 1 3 4
# 20 "/usr/include/features-time64.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 21 "/usr/include/features-time64.h" 2 3 4
# 1 "/usr/include/bits/timesize.h" 1 3 4
# 19 "/usr/include/bits/timesize.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 20 "/usr/include/bits/timesize.h" 2 3 4
# 22 "/usr/include/features-time64.h" 2 3 4
# 403 "/usr/include/features.h" 2 3 4
# 511 "/usr/include/features.h" 3 4
# 1 "/usr/include/sys/cdefs.h" 1 3 4
# 730 "/usr/include/sys/cdefs.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 731 "/usr/include/sys/cdefs.h" 2 3 4
# 1 "/usr/include/bits/long-double.h" 1 3 4
# 732 "/usr/include/sys/cdefs.h" 2 3 4
# 512 "/usr/include/features.h" 2 3 4
# 535 "/usr/include/features.h" 3 4
# 1 "/usr/include/gnu/stubs.h" 1 3 4
# 10 "/usr/include/gnu/stubs.h" 3 4
# 1 "/usr/include/gnu/stubs-64.h" 1 3 4
# 11 "/usr/include/gnu/stubs.h" 2 3 4
# 536 "/usr/include/features.h" 2 3 4
# 26 "/usr/include/sys/types.h" 2 3 4



# 1 "/usr/include/bits/types.h" 1 3 4
# 27 "/usr/include/bits/types.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 28 "/usr/include/bits/types.h" 2 3 4
# 1 "/usr/include/bits/timesize.h" 1 3 4
# 19 "/usr/include/bits/timesize.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 20 "/usr/include/bits/timesize.h" 2 3 4
# 29 "/usr/include/bits/types.h" 2 3 4



# 31 "/usr/include/bits/types.h" 3 4
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

typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;






typedef __int8_t __int_least8_t;
typedef __uint8_t __uint_least8_t;
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
typedef __int64_t __int_least64_t;
typedef __uint64_t __uint_least64_t;



typedef long int __quad_t;
typedef unsigned long int __u_quad_t;







typedef long int __intmax_t;
typedef unsigned long int __uintmax_t;
# 141 "/usr/include/bits/types.h" 3 4
# 1 "/usr/include/bits/typesizes.h" 1 3 4
# 142 "/usr/include/bits/types.h" 2 3 4
# 1 "/usr/include/bits/time64.h" 1 3 4
# 143 "/usr/include/bits/types.h" 2 3 4


typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __ino64_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __off64_t;
typedef int __pid_t;
typedef struct { int __val[2]; } __fsid_t;
typedef long int __clock_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned int __id_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;
typedef long int __suseconds64_t;

typedef int __daddr_t;
typedef int __key_t;


typedef int __clockid_t;


typedef void * __timer_t;


typedef long int __blksize_t;




typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;


typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;


typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;


typedef long int __fsword_t;

typedef long int __ssize_t;


typedef long int __syscall_slong_t;

typedef unsigned long int __syscall_ulong_t;



typedef __off64_t __loff_t;
typedef char *__caddr_t;


typedef long int __intptr_t;


typedef unsigned int __socklen_t;




typedef int __sig_atomic_t;
# 30 "/usr/include/sys/types.h" 2 3 4



typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;


typedef __loff_t loff_t;




typedef __ino_t ino_t;
# 59 "/usr/include/sys/types.h" 3 4
typedef __dev_t dev_t;




typedef __gid_t gid_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;




typedef __uid_t uid_t;





typedef __off_t off_t;
# 97 "/usr/include/sys/types.h" 3 4
typedef __pid_t pid_t;





typedef __id_t id_t;




typedef __ssize_t ssize_t;





typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;





typedef __key_t key_t;




# 1 "/usr/include/bits/types/clock_t.h" 1 3 4






typedef __clock_t clock_t;
# 127 "/usr/include/sys/types.h" 2 3 4

# 1 "/usr/include/bits/types/clockid_t.h" 1 3 4






typedef __clockid_t clockid_t;
# 129 "/usr/include/sys/types.h" 2 3 4
# 1 "/usr/include/bits/types/time_t.h" 1 3 4
# 10 "/usr/include/bits/types/time_t.h" 3 4
typedef __time_t time_t;
# 130 "/usr/include/sys/types.h" 2 3 4
# 1 "/usr/include/bits/types/timer_t.h" 1 3 4






typedef __timer_t timer_t;
# 131 "/usr/include/sys/types.h" 2 3 4
# 144 "/usr/include/sys/types.h" 3 4
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 1 3 4
# 145 "/usr/include/sys/types.h" 2 3 4



typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;




# 1 "/usr/include/bits/stdint-intn.h" 1 3 4
# 24 "/usr/include/bits/stdint-intn.h" 3 4
typedef __int8_t int8_t;
typedef __int16_t int16_t;
typedef __int32_t int32_t;
typedef __int64_t int64_t;
# 156 "/usr/include/sys/types.h" 2 3 4


typedef __uint8_t u_int8_t;
typedef __uint16_t u_int16_t;
typedef __uint32_t u_int32_t;
typedef __uint64_t u_int64_t;


typedef int register_t __attribute__ ((__mode__ (__word__)));
# 176 "/usr/include/sys/types.h" 3 4
# 1 "/usr/include/endian.h" 1 3 4
# 24 "/usr/include/endian.h" 3 4
# 1 "/usr/include/bits/endian.h" 1 3 4
# 35 "/usr/include/bits/endian.h" 3 4
# 1 "/usr/include/bits/endianness.h" 1 3 4
# 36 "/usr/include/bits/endian.h" 2 3 4
# 25 "/usr/include/endian.h" 2 3 4
# 35 "/usr/include/endian.h" 3 4
# 1 "/usr/include/bits/byteswap.h" 1 3 4
# 33 "/usr/include/bits/byteswap.h" 3 4
static __inline __uint16_t
__bswap_16 (__uint16_t __bsx)
{

  return __builtin_bswap16 (__bsx);



}






static __inline __uint32_t
__bswap_32 (__uint32_t __bsx)
{

  return __builtin_bswap32 (__bsx);



}
# 69 "/usr/include/bits/byteswap.h" 3 4
__extension__ static __inline __uint64_t
__bswap_64 (__uint64_t __bsx)
{

  return __builtin_bswap64 (__bsx);



}
# 36 "/usr/include/endian.h" 2 3 4
# 1 "/usr/include/bits/uintn-identity.h" 1 3 4
# 32 "/usr/include/bits/uintn-identity.h" 3 4
static __inline __uint16_t
__uint16_identity (__uint16_t __x)
{
  return __x;
}

static __inline __uint32_t
__uint32_identity (__uint32_t __x)
{
  return __x;
}

static __inline __uint64_t
__uint64_identity (__uint64_t __x)
{
  return __x;
}
# 37 "/usr/include/endian.h" 2 3 4
# 177 "/usr/include/sys/types.h" 2 3 4


# 1 "/usr/include/sys/select.h" 1 3 4
# 30 "/usr/include/sys/select.h" 3 4
# 1 "/usr/include/bits/select.h" 1 3 4
# 31 "/usr/include/sys/select.h" 2 3 4


# 1 "/usr/include/bits/types/sigset_t.h" 1 3 4



# 1 "/usr/include/bits/types/__sigset_t.h" 1 3 4




typedef struct
{
  unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
} __sigset_t;
# 5 "/usr/include/bits/types/sigset_t.h" 2 3 4


typedef __sigset_t sigset_t;
# 34 "/usr/include/sys/select.h" 2 3 4



# 1 "/usr/include/bits/types/struct_timeval.h" 1 3 4







struct timeval
{




  __time_t tv_sec;
  __suseconds_t tv_usec;

};
# 38 "/usr/include/sys/select.h" 2 3 4

# 1 "/usr/include/bits/types/struct_timespec.h" 1 3 4
# 11 "/usr/include/bits/types/struct_timespec.h" 3 4
struct timespec
{



  __time_t tv_sec;




  __syscall_slong_t tv_nsec;
# 31 "/usr/include/bits/types/struct_timespec.h" 3 4
};
# 40 "/usr/include/sys/select.h" 2 3 4



typedef __suseconds_t suseconds_t;





typedef long int __fd_mask;
# 59 "/usr/include/sys/select.h" 3 4
typedef struct
  {






    __fd_mask __fds_bits[1024 / (8 * (int) sizeof (__fd_mask))];


  } fd_set;






typedef __fd_mask fd_mask;
# 91 "/usr/include/sys/select.h" 3 4

# 102 "/usr/include/sys/select.h" 3 4
extern int select (int __nfds, fd_set *__restrict __readfds,
     fd_set *__restrict __writefds,
     fd_set *__restrict __exceptfds,
     struct timeval *__restrict __timeout);
# 127 "/usr/include/sys/select.h" 3 4
extern int pselect (int __nfds, fd_set *__restrict __readfds,
      fd_set *__restrict __writefds,
      fd_set *__restrict __exceptfds,
      const struct timespec *__restrict __timeout,
      const __sigset_t *__restrict __sigmask);
# 150 "/usr/include/sys/select.h" 3 4
# 1 "/usr/include/bits/select2.h" 1 3 4
# 23 "/usr/include/bits/select2.h" 3 4
# 1 "/usr/include/bits/select-decl.h" 1 3 4
# 27 "/usr/include/bits/select-decl.h" 3 4
extern long int __fdelt_chk (long int __d);
extern long int __fdelt_warn (long int __d)
  __attribute__((__warning__ ("bit outside of fd_set selected")));
# 24 "/usr/include/bits/select2.h" 2 3 4
# 151 "/usr/include/sys/select.h" 2 3 4



# 180 "/usr/include/sys/types.h" 2 3 4





typedef __blksize_t blksize_t;






typedef __blkcnt_t blkcnt_t;



typedef __fsblkcnt_t fsblkcnt_t;



typedef __fsfilcnt_t fsfilcnt_t;
# 227 "/usr/include/sys/types.h" 3 4
# 1 "/usr/include/bits/pthreadtypes.h" 1 3 4
# 23 "/usr/include/bits/pthreadtypes.h" 3 4
# 1 "/usr/include/bits/thread-shared-types.h" 1 3 4
# 44 "/usr/include/bits/thread-shared-types.h" 3 4
# 1 "/usr/include/bits/pthreadtypes-arch.h" 1 3 4
# 21 "/usr/include/bits/pthreadtypes-arch.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 22 "/usr/include/bits/pthreadtypes-arch.h" 2 3 4
# 45 "/usr/include/bits/thread-shared-types.h" 2 3 4

# 1 "/usr/include/bits/atomic_wide_counter.h" 1 3 4
# 25 "/usr/include/bits/atomic_wide_counter.h" 3 4
typedef union
{
  __extension__ unsigned long long int __value64;
  struct
  {
    unsigned int __low;
    unsigned int __high;
  } __value32;
} __atomic_wide_counter;
# 47 "/usr/include/bits/thread-shared-types.h" 2 3 4




typedef struct __pthread_internal_list
{
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;

typedef struct __pthread_internal_slist
{
  struct __pthread_internal_slist *__next;
} __pthread_slist_t;
# 76 "/usr/include/bits/thread-shared-types.h" 3 4
# 1 "/usr/include/bits/struct_mutex.h" 1 3 4
# 22 "/usr/include/bits/struct_mutex.h" 3 4
struct __pthread_mutex_s
{
  int __lock;
  unsigned int __count;
  int __owner;

  unsigned int __nusers;



  int __kind;

  short __spins;
  short __elision;
  __pthread_list_t __list;
# 53 "/usr/include/bits/struct_mutex.h" 3 4
};
# 77 "/usr/include/bits/thread-shared-types.h" 2 3 4
# 89 "/usr/include/bits/thread-shared-types.h" 3 4
# 1 "/usr/include/bits/struct_rwlock.h" 1 3 4
# 23 "/usr/include/bits/struct_rwlock.h" 3 4
struct __pthread_rwlock_arch_t
{
  unsigned int __readers;
  unsigned int __writers;
  unsigned int __wrphase_futex;
  unsigned int __writers_futex;
  unsigned int __pad3;
  unsigned int __pad4;

  int __cur_writer;
  int __shared;
  signed char __rwelision;




  unsigned char __pad1[7];


  unsigned long int __pad2;


  unsigned int __flags;
# 55 "/usr/include/bits/struct_rwlock.h" 3 4
};
# 90 "/usr/include/bits/thread-shared-types.h" 2 3 4




struct __pthread_cond_s
{
  __atomic_wide_counter __wseq;
  __atomic_wide_counter __g1_start;
  unsigned int __g_refs[2] ;
  unsigned int __g_size[2];
  unsigned int __g1_orig_size;
  unsigned int __wrefs;
  unsigned int __g_signals[2];
};

typedef unsigned int __tss_t;
typedef unsigned long int __thrd_t;

typedef struct
{
  int __data ;
} __once_flag;
# 24 "/usr/include/bits/pthreadtypes.h" 2 3 4



typedef unsigned long int pthread_t;




typedef union
{
  char __size[4];
  int __align;
} pthread_mutexattr_t;




typedef union
{
  char __size[4];
  int __align;
} pthread_condattr_t;



typedef unsigned int pthread_key_t;



typedef int pthread_once_t;


union pthread_attr_t
{
  char __size[56];
  long int __align;
};

typedef union pthread_attr_t pthread_attr_t;




typedef union
{
  struct __pthread_mutex_s __data;
  char __size[40];
  long int __align;
} pthread_mutex_t;


typedef union
{
  struct __pthread_cond_s __data;
  char __size[48];
  __extension__ long long int __align;
} pthread_cond_t;





typedef union
{
  struct __pthread_rwlock_arch_t __data;
  char __size[56];
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
  char __size[32];
  long int __align;
} pthread_barrier_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_barrierattr_t;
# 228 "/usr/include/sys/types.h" 2 3 4



# 37 "/usr/include/nspr/prinet.h" 2
# 1 "/usr/include/sys/socket.h" 1 3 4
# 24 "/usr/include/sys/socket.h" 3 4


# 1 "/usr/include/bits/types/struct_iovec.h" 1 3 4
# 23 "/usr/include/bits/types/struct_iovec.h" 3 4
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 1 3 4
# 24 "/usr/include/bits/types/struct_iovec.h" 2 3 4


struct iovec
  {
    void *iov_base;
    size_t iov_len;
  };
# 27 "/usr/include/sys/socket.h" 2 3 4

# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 1 3 4
# 29 "/usr/include/sys/socket.h" 2 3 4




# 1 "/usr/include/bits/socket.h" 1 3 4
# 27 "/usr/include/bits/socket.h" 3 4
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 1 3 4
# 28 "/usr/include/bits/socket.h" 2 3 4





typedef __socklen_t socklen_t;




# 1 "/usr/include/bits/socket_type.h" 1 3 4
# 24 "/usr/include/bits/socket_type.h" 3 4
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


  SOCK_NONBLOCK = 00004000


};
# 39 "/usr/include/bits/socket.h" 2 3 4
# 181 "/usr/include/bits/socket.h" 3 4
# 1 "/usr/include/bits/sockaddr.h" 1 3 4
# 28 "/usr/include/bits/sockaddr.h" 3 4
typedef unsigned short int sa_family_t;
# 182 "/usr/include/bits/socket.h" 2 3 4


struct __attribute__ ((__may_alias__)) sockaddr
  {
    sa_family_t sa_family;
    char sa_data[14];
  };
# 197 "/usr/include/bits/socket.h" 3 4
struct __attribute__ ((__may_alias__)) sockaddr_storage
  {
    sa_family_t ss_family;
    char __ss_padding[(128 - (sizeof (unsigned short int)) - sizeof (unsigned long int))];
    unsigned long int __ss_align;
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

    MSG_BATCH = 0x40000,

    MSG_ZEROCOPY = 0x4000000,

    MSG_FASTOPEN = 0x20000000,


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


struct cmsghdr
  {
    size_t cmsg_len;




    int cmsg_level;
    int cmsg_type;

    __extension__ unsigned char __cmsg_data [];

  };
# 317 "/usr/include/bits/socket.h" 3 4
extern struct cmsghdr *__cmsg_nxthdr (struct msghdr *__mhdr,
          struct cmsghdr *__cmsg) __attribute__ ((__nothrow__ , __leaf__));




extern __inline __attribute__ ((__gnu_inline__)) struct cmsghdr *
__attribute__ ((__nothrow__ , __leaf__)) __cmsg_nxthdr (struct msghdr *__mhdr, struct cmsghdr *__cmsg)
{







  unsigned char * __msg_control_ptr = (unsigned char *) __mhdr->msg_control;
  unsigned char * __cmsg_ptr = (unsigned char *) __cmsg;

  size_t __size_needed = sizeof (struct cmsghdr)
                         + ((sizeof (size_t) - ((__cmsg->cmsg_len) & (sizeof (size_t) - 1))) & (sizeof (size_t) - 1));


  if ((size_t) __cmsg->cmsg_len < sizeof (struct cmsghdr))
    return (struct cmsghdr *) 0;



  if (((size_t)
         (__msg_control_ptr + __mhdr->msg_controllen - __cmsg_ptr)
       < __size_needed)
      || ((size_t)
            (__msg_control_ptr + __mhdr->msg_controllen - __cmsg_ptr
             - __size_needed)
          < __cmsg->cmsg_len))

    return (struct cmsghdr *) 0;


  __cmsg = (struct cmsghdr *) ((unsigned char *) __cmsg
          + (((__cmsg->cmsg_len) + sizeof (size_t) - 1) & (size_t) ~(sizeof (size_t) - 1)));
  return __cmsg;
}




enum
  {
    SCM_RIGHTS = 0x01
# 376 "/usr/include/bits/socket.h" 3 4
  };
# 390 "/usr/include/bits/socket.h" 3 4
# 1 "/usr/include/asm/socket.h" 1 3 4
# 1 "/usr/include/asm-generic/socket.h" 1 3 4




# 1 "/usr/include/linux/posix_types.h" 1 3 4




# 1 "/usr/include/linux/stddef.h" 1 3 4
# 6 "/usr/include/linux/posix_types.h" 2 3 4
# 25 "/usr/include/linux/posix_types.h" 3 4
typedef struct {
 unsigned long fds_bits[1024 / (8 * sizeof(long))];
} __kernel_fd_set;


typedef void (*__kernel_sighandler_t)(int);


typedef int __kernel_key_t;
typedef int __kernel_mqd_t;

# 1 "/usr/include/asm/posix_types.h" 1 3 4






# 1 "/usr/include/asm/posix_types_64.h" 1 3 4
# 11 "/usr/include/asm/posix_types_64.h" 3 4
typedef unsigned short __kernel_old_uid_t;
typedef unsigned short __kernel_old_gid_t;


typedef unsigned long __kernel_old_dev_t;


# 1 "/usr/include/asm-generic/posix_types.h" 1 3 4




# 1 "/usr/include/asm/bitsperlong.h" 1 3 4
# 11 "/usr/include/asm/bitsperlong.h" 3 4
# 1 "/usr/include/asm-generic/bitsperlong.h" 1 3 4
# 12 "/usr/include/asm/bitsperlong.h" 2 3 4
# 6 "/usr/include/asm-generic/posix_types.h" 2 3 4
# 15 "/usr/include/asm-generic/posix_types.h" 3 4
typedef long __kernel_long_t;
typedef unsigned long __kernel_ulong_t;



typedef __kernel_ulong_t __kernel_ino_t;



typedef unsigned int __kernel_mode_t;



typedef int __kernel_pid_t;



typedef int __kernel_ipc_pid_t;



typedef unsigned int __kernel_uid_t;
typedef unsigned int __kernel_gid_t;



typedef __kernel_long_t __kernel_suseconds_t;



typedef int __kernel_daddr_t;



typedef unsigned int __kernel_uid32_t;
typedef unsigned int __kernel_gid32_t;
# 72 "/usr/include/asm-generic/posix_types.h" 3 4
typedef __kernel_ulong_t __kernel_size_t;
typedef __kernel_long_t __kernel_ssize_t;
typedef __kernel_long_t __kernel_ptrdiff_t;




typedef struct {
 int val[2];
} __kernel_fsid_t;





typedef __kernel_long_t __kernel_off_t;
typedef long long __kernel_loff_t;
typedef __kernel_long_t __kernel_old_time_t;
typedef __kernel_long_t __kernel_time_t;
typedef long long __kernel_time64_t;
typedef __kernel_long_t __kernel_clock_t;
typedef int __kernel_timer_t;
typedef int __kernel_clockid_t;
typedef char * __kernel_caddr_t;
typedef unsigned short __kernel_uid16_t;
typedef unsigned short __kernel_gid16_t;
# 19 "/usr/include/asm/posix_types_64.h" 2 3 4
# 8 "/usr/include/asm/posix_types.h" 2 3 4
# 37 "/usr/include/linux/posix_types.h" 2 3 4
# 6 "/usr/include/asm-generic/socket.h" 2 3 4
# 1 "/usr/include/asm/sockios.h" 1 3 4
# 1 "/usr/include/asm-generic/sockios.h" 1 3 4
# 2 "/usr/include/asm/sockios.h" 2 3 4
# 7 "/usr/include/asm-generic/socket.h" 2 3 4
# 2 "/usr/include/asm/socket.h" 2 3 4
# 391 "/usr/include/bits/socket.h" 2 3 4






struct linger
  {
    int l_onoff;
    int l_linger;
  };
# 34 "/usr/include/sys/socket.h" 2 3 4


# 1 "/usr/include/bits/types/struct_osockaddr.h" 1 3 4





struct osockaddr
{
  unsigned short int sa_family;
  unsigned char sa_data[14];
};
# 37 "/usr/include/sys/socket.h" 2 3 4




enum
{
  SHUT_RD = 0,

  SHUT_WR,

  SHUT_RDWR

};
# 102 "/usr/include/sys/socket.h" 3 4
extern int socket (int __domain, int __type, int __protocol) __attribute__ ((__nothrow__ , __leaf__));





extern int socketpair (int __domain, int __type, int __protocol,
         int __fds[2]) __attribute__ ((__nothrow__ , __leaf__));


extern int bind (int __fd, const struct sockaddr * __addr, socklen_t __len)
     __attribute__ ((__nothrow__ , __leaf__));


extern int getsockname (int __fd, struct sockaddr *__restrict __addr,
   socklen_t *__restrict __len) __attribute__ ((__nothrow__ , __leaf__));
# 126 "/usr/include/sys/socket.h" 3 4
extern int connect (int __fd, const struct sockaddr * __addr, socklen_t __len);



extern int getpeername (int __fd, struct sockaddr *__restrict __addr,
   socklen_t *__restrict __len) __attribute__ ((__nothrow__ , __leaf__));






extern ssize_t send (int __fd, const void *__buf, size_t __n, int __flags);






extern ssize_t recv (int __fd, void *__buf, size_t __n, int __flags);






extern ssize_t sendto (int __fd, const void *__buf, size_t __n,
         int __flags, const struct sockaddr * __addr,
         socklen_t __addr_len);
# 163 "/usr/include/sys/socket.h" 3 4
extern ssize_t recvfrom (int __fd, void *__restrict __buf, size_t __n,
    int __flags, struct sockaddr *__restrict __addr,
    socklen_t *__restrict __addr_len);
# 174 "/usr/include/sys/socket.h" 3 4
extern ssize_t sendmsg (int __fd, const struct msghdr *__message,
   int __flags);
# 216 "/usr/include/sys/socket.h" 3 4
extern ssize_t recvmsg (int __fd, struct msghdr *__message, int __flags);
# 255 "/usr/include/sys/socket.h" 3 4
extern int getsockopt (int __fd, int __level, int __optname,
         void *__restrict __optval,
         socklen_t *__restrict __optlen) __attribute__ ((__nothrow__ , __leaf__));
# 277 "/usr/include/sys/socket.h" 3 4
extern int setsockopt (int __fd, int __level, int __optname,
         const void *__optval, socklen_t __optlen) __attribute__ ((__nothrow__ , __leaf__));
# 296 "/usr/include/sys/socket.h" 3 4
extern int listen (int __fd, int __n) __attribute__ ((__nothrow__ , __leaf__));
# 306 "/usr/include/sys/socket.h" 3 4
extern int accept (int __fd, struct sockaddr *__restrict __addr,
     socklen_t *__restrict __addr_len);
# 324 "/usr/include/sys/socket.h" 3 4
extern int shutdown (int __fd, int __how) __attribute__ ((__nothrow__ , __leaf__));




extern int sockatmark (int __fd) __attribute__ ((__nothrow__ , __leaf__));







extern int isfdtype (int __fd, int __fdtype) __attribute__ ((__nothrow__ , __leaf__));





# 1 "/usr/include/bits/socket2.h" 1 3 4
# 23 "/usr/include/bits/socket2.h" 3 4
extern ssize_t __recv_chk (int __fd, void *__buf, size_t __n, size_t __buflen,
      int __flags);
extern ssize_t __recv_alias (int __fd, void *__buf, size_t __n, int __flags) __asm__ ("" "recv")
                          ;
extern ssize_t __recv_chk_warn (int __fd, void *__buf, size_t __n, size_t __buflen, int __flags) __asm__ ("" "__recv_chk")


     __attribute__((__warning__ ("recv called with bigger length than size of destination " "buffer")))
            ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) ssize_t
recv (int __fd, void * __buf, size_t __n,
      int __flags)
    


{
  size_t __sz = __builtin_object_size (__buf, 0);
  if (((__builtin_constant_p (__sz) && (__sz) == (long unsigned int) -1) || (((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= ((__sz)) / ((sizeof (char))))) && (((long unsigned int) (__n)) <= ((__sz)) / ((sizeof (char)))))))
    return __recv_alias (__fd, __buf, __n, __flags);

  if ((((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= (__sz) / (sizeof (char)))) && !(((long unsigned int) (__n)) <= (__sz) / (sizeof (char)))))
    return __recv_chk_warn (__fd, __buf, __n, __sz, __flags);

  return __recv_chk (__fd, __buf, __n, __sz, __flags);
}

extern ssize_t __recvfrom_chk (int __fd, void *__restrict __buf, size_t __n,
          size_t __buflen, int __flags,
          struct sockaddr *__restrict __addr,
          socklen_t *__restrict __addr_len);
extern ssize_t __recvfrom_alias (int __fd, void *__restrict __buf, size_t __n, int __flags, struct sockaddr *__restrict __addr, socklen_t *__restrict __addr_len) __asm__ ("" "recvfrom")


                                                   ;
extern ssize_t __recvfrom_chk_warn (int __fd, void *__restrict __buf, size_t __n, size_t __buflen, int __flags, struct sockaddr *__restrict __addr, socklen_t *__restrict __addr_len) __asm__ ("" "__recvfrom_chk")




     __attribute__((__warning__ ("recvfrom called with bigger length than size of " "destination buffer")))
                        ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) ssize_t
recvfrom (int __fd, void * __restrict __buf,
   size_t __n, int __flags,
   struct sockaddr *__restrict __addr, socklen_t *__restrict __addr_len)
    


{
  size_t __sz = __builtin_object_size (__buf, 0);
  if (((__builtin_constant_p (__sz) && (__sz) == (long unsigned int) -1) || (((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= ((__sz)) / ((sizeof (char))))) && (((long unsigned int) (__n)) <= ((__sz)) / ((sizeof (char)))))))
    return __recvfrom_alias (__fd, __buf, __n, __flags, __addr, __addr_len);

  if ((((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= (__sz) / (sizeof (char)))) && !(((long unsigned int) (__n)) <= (__sz) / (sizeof (char)))))
    return __recvfrom_chk_warn (__fd, __buf, __n, __sz, __flags, __addr,
    __addr_len);

  return __recvfrom_chk (__fd, __buf, __n, __sz, __flags, __addr, __addr_len);
}
# 344 "/usr/include/sys/socket.h" 2 3 4



# 38 "/usr/include/nspr/prinet.h" 2
# 1 "/usr/include/netinet/in.h" 1 3 4
# 22 "/usr/include/netinet/in.h" 3 4
# 1 "/usr/include/bits/stdint-uintn.h" 1 3 4
# 24 "/usr/include/bits/stdint-uintn.h" 3 4
typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;
# 23 "/usr/include/netinet/in.h" 2 3 4







typedef uint32_t in_addr_t;
struct in_addr
  {
    in_addr_t s_addr;
  };


# 1 "/usr/include/bits/in.h" 1 3 4
# 145 "/usr/include/bits/in.h" 3 4
struct ip_opts
  {
    struct in_addr ip_dst;
    char ip_opts[40];
  };


struct in_pktinfo
  {
    int ipi_ifindex;
    struct in_addr ipi_spec_dst;
    struct in_addr ipi_addr;
  };
# 38 "/usr/include/netinet/in.h" 2 3 4


enum
  {
    IPPROTO_IP = 0,

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

    IPPROTO_RSVP = 46,

    IPPROTO_GRE = 47,

    IPPROTO_ESP = 50,

    IPPROTO_AH = 51,

    IPPROTO_MTP = 92,

    IPPROTO_BEETPH = 94,

    IPPROTO_ENCAP = 98,

    IPPROTO_PIM = 103,

    IPPROTO_COMP = 108,

    IPPROTO_L2TP = 115,

    IPPROTO_SCTP = 132,

    IPPROTO_UDPLITE = 136,

    IPPROTO_MPLS = 137,

    IPPROTO_ETHERNET = 143,

    IPPROTO_RAW = 255,

    IPPROTO_MPTCP = 262,

    IPPROTO_MAX
  };





enum
  {
    IPPROTO_HOPOPTS = 0,

    IPPROTO_ROUTING = 43,

    IPPROTO_FRAGMENT = 44,

    IPPROTO_ICMPV6 = 58,

    IPPROTO_NONE = 59,

    IPPROTO_DSTOPTS = 60,

    IPPROTO_MH = 135

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
# 221 "/usr/include/netinet/in.h" 3 4
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
# 247 "/usr/include/netinet/in.h" 3 4
struct __attribute__ ((__may_alias__)) sockaddr_in
  {
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;


    unsigned char sin_zero[sizeof (struct sockaddr)
      - (sizeof (unsigned short int))
      - sizeof (in_port_t)
      - sizeof (struct in_addr)];
  };





struct __attribute__ ((__may_alias__)) sockaddr_in6
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


struct ip_mreqn
  {

    struct in_addr imr_multiaddr;


    struct in_addr imr_address;


    int imr_ifindex;
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
# 399 "/usr/include/netinet/in.h" 3 4
extern uint32_t ntohl (uint32_t __netlong) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
extern uint16_t ntohs (uint16_t __netshort)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
extern uint32_t htonl (uint32_t __hostlong)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
extern uint16_t htons (uint16_t __hostshort)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));




# 1 "/usr/include/bits/byteswap.h" 1 3 4
# 411 "/usr/include/netinet/in.h" 2 3 4
# 1 "/usr/include/bits/uintn-identity.h" 1 3 4
# 412 "/usr/include/netinet/in.h" 2 3 4
# 527 "/usr/include/netinet/in.h" 3 4
extern int bindresvport (int __sockfd, struct sockaddr_in *__sock_in) __attribute__ ((__nothrow__ , __leaf__));


extern int bindresvport6 (int __sockfd, struct sockaddr_in6 *__sock_in)
     __attribute__ ((__nothrow__ , __leaf__));
# 655 "/usr/include/netinet/in.h" 3 4

# 39 "/usr/include/nspr/prinet.h" 2
# 50 "/usr/include/nspr/prinet.h"
# 1 "/usr/include/arpa/inet.h" 1 3 4
# 30 "/usr/include/arpa/inet.h" 3 4




extern in_addr_t inet_addr (const char *__cp) __attribute__ ((__nothrow__ , __leaf__));


extern in_addr_t inet_lnaof (struct in_addr __in) __attribute__ ((__nothrow__ , __leaf__));



extern struct in_addr inet_makeaddr (in_addr_t __net, in_addr_t __host)
     __attribute__ ((__nothrow__ , __leaf__));


extern in_addr_t inet_netof (struct in_addr __in) __attribute__ ((__nothrow__ , __leaf__));



extern in_addr_t inet_network (const char *__cp) __attribute__ ((__nothrow__ , __leaf__));



extern char *inet_ntoa (struct in_addr __in) __attribute__ ((__nothrow__ , __leaf__));




extern int inet_pton (int __af, const char *__restrict __cp,
        void *__restrict __buf) __attribute__ ((__nothrow__ , __leaf__));




extern const char *inet_ntop (int __af, const void *__restrict __cp,
         char *__restrict __buf, socklen_t __len)
     __attribute__ ((__nothrow__ , __leaf__));






extern int inet_aton (const char *__cp, struct in_addr *__inp) __attribute__ ((__nothrow__ , __leaf__));



extern char *inet_neta (in_addr_t __net, char *__buf, size_t __len) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__deprecated__ ("Use inet_ntop instead")));




extern char *inet_net_ntop (int __af, const void *__cp, int __bits,
       char *__buf, size_t __len) __attribute__ ((__nothrow__ , __leaf__));




extern int inet_net_pton (int __af, const char *__cp,
     void *__buf, size_t __len) __attribute__ ((__nothrow__ , __leaf__));




extern unsigned int inet_nsap_addr (const char *__cp,
        unsigned char *__buf, int __len) __attribute__ ((__nothrow__ , __leaf__));



extern char *inet_nsap_ntoa (int __len, const unsigned char *__cp,
        char *__buf) __attribute__ ((__nothrow__ , __leaf__));



# 51 "/usr/include/nspr/prinet.h" 2

# 1 "/usr/include/netdb.h" 1 3 4
# 32 "/usr/include/netdb.h" 3 4
# 1 "/usr/include/rpc/netdb.h" 1 3 4
# 42 "/usr/include/rpc/netdb.h" 3 4
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 1 3 4
# 43 "/usr/include/rpc/netdb.h" 2 3 4



struct rpcent
{
  char *r_name;
  char **r_aliases;
  int r_number;
};

extern void setrpcent (int __stayopen) __attribute__ ((__nothrow__ , __leaf__));
extern void endrpcent (void) __attribute__ ((__nothrow__ , __leaf__));
extern struct rpcent *getrpcbyname (const char *__name) __attribute__ ((__nothrow__ , __leaf__));
extern struct rpcent *getrpcbynumber (int __number) __attribute__ ((__nothrow__ , __leaf__));
extern struct rpcent *getrpcent (void) __attribute__ ((__nothrow__ , __leaf__));


extern int getrpcbyname_r (const char *__name, struct rpcent *__result_buf,
      char *__buffer, size_t __buflen,
      struct rpcent **__result) __attribute__ ((__nothrow__ , __leaf__));

extern int getrpcbynumber_r (int __number, struct rpcent *__result_buf,
        char *__buffer, size_t __buflen,
        struct rpcent **__result) __attribute__ ((__nothrow__ , __leaf__));

extern int getrpcent_r (struct rpcent *__result_buf, char *__buffer,
   size_t __buflen, struct rpcent **__result) __attribute__ ((__nothrow__ , __leaf__));



# 33 "/usr/include/netdb.h" 2 3 4







# 1 "/usr/include/bits/netdb.h" 1 3 4
# 26 "/usr/include/bits/netdb.h" 3 4
struct netent
{
  char *n_name;
  char **n_aliases;
  int n_addrtype;
  uint32_t n_net;
};
# 41 "/usr/include/netdb.h" 2 3 4
# 51 "/usr/include/netdb.h" 3 4








extern int *__h_errno_location (void) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
# 90 "/usr/include/netdb.h" 3 4
extern void herror (const char *__str) __attribute__ ((__nothrow__ , __leaf__));


extern const char *hstrerror (int __err_num) __attribute__ ((__nothrow__ , __leaf__));




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






extern struct hostent *gethostbyaddr (const void *__addr, __socklen_t __len,
          int __type);





extern struct hostent *gethostbyname (const char *__name);
# 153 "/usr/include/netdb.h" 3 4
extern struct hostent *gethostbyname2 (const char *__name, int __af);
# 165 "/usr/include/netdb.h" 3 4
extern int gethostent_r (struct hostent *__restrict __result_buf,
    char *__restrict __buf, size_t __buflen,
    struct hostent **__restrict __result,
    int *__restrict __h_errnop);

extern int gethostbyaddr_r (const void *__restrict __addr, __socklen_t __len,
       int __type,
       struct hostent *__restrict __result_buf,
       char *__restrict __buf, size_t __buflen,
       struct hostent **__restrict __result,
       int *__restrict __h_errnop);

extern int gethostbyname_r (const char *__restrict __name,
       struct hostent *__restrict __result_buf,
       char *__restrict __buf, size_t __buflen,
       struct hostent **__restrict __result,
       int *__restrict __h_errnop);

extern int gethostbyname2_r (const char *__restrict __name, int __af,
        struct hostent *__restrict __result_buf,
        char *__restrict __buf, size_t __buflen,
        struct hostent **__restrict __result,
        int *__restrict __h_errnop);
# 196 "/usr/include/netdb.h" 3 4
extern void setnetent (int __stay_open);





extern void endnetent (void);






extern struct netent *getnetent (void);






extern struct netent *getnetbyaddr (uint32_t __net, int __type);





extern struct netent *getnetbyname (const char *__name);
# 235 "/usr/include/netdb.h" 3 4
extern int getnetent_r (struct netent *__restrict __result_buf,
   char *__restrict __buf, size_t __buflen,
   struct netent **__restrict __result,
   int *__restrict __h_errnop);

extern int getnetbyaddr_r (uint32_t __net, int __type,
      struct netent *__restrict __result_buf,
      char *__restrict __buf, size_t __buflen,
      struct netent **__restrict __result,
      int *__restrict __h_errnop);

extern int getnetbyname_r (const char *__restrict __name,
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






extern struct servent *getservbyname (const char *__name, const char *__proto);






extern struct servent *getservbyport (int __port, const char *__proto);
# 306 "/usr/include/netdb.h" 3 4
extern int getservent_r (struct servent *__restrict __result_buf,
    char *__restrict __buf, size_t __buflen,
    struct servent **__restrict __result);

extern int getservbyname_r (const char *__restrict __name,
       const char *__restrict __proto,
       struct servent *__restrict __result_buf,
       char *__restrict __buf, size_t __buflen,
       struct servent **__restrict __result);

extern int getservbyport_r (int __port, const char *__restrict __proto,
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





extern struct protoent *getprotobyname (const char *__name);





extern struct protoent *getprotobynumber (int __proto);
# 372 "/usr/include/netdb.h" 3 4
extern int getprotoent_r (struct protoent *__restrict __result_buf,
     char *__restrict __buf, size_t __buflen,
     struct protoent **__restrict __result);

extern int getprotobyname_r (const char *__restrict __name,
        struct protoent *__restrict __result_buf,
        char *__restrict __buf, size_t __buflen,
        struct protoent **__restrict __result);

extern int getprotobynumber_r (int __proto,
          struct protoent *__restrict __result_buf,
          char *__restrict __buf, size_t __buflen,
          struct protoent **__restrict __result);
# 393 "/usr/include/netdb.h" 3 4
extern int setnetgrent (const char *__netgroup);







extern void endnetgrent (void);
# 410 "/usr/include/netdb.h" 3 4
extern int getnetgrent (char **__restrict __hostp,
   char **__restrict __userp,
   char **__restrict __domainp);
# 421 "/usr/include/netdb.h" 3 4
extern int innetgr (const char *__netgroup, const char *__host,
      const char *__user, const char *__domain);







extern int getnetgrent_r (char **__restrict __hostp,
     char **__restrict __userp,
     char **__restrict __domainp,
     char *__restrict __buffer, size_t __buflen);
# 449 "/usr/include/netdb.h" 3 4
extern int rcmd (char **__restrict __ahost, unsigned short int __rport,
   const char *__restrict __locuser,
   const char *__restrict __remuser,
   const char *__restrict __cmd, int *__restrict __fd2p);
# 461 "/usr/include/netdb.h" 3 4
extern int rcmd_af (char **__restrict __ahost, unsigned short int __rport,
      const char *__restrict __locuser,
      const char *__restrict __remuser,
      const char *__restrict __cmd, int *__restrict __fd2p,
      sa_family_t __af);
# 477 "/usr/include/netdb.h" 3 4
extern int rexec (char **__restrict __ahost, int __rport,
    const char *__restrict __name,
    const char *__restrict __pass,
    const char *__restrict __cmd, int *__restrict __fd2p);
# 489 "/usr/include/netdb.h" 3 4
extern int rexec_af (char **__restrict __ahost, int __rport,
       const char *__restrict __name,
       const char *__restrict __pass,
       const char *__restrict __cmd, int *__restrict __fd2p,
       sa_family_t __af);
# 503 "/usr/include/netdb.h" 3 4
extern int ruserok (const char *__rhost, int __suser,
      const char *__remuser, const char *__locuser);
# 513 "/usr/include/netdb.h" 3 4
extern int ruserok_af (const char *__rhost, int __suser,
         const char *__remuser, const char *__locuser,
         sa_family_t __af);
# 526 "/usr/include/netdb.h" 3 4
extern int iruserok (uint32_t __raddr, int __suser,
       const char *__remuser, const char *__locuser);
# 537 "/usr/include/netdb.h" 3 4
extern int iruserok_af (const void *__raddr, int __suser,
   const char *__remuser, const char *__locuser,
   sa_family_t __af);
# 549 "/usr/include/netdb.h" 3 4
extern int rresvport (int *__alport);
# 558 "/usr/include/netdb.h" 3 4
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
# 660 "/usr/include/netdb.h" 3 4
extern int getaddrinfo (const char *__restrict __name,
   const char *__restrict __service,
   const struct addrinfo *__restrict __req,
   struct addrinfo **__restrict __pai);


extern void freeaddrinfo (struct addrinfo *__ai) __attribute__ ((__nothrow__ , __leaf__));


extern const char *gai_strerror (int __ecode) __attribute__ ((__nothrow__ , __leaf__));





extern int getnameinfo (const struct sockaddr *__restrict __sa,
   socklen_t __salen, char *__restrict __host,
   socklen_t __hostlen, char *__restrict __serv,
   socklen_t __servlen, int __flags);
# 722 "/usr/include/netdb.h" 3 4

# 53 "/usr/include/nspr/prinet.h" 2
# 18 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prinit.h" 1
# 9 "/usr/include/nspr/prinit.h"
# 1 "/usr/include/nspr/prthread.h" 1
# 49 "/usr/include/nspr/prthread.h"



# 51 "/usr/include/nspr/prthread.h"
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
# 105 "/usr/include/nspr/prthread.h"
extern __attribute__((visibility("default"))) PRThread* PR_CreateThread(PRThreadType type,
                                    void ( *start)(void *arg),
                                    void *arg,
                                    PRThreadPriority priority,
                                    PRThreadScope scope,
                                    PRThreadState state,
                                    PRUint32 stackSize);
# 126 "/usr/include/nspr/prthread.h"
extern __attribute__((visibility("default"))) PRStatus PR_JoinThread(PRThread *thread);





extern __attribute__((visibility("default"))) PRThread* PR_GetCurrentThread(void);







extern __attribute__((visibility("default"))) PRThreadPriority PR_GetThreadPriority(const PRThread *thread);
# 152 "/usr/include/nspr/prthread.h"
extern __attribute__((visibility("default"))) void PR_SetThreadPriority(PRThread *thread, PRThreadPriority priority);





extern __attribute__((visibility("default"))) PRStatus PR_SetCurrentThreadName(const char *name);




extern __attribute__((visibility("default"))) const char * PR_GetThreadName(const PRThread *thread);
# 189 "/usr/include/nspr/prthread.h"
typedef void ( *PRThreadPrivateDTOR)(void *priv);

extern __attribute__((visibility("default"))) PRStatus PR_NewThreadPrivateIndex(
    PRUintn *newIndex, PRThreadPrivateDTOR destructor);
# 205 "/usr/include/nspr/prthread.h"
extern __attribute__((visibility("default"))) PRStatus PR_SetThreadPrivate(PRUintn tpdIndex, void *priv);
# 216 "/usr/include/nspr/prthread.h"
extern __attribute__((visibility("default"))) void* PR_GetThreadPrivate(PRUintn tpdIndex);
# 228 "/usr/include/nspr/prthread.h"
extern __attribute__((visibility("default"))) PRStatus PR_Interrupt(PRThread *thread);





extern __attribute__((visibility("default"))) void PR_ClearInterrupt(void);




extern __attribute__((visibility("default"))) void PR_BlockInterrupt(void);




extern __attribute__((visibility("default"))) void PR_UnblockInterrupt(void);
# 253 "/usr/include/nspr/prthread.h"
extern __attribute__((visibility("default"))) PRStatus PR_Sleep(PRIntervalTime ticks);




extern __attribute__((visibility("default"))) PRThreadScope PR_GetThreadScope(const PRThread *thread);




extern __attribute__((visibility("default"))) PRThreadType PR_GetThreadType(const PRThread *thread);




extern __attribute__((visibility("default"))) PRThreadState PR_GetThreadState(const PRThread *thread);


# 10 "/usr/include/nspr/prinit.h" 2

# 1 "/usr/include/nspr/prwin.h" 1
# 12 "/usr/include/nspr/prinit.h" 2
# 1 "/usr/include/stdio.h" 1 3 4
# 28 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/bits/libc-header-start.h" 1 3 4
# 29 "/usr/include/stdio.h" 2 3 4





# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 1 3 4
# 35 "/usr/include/stdio.h" 2 3 4


# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stdarg.h" 1 3 4
# 40 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stdarg.h" 3 4

# 40 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stdarg.h" 3 4
typedef __builtin_va_list __gnuc_va_list;
# 38 "/usr/include/stdio.h" 2 3 4


# 1 "/usr/include/bits/types/__fpos_t.h" 1 3 4




# 1 "/usr/include/bits/types/__mbstate_t.h" 1 3 4
# 13 "/usr/include/bits/types/__mbstate_t.h" 3 4
typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;
# 6 "/usr/include/bits/types/__fpos_t.h" 2 3 4




typedef struct _G_fpos_t
{
  __off_t __pos;
  __mbstate_t __state;
} __fpos_t;
# 41 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/bits/types/__fpos64_t.h" 1 3 4
# 10 "/usr/include/bits/types/__fpos64_t.h" 3 4
typedef struct _G_fpos64_t
{
  __off64_t __pos;
  __mbstate_t __state;
} __fpos64_t;
# 42 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/bits/types/__FILE.h" 1 3 4



struct _IO_FILE;
typedef struct _IO_FILE __FILE;
# 43 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/bits/types/FILE.h" 1 3 4



struct _IO_FILE;


typedef struct _IO_FILE FILE;
# 44 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/bits/types/struct_FILE.h" 1 3 4
# 35 "/usr/include/bits/types/struct_FILE.h" 3 4
struct _IO_FILE;
struct _IO_marker;
struct _IO_codecvt;
struct _IO_wide_data;




typedef void _IO_lock_t;





struct _IO_FILE
{
  int _flags;


  char *_IO_read_ptr;
  char *_IO_read_end;
  char *_IO_read_base;
  char *_IO_write_base;
  char *_IO_write_ptr;
  char *_IO_write_end;
  char *_IO_buf_base;
  char *_IO_buf_end;


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







  __off64_t _offset;

  struct _IO_codecvt *_codecvt;
  struct _IO_wide_data *_wide_data;
  struct _IO_FILE *_freeres_list;
  void *_freeres_buf;
  struct _IO_FILE **_prevchain;
  int _mode;

  char _unused2[15 * sizeof (int) - 5 * sizeof (void *)];
};
# 45 "/usr/include/stdio.h" 2 3 4


# 1 "/usr/include/bits/types/cookie_io_functions_t.h" 1 3 4
# 27 "/usr/include/bits/types/cookie_io_functions_t.h" 3 4
typedef __ssize_t cookie_read_function_t (void *__cookie, char *__buf,
                                          size_t __nbytes);







typedef __ssize_t cookie_write_function_t (void *__cookie, const char *__buf,
                                           size_t __nbytes);







typedef int cookie_seek_function_t (void *__cookie, __off64_t *__pos, int __w);


typedef int cookie_close_function_t (void *__cookie);






typedef struct _IO_cookie_io_functions_t
{
  cookie_read_function_t *read;
  cookie_write_function_t *write;
  cookie_seek_function_t *seek;
  cookie_close_function_t *close;
} cookie_io_functions_t;
# 48 "/usr/include/stdio.h" 2 3 4





typedef __gnuc_va_list va_list;
# 85 "/usr/include/stdio.h" 3 4
typedef __fpos_t fpos_t;
# 129 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/bits/stdio_lim.h" 1 3 4
# 130 "/usr/include/stdio.h" 2 3 4
# 149 "/usr/include/stdio.h" 3 4
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;






extern int remove (const char *__filename) __attribute__ ((__nothrow__ , __leaf__));

extern int rename (const char *__old, const char *__new) __attribute__ ((__nothrow__ , __leaf__));



extern int renameat (int __oldfd, const char *__old, int __newfd,
       const char *__new) __attribute__ ((__nothrow__ , __leaf__));
# 184 "/usr/include/stdio.h" 3 4
extern int fclose (FILE *__stream) __attribute__ ((__nonnull__ (1)));
# 194 "/usr/include/stdio.h" 3 4
extern FILE *tmpfile (void)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__));
# 211 "/usr/include/stdio.h" 3 4
extern char *tmpnam (char[20]) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__));




extern char *tmpnam_r (char __s[20]) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__));
# 228 "/usr/include/stdio.h" 3 4
extern char *tempnam (const char *__dir, const char *__pfx)
   __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__malloc__));






extern int fflush (FILE *__stream);
# 245 "/usr/include/stdio.h" 3 4
extern int fflush_unlocked (FILE *__stream);
# 264 "/usr/include/stdio.h" 3 4
extern FILE *fopen (const char *__restrict __filename,
      const char *__restrict __modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__));




extern FILE *freopen (const char *__restrict __filename,
        const char *__restrict __modes,
        FILE *__restrict __stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (3)));
# 299 "/usr/include/stdio.h" 3 4
extern FILE *fdopen (int __fd, const char *__modes) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__));





extern FILE *fopencookie (void *__restrict __magic_cookie,
     const char *__restrict __modes,
     cookie_io_functions_t __io_funcs) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__));




extern FILE *fmemopen (void *__s, size_t __len, const char *__modes)
  __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__));




extern FILE *open_memstream (char **__bufloc, size_t *__sizeloc) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__));
# 334 "/usr/include/stdio.h" 3 4
extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__nonnull__ (1)));



extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
      int __modes, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));




extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
         size_t __size) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));


extern void setlinebuf (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));







extern int fprintf (FILE *__restrict __stream,
      const char *__restrict __format, ...) __attribute__ ((__nonnull__ (1)));




extern int printf (const char *__restrict __format, ...);

extern int sprintf (char *__restrict __s,
      const char *__restrict __format, ...) __attribute__ ((__nothrow__));





extern int vfprintf (FILE *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nonnull__ (1)));




extern int vprintf (const char *__restrict __format, __gnuc_va_list __arg);

extern int vsprintf (char *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nothrow__));



extern int snprintf (char *__restrict __s, size_t __maxlen,
       const char *__restrict __format, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 4)));

extern int vsnprintf (char *__restrict __s, size_t __maxlen,
        const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 0)));





extern int vasprintf (char **__restrict __ptr, const char *__restrict __f,
        __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 2, 0))) __attribute__ ((__warn_unused_result__));
extern int __asprintf (char **__restrict __ptr,
         const char *__restrict __fmt, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 2, 3))) __attribute__ ((__warn_unused_result__));
extern int asprintf (char **__restrict __ptr,
       const char *__restrict __fmt, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 2, 3))) __attribute__ ((__warn_unused_result__));




extern int vdprintf (int __fd, const char *__restrict __fmt,
       __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 2, 0)));
extern int dprintf (int __fd, const char *__restrict __fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));







extern int fscanf (FILE *__restrict __stream,
     const char *__restrict __format, ...) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));




extern int scanf (const char *__restrict __format, ...) __attribute__ ((__warn_unused_result__));

extern int sscanf (const char *__restrict __s,
     const char *__restrict __format, ...) __attribute__ ((__nothrow__ , __leaf__));





# 1 "/usr/include/bits/floatn.h" 1 3 4
# 119 "/usr/include/bits/floatn.h" 3 4
# 1 "/usr/include/bits/floatn-common.h" 1 3 4
# 24 "/usr/include/bits/floatn-common.h" 3 4
# 1 "/usr/include/bits/long-double.h" 1 3 4
# 25 "/usr/include/bits/floatn-common.h" 2 3 4
# 120 "/usr/include/bits/floatn.h" 2 3 4
# 438 "/usr/include/stdio.h" 2 3 4
# 463 "/usr/include/stdio.h" 3 4
extern int fscanf (FILE *__restrict __stream, const char *__restrict __format, ...) __asm__ ("" "__isoc99_fscanf")

                          __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
extern int scanf (const char *__restrict __format, ...) __asm__ ("" "__isoc99_scanf")
                         __attribute__ ((__warn_unused_result__));
extern int sscanf (const char *__restrict __s, const char *__restrict __format, ...) __asm__ ("" "__isoc99_sscanf") __attribute__ ((__nothrow__ , __leaf__))

                      ;
# 490 "/usr/include/stdio.h" 3 4
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format,
      __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));





extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 1, 0))) __attribute__ ((__warn_unused_result__));


extern int vsscanf (const char *__restrict __s,
      const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__format__ (__scanf__, 2, 0)));
# 540 "/usr/include/stdio.h" 3 4
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vfscanf")



     __attribute__ ((__format__ (__scanf__, 2, 0))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vscanf")

     __attribute__ ((__format__ (__scanf__, 1, 0))) __attribute__ ((__warn_unused_result__));
extern int vsscanf (const char *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vsscanf") __attribute__ ((__nothrow__ , __leaf__))



     __attribute__ ((__format__ (__scanf__, 2, 0)));
# 575 "/usr/include/stdio.h" 3 4
extern int fgetc (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int getc (FILE *__stream) __attribute__ ((__nonnull__ (1)));





extern int getchar (void);






extern int getc_unlocked (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int getchar_unlocked (void);
# 600 "/usr/include/stdio.h" 3 4
extern int fgetc_unlocked (FILE *__stream) __attribute__ ((__nonnull__ (1)));
# 611 "/usr/include/stdio.h" 3 4
extern int fputc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern int putc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));





extern int putchar (int __c);
# 627 "/usr/include/stdio.h" 3 4
extern int fputc_unlocked (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));







extern int putc_unlocked (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern int putchar_unlocked (int __c);






extern int getw (FILE *__stream) __attribute__ ((__nonnull__ (1)));


extern int putw (int __w, FILE *__stream) __attribute__ ((__nonnull__ (2)));







extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
     __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 1, 2))) __attribute__ ((__nonnull__ (3)));
# 667 "/usr/include/stdio.h" 3 4
extern char *gets (char *__s) __attribute__ ((__warn_unused_result__)) __attribute__ ((__deprecated__));
# 689 "/usr/include/stdio.h" 3 4
extern __ssize_t __getdelim (char **__restrict __lineptr,
                             size_t *__restrict __n, int __delimiter,
                             FILE *__restrict __stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (4)));
extern __ssize_t getdelim (char **__restrict __lineptr,
                           size_t *__restrict __n, int __delimiter,
                           FILE *__restrict __stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (4)));


extern __ssize_t getline (char **__restrict __lineptr,
                          size_t *__restrict __n,
                          FILE *__restrict __stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (3)));







extern int fputs (const char *__restrict __s, FILE *__restrict __stream)
  __attribute__ ((__nonnull__ (2)));





extern int puts (const char *__s);






extern int ungetc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));






extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream) __attribute__ ((__warn_unused_result__))
  __attribute__ ((__nonnull__ (4)));




extern size_t fwrite (const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s) __attribute__ ((__nonnull__ (4)));
# 756 "/usr/include/stdio.h" 3 4
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
         size_t __n, FILE *__restrict __stream) __attribute__ ((__warn_unused_result__))
  __attribute__ ((__nonnull__ (4)));
extern size_t fwrite_unlocked (const void *__restrict __ptr, size_t __size,
          size_t __n, FILE *__restrict __stream)
  __attribute__ ((__nonnull__ (4)));







extern int fseek (FILE *__stream, long int __off, int __whence)
  __attribute__ ((__nonnull__ (1)));




extern long int ftell (FILE *__stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));




extern void rewind (FILE *__stream) __attribute__ ((__nonnull__ (1)));
# 793 "/usr/include/stdio.h" 3 4
extern int fseeko (FILE *__stream, __off_t __off, int __whence)
  __attribute__ ((__nonnull__ (1)));




extern __off_t ftello (FILE *__stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
# 819 "/usr/include/stdio.h" 3 4
extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos)
  __attribute__ ((__nonnull__ (1)));




extern int fsetpos (FILE *__stream, const fpos_t *__pos) __attribute__ ((__nonnull__ (1)));
# 850 "/usr/include/stdio.h" 3 4
extern void clearerr (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));

extern int feof (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));

extern int ferror (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));



extern void clearerr_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
extern int feof_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
extern int ferror_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));







extern void perror (const char *__s) __attribute__ ((__cold__));




extern int fileno (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));




extern int fileno_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
# 887 "/usr/include/stdio.h" 3 4
extern int pclose (FILE *__stream) __attribute__ ((__nonnull__ (1)));





extern FILE *popen (const char *__command, const char *__modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__));






extern char *ctermid (char *__s) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__access__ (__write_only__, 1)));
# 931 "/usr/include/stdio.h" 3 4
extern void flockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));



extern int ftrylockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));


extern void funlockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
# 949 "/usr/include/stdio.h" 3 4
extern int __uflow (FILE *);
extern int __overflow (FILE *, int);



# 1 "/usr/include/bits/stdio2-decl.h" 1 3 4
# 26 "/usr/include/bits/stdio2-decl.h" 3 4
extern int __sprintf_chk (char *__restrict __s, int __flag, size_t __slen,
     const char *__restrict __format, ...) __attribute__ ((__nothrow__ , __leaf__))
    __attribute__ ((__access__ (__write_only__, 1, 3)));
extern int __vsprintf_chk (char *__restrict __s, int __flag, size_t __slen,
      const char *__restrict __format,
      __gnuc_va_list __ap) __attribute__ ((__nothrow__ , __leaf__))
    __attribute__ ((__access__ (__write_only__, 1, 3)));



extern int __snprintf_chk (char *__restrict __s, size_t __n, int __flag,
      size_t __slen, const char *__restrict __format,
      ...) __attribute__ ((__nothrow__ , __leaf__))
    __attribute__ ((__access__ (__write_only__, 1, 2)));
extern int __vsnprintf_chk (char *__restrict __s, size_t __n, int __flag,
       size_t __slen, const char *__restrict __format,
       __gnuc_va_list __ap) __attribute__ ((__nothrow__ , __leaf__))
    __attribute__ ((__access__ (__write_only__, 1, 2)));





extern int __fprintf_chk (FILE *__restrict __stream, int __flag,
     const char *__restrict __format, ...)
    __attribute__ ((__nonnull__ (1)));
extern int __printf_chk (int __flag, const char *__restrict __format, ...);
extern int __vfprintf_chk (FILE *__restrict __stream, int __flag,
      const char *__restrict __format,
      __gnuc_va_list __ap) __attribute__ ((__nonnull__ (1)));
extern int __vprintf_chk (int __flag, const char *__restrict __format,
     __gnuc_va_list __ap);


extern int __dprintf_chk (int __fd, int __flag, const char *__restrict __fmt,
     ...) __attribute__ ((__format__ (__printf__, 3, 4)));
extern int __vdprintf_chk (int __fd, int __flag,
      const char *__restrict __fmt, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 3, 0)));
# 89 "/usr/include/bits/stdio2-decl.h" 3 4
extern char *__gets_warn (char *__str) __asm__ ("" "gets")
     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("please use fgets or getline instead, gets can't " "specify buffer size")))
                               ;

extern char *__gets_chk (char *__str, size_t) __attribute__ ((__warn_unused_result__));


extern char *__fgets_alias (char *__restrict __s, int __n, FILE *__restrict __stream) __asm__ ("" "fgets")


    __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 1, 2)));
extern char *__fgets_chk_warn (char *__restrict __s, size_t __size, int __n, FILE *__restrict __stream) __asm__ ("" "__fgets_chk")


     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("fgets called with bigger size than length " "of destination buffer")))
                                 ;

extern char *__fgets_chk (char *__restrict __s, size_t __size, int __n,
     FILE *__restrict __stream)
    __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 1, 3))) __attribute__ ((__nonnull__ (4)));

extern size_t __fread_alias (void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream) __asm__ ("" "fread")


            __attribute__ ((__warn_unused_result__));
extern size_t __fread_chk_warn (void *__restrict __ptr, size_t __ptrlen, size_t __size, size_t __n, FILE *__restrict __stream) __asm__ ("" "__fread_chk")




     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("fread called with bigger size * nmemb than length " "of destination buffer")))
                                 ;

extern size_t __fread_chk (void *__restrict __ptr, size_t __ptrlen,
      size_t __size, size_t __n,
      FILE *__restrict __stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (5)));
# 145 "/usr/include/bits/stdio2-decl.h" 3 4
extern size_t __fread_unlocked_alias (void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream) __asm__ ("" "fread_unlocked")


                     __attribute__ ((__warn_unused_result__));
extern size_t __fread_unlocked_chk_warn (void *__restrict __ptr, size_t __ptrlen, size_t __size, size_t __n, FILE *__restrict __stream) __asm__ ("" "__fread_unlocked_chk")




     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("fread_unlocked called with bigger size * nmemb than " "length of destination buffer")))
                                        ;

extern size_t __fread_unlocked_chk (void *__restrict __ptr, size_t __ptrlen,
        size_t __size, size_t __n,
        FILE *__restrict __stream)
    __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (5)));
# 955 "/usr/include/stdio.h" 2 3 4
# 966 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/bits/stdio.h" 1 3 4
# 46 "/usr/include/bits/stdio.h" 3 4
extern __inline __attribute__ ((__gnu_inline__)) int
getchar (void)
{
  return getc (stdin);
}




extern __inline __attribute__ ((__gnu_inline__)) int
fgetc_unlocked (FILE *__fp)
{
  return (__builtin_expect (((__fp)->_IO_read_ptr >= (__fp)->_IO_read_end), 0) ? __uflow (__fp) : *(unsigned char *) (__fp)->_IO_read_ptr++);
}





extern __inline __attribute__ ((__gnu_inline__)) int
getc_unlocked (FILE *__fp)
{
  return (__builtin_expect (((__fp)->_IO_read_ptr >= (__fp)->_IO_read_end), 0) ? __uflow (__fp) : *(unsigned char *) (__fp)->_IO_read_ptr++);
}


extern __inline __attribute__ ((__gnu_inline__)) int
getchar_unlocked (void)
{
  return (__builtin_expect (((stdin)->_IO_read_ptr >= (stdin)->_IO_read_end), 0) ? __uflow (stdin) : *(unsigned char *) (stdin)->_IO_read_ptr++);
}




extern __inline __attribute__ ((__gnu_inline__)) int
putchar (int __c)
{
  return putc (__c, stdout);
}




extern __inline __attribute__ ((__gnu_inline__)) int
fputc_unlocked (int __c, FILE *__stream)
{
  return (__builtin_expect (((__stream)->_IO_write_ptr >= (__stream)->_IO_write_end), 0) ? __overflow (__stream, (unsigned char) (__c)) : (unsigned char) (*(__stream)->_IO_write_ptr++ = (__c)));
}





extern __inline __attribute__ ((__gnu_inline__)) int
putc_unlocked (int __c, FILE *__stream)
{
  return (__builtin_expect (((__stream)->_IO_write_ptr >= (__stream)->_IO_write_end), 0) ? __overflow (__stream, (unsigned char) (__c)) : (unsigned char) (*(__stream)->_IO_write_ptr++ = (__c)));
}


extern __inline __attribute__ ((__gnu_inline__)) int
putchar_unlocked (int __c)
{
  return (__builtin_expect (((stdout)->_IO_write_ptr >= (stdout)->_IO_write_end), 0) ? __overflow (stdout, (unsigned char) (__c)) : (unsigned char) (*(stdout)->_IO_write_ptr++ = (__c)));
}
# 127 "/usr/include/bits/stdio.h" 3 4
extern __inline __attribute__ ((__gnu_inline__)) int
__attribute__ ((__nothrow__ , __leaf__)) feof_unlocked (FILE *__stream)
{
  return (((__stream)->_flags & 0x0010) != 0);
}


extern __inline __attribute__ ((__gnu_inline__)) int
__attribute__ ((__nothrow__ , __leaf__)) ferror_unlocked (FILE *__stream)
{
  return (((__stream)->_flags & 0x0020) != 0);
}
# 967 "/usr/include/stdio.h" 2 3 4



# 1 "/usr/include/bits/stdio2.h" 1 3 4
# 27 "/usr/include/bits/stdio2.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__ , __leaf__)) sprintf (char *__restrict __s, const char *__restrict __fmt, ...)
{
  return __builtin___sprintf_chk (__s, 2 - 1,
      __builtin_object_size (__s, 2 > 1), __fmt,
      __builtin_va_arg_pack ());
}
# 54 "/usr/include/bits/stdio2.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__ , __leaf__)) vsprintf (char * __restrict __s, const char *__restrict __fmt, __gnuc_va_list __ap)

{
  return __builtin___vsprintf_chk (__s, 2 - 1,
       __builtin_object_size (__s, 2 > 1), __fmt, __ap);
}



extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__ , __leaf__)) snprintf (char *__restrict __s, size_t __n, const char *__restrict __fmt, ...)

{
  return __builtin___snprintf_chk (__s, __n, 2 - 1,
       __builtin_object_size (__s, 2 > 1), __fmt,
       __builtin_va_arg_pack ());
}
# 92 "/usr/include/bits/stdio2.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__ , __leaf__)) vsnprintf (char * __restrict __s, size_t __n, const char *__restrict __fmt, __gnuc_va_list __ap)


    


{
  return __builtin___vsnprintf_chk (__s, __n, 2 - 1,
        __builtin_object_size (__s, 2 > 1), __fmt, __ap);
}





extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__nonnull__ (1))) int
fprintf (FILE *__restrict __stream, const char *__restrict __fmt, ...)
{
  return __fprintf_chk (__stream, 2 - 1, __fmt,
   __builtin_va_arg_pack ());
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
printf (const char *__restrict __fmt, ...)
{
  return __printf_chk (2 - 1, __fmt, __builtin_va_arg_pack ());
}
# 151 "/usr/include/bits/stdio2.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
vprintf (const char * __restrict __fmt,
  __gnuc_va_list __ap)
{

  return __vfprintf_chk (stdout, 2 - 1, __fmt, __ap);



}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__nonnull__ (1))) int
vfprintf (FILE *__restrict __stream,
   const char *__restrict __fmt, __gnuc_va_list __ap)
{
  return __vfprintf_chk (__stream, 2 - 1, __fmt, __ap);
}



extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
dprintf (int __fd, const char *__restrict __fmt, ...)
{
  return __dprintf_chk (__fd, 2 - 1, __fmt,
   __builtin_va_arg_pack ());
}
# 194 "/usr/include/bits/stdio2.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
vdprintf (int __fd, const char *__restrict __fmt, __gnuc_va_list __ap)
{
  return __vdprintf_chk (__fd, 2 - 1, __fmt, __ap);
}
# 291 "/usr/include/bits/stdio2.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) char *
gets (char * __str)
    


{
  if (__builtin_object_size (__str, 2 > 1) != (size_t) -1)
    return __gets_chk (__str, __builtin_object_size (__str, 2 > 1));
  return __gets_warn (__str);
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 1, 2)))
__attribute__ ((__nonnull__ (3))) char *
fgets (char * __restrict __s, int __n,
       FILE *__restrict __stream)
    


{
  size_t __sz = __builtin_object_size (__s, 2 > 1);
  if (((__builtin_constant_p (__sz) && (__sz) == (long unsigned int) -1) || (((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= ((__sz)) / ((sizeof (char))))) && (((long unsigned int) (__n)) <= ((__sz)) / ((sizeof (char)))))))
    return __fgets_alias (__s, __n, __stream);

  if ((((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= (__sz) / (sizeof (char)))) && !(((long unsigned int) (__n)) <= (__sz) / (sizeof (char)))))
    return __fgets_chk_warn (__s, __sz, __n, __stream);

  return __fgets_chk (__s, __sz, __n, __stream);
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (4))) size_t
fread (void * __restrict __ptr,
       size_t __size, size_t __n, FILE *__restrict __stream)
    



{
  size_t __sz = __builtin_object_size (__ptr, 0);
  if (((__builtin_constant_p (__sz) && (__sz) == (long unsigned int) -1) || (((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= ((__sz)) / ((__size)))) && (((long unsigned int) (__n)) <= ((__sz)) / ((__size))))))
    return __fread_alias (__ptr, __size, __n, __stream);

  if ((((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= (__sz) / (__size))) && !(((long unsigned int) (__n)) <= (__sz) / (__size))))
    return __fread_chk_warn (__ptr, __sz, __size, __n, __stream);

  return __fread_chk (__ptr, __sz, __size, __n, __stream);
}
# 361 "/usr/include/bits/stdio2.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (4))) size_t
fread_unlocked (void * __restrict __ptr,
  size_t __size, size_t __n, FILE *__restrict __stream)
    



{
  size_t __sz = __builtin_object_size (__ptr, 0);
  if (((__builtin_constant_p (__sz) && (__sz) == (long unsigned int) -1) || (((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= ((__sz)) / ((__size)))) && (((long unsigned int) (__n)) <= ((__sz)) / ((__size))))))
    {

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
       int __c = getc_unlocked (__stream);
       if (__c == (-1))
  break;
       *__cptr++ = __c;
     }
   return (__cptr - (char *) __ptr) / __size;
 }

      return __fread_unlocked_alias (__ptr, __size, __n, __stream);
    }

  if ((((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= (__sz) / (__size))) && !(((long unsigned int) (__n)) <= (__sz) / (__size))))
    return __fread_unlocked_chk_warn (__ptr, __sz, __size, __n, __stream);

  return __fread_unlocked_chk (__ptr, __sz, __size, __n, __stream);

}
# 971 "/usr/include/stdio.h" 2 3 4



# 13 "/usr/include/nspr/prinit.h" 2


# 56 "/usr/include/nspr/prinit.h"

# 56 "/usr/include/nspr/prinit.h"
typedef PRBool (*PRVersionCheck)(const char*);
# 66 "/usr/include/nspr/prinit.h"
extern __attribute__((visibility("default"))) PRBool PR_VersionCheck(const char *importedVersion);




extern __attribute__((visibility("default"))) const char* PR_GetVersion(void);
# 84 "/usr/include/nspr/prinit.h"
extern __attribute__((visibility("default"))) void PR_Init(
    PRThreadType type, PRThreadPriority priority, PRUintn maxPTDs);
# 105 "/usr/include/nspr/prinit.h"
typedef PRIntn ( *PRPrimordialFn)(PRIntn argc, char **argv);

extern __attribute__((visibility("default"))) PRIntn PR_Initialize(
    PRPrimordialFn prmain, PRIntn argc, char **argv, PRUintn maxPTDs);




extern __attribute__((visibility("default"))) PRBool PR_Initialized(void);
# 131 "/usr/include/nspr/prinit.h"
extern __attribute__((visibility("default"))) PRStatus PR_Cleanup(void);





extern __attribute__((visibility("default"))) void PR_DisableClockInterrupts(void);





extern __attribute__((visibility("default"))) void PR_EnableClockInterrupts(void);





extern __attribute__((visibility("default"))) void PR_BlockClockInterrupts(void);





extern __attribute__((visibility("default"))) void PR_UnblockClockInterrupts(void);




extern __attribute__((visibility("default"))) void PR_SetConcurrency(PRUintn numCPUs);






extern __attribute__((visibility("default"))) PRStatus PR_SetFDCacheSize(PRIntn low, PRIntn high);






extern __attribute__((visibility("default"))) void PR_ProcessExit(PRIntn status);






extern __attribute__((visibility("default"))) void PR_Abort(void);
# 191 "/usr/include/nspr/prinit.h"
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



# 19 "/usr/include/nspr/nspr.h" 2

# 1 "/usr/include/nspr/prio.h" 1
# 16 "/usr/include/nspr/prio.h"
# 1 "/usr/include/nspr/prlong.h" 1
# 19 "/usr/include/nspr/prlong.h"

# 30 "/usr/include/nspr/prlong.h"
extern __attribute__((visibility("default"))) PRInt64 LL_MaxInt(void);
extern __attribute__((visibility("default"))) PRInt64 LL_MinInt(void);
extern __attribute__((visibility("default"))) PRInt64 LL_Zero(void);
extern __attribute__((visibility("default"))) PRUint64 LL_MaxUint(void);
# 401 "/usr/include/nspr/prlong.h"

# 17 "/usr/include/nspr/prio.h" 2
# 1 "/usr/include/nspr/prtime.h" 1
# 21 "/usr/include/nspr/prtime.h"

# 48 "/usr/include/nspr/prtime.h"
typedef PRInt64 PRTime;






typedef struct PRTimeParameters {
    PRInt32 tp_gmt_offset;
    PRInt32 tp_dst_offset;
} PRTimeParameters;
# 83 "/usr/include/nspr/prtime.h"
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
# 126 "/usr/include/nspr/prtime.h"
typedef PRTimeParameters ( *PRTimeParamFn)(const PRExplodedTime *gmt);
# 145 "/usr/include/nspr/prtime.h"
extern __attribute__((visibility("default"))) PRTime
PR_Now(void);
# 158 "/usr/include/nspr/prtime.h"
extern __attribute__((visibility("default"))) void PR_ExplodeTime(
    PRTime usecs, PRTimeParamFn params, PRExplodedTime *exploded);


extern __attribute__((visibility("default"))) PRTime
PR_ImplodeTime(const PRExplodedTime *exploded);
# 177 "/usr/include/nspr/prtime.h"
extern __attribute__((visibility("default"))) void PR_NormalizeTime(
    PRExplodedTime *exploded, PRTimeParamFn params);






extern __attribute__((visibility("default"))) PRTimeParameters PR_LocalTimeParameters(const PRExplodedTime *gmt);


extern __attribute__((visibility("default"))) PRTimeParameters PR_GMTParameters(const PRExplodedTime *gmt);





extern __attribute__((visibility("default"))) PRTimeParameters PR_USPacificTimeParameters(const PRExplodedTime *gmt);
# 231 "/usr/include/nspr/prtime.h"
extern __attribute__((visibility("default"))) PRStatus PR_ParseTimeStringToExplodedTime (
    const char *string,
    PRBool default_to_gmt,
    PRExplodedTime *result);
# 244 "/usr/include/nspr/prtime.h"
extern __attribute__((visibility("default"))) PRStatus PR_ParseTimeString (
    const char *string,
    PRBool default_to_gmt,
    PRTime *result);


extern __attribute__((visibility("default"))) PRUint32 PR_FormatTime(char *buf, int buflen, const char *fmt,
                                 const PRExplodedTime *time);




extern __attribute__((visibility("default"))) PRUint32
PR_FormatTimeUSEnglish(char *buf, PRUint32 bufSize,
                       const char *format, const PRExplodedTime *time);


# 18 "/usr/include/nspr/prio.h" 2






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
# 50 "/usr/include/nspr/prio.h"
typedef PRIntn PRDescIdentity;

struct PRFileDesc {
    const PRIOMethods *methods;
    PRFilePrivate *secret;
    PRFileDesc *lower, *higher;
    void ( *dtor)(PRFileDesc *fd);

    PRDescIdentity identity;
};
# 70 "/usr/include/nspr/prio.h"
typedef enum PRTransmitFileFlags {
    PR_TRANSMITFILE_KEEP_OPEN = 0,

    PR_TRANSMITFILE_CLOSE_SOCKET = 1

} PRTransmitFileFlags;
# 130 "/usr/include/nspr/prio.h"
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
# 180 "/usr/include/nspr/prio.h"
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
    PR_SockOpt_Reuseport,

    PR_SockOpt_DontFrag,
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
        PRBool reuse_port;

        PRBool dont_fragment;
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
# 252 "/usr/include/nspr/prio.h"
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
# 295 "/usr/include/nspr/prio.h"
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
# 409 "/usr/include/nspr/prio.h"
typedef enum PRSpecialFD
{
    PR_StandardInput,
    PR_StandardOutput,
    PR_StandardError
} PRSpecialFD;

extern __attribute__((visibility("default"))) PRFileDesc* PR_GetSpecialFD(PRSpecialFD id);
# 456 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRDescIdentity PR_GetUniqueIdentity(const char *layer_name);
extern __attribute__((visibility("default"))) const char* PR_GetNameForIdentity(PRDescIdentity ident);
extern __attribute__((visibility("default"))) PRDescIdentity PR_GetLayersIdentity(PRFileDesc* fd);
extern __attribute__((visibility("default"))) PRFileDesc* PR_GetIdentitiesLayer(PRFileDesc* fd_stack, PRDescIdentity id);
# 469 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) const PRIOMethods * PR_GetDefaultIOMethods(void);
# 480 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRFileDesc* PR_CreateIOLayerStub(
    PRDescIdentity ident, const PRIOMethods *methods);
# 497 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRFileDesc* PR_CreateIOLayer(PRFileDesc* fd);
# 513 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_PushIOLayer(
    PRFileDesc *fd_stack, PRDescIdentity id, PRFileDesc *layer);
# 530 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRFileDesc* PR_PopIOLayer(PRFileDesc *fd_stack, PRDescIdentity id);
# 604 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRFileDesc* PR_Open(const char *name, PRIntn flags, PRIntn mode);
# 630 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRFileDesc* PR_OpenFile(
    const char *name, PRIntn flags, PRIntn mode);
# 661 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_Close(PRFileDesc *fd);
# 696 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRInt32 PR_Read(PRFileDesc *fd, void *buf, PRInt32 amount);
# 720 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRInt32 PR_Write(PRFileDesc *fd,const void *buf,PRInt32 amount);
# 754 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRInt32 PR_Writev(
    PRFileDesc *fd, const PRIOVec *iov, PRInt32 iov_size,
    PRIntervalTime timeout);
# 775 "/usr/include/nspr/prio.h"
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
# 818 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_GetFileInfo(const char *fn, PRFileInfo *info);
extern __attribute__((visibility("default"))) PRStatus PR_GetFileInfo64(const char *fn, PRFileInfo64 *info);
# 845 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_GetOpenFileInfo(PRFileDesc *fd, PRFileInfo *info);
extern __attribute__((visibility("default"))) PRStatus PR_GetOpenFileInfo64(PRFileDesc *fd, PRFileInfo64 *info);
# 864 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_Rename(const char *from, const char *to);
# 890 "/usr/include/nspr/prio.h"
typedef enum PRAccessHow {
    PR_ACCESS_EXISTS = 1,
    PR_ACCESS_WRITE_OK = 2,
    PR_ACCESS_READ_OK = 3
} PRAccessHow;

extern __attribute__((visibility("default"))) PRStatus PR_Access(const char *name, PRAccessHow how);
# 931 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PROffset32 PR_Seek(PRFileDesc *fd, PROffset32 offset, PRSeekWhence whence);
extern __attribute__((visibility("default"))) PROffset64 PR_Seek64(PRFileDesc *fd, PROffset64 offset, PRSeekWhence whence);
# 954 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRInt32 PR_Available(PRFileDesc *fd);
extern __attribute__((visibility("default"))) PRInt64 PR_Available64(PRFileDesc *fd);
# 974 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_Sync(PRFileDesc *fd);



struct PRDirEntry {
    const char *name;
};
# 1013 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRDir* PR_OpenDir(const char *name);
# 1045 "/usr/include/nspr/prio.h"
typedef enum PRDirFlags {
    PR_SKIP_NONE = 0x0,
    PR_SKIP_DOT = 0x1,
    PR_SKIP_DOT_DOT = 0x2,
    PR_SKIP_BOTH = 0x3,
    PR_SKIP_HIDDEN = 0x4
} PRDirFlags;

extern __attribute__((visibility("default"))) PRDirEntry* PR_ReadDir(PRDir *dir, PRDirFlags flags);
# 1079 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_CloseDir(PRDir *dir);
# 1108 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_MkDir(const char *name, PRIntn mode);
# 1120 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_MakeDir(const char *name, PRIntn mode);
# 1140 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_RmDir(const char *name);
# 1159 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRFileDesc* PR_NewUDPSocket(void);
# 1178 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRFileDesc* PR_NewTCPSocket(void);
# 1198 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRFileDesc* PR_OpenUDPSocket(PRIntn af);
# 1218 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRFileDesc* PR_OpenTCPSocket(PRIntn af);
# 1246 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_Connect(
    PRFileDesc *fd, const PRNetAddr *addr, PRIntervalTime timeout);
# 1281 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_ConnectContinue(PRFileDesc *fd, PRInt16 out_flags);
# 1311 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_GetConnectStatus(const PRPollDesc *pd);
# 1335 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRFileDesc* PR_Accept(
    PRFileDesc *fd, PRNetAddr *addr, PRIntervalTime timeout);
# 1357 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_Bind(PRFileDesc *fd, const PRNetAddr *addr);
# 1379 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_Listen(PRFileDesc *fd, PRIntn backlog);
# 1403 "/usr/include/nspr/prio.h"
typedef enum PRShutdownHow
{
    PR_SHUTDOWN_RCV = 0,
    PR_SHUTDOWN_SEND = 1,
    PR_SHUTDOWN_BOTH = 2
} PRShutdownHow;

extern __attribute__((visibility("default"))) PRStatus PR_Shutdown(PRFileDesc *fd, PRShutdownHow how);
# 1443 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRInt32 PR_Recv(PRFileDesc *fd, void *buf, PRInt32 amount,
                             PRIntn flags, PRIntervalTime timeout);
# 1474 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRInt32 PR_Send(PRFileDesc *fd, const void *buf, PRInt32 amount,
                             PRIntn flags, PRIntervalTime timeout);
# 1509 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRInt32 PR_RecvFrom(
    PRFileDesc *fd, void *buf, PRInt32 amount, PRIntn flags,
    PRNetAddr *addr, PRIntervalTime timeout);
# 1542 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRInt32 PR_SendTo(
    PRFileDesc *fd, const void *buf, PRInt32 amount, PRIntn flags,
    const PRNetAddr *addr, PRIntervalTime timeout);
# 1582 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRInt32 PR_TransmitFile(
    PRFileDesc *networkSocket, PRFileDesc *sourceFile,
    const void *headers, PRInt32 hlen, PRTransmitFileFlags flags,
    PRIntervalTime timeout);
# 1621 "/usr/include/nspr/prio.h"
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
# 1688 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRInt32 PR_AcceptRead(
    PRFileDesc *listenSock, PRFileDesc **acceptedSock,
    PRNetAddr **peerAddr, void *buf, PRInt32 amount, PRIntervalTime timeout);
# 1712 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_NewTCPSocketPair(PRFileDesc *fds[2]);
# 1732 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_GetSockName(PRFileDesc *fd, PRNetAddr *addr);
# 1754 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_GetPeerName(PRFileDesc *fd, PRNetAddr *addr);

extern __attribute__((visibility("default"))) PRStatus PR_GetSocketOption(
    PRFileDesc *fd, PRSocketOptionData *data);

extern __attribute__((visibility("default"))) PRStatus PR_SetSocketOption(
    PRFileDesc *fd, const PRSocketOptionData *data);
# 1789 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_SetFDInheritable(
    PRFileDesc *fd,
    PRBool inheritable);
# 1809 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRFileDesc * PR_GetInheritedFD(const char *name);
# 1819 "/usr/include/nspr/prio.h"
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
# 1858 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_SyncMemMap(
    PRFileDesc *fd,
    void *addr,
    PRUint32 len);
# 1876 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRStatus PR_CreatePipe(
    PRFileDesc **readPipe,
    PRFileDesc **writePipe
);





struct PRPollDesc {
    PRFileDesc* fd;
    PRInt16 in_flags;
    PRInt16 out_flags;
};
# 1957 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRInt32 PR_Poll(
    PRPollDesc *pds, PRIntn npds, PRIntervalTime timeout);
# 2001 "/usr/include/nspr/prio.h"
extern __attribute__((visibility("default"))) PRFileDesc * PR_NewPollableEvent(void);

extern __attribute__((visibility("default"))) PRStatus PR_DestroyPollableEvent(PRFileDesc *event);

extern __attribute__((visibility("default"))) PRStatus PR_SetPollableEvent(PRFileDesc *event);

extern __attribute__((visibility("default"))) PRStatus PR_WaitForPollableEvent(PRFileDesc *event);


# 21 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/pripcsem.h" 1
# 30 "/usr/include/nspr/pripcsem.h"






typedef struct PRSem PRSem;
# 60 "/usr/include/nspr/pripcsem.h"
extern __attribute__((visibility("default"))) PRSem * PR_OpenSemaphore(
    const char *name, PRIntn flags, PRIntn mode, PRUintn value);
# 73 "/usr/include/nspr/pripcsem.h"
extern __attribute__((visibility("default"))) PRStatus PR_WaitSemaphore(PRSem *sem);







extern __attribute__((visibility("default"))) PRStatus PR_PostSemaphore(PRSem *sem);







extern __attribute__((visibility("default"))) PRStatus PR_CloseSemaphore(PRSem *sem);







extern __attribute__((visibility("default"))) PRStatus PR_DeleteSemaphore(const char *name);


# 22 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prlink.h" 1
# 14 "/usr/include/nspr/prlink.h"


typedef struct PRLibrary PRLibrary;

typedef struct PRStaticLinkTable {
    const char *name;
    void (*fp)(void);
} PRStaticLinkTable;
# 30 "/usr/include/nspr/prlink.h"
extern __attribute__((visibility("default"))) PRStatus PR_SetLibraryPath(const char *path);
# 40 "/usr/include/nspr/prlink.h"
extern __attribute__((visibility("default"))) char* PR_GetLibraryPath(void);
# 53 "/usr/include/nspr/prlink.h"
extern __attribute__((visibility("default"))) char* PR_GetLibraryName(const char *dir, const char *lib);





extern __attribute__((visibility("default"))) void PR_FreeLibraryName(char *mem);
# 73 "/usr/include/nspr/prlink.h"
extern __attribute__((visibility("default"))) PRLibrary* PR_LoadLibrary(const char *name);
# 94 "/usr/include/nspr/prlink.h"
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
# 144 "/usr/include/nspr/prlink.h"
extern __attribute__((visibility("default"))) PRLibrary *
PR_LoadLibraryWithFlags(
    PRLibSpec libSpec,
    PRIntn flags
);
# 159 "/usr/include/nspr/prlink.h"
extern __attribute__((visibility("default"))) PRStatus PR_UnloadLibrary(PRLibrary *lib);
# 169 "/usr/include/nspr/prlink.h"
extern __attribute__((visibility("default"))) void* PR_FindSymbol(PRLibrary *lib, const char *name);
# 179 "/usr/include/nspr/prlink.h"
typedef void (*PRFuncPtr)(void);
extern __attribute__((visibility("default"))) PRFuncPtr PR_FindFunctionSymbol(PRLibrary *lib, const char *name);
# 192 "/usr/include/nspr/prlink.h"
extern __attribute__((visibility("default"))) void* PR_FindSymbolAndLibrary(const char *name,
                                        PRLibrary* *lib);
# 203 "/usr/include/nspr/prlink.h"
extern __attribute__((visibility("default"))) PRFuncPtr PR_FindFunctionSymbolAndLibrary(const char *name,
        PRLibrary* *lib);
# 217 "/usr/include/nspr/prlink.h"
extern __attribute__((visibility("default"))) PRLibrary* PR_LoadStaticLibrary(
    const char *name, const PRStaticLinkTable *table);







extern __attribute__((visibility("default"))) char * PR_GetLibraryFilePathname(const char *name, PRFuncPtr addr);


# 23 "/usr/include/nspr/nspr.h" 2

# 1 "/usr/include/nspr/prlog.h" 1
# 11 "/usr/include/nspr/prlog.h"

# 106 "/usr/include/nspr/prlog.h"
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

extern __attribute__((visibility("default"))) void PR_Assert(const char *s, const char *file, PRIntn ln)
;
# 224 "/usr/include/nspr/prlog.h"

# 25 "/usr/include/nspr/nspr.h" 2

# 1 "/usr/include/nspr/prmem.h" 1
# 15 "/usr/include/nspr/prmem.h"
# 1 "/usr/include/stdlib.h" 1 3 4
# 26 "/usr/include/stdlib.h" 3 4
# 1 "/usr/include/bits/libc-header-start.h" 1 3 4
# 27 "/usr/include/stdlib.h" 2 3 4





# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 1 3 4
# 33 "/usr/include/stdlib.h" 2 3 4







# 1 "/usr/include/bits/waitflags.h" 1 3 4
# 41 "/usr/include/stdlib.h" 2 3 4
# 1 "/usr/include/bits/waitstatus.h" 1 3 4
# 42 "/usr/include/stdlib.h" 2 3 4
# 59 "/usr/include/stdlib.h" 3 4

# 59 "/usr/include/stdlib.h" 3 4
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
# 98 "/usr/include/stdlib.h" 3 4
extern size_t __ctype_get_mb_cur_max (void) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__));



extern double atof (const char *__nptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

extern int atoi (const char *__nptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));

extern long int atol (const char *__nptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));



__extension__ extern long long int atoll (const char *__nptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));



extern double strtod (const char *__restrict __nptr,
        char **__restrict __endptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));



extern float strtof (const char *__restrict __nptr,
       char **__restrict __endptr) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));

extern long double strtold (const char *__restrict __nptr,
       char **__restrict __endptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
# 177 "/usr/include/stdlib.h" 3 4
extern long int strtol (const char *__restrict __nptr,
   char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));

extern unsigned long int strtoul (const char *__restrict __nptr,
      char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));



__extension__
extern long long int strtoq (const char *__restrict __nptr,
        char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));

__extension__
extern unsigned long long int strtouq (const char *__restrict __nptr,
           char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));




__extension__
extern long long int strtoll (const char *__restrict __nptr,
         char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));

__extension__
extern unsigned long long int strtoull (const char *__restrict __nptr,
     char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
# 480 "/usr/include/stdlib.h" 3 4
extern __inline __attribute__ ((__gnu_inline__)) int
__attribute__ ((__nothrow__ , __leaf__)) atoi (const char *__nptr)
{
  return (int) strtol (__nptr, (char **) ((void *)0), 10);
}
extern __inline __attribute__ ((__gnu_inline__)) long int
__attribute__ ((__nothrow__ , __leaf__)) atol (const char *__nptr)
{
  return strtol (__nptr, (char **) ((void *)0), 10);
}


__extension__ extern __inline __attribute__ ((__gnu_inline__)) long long int
__attribute__ ((__nothrow__ , __leaf__)) atoll (const char *__nptr)
{
  return strtoll (__nptr, (char **) ((void *)0), 10);
}
# 505 "/usr/include/stdlib.h" 3 4
extern char *l64a (long int __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__));


extern long int a64l (const char *__s)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
# 521 "/usr/include/stdlib.h" 3 4
extern long int random (void) __attribute__ ((__nothrow__ , __leaf__));


extern void srandom (unsigned int __seed) __attribute__ ((__nothrow__ , __leaf__));





extern char *initstate (unsigned int __seed, char *__statebuf,
   size_t __statelen) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));



extern char *setstate (char *__statebuf) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));







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
       int32_t *__restrict __result) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));

extern int srandom_r (unsigned int __seed, struct random_data *__buf)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));

extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
   size_t __statelen,
   struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 4)));

extern int setstate_r (char *__restrict __statebuf,
         struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));





extern int rand (void) __attribute__ ((__nothrow__ , __leaf__));

extern void srand (unsigned int __seed) __attribute__ ((__nothrow__ , __leaf__));



extern int rand_r (unsigned int *__seed) __attribute__ ((__nothrow__ , __leaf__));







extern double drand48 (void) __attribute__ ((__nothrow__ , __leaf__));
extern double erand48 (unsigned short int __xsubi[3]) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));


extern long int lrand48 (void) __attribute__ ((__nothrow__ , __leaf__));
extern long int nrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));


extern long int mrand48 (void) __attribute__ ((__nothrow__ , __leaf__));
extern long int jrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));


extern void srand48 (long int __seedval) __attribute__ ((__nothrow__ , __leaf__));
extern unsigned short int *seed48 (unsigned short int __seed16v[3])
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
extern void lcong48 (unsigned short int __param[7]) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));





struct drand48_data
  {
    unsigned short int __x[3];
    unsigned short int __old_x[3];
    unsigned short int __c;
    unsigned short int __init;
    __extension__ unsigned long long int __a;

  };


extern int drand48_r (struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern int erand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern int lrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern int nrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern int mrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern int jrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern int srand48_r (long int __seedval, struct drand48_data *__buffer)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));

extern int seed48_r (unsigned short int __seed16v[3],
       struct drand48_data *__buffer) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));

extern int lcong48_r (unsigned short int __param[7],
        struct drand48_data *__buffer)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern __uint32_t arc4random (void)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__));


extern void arc4random_buf (void *__buf, size_t __size)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));



extern __uint32_t arc4random_uniform (__uint32_t __upper_bound)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__));




extern void *malloc (size_t __size) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__))
     __attribute__ ((__alloc_size__ (1))) __attribute__ ((__warn_unused_result__));

extern void *calloc (size_t __nmemb, size_t __size)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__alloc_size__ (1, 2))) __attribute__ ((__warn_unused_result__));






extern void *realloc (void *__ptr, size_t __size)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__alloc_size__ (2)));


extern void free (void *__ptr) __attribute__ ((__nothrow__ , __leaf__));







extern void *reallocarray (void *__ptr, size_t __nmemb, size_t __size)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__))
     __attribute__ ((__alloc_size__ (2, 3)))
    __attribute__ ((__malloc__));


extern void *reallocarray (void *__ptr, size_t __nmemb, size_t __size)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__));



# 1 "/usr/include/alloca.h" 1 3 4
# 24 "/usr/include/alloca.h" 3 4
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 1 3 4
# 25 "/usr/include/alloca.h" 2 3 4







extern void *alloca (size_t __size) __attribute__ ((__nothrow__ , __leaf__));






# 707 "/usr/include/stdlib.h" 2 3 4





extern void *valloc (size_t __size) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__))
     __attribute__ ((__alloc_size__ (1))) __attribute__ ((__warn_unused_result__));




extern int posix_memalign (void **__memptr, size_t __alignment, size_t __size)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
# 730 "/usr/include/stdlib.h" 3 4
extern void abort (void) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));



extern int atexit (void (*__func) (void)) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
# 749 "/usr/include/stdlib.h" 3 4
extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));





extern void exit (int __status) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));
# 768 "/usr/include/stdlib.h" 3 4
extern void _Exit (int __status) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));




extern char *getenv (const char *__name) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
# 786 "/usr/include/stdlib.h" 3 4
extern int putenv (char *__string) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));





extern int setenv (const char *__name, const char *__value, int __replace)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));


extern int unsetenv (const char *__name) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));






extern int clearenv (void) __attribute__ ((__nothrow__ , __leaf__));
# 814 "/usr/include/stdlib.h" 3 4
extern char *mktemp (char *__template) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
# 827 "/usr/include/stdlib.h" 3 4
extern int mkstemp (char *__template) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
# 849 "/usr/include/stdlib.h" 3 4
extern int mkstemps (char *__template, int __suffixlen) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
# 870 "/usr/include/stdlib.h" 3 4
extern char *mkdtemp (char *__template) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
# 923 "/usr/include/stdlib.h" 3 4
extern int system (const char *__command) __attribute__ ((__warn_unused_result__));
# 940 "/usr/include/stdlib.h" 3 4
extern char *realpath (const char *__restrict __name,
         char *__restrict __resolved) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__));






typedef int (*__compar_fn_t) (const void *, const void *);
# 960 "/usr/include/stdlib.h" 3 4
extern void *bsearch (const void *__key, const void *__base,
        size_t __nmemb, size_t __size, __compar_fn_t __compar)
     __attribute__ ((__nonnull__ (1, 2, 5))) __attribute__ ((__warn_unused_result__));


# 1 "/usr/include/bits/stdlib-bsearch.h" 1 3 4
# 19 "/usr/include/bits/stdlib-bsearch.h" 3 4
extern __inline __attribute__ ((__gnu_inline__)) void *
bsearch (const void *__key, const void *__base, size_t __nmemb, size_t __size,
  __compar_fn_t __compar)
{
  size_t __l, __u, __idx;
  const void *__p;
  int __comparison;

  __l = 0;
  __u = __nmemb;
  while (__l < __u)
    {
      __idx = (__l + __u) / 2;
      __p = (const void *) (((const char *) __base) + (__idx * __size));
      __comparison = (*__compar) (__key, __p);
      if (__comparison < 0)
 __u = __idx;
      else if (__comparison > 0)
 __l = __idx + 1;
      else
 {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

   return (void *) __p;

#pragma GCC diagnostic pop

 }
    }

  return ((void *)0);
}
# 966 "/usr/include/stdlib.h" 2 3 4




extern void qsort (void *__base, size_t __nmemb, size_t __size,
     __compar_fn_t __compar) __attribute__ ((__nonnull__ (1, 4)));
# 980 "/usr/include/stdlib.h" 3 4
extern int abs (int __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));
extern long int labs (long int __x) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));


__extension__ extern long long int llabs (long long int __x)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));






extern div_t div (int __numer, int __denom)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));
extern ldiv_t ldiv (long int __numer, long int __denom)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));


__extension__ extern lldiv_t lldiv (long long int __numer,
        long long int __denom)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__)) __attribute__ ((__warn_unused_result__));
# 1012 "/usr/include/stdlib.h" 3 4
extern char *ecvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4))) __attribute__ ((__warn_unused_result__));




extern char *fcvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4))) __attribute__ ((__warn_unused_result__));




extern char *gcvt (double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3))) __attribute__ ((__warn_unused_result__));




extern char *qecvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4))) __attribute__ ((__warn_unused_result__));
extern char *qfcvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4))) __attribute__ ((__warn_unused_result__));
extern char *qgcvt (long double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3))) __attribute__ ((__warn_unused_result__));




extern int ecvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int fcvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4, 5)));

extern int qecvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int qfcvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4, 5)));





extern int mblen (const char *__s, size_t __n) __attribute__ ((__nothrow__ , __leaf__));


extern int mbtowc (wchar_t *__restrict __pwc,
     const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__ , __leaf__));


extern int wctomb (char *__s, wchar_t __wchar) __attribute__ ((__nothrow__ , __leaf__));



extern size_t mbstowcs (wchar_t *__restrict __pwcs,
   const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__ , __leaf__))
    __attribute__ ((__access__ (__read_only__, 2)));

extern size_t wcstombs (char *__restrict __s,
   const wchar_t *__restrict __pwcs, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__access__ (__write_only__, 1, 3)))
  __attribute__ ((__access__ (__read_only__, 2)));






extern int rpmatch (const char *__response) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__warn_unused_result__));
# 1099 "/usr/include/stdlib.h" 3 4
extern int getsubopt (char **__restrict __optionp,
        char *const *__restrict __tokens,
        char **__restrict __valuep)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2, 3))) __attribute__ ((__warn_unused_result__));
# 1145 "/usr/include/stdlib.h" 3 4
extern int getloadavg (double __loadavg[], int __nelem)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
# 1155 "/usr/include/stdlib.h" 3 4
# 1 "/usr/include/bits/stdlib-float.h" 1 3 4
# 24 "/usr/include/bits/stdlib-float.h" 3 4
extern __inline __attribute__ ((__gnu_inline__)) double
__attribute__ ((__nothrow__ , __leaf__)) atof (const char *__nptr)
{
  return strtod (__nptr, (char **) ((void *)0));
}
# 1156 "/usr/include/stdlib.h" 2 3 4



# 1 "/usr/include/bits/stdlib.h" 1 3 4
# 23 "/usr/include/bits/stdlib.h" 3 4
extern char *__realpath_chk (const char *__restrict __name,
        char *__restrict __resolved,
        size_t __resolvedlen) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__));
extern char *__realpath_alias (const char *__restrict __name, char *__restrict __resolved) __asm__ ("" "realpath") __attribute__ ((__nothrow__ , __leaf__))

                                                 __attribute__ ((__warn_unused_result__));
extern char *__realpath_chk_warn (const char *__restrict __name, char *__restrict __resolved, size_t __resolvedlen) __asm__ ("" "__realpath_chk") __attribute__ ((__nothrow__ , __leaf__))


                                                __attribute__ ((__warn_unused_result__))
     __attribute__((__warning__ ("second argument of realpath must be either NULL or at " "least PATH_MAX bytes long buffer")))
                                      ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) char *
__attribute__ ((__nothrow__ , __leaf__)) realpath (const char *__restrict __name, char * __restrict __resolved)







{
  size_t __sz = __builtin_object_size (__resolved, 2 > 1);

  if (__sz == (size_t) -1)
    return __realpath_alias (__name, __resolved);





  return __realpath_chk (__name, __resolved, __sz);
}


extern int __ptsname_r_chk (int __fd, char *__buf, size_t __buflen,
       size_t __nreal) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)))
    __attribute__ ((__access__ (__write_only__, 2, 3)));
extern int __ptsname_r_alias (int __fd, char *__buf, size_t __buflen) __asm__ ("" "ptsname_r") __attribute__ ((__nothrow__ , __leaf__))

     __attribute__ ((__nonnull__ (2))) __attribute__ ((__access__ (__write_only__, 2, 3)));
extern int __ptsname_r_chk_warn (int __fd, char *__buf, size_t __buflen, size_t __nreal) __asm__ ("" "__ptsname_r_chk") __attribute__ ((__nothrow__ , __leaf__))


     __attribute__ ((__nonnull__ (2))) __attribute__((__warning__ ("ptsname_r called with buflen bigger than " "size of buf")))
                   ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__ , __leaf__)) ptsname_r (int __fd, char * __buf, size_t __buflen)


    


{
  return (((__builtin_constant_p (__builtin_object_size (__buf, 2 > 1)) && (__builtin_object_size (__buf, 2 > 1)) == (long unsigned int) -1) || (((__typeof (__buflen)) 0 < (__typeof (__buflen)) -1 || (__builtin_constant_p (__buflen) && (__buflen) > 0)) && __builtin_constant_p ((((long unsigned int) (__buflen)) <= ((__builtin_object_size (__buf, 2 > 1))) / ((sizeof (char))))) && (((long unsigned int) (__buflen)) <= ((__builtin_object_size (__buf, 2 > 1))) / ((sizeof (char)))))) ? __ptsname_r_alias (__fd, __buf, __buflen) : ((((__typeof (__buflen)) 0 < (__typeof (__buflen)) -1 || (__builtin_constant_p (__buflen) && (__buflen) > 0)) && __builtin_constant_p ((((long unsigned int) (__buflen)) <= (__builtin_object_size (__buf, 2 > 1)) / (sizeof (char)))) && !(((long unsigned int) (__buflen)) <= (__builtin_object_size (__buf, 2 > 1)) / (sizeof (char)))) ? __ptsname_r_chk_warn (__fd, __buf, __buflen, __builtin_object_size (__buf, 2 > 1)) : __ptsname_r_chk (__fd, __buf, __buflen, __builtin_object_size (__buf, 2 > 1))))

                           ;
}


extern int __wctomb_chk (char *__s, wchar_t __wchar, size_t __buflen)
  __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__));
extern int __wctomb_alias (char *__s, wchar_t __wchar) __asm__ ("" "wctomb") __attribute__ ((__nothrow__ , __leaf__))
              __attribute__ ((__warn_unused_result__));

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) int
__attribute__ ((__nothrow__ , __leaf__)) wctomb (char * __s, wchar_t __wchar)
{







  if (__builtin_object_size (__s, 2 > 1) != (size_t) -1
      && 16 > __builtin_object_size (__s, 2 > 1))
    return __wctomb_chk (__s, __wchar, __builtin_object_size (__s, 2 > 1));
  return __wctomb_alias (__s, __wchar);
}


extern size_t __mbstowcs_chk (wchar_t *__restrict __dst,
         const char *__restrict __src,
         size_t __len, size_t __dstlen) __attribute__ ((__nothrow__ , __leaf__))
    __attribute__ ((__access__ (__write_only__, 1, 3))) __attribute__ ((__access__ (__read_only__, 2)));
extern size_t __mbstowcs_nulldst (wchar_t *__restrict __dst, const char *__restrict __src, size_t __len) __asm__ ("" "mbstowcs") __attribute__ ((__nothrow__ , __leaf__))



    __attribute__ ((__access__ (__read_only__, 2)));
extern size_t __mbstowcs_alias (wchar_t *__restrict __dst, const char *__restrict __src, size_t __len) __asm__ ("" "mbstowcs") __attribute__ ((__nothrow__ , __leaf__))



    __attribute__ ((__access__ (__write_only__, 1, 3))) __attribute__ ((__access__ (__read_only__, 2)));
extern size_t __mbstowcs_chk_warn (wchar_t *__restrict __dst, const char *__restrict __src, size_t __len, size_t __dstlen) __asm__ ("" "__mbstowcs_chk") __attribute__ ((__nothrow__ , __leaf__))



     __attribute__((__warning__ ("mbstowcs called with dst buffer smaller than len " "* sizeof (wchar_t)")))
                        ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) size_t
__attribute__ ((__nothrow__ , __leaf__)) mbstowcs (wchar_t * __restrict __dst, const char *__restrict __src, size_t __len)


    


{
  if (__builtin_constant_p (__dst == ((void *)0)) && __dst == ((void *)0))
    return __mbstowcs_nulldst (__dst, __src, __len);
  else
    return (((__builtin_constant_p (__builtin_object_size (__dst, 2 > 1)) && (__builtin_object_size (__dst, 2 > 1)) == (long unsigned int) -1) || (((__typeof (__len)) 0 < (__typeof (__len)) -1 || (__builtin_constant_p (__len) && (__len) > 0)) && __builtin_constant_p ((((long unsigned int) (__len)) <= ((__builtin_object_size (__dst, 2 > 1))) / ((sizeof (wchar_t))))) && (((long unsigned int) (__len)) <= ((__builtin_object_size (__dst, 2 > 1))) / ((sizeof (wchar_t)))))) ? __mbstowcs_alias (__dst, __src, __len) : ((((__typeof (__len)) 0 < (__typeof (__len)) -1 || (__builtin_constant_p (__len) && (__len) > 0)) && __builtin_constant_p ((((long unsigned int) (__len)) <= (__builtin_object_size (__dst, 2 > 1)) / (sizeof (wchar_t)))) && !(((long unsigned int) (__len)) <= (__builtin_object_size (__dst, 2 > 1)) / (sizeof (wchar_t)))) ? __mbstowcs_chk_warn (__dst, __src, __len, (__builtin_object_size (__dst, 2 > 1)) / (sizeof (wchar_t))) : __mbstowcs_chk (__dst, __src, __len, (__builtin_object_size (__dst, 2 > 1)) / (sizeof (wchar_t)))))
                                                      ;
}

extern size_t __wcstombs_chk (char *__restrict __dst,
         const wchar_t *__restrict __src,
         size_t __len, size_t __dstlen) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__access__ (__write_only__, 1, 3))) __attribute__ ((__access__ (__read_only__, 2)));
extern size_t __wcstombs_alias (char *__restrict __dst, const wchar_t *__restrict __src, size_t __len) __asm__ ("" "wcstombs") __attribute__ ((__nothrow__ , __leaf__))



  __attribute__ ((__access__ (__write_only__, 1, 3))) __attribute__ ((__access__ (__read_only__, 2)));
extern size_t __wcstombs_chk_warn (char *__restrict __dst, const wchar_t *__restrict __src, size_t __len, size_t __dstlen) __asm__ ("" "__wcstombs_chk") __attribute__ ((__nothrow__ , __leaf__))



     __attribute__((__warning__ ("wcstombs called with dst buffer smaller than len")));

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) size_t
__attribute__ ((__nothrow__ , __leaf__)) wcstombs (char * __restrict __dst, const wchar_t *__restrict __src, size_t __len)


{
  return (((__builtin_constant_p (__builtin_object_size (__dst, 2 > 1)) && (__builtin_object_size (__dst, 2 > 1)) == (long unsigned int) -1) || (((__typeof (__len)) 0 < (__typeof (__len)) -1 || (__builtin_constant_p (__len) && (__len) > 0)) && __builtin_constant_p ((((long unsigned int) (__len)) <= ((__builtin_object_size (__dst, 2 > 1))) / ((sizeof (char))))) && (((long unsigned int) (__len)) <= ((__builtin_object_size (__dst, 2 > 1))) / ((sizeof (char)))))) ? __wcstombs_alias (__dst, __src, __len) : ((((__typeof (__len)) 0 < (__typeof (__len)) -1 || (__builtin_constant_p (__len) && (__len) > 0)) && __builtin_constant_p ((((long unsigned int) (__len)) <= (__builtin_object_size (__dst, 2 > 1)) / (sizeof (char)))) && !(((long unsigned int) (__len)) <= (__builtin_object_size (__dst, 2 > 1)) / (sizeof (char)))) ? __wcstombs_chk_warn (__dst, __src, __len, __builtin_object_size (__dst, 2 > 1)) : __wcstombs_chk (__dst, __src, __len, __builtin_object_size (__dst, 2 > 1))))

                         ;
}
# 1160 "/usr/include/stdlib.h" 2 3 4








# 16 "/usr/include/nspr/prmem.h" 2


# 34 "/usr/include/nspr/prmem.h"

# 34 "/usr/include/nspr/prmem.h"
extern __attribute__((visibility("default"))) void * PR_Malloc(PRUint32 size);

extern __attribute__((visibility("default"))) void * PR_Calloc(PRUint32 nelem, PRUint32 elsize);

extern __attribute__((visibility("default"))) void * PR_Realloc(void *ptr, PRUint32 size);

extern __attribute__((visibility("default"))) void PR_Free(void *ptr);
# 124 "/usr/include/nspr/prmem.h"

# 27 "/usr/include/nspr/nspr.h" 2

# 1 "/usr/include/nspr/prmwait.h" 1
# 14 "/usr/include/nspr/prmwait.h"

# 37 "/usr/include/nspr/prmwait.h"
typedef struct PRWaitGroup PRWaitGroup;
# 61 "/usr/include/nspr/prmwait.h"
typedef enum PRMWStatus
{
    PR_MW_PENDING = 1,
    PR_MW_SUCCESS = 0,
    PR_MW_FAILURE = -1,
    PR_MW_TIMEOUT = -2,
    PR_MW_INTERRUPT = -3
} PRMWStatus;
# 77 "/usr/include/nspr/prmwait.h"
typedef struct PRMemoryDescriptor
{
    void *start;
    PRSize length;
} PRMemoryDescriptor;
# 90 "/usr/include/nspr/prmwait.h"
typedef struct PRMWaitClientData PRMWaitClientData;
# 108 "/usr/include/nspr/prmwait.h"
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
# 130 "/usr/include/nspr/prmwait.h"
typedef struct PRMWaitEnumerator PRMWaitEnumerator;
# 164 "/usr/include/nspr/prmwait.h"
extern __attribute__((visibility("default"))) PRStatus PR_AddWaitFileDesc(PRWaitGroup *group, PRRecvWait *desc);
# 198 "/usr/include/nspr/prmwait.h"
extern __attribute__((visibility("default"))) PRRecvWait* PR_WaitRecvReady(PRWaitGroup *group);
# 233 "/usr/include/nspr/prmwait.h"
extern __attribute__((visibility("default"))) PRStatus PR_CancelWaitFileDesc(PRWaitGroup *group, PRRecvWait *desc);
# 260 "/usr/include/nspr/prmwait.h"
extern __attribute__((visibility("default"))) PRRecvWait* PR_CancelWaitGroup(PRWaitGroup *group);
# 287 "/usr/include/nspr/prmwait.h"
extern __attribute__((visibility("default"))) PRWaitGroup* PR_CreateWaitGroup(PRInt32 size);
# 310 "/usr/include/nspr/prmwait.h"
extern __attribute__((visibility("default"))) PRStatus PR_DestroyWaitGroup(PRWaitGroup *group);
# 332 "/usr/include/nspr/prmwait.h"
extern __attribute__((visibility("default"))) PRMWaitEnumerator* PR_CreateMWaitEnumerator(PRWaitGroup *group);
# 350 "/usr/include/nspr/prmwait.h"
extern __attribute__((visibility("default"))) PRStatus PR_DestroyMWaitEnumerator(PRMWaitEnumerator* enumerator);
# 373 "/usr/include/nspr/prmwait.h"
extern __attribute__((visibility("default"))) PRRecvWait* PR_EnumerateWaitGroup(
    PRMWaitEnumerator *enumerator, const PRRecvWait *previous);


# 29 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prnetdb.h" 1
# 12 "/usr/include/nspr/prnetdb.h"








extern __attribute__((visibility("default"))) PRStatus PR_StringToNetAddr(
    const char *string, PRNetAddr *addr);

extern __attribute__((visibility("default"))) PRStatus PR_NetAddrToString(
    const PRNetAddr *addr, char *string, PRUint32 size);
# 36 "/usr/include/nspr/prnetdb.h"
typedef struct PRHostEnt {
    char *h_name;
    char **h_aliases;




    PRInt32 h_addrtype;
    PRInt32 h_length;

    char **h_addr_list;
} PRHostEnt;
# 82 "/usr/include/nspr/prnetdb.h"
extern __attribute__((visibility("default"))) PRStatus PR_GetHostByName(
    const char *hostname, char *buf, PRIntn bufsize, PRHostEnt *hostentry);
# 119 "/usr/include/nspr/prnetdb.h"
extern __attribute__((visibility("default"))) PRStatus PR_GetIPNodeByName(
    const char *hostname,
    PRUint16 af,
    PRIntn flags,
    char *buf,
    PRIntn bufsize,
    PRHostEnt *hostentry);
# 148 "/usr/include/nspr/prnetdb.h"
extern __attribute__((visibility("default"))) PRStatus PR_GetHostByAddr(
    const PRNetAddr *hostaddr, char *buf, PRIntn bufsize, PRHostEnt *hostentry);
# 182 "/usr/include/nspr/prnetdb.h"
extern __attribute__((visibility("default"))) PRIntn PR_EnumerateHostEnt(
    PRIntn enumIndex, const PRHostEnt *hostEnt, PRUint16 port, PRNetAddr *address);
# 207 "/usr/include/nspr/prnetdb.h"
typedef enum PRNetAddrValue
{
    PR_IpAddrNull,
    PR_IpAddrAny,
    PR_IpAddrLoopback,
    PR_IpAddrV4Mapped
} PRNetAddrValue;

extern __attribute__((visibility("default"))) PRStatus PR_InitializeNetAddr(
    PRNetAddrValue val, PRUint16 port, PRNetAddr *addr);
# 243 "/usr/include/nspr/prnetdb.h"
extern __attribute__((visibility("default"))) PRStatus PR_SetNetAddr(
    PRNetAddrValue val, PRUint16 af, PRUint16 port, PRNetAddr *addr);
# 259 "/usr/include/nspr/prnetdb.h"
extern __attribute__((visibility("default"))) PRBool PR_IsNetAddrType(const PRNetAddr *addr, PRNetAddrValue val);
# 276 "/usr/include/nspr/prnetdb.h"
extern __attribute__((visibility("default"))) void PR_ConvertIPv4AddrToIPv6(PRUint32 v4addr, PRIPv6Addr *v6addr);
# 327 "/usr/include/nspr/prnetdb.h"
typedef struct PRProtoEnt {
    char *p_name;
    char **p_aliases;



    PRInt32 p_num;

} PRProtoEnt;

extern __attribute__((visibility("default"))) PRStatus PR_GetProtoByName(
    const char* protocolname, char* buffer, PRInt32 bufsize, PRProtoEnt* result);
# 362 "/usr/include/nspr/prnetdb.h"
extern __attribute__((visibility("default"))) PRStatus PR_GetProtoByNumber(
    PRInt32 protocolnumber, char* buffer, PRInt32 bufsize, PRProtoEnt* result);
# 386 "/usr/include/nspr/prnetdb.h"
typedef struct PRAddrInfo PRAddrInfo;

extern __attribute__((visibility("default"))) PRAddrInfo* PR_GetAddrInfoByName(
    const char *hostname, PRUint16 af, PRIntn flags);
# 403 "/usr/include/nspr/prnetdb.h"
extern __attribute__((visibility("default"))) void PR_FreeAddrInfo(PRAddrInfo *addrInfo);
# 428 "/usr/include/nspr/prnetdb.h"
extern __attribute__((visibility("default"))) void * PR_EnumerateAddrInfo(
    void *enumPtr, const PRAddrInfo *addrInfo, PRUint16 port, PRNetAddr *result);

extern __attribute__((visibility("default"))) PRStatus PR_GetPrefLoopbackAddrInfo(PRNetAddr *result,
                                              PRUint16 port);
# 450 "/usr/include/nspr/prnetdb.h"
extern __attribute__((visibility("default"))) const char * PR_GetCanonNameFromAddrInfo(
    const PRAddrInfo *addrInfo);
# 466 "/usr/include/nspr/prnetdb.h"
extern __attribute__((visibility("default"))) PRUint16 PR_ntohs(PRUint16);
extern __attribute__((visibility("default"))) PRUint32 PR_ntohl(PRUint32);
extern __attribute__((visibility("default"))) PRUint64 PR_ntohll(PRUint64);
extern __attribute__((visibility("default"))) PRUint16 PR_htons(PRUint16);
extern __attribute__((visibility("default"))) PRUint32 PR_htonl(PRUint32);
extern __attribute__((visibility("default"))) PRUint64 PR_htonll(PRUint64);


# 30 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prprf.h" 1
# 28 "/usr/include/nspr/prprf.h"
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stdarg.h" 1 3 4
# 29 "/usr/include/nspr/prprf.h" 2








extern __attribute__((visibility("default"))) PRUint32 PR_snprintf(char *out, PRUint32 outlen, const char *fmt, ...);






extern __attribute__((visibility("default"))) char* PR_smprintf(const char *fmt, ...);




extern __attribute__((visibility("default"))) void PR_smprintf_free(char *mem);
# 58 "/usr/include/nspr/prprf.h"
extern __attribute__((visibility("default"))) char* PR_sprintf_append(char *last, const char *fmt, ...);
# 67 "/usr/include/nspr/prprf.h"
typedef PRIntn (*PRStuffFunc)(void *arg, const char *s, PRUint32 slen);

extern __attribute__((visibility("default"))) PRUint32 PR_sxprintf(PRStuffFunc f, void *arg, const char *fmt, ...);




extern __attribute__((visibility("default"))) PRUint32 PR_fprintf(struct PRFileDesc* fd, const char *fmt, ...);




extern __attribute__((visibility("default"))) PRUint32 PR_vsnprintf(char *out, PRUint32 outlen, const char *fmt, va_list ap);
extern __attribute__((visibility("default"))) char* PR_vsmprintf(const char *fmt, va_list ap);
extern __attribute__((visibility("default"))) char* PR_vsprintf_append(char *last, const char *fmt, va_list ap);
extern __attribute__((visibility("default"))) PRUint32 PR_vsxprintf(PRStuffFunc f, void *arg, const char *fmt, va_list ap);
extern __attribute__((visibility("default"))) PRUint32 PR_vfprintf(struct PRFileDesc* fd, const char *fmt, va_list ap);
# 118 "/usr/include/nspr/prprf.h"
extern __attribute__((visibility("default"))) PRInt32 PR_sscanf(const char *buf, const char *fmt, ...);


# 31 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prproces.h" 1
# 12 "/usr/include/nspr/prproces.h"






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
# 66 "/usr/include/nspr/prproces.h"
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


# 32 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prrng.h" 1
# 19 "/usr/include/nspr/prrng.h"

# 67 "/usr/include/nspr/prrng.h"
extern __attribute__((visibility("default"))) PRSize PR_GetRandomNoise(
    void *buf,
    PRSize size
);


# 33 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prrwlock.h" 1
# 17 "/usr/include/nspr/prrwlock.h"

# 27 "/usr/include/nspr/prrwlock.h"
typedef struct PRRWLock PRRWLock;
# 44 "/usr/include/nspr/prrwlock.h"
extern __attribute__((visibility("default"))) PRRWLock* PR_NewRWLock(PRUint32 lock_rank, const char *lock_name);
# 54 "/usr/include/nspr/prrwlock.h"
extern __attribute__((visibility("default"))) void PR_DestroyRWLock(PRRWLock *lock);
# 64 "/usr/include/nspr/prrwlock.h"
extern __attribute__((visibility("default"))) void PR_RWLock_Rlock(PRRWLock *lock);
# 74 "/usr/include/nspr/prrwlock.h"
extern __attribute__((visibility("default"))) void PR_RWLock_Wlock(PRRWLock *lock);
# 84 "/usr/include/nspr/prrwlock.h"
extern __attribute__((visibility("default"))) void PR_RWLock_Unlock(PRRWLock *lock);


# 34 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prshm.h" 1
# 105 "/usr/include/nspr/prshm.h"





typedef struct PRSharedMemory PRSharedMemory;
# 147 "/usr/include/nspr/prshm.h"
extern __attribute__((visibility("default"))) PRSharedMemory *
PR_OpenSharedMemory(
    const char *name,
    PRSize size,
    PRIntn flags,
    PRIntn mode
);
# 181 "/usr/include/nspr/prshm.h"
extern __attribute__((visibility("default"))) void *
PR_AttachSharedMemory(
    PRSharedMemory *shm,
    PRIntn flags
);
# 207 "/usr/include/nspr/prshm.h"
extern __attribute__((visibility("default"))) PRStatus
PR_DetachSharedMemory(
    PRSharedMemory *shm,
    void *addr
);
# 229 "/usr/include/nspr/prshm.h"
extern __attribute__((visibility("default"))) PRStatus
PR_CloseSharedMemory(
    PRSharedMemory *shm
);
# 250 "/usr/include/nspr/prshm.h"
extern __attribute__((visibility("default"))) PRStatus
PR_DeleteSharedMemory(
    const char *name
);


# 35 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prshma.h" 1
# 106 "/usr/include/nspr/prshma.h"

# 134 "/usr/include/nspr/prshma.h"
extern __attribute__((visibility("default"))) PRFileMap *
PR_OpenAnonFileMap(
    const char *dirName,
    PRSize size,
    PRFileMapProtect prot
);
# 162 "/usr/include/nspr/prshma.h"
extern __attribute__((visibility("default"))) PRStatus
PR_ProcessAttrSetInheritableFileMap(
    PRProcessAttr *attr,
    PRFileMap *fm,
    const char *shmname
);
# 187 "/usr/include/nspr/prshma.h"
extern __attribute__((visibility("default"))) PRFileMap *
PR_GetInheritedFileMap(
    const char *shmname
);
# 211 "/usr/include/nspr/prshma.h"
extern __attribute__((visibility("default"))) PRStatus
PR_ExportFileMapAsString(
    PRFileMap *fm,
    PRSize bufsize,
    char *buf
);
# 233 "/usr/include/nspr/prshma.h"
extern __attribute__((visibility("default"))) PRFileMap *
PR_ImportFileMapFromString(
    const char *fmstring
);


# 36 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prsystem.h" 1
# 14 "/usr/include/nspr/prsystem.h"








extern __attribute__((visibility("default"))) char PR_GetDirectorySeparator(void);






extern __attribute__((visibility("default"))) char PR_GetDirectorySepartor(void);







extern __attribute__((visibility("default"))) char PR_GetPathSeparator(void);


typedef enum {
    PR_SI_HOSTNAME,

    PR_SI_SYSNAME,
    PR_SI_RELEASE,
    PR_SI_ARCHITECTURE,
    PR_SI_HOSTNAME_UNTRUNCATED,

    PR_SI_RELEASE_BUILD
} PRSysInfo;
# 63 "/usr/include/nspr/prsystem.h"
extern __attribute__((visibility("default"))) PRStatus PR_GetSystemInfo(PRSysInfo cmd, char *buf, PRUint32 buflen);




extern __attribute__((visibility("default"))) PRInt32 PR_GetPageSize(void);




extern __attribute__((visibility("default"))) PRInt32 PR_GetPageShift(void);
# 89 "/usr/include/nspr/prsystem.h"
extern __attribute__((visibility("default"))) PRInt32 PR_GetNumberOfProcessors( void );
# 105 "/usr/include/nspr/prsystem.h"
extern __attribute__((visibility("default"))) PRUint64 PR_GetPhysicalMemorySize(void);


# 37 "/usr/include/nspr/nspr.h" 2


# 1 "/usr/include/nspr/prtpool.h" 1
# 20 "/usr/include/nspr/prtpool.h"


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


# 40 "/usr/include/nspr/nspr.h" 2
# 1 "/usr/include/nspr/prtrace.h" 1
# 68 "/usr/include/nspr/prtrace.h"







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
# 176 "/usr/include/nspr/prtrace.h"
extern __attribute__((visibility("default"))) PRTraceHandle
PR_CreateTrace(
    const char *qName,
    const char *rName,
    const char *description
);
# 210 "/usr/include/nspr/prtrace.h"
extern __attribute__((visibility("default"))) void
PR_DestroyTrace(
    PRTraceHandle handle
);
# 251 "/usr/include/nspr/prtrace.h"
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
# 338 "/usr/include/nspr/prtrace.h"
extern __attribute__((visibility("default"))) void
PR_SetTraceOption(
    PRTraceOption command,
    void *value
);
# 376 "/usr/include/nspr/prtrace.h"
extern __attribute__((visibility("default"))) void
PR_GetTraceOption(
    PRTraceOption command,
    void *value
);
# 412 "/usr/include/nspr/prtrace.h"
extern __attribute__((visibility("default"))) PRTraceHandle
PR_GetTraceHandleFromName(
    const char *qName,
    const char *rName
);
# 444 "/usr/include/nspr/prtrace.h"
extern __attribute__((visibility("default"))) void
PR_GetTraceNameFromHandle(
    PRTraceHandle handle,
    const char **qName,
    const char **rName,
    const char **description
);
# 486 "/usr/include/nspr/prtrace.h"
extern __attribute__((visibility("default"))) PRTraceHandle
PR_FindNextTraceQname(
    PRTraceHandle handle
);
# 528 "/usr/include/nspr/prtrace.h"
extern __attribute__((visibility("default"))) PRTraceHandle
PR_FindNextTraceRname(
    PRTraceHandle rhandle,
    PRTraceHandle qhandle
);
# 584 "/usr/include/nspr/prtrace.h"
extern __attribute__((visibility("default"))) void
PR_RecordTraceEntries(
    void
);
# 636 "/usr/include/nspr/prtrace.h"
extern __attribute__((visibility("default"))) PRIntn
PR_GetTraceEntries(
    PRTraceEntry *buffer,
    PRInt32 count,
    PRInt32 *found
);


# 41 "/usr/include/nspr/nspr.h" 2
# 15 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbaset.h" 2
# 1 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/util/nssilock.h" 1
# 104 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/util/nssilock.h"
# 1 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/util/utilrename.h" 1
# 105 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/util/nssilock.h" 2
# 1 "/usr/include/nspr/prtypes.h" 1
# 106 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/util/nssilock.h" 2
# 1 "/usr/include/nspr/prmon.h" 1
# 107 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/util/nssilock.h" 2
# 1 "/usr/include/nspr/prlock.h" 1
# 108 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/util/nssilock.h" 2
# 1 "/usr/include/nspr/prcvar.h" 1
# 109 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/util/nssilock.h" 2

# 1 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/util/nssilckt.h" 1
# 110 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/util/nssilckt.h"
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
# 111 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/util/nssilock.h" 2


# 266 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/util/nssilock.h"

# 16 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbaset.h" 2
# 32 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbaset.h"

# 43 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbaset.h"
typedef PRInt32 NSSError;
# 56 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbaset.h"
struct NSSArenaStr;
typedef struct NSSArenaStr NSSArena;
# 66 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbaset.h"
struct NSSItemStr {
    void *data;
    PRUint32 size;
};
typedef struct NSSItemStr NSSItem;







typedef NSSItem NSSBER;
# 87 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbaset.h"
typedef NSSBER NSSDER;
# 98 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbaset.h"
typedef NSSItem NSSBitString;







typedef char NSSUTF8;







typedef char NSSASCII7;


# 17 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/baset.h" 2


# 1 "/usr/include/nspr/plhash.h" 1
# 14 "/usr/include/nspr/plhash.h"


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


# 20 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/baset.h" 2









struct nssArenaMarkStr;
typedef struct nssArenaMarkStr nssArenaMark;
# 69 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/baset.h"
typedef struct nssListStr nssList;
typedef struct nssListIteratorStr nssListIterator;
typedef PRBool (*nssListCompareFunc)(void *a, void *b);
typedef PRIntn (*nssListSortFunc)(void *a, void *b);
typedef void (*nssListElementDestructorFunc)(void *el);

typedef struct nssHashStr nssHash;
typedef void( *nssHashIterator)(const void *key, void *value,
                                           void *arg);
# 108 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/baset.h"
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


# 17 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h" 2



# 1 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbase.h" 1
# 19 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbase.h"

# 47 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbase.h"
extern NSSArena *NSSArena_Create(void);

extern const NSSError NSS_ERROR_NO_MEMORY;
# 67 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbase.h"
extern PRStatus NSSArena_Destroy(NSSArena *arena);

extern const NSSError NSS_ERROR_INVALID_ARENA;
# 95 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbase.h"
extern NSSError NSS_GetError(void);

extern const NSSError NSS_ERROR_NO_ERROR;
# 117 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbase.h"
extern NSSError *NSS_GetErrorStack(void);
# 187 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbase.h"
extern void *NSS_ZAlloc(NSSArena *arenaOpt, PRUint32 size);
# 209 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbase.h"
extern void *NSS_ZRealloc(void *pointer, PRUint32 newSize);
# 229 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/nssbase.h"
extern PRStatus NSS_ZFreeIf(void *pointer);


# 21 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h" 2





# 86 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern NSSArena *nssArena_Create(void);

extern const NSSError NSS_ERROR_NO_MEMORY;
# 106 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern PRStatus nssArena_Destroy(NSSArena *arena);

extern const NSSError NSS_ERROR_INVALID_ARENA;
# 131 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern nssArenaMark *nssArena_Mark(NSSArena *arena);

extern const NSSError NSS_ERROR_INVALID_ARENA;
extern const NSSError NSS_ERROR_NO_MEMORY;
extern const NSSError NSS_ERROR_ARENA_MARKED_BY_ANOTHER_THREAD;
# 156 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern PRStatus nssArena_Release(NSSArena *arena, nssArenaMark *arenaMark);

extern const NSSError NSS_ERROR_INVALID_ARENA;
extern const NSSError NSS_ERROR_INVALID_ARENA_MARK;
# 183 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern PRStatus nssArena_Unmark(NSSArena *arena, nssArenaMark *arenaMark);

extern const NSSError NSS_ERROR_INVALID_ARENA;
extern const NSSError NSS_ERROR_INVALID_ARENA_MARK;
extern const NSSError NSS_ERROR_ARENA_MARKED_BY_ANOTHER_THREAD;
# 274 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern void *nss_ZAlloc(NSSArena *arenaOpt, PRUint32 size);

extern const NSSError NSS_ERROR_INVALID_ARENA;
extern const NSSError NSS_ERROR_NO_MEMORY;
extern const NSSError NSS_ERROR_ARENA_MARKED_BY_ANOTHER_THREAD;
# 298 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern PRStatus nss_ZFreeIf(void *pointer);

extern const NSSError NSS_ERROR_INVALID_POINTER;
# 322 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern void *nss_ZRealloc(void *pointer, PRUint32 newSize);

extern const NSSError NSS_ERROR_INVALID_POINTER;
extern const NSSError NSS_ERROR_NO_MEMORY;
extern const NSSError NSS_ERROR_ARENA_MARKED_BY_ANOTHER_THREAD;
# 441 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern PRStatus nssArena_Shutdown(void);
# 454 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern PLHashAllocOps nssArenaHashAllocOps;
# 473 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern void nss_SetError(PRUint32 error);







extern void nss_ClearErrorStack(void);







extern void nss_DestroyErrorStack(void);
# 499 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern NSSItem *nssItem_Create(NSSArena *arenaOpt, NSSItem *rvOpt,
                                   PRUint32 length, const void *data);

extern void nssItem_Destroy(NSSItem *item);

extern NSSItem *nssItem_Duplicate(NSSItem *obj, NSSArena *arenaOpt,
                                      NSSItem *rvOpt);

extern PRBool nssItem_Equal(const NSSItem *one, const NSSItem *two,
                                PRStatus *statusOpt);
# 535 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern PRBool nssUTF8_CaseIgnoreMatch(const NSSUTF8 *a, const NSSUTF8 *b,
                                          PRStatus *statusOpt);
# 555 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern NSSUTF8 *nssUTF8_Duplicate(const NSSUTF8 *s, NSSArena *arenaOpt);
# 579 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern PRBool nssUTF8_PrintableMatch(const NSSUTF8 *a, const NSSUTF8 *b,
                                         PRStatus *statusOpt);
# 598 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern PRUint32 nssUTF8_Size(const NSSUTF8 *s, PRStatus *statusOpt);

extern const NSSError NSS_ERROR_INVALID_POINTER;
extern const NSSError NSS_ERROR_VALUE_TOO_LARGE;
# 620 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern PRUint32 nssUTF8_Length(const NSSUTF8 *s, PRStatus *statusOpt);

extern const NSSError NSS_ERROR_INVALID_POINTER;
extern const NSSError NSS_ERROR_VALUE_TOO_LARGE;
extern const NSSError NSS_ERROR_INVALID_STRING;
# 649 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern NSSUTF8 *nssUTF8_Create(NSSArena *arenaOpt, nssStringType type,
                                   const void *inputString,
                                   PRUint32 size
                                   );

extern const NSSError NSS_ERROR_INVALID_POINTER;
extern const NSSError NSS_ERROR_NO_MEMORY;
extern const NSSError NSS_ERROR_UNSUPPORTED_TYPE;

extern NSSItem *nssUTF8_GetEncoding(NSSArena *arenaOpt, NSSItem *rvOpt,
                                        nssStringType type, NSSUTF8 *string);
# 672 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern const NSSError NSS_ERROR_INVALID_POINTER;
extern const NSSError NSS_ERROR_INVALID_ARGUMENT;

extern PRStatus nssUTF8_CopyIntoFixedBuffer(NSSUTF8 *string, char *buffer,
                                                PRUint32 bufferSize, char pad);






extern PRBool nssUTF8_Equal(const NSSUTF8 *a, const NSSUTF8 *b,
                                PRStatus *statusOpt);
# 701 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern nssList *nssList_Create(NSSArena *arenaOpt, PRBool threadSafe);




extern PRStatus nssList_Destroy(nssList *list);

extern void nssList_Clear(nssList *list,
                              nssListElementDestructorFunc destructor);
# 718 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern void nssList_SetCompareFunction(nssList *list,
                                           nssListCompareFunc compareFunc);






extern void nssList_SetSortFunction(nssList *list,
                                        nssListSortFunc sortFunc);




extern PRStatus nssList_Add(nssList *list, void *data);







extern PRStatus nssList_AddUnique(nssList *list, void *data);






extern PRStatus nssList_Remove(nssList *list, void *data);







extern void *nssList_Get(nssList *list, void *data);




extern PRUint32 nssList_Count(nssList *list);







extern PRStatus nssList_GetArray(nssList *list, void **rvArray,
                                     PRUint32 maxElements);






extern nssListIterator *nssList_CreateIterator(nssList *list);

extern nssList *nssList_Clone(nssList *list);




extern void nssListIterator_Destroy(nssListIterator *iter);







extern void *nssListIterator_Start(nssListIterator *iter);






extern void *nssListIterator_Next(nssListIterator *iter);







extern PRStatus nssListIterator_Finish(nssListIterator *iter);
# 826 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern nssHash *nssHash_Create(NSSArena *arenaOpt, PRUint32 numBuckets,
                                   PLHashFunction keyHash,
                                   PLHashComparator keyCompare,
                                   PLHashComparator valueCompare);

extern nssHash *nssHash_CreatePointer(NSSArena *arenaOpt,
                                          PRUint32 numBuckets);

extern nssHash *nssHash_CreateString(NSSArena *arenaOpt,
                                         PRUint32 numBuckets);

extern nssHash *nssHash_CreateItem(NSSArena *arenaOpt, PRUint32 numBuckets);





extern void nssHash_Destroy(nssHash *hash);






extern const NSSError NSS_ERROR_HASH_COLLISION;

extern PRStatus nssHash_Add(nssHash *hash, const void *key,
                                const void *value);





extern void nssHash_Remove(nssHash *hash, const void *it);





extern PRUint32 nssHash_Count(nssHash *hash);





extern PRBool nssHash_Exists(nssHash *hash, const void *it);





extern void *nssHash_Lookup(nssHash *hash, const void *it);





extern void nssHash_Iterate(nssHash *hash, nssHashIterator fcn,
                                void *closure);
# 1066 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern void *nsslibc_memcpy(void *dest, const void *source, PRUint32 n);

extern const NSSError NSS_ERROR_INVALID_POINTER;
# 1081 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern void *nsslibc_memset(void *dest, PRUint8 byte, PRUint32 n);

extern const NSSError NSS_ERROR_INVALID_POINTER;
# 1097 "/tmp/nss-3.79.0-build/nss-3.79/nss/lib/base/base.h"
extern PRBool nsslibc_memequal(const void *a, const void *b, PRUint32 len,
                                   PRStatus *statusOpt);

extern const NSSError NSS_ERROR_INVALID_POINTER;




# 49 "arena-orig.c" 2







# 1 "/usr/include/nspr/plarena.h" 1
# 17 "/usr/include/nspr/plarena.h"


typedef struct PLArena PLArena;

struct PLArena {
    PLArena *next;
    PRUword base;
    PRUword limit;
    PRUword avail;
};
# 49 "/usr/include/nspr/plarena.h"
typedef struct PLArenaPool PLArenaPool;

struct PLArenaPool {
    PLArena first;
    PLArena *current;
    PRUint32 arenasize;
    PRUword mask;



};
# 249 "/usr/include/nspr/plarena.h"
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





typedef size_t (*PLMallocSizeFn)(const void *ptr);





extern __attribute__((visibility("default"))) size_t PL_SizeOfArenaPoolExcludingPool(
    const PLArenaPool *pool, PLMallocSizeFn mallocSizeOf);
# 336 "/usr/include/nspr/plarena.h"

# 57 "arena-orig.c" 2

# 1 "/usr/include/string.h" 1 3 4
# 26 "/usr/include/string.h" 3 4
# 1 "/usr/include/bits/libc-header-start.h" 1 3 4
# 27 "/usr/include/string.h" 2 3 4






# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 1 3 4
# 34 "/usr/include/string.h" 2 3 4
# 43 "/usr/include/string.h" 3 4

# 43 "/usr/include/string.h" 3 4
extern void *memcpy (void *__restrict __dest, const void *__restrict __src,
       size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern void *memmove (void *__dest, const void *__src, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));





extern void *memccpy (void *__restrict __dest, const void *__restrict __src,
        int __c, size_t __n)
    __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__access__ (__write_only__, 1, 4)));




extern void *memset (void *__s, int __c, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));


extern int memcmp (const void *__s1, const void *__s2, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 80 "/usr/include/string.h" 3 4
extern int __memcmpeq (const void *__s1, const void *__s2, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 107 "/usr/include/string.h" 3 4
extern void *memchr (const void *__s, int __c, size_t __n)
      __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 141 "/usr/include/string.h" 3 4
extern char *strcpy (char *__restrict __dest, const char *__restrict __src)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncpy (char *__restrict __dest,
        const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern char *strcat (char *__restrict __dest, const char *__restrict __src)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncat (char *__restrict __dest, const char *__restrict __src,
        size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcmp (const char *__s1, const char *__s2)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern int strncmp (const char *__s1, const char *__s2, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcoll (const char *__s1, const char *__s2)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern size_t strxfrm (char *__restrict __dest,
         const char *__restrict __src, size_t __n)
    __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2))) __attribute__ ((__access__ (__write_only__, 1, 3)));



# 1 "/usr/include/bits/types/locale_t.h" 1 3 4
# 22 "/usr/include/bits/types/locale_t.h" 3 4
# 1 "/usr/include/bits/types/__locale_t.h" 1 3 4
# 27 "/usr/include/bits/types/__locale_t.h" 3 4
struct __locale_struct
{

  struct __locale_data *__locales[13];


  const unsigned short int *__ctype_b;
  const int *__ctype_tolower;
  const int *__ctype_toupper;


  const char *__names[13];
};

typedef struct __locale_struct *__locale_t;
# 23 "/usr/include/bits/types/locale_t.h" 2 3 4

typedef __locale_t locale_t;
# 173 "/usr/include/string.h" 2 3 4


extern int strcoll_l (const char *__s1, const char *__s2, locale_t __l)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 3)));


extern size_t strxfrm_l (char *__dest, const char *__src, size_t __n,
    locale_t __l) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 4)))
     __attribute__ ((__access__ (__write_only__, 1, 3)));





extern char *strdup (const char *__s)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));






extern char *strndup (const char *__string, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));
# 246 "/usr/include/string.h" 3 4
extern char *strchr (const char *__s, int __c)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 273 "/usr/include/string.h" 3 4
extern char *strrchr (const char *__s, int __c)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 286 "/usr/include/string.h" 3 4
extern char *strchrnul (const char *__s, int __c)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));





extern size_t strcspn (const char *__s, const char *__reject)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern size_t strspn (const char *__s, const char *__accept)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 323 "/usr/include/string.h" 3 4
extern char *strpbrk (const char *__s, const char *__accept)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 350 "/usr/include/string.h" 3 4
extern char *strstr (const char *__haystack, const char *__needle)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));




extern char *strtok (char *__restrict __s, const char *__restrict __delim)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));



extern char *__strtok_r (char *__restrict __s,
    const char *__restrict __delim,
    char **__restrict __save_ptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 3)));

extern char *strtok_r (char *__restrict __s, const char *__restrict __delim,
         char **__restrict __save_ptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 3)));
# 380 "/usr/include/string.h" 3 4
extern char *strcasestr (const char *__haystack, const char *__needle)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));







extern void *memmem (const void *__haystack, size_t __haystacklen,
       const void *__needle, size_t __needlelen)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 3)))
    __attribute__ ((__access__ (__read_only__, 1, 2)))
    __attribute__ ((__access__ (__read_only__, 3, 4)));



extern void *__mempcpy (void *__restrict __dest,
   const void *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern void *mempcpy (void *__restrict __dest,
        const void *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));




extern size_t strlen (const char *__s)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));




extern size_t strnlen (const char *__string, size_t __maxlen)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));




extern char *strerror (int __errnum) __attribute__ ((__nothrow__ , __leaf__));
# 432 "/usr/include/string.h" 3 4
extern int strerror_r (int __errnum, char *__buf, size_t __buflen) __asm__ ("" "__xpg_strerror_r") __attribute__ ((__nothrow__ , __leaf__))

                        __attribute__ ((__nonnull__ (2)))
    __attribute__ ((__access__ (__write_only__, 2, 3)));
# 458 "/usr/include/string.h" 3 4
extern char *strerror_l (int __errnum, locale_t __l) __attribute__ ((__nothrow__ , __leaf__));



# 1 "/usr/include/strings.h" 1 3 4
# 23 "/usr/include/strings.h" 3 4
# 1 "/usr/lib/gcc/x86_64-pc-linux-gnu/14/include/stddef.h" 1 3 4
# 24 "/usr/include/strings.h" 2 3 4










extern int bcmp (const void *__s1, const void *__s2, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern void bcopy (const void *__src, void *__dest, size_t __n)
  __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern void bzero (void *__s, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
# 68 "/usr/include/strings.h" 3 4
extern char *index (const char *__s, int __c)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 96 "/usr/include/strings.h" 3 4
extern char *rindex (const char *__s, int __c)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));






extern int ffs (int __i) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));





extern int ffsl (long int __l) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
__extension__ extern int ffsll (long long int __ll)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));



extern int strcasecmp (const char *__s1, const char *__s2)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strncasecmp (const char *__s1, const char *__s2, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));






extern int strcasecmp_l (const char *__s1, const char *__s2, locale_t __loc)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 3)));



extern int strncasecmp_l (const char *__s1, const char *__s2,
     size_t __n, locale_t __loc)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 4)));








# 1 "/usr/include/bits/strings_fortified.h" 1 3 4
# 22 "/usr/include/bits/strings_fortified.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void
__attribute__ ((__nothrow__ , __leaf__)) bcopy (const void *__src, void *__dest, size_t __len)
{
  (void) __builtin___memmove_chk (__dest, __src, __len,
      __builtin_object_size (__dest, 0));
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void
__attribute__ ((__nothrow__ , __leaf__)) bzero (void *__dest, size_t __len)
{
  (void) __builtin___memset_chk (__dest, '\0', __len,
     __builtin_object_size (__dest, 0));
}
# 145 "/usr/include/strings.h" 2 3 4
# 463 "/usr/include/string.h" 2 3 4



extern void explicit_bzero (void *__s, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)))
    __attribute__ ((__access__ (__write_only__, 1, 2)));



extern char *strsep (char **__restrict __stringp,
       const char *__restrict __delim)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));




extern char *strsignal (int __sig) __attribute__ ((__nothrow__ , __leaf__));
# 489 "/usr/include/string.h" 3 4
extern char *__stpcpy (char *__restrict __dest, const char *__restrict __src)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *stpcpy (char *__restrict __dest, const char *__restrict __src)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));



extern char *__stpncpy (char *__restrict __dest,
   const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *stpncpy (char *__restrict __dest,
        const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));




extern size_t strlcpy (char *__restrict __dest,
         const char *__restrict __src, size_t __n)
  __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__access__ (__write_only__, 1, 3)));



extern size_t strlcat (char *__restrict __dest,
         const char *__restrict __src, size_t __n)
  __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__access__ (__read_write__, 1, 3)));
# 548 "/usr/include/string.h" 3 4
# 1 "/usr/include/bits/string_fortified.h" 1 3 4
# 25 "/usr/include/bits/string_fortified.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void *
__attribute__ ((__nothrow__ , __leaf__)) memcpy (void *__restrict __dest, const void *__restrict __src, size_t __len)

{
  return __builtin___memcpy_chk (__dest, __src, __len,
     __builtin_object_size (__dest, 0));
}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void *
__attribute__ ((__nothrow__ , __leaf__)) memmove (void *__dest, const void *__src, size_t __len)
{
  return __builtin___memmove_chk (__dest, __src, __len,
      __builtin_object_size (__dest, 0));
}
# 56 "/usr/include/bits/string_fortified.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void *
__attribute__ ((__nothrow__ , __leaf__)) memset (void *__dest, int __ch, size_t __len)
{
  return __builtin___memset_chk (__dest, __ch, __len,
     __builtin_object_size (__dest, 0));
}




void __explicit_bzero_chk (void *__dest, size_t __len, size_t __destlen)
  __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__access__ (__write_only__, 1, 2)));

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void
__attribute__ ((__nothrow__ , __leaf__)) explicit_bzero (void *__dest, size_t __len)
{
  __explicit_bzero_chk (__dest, __len, __builtin_object_size (__dest, 0));
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__ , __leaf__)) strcpy (char * __restrict __dest, const char *__restrict __src)

    
{
  return __builtin___strcpy_chk (__dest, __src, __builtin_object_size (__dest, 2 > 1));
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__ , __leaf__)) stpcpy (char * __restrict __dest, const char *__restrict __src)

    
{
  return __builtin___stpcpy_chk (__dest, __src, __builtin_object_size (__dest, 2 > 1));
}



extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__ , __leaf__)) strncpy (char * __restrict __dest, const char *__restrict __src, size_t __len)

    
{
  return __builtin___strncpy_chk (__dest, __src, __len,
      __builtin_object_size (__dest, 2 > 1));
}



extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__ , __leaf__)) stpncpy (char * __dest, const char *__src, size_t __n)

    
{
  return __builtin___stpncpy_chk (__dest, __src, __n,
      __builtin_object_size (__dest, 2 > 1));
}
# 135 "/usr/include/bits/string_fortified.h" 3 4
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__ , __leaf__)) strcat (char * __restrict __dest, const char *__restrict __src)

    
{
  return __builtin___strcat_chk (__dest, __src, __builtin_object_size (__dest, 2 > 1));
}


extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__ , __leaf__)) strncat (char * __restrict __dest, const char *__restrict __src, size_t __len)

    
{
  return __builtin___strncat_chk (__dest, __src, __len,
      __builtin_object_size (__dest, 2 > 1));
}


extern size_t __strlcpy_chk (char *__dest, const char *__src, size_t __n,
        size_t __destlen) __attribute__ ((__nothrow__ , __leaf__));
extern size_t __strlcpy_alias (char *__dest, const char *__src, size_t __n) __asm__ ("" "strlcpy") __attribute__ ((__nothrow__ , __leaf__))

                 ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) size_t
__attribute__ ((__nothrow__ , __leaf__)) strlcpy (char * __restrict __dest, const char *__restrict __src, size_t __n)

    
{
  if (__builtin_object_size (__dest, 2 > 1) != (size_t) -1
      && (!__builtin_constant_p (__n > __builtin_object_size (__dest, 2 > 1))
   || __n > __builtin_object_size (__dest, 2 > 1)))
    return __strlcpy_chk (__dest, __src, __n, __builtin_object_size (__dest, 2 > 1));
  return __strlcpy_alias (__dest, __src, __n);
}

extern size_t __strlcat_chk (char *__dest, const char *__src, size_t __n,
        size_t __destlen) __attribute__ ((__nothrow__ , __leaf__));
extern size_t __strlcat_alias (char *__dest, const char *__src, size_t __n) __asm__ ("" "strlcat") __attribute__ ((__nothrow__ , __leaf__))

                 ;

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) size_t
__attribute__ ((__nothrow__ , __leaf__)) strlcat (char * __restrict __dest, const char *__restrict __src, size_t __n)

{
  if (__builtin_object_size (__dest, 2 > 1) != (size_t) -1
      && (!__builtin_constant_p (__n > __builtin_object_size (__dest, 2 > 1))
   || __n > __builtin_object_size (__dest, 2 > 1)))
    return __strlcat_chk (__dest, __src, __n, __builtin_object_size (__dest, 2 > 1));
  return __strlcat_alias (__dest, __src, __n);
}
# 549 "/usr/include/string.h" 2 3 4




# 59 "arena-orig.c" 2
# 89 "arena-orig.c"

# 89 "arena-orig.c"
struct NSSArenaStr {
  PLArenaPool pool;
  PRLock *lock;
# 101 "arena-orig.c"
};







struct nssArenaMarkStr {
  PRUint32 magic;
  void *mark;







};
# 379 "arena-orig.c"
 NSSArena *
NSSArena_Create
(
  void
)
{
  nss_ClearErrorStack();
  return nssArena_Create();
}
# 404 "arena-orig.c"
 NSSArena *
nssArena_Create
(
  void
)
{
  NSSArena *rv = (NSSArena *)
# 410 "arena-orig.c" 3 4
                            ((void *)0)
# 410 "arena-orig.c"
                                ;

  rv = ((NSSArena *)nss_ZAlloc(((NSSArena *)
# 412 "arena-orig.c" 3 4
      ((void *)0)
# 412 "arena-orig.c"
      ), sizeof(NSSArena)));
  if( (NSSArena *)
# 413 "arena-orig.c" 3 4
                 ((void *)0) 
# 413 "arena-orig.c"
                      == rv ) {
    nss_SetError(NSS_ERROR_NO_MEMORY);
    return (NSSArena *)
# 415 "arena-orig.c" 3 4
                      ((void *)0)
# 415 "arena-orig.c"
                          ;
  }

  rv->lock = PR_NewLock();
  if( (PRLock *)
# 419 "arena-orig.c" 3 4
               ((void *)0) 
# 419 "arena-orig.c"
                    == rv->lock ) {
    (void)nss_ZFreeIf(rv);
    nss_SetError(NSS_ERROR_NO_MEMORY);
    return (NSSArena *)
# 422 "arena-orig.c" 3 4
                      ((void *)0)
# 422 "arena-orig.c"
                          ;
  }
# 442 "arena-orig.c"
  PL_InitArenaPool(&rv->pool, "NSS", 2048, sizeof(double));
# 457 "arena-orig.c"
  return rv;
}
# 476 "arena-orig.c"
 PRStatus
NSSArena_Destroy
(
  NSSArena *arena
)
{
  nss_ClearErrorStack();







  return nssArena_Destroy(arena);
}
# 509 "arena-orig.c"
 PRStatus
nssArena_Destroy
(
  NSSArena *arena
)
{
  PRLock *lock;







  if( (PRLock *)
# 523 "arena-orig.c" 3 4
               ((void *)0) 
# 523 "arena-orig.c"
                    == arena->lock ) {

    nss_SetError(NSS_ERROR_INVALID_ARENA);
    return PR_FAILURE;
  }
  PR_Lock(arena->lock);
# 542 "arena-orig.c"
  PL_FinishArenaPool(&arena->pool);
  lock = arena->lock;
  arena->lock = (PRLock *)
# 544 "arena-orig.c" 3 4
                         ((void *)0)
# 544 "arena-orig.c"
                             ;
  PR_Unlock(lock);
  PR_DestroyLock(lock);
  (void)nss_ZFreeIf(arena);
  return PR_SUCCESS;
}

static void *nss_zalloc_arena_locked(NSSArena *arena, PRUint32 size);
# 574 "arena-orig.c"
 nssArenaMark *
nssArena_Mark
(
  NSSArena *arena
)
{
  nssArenaMark *rv;
  void *p;







  if( (PRLock *)
# 589 "arena-orig.c" 3 4
               ((void *)0) 
# 589 "arena-orig.c"
                    == arena->lock ) {

    nss_SetError(NSS_ERROR_INVALID_ARENA);
    return (nssArenaMark *)
# 592 "arena-orig.c" 3 4
                          ((void *)0)
# 592 "arena-orig.c"
                              ;
  }
  PR_Lock(arena->lock);
# 611 "arena-orig.c"
  p = ((void *) (&arena->pool)->current->avail);



  rv = (nssArenaMark *)nss_zalloc_arena_locked(arena, sizeof(nssArenaMark));
  if( (nssArenaMark *)
# 616 "arena-orig.c" 3 4
                     ((void *)0) 
# 616 "arena-orig.c"
                          == rv ) {
    PR_Unlock(arena->lock);
    nss_SetError(NSS_ERROR_NO_MEMORY);
    return (nssArenaMark *)
# 619 "arena-orig.c" 3 4
                          ((void *)0)
# 619 "arena-orig.c"
                              ;
  }
# 632 "arena-orig.c"
  rv->mark = p;
  rv->magic = 0x4d41524b;





  PR_Unlock(arena->lock);

  return rv;
}
# 651 "arena-orig.c"
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

  if( (PRLock *)
# 672 "arena-orig.c" 3 4
               ((void *)0) 
# 672 "arena-orig.c"
                    == arena->lock ) {

    nss_SetError(NSS_ERROR_INVALID_ARENA);
    return PR_FAILURE;
  }
  PR_Lock(arena->lock);
# 689 "arena-orig.c"
  if( 0x4d41524b != arenaMark->magic ) {

    PR_Unlock(arena->lock);
    nss_SetError(NSS_ERROR_INVALID_ARENA_MARK);
    return PR_FAILURE;
  }

  arenaMark->magic = 0;
  inner_mark = arenaMark->mark;
# 729 "arena-orig.c"
  if( release ) {
# 740 "arena-orig.c"
    PR_ARENA_RELEASE(&arena->pool, inner_mark);

  }

  PR_Unlock(arena->lock);
  return PR_SUCCESS;
}
# 767 "arena-orig.c"
 PRStatus
nssArena_Release
(
  NSSArena *arena,
  nssArenaMark *arenaMark
)
{
  return nss_arena_unmark_release(arena, arenaMark, 1);
}
# 799 "arena-orig.c"
 PRStatus
nssArena_Unmark
(
  NSSArena *arena,
  nssArenaMark *arenaMark
)
{
  return nss_arena_unmark_release(arena, arenaMark, 0);
}
# 817 "arena-orig.c"
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
  PR_ARENA_ALLOCATE(p, &arena->pool, my_size);
  if( (void *)
# 834 "arena-orig.c" 3 4
             ((void *)0) 
# 834 "arena-orig.c"
                  == p ) {
    nss_SetError(NSS_ERROR_NO_MEMORY);
    return (void *)
# 836 "arena-orig.c" 3 4
                  ((void *)0)
# 836 "arena-orig.c"
                      ;
  }





  h = (struct pointer_header *)p;
  h->arena = arena;
  h->size = size;
  rv = (void *)((char *)h + sizeof(struct pointer_header));
  (void)nsslibc_memset(rv, 0, size);
  return rv;
}
# 874 "arena-orig.c"
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
    return (void *)
# 887 "arena-orig.c" 3 4
                  ((void *)0)
# 887 "arena-orig.c"
                      ;
  }

  if( (NSSArena *)
# 890 "arena-orig.c" 3 4
                 ((void *)0) 
# 890 "arena-orig.c"
                      == arenaOpt ) {

    h = (struct pointer_header *)PR_Calloc(1, my_size);
    if( (struct pointer_header *)
# 893 "arena-orig.c" 3 4
                                ((void *)0) 
# 893 "arena-orig.c"
                                     == h ) {
      nss_SetError(NSS_ERROR_NO_MEMORY);
      return (void *)
# 895 "arena-orig.c" 3 4
                    ((void *)0)
# 895 "arena-orig.c"
                        ;
    }

    h->arena = (NSSArena *)
# 898 "arena-orig.c" 3 4
                          ((void *)0)
# 898 "arena-orig.c"
                              ;
    h->size = size;


    return (void *)((char *)h + sizeof(struct pointer_header));
  } else {
    void *rv;







    if( (PRLock *)
# 912 "arena-orig.c" 3 4
                 ((void *)0) 
# 912 "arena-orig.c"
                      == arenaOpt->lock ) {

      nss_SetError(NSS_ERROR_INVALID_ARENA);
      return (void *)
# 915 "arena-orig.c" 3 4
                    ((void *)0)
# 915 "arena-orig.c"
                        ;
    }
    PR_Lock(arenaOpt->lock);
# 929 "arena-orig.c"
    rv = nss_zalloc_arena_locked(arenaOpt, size);

    PR_Unlock(arenaOpt->lock);
    return rv;
  }

}
# 955 "arena-orig.c"
 PRStatus
nss_ZFreeIf
(
  void *pointer
)
{
  struct pointer_header *h;

  if( (void *)
# 963 "arena-orig.c" 3 4
             ((void *)0) 
# 963 "arena-orig.c"
                  == pointer ) {
    return PR_SUCCESS;
  }

  h = (struct pointer_header *)((char *)pointer
    - sizeof(struct pointer_header));



  if( (NSSArena *)
# 972 "arena-orig.c" 3 4
                 ((void *)0) 
# 972 "arena-orig.c"
                      == h->arena ) {

    (void)nsslibc_memset(pointer, 0, h->size);
    PR_Free(h);
    return PR_SUCCESS;
  } else {







    if( (PRLock *)
# 985 "arena-orig.c" 3 4
                 ((void *)0) 
# 985 "arena-orig.c"
                      == h->arena->lock ) {

      nss_SetError(NSS_ERROR_INVALID_POINTER);
      return PR_FAILURE;
    }
    PR_Lock(h->arena->lock);

    (void)nsslibc_memset(pointer, 0, h->size);



    PR_Unlock(h->arena->lock);
    return PR_SUCCESS;
  }

}
# 1022 "arena-orig.c"
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
    return (void *)
# 1037 "arena-orig.c" 3 4
                  ((void *)0)
# 1037 "arena-orig.c"
                      ;
  }

  if( (void *)
# 1040 "arena-orig.c" 3 4
             ((void *)0) 
# 1040 "arena-orig.c"
                  == pointer ) {
    nss_SetError(NSS_ERROR_INVALID_POINTER);
    return (void *)
# 1042 "arena-orig.c" 3 4
                  ((void *)0)
# 1042 "arena-orig.c"
                      ;
  }

  h = (struct pointer_header *)((char *)pointer
    - sizeof(struct pointer_header));



  if( newSize == h->size ) {

    return pointer;
  }

  arena = h->arena;
  if (!arena) {

    new_h = (struct pointer_header *)PR_Calloc(1, my_newSize);
    if( (struct pointer_header *)
# 1059 "arena-orig.c" 3 4
                                ((void *)0) 
# 1059 "arena-orig.c"
                                     == new_h ) {
      nss_SetError(NSS_ERROR_NO_MEMORY);
      return (void *)
# 1061 "arena-orig.c" 3 4
                    ((void *)0)
# 1061 "arena-orig.c"
                        ;
    }

    new_h->arena = (NSSArena *)
# 1064 "arena-orig.c" 3 4
                              ((void *)0)
# 1064 "arena-orig.c"
                                  ;
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
      return (void *)
# 1093 "arena-orig.c" 3 4
                    ((void *)0)
# 1093 "arena-orig.c"
                        ;
    }
    PR_Lock(arena->lock);
# 1107 "arena-orig.c"
    if( newSize < h->size ) {
# 1120 "arena-orig.c"
      char *extra = &((char *)pointer)[ newSize ];
      (void)nsslibc_memset(extra, 0, (h->size - newSize));
      PR_Unlock(arena->lock);
      return pointer;
    }

    PR_ARENA_ALLOCATE(p, &arena->pool, my_newSize);
    if( (void *)
# 1127 "arena-orig.c" 3 4
               ((void *)0) 
# 1127 "arena-orig.c"
                    == p ) {
      PR_Unlock(arena->lock);
      nss_SetError(NSS_ERROR_NO_MEMORY);
      return (void *)
# 1130 "arena-orig.c" 3 4
                    ((void *)0)
# 1130 "arena-orig.c"
                        ;
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
    h->arena = (NSSArena *)
# 1142 "arena-orig.c" 3 4
                          ((void *)0)
# 1142 "arena-orig.c"
                              ;
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
