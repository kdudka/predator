#include <verifier-builtins.h>

void *malloc(int);
void *calloc(int, int);
void free(void *);

static void *mallocnull(unsigned size)
{
    return calloc(1, size);
}

void *get_nondet_ptr(void)
{
    void *a;
    return a;
}

int get_nondet_int(void)
{
    int a;
    return a;
}

typedef unsigned long __kernel_ino_t;
typedef unsigned short __kernel_mode_t;
typedef unsigned short __kernel_nlink_t;
typedef long __kernel_off_t;
typedef int __kernel_pid_t;
typedef unsigned int __kernel_size_t;
typedef int __kernel_ssize_t;
typedef long __kernel_time_t;
typedef long __kernel_clock_t;
typedef int __kernel_timer_t;
typedef int __kernel_clockid_t;
typedef unsigned int __kernel_uid32_t;
typedef unsigned int __kernel_gid32_t;
typedef long long __kernel_loff_t;
typedef unsigned short umode_t;
typedef signed char __s8;
typedef unsigned char __u8;
typedef short __s16;
typedef unsigned short __u16;
typedef int __s32;
typedef unsigned int __u32;
typedef long long __s64;
typedef unsigned long long __u64;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef long long s64;
typedef unsigned long long u64;
typedef __u32 __kernel_dev_t;
typedef __kernel_dev_t dev_t;
typedef __kernel_ino_t ino_t;
typedef __kernel_mode_t mode_t;
typedef __kernel_nlink_t nlink_t;
typedef __kernel_off_t off_t;
typedef __kernel_pid_t pid_t;
typedef __kernel_timer_t timer_t;
typedef __kernel_clockid_t clockid_t;
typedef __kernel_uid32_t uid_t;
typedef __kernel_gid32_t gid_t;
typedef __kernel_loff_t loff_t;
typedef __kernel_size_t size_t;
typedef __kernel_ssize_t ssize_t;
typedef __kernel_time_t time_t;
typedef __kernel_clock_t clock_t;
typedef unsigned char u_char;
typedef unsigned long sector_t;
typedef unsigned int gfp_t;
typedef unsigned long mm_segment_t;
typedef unsigned long pgd_t[2];
typedef unsigned long pgprot_t;
typedef __u32 Elf32_Addr;
typedef __u16 Elf32_Half;
typedef __u32 Elf32_Word;
typedef __kernel_uid32_t qid_t;
typedef __u64 qsize_t;
typedef union __anonunion_dvd_struct_46 dvd_struct;
typedef __u8 dvd_key[5];
typedef __u8 dvd_challenge[10];

struct thread_info;
struct task_struct;
struct siginfo;
struct mm_struct;
struct bio;
struct anon_vma;
struct exec_domain;
struct completion;
struct device_driver;
struct nameidata;
struct vfsmount;
struct dentry;
struct kmem_cache;
struct page;
struct ctl_table;
struct file;
struct vm_area_struct;
struct exec_domain;
struct timer_base_s;
struct hrtimer_base;
struct namespace;
struct kioctx;
struct tty_struct;
struct key;
struct backing_dev_info;
struct reclaim_state;
struct io_context;
struct audit_context;
struct linux_binfmt;
struct files_struct;
struct key;
struct kobject;
struct module;
struct hd_geometry;
struct iovec;
struct nameidata;
struct kiocb;
struct pipe_inode_info;
struct poll_table_struct;
struct kstatfs;
struct vm_area_struct;
struct vfsmount;
struct prio_tree_node;
struct radix_tree_node;
struct dentry_operations;
struct super_block;
struct dcookie_struct;
struct inode;
struct module_param_attrs;
struct exception_table_entry;
struct module;
struct subsystem;
struct kset;
struct kobj_type;
struct vm_area_struct;
struct iattr;
struct file_system_type;
struct super_operations;
struct export_operations;
struct xattr_handler;
struct fasync_struct;
struct nfs4_lock_state;
struct nlm_lockowner;
struct inode_operations;
struct file_operations;
struct file_lock;
struct cdev;
struct dnotify_struct;
struct hd_struct;
struct gendisk;
struct backing_dev_info;
struct page;
struct address_space;
struct writeback_control;
struct quota_format_type;
struct request_sense;
struct page;
struct inode;
struct vm_operations_struct;
struct seq_file;
struct address_space;
struct proc_dir_entry;
struct file;
struct completion;
struct cdrom_device_ops;
struct dma_coherent_mem;
struct device;
struct device_driver;
struct bus_attribute;
struct device_attribute;
struct driver_attribute;
struct klist_node;
struct request_pm_state;
struct bio;
struct request_queue;
struct timer_rand_state;
struct work_struct;
struct bio_vec;
struct request;
struct cfq_queue;

struct restart_block {
    long (*fn)(struct restart_block * ) ;
    unsigned long arg0 ;
    unsigned long arg1 ;
    unsigned long arg2 ;
    unsigned long arg3 ;
};
struct vfp_hard_struct {
    __u64 fpregs[16] ;
    __u32 fpmx_state ;
    __u32 fpexc ;
    __u32 fpscr ;
    __u32 fpinst ;
    __u32 fpinst2 ;
};
union vfp_state {
    struct vfp_hard_struct hard ;
};
struct fp_hard_struct {
    unsigned int save[35] ;
};
struct fp_soft_struct {
    unsigned int save[35] ;
};
union fp_state {
    struct fp_hard_struct hard ;
    struct fp_soft_struct soft ;
};
struct cpu_context_save {
    __u32 r4 ;
    __u32 r5 ;
    __u32 r6 ;
    __u32 r7 ;
    __u32 r8 ;
    __u32 r9 ;
    __u32 sl ;
    __u32 fp ;
    __u32 sp ;
    __u32 pc ;
    __u32 extra[2] ;
};
struct thread_info {
    unsigned long flags ;
    int preempt_count ;
    mm_segment_t addr_limit ;
    struct task_struct *task ;
    struct exec_domain *exec_domain ;
    __u32 cpu ;
    __u32 cpu_domain ;
    struct cpu_context_save cpu_context ;
    __u8 used_cp[16] ;
    unsigned long tp_value ;
    union fp_state fpstate __attribute__((__aligned__(8))) ;
    union vfp_state vfpstate ;
    struct restart_block restart_block ;
};
struct __anonstruct_raw_spinlock_t_3 {
};
typedef struct __anonstruct_raw_spinlock_t_3 raw_spinlock_t;
struct __anonstruct_raw_rwlock_t_4 {
};
typedef struct __anonstruct_raw_rwlock_t_4 raw_rwlock_t;
struct __anonstruct_spinlock_t_5 {
    raw_spinlock_t raw_lock ;
};
typedef struct __anonstruct_spinlock_t_5 spinlock_t;
struct __anonstruct_rwlock_t_6 {
    raw_rwlock_t raw_lock ;
};
typedef struct __anonstruct_rwlock_t_6 rwlock_t;
struct __anonstruct_atomic_t_7 {
    int counter ;
};
typedef struct __anonstruct_atomic_t_7 atomic_t;
typedef __u32 kernel_cap_t;
struct timespec {
    time_t tv_sec ;
    long tv_nsec ;
};
struct rb_node {
    struct rb_node *rb_parent ;
    int rb_color ;
    struct rb_node *rb_right ;
    struct rb_node *rb_left ;
};
struct rb_root {
    struct rb_node *rb_node ;
};
struct __anonstruct_cpumask_t_9 {
    unsigned long bits[1] ;
};
typedef struct __anonstruct_cpumask_t_9 cpumask_t;
union debug_insn {
    u32 arm ;
    u16 thumb ;
};
struct debug_entry {
    u32 address ;
    union debug_insn insn ;
};
struct debug_info {
    int nsaved ;
    struct debug_entry bp[2] ;
};
struct thread_struct {
    unsigned long address ;
    unsigned long trap_no ;
    unsigned long error_code ;
    struct debug_info debug ;
};
struct list_head {
    struct list_head *next ;
    struct list_head *prev ;
};
struct hlist_node;
struct hlist_head {
    struct hlist_node *first ;
};
struct hlist_node {
    struct hlist_node *next ;
    struct hlist_node **pprev ;
};
struct __wait_queue;
typedef struct __wait_queue wait_queue_t;
struct __wait_queue {
    unsigned int flags ;
    void *private ;
    int (*func)(wait_queue_t *wait , unsigned int mode , int sync , void *key ) ;
    struct list_head task_list ;
};
struct __wait_queue_head {
    spinlock_t lock ;
    struct list_head task_list ;
};
typedef struct __wait_queue_head wait_queue_head_t;
struct rw_semaphore {
    __s32 activity ;
    spinlock_t wait_lock ;
    struct list_head wait_list ;
};
struct semaphore {
    atomic_t count ;
    int sleepers ;
    wait_queue_head_t wait ;
};
struct __anonstruct_mm_context_t_11 {
};
typedef struct __anonstruct_mm_context_t_11 mm_context_t;
typedef unsigned long cputime_t;
struct sem_undo;
struct sem_undo {
    struct sem_undo *proc_next ;
    struct sem_undo *id_next ;
    int semid ;
    short *semadj ;
};
struct sem_undo_list {
    atomic_t refcnt ;
    spinlock_t lock ;
    struct sem_undo *proc_list ;
};
struct sysv_sem {
    struct sem_undo_list *undo_list ;
};
struct __anonstruct_sigset_t_12 {
    unsigned long sig[2] ;
};
typedef struct __anonstruct_sigset_t_12 sigset_t;
typedef void __signalfn_t(int);
typedef __signalfn_t *__sighandler_t;
typedef void __restorefn_t(void);
typedef __restorefn_t *__sigrestore_t;
struct sigaction {
    __sighandler_t sa_handler ;
    unsigned long sa_flags ;
    __sigrestore_t sa_restorer ;
    sigset_t sa_mask ;
};
struct k_sigaction {
    struct sigaction sa ;
};
union sigval {
    int sival_int ;
    void *sival_ptr ;
};
typedef union sigval sigval_t;
struct __anonstruct__kill_14 {
    pid_t _pid ;
    uid_t _uid ;
};
struct __anonstruct__timer_15 {
    timer_t _tid ;
    int _overrun ;
    char _pad[(int )(sizeof(uid_t ) - sizeof(int ))] ;
    sigval_t _sigval ;
    int _sys_private ;
};
struct __anonstruct__rt_16 {
    pid_t _pid ;
    uid_t _uid ;
    sigval_t _sigval ;
};
struct __anonstruct__sigchld_17 {
    pid_t _pid ;
    uid_t _uid ;
    int _status ;
    clock_t _utime ;
    clock_t _stime ;
};
struct __anonstruct__sigfault_18 {
    void *_addr ;
};
struct __anonstruct__sigpoll_19 {
    long _band ;
    int _fd ;
};
union __anonunion__sifields_13 {
    int _pad[(int )((128U - 3U * sizeof(int )) / sizeof(int ))] ;
    struct __anonstruct__kill_14 _kill ;
    struct __anonstruct__timer_15 _timer ;
    struct __anonstruct__rt_16 _rt ;
    struct __anonstruct__sigchld_17 _sigchld ;
    struct __anonstruct__sigfault_18 _sigfault ;
    struct __anonstruct__sigpoll_19 _sigpoll ;
};
struct siginfo {
    int si_signo ;
    int si_errno ;
    int si_code ;
    union __anonunion__sifields_13 _sifields ;
};
typedef struct siginfo siginfo_t;
struct user_struct;
struct sigpending {
    struct list_head list ;
    sigset_t signal ;
};
struct fs_struct {
    atomic_t count ;
    rwlock_t lock ;
    int umask ;
    struct dentry *root ;
    struct dentry *pwd ;
    struct dentry *altroot ;
    struct vfsmount *rootmnt ;
    struct vfsmount *pwdmnt ;
    struct vfsmount *altrootmnt ;
};
struct completion {
    unsigned int done ;
    wait_queue_head_t wait ;
};
struct pid {
    int nr ;
    struct hlist_node pid_chain ;
    struct list_head pid_list ;
};
typedef struct kmem_cache kmem_cache_t;
struct cache_sizes {
    size_t cs_size ;
    kmem_cache_t *cs_cachep ;
    kmem_cache_t *cs_dmacachep ;
};
struct __anonstruct_seccomp_t_22 {

};
typedef struct __anonstruct_seccomp_t_22 seccomp_t;
struct rcu_head {
    struct rcu_head *next ;
    void (*func)(struct rcu_head *head ) ;
};
struct rlimit {
    unsigned long rlim_cur ;
    unsigned long rlim_max ;
};
struct timer_list {
    struct list_head entry ;
    unsigned long expires ;
    void (*function)(unsigned long ) ;
    unsigned long data ;
    struct timer_base_s *base ;
};
struct __anonstruct_tv_24 {
    s32 nsec ;
    s32 sec ;
};
union __anonunion_ktime_t_23 {
    s64 tv64 ;
    struct __anonstruct_tv_24 tv ;
};
typedef union __anonunion_ktime_t_23 ktime_t;
enum hrtimer_state {
    HRTIMER_INACTIVE = 0,
    HRTIMER_EXPIRED = 1,
    HRTIMER_RUNNING = 2,
    HRTIMER_PENDING = 3
} ;
struct hrtimer {
    struct rb_node node ;
    ktime_t expires ;
    enum hrtimer_state state ;
    int (*function)(void * ) ;
    void *data ;
    struct hrtimer_base *base ;
};
struct hrtimer_base {
    clockid_t index ;
    spinlock_t lock ;
    struct rb_root active ;
    struct rb_node *first ;
    ktime_t resolution ;
    ktime_t (*get_time)(void) ;
    struct hrtimer *curr_timer ;
};
typedef struct task_struct task_t;
struct work_struct {
    unsigned long pending ;
    struct list_head entry ;
    void (*func)(void * ) ;
    void *data ;
    void *wq_data ;
    struct timer_list timer ;
};
struct io_event {
    __u64 data ;
    __u64 obj ;
    __s64 res ;
    __s64 res2 ;
};
union __anonunion_ki_obj_26 {
    void *user ;
    struct task_struct *tsk ;
};
struct kiocb {
    struct list_head ki_run_list ;
    long ki_flags ;
    int ki_users ;
    unsigned int ki_key ;
    struct file *ki_filp ;
    struct kioctx *ki_ctx ;
    int (*ki_cancel)(struct kiocb * , struct io_event * ) ;
    ssize_t (*ki_retry)(struct kiocb * ) ;
    void (*ki_dtor)(struct kiocb * ) ;
    union __anonunion_ki_obj_26 ki_obj ;
    __u64 ki_user_data ;
    wait_queue_t ki_wait ;
    loff_t ki_pos ;
    void *private ;
    unsigned short ki_opcode ;
    size_t ki_nbytes ;
    char *ki_buf ;
    size_t ki_left ;
    long ki_retried ;
    long ki_kicked ;
    long ki_queued ;
    struct list_head ki_list ;
};
struct aio_ring_info {
    unsigned long mmap_base ;
    unsigned long mmap_size ;
    struct page **ring_pages ;
    spinlock_t ring_lock ;
    long nr_pages ;
    unsigned int nr ;
    unsigned int tail ;
    struct page *internal_pages[8] ;
};
struct kioctx {
    atomic_t users ;
    int dead ;
    struct mm_struct *mm ;
    unsigned long user_id ;
    struct kioctx *next ;
    wait_queue_head_t wait ;
    spinlock_t ctx_lock ;
    int reqs_active ;
    struct list_head active_reqs ;
    struct list_head run_list ;
    unsigned int max_reqs ;
    struct aio_ring_info ring_info ;
    struct work_struct wq ;
};
typedef unsigned long mm_counter_t;
struct mm_struct {
    struct vm_area_struct *mmap ;
    struct rb_root mm_rb ;
    struct vm_area_struct *mmap_cache ;
    unsigned long (*get_unmapped_area)(struct file *filp , unsigned long addr , unsigned long len ,
            unsigned long pgoff , unsigned long flags ) ;
    void (*unmap_area)(struct mm_struct *mm , unsigned long addr ) ;
    unsigned long mmap_base ;
    unsigned long task_size ;
    unsigned long cached_hole_size ;
    unsigned long free_area_cache ;
    pgd_t *pgd ;
    atomic_t mm_users ;
    atomic_t mm_count ;
    int map_count ;
    struct rw_semaphore mmap_sem ;
    spinlock_t page_table_lock ;
    struct list_head mmlist ;
    mm_counter_t _file_rss ;
    mm_counter_t _anon_rss ;
    unsigned long hiwater_rss ;
    unsigned long hiwater_vm ;
    unsigned long total_vm ;
    unsigned long locked_vm ;
    unsigned long shared_vm ;
    unsigned long exec_vm ;
    unsigned long stack_vm ;
    unsigned long reserved_vm ;
    unsigned long def_flags ;
    unsigned long nr_ptes ;
    unsigned long start_code ;
    unsigned long end_code ;
    unsigned long start_data ;
    unsigned long end_data ;
    unsigned long start_brk ;
    unsigned long brk ;
    unsigned long start_stack ;
    unsigned long arg_start ;
    unsigned long arg_end ;
    unsigned long env_start ;
    unsigned long env_end ;
    unsigned long saved_auxv[44] ;
    unsigned int dumpable : 2 ;
    cpumask_t cpu_vm_mask ;
    mm_context_t context ;
    unsigned long swap_token_time ;
    char recent_pagein ;
    int core_waiters ;
    struct completion *core_startup_done ;
    struct completion core_done ;
    rwlock_t ioctx_list_lock ;
    struct kioctx *ioctx_list ;
};
struct sighand_struct {
    atomic_t count ;
    struct k_sigaction action[64] ;
    spinlock_t siglock ;
    struct rcu_head rcu ;
};
struct signal_struct {
    atomic_t count ;
    atomic_t live ;
    wait_queue_head_t wait_chldexit ;
    task_t *curr_target ;
    struct sigpending shared_pending ;
    int group_exit_code ;
    struct task_struct *group_exit_task ;
    int notify_count ;
    int group_stop_count ;
    unsigned int flags ;
    struct list_head posix_timers ;
    struct hrtimer real_timer ;
    ktime_t it_real_incr ;
    cputime_t it_prof_expires ;
    cputime_t it_virt_expires ;
    cputime_t it_prof_incr ;
    cputime_t it_virt_incr ;
    pid_t pgrp ;
    pid_t tty_old_pgrp ;
    pid_t session ;
    int leader ;
    struct tty_struct *tty ;
    cputime_t utime ;
    cputime_t stime ;
    cputime_t cutime ;
    cputime_t cstime ;
    unsigned long nvcsw ;
    unsigned long nivcsw ;
    unsigned long cnvcsw ;
    unsigned long cnivcsw ;
    unsigned long min_flt ;
    unsigned long maj_flt ;
    unsigned long cmin_flt ;
    unsigned long cmaj_flt ;
    unsigned long long sched_time ;
    struct rlimit rlim[15] ;
    struct list_head cpu_timers[3] ;
    struct key *session_keyring ;
    struct key *process_keyring ;
};
struct user_struct {
    atomic_t __count ;
    atomic_t processes ;
    atomic_t files ;
    atomic_t sigpending ;
    atomic_t inotify_watches ;
    atomic_t inotify_devs ;
    unsigned long mq_bytes ;
    unsigned long locked_shm ;
    struct key *uid_keyring ;
    struct key *session_keyring ;
    struct list_head uidhash_list ;
    uid_t uid ;
};
struct prio_array;
typedef struct prio_array prio_array_t;
struct group_info {
    int ngroups ;
    atomic_t usage ;
    gid_t small_block[32] ;
    int nblocks ;
    gid_t *blocks[0] ;
};
struct task_struct {
    long state ;
    struct thread_info *thread_info ;
    atomic_t usage ;
    unsigned long flags ;
    unsigned long ptrace ;
    int lock_depth ;
    int prio ;
    int static_prio ;
    struct list_head run_list ;
    prio_array_t *array ;
    unsigned short ioprio ;
    unsigned long sleep_avg ;
    unsigned long long timestamp ;
    unsigned long long last_ran ;
    unsigned long long sched_time ;
    int activated ;
    unsigned long policy ;
    cpumask_t cpus_allowed ;
    unsigned int time_slice ;
    unsigned int first_time_slice ;
    struct list_head tasks ;
    struct list_head ptrace_children ;
    struct list_head ptrace_list ;
    struct mm_struct *mm ;
    struct mm_struct *active_mm ;
    struct linux_binfmt *binfmt ;
    long exit_state ;
    int exit_code ;
    int exit_signal ;
    int pdeath_signal ;
    unsigned long personality ;
    unsigned int did_exec : 1 ;
    pid_t pid ;
    pid_t tgid ;
    struct task_struct *real_parent ;
    struct task_struct *parent ;
    struct list_head children ;
    struct list_head sibling ;
    struct task_struct *group_leader ;
    struct pid pids[4] ;
    struct completion *vfork_done ;
    int *set_child_tid ;
    int *clear_child_tid ;
    unsigned long rt_priority ;
    cputime_t utime ;
    cputime_t stime ;
    unsigned long nvcsw ;
    unsigned long nivcsw ;
    struct timespec start_time ;
    unsigned long min_flt ;
    unsigned long maj_flt ;
    cputime_t it_prof_expires ;
    cputime_t it_virt_expires ;
    unsigned long long it_sched_expires ;
    struct list_head cpu_timers[3] ;
    uid_t uid ;
    uid_t euid ;
    uid_t suid ;
    uid_t fsuid ;
    gid_t gid ;
    gid_t egid ;
    gid_t sgid ;
    gid_t fsgid ;
    struct group_info *group_info ;
    kernel_cap_t cap_effective ;
    kernel_cap_t cap_inheritable ;
    kernel_cap_t cap_permitted ;
    unsigned int keep_capabilities : 1 ;
    struct user_struct *user ;
    struct key *request_key_auth ;
    struct key *thread_keyring ;
    unsigned char jit_keyring ;
    int oomkilladj ;
    char comm[16] ;
    int link_count ;
    int total_link_count ;
    struct sysv_sem sysvsem ;
    struct thread_struct thread ;
    struct fs_struct *fs ;
    struct files_struct *files ;
    struct namespace *namespace ;
    struct signal_struct *signal ;
    struct sighand_struct *sighand ;
    sigset_t blocked ;
    sigset_t real_blocked ;
    sigset_t saved_sigmask ;
    struct sigpending pending ;
    unsigned long sas_ss_sp ;
    size_t sas_ss_size ;
    int (*notifier)(void *priv ) ;
    void *notifier_data ;
    sigset_t *notifier_mask ;
    void *security ;
    struct audit_context *audit_context ;
    seccomp_t seccomp ;
    u32 parent_exec_id ;
    u32 self_exec_id ;
    spinlock_t alloc_lock ;
    spinlock_t proc_lock ;
    void *journal_info ;
    struct reclaim_state *reclaim_state ;
    struct dentry *proc_dentry ;
    struct backing_dev_info *backing_dev_info ;
    struct io_context *io_context ;
    unsigned long ptrace_message ;
    siginfo_t *last_siginfo ;
    wait_queue_t *io_wait ;
    u64 rchar ;
    u64 wchar ;
    u64 syscr ;
    u64 syscw ;
    u64 acct_rss_mem1 ;
    u64 acct_vm_mem1 ;
    clock_t acct_stimexpd ;
    atomic_t fs_excl ;
    struct rcu_head rcu ;
};
struct kstat {
    unsigned long ino ;
    dev_t dev ;
    umode_t mode ;
    unsigned int nlink ;
    uid_t uid ;
    gid_t gid ;
    dev_t rdev ;
    loff_t size ;
    struct timespec atime ;
    struct timespec mtime ;
    struct timespec ctime ;
    unsigned long blksize ;
    unsigned long blocks ;
};
struct elf32_sym {
    Elf32_Word st_name ;
    Elf32_Addr st_value ;
    Elf32_Word st_size ;
    unsigned char st_info ;
    unsigned char st_other ;
    Elf32_Half st_shndx ;
};
typedef struct elf32_sym Elf32_Sym;
struct attribute {
    char *name ;
    struct module *owner ;
    mode_t mode ;
};
struct attribute_group {
    char *name ;
    struct attribute **attrs ;
};
struct sysfs_ops {
    ssize_t (*show)(struct kobject * , struct attribute * , char * ) ;
    ssize_t (*store)(struct kobject * , struct attribute * , char * , size_t ) ;
};
struct kref {
    atomic_t refcount ;
};
struct kobject {
    char *k_name ;
    char name[20] ;
    struct kref kref ;
    struct list_head entry ;
    struct kobject *parent ;
    struct kset *kset ;
    struct kobj_type *ktype ;
    struct dentry *dentry ;
};
struct kobj_type {
    void (*release)(struct kobject * ) ;
    struct sysfs_ops *sysfs_ops ;
    struct attribute **default_attrs ;
};
struct kset_uevent_ops {
    int (*filter)(struct kset *kset , struct kobject *kobj ) ;
    char *(*name)(struct kset *kset , struct kobject *kobj ) ;
    int (*uevent)(struct kset *kset , struct kobject *kobj , char **envp , int num_envp ,
            char *buffer , int buffer_size ) ;
};
struct kset {
    struct subsystem *subsys ;
    struct kobj_type *ktype ;
    struct list_head list ;
    spinlock_t list_lock ;
    struct kobject kobj ;
    struct kset_uevent_ops *uevent_ops ;
};
struct subsystem {
    struct kset kset ;
    struct rw_semaphore rwsem ;
};
struct __anonstruct_local_t_32 {
    atomic_t a ;
};
typedef struct __anonstruct_local_t_32 local_t;
struct mod_arch_specific {
    int foo ;
};
struct kernel_symbol {
    unsigned long value ;
    char *name ;
};
struct module_attribute {
    struct attribute attr ;
    ssize_t (*show)(struct module_attribute * , struct module * , char * ) ;
    ssize_t (*store)(struct module_attribute * , struct module * , char * ,
            size_t count ) ;
    void (*setup)(struct module * , char * ) ;
    int (*test)(struct module * ) ;
    void (*free)(struct module * ) ;
};
struct module_kobject {
    struct kobject kobj ;
    struct module *mod ;
};
struct module_ref {
    local_t count ;
} __attribute__((__aligned__((1) << (5) ))) ;
enum module_state {
    MODULE_STATE_LIVE = 0,
    MODULE_STATE_COMING = 1,
    MODULE_STATE_GOING = 2
} ;
struct module_sect_attr {
    struct module_attribute mattr ;
    char name[32] ;
    unsigned long address ;
};
struct module_sect_attrs {
    struct attribute_group grp ;
    struct module_sect_attr attrs[0] ;
};
struct module {
    enum module_state state ;
    struct list_head list ;
    char name[(int )(64U - sizeof(unsigned long ))] ;
    struct module_kobject mkobj ;
    struct module_param_attrs *param_attrs ;
    char *version ;
    char *srcversion ;
    struct kernel_symbol *syms ;
    unsigned int num_syms ;
    unsigned long *crcs ;
    struct kernel_symbol *gpl_syms ;
    unsigned int num_gpl_syms ;
    unsigned long *gpl_crcs ;
    unsigned int num_exentries ;
    struct exception_table_entry *extable ;
    int (*init)(void) ;
    void *module_init ;
    void *module_core ;
    unsigned long init_size ;
    unsigned long core_size ;
    unsigned long init_text_size ;
    unsigned long core_text_size ;
    struct mod_arch_specific arch ;
    int unsafe ;
    int license_gplok ;
    struct module_ref ref[1] ;
    struct list_head modules_which_use_me ;
    struct task_struct *waiter ;
    void (*exit)(void) ;
    Elf32_Sym *symtab ;
    unsigned long num_symtab ;
    char *strtab ;
    struct module_sect_attrs *sect_attrs ;
    void *percpu ;
    char *args ;
};
struct qstr {
    unsigned int hash ;
    unsigned int len ;
    unsigned char *name ;
};
union __anonunion_d_u_33 {
    struct list_head d_child ;
    struct rcu_head d_rcu ;
};
struct dentry {
    atomic_t d_count ;
    unsigned int d_flags ;
    spinlock_t d_lock ;
    struct inode *d_inode ;
    struct hlist_node d_hash ;
    struct dentry *d_parent ;
    struct qstr d_name ;
    struct list_head d_lru ;
    union __anonunion_d_u_33 d_u ;
    struct list_head d_subdirs ;
    struct list_head d_alias ;
    unsigned long d_time ;
    struct dentry_operations *d_op ;
    struct super_block *d_sb ;
    void *d_fsdata ;
    struct dcookie_struct *d_cookie ;
    int d_mounted ;
    unsigned char d_iname[36] ;
};
struct dentry_operations {
    int (*d_revalidate)(struct dentry * , struct nameidata * ) ;
    int (*d_hash)(struct dentry * , struct qstr * ) ;
    int (*d_compare)(struct dentry * , struct qstr * , struct qstr * ) ;
    int (*d_delete)(struct dentry * ) ;
    void (*d_release)(struct dentry * ) ;
    void (*d_iput)(struct dentry * , struct inode * ) ;
};
struct radix_tree_root {
    unsigned int height ;
    gfp_t gfp_mask ;
    struct radix_tree_node *rnode ;
};
struct raw_prio_tree_node {
    struct prio_tree_node *left ;
    struct prio_tree_node *right ;
    struct prio_tree_node *parent ;
};
struct prio_tree_node {
    struct prio_tree_node *left ;
    struct prio_tree_node *right ;
    struct prio_tree_node *parent ;
    unsigned long start ;
    unsigned long last ;
};
struct prio_tree_root {
    struct prio_tree_node *prio_tree_node ;
    unsigned short index_bits ;
    unsigned short raw ;
};
struct mutex {
    atomic_t count ;
    spinlock_t wait_lock ;
    struct list_head wait_list ;
};
struct iattr {
    unsigned int ia_valid ;
    umode_t ia_mode ;
    uid_t ia_uid ;
    gid_t ia_gid ;
    loff_t ia_size ;
    struct timespec ia_atime ;
    struct timespec ia_mtime ;
    struct timespec ia_ctime ;
    struct file *ia_file ;
};
struct if_dqblk {
    __u64 dqb_bhardlimit ;
    __u64 dqb_bsoftlimit ;
    __u64 dqb_curspace ;
    __u64 dqb_ihardlimit ;
    __u64 dqb_isoftlimit ;
    __u64 dqb_curinodes ;
    __u64 dqb_btime ;
    __u64 dqb_itime ;
    __u32 dqb_valid ;
};
struct if_dqinfo {
    __u64 dqi_bgrace ;
    __u64 dqi_igrace ;
    __u32 dqi_flags ;
    __u32 dqi_valid ;
};
struct fs_disk_quota {
    __s8 d_version ;
    __s8 d_flags ;
    __u16 d_fieldmask ;
    __u32 d_id ;
    __u64 d_blk_hardlimit ;
    __u64 d_blk_softlimit ;
    __u64 d_ino_hardlimit ;
    __u64 d_ino_softlimit ;
    __u64 d_bcount ;
    __u64 d_icount ;
    __s32 d_itimer ;
    __s32 d_btimer ;
    __u16 d_iwarns ;
    __u16 d_bwarns ;
    __s32 d_padding2 ;
    __u64 d_rtb_hardlimit ;
    __u64 d_rtb_softlimit ;
    __u64 d_rtbcount ;
    __s32 d_rtbtimer ;
    __u16 d_rtbwarns ;
    __s16 d_padding3 ;
    char d_padding4[8] ;
};
struct fs_qfilestat {
    __u64 qfs_ino ;
    __u64 qfs_nblks ;
    __u32 qfs_nextents ;
};
typedef struct fs_qfilestat fs_qfilestat_t;
struct fs_quota_stat {
    __s8 qs_version ;
    __u16 qs_flags ;
    __s8 qs_pad ;
    fs_qfilestat_t qs_uquota ;
    fs_qfilestat_t qs_gquota ;
    __u32 qs_incoredqs ;
    __s32 qs_btimelimit ;
    __s32 qs_itimelimit ;
    __s32 qs_rtbtimelimit ;
    __u16 qs_bwarnlimit ;
    __u16 qs_iwarnlimit ;
};
struct v1_mem_dqinfo {
};
struct v2_mem_dqinfo {
    unsigned int dqi_blocks ;
    unsigned int dqi_free_blk ;
    unsigned int dqi_free_entry ;
};
struct mem_dqblk {
    __u32 dqb_bhardlimit ;
    __u32 dqb_bsoftlimit ;
    qsize_t dqb_curspace ;
    __u32 dqb_ihardlimit ;
    __u32 dqb_isoftlimit ;
    __u32 dqb_curinodes ;
    time_t dqb_btime ;
    time_t dqb_itime ;
};
union __anonunion_u_34 {
    struct v1_mem_dqinfo v1_i ;
    struct v2_mem_dqinfo v2_i ;
};
struct mem_dqinfo {
    struct quota_format_type *dqi_format ;
    struct list_head dqi_dirty_list ;
    unsigned long dqi_flags ;
    unsigned int dqi_bgrace ;
    unsigned int dqi_igrace ;
    union __anonunion_u_34 u ;
};
struct dquot {
    struct hlist_node dq_hash ;
    struct list_head dq_inuse ;
    struct list_head dq_free ;
    struct list_head dq_dirty ;
    struct semaphore dq_lock ;
    atomic_t dq_count ;
    wait_queue_head_t dq_wait_unused ;
    struct super_block *dq_sb ;
    unsigned int dq_id ;
    loff_t dq_off ;
    unsigned long dq_flags ;
    short dq_type ;
    struct mem_dqblk dq_dqb ;
};
struct quota_format_ops {
    int (*check_quota_file)(struct super_block *sb , int type ) ;
    int (*read_file_info)(struct super_block *sb , int type ) ;
    int (*write_file_info)(struct super_block *sb , int type ) ;
    int (*free_file_info)(struct super_block *sb , int type ) ;
    int (*read_dqblk)(struct dquot *dquot ) ;
    int (*commit_dqblk)(struct dquot *dquot ) ;
    int (*release_dqblk)(struct dquot *dquot ) ;
};
struct dquot_operations {
    int (*initialize)(struct inode * , int ) ;
    int (*drop)(struct inode * ) ;
    int (*alloc_space)(struct inode * , qsize_t , int ) ;
    int (*alloc_inode)(struct inode * , unsigned long ) ;
    int (*free_space)(struct inode * , qsize_t ) ;
    int (*free_inode)(struct inode * , unsigned long ) ;
    int (*transfer)(struct inode * , struct iattr * ) ;
    int (*write_dquot)(struct dquot * ) ;
    int (*acquire_dquot)(struct dquot * ) ;
    int (*release_dquot)(struct dquot * ) ;
    int (*mark_dirty)(struct dquot * ) ;
    int (*write_info)(struct super_block * , int ) ;
};
struct quotactl_ops {
    int (*quota_on)(struct super_block * , int , int , char * ) ;
    int (*quota_off)(struct super_block * , int ) ;
    int (*quota_sync)(struct super_block * , int ) ;
    int (*get_info)(struct super_block * , int , struct if_dqinfo * ) ;
    int (*set_info)(struct super_block * , int , struct if_dqinfo * ) ;
    int (*get_dqblk)(struct super_block * , int , qid_t , struct if_dqblk * ) ;
    int (*set_dqblk)(struct super_block * , int , qid_t , struct if_dqblk * ) ;
    int (*get_xstate)(struct super_block * , struct fs_quota_stat * ) ;
    int (*set_xstate)(struct super_block * , unsigned int , int ) ;
    int (*get_xquota)(struct super_block * , int , qid_t , struct fs_disk_quota * ) ;
    int (*set_xquota)(struct super_block * , int , qid_t , struct fs_disk_quota * ) ;
};
struct quota_format_type {
    int qf_fmt_id ;
    struct quota_format_ops *qf_ops ;
    struct module *qf_owner ;
    struct quota_format_type *qf_next ;
};
struct quota_info {
    unsigned int flags ;
    struct semaphore dqio_sem ;
    struct semaphore dqonoff_sem ;
    struct rw_semaphore dqptr_sem ;
    struct inode *files[2] ;
    struct mem_dqinfo info[2] ;
    struct quota_format_ops *ops[2] ;
};
struct address_space_operations {
    int (*writepage)(struct page *page , struct writeback_control *wbc ) ;
    int (*readpage)(struct file * , struct page * ) ;
    int (*sync_page)(struct page * ) ;
    int (*writepages)(struct address_space * , struct writeback_control * ) ;
    int (*set_page_dirty)(struct page *page ) ;
    int (*readpages)(struct file *filp , struct address_space *mapping , struct list_head *pages ,
            unsigned int nr_pages ) ;
    int (*prepare_write)(struct file * , struct page * , unsigned int , unsigned int ) ;
    int (*commit_write)(struct file * , struct page * , unsigned int , unsigned int ) ;
    sector_t (*bmap)(struct address_space * , sector_t ) ;
    int (*invalidatepage)(struct page * , unsigned long ) ;
    int (*releasepage)(struct page * , gfp_t ) ;
    ssize_t (*direct_IO)(int , struct kiocb * , struct iovec *iov , loff_t offset ,
            unsigned long nr_segs ) ;
    struct page *(*get_xip_page)(struct address_space * , sector_t , int ) ;
    int (*migratepage)(struct page * , struct page * ) ;
};
struct address_space {
    struct inode *host ;
    struct radix_tree_root page_tree ;
    rwlock_t tree_lock ;
    unsigned int i_mmap_writable ;
    struct prio_tree_root i_mmap ;
    struct list_head i_mmap_nonlinear ;
    spinlock_t i_mmap_lock ;
    unsigned int truncate_count ;
    unsigned long nrpages ;
    unsigned long writeback_index ;
    struct address_space_operations *a_ops ;
    unsigned long flags ;
    struct backing_dev_info *backing_dev_info ;
    spinlock_t private_lock ;
    struct list_head private_list ;
    struct address_space *assoc_mapping ;
} __attribute__((__aligned__(sizeof(long )))) ;
struct block_device {
    dev_t bd_dev ;
    struct inode *bd_inode ;
    int bd_openers ;
    struct semaphore bd_sem ;
    struct semaphore bd_mount_sem ;
    struct list_head bd_inodes ;
    void *bd_holder ;
    int bd_holders ;
    struct block_device *bd_contains ;
    unsigned int bd_block_size ;
    struct hd_struct *bd_part ;
    unsigned int bd_part_count ;
    int bd_invalidated ;
    struct gendisk *bd_disk ;
    struct list_head bd_list ;
    struct backing_dev_info *bd_inode_backing_dev_info ;
    unsigned long bd_private ;
};
union __anonunion_u_35 {
    void *generic_ip ;
};
struct inode {
    struct hlist_node i_hash ;
    struct list_head i_list ;
    struct list_head i_sb_list ;
    struct list_head i_dentry ;
    unsigned long i_ino ;
    atomic_t i_count ;
    umode_t i_mode ;
    unsigned int i_nlink ;
    uid_t i_uid ;
    gid_t i_gid ;
    dev_t i_rdev ;
    loff_t i_size ;
    struct timespec i_atime ;
    struct timespec i_mtime ;
    struct timespec i_ctime ;
    unsigned int i_blkbits ;
    unsigned long i_blksize ;
    unsigned long i_version ;
    unsigned long i_blocks ;
    unsigned short i_bytes ;
    spinlock_t i_lock ;
    struct mutex i_mutex ;
    struct rw_semaphore i_alloc_sem ;
    struct inode_operations *i_op ;
    struct file_operations *i_fop ;
    struct super_block *i_sb ;
    struct file_lock *i_flock ;
    struct address_space *i_mapping ;
    struct address_space i_data ;
    struct dquot *i_dquot[2] ;
    struct list_head i_devices ;
    struct pipe_inode_info *i_pipe ;
    struct block_device *i_bdev ;
    struct cdev *i_cdev ;
    int i_cindex ;
    __u32 i_generation ;
    unsigned long i_dnotify_mask ;
    struct dnotify_struct *i_dnotify ;
    struct list_head inotify_watches ;
    struct semaphore inotify_sem ;
    unsigned long i_state ;
    unsigned long dirtied_when ;
    unsigned int i_flags ;
    atomic_t i_writecount ;
    void *i_security ;
    union __anonunion_u_35 u ;
};
struct fown_struct {
    rwlock_t lock ;
    int pid ;
    uid_t uid ;
    uid_t euid ;
    void *security ;
    int signum ;
};
struct file_ra_state {
    unsigned long start ;
    unsigned long size ;
    unsigned long flags ;
    unsigned long cache_hit ;
    unsigned long prev_page ;
    unsigned long ahead_start ;
    unsigned long ahead_size ;
    unsigned long ra_pages ;
    unsigned long mmap_hit ;
    unsigned long mmap_miss ;
};
union __anonunion_f_u_36 {
    struct list_head fu_list ;
    struct rcu_head fu_rcuhead ;
};
struct file {
    union __anonunion_f_u_36 f_u ;
    struct dentry *f_dentry ;
    struct vfsmount *f_vfsmnt ;
    struct file_operations *f_op ;
    atomic_t f_count ;
    unsigned int f_flags ;
    mode_t f_mode ;
    loff_t f_pos ;
    struct fown_struct f_owner ;
    unsigned int f_uid ;
    unsigned int f_gid ;
    struct file_ra_state f_ra ;
    unsigned long f_version ;
    void *f_security ;
    void *private_data ;
    struct list_head f_ep_links ;
    spinlock_t f_ep_lock ;
    struct address_space *f_mapping ;
};
typedef struct files_struct *fl_owner_t;
struct file_lock_operations {
    void (*fl_insert)(struct file_lock * ) ;
    void (*fl_remove)(struct file_lock * ) ;
    void (*fl_copy_lock)(struct file_lock * , struct file_lock * ) ;
    void (*fl_release_private)(struct file_lock * ) ;
};
struct lock_manager_operations {
    int (*fl_compare_owner)(struct file_lock * , struct file_lock * ) ;
    void (*fl_notify)(struct file_lock * ) ;
    void (*fl_copy_lock)(struct file_lock * , struct file_lock * ) ;
    void (*fl_release_private)(struct file_lock * ) ;
    void (*fl_break)(struct file_lock * ) ;
    int (*fl_mylease)(struct file_lock * , struct file_lock * ) ;
    int (*fl_change)(struct file_lock ** , int ) ;
};
struct nfs_lock_info {
    u32 state ;
    u32 flags ;
    struct nlm_lockowner *owner ;
};
struct nfs4_lock_info {
    struct nfs4_lock_state *owner ;
};
union __anonunion_fl_u_37 {
    struct nfs_lock_info nfs_fl ;
    struct nfs4_lock_info nfs4_fl ;
};
struct file_lock {
    struct file_lock *fl_next ;
    struct list_head fl_link ;
    struct list_head fl_block ;
    fl_owner_t fl_owner ;
    unsigned int fl_pid ;
    wait_queue_head_t fl_wait ;
    struct file *fl_file ;
    unsigned char fl_flags ;
    unsigned char fl_type ;
    loff_t fl_start ;
    loff_t fl_end ;
    struct fasync_struct *fl_fasync ;
    unsigned long fl_break_time ;
    struct file_lock_operations *fl_ops ;
    struct lock_manager_operations *fl_lmops ;
    union __anonunion_fl_u_37 fl_u ;
};
struct fasync_struct {
    int magic ;
    int fa_fd ;
    struct fasync_struct *fa_next ;
    struct file *fa_file ;
};
struct super_block {
    struct list_head s_list ;
    dev_t s_dev ;
    unsigned long s_blocksize ;
    unsigned char s_blocksize_bits ;
    unsigned char s_dirt ;
    unsigned long long s_maxbytes ;
    struct file_system_type *s_type ;
    struct super_operations *s_op ;
    struct dquot_operations *dq_op ;
    struct quotactl_ops *s_qcop ;
    struct export_operations *s_export_op ;
    unsigned long s_flags ;
    unsigned long s_magic ;
    struct dentry *s_root ;
    struct rw_semaphore s_umount ;
    struct mutex s_lock ;
    int s_count ;
    int s_syncing ;
    int s_need_sync_fs ;
    atomic_t s_active ;
    void *s_security ;
    struct xattr_handler **s_xattr ;
    struct list_head s_inodes ;
    struct list_head s_dirty ;
    struct list_head s_io ;
    struct hlist_head s_anon ;
    struct list_head s_files ;
    struct block_device *s_bdev ;
    struct list_head s_instances ;
    struct quota_info s_dquot ;
    int s_frozen ;
    wait_queue_head_t s_wait_unfrozen ;
    char s_id[32] ;
    void *s_fs_info ;
    struct semaphore s_vfs_rename_sem ;
    u32 s_time_gran ;
};
struct block_device_operations {
    int (*open)(struct inode * , struct file * ) ;
    int (*release)(struct inode * , struct file * ) ;
    int (*ioctl)(struct inode * , struct file * , unsigned int , unsigned long ) ;
    long (*unlocked_ioctl)(struct file * , unsigned int , unsigned long ) ;
    long (*compat_ioctl)(struct file * , unsigned int , unsigned long ) ;
    int (*direct_access)(struct block_device * , sector_t , unsigned long * ) ;
    int (*media_changed)(struct gendisk * ) ;
    int (*revalidate_disk)(struct gendisk * ) ;
    int (*getgeo)(struct block_device * , struct hd_geometry * ) ;
    struct module *owner ;
};
union __anonunion_arg_40 {
    char *buf ;
    void *data ;
};
struct __anonstruct_read_descriptor_t_39 {
    size_t written ;
    size_t count ;
    union __anonunion_arg_40 arg ;
    int error ;
};
typedef struct __anonstruct_read_descriptor_t_39 read_descriptor_t;
struct file_operations {
    struct module *owner ;
    loff_t (*llseek)(struct file * , loff_t , int ) ;
    ssize_t (*read)(struct file * , char * , size_t , loff_t * ) ;
    ssize_t (*aio_read)(struct kiocb * , char * , size_t , loff_t ) ;
    ssize_t (*write)(struct file * , char * , size_t , loff_t * ) ;
    ssize_t (*aio_write)(struct kiocb * , char * , size_t , loff_t ) ;
    int (*readdir)(struct file * , void * , int (*)(void *, char *, int, loff_t, ino_t, unsigned int)) ;
    unsigned int (*poll)(struct file * , struct poll_table_struct * ) ;
    int (*ioctl)(struct inode * , struct file * , unsigned int , unsigned long ) ;
    long (*unlocked_ioctl)(struct file * , unsigned int , unsigned long ) ;
    long (*compat_ioctl)(struct file * , unsigned int , unsigned long ) ;
    int (*mmap)(struct file * , struct vm_area_struct * ) ;
    int (*open)(struct inode * , struct file * ) ;
    int (*flush)(struct file * ) ;
    int (*release)(struct inode * , struct file * ) ;
    int (*fsync)(struct file * , struct dentry * , int datasync ) ;
    int (*aio_fsync)(struct kiocb * , int datasync ) ;
    int (*fasync)(int , struct file * , int ) ;
    int (*lock)(struct file * , int , struct file_lock * ) ;
    ssize_t (*readv)(struct file * , struct iovec * , unsigned long , loff_t * ) ;
    ssize_t (*writev)(struct file * , struct iovec * , unsigned long , loff_t * ) ;
    ssize_t (*sendfile)(struct file * , loff_t * , size_t ,
            int (*)(read_descriptor_t * , struct page * , unsigned long , unsigned long ) ,
            void * ) ;
    ssize_t (*sendpage)(struct file * , struct page * , int , size_t , loff_t * , int ) ;
    unsigned long (*get_unmapped_area)(struct file * , unsigned long , unsigned long ,
            unsigned long , unsigned long ) ;
    int (*check_flags)(int ) ;
    int (*dir_notify)(struct file *filp , unsigned long arg ) ;
    int (*flock)(struct file * , int , struct file_lock * ) ;
};
struct inode_operations {
    int (*create)(struct inode * , struct dentry * , int , struct nameidata * ) ;
    struct dentry *(*lookup)(struct inode * , struct dentry * , struct nameidata * ) ;
    int (*link)(struct dentry * , struct inode * , struct dentry * ) ;
    int (*unlink)(struct inode * , struct dentry * ) ;
    int (*symlink)(struct inode * , struct dentry * , char * ) ;
    int (*mkdir)(struct inode * , struct dentry * , int ) ;
    int (*rmdir)(struct inode * , struct dentry * ) ;
    int (*mknod)(struct inode * , struct dentry * , int , dev_t ) ;
    int (*rename)(struct inode * , struct dentry * , struct inode * , struct dentry * ) ;
    int (*readlink)(struct dentry * , char * , int ) ;
    void *(*follow_link)(struct dentry * , struct nameidata * ) ;
    void (*put_link)(struct dentry * , struct nameidata * , void * ) ;
    void (*truncate)(struct inode * ) ;
    int (*permission)(struct inode * , int , struct nameidata * ) ;
    int (*setattr)(struct dentry * , struct iattr * ) ;
    int (*getattr)(struct vfsmount *mnt , struct dentry * , struct kstat * ) ;
    int (*setxattr)(struct dentry * , char * , void * , size_t , int ) ;
    ssize_t (*getxattr)(struct dentry * , char * , void * , size_t ) ;
    ssize_t (*listxattr)(struct dentry * , char * , size_t ) ;
    int (*removexattr)(struct dentry * , char * ) ;
    void (*truncate_range)(struct inode * , loff_t , loff_t ) ;
};
struct super_operations {
    struct inode *(*alloc_inode)(struct super_block *sb ) ;
    void (*destroy_inode)(struct inode * ) ;
    void (*read_inode)(struct inode * ) ;
    void (*dirty_inode)(struct inode * ) ;
    int (*write_inode)(struct inode * , int ) ;
    void (*put_inode)(struct inode * ) ;
    void (*drop_inode)(struct inode * ) ;
    void (*delete_inode)(struct inode * ) ;
    void (*put_super)(struct super_block * ) ;
    void (*write_super)(struct super_block * ) ;
    int (*sync_fs)(struct super_block *sb , int wait ) ;
    void (*write_super_lockfs)(struct super_block * ) ;
    void (*unlockfs)(struct super_block * ) ;
    int (*statfs)(struct super_block * , struct kstatfs * ) ;
    int (*remount_fs)(struct super_block * , int * , char * ) ;
    void (*clear_inode)(struct inode * ) ;
    void (*umount_begin)(struct super_block * ) ;
    int (*show_options)(struct seq_file * , struct vfsmount * ) ;
    ssize_t (*quota_read)(struct super_block * , int , char * , size_t , loff_t ) ;
    ssize_t (*quota_write)(struct super_block * , int , char * , size_t ,
            loff_t ) ;
};
struct export_operations {
    struct dentry *(*decode_fh)(struct super_block *sb , __u32 *fh , int fh_len , int fh_type ,
            int (*acceptable)(void *context , struct dentry *de ) ,
            void *context ) ;
    int (*encode_fh)(struct dentry *de , __u32 *fh , int *max_len , int connectable ) ;
    int (*get_name)(struct dentry *parent , char *name , struct dentry *child ) ;
    struct dentry *(*get_parent)(struct dentry *child ) ;
    struct dentry *(*get_dentry)(struct super_block *sb , void *inump ) ;
    struct dentry *(*find_exported_dentry)(struct super_block *sb , void *obj , void *parent ,
            int (*acceptable)(void *context , struct dentry *de ) ,
            void *context ) ;
};
struct file_system_type {
    char *name ;
    int fs_flags ;
    struct super_block *(*get_sb)(struct file_system_type * , int , char * , void * ) ;
    void (*kill_sb)(struct super_block * ) ;
    struct module *owner ;
    struct file_system_type *next ;
    struct list_head fs_supers ;
};
struct __anonstruct_vm_set_43 {
    struct list_head list ;
    void *parent ;
    struct vm_area_struct *head ;
};
union __anonunion_shared_42 {
    struct __anonstruct_vm_set_43 vm_set ;
    struct raw_prio_tree_node prio_tree_node ;
};
struct vm_area_struct {
    struct mm_struct *vm_mm ;
    unsigned long vm_start ;
    unsigned long vm_end ;
    struct vm_area_struct *vm_next ;
    pgprot_t vm_page_prot ;
    unsigned long vm_flags ;
    struct rb_node vm_rb ;
    union __anonunion_shared_42 shared ;
    struct list_head anon_vma_node ;
    struct anon_vma *anon_vma ;
    struct vm_operations_struct *vm_ops ;
    unsigned long vm_pgoff ;
    struct file *vm_file ;
    void *vm_private_data ;
    unsigned long vm_truncate_count ;
};
struct vm_operations_struct {
    void (*open)(struct vm_area_struct *area ) ;
    void (*close)(struct vm_area_struct *area ) ;
    struct page *(*nopage)(struct vm_area_struct *area , unsigned long address , int *type ) ;
    int (*populate)(struct vm_area_struct *area , unsigned long address , unsigned long len ,
            pgprot_t prot , unsigned long pgoff , int nonblock ) ;
};
struct __anonstruct____missing_field_name_45 {
    unsigned long private ;
    struct address_space *mapping ;
};
union __anonunion____missing_field_name_44 {
    struct __anonstruct____missing_field_name_45 __annonCompField1 ;
};
struct page {
    unsigned long flags ;
    atomic_t _count ;
    atomic_t _mapcount ;
    union __anonunion____missing_field_name_44 __annonCompField2 ;
    unsigned long index ;
    struct list_head lru ;
};
struct exception_table_entry {
    unsigned long insn ;
    unsigned long fixup ;
};
struct cdrom_msf0 {
    __u8 minute ;
    __u8 second ;
    __u8 frame ;
};
union cdrom_addr {
    struct cdrom_msf0 msf ;
    int lba ;
};
struct cdrom_msf {
    __u8 cdmsf_min0 ;
    __u8 cdmsf_sec0 ;
    __u8 cdmsf_frame0 ;
    __u8 cdmsf_min1 ;
    __u8 cdmsf_sec1 ;
    __u8 cdmsf_frame1 ;
};
struct cdrom_ti {
    __u8 cdti_trk0 ;
    __u8 cdti_ind0 ;
    __u8 cdti_trk1 ;
    __u8 cdti_ind1 ;
};
struct cdrom_tochdr {
    __u8 cdth_trk0 ;
    __u8 cdth_trk1 ;
};
struct cdrom_volctrl {
    __u8 channel0 ;
    __u8 channel1 ;
    __u8 channel2 ;
    __u8 channel3 ;
};
struct cdrom_subchnl {
    __u8 cdsc_format ;
    __u8 cdsc_audiostatus ;
    __u8 cdsc_adr : 4 ;
    __u8 cdsc_ctrl : 4 ;
    __u8 cdsc_trk ;
    __u8 cdsc_ind ;
    union cdrom_addr cdsc_absaddr ;
    union cdrom_addr cdsc_reladdr ;
};
struct cdrom_tocentry {
    __u8 cdte_track ;
    __u8 cdte_adr : 4 ;
    __u8 cdte_ctrl : 4 ;
    __u8 cdte_format ;
    union cdrom_addr cdte_addr ;
    __u8 cdte_datamode ;
};
struct cdrom_read_audio {
    union cdrom_addr addr ;
    __u8 addr_format ;
    int nframes ;
    __u8 *buf ;
};
struct cdrom_multisession {
    union cdrom_addr addr ;
    __u8 xa_flag ;
    __u8 addr_format ;
};
struct cdrom_mcn {
    __u8 medium_catalog_number[14] ;
};
struct cdrom_blk {
    unsigned int from ;
    unsigned short len ;
};
struct dvd_layer {
    __u8 book_version : 4 ;
    __u8 book_type : 4 ;
    __u8 min_rate : 4 ;
    __u8 disc_size : 4 ;
    __u8 layer_type : 4 ;
    __u8 track_path : 1 ;
    __u8 nlayers : 2 ;
    __u8 track_density : 4 ;
    __u8 linear_density : 4 ;
    __u8 bca : 1 ;
    __u32 start_sector ;
    __u32 end_sector ;
    __u32 end_sector_l0 ;
};
struct dvd_physical {
    __u8 type ;
    __u8 layer_num ;
    struct dvd_layer layer[4] ;
};
struct dvd_copyright {
    __u8 type ;
    __u8 layer_num ;
    __u8 cpst ;
    __u8 rmi ;
};
struct dvd_disckey {
    __u8 type ;
    unsigned int agid : 2 ;
    __u8 value[2048] ;
};
struct dvd_bca {
    __u8 type ;
    int len ;
    __u8 value[188] ;
};
struct dvd_manufact {
    __u8 type ;
    __u8 layer_num ;
    int len ;
    __u8 value[2048] ;
};
union __anonunion_dvd_struct_46 {
    __u8 type ;
    struct dvd_physical physical ;
    struct dvd_copyright copyright ;
    struct dvd_disckey disckey ;
    struct dvd_bca bca ;
    struct dvd_manufact manufact ;
};
struct dvd_lu_send_agid {
    __u8 type ;
    unsigned int agid : 2 ;
};
struct dvd_host_send_challenge {
    __u8 type ;
    unsigned int agid : 2 ;
    dvd_challenge chal ;
};
struct dvd_send_key {
    __u8 type ;
    unsigned int agid : 2 ;
    dvd_key key ;
};
struct dvd_lu_send_challenge {
    __u8 type ;
    unsigned int agid : 2 ;
    dvd_challenge chal ;
};
struct dvd_lu_send_title_key {
    __u8 type ;
    unsigned int agid : 2 ;
    dvd_key title_key ;
    int lba ;
    unsigned int cpm : 1 ;
    unsigned int cp_sec : 1 ;
    unsigned int cgms : 2 ;
};
struct dvd_lu_send_asf {
    __u8 type ;
    unsigned int agid : 2 ;
    unsigned int asf : 1 ;
};
struct dvd_host_send_rpcstate {
    __u8 type ;
    __u8 pdrc ;
};
struct dvd_lu_send_rpcstate {
    __u8 type : 2 ;
    __u8 vra : 3 ;
    __u8 ucca : 3 ;
    __u8 region_mask ;
    __u8 rpc_scheme ;
};
union __anonunion_dvd_authinfo_47 {
    __u8 type ;
    struct dvd_lu_send_agid lsa ;
    struct dvd_host_send_challenge hsc ;
    struct dvd_send_key lsk ;
    struct dvd_lu_send_challenge lsc ;
    struct dvd_send_key hsk ;
    struct dvd_lu_send_title_key lstk ;
    struct dvd_lu_send_asf lsasf ;
    struct dvd_host_send_rpcstate hrpcs ;
    struct dvd_lu_send_rpcstate lrpcs ;
};
typedef union __anonunion_dvd_authinfo_47 dvd_authinfo;
struct request_sense {
    __u8 error_code : 7 ;
    __u8 valid : 1 ;
    __u8 segment_number ;
    __u8 sense_key : 4 ;
    __u8 reserved2 : 1 ;
    __u8 ili : 1 ;
    __u8 reserved1 : 2 ;
    __u8 information[4] ;
    __u8 add_sense_len ;
    __u8 command_info[4] ;
    __u8 asc ;
    __u8 ascq ;
    __u8 fruc ;
    __u8 sks[3] ;
    __u8 asb[46] ;
};
struct mrw_feature_desc {
    __u16 feature_code ;
    __u8 curr : 1 ;
    __u8 persistent : 1 ;
    __u8 feature_version : 4 ;
    __u8 reserved1 : 2 ;
    __u8 add_len ;
    __u8 write : 1 ;
    __u8 reserved2 : 7 ;
    __u8 reserved3 ;
    __u8 reserved4 ;
    __u8 reserved5 ;
};
struct rwrt_feature_desc {
    __u16 feature_code ;
    __u8 curr : 1 ;
    __u8 persistent : 1 ;
    __u8 feature_version : 4 ;
    __u8 reserved1 : 2 ;
    __u8 add_len ;
    __u32 last_lba ;
    __u32 block_size ;
    __u16 blocking ;
    __u8 page_present : 1 ;
    __u8 reserved2 : 7 ;
    __u8 reserved3 ;
};
struct __anonstruct_disc_information_48 {
    __u16 disc_information_length ;
    __u8 disc_status : 2 ;
    __u8 border_status : 2 ;
    __u8 erasable : 1 ;
    __u8 reserved1 : 3 ;
    __u8 n_first_track ;
    __u8 n_sessions_lsb ;
    __u8 first_track_lsb ;
    __u8 last_track_lsb ;
    __u8 mrw_status : 2 ;
    __u8 dbit : 1 ;
    __u8 reserved2 : 2 ;
    __u8 uru : 1 ;
    __u8 dbc_v : 1 ;
    __u8 did_v : 1 ;
    __u8 disc_type ;
    __u8 n_sessions_msb ;
    __u8 first_track_msb ;
    __u8 last_track_msb ;
    __u32 disc_id ;
    __u32 lead_in ;
    __u32 lead_out ;
    __u8 disc_bar_code[8] ;
    __u8 reserved3 ;
    __u8 n_opc ;
};
typedef struct __anonstruct_disc_information_48 disc_information;
struct __anonstruct_track_information_49 {
    __u16 track_information_length ;
    __u8 track_lsb ;
    __u8 session_lsb ;
    __u8 reserved1 ;
    __u8 track_mode : 4 ;
    __u8 copy : 1 ;
    __u8 damage : 1 ;
    __u8 reserved2 : 2 ;
    __u8 data_mode : 4 ;
    __u8 fp : 1 ;
    __u8 packet : 1 ;
    __u8 blank : 1 ;
    __u8 rt : 1 ;
    __u8 nwa_v : 1 ;
    __u8 lra_v : 1 ;
    __u8 reserved3 : 6 ;
    __u32 track_start ;
    __u32 next_writable ;
    __u32 free_blocks ;
    __u32 fixed_packet_size ;
    __u32 track_size ;
    __u32 last_rec_address ;
};
typedef struct __anonstruct_track_information_49 track_information;
struct feature_header {
    __u32 data_len ;
    __u8 reserved1 ;
    __u8 reserved2 ;
    __u16 curr_profile ;
};
struct mode_page_header {
    __u16 mode_data_length ;
    __u8 medium_type ;
    __u8 reserved1 ;
    __u8 reserved2 ;
    __u8 reserved3 ;
    __u16 desc_length ;
};
struct klist {
    spinlock_t k_lock ;
    struct list_head k_list ;
    void (*get)(struct klist_node * ) ;
    void (*put)(struct klist_node * ) ;
};
struct klist_node {
    struct klist *n_klist ;
    struct list_head n_node ;
    struct kref n_ref ;
    struct completion n_removed ;
};
struct pm_message {
    int event ;
};
typedef struct pm_message pm_message_t;
struct dev_pm_info {
    pm_message_t power_state ;
    unsigned int can_wakeup : 1 ;
    unsigned int should_wakeup : 1 ;
    pm_message_t prev_state ;
    void *saved_state ;
    struct device *pm_parent ;
    struct list_head entry ;
};
struct bus_type {
    char *name ;
    struct subsystem subsys ;
    struct kset drivers ;
    struct kset devices ;
    struct klist klist_devices ;
    struct klist klist_drivers ;
    struct bus_attribute *bus_attrs ;
    struct device_attribute *dev_attrs ;
    struct driver_attribute *drv_attrs ;
    int (*match)(struct device *dev , struct device_driver *drv ) ;
    int (*uevent)(struct device *dev , char **envp , int num_envp , char *buffer ,
            int buffer_size ) ;
    int (*probe)(struct device *dev ) ;
    int (*remove)(struct device *dev ) ;
    void (*shutdown)(struct device *dev ) ;
    int (*suspend)(struct device *dev , pm_message_t state ) ;
    int (*resume)(struct device *dev ) ;
};
struct bus_attribute {
    struct attribute attr ;
    ssize_t (*show)(struct bus_type * , char *buf ) ;
    ssize_t (*store)(struct bus_type * , char *buf , size_t count ) ;
};
struct device_driver {
    char *name ;
    struct bus_type *bus ;
    struct completion unloaded ;
    struct kobject kobj ;
    struct klist klist_devices ;
    struct klist_node knode_bus ;
    struct module *owner ;
    int (*probe)(struct device *dev ) ;
    int (*remove)(struct device *dev ) ;
    void (*shutdown)(struct device *dev ) ;
    int (*suspend)(struct device *dev , pm_message_t state ) ;
    int (*resume)(struct device *dev ) ;
};
struct driver_attribute {
    struct attribute attr ;
    ssize_t (*show)(struct device_driver * , char *buf ) ;
    ssize_t (*store)(struct device_driver * , char *buf , size_t count ) ;
};
struct device_attribute {
    struct attribute attr ;
    ssize_t (*show)(struct device *dev , struct device_attribute *attr , char *buf ) ;
    ssize_t (*store)(struct device *dev , struct device_attribute *attr , char *buf ,
            size_t count ) ;
};
struct device {
    struct klist klist_children ;
    struct klist_node knode_parent ;
    struct klist_node knode_driver ;
    struct klist_node knode_bus ;
    struct device *parent ;
    struct kobject kobj ;
    char bus_id[20] ;
    struct device_attribute uevent_attr ;
    struct semaphore sem ;
    struct bus_type *bus ;
    struct device_driver *driver ;
    void *driver_data ;
    void *platform_data ;
    void *firmware_data ;
    struct dev_pm_info power ;
    u64 *dma_mask ;
    u64 coherent_dma_mask ;
    struct list_head dma_pools ;
    struct dma_coherent_mem *dma_mem ;
    void (*release)(struct device *dev ) ;
};
struct packet_command {
    unsigned char cmd[12] ;
    unsigned char *buffer ;
    unsigned int buflen ;
    int stat ;
    struct request_sense *sense ;
    unsigned char data_direction ;
    int quiet ;
    int timeout ;
    void *reserved[1] ;
};
struct cdrom_device_info {
    struct cdrom_device_ops *ops ;
    struct cdrom_device_info *next ;
    struct gendisk *disk ;
    void *handle ;
    int mask ;
    int speed ;
    int capacity ;
    int options : 30 ;
    unsigned int mc_flags : 2 ;
    int use_count ;
    char name[20] ;
    __u8 sanyo_slot : 2 ;
    __u8 reserved : 6 ;
    int cdda_method ;
    __u8 last_sense ;
    __u8 media_written ;
    unsigned short mmc3_profile ;
    int for_data ;
    int (*exit)(struct cdrom_device_info * ) ;
    int mrw_mode_page ;
};
struct cdrom_device_ops {
    int (*open)(struct cdrom_device_info * , int ) ;
    void (*release)(struct cdrom_device_info * ) ;
    int (*drive_status)(struct cdrom_device_info * , int ) ;
    int (*media_changed)(struct cdrom_device_info * , int ) ;
    int (*tray_move)(struct cdrom_device_info * , int ) ;
    int (*lock_door)(struct cdrom_device_info * , int ) ;
    int (*select_speed)(struct cdrom_device_info * , int ) ;
    int (*select_disc)(struct cdrom_device_info * , int ) ;
    int (*get_last_session)(struct cdrom_device_info * , struct cdrom_multisession * ) ;
    int (*get_mcn)(struct cdrom_device_info * , struct cdrom_mcn * ) ;
    int (*reset)(struct cdrom_device_info * ) ;
    int (*audio_ioctl)(struct cdrom_device_info * , unsigned int , void * ) ;
    int (*dev_ioctl)(struct cdrom_device_info * , unsigned int , unsigned long ) ;
    int capability ;
    int n_minors ;
    int (*generic_packet)(struct cdrom_device_info * , struct packet_command * ) ;
};
struct __anonstruct_tracktype_51 {
    int data ;
    int audio ;
    int cdi ;
    int xa ;
    long error ;
};
typedef struct __anonstruct_tracktype_51 tracktype;
struct cdrom_mechstat_header {
    __u8 curslot : 5 ;
    __u8 changer_state : 2 ;
    __u8 fault : 1 ;
    __u8 reserved1 : 4 ;
    __u8 door_open : 1 ;
    __u8 mech_state : 3 ;
    __u8 curlba[3] ;
    __u8 nslots ;
    __u16 slot_tablelen ;
};
struct cdrom_slot {
    __u8 change : 1 ;
    __u8 reserved1 : 6 ;
    __u8 disc_present : 1 ;
    __u8 reserved2[3] ;
};
struct cdrom_changer_info {
    struct cdrom_mechstat_header hdr ;
    struct cdrom_slot slots[256] ;
};
struct modesel_head {
    __u8 reserved1 ;
    __u8 medium ;
    __u8 reserved2 ;
    __u8 block_desc_length ;
    __u8 density ;
    __u8 number_of_blocks_hi ;
    __u8 number_of_blocks_med ;
    __u8 number_of_blocks_lo ;
    __u8 reserved3 ;
    __u8 block_length_hi ;
    __u8 block_length_med ;
    __u8 block_length_lo ;
};
struct __anonstruct_rpc_state_t_54 {
    __u16 report_key_length ;
    __u8 reserved1 ;
    __u8 reserved2 ;
    __u8 ucca : 3 ;
    __u8 vra : 3 ;
    __u8 type_code : 2 ;
    __u8 region_mask ;
    __u8 rpc_scheme ;
    __u8 reserved3 ;
};
typedef struct __anonstruct_rpc_state_t_54 rpc_state_t;
struct event_header {
    __u16 data_len ;
    __u8 notification_class : 3 ;
    __u8 reserved1 : 4 ;
    __u8 nea : 1 ;
    __u8 supp_event_class ;
};
struct media_event_desc {
    __u8 media_event_code : 4 ;
    __u8 reserved1 : 4 ;
    __u8 door_open : 1 ;
    __u8 media_present : 1 ;
    __u8 reserved2 : 6 ;
    __u8 start_slot ;
    __u8 end_slot ;
};
typedef struct ctl_table ctl_table;
typedef int ctl_handler(ctl_table *table , int *name , int nlen , void *oldval , size_t *oldlenp ,
        void *newval , size_t newlen , void **context );
typedef int proc_handler(ctl_table *ctl , int write , struct file *filp , void *buffer ,
        size_t *lenp , loff_t *ppos );
struct ctl_table {
    int ctl_name ;
    char *procname ;
    void *data ;
    int maxlen ;
    mode_t mode ;
    ctl_table *child ;
    proc_handler *proc_handler ;
    ctl_handler *strategy ;
    struct proc_dir_entry *de ;
    void *extra1 ;
    void *extra2 ;
};
struct ctl_table_header {
    ctl_table *ctl_table ;
    struct list_head ctl_entry ;
    int used ;
    struct completion *unregistering ;
};
typedef int read_proc_t(char *page , char **start , off_t off , int count , int *eof ,
        void *data );
typedef int write_proc_t(struct file *file , char *buffer , unsigned long count ,
        void *data );
typedef int get_info_t(char * , char ** , off_t , int );
struct proc_dir_entry {
    unsigned int low_ino ;
    unsigned short namelen ;
    char *name ;
    mode_t mode ;
    nlink_t nlink ;
    uid_t uid ;
    gid_t gid ;
    unsigned long size ;
    struct inode_operations *proc_iops ;
    struct file_operations *proc_fops ;
    get_info_t *get_info ;
    struct module *owner ;
    struct proc_dir_entry *next ;
    struct proc_dir_entry *parent ;
    struct proc_dir_entry *subdir ;
    void *data ;
    read_proc_t *read_proc ;
    write_proc_t *write_proc ;
    atomic_t count ;
    int deleted ;
    void *set ;
};
struct hd_struct {
    sector_t start_sect ;
    sector_t nr_sects ;
    struct kobject kobj ;
    unsigned int ios[2] ;
    unsigned int sectors[2] ;
    int policy ;
    int partno ;
};
struct disk_stats {
    unsigned int sectors[2] ;
    unsigned int ios[2] ;
    unsigned int merges[2] ;
    unsigned int ticks[2] ;
    unsigned int io_ticks ;
    unsigned int time_in_queue ;
};
struct gendisk {
    int major ;
    int first_minor ;
    int minors ;
    char disk_name[32] ;
    struct hd_struct **part ;
    struct block_device_operations *fops ;
    struct request_queue *queue ;
    void *private_data ;
    sector_t capacity ;
    int flags ;
    char devfs_name[64] ;
    int number ;
    struct device *driverfs_dev ;
    struct kobject kobj ;
    struct timer_rand_state *random ;
    int policy ;
    atomic_t sync_io ;
    unsigned long stamp ;
    int in_flight ;
    struct disk_stats dkstats ;
};
typedef int congested_fn(void * , int );
struct backing_dev_info {
    unsigned long ra_pages ;
    unsigned long state ;
    unsigned int capabilities ;
    congested_fn *congested_fn ;
    void *congested_data ;
    void (*unplug_io_fn)(struct backing_dev_info * , struct page * ) ;
    void *unplug_io_data ;
};
typedef void *mempool_alloc_t(gfp_t gfp_mask , void *pool_data );
typedef void mempool_free_t(void *element , void *pool_data );
struct mempool_s {
    spinlock_t lock ;
    int min_nr ;
    int curr_nr ;
    void **elements ;
    void *pool_data ;
    mempool_alloc_t *alloc ;
    mempool_free_t *free ;
    wait_queue_head_t wait ;
};
typedef struct mempool_s mempool_t;
struct bio_vec {
    struct page *bv_page ;
    unsigned int bv_len ;
    unsigned int bv_offset ;
};
typedef int bio_end_io_t(struct bio * , unsigned int , int );
typedef void bio_destructor_t(struct bio * );
struct bio {
    sector_t bi_sector ;
    struct bio *bi_next ;
    struct block_device *bi_bdev ;
    unsigned long bi_flags ;
    unsigned long bi_rw ;
    unsigned short bi_vcnt ;
    unsigned short bi_idx ;
    unsigned short bi_phys_segments ;
    unsigned short bi_hw_segments ;
    unsigned int bi_size ;
    unsigned int bi_hw_front_size ;
    unsigned int bi_hw_back_size ;
    unsigned int bi_max_vecs ;
    struct bio_vec *bi_io_vec ;
    bio_end_io_t *bi_end_io ;
    atomic_t bi_cnt ;
    void *bi_private ;
    bio_destructor_t *bi_destructor ;
};
typedef struct request_queue request_queue_t;
struct elevator_queue;
typedef struct elevator_queue elevator_t;
struct as_io_context {
    spinlock_t lock ;
    void (*dtor)(struct as_io_context *aic ) ;
    void (*exit)(struct as_io_context *aic ) ;
    unsigned long state ;
    atomic_t nr_queued ;
    atomic_t nr_dispatched ;
    unsigned long last_end_request ;
    unsigned long ttime_total ;
    unsigned long ttime_samples ;
    unsigned long ttime_mean ;
    unsigned int seek_samples ;
    sector_t last_request_pos ;
    u64 seek_total ;
    sector_t seek_mean ;
};
struct cfq_io_context {
    struct list_head list ;
    struct cfq_queue *cfqq ;
    void *key ;
    struct io_context *ioc ;
    unsigned long last_end_request ;
    unsigned long last_queue ;
    unsigned long ttime_total ;
    unsigned long ttime_samples ;
    unsigned long ttime_mean ;
    void (*dtor)(struct cfq_io_context * ) ;
    void (*exit)(struct cfq_io_context * ) ;
};
struct io_context {
    atomic_t refcount ;
    struct task_struct *task ;
    int (*set_ioprio)(struct io_context * , unsigned int ) ;
    unsigned long last_waited ;
    int nr_batch_requests ;
    struct as_io_context *aic ;
    struct cfq_io_context *cic ;
};
typedef void rq_end_io_fn(struct request * , int );
struct request_list {
    int count[2] ;
    int starved[2] ;
    int elvpriv ;
    mempool_t *rq_pool ;
    wait_queue_head_t wait[2] ;
};
struct request {
    struct list_head queuelist ;
    struct list_head donelist ;
    unsigned long flags ;
    sector_t sector ;
    unsigned long nr_sectors ;
    unsigned int current_nr_sectors ;
    sector_t hard_sector ;
    unsigned long hard_nr_sectors ;
    unsigned int hard_cur_sectors ;
    struct bio *bio ;
    struct bio *biotail ;
    void *elevator_private ;
    void *completion_data ;
    unsigned short ioprio ;
    int rq_status ;
    struct gendisk *rq_disk ;
    int errors ;
    unsigned long start_time ;
    unsigned short nr_phys_segments ;
    unsigned short nr_hw_segments ;
    int tag ;
    char *buffer ;
    int ref_count ;
    request_queue_t *q ;
    struct request_list *rl ;
    struct completion *waiting ;
    void *special ;
    unsigned int cmd_len ;
    unsigned char cmd[16] ;
    unsigned int data_len ;
    void *data ;
    unsigned int sense_len ;
    struct request_sense *sense ;
    unsigned int timeout ;
    int retries ;
    struct request_pm_state *pm ;
    rq_end_io_fn *end_io ;
    void *end_io_data ;
};
struct request_pm_state {
    int pm_step ;
    u32 pm_state ;
    void *data ;
};
typedef int elevator_merge_fn(request_queue_t * , struct request ** , struct bio * );
typedef void elevator_merge_req_fn(request_queue_t * , struct request * , struct request * );
typedef void elevator_merged_fn(request_queue_t * , struct request * );
typedef int elevator_dispatch_fn(request_queue_t * , int );
typedef void elevator_add_req_fn(request_queue_t * , struct request * );
typedef int elevator_queue_empty_fn(request_queue_t * );
typedef struct request *elevator_request_list_fn(request_queue_t * , struct request * );
typedef void elevator_completed_req_fn(request_queue_t * , struct request * );
typedef int elevator_may_queue_fn(request_queue_t * , int , struct bio * );
typedef int elevator_set_req_fn(request_queue_t * , struct request * , struct bio * ,
        gfp_t );
typedef void elevator_put_req_fn(request_queue_t * , struct request * );
typedef void elevator_activate_req_fn(request_queue_t * , struct request * );
typedef void elevator_deactivate_req_fn(request_queue_t * , struct request * );
typedef int elevator_init_fn(request_queue_t * , elevator_t * );
typedef void elevator_exit_fn(elevator_t * );
struct elevator_ops {
    elevator_merge_fn *elevator_merge_fn ;
    elevator_merged_fn *elevator_merged_fn ;
    elevator_merge_req_fn *elevator_merge_req_fn ;
    elevator_dispatch_fn *elevator_dispatch_fn ;
    elevator_add_req_fn *elevator_add_req_fn ;
    elevator_activate_req_fn *elevator_activate_req_fn ;
    elevator_deactivate_req_fn *elevator_deactivate_req_fn ;
    elevator_queue_empty_fn *elevator_queue_empty_fn ;
    elevator_completed_req_fn *elevator_completed_req_fn ;
    elevator_request_list_fn *elevator_former_req_fn ;
    elevator_request_list_fn *elevator_latter_req_fn ;
    elevator_set_req_fn *elevator_set_req_fn ;
    elevator_put_req_fn *elevator_put_req_fn ;
    elevator_may_queue_fn *elevator_may_queue_fn ;
    elevator_init_fn *elevator_init_fn ;
    elevator_exit_fn *elevator_exit_fn ;
};
struct elevator_type {
    struct list_head list ;
    struct elevator_ops ops ;
    struct elevator_type *elevator_type ;
    struct kobj_type *elevator_ktype ;
    char elevator_name[16] ;
    struct module *elevator_owner ;
};
struct elevator_queue {
    struct elevator_ops *ops ;
    void *elevator_data ;
    struct kobject kobj ;
    struct elevator_type *elevator_type ;
};
typedef int merge_request_fn(request_queue_t * , struct request * , struct bio * );
typedef int merge_requests_fn(request_queue_t * , struct request * , struct request * );
typedef void request_fn_proc(request_queue_t *q );
typedef int make_request_fn(request_queue_t *q , struct bio *bio );
typedef int prep_rq_fn(request_queue_t * , struct request * );
typedef void unplug_fn(request_queue_t * );
typedef int merge_bvec_fn(request_queue_t * , struct bio * , struct bio_vec * );
typedef void activity_fn(void *data , int rw );
typedef int issue_flush_fn(request_queue_t * , struct gendisk * , sector_t * );
typedef void prepare_flush_fn(request_queue_t * , struct request * );
typedef void softirq_done_fn(struct request * );
struct blk_queue_tag {
    struct request **tag_index ;
    unsigned long *tag_map ;
    struct list_head busy_list ;
    int busy ;
    int max_depth ;
    int real_max_depth ;
    atomic_t refcnt ;
};
struct request_queue {
    struct list_head queue_head ;
    struct request *last_merge ;
    elevator_t *elevator ;
    struct request_list rq ;
    request_fn_proc *request_fn ;
    merge_request_fn *back_merge_fn ;
    merge_request_fn *front_merge_fn ;
    merge_requests_fn *merge_requests_fn ;
    make_request_fn *make_request_fn ;
    prep_rq_fn *prep_rq_fn ;
    unplug_fn *unplug_fn ;
    merge_bvec_fn *merge_bvec_fn ;
    activity_fn *activity_fn ;
    issue_flush_fn *issue_flush_fn ;
    prepare_flush_fn *prepare_flush_fn ;
    softirq_done_fn *softirq_done_fn ;
    sector_t end_sector ;
    struct request *boundary_rq ;
    struct timer_list unplug_timer ;
    int unplug_thresh ;
    unsigned long unplug_delay ;
    struct work_struct unplug_work ;
    struct backing_dev_info backing_dev_info ;
    void *queuedata ;
    void *activity_data ;
    unsigned long bounce_pfn ;
    gfp_t bounce_gfp ;
    unsigned long queue_flags ;
    spinlock_t __queue_lock ;
    spinlock_t *queue_lock ;
    struct kobject kobj ;
    unsigned long nr_requests ;
    unsigned int nr_congestion_on ;
    unsigned int nr_congestion_off ;
    unsigned int nr_batching ;
    unsigned int max_sectors ;
    unsigned int max_hw_sectors ;
    unsigned short max_phys_segments ;
    unsigned short max_hw_segments ;
    unsigned short hardsect_size ;
    unsigned int max_segment_size ;
    unsigned long seg_boundary_mask ;
    unsigned int dma_alignment ;
    struct blk_queue_tag *queue_tags ;
    atomic_t refcnt ;
    unsigned int nr_sorted ;
    unsigned int in_flight ;
    unsigned int sg_timeout ;
    unsigned int sg_reserved_size ;
    int node ;
    unsigned int ordered ;
    unsigned int next_ordered ;
    unsigned int ordseq ;
    int orderr ;
    int ordcolor ;
    struct request pre_flush_rq ;
    struct request bar_rq ;
    struct request post_flush_rq ;
    struct request *orig_bar_rq ;
    unsigned int bi_size ;
};
struct cdrom_sysctl_settings {
    char info[1000] ;
    int autoclose ;
    int autoeject ;
    int debug ;
    int lock ;
    int check ;
};

int capable(int cap )
{
    int a; return a;
}
int HsPrintk(char *format, ...)
{
    int a; return a;
}
extern int check_disk_change(struct block_device *x)
{
    int a; return a;
}
void invalidate_bdev(struct block_device *x, int y)
{
    return ;
}
extern void blk_queue_bounce(request_queue_t *q , struct bio **bio)
{
    return;
}
void blk_put_request(struct request *req)
{
    return;
}
struct request *blk_get_request(struct request_queue *q , int rw , gfp_t gfp_mask)
{
    struct request *a; return a;
}
int scsi_cmd_ioctl(struct file *x, struct gendisk *y, unsigned int u, void *v)
{
    int a; return a;
}
int blk_rq_map_user(struct request_queue *q, struct request *rq, void *ubuf, unsigned int len)
{
    int a; return a;
}
int blk_rq_unmap_user(struct bio *bio , unsigned int ulen )
{
    int a; return a;
}
int blk_execute_rq(request_queue_t *q , struct gendisk *bd_disk , struct request *rq , int at_head )
{
    int a; return a;
}

__inline static struct thread_info *( __attribute__((__always_inline__)) current_thread_info)(void)
{


}
__inline static __u32 ( __attribute__((__always_inline__)) ___arch__swab32)(__u32 x )
{
    __u32 t ;

    x = (x << 24) | (x >> 8);
    t &= 4278255615U;
    x ^= t >> 8;
    return (x);
}
__inline static __u32 ( __attribute__((__always_inline__)) __fswab32)(__u32 x )
{
    __u32 tmp ;
    tmp = ___arch__swab32(x);
    return (tmp);
}
__inline static struct task_struct *( __attribute__((__always_inline__)) get_current)(void)
{
    struct thread_info *tmp ;
    tmp = current_thread_info();
    return (tmp->task);
}
void *HsMemcpy(void *x, void *y, size_t z)
{
    void *a;
    return a;
}
void __memzero(void *x, size_t y)
{
    return;
}
unsigned long __arch_copy_from_user(void *x, void *y, unsigned long z)
{
    unsigned long a;
    return a;
}
unsigned long __arch_copy_to_user(void *x, void *y, unsigned long z)
{
    unsigned long a;
    return a;
}
__inline static unsigned long ( __attribute__((__always_inline__)) copy_from_user)(void *to ,
        void *from ,
        unsigned long n )
{
    void *__p ;
    size_t __n ;
    unsigned long flag ;
    unsigned long sum ;
    struct thread_info *tmp ;
    tmp = current_thread_info();


    if (flag == 0UL) {
        n = __arch_copy_from_user(to, from, n);
    } else {
        __p = to;
        __n = (size_t )n;
        if (__n != 0U) {
            __memzero(__p, __n);
        }
    }
    return (n);
}
__inline static unsigned long ( __attribute__((__always_inline__)) copy_to_user)(void *to ,
        void *from ,
        unsigned long n )
{
    unsigned long flag ;
    unsigned long sum ;
    struct thread_info *tmp ;
    tmp = current_thread_info();


    if (flag == 0UL) {
        n = __arch_copy_to_user(to, from, n);
    }
    return (n);
}
__inline static unsigned long ( __attribute__((__always_inline__)) __copy_to_user)(void *to ,
        void *from ,
        unsigned long n )
{
    unsigned long tmp ;
    tmp = __arch_copy_to_user(to, from, n);
    return (tmp);
}
__inline static int ( __attribute__((__always_inline__)) msf_to_lba)(char m , char s , char f )
{
    return ((((int )m * 60 + (int )s) * 75 + (int )f) - 150);
}

static int debug ;
static int keeplocked ;
static int autoclose = 1;
static int autoeject ;
static int lockdoor = 1;
static int check_media_type ;
static int mrw_format_restart = 1;
static char *mrw_format_status[4] = { "not mrw", "bgformat inactive", "bgformat active", "mrw complete"};
static char *mrw_address_space[2] = { "DMA", "GAA"};

static struct cdrom_sysctl_settings cdrom_sysctl_settings;
static ctl_table *cdrom_root_table;
static struct ctl_table_header *cdrom_sysctl_header;
static int initialized;

static struct cdrom_device_info *topCdromPtr;
static char banner_printed;

struct ctl_table_header *register_sysctl_table(ctl_table *table , int insert_at_head)
{
    struct ctl_table_header *cdrom_sysctl_header ;
    struct ctl_table_header *tmp ;
    tmp = (struct ctl_table_header *)mallocnull(sizeof(struct ctl_table_header ));
    cdrom_sysctl_header = tmp;




    if (tmp == 0) return tmp;

    cdrom_sysctl_header->ctl_table = table;
    return (cdrom_sysctl_header);
}

static int cdrom_sysctl_register(void)
{
    int ret;

    if (initialized == 1) {
        return 0;
    }
    cdrom_sysctl_header = register_sysctl_table(cdrom_root_table, 1);
    if (cdrom_sysctl_header == 0) return -2;

    if (cdrom_root_table->ctl_name) {
        if ((cdrom_root_table->child)->de) {
            ((cdrom_root_table->child)->de)->owner = (struct module *)0;
        }
    }
    cdrom_sysctl_settings.autoclose = autoclose;
    cdrom_sysctl_settings.autoeject = autoeject;
    cdrom_sysctl_settings.debug = debug;
    cdrom_sysctl_settings.lock = lockdoor;
    cdrom_sysctl_settings.check = check_media_type;
    initialized = 1;
    return 0;
}

static int cdrom_init(void)
{
    int ret;
    ret = cdrom_sysctl_register();
    return ret;
}

static int cdrom_dummy_generic_packet(struct cdrom_device_info *cdi , struct packet_command *cgc )
{
    if (cgc->sense) {
        (cgc->sense)->sense_key = (unsigned char)5;
        (cgc->sense)->asc = (unsigned char)32;
        (cgc->sense)->ascq = (unsigned char)0;
    }
    cgc->stat = -5;
    return (-5);
}

static int cdrom_mrw_exit(struct cdrom_device_info *cdi ) ;

int register_cdrom(struct cdrom_device_info *cdi )
{
    struct cdrom_device_ops *cdo ;
    int *change_capability ;

    cdo = cdi->ops;
    change_capability = (int *)(& cdo->capability);
    if (debug == 1) {
        HsPrintk("<6>cdrom: entering register_cdrom\n");
    }
    if ((unsigned int )cdo->open == (unsigned int )((void *)0)) {
        return (-2);
    } else {
        if ((unsigned int )cdo->release == (unsigned int )((void *)0)) {
            return (-2);
        }
    }
    if (! banner_printed) {
        HsPrintk("<6>Uniform CD-ROM driver Revision: 3.20\n");
        banner_printed = (char)1;
        cdrom_sysctl_register();
    }
    if ((unsigned int )cdo->drive_status == (unsigned int )((void *)0)) {
        (*change_capability) &= -2049;
    }
    if ((unsigned int )cdo->media_changed == (unsigned int )((void *)0)) {
        (*change_capability) &= -129;
    }
    if ((unsigned int )cdo->tray_move == (unsigned int )((void *)0)) {
        (*change_capability) &= -4;
    }
    if ((unsigned int )cdo->lock_door == (unsigned int )((void *)0)) {
        (*change_capability) &= -5;
    }
    if ((unsigned int )cdo->select_speed == (unsigned int )((void *)0)) {
        (*change_capability) &= -9;
    }
    if ((unsigned int )cdo->get_last_session == (unsigned int )((void *)0)) {
        (*change_capability) &= -33;
    }
    if ((unsigned int )cdo->get_mcn == (unsigned int )((void *)0)) {
        (*change_capability) &= -65;
    }
    if ((unsigned int )cdo->reset == (unsigned int )((void *)0)) {
        (*change_capability) &= -513;
    }
    if ((unsigned int )cdo->audio_ioctl == (unsigned int )((void *)0)) {
        (*change_capability) &= -257;
    }
    if ((unsigned int )cdo->dev_ioctl == (unsigned int )((void *)0)) {
        (*change_capability) &= -1025;
    }
    if ((unsigned int )cdo->generic_packet == (unsigned int )((void *)0)) {
        (*change_capability) &= -4097;
    }
    cdi->mc_flags = 0U;
    cdo->n_minors = 0;
    cdi->options = 4;
    if (autoclose == 1) {
        if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 1) {
            cdi->options |= 1;
        }
    }
    if (autoeject == 1) {
        if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 2) {
            cdi->options |= 2;
        }
    }
    if (lockdoor == 1) {
        cdi->options |= 8;
    }
    if (check_media_type == 1) {
        cdi->options |= 16;
    }
    if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 1048576) {
        cdi->exit = & cdrom_mrw_exit;
    }
    if (cdi->disk) {
        cdi->cdda_method = 2;
    } else {
        cdi->cdda_method = 0;
    }
    if (! cdo->generic_packet) {
        cdo->generic_packet = & cdrom_dummy_generic_packet;
    }
    if (debug == 1) {
        HsPrintk("<6>cdrom: drive \"/dev/%s\" registered\n", cdi->name);
    }
    cdi->next = topCdromPtr;
    topCdromPtr = cdi;
    return (0);
}

void init_cdrom_command(struct packet_command *cgc, void *buf, int len, int type)
{
    void *__p ;
    size_t __n ;
    void *__p___0 ;
    size_t __n___0 ;

    __p = (void *)cgc;
    __n = sizeof(struct packet_command );
    if (__n != 0U) {
        __memzero(__p, __n);
    }
    if (buf) {
        __p___0 = buf;
        __n___0 = (size_t )len;
        if (__n___0 != 0U) {
            __memzero(__p___0, __n___0);
        }
    }
    cgc->buffer = (unsigned char *)((char *)buf);
    cgc->buflen = (unsigned int )len;
    cgc->data_direction = (unsigned char )type;
    cgc->timeout = 500;
    return;
}


static int cdrom_get_disc_info(struct cdrom_device_info *cdi , disc_information *di ) ;

static int cdrom_mrw_bgformat_susp(struct cdrom_device_info *cdi , int immed )
{
    struct packet_command cgc ;
    int tmp ;

    init_cdrom_command(& cgc, (void *)0, 0, 3);
    cgc.cmd[0] = (unsigned char)91;
    cgc.cmd[1] = (unsigned char )(! (! immed));
    cgc.cmd[2] = (unsigned char)2;
    cgc.timeout = 30000;

    tmp = get_nondet_int();
    return (tmp);
}

static int cdrom_flush_cache(struct cdrom_device_info *cdi )
{
    struct packet_command cgc ;
    int tmp ;

    init_cdrom_command(& cgc, (void *)0, 0, 3);
    cgc.cmd[0] = (unsigned char)53;
    cgc.timeout = 30000;

    tmp = get_nondet_int();

    return (tmp);
}

static int cdrom_mrw_exit(struct cdrom_device_info *cdi )
{
    disc_information di ;
    int ret ;

    ret = cdrom_get_disc_info(cdi, & di);
    if (ret < 0) {
        return (1);
    } else {
        if (ret < (int )((unsigned int )(& ((disc_information *)0)->disc_type))) {
            return (1);
        }
    }
    ret = 0;
    if ((int )di.mrw_status == 2) {
        HsPrintk("<6>cdrom: issuing MRW back ground format suspend\n");
        ret = cdrom_mrw_bgformat_susp(cdi, 0);
    }
    if (! ret) {
        if (cdi->media_written) {
            ret = cdrom_flush_cache(cdi);
        }
    }
    return (ret);
}

int unregister_cdrom(struct cdrom_device_info *unreg )
{
    struct cdrom_device_info *cdi ;
    struct cdrom_device_info *prev ;

    if (debug == 1) {
        HsPrintk("<6>cdrom: entering unregister_cdrom\n");
    }
    prev = (struct cdrom_device_info *)((void *)0);
    cdi = topCdromPtr;
    while (1) {
        if (cdi) {
            if (! ((unsigned int )cdi != (unsigned int )unreg)) {
                break;
            }
        } else {
            break;
        }
        prev = cdi;
        cdi = cdi->next;
    }
    if ((unsigned int )cdi == (unsigned int )((void *)0)) {
        return (-2);
    }
    if (prev) {
        prev->next = cdi->next;
    } else {
        topCdromPtr = cdi->next;
    }
    if (cdi->exit) {

        cdrom_mrw_exit(cdi);
    }
    (cdi->ops)->n_minors --;
    if (debug == 1) {
        HsPrintk("<6>cdrom: drive \"/dev/%s\" unregistered\n", cdi->name);
    }
    return (0);
}

__u16 __fswab16(__u16 x )
{
    __u16 __tmp ;
    __u16 __x ;
    __tmp = x;
    __x = __tmp;
    return ((unsigned short )((((int )__x & 255) << 8) | (((int )__x & 65280) >> 8)));
}

int cdrom_get_media_event(struct cdrom_device_info *cdi , struct media_event_desc *med )
{


    struct event_header *buffer;
    struct event_header sbuffer;
    buffer = &sbuffer;

    struct packet_command cgc;
    struct event_header *eh;
    int tmp;
    __u16 tmp___0;

    struct request_sense rs;

    eh = (struct event_header *)(buffer);
    init_cdrom_command(& cgc, (void *)(buffer), (int )sizeof(buffer), 2);
    cgc.cmd[0] = (unsigned char)74;
    cgc.cmd[1] = (unsigned char)1;
    cgc.cmd[4] = (unsigned char)16;
    cgc.cmd[8] = (unsigned char )sizeof(buffer);
    cgc.quiet = 1;


    cgc.sense = &rs;




    tmp = cdrom_dummy_generic_packet(cdi, &cgc);
    if (tmp) {
        return (1);
    }
    tmp___0 = __fswab16(eh->data_len);
    if ((unsigned int )tmp___0 < sizeof((*med))) {
        return (1);
    }
    if (eh->nea) {
        return (1);
    } else {
        if ((int )eh->notification_class != 4) {
            return (1);
        }
    }
    HsMemcpy((void *)med, (void *)(& buffer[sizeof((*eh))]), sizeof((*med)));
    return (0);
}

void unregister_sysctl_table(struct ctl_table_header *header)
{
    ctl_table *table, *temp;
    struct proc_dir_entry *de;
    table = header->ctl_table;
    while (table != 0) {
        de = table->de;
        if (de) {
            free(de);
        }
        temp = table;
        table = table->child;
        free(temp);
    }
    free(header);
}

static void cdrom_sysctl_unregister(void)
{
    if (cdrom_sysctl_header) {
        unregister_sysctl_table(cdrom_sysctl_header);
    }
    return;
}

static void cdrom_exit(void)
{
    HsPrintk("<6>Uniform CD-ROM driver unloaded\n");
    cdrom_sysctl_unregister();
    return;
}

static void cdrom_count_tracks(struct cdrom_device_info *cdi , tracktype *tracks )
{
    struct cdrom_tochdr header ;
    struct cdrom_tocentry entry ;
    int ret ;
    int i ;
    int tmp ;

    tracks->data = 0;
    tracks->audio = 0;
    tracks->cdi = 0;
    tracks->xa = 0;
    tracks->error = 0L;
    if (debug == 1) {
        HsPrintk("<6>cdrom: entering cdrom_count_tracks\n");
    }
    if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 256)) {
        tracks->error = 0L;
        return;
    }

    ret = get_nondet_int();
    if (ret) {
        if (ret == -123) {
            tracks->error = 1L;
        } else {
            tracks->error = 0L;
        }
        return;
    }
    entry.cdte_format = (unsigned char)2;
    i = (int )header.cdth_trk0;
    while (i <= (int )header.cdth_trk1) {
        entry.cdte_track = (unsigned char )i;

        tmp = get_nondet_int();
        if (tmp) {
            tracks->error = 0L;
            return;
        }
        if ((int )entry.cdte_ctrl & 4) {
            if ((int )entry.cdte_format == 16) {
                tracks->cdi ++;
            } else {
                if ((int )entry.cdte_format == 32) {
                    tracks->xa ++;
                } else {
                    tracks->data ++;
                }
            }
        } else {
            tracks->audio ++;
        }
        if (debug == 1) {
            HsPrintk("<6>cdrom: track %d: format=%d, ctrl=%d\n", i, entry.cdte_format, entry.cdte_ctrl);
        }
        i ++;
    }
    if (debug == 1) {
        HsPrintk("<6>cdrom: disc has %d tracks: %d=audio %d=data %d=Cd-I %d=XA\n", header.cdth_trk1,
                tracks->audio, tracks->data, tracks->cdi, tracks->xa);
    }
    return;
}

static int open_for_data(struct cdrom_device_info *cdi )
{
    int ret ;
    struct cdrom_device_ops *cdo ;
    tracktype tracks ;
    struct task_struct *tmp ;

    cdo = cdi->ops;
    if (debug == 1) {
        HsPrintk("<6>cdrom: entering open_for_data\n");
    }
    if ((unsigned int )cdo->drive_status != (unsigned int )((void *)0)) {

        ret = get_nondet_int();
        if (debug == 1) {
            HsPrintk("<6>cdrom: drive_status=%d\n", ret);
        }
        if (ret == 2) {
            if (debug == 1) {
                HsPrintk("<6>cdrom: the tray is open...\n");
            }
            if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 1) {
                if (cdi->options & 1) {
                    if (debug == 1) {
                        HsPrintk("<6>cdrom: trying to close the tray.\n");
                    }

                    ret = get_nondet_int();
                    if (ret) {
                        if (debug == 1) {
                            HsPrintk("<6>cdrom: bummer. tried to close the tray but failed.\n");
                        }
                        ret = -123;
                        goto clean_up_and_return;
                    }
                } else {
                    goto _L;
                }
            } else {
_L:
                if (debug == 1) {
                    HsPrintk("<6>cdrom: bummer. this drive can\'t close the tray.\n");
                }
                ret = -123;
                goto clean_up_and_return;
            }

            ret = get_nondet_int();
            if (ret == 1) {
                goto _L___0;
            } else {
                if (ret == 2) {
_L___0:
                    if (debug == 1) {
                        HsPrintk("<6>cdrom: bummer. the tray is still not closed.\n");
                    }
                    if (debug == 1) {
                        HsPrintk("<6>cdrom: tray might not contain a medium.\n");
                    }
                    ret = -123;
                    goto clean_up_and_return;
                }
            }
            if (debug == 1) {
                HsPrintk("<6>cdrom: the tray is now closed.\n");
            }
        }

        ret = get_nondet_int();
        if (ret != 4) {
            ret = -123;
            goto clean_up_and_return;
        }
    }
    cdrom_count_tracks(cdi, & tracks);
    if (tracks.error == 1L) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: bummer. no disc.\n");
        }
        ret = -123;
        goto clean_up_and_return;
    }
    if (tracks.data == 0) {
        if (cdi->options & 16) {
            if (debug == 1) {
                HsPrintk("<6>cdrom: bummer. wrong media type.\n");
            }


            ret = -124;
        }
        goto clean_up_and_return;
    } else {
        if (debug == 1) {
            HsPrintk("<6>cdrom: wrong media type, but CDO_CHECK_TYPE not set.\n");
        }
    }
    if (debug == 1) {
        HsPrintk("<6>cdrom: all seems well, opening the device.\n");
    }

    ret = get_nondet_int();
    if (debug == 1) {
        HsPrintk("<6>cdrom: opening the device gave me %d.\n", ret);
    }
    if (ret) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: open device failed.\n");
        }
        goto clean_up_and_return;
    }
    if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 4) {
        if (cdi->options & 8) {

            if (debug == 1) {
                HsPrintk("<6>cdrom: door locked.\n");
            }
        }
    }
    if (debug == 1) {
        HsPrintk("<6>cdrom: device opened successfully.\n");
    }
    return (ret);

clean_up_and_return:
    if (debug == 1) {
        HsPrintk("<6>cdrom: open failed.\n");
    }
    if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 4) {
        if (cdi->options & 8) {

            if (debug == 1) {
                HsPrintk("<6>cdrom: door unlocked.\n");
            }
        }
    }
    return (ret);
}

static void cdrom_mmc3_profile(struct cdrom_device_info *cdi )
{
    struct packet_command cgc ;
    char buffer[32] ;
    int ret ;
    int mmc3_profile ;

    init_cdrom_command(& cgc, (void *)(buffer), (int )sizeof(buffer), 2);
    cgc.cmd[0] = (unsigned char)70;
    cgc.cmd[1] = (unsigned char)0;
    cgc.cmd[3] = (unsigned char)0;
    cgc.cmd[2] = cgc.cmd[3];
    cgc.cmd[8] = (unsigned char )sizeof(buffer);
    cgc.quiet = 1;

    ret = get_nondet_int();
    if (ret) {
        mmc3_profile = 65535;
    } else {
        mmc3_profile = ((int )buffer[6] << 8) | (int )buffer[7];
    }
    cdi->mmc3_profile = (unsigned short )mmc3_profile;
    return;
}

int cdrom_mode_sense(struct cdrom_device_info *cdi , struct packet_command *cgc ,
        int page_code , int page_control )
{
    struct cdrom_device_ops *cdo ;
    void *__p ;
    size_t __n ;
    int tmp ;

    cdo = cdi->ops;

    __p = get_nondet_ptr();
    __n = sizeof(cgc->cmd);
    if (__n != 0U) {
        __memzero(__p, __n);
    }
    cgc->cmd[0] = (unsigned char)90;
    cgc->cmd[2] = (unsigned char )(page_code | (page_control << 6));
    cgc->cmd[7] = (unsigned char )(cgc->buflen >> 8);
    cgc->cmd[8] = (unsigned char )(cgc->buflen & 255U);
    cgc->data_direction = (unsigned char)2;

    tmp = get_nondet_int();
    return (tmp);
}

static int cdrom_mrw_probe_pc(struct cdrom_device_info *cdi )
{
    struct packet_command cgc ;
    char buffer[16] ;
    int tmp ;
    int tmp___0 ;

    init_cdrom_command(& cgc, (void *)(buffer), (int )sizeof(buffer), 2);
    cgc.timeout = 100;
    cgc.quiet = 1;
    tmp___0 = cdrom_mode_sense(cdi, & cgc, 3, 0);

    if (tmp___0) {
        tmp = cdrom_mode_sense(cdi, & cgc, 44, 0);
        if (! tmp) {
            cdi->mrw_mode_page = 44;
            return (0);
        }
    } else {
        cdi->mrw_mode_page = 3;
        return (0);
    }
    return (1);
}

static int cdrom_is_mrw(struct cdrom_device_info *cdi , int *write )
{
    struct packet_command cgc ;
    struct mrw_feature_desc *mfd ;
    unsigned char buffer[16] ;
    int ret ;
    __u16 tmp ;

    (*write) = 0;
    init_cdrom_command(& cgc, (void *)(buffer), (int )sizeof(buffer), 2);
    cgc.cmd[0] = (unsigned char)70;
    cgc.cmd[3] = (unsigned char)40;
    cgc.cmd[8] = (unsigned char )sizeof(buffer);
    cgc.quiet = 1;

    ret = get_nondet_int();
    if (ret) {
        return (ret);
    }
    mfd = (struct mrw_feature_desc *)(& buffer[sizeof(struct feature_header )]);
    tmp = __fswab16(mfd->feature_code);
    if ((int )tmp != 40) {
        return (1);
    }
    (*write) = (int )mfd->write;
    ret = cdrom_mrw_probe_pc(cdi);
    if (ret) {
        (*write) = 0;
        return (ret);
    }
    return (0);
}

static int cdrom_get_random_writable(struct cdrom_device_info *cdi , struct rwrt_feature_desc *rfd )
{
    struct packet_command cgc ;
    char buffer[24] ;
    int ret ;

    init_cdrom_command(& cgc, (void *)(buffer), (int )sizeof(buffer), 2);
    cgc.cmd[0] = (unsigned char)70;
    cgc.cmd[3] = (unsigned char)32;
    cgc.cmd[8] = (unsigned char )sizeof(buffer);
    cgc.quiet = 1;

    ret = get_nondet_int();
    if (ret) {
        return (ret);
    }
    HsMemcpy((void *)rfd, (void *)(& buffer[sizeof(struct feature_header )]),
            sizeof((*rfd)));
    return (0);
}

static int cdrom_is_random_writable(struct cdrom_device_info *cdi , int *write )
{
    struct rwrt_feature_desc rfd ;
    int ret ;
    __u16 tmp ;

    (*write) = 0;
    ret = cdrom_get_random_writable(cdi, & rfd);
    if (ret) {
        return (ret);
    }
    tmp = __fswab16(rfd.feature_code);
    if (32 == (int )tmp) {
        (*write) = 1;
    }
    return (0);
}

int cdrom_mode_select(struct cdrom_device_info *cdi , struct packet_command *cgc )
{
    struct cdrom_device_ops *cdo ;
    void *__p ;
    size_t __n ;
    void *__p___0 ;
    size_t __n___0 ;
    int tmp ;

    cdo = cdi->ops;
    __p = (void *)(cgc->cmd);
    __n = sizeof(cgc->cmd);
    if (__n != 0U) {
        __memzero(__p, __n);
    }
    __p___0 = (void *)cgc->buffer;
    __n___0 = (size_t )2;
    if (__n___0 != 0U) {
        __memzero(__p___0, __n___0);
    }
    cgc->cmd[0] = (unsigned char)85;
    cgc->cmd[1] = (unsigned char)16;
    cgc->cmd[7] = (unsigned char )(cgc->buflen >> 8);
    cgc->cmd[8] = (unsigned char )(cgc->buflen & 255U);
    cgc->data_direction = (unsigned char)1;

    tmp = get_nondet_int();
    return (tmp);
}

static int cdrom_mrw_set_lba_space(struct cdrom_device_info *cdi , int space )
{
    struct packet_command cgc ;
    struct mode_page_header *mph ;
    struct mode_page_header rbuffer;
    struct mode_page_header *buffer;

    int ret ;
    int offset ;
    int size ;
    __u16 tmp ;

    buffer = &rbuffer;

    init_cdrom_command(& cgc, (void *)(buffer), (int )sizeof(buffer), 2);
    cgc.buffer = (unsigned char *)(buffer);
    cgc.buflen = sizeof(buffer);
    ret = cdrom_mode_sense(cdi, & cgc, cdi->mrw_mode_page, 0);
    if (ret) {
        return (ret);
    }
    mph = (struct mode_page_header *)(buffer);
    offset = (int )__fswab16(mph->desc_length);
    tmp = __fswab16(mph->mode_data_length);
    size = (int )tmp + 2;
    ((char *)buffer)[offset + 3] = (char )space;
    cgc.buflen = (unsigned int )size;
    ret = cdrom_mode_select(cdi, & cgc);
    if (ret) {
        return (ret);
    }
    HsPrintk("<6>cdrom: %s: mrw address space %s selected\n", cdi->name, mrw_address_space[space]);
    return (0);
}

static int cdrom_get_disc_info(struct cdrom_device_info *cdi , disc_information *di )
{
    struct cdrom_device_ops *cdo ;
    struct packet_command cgc ;
    int ret ;
    int buflen ;
    __u16 tmp ;

    cdo = cdi->ops;
    init_cdrom_command(& cgc, (void *)di, (int )sizeof((*di)), 2);
    cgc.cmd[0] = (unsigned char)81;
    cgc.buflen = 2U;
    cgc.cmd[8] = (unsigned char )cgc.buflen;
    cgc.quiet = 1;

    ret = get_nondet_int();
    if (ret) {
        return (ret);
    }
    tmp = __fswab16(di->disc_information_length);
    buflen = (int )((unsigned int )tmp + sizeof(di->disc_information_length));
    if ((unsigned int )buflen > sizeof(disc_information )) {
        buflen = (int )sizeof(disc_information );
    }
    cgc.buflen = (unsigned int )buflen;
    cgc.cmd[8] = (unsigned char )cgc.buflen;

    ret = get_nondet_int();
    if (ret) {
        return (ret);
    }
    return (buflen);
}

static int cdrom_mrw_bgformat(struct cdrom_device_info *cdi , int cont )
{
    struct packet_command cgc ;
    unsigned char buffer[12] ;
    int ret ;
    char *tmp ;

    if (cont) {
        tmp = "Re";
    } else {
        tmp = "";
    }
    HsPrintk("<6>cdrom: %sstarting format\n", tmp);
    init_cdrom_command(& cgc, (void *)(buffer), (int )sizeof(buffer), 1);
    cgc.cmd[0] = (unsigned char)4;
    cgc.cmd[1] = (unsigned char)17;
    cgc.timeout = 30000;
    buffer[1] = (unsigned char)2;
    buffer[3] = (unsigned char)8;
    buffer[4] = (unsigned char)255;
    buffer[5] = (unsigned char)255;
    buffer[6] = (unsigned char)255;
    buffer[7] = (unsigned char)255;
    buffer[8] = (unsigned char)144;
    buffer[11] = (unsigned char )cont;

    ret = get_nondet_int();
    if (ret) {
        HsPrintk("<6>cdrom: bgformat failed\n");
    }
    return (ret);
}

static int cdrom_mrw_open_write(struct cdrom_device_info *cdi )
{
    disc_information di ;
    int ret ;
    int tmp ;

    tmp = cdrom_mrw_set_lba_space(cdi, 0);
    if (tmp) {
        HsPrintk("<3>cdrom: failed setting lba address space\n");
        return (1);
    }
    ret = cdrom_get_disc_info(cdi, & di);
    if (ret < 0) {
        return (1);
    } else {
        if ((unsigned int )ret < (unsigned int )(& ((disc_information *)0)->disc_type)) {
            return (1);
        }
    }
    if (! di.erasable) {
        return (1);
    }
    ret = 0;
    HsPrintk("<6>cdrom open: mrw_status \'%s\'\n", mrw_format_status[di.mrw_status]);
    if (! di.mrw_status) {
        ret = 1;
    } else {
        if ((int )di.mrw_status == 1) {
            if (mrw_format_restart) {
                ret = cdrom_mrw_bgformat(cdi, 1);
            }
        }
    }
    return (ret);
}

static int cdrom_media_erasable(struct cdrom_device_info *cdi )
{
    disc_information di ;
    int ret ;

    ret = cdrom_get_disc_info(cdi, & di);

    if (ret < 0) {
        return (-1);
    } else {
        if ((unsigned int )ret < (unsigned int )(& ((disc_information *)0)->n_first_track)) {
            return (-1);
        }
    }
    return ((int )di.erasable);
}

static int cdrom_dvdram_open_write(struct cdrom_device_info *cdi )
{
    int ret ;
    int tmp ;

    tmp = cdrom_media_erasable(cdi);
    ret = tmp;
    if (! ret) {
        return (1);
    }
    return (0);
}

static int cdrom_has_defect_mgt(struct cdrom_device_info *cdi )
{
    struct packet_command cgc ;
    char buffer[16] ;
    __u16 *feature_code ;
    int ret ;
    __u16 tmp ;

    init_cdrom_command(& cgc, (void *)(buffer), (int )sizeof(buffer), 2);
    cgc.cmd[0] = (unsigned char)70;
    cgc.cmd[3] = (unsigned char)36;
    cgc.cmd[8] = (unsigned char )sizeof(buffer);
    cgc.quiet = 1;

    ret = get_nondet_int();
    if (ret) {
        return (ret);
    }
    feature_code = (__u16 *)(& buffer[sizeof(struct feature_header )]);
    tmp = __fswab16((*feature_code));
    if ((int )tmp == 36) {
        return (0);
    }
    return (1);
}

static int cdrom_ram_open_write(struct cdrom_device_info *cdi )
{
    struct rwrt_feature_desc rfd ;
    int ret ;
    __u16 tmp ;

    ret = cdrom_has_defect_mgt(cdi);
    if (ret) {
        return (ret);
    }
    ret = cdrom_get_random_writable(cdi, & rfd);
    if (ret) {
        return (ret);
    } else {
        tmp = __fswab16(rfd.feature_code);
        if (32 == (int )tmp) {
            ret = ! rfd.curr;
        }
    }
    if (debug == 1) {
        HsPrintk("<6>cdrom: can open for random write\n");
    }
    return (ret);
}

static int mo_open_write(struct cdrom_device_info *cdi )
{
    struct packet_command cgc ;
    char buffer[255] ;
    int ret ;

    init_cdrom_command(& cgc, (void *)(& buffer), 4, 2);
    cgc.quiet = 1;
    ret = cdrom_mode_sense(cdi, & cgc, 63, 0);
    if (ret) {
        ret = cdrom_mode_sense(cdi, & cgc, 0, 0);
    }
    if (ret) {
        cgc.buflen = 255U;
        ret = cdrom_mode_sense(cdi, & cgc, 63, 0);
    }
    if (ret) {
        return (0);
    }
    return ((int )buffer[3] & 128);
}

static int cdrom_is_dvd_rw(struct cdrom_device_info *cdi )
{
    if ((int )cdi->mmc3_profile == 18 || (int )cdi->mmc3_profile == 26) {
        return (0);
    } else {
        return (1);
    }
}

static int cdrom_open_write(struct cdrom_device_info *cdi )
{
    int mrw ;
    int mrw_write ;
    int ram_write ;
    int ret ;
    int tmp ;
    int tmp___0 ;

    ret = 1;
    mrw = 0;
    tmp = cdrom_is_mrw(cdi, & mrw_write);
    if (! tmp) {
        mrw = 1;
    }
    if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 262144) {
        ram_write = 1;
    } else {
        cdrom_is_random_writable(cdi, & ram_write);
    }
    if (mrw) {
        cdi->mask &= -524289;
    } else {
        cdi->mask |= 524288;
    }
    if (mrw_write) {
        cdi->mask &= -1048577;
    } else {
        cdi->mask |= 1048576;
    }
    if (ram_write) {
        cdi->mask &= -2097153;
    } else {
        cdi->mask |= 2097152;
    }
    if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 1048576) {
        ret = cdrom_mrw_open_write(cdi);
    } else {
        if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 131072) {
            ret = cdrom_dvdram_open_write(cdi);
        } else {
            if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 2097152) {
                if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 909312)) {
                    ret = cdrom_ram_open_write(cdi);
                } else {
                    goto _L;
                }
            } else {
_L:
                if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 262144) {
                    ret = mo_open_write(cdi);
                } else {
                    tmp___0 = cdrom_is_dvd_rw(cdi);
                    if (! tmp___0) {
                        ret = 0;
                    }
                }
            }
        }
    }
    return (ret);
}

int cdrom_open(struct cdrom_device_info *cdi , struct inode *ip , struct file *fp )
{
    int ret ;
    int tmp ;
    if (debug == 1) {
        HsPrintk("<6>cdrom: entering cdrom_open\n");
    }
    cdi->use_count ++;
    if (fp->f_flags & 2048U) {
        if (cdi->options & 4) {

        } else {
            goto _L;
        }
    } else {
_L:
        ret = open_for_data(cdi);
        if (ret) {
            goto err;
        }
        cdrom_mmc3_profile(cdi);
        if ((int )fp->f_mode & 2) {
            ret = -30;
            tmp = cdrom_open_write(cdi);
            if (tmp) {
                goto err;
            }
            if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 2097152)) {
                goto err;
            }
            ret = 0;
            cdi->media_written = (unsigned char)0;
        }
    }
    if (ret) {
        goto err;
    }
    if (debug == 1) {
        HsPrintk("<6>cdrom: Use count for \"/dev/%s\" now %d\n", cdi->name, cdi->use_count);
    }
    check_disk_change(ip->i_bdev);
    return (0);
err:
    cdi->use_count --;
    return (ret);
}

static void cdrom_dvd_rw_close_write(struct cdrom_device_info *cdi )
{
    struct packet_command cgc ;
    if ((int )cdi->mmc3_profile != 26) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: %s: No DVD+RW\n", cdi->name);
        }
        return;
    }
    if (! cdi->media_written) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: %s: DVD+RW media clean\n", cdi->name);
        }
        return;
    }
    HsPrintk("<6>cdrom: %s: dirty DVD+RW media, \"finalizing\"\n", cdi->name);
    init_cdrom_command(& cgc, (void *)0, 0, 3);
    cgc.cmd[0] = (unsigned char)53;
    cgc.timeout = 3000;

    init_cdrom_command(& cgc, (void *)0, 0, 3);
    cgc.cmd[0] = (unsigned char)91;
    cgc.timeout = 300000;
    cgc.quiet = 1;

    init_cdrom_command(& cgc, (void *)0, 0, 3);
    cgc.cmd[0] = (unsigned char)91;
    cgc.cmd[2] = (unsigned char)2;
    cgc.quiet = 1;
    cgc.timeout = 300000;

    cdi->media_written = (unsigned char)0;
    return;
}

static int cdrom_close_write(struct cdrom_device_info *cdi )
{
    return (0);
}

int cdrom_release(struct cdrom_device_info *cdi , struct file *fp )
{
    struct cdrom_device_ops *cdo ;
    int opened_for_data ;
    int tmp ;

    cdo = cdi->ops;
    if (debug == 1) {
        HsPrintk("<6>cdrom: entering cdrom_release\n");
    }
    if (cdi->use_count > 0) {
        cdi->use_count --;
    }
    if (cdi->use_count == 0) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: Use count for \"/dev/%s\" now zero\n", cdi->name);
        }
    }
    if (cdi->use_count == 0) {
        cdrom_dvd_rw_close_write(cdi);
    }
    if (cdi->use_count == 0) {
        if (cdo->capability & 4) {
            if (! keeplocked) {
                if (debug == 1) {
                    HsPrintk("<6>cdrom: Unlocking door!\n");
                }

            }
        }
    }
    if (! (cdi->options & 4)) {
        tmp = 1;
    } else {
        if (fp) {
            if (fp->f_flags & 2048U) {
                tmp = 0;
            } else {
                tmp = 1;
            }
        } else {
            tmp = 1;
        }
    }
    opened_for_data = tmp;
    if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 2097152) {
        if (! cdi->use_count) {
            if (cdi->for_data) {
                cdrom_close_write(cdi);
            }
        }
    }

    if (cdi->use_count == 0) {
        if (opened_for_data) {
            if (cdi->options & 2) {
                if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 2) {

                }
            }
        }
    }
    return (0);
}

static int cdrom_read_mech_status(struct cdrom_device_info *cdi , struct cdrom_changer_info *buf )
{
    struct packet_command cgc ;
    struct cdrom_device_ops *cdo ;
    int length ;
    int tmp ;

    cdo = cdi->ops;
    if (cdi->sanyo_slot) {
        buf->hdr.nslots = (unsigned char)3;
        if ((int )cdi->sanyo_slot == 3) {
            buf->hdr.curslot = (unsigned char)0;
        } else {
            buf->hdr.curslot = cdi->sanyo_slot;
        }
        length = 0;
        while (length < 3) {
            buf->slots[length].disc_present = (unsigned char)1;
            buf->slots[length].change = (unsigned char)0;
            length ++;
        }
        return (0);
    }
    length = (int )(sizeof(struct cdrom_mechstat_header ) + (unsigned int )cdi->capacity * sizeof(struct cdrom_slot ));
    init_cdrom_command(& cgc, (void *)buf, length, 2);
    cgc.cmd[0] = (unsigned char)189;
    cgc.cmd[8] = (unsigned char )((length >> 8) & 255);
    cgc.cmd[9] = (unsigned char )(length & 255);

    tmp = get_nondet_int();
    return (tmp);
}

int cdrom_number_of_slots(struct cdrom_device_info *cdi)
{
    int status ;
    int nslots ;
    struct cdrom_changer_info *info ;

    nslots = 1;
    if (debug == 1) {
        HsPrintk("<6>cdrom: entering cdrom_number_of_slots()\n");
    }
    cdi->capacity = 0;
    info = (struct cdrom_changer_info *)mallocnull(sizeof((*info)));
    if (!info) {
        return (-12);
    }
    status = cdrom_read_mech_status(cdi, info);
    if (status == 0) {
        nslots = (int )info->hdr.nslots;
    }
    free((void *)info);
    return (nslots);
}

static int cdrom_switch_blocksize(struct cdrom_device_info *cdi , int size )
{
    struct cdrom_device_ops *cdo ;
    struct packet_command cgc ;
    struct modesel_head mh ;
    void *__p ;
    size_t __n ;
    void *__p___0 ;
    size_t __n___0 ;
    int tmp ;

    cdo = cdi->ops;
    __p = (void *)(& mh);
    __n = sizeof(mh);
    if (__n != 0U) {
        __memzero(__p, __n);
    }
    mh.block_desc_length = (unsigned char)8;
    mh.block_length_med = (unsigned char )((size >> 8) & 255);
    mh.block_length_lo = (unsigned char )(size & 255);
    __p___0 = (void *)(& cgc);
    __n___0 = sizeof(cgc);
    if (__n___0 != 0U) {
        __memzero(__p___0, __n___0);
    }
    cgc.cmd[0] = (unsigned char)21;
    cgc.cmd[1] = (unsigned char)16;
    cgc.cmd[4] = (unsigned char)12;
    cgc.buflen = sizeof(mh);
    cgc.buffer = (unsigned char *)((char *)(& mh));
    cgc.data_direction = (unsigned char)1;
    mh.block_desc_length = (unsigned char)8;
    mh.block_length_med = (unsigned char )((size >> 8) & 255);
    mh.block_length_lo = (unsigned char )(size & 255);

    tmp = get_nondet_int();
    return (tmp);
}

static int cdrom_get_track_info(struct cdrom_device_info *cdi , __u16 track , __u8 type ,
        track_information *ti )
{
    struct cdrom_device_ops *cdo ;
    struct packet_command cgc ;
    int ret ;
    int buflen ;
    __u16 tmp ;

    cdo = cdi->ops;
    init_cdrom_command(& cgc, (void *)ti, 8, 2);
    cgc.cmd[0] = (unsigned char)82;
    cgc.cmd[1] = (unsigned char )((int )type & 3);
    cgc.cmd[4] = (unsigned char )(((int )track & 65280) >> 8);
    cgc.cmd[5] = (unsigned char )((int )track & 255);
    cgc.cmd[8] = (unsigned char)8;
    cgc.quiet = 1;

    ret = get_nondet_int();
    if (ret) {
        return (ret);
    }
    tmp = __fswab16(ti->track_information_length);
    buflen = (int )((unsigned int )tmp + sizeof(ti->track_information_length));
    if ((unsigned int )buflen > sizeof(track_information )) {
        buflen = (int )sizeof(track_information );
    }
    cgc.buflen = (unsigned int )buflen;
    cgc.cmd[8] = (unsigned char )cgc.buflen;

    ret = get_nondet_int();
    if (ret) {
        return (ret);
    }
    return (buflen);
}

static void sanitize_format(union cdrom_addr *addr , u_char *curr , u_char requested )
{
    int lba ;

    if ((int )(*curr) == (int )requested) {
        return;
    }
    if ((int )requested == 1) {
        addr->lba = (int )addr->msf.frame + 75 * (((int )addr->msf.second - 2) + 60 * (int )addr->msf.minute);
    } else {
        lba = addr->lba;
        addr->msf.frame = (unsigned char )(lba % 75);
        lba /= 75;
        lba += 2;
        addr->msf.second = (unsigned char )(lba % 60);
        addr->msf.minute = (unsigned char )(lba / 60);
    }
    (*curr) = requested;
    return;
}

int cdrom_get_last_written(struct cdrom_device_info *cdi , long *last_written )
{
    struct cdrom_tocentry toc ;
    disc_information di ;
    track_information ti ;
    __u32 last_track ;
    int ret ;
    int ti_size ;
    __u32 tmp ;
    __u32 tmp___0 ;
    __u32 tmp___1 ;

    ret = -1;
    if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 4096)) {
        goto use_toc;
    }
    ret = cdrom_get_disc_info(cdi, & di);

    if (__VERIFIER_nondet_int()) {
        goto use_toc;
    }
    last_track = (unsigned int )(((int )di.last_track_msb << 8) | (int )di.last_track_lsb);
    ti_size = cdrom_get_track_info(cdi, (unsigned short )last_track, (unsigned char)1, & ti);

    if (__VERIFIER_nondet_int()) {
        goto use_toc;
    }
    if (ti.blank) {
        if (last_track == 1U) {
            goto use_toc;
        }
        last_track --;
        ti_size = cdrom_get_track_info(cdi, (unsigned short )last_track, (unsigned char)1, & ti);
    }

    if (__VERIFIER_nondet_int()) {
        goto use_toc;
    }
    if (ti.lra_v) {

        if (__VERIFIER_nondet_int()) {
            (*last_written) = (long )__fswab32(ti.last_rec_address);
        } else {
            goto _L;
        }
    } else {
_L:
        tmp = __fswab32(ti.track_start);
        tmp___0 = __fswab32(ti.track_size);
        (*last_written) = (long )(tmp + tmp___0);
        if (ti.free_blocks) {
            tmp___1 = __fswab32(ti.free_blocks);
            (*last_written) = (long )((unsigned long )(*last_written) - (unsigned long )(tmp___1 + 7U));
        }
    }
    return (0);
use_toc:
    toc.cdte_format = (unsigned char)2;
    toc.cdte_track = (unsigned char)170;

    ret = get_nondet_int();
    if (ret) {
        return (ret);
    }
    sanitize_format(& toc.cdte_addr, & toc.cdte_format, (unsigned char)1);
    (*last_written) = (long )toc.cdte_addr.lba;
    return (0);
}

static int cdrom_get_next_writable(struct cdrom_device_info *cdi , long *next_writable )
{
    disc_information di ;
    track_information ti ;
    __u16 last_track ;
    int ret ;
    int ti_size ;

    if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 4096)) {
        goto use_last_written;
    }
    ret = cdrom_get_disc_info(cdi, & di);
    if (ret < 0) {
        goto use_last_written;
    } else {
        if ((unsigned int )ret < (unsigned int )(& ((disc_information *)0)->last_track_lsb) + sizeof(di.last_track_lsb)) {
            goto use_last_written;
        }
    }
    last_track = (unsigned short )(((int )di.last_track_msb << 8) | (int )di.last_track_lsb);
    ti_size = cdrom_get_track_info(cdi, last_track, (unsigned char)1, & ti);
    if (ti_size < 0) {
        goto use_last_written;
    } else {
        if ((unsigned int )ti_size < (unsigned int )(& ((track_information *)0)->track_start)) {
            goto use_last_written;
        }
    }
    if (ti.blank) {
        if ((int )last_track == 1) {
            goto use_last_written;
        }
        last_track = (__u16 )((int )last_track - 1);
        ti_size = cdrom_get_track_info(cdi, last_track, (unsigned char)1, & ti);
        if (ti_size < 0) {
            goto use_last_written;
        }
    }
    if (ti.nwa_v) {
        if ((unsigned int )ti_size >= (unsigned int )(& ((track_information *)0)->next_writable) + sizeof(ti.next_writable)) {
            (*next_writable) = (long )__fswab32(ti.next_writable);
            return (0);
        }
    }
use_last_written:
    ret = cdrom_get_last_written(cdi, next_writable);
    if (ret) {
        (*next_writable) = 0L;
        return (ret);
    } else {
        (*next_writable) += 7L;
        return (0);
    }
}

static int cdrom_read_cd(struct cdrom_device_info *cdi , struct packet_command *cgc ,
        int lba , int blocksize , int nblocks )
{
    struct cdrom_device_ops *cdo ;
    void *__p ;
    size_t __n ;
    int tmp ;

    cdo = cdi->ops;
    __p = (void *)(& cgc->cmd);
    __n = sizeof(cgc->cmd);
    if (__n != 0U) {
        __memzero(__p, __n);
    }
    cgc->cmd[0] = (unsigned char)40;
    cgc->cmd[2] = (unsigned char )((lba >> 24) & 255);
    cgc->cmd[3] = (unsigned char )((lba >> 16) & 255);
    cgc->cmd[4] = (unsigned char )((lba >> 8) & 255);
    cgc->cmd[5] = (unsigned char )(lba & 255);
    cgc->cmd[6] = (unsigned char )((nblocks >> 16) & 255);
    cgc->cmd[7] = (unsigned char )((nblocks >> 8) & 255);
    cgc->cmd[8] = (unsigned char )(nblocks & 255);
    cgc->buflen = (unsigned int )(blocksize * nblocks);

    tmp = get_nondet_int();
    return (tmp);
}

static int cdrom_read_block(struct cdrom_device_info *cdi , struct packet_command *cgc ,
        int lba , int nblocks , int format , int blksize )
{
    struct cdrom_device_ops *cdo ;
    void *__p ;
    size_t __n ;
    int tmp ;

    cdo = cdi->ops;
    __p = (void *)(& cgc->cmd);
    __n = sizeof(cgc->cmd);
    if (__n != 0U) {
        __memzero(__p, __n);
    }
    cgc->cmd[0] = (unsigned char)190;
    cgc->cmd[1] = (unsigned char )(format << 2);
    cgc->cmd[2] = (unsigned char )((lba >> 24) & 255);
    cgc->cmd[3] = (unsigned char )((lba >> 16) & 255);
    cgc->cmd[4] = (unsigned char )((lba >> 8) & 255);
    cgc->cmd[5] = (unsigned char )(lba & 255);
    cgc->cmd[6] = (unsigned char )((nblocks >> 16) & 255);
    cgc->cmd[7] = (unsigned char )((nblocks >> 8) & 255);
    cgc->cmd[8] = (unsigned char )(nblocks & 255);
    cgc->buflen = (unsigned int )(blksize * nblocks);
    if (blksize == 2336) {
        cgc->cmd[9] = (unsigned char)88;
    }
    else if (blksize == 2340) {
        cgc->cmd[9] = (unsigned char)120;
    }
    else if (blksize == 2352) {
        cgc->cmd[9] = (unsigned char)248;
    }
    else {
        cgc->cmd[9] = (unsigned char)16;
    }

    tmp = get_nondet_int();
    return (tmp);
}

static int cdrom_read_cdda_old(struct cdrom_device_info *cdi , __u8 *ubuf , int lba ,
        int nframes )
{
    struct packet_command cgc ;
    int ret ;
    int nr ;
    void *__p ;
    size_t __n ;
    unsigned long flag ;
    unsigned long sum ;
    struct thread_info *tmp ;
    unsigned long tmp___0 ;

    ret = 0;
    cdi->last_sense = (unsigned char)0;
    __p = (void *)(& cgc);
    __n = sizeof(cgc);
    if (__n != 0U) {
        __memzero(__p, __n);
    }
    nr = nframes;
    while (1) {
        cgc.buffer = (unsigned char *)mallocnull((unsigned int )(2352 * nr));
        if (cgc.buffer) {
            break;
        }
        nr >>= 1;
        if (! nr) {
            break;
        }
    }
    if (! nr) {
        return (-12);
    }
    tmp = current_thread_info();


    if (! (flag == 0UL)) {
        ret = -14;
        goto out;
    }
    cgc.data_direction = (unsigned char)2;
    while (nframes > 0) {
        if (nr > nframes) {
            nr = nframes;
        }
        ret = cdrom_read_block(cdi, & cgc, lba, nr, 1, 2352);
        if (ret) {
            break;
        }
        tmp___0 = __copy_to_user((void *)ubuf, (void *)cgc.buffer, (unsigned long )(2352 * nr));
        if (tmp___0) {
            ret = -14;
            break;
        }
        ubuf += 2352 * nr;
        nframes -= nr;
        lba += nr;
    }
out:
    free((void *)cgc.buffer);
    return (ret);
}

static int cdrom_read_cdda_bpc(struct cdrom_device_info *cdi , __u8 *ubuf , int lba ,
        int nframes )
{
    request_queue_t *q ;
    struct request *rq ;
    struct bio *bio ;
    unsigned int len ;
    int nr ;
    int ret ;
    void *__p ;
    size_t __n ;
    struct request_sense *s ;
    int tmp ;
    int tmp___0 ;

    q = (cdi->disk)->queue;
    ret = 0;
    if (! q) {
        return (-6);
    }


    rq = q->boundary_rq;
    if (! rq) {
        return (-12);
    }
    cdi->last_sense = (unsigned char)0;
    while (nframes) {
        nr = nframes;
        if (cdi->cdda_method == 1) {
            nr = 1;
        }
        if ((unsigned int )(nr * 2352) > q->max_sectors << 9) {
            nr = (int )((q->max_sectors << 9) / 2352U);
        }
        len = (unsigned int )(nr * 2352);
        ret = blk_rq_map_user(q, rq, (void *)ubuf, len);
        if (ret) {
            break;
        }

        __p = get_nondet_ptr();
        __n = sizeof(rq->cmd);
        if (__n != 0U) {
            __memzero(__p, __n);
        }
        rq->cmd[0] = (unsigned char)190;
        rq->cmd[1] = (unsigned char)4;
        rq->cmd[2] = (unsigned char )((lba >> 24) & 255);
        rq->cmd[3] = (unsigned char )((lba >> 16) & 255);
        rq->cmd[4] = (unsigned char )((lba >> 8) & 255);
        rq->cmd[5] = (unsigned char )(lba & 255);
        rq->cmd[6] = (unsigned char )((nr >> 16) & 255);
        rq->cmd[7] = (unsigned char )((nr >> 8) & 255);
        rq->cmd[8] = (unsigned char )(nr & 255);
        rq->cmd[9] = (unsigned char)248;
        rq->cmd_len = 12U;
        rq->flags |= 8192UL;
        rq->timeout = 6000U;
        bio = rq->bio;
        if (rq->bio) {
            blk_queue_bounce(q, & rq->bio);
        }
        tmp = blk_execute_rq(q, cdi->disk, rq, 0);
        if (tmp) {
            s = (struct request_sense *)rq->sense;
            ret = -5;
            cdi->last_sense = s->sense_key;
            cdi->last_sense = get_nondet_int();
        }
        tmp___0 = blk_rq_unmap_user(bio, len);
        if (tmp___0) {
            ret = -14;
        }
        if (ret) {
            break;
        }
        nframes -= nr;
        lba += nr;
        ubuf += len;
    }
    blk_put_request(rq);
    return (ret);
}

static int cdrom_read_cdda(struct cdrom_device_info *cdi , __u8 *ubuf , int lba , int nframes )
{
    int ret ;
    int tmp ;
    int tmp___0 ;

    if (cdi->cdda_method == 0) {
        tmp = cdrom_read_cdda_old(cdi, ubuf, lba, nframes);
        return (tmp);
    }
retry:
    ret = cdrom_read_cdda_bpc(cdi, ubuf, lba, nframes);
    if (! ret) {
        return (ret);
    } else {
        if (ret != -5) {
            return (ret);
        }
    }
    if (cdi->cdda_method == 2) {
        if (nframes > 1) {
            HsPrintk("cdrom: dropping to single frame dma\n");
            cdi->cdda_method = 1;
            goto retry;
        }
    }
    if ((int )cdi->last_sense != 4) {
        if ((int )cdi->last_sense != 11) {
            return (ret);
        }
    }
    HsPrintk("cdrom: dropping to old style cdda (sense=%x)\n", cdi->last_sense);
    cdi->cdda_method = 0;
    tmp___0 = cdrom_read_cdda_old(cdi, ubuf, lba, nframes);
    return (tmp___0);
}

static int cdrom_read_subchannel(struct cdrom_device_info *cdi, struct cdrom_subchnl *subchnl, int mcn )
{
    struct cdrom_device_ops *cdo ;
    struct packet_command cgc ;
    char buffer[32] ;
    int ret ;

    cdo = cdi->ops;
    init_cdrom_command(& cgc, (void *)(buffer), 16, 2);
    cgc.cmd[0] = (unsigned char)66;
    cgc.cmd[1] = (unsigned char)2;
    cgc.cmd[2] = (unsigned char)64;
    if (mcn) {
        cgc.cmd[3] = (unsigned char)2;
    } else {
        cgc.cmd[3] = (unsigned char)1;
    }
    cgc.cmd[8] = (unsigned char)16;

    ret = get_nondet_int();
    if (ret) {
        return (ret);
    }
    subchnl->cdsc_audiostatus = (*(cgc.buffer + 1));
    subchnl->cdsc_format = (unsigned char)2;
    subchnl->cdsc_ctrl = (unsigned char )((int )(*(cgc.buffer + 5)) & 15);
    subchnl->cdsc_trk = (*(cgc.buffer + 6));
    subchnl->cdsc_ind = (*(cgc.buffer + 7));
    subchnl->cdsc_reladdr.msf.minute = (*(cgc.buffer + 13));
    subchnl->cdsc_reladdr.msf.second = (*(cgc.buffer + 14));
    subchnl->cdsc_reladdr.msf.frame = (*(cgc.buffer + 15));
    subchnl->cdsc_absaddr.msf.minute = (*(cgc.buffer + 9));
    subchnl->cdsc_absaddr.msf.second = (*(cgc.buffer + 10));
    subchnl->cdsc_absaddr.msf.frame = (*(cgc.buffer + 11));
    return (0);
}

static int dvd_read_physical(struct cdrom_device_info *cdi , dvd_struct *s )
{
    unsigned char buf[21] ;
    unsigned char *base ;
    struct dvd_layer *layer ;
    struct packet_command cgc ;
    struct cdrom_device_ops *cdo ;
    int ret ;
    int layer_num ;
    void *__p ;
    size_t __n ;

    cdo = cdi->ops;
    layer_num = (int )s->physical.layer_num;
    if (layer_num >= 4) {
        return (-22);
    }
    init_cdrom_command(& cgc, (void *)(buf), (int )sizeof(buf), 2);
    cgc.cmd[0] = (unsigned char)173;
    cgc.cmd[6] = (unsigned char )layer_num;
    cgc.cmd[7] = s->type;
    cgc.cmd[9] = (unsigned char )(cgc.buflen & 255U);
    cgc.quiet = 1;

    ret = get_nondet_int();
    if (ret) {
        return (ret);
    }
    base = & buf[4];
    layer = & s->physical.layer[layer_num];
    __p = (void *)layer;
    __n = sizeof((*layer));
    if (__n != 0U) {
        __memzero(__p, __n);
    }
    layer->book_version = (unsigned char )((int )(*(base + 0)) & 15);
    layer->book_type = (unsigned char )((int )(*(base + 0)) >> 4);
    layer->min_rate = (unsigned char )((int )(*(base + 1)) & 15);
    layer->disc_size = (unsigned char )((int )(*(base + 1)) >> 4);
    layer->layer_type = (unsigned char )((int )(*(base + 2)) & 15);
    layer->track_path = (unsigned char )(((int )(*(base + 2)) >> 4) & 1);
    layer->nlayers = (unsigned char )(((int )(*(base + 2)) >> 5) & 3);
    layer->track_density = (unsigned char )((int )(*(base + 3)) & 15);
    layer->linear_density = (unsigned char )((int )(*(base + 3)) >> 4);
    layer->start_sector = (unsigned int )((((int )(*(base + 5)) << 16) | ((int )(*(base + 6)) << 8)) | (int )(*(base + 7)));
    layer->end_sector = (unsigned int )((((int )(*(base + 9)) << 16) | ((int )(*(base + 10)) << 8)) | (int )(*(base + 11)));
    layer->end_sector_l0 = (unsigned int )((((int )(*(base + 13)) << 16) | ((int )(*(base + 14)) << 8)) | (int )(*(base + 15)));
    layer->bca = (unsigned char )((int )(*(base + 16)) >> 7);
    return (0);
}

static int dvd_read_copyright(struct cdrom_device_info *cdi , dvd_struct *s )
{
    int ret ;
    u_char buf[8] ;
    struct packet_command cgc ;
    struct cdrom_device_ops *cdo ;

    cdo = cdi->ops;
    init_cdrom_command(& cgc, (void *)(buf), (int )sizeof(buf), 2);
    cgc.cmd[0] = (unsigned char)173;
    cgc.cmd[6] = s->copyright.layer_num;
    cgc.cmd[7] = s->type;
    cgc.cmd[8] = (unsigned char )(cgc.buflen >> 8);
    cgc.cmd[9] = (unsigned char )(cgc.buflen & 255U);

    ret = get_nondet_int();
    if (ret) {
        return (ret);
    }
    s->copyright.cpst = buf[4];
    s->copyright.rmi = buf[5];
    return (0);
}

static int dvd_read_disckey(struct cdrom_device_info *cdi , dvd_struct *s )
{
    int ret ;
    int size ;
    u_char *buf ;
    struct packet_command cgc ;
    struct cdrom_device_ops *cdo ;

    cdo = cdi->ops;
    size = (int )(sizeof(s->disckey.value) + 4U);
    buf = (u_char *)mallocnull((unsigned int )size);
    if ((unsigned int )buf == (unsigned int )((void *)0)) {
        return (-12);
    }
    init_cdrom_command(& cgc, (void *)buf, size, 2);
    cgc.cmd[0] = (unsigned char)173;
    cgc.cmd[7] = s->type;
    cgc.cmd[8] = (unsigned char )(size >> 8);
    cgc.cmd[9] = (unsigned char )(size & 255);
    cgc.cmd[10] = (unsigned char )(s->disckey.agid << 6);

    ret = get_nondet_int();
    if (! ret) {
        HsMemcpy((void *)(s->disckey.value), (void *)(buf + 4), sizeof(s->disckey.value));
    }
    free((void *)buf);
    return (ret);
}

static int dvd_read_bca(struct cdrom_device_info *cdi , dvd_struct *s )
{
    int ret ;
    u_char buf[192] ;
    struct packet_command cgc ;
    struct cdrom_device_ops *cdo ;

    cdo = cdi->ops;
    init_cdrom_command(& cgc, (void *)(buf), (int )sizeof(buf), 2);
    cgc.cmd[0] = (unsigned char)173;
    cgc.cmd[7] = s->type;
    cgc.buflen = 255U;
    cgc.cmd[9] = (unsigned char )cgc.buflen;

    ret = get_nondet_int();
    if (ret) {
        return (ret);
    }
    s->bca.len = ((int )buf[0] << 8) | (int )buf[1];
    if (s->bca.len < 12) {
        HsPrintk("<6>cdrom: Received invalid BCA length (%d)\n", s->bca.len);
        return (-5);
    } else {
        if (s->bca.len > 188) {
            HsPrintk("<6>cdrom: Received invalid BCA length (%d)\n", s->bca.len);
            return (-5);
        }
    }
    HsMemcpy((void *)(s->bca.value), (void *)(& buf[4]), (unsigned int )s->bca.len);
    return (0);
}

static int dvd_read_manufact(struct cdrom_device_info *cdi , dvd_struct *s )
{
    int ret ;
    int size ;
    u_char *buf ;
    struct packet_command cgc ;
    struct cdrom_device_ops *cdo ;

    ret = 0;
    cdo = cdi->ops;
    size = (int )(sizeof(s->manufact.value) + 4U);
    buf = (u_char *)mallocnull((unsigned int )size);
    if ((unsigned int )buf == (unsigned int )((void *)0)) {
        return (-12);
    }
    init_cdrom_command(& cgc, (void *)buf, size, 2);
    cgc.cmd[0] = (unsigned char)173;
    cgc.cmd[7] = s->type;
    cgc.cmd[8] = (unsigned char )(size >> 8);
    cgc.cmd[9] = (unsigned char )(size & 255);

    ret = get_nondet_int();
    if (ret) {
        free((void *)buf);
        return (ret);
    }
    s->manufact.len = ((int )(*(buf + 0)) << 8) | (int )(*(buf + 1));
    if (s->manufact.len < 0) {
        HsPrintk("<6>cdrom: Received invalid manufacture info length (%d)\n", s->manufact.len);
        ret = -5;
    } else {
        if (s->manufact.len > 2048) {
            HsPrintk("<6>cdrom: Received invalid manufacture info length (%d)\n", s->manufact.len);
            ret = -5;
        } else {
            HsMemcpy((void *)(s->manufact.value), (void *)(buf + 4), (unsigned int )s->manufact.len);
        }
    }
    free((void *)buf);
    return (ret);
}

static int dvd_read_struct(struct cdrom_device_info *cdi , dvd_struct *s )
{
    int tmp ;
    int tmp___0 ;
    int tmp___1 ;
    int tmp___2 ;
    int tmp___3 ;

    if (s->type == 0) {
        tmp = dvd_read_physical(cdi, s);
        return (tmp);
    }
    else if (s->type == 1) {
        tmp___0 = dvd_read_copyright(cdi, s);
        return (tmp___0);
    }
    else if (s->type == 2) {
        tmp___1 = dvd_read_disckey(cdi, s);
        return (tmp___1);
    }
    else if (s->type == 3) {
        tmp___2 = dvd_read_bca(cdi, s);
        return (tmp___2);
    }
    else if (s->type == 4) {
        tmp___3 = dvd_read_manufact(cdi, s);
        return (tmp___3);
    }
    else {
        HsPrintk("<6>cdrom: : Invalid DVD structure read requested (%d)\n", s->type);
        return (-22);
    }
}

static void setup_report_key(struct packet_command *cgc , unsigned int agid , unsigned int type )
{
    cgc->cmd[0] = (unsigned char)164;
    cgc->cmd[10] = (unsigned char )(type | (agid << 6));
    if (type == 0 || type == 8 || type == 5) {
        cgc->buflen = 8U;
    }
    else if (type == 1) {
        cgc->buflen = 16U;
    }
    else if (type == 2 || type == 4) {
        cgc->buflen = 12U;
    }
    cgc->cmd[9] = (unsigned char )cgc->buflen;
    cgc->data_direction = (unsigned char)2;
    return;
}

static void setup_send_key(struct packet_command *cgc , unsigned int agid , unsigned int type )
{
    cgc->cmd[0] = (unsigned char)163;
    cgc->cmd[10] = (unsigned char )(type | (agid << 6));
    if (type == 1) {
        cgc->buflen = 16U;
    }
    else if (type == 3) {
        cgc->buflen = 12U;
    }
    else if (type == 6) {
        cgc->buflen = 8U;
    }
    cgc->cmd[9] = (unsigned char )cgc->buflen;
    cgc->data_direction = (unsigned char)1;
    return;
}

static int dvd_do_auth(struct cdrom_device_info *cdi , dvd_authinfo *ai )
{
    int ret ;
    u_char buf[20] ;
    struct packet_command cgc ;
    struct cdrom_device_ops *cdo ;
    rpc_state_t rpc_state ;
    void *__p ;
    size_t __n ;
    void *__p___0 ;
    size_t __n___0 ;

    cdo = cdi->ops;
    __p = (void *)(buf);
    __n = sizeof(buf);
    if (__n != 0U) {
        __memzero(__p, __n);
    }
    init_cdrom_command(& cgc, (void *)(buf), 0, 2);
    if ((int )ai->type == 0) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering DVD_LU_SEND_AGID\n");
        }
        cgc.quiet = 1;
        setup_report_key(& cgc, ai->lsa.agid, 0U);

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
        ai->lsa.agid = (unsigned int )((int )buf[7] >> 6);
    }
    else if ((int )ai->type == 2) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering DVD_LU_SEND_KEY1\n");
        }
        setup_report_key(& cgc, ai->lsk.agid, 2U);

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
        HsMemcpy((void *)(ai->lsk.key), (void *)(& buf[4]), sizeof(dvd_key ));
    }
    else if ((int )ai->type == 3) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering DVD_LU_SEND_CHALLENGE\n");
        }
        setup_report_key(& cgc, ai->lsc.agid, 1U);

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
        HsMemcpy((void *)(ai->lsc.chal), (void *)(& buf[4]), sizeof(dvd_challenge ));
    }
    else if ((int )ai->type == 7) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering DVD_LU_SEND_TITLE_KEY\n");
        }
        cgc.quiet = 1;
        setup_report_key(& cgc, ai->lstk.agid, 4U);
        cgc.cmd[5] = (unsigned char )ai->lstk.lba;
        cgc.cmd[4] = (unsigned char )(ai->lstk.lba >> 8);
        cgc.cmd[3] = (unsigned char )(ai->lstk.lba >> 16);
        cgc.cmd[2] = (unsigned char )(ai->lstk.lba >> 24);

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
        ai->lstk.cpm = (unsigned int )(((int )buf[4] >> 7) & 1);
        ai->lstk.cp_sec = (unsigned int )(((int )buf[4] >> 6) & 1);
        ai->lstk.cgms = (unsigned int )(((int )buf[4] >> 4) & 3);
        HsMemcpy((void *)(ai->lstk.title_key), (void *)(& buf[5]), sizeof(dvd_key ));
    }
    else if ((int )ai->type == 8) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering DVD_LU_SEND_ASF\n");
        }
        setup_report_key(& cgc, ai->lsasf.agid, 5U);

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
        ai->lsasf.asf = (unsigned int )((int )buf[7] & 1);
    }
    else if ((int )ai->type == 1) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering DVD_HOST_SEND_CHALLENGE\n");
        }
        setup_send_key(& cgc, ai->hsc.agid, 1U);
        buf[1] = (unsigned char)14;
        HsMemcpy((void *)(& buf[4]), (void *)(ai->hsc.chal), sizeof(dvd_challenge ));

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
        ai->type = (unsigned char)2;
    }
    else if ((int )ai->type == 4) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering DVD_HOST_SEND_KEY2\n");
        }
        setup_send_key(& cgc, ai->hsk.agid, 3U);
        buf[1] = (unsigned char)10;
        HsMemcpy((void *)(& buf[4]), (void *)(ai->hsk.key), sizeof(dvd_key ));

        ret = get_nondet_int();
        if (ret) {
            ai->type = (unsigned char)6;
            return (ret);
        }
        ai->type = (unsigned char)5;
    }
    else if ((int )ai->type == 9) {
        cgc.quiet = 1;
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering DVD_INVALIDATE_AGID\n");
        }
        setup_report_key(& cgc, ai->lsa.agid, 63U);

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
    }
    else if ((int )ai->type == 10) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering DVD_LU_SEND_RPC_STATE\n");
        }
        setup_report_key(& cgc, 0U, 8U);
        __p___0 = (void *)(& rpc_state);
        __n___0 = sizeof(rpc_state_t );
        if (__n___0 != 0U) {
            __memzero(__p___0, __n___0);
        }
        cgc.buffer = (unsigned char *)((char *)(& rpc_state));

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
        ai->lrpcs.type = rpc_state.type_code;
        ai->lrpcs.vra = rpc_state.vra;
        ai->lrpcs.ucca = rpc_state.ucca;
        ai->lrpcs.region_mask = rpc_state.region_mask;
        ai->lrpcs.rpc_scheme = rpc_state.rpc_scheme;
    }
    else if ((int )ai->type == 11) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering DVD_HOST_SEND_RPC_STATE\n");
        }
        setup_send_key(& cgc, 0U, 6U);
        buf[1] = (unsigned char)6;
        buf[4] = ai->hrpcs.pdrc;

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
    }
    else {
        HsPrintk("<6>cdrom: Invalid DVD key ioctl (%d)\n", ai->type);
        return (-25);
    }
    return (0);
}

static int mmc_ioctl(struct cdrom_device_info *cdi , unsigned int cmd , unsigned long arg )
{
    struct cdrom_device_ops *cdo ;
    struct packet_command cgc ;
    struct request_sense sense ;
    unsigned char buffer[32] ;
    int ret ;
    void *__p ;
    size_t __n ;
    struct cdrom_msf msf ;
    int blocksize ;
    int format ;
    int lba ;
    unsigned long tmp ;
    void *__p___0 ;
    size_t __n___0 ;
    int tmp___1 ;
    unsigned long tmp___2 ;
    struct cdrom_read_audio ra ;
    int lba___0 ;
    unsigned long tmp___3 ;
    int tmp___4 ;
    struct cdrom_subchnl q ;
    u_char requested ;
    u_char back ;
    unsigned long tmp___5 ;
    unsigned long tmp___6 ;
    struct cdrom_msf msf___0 ;
    unsigned long tmp___7 ;
    int tmp___8 ;
    struct cdrom_blk blk ;
    unsigned long tmp___9 ;
    int tmp___10 ;
    struct cdrom_volctrl volctrl ;
    char mask[(int )sizeof(buffer)] ;
    unsigned short offset ;
    unsigned long tmp___11 ;
    __u16 tmp___12 ;
    unsigned long tmp___13 ;
    void *__p___1 ;
    size_t __n___1 ;
    int tmp___14 ;
    int tmp___15 ;
    int tmp___16 ;
    dvd_struct *s ;
    int size ;
    unsigned long tmp___18 ;
    unsigned long tmp___19 ;
    dvd_authinfo ai ;
    unsigned long tmp___20 ;
    unsigned long tmp___21 ;
    long next ;
    unsigned long tmp___22 ;
    long last ;
    unsigned long tmp___23 ;

    cdo = cdi->ops;
    ret = 0;
    __p = (void *)(& cgc);
    __n = sizeof(cgc);
    if (__n != 0U) {
        __memzero(__p, __n);
    }
    if (cmd == 21268 || cmd == 21261 || cmd == 21260) {
        blocksize = 0;
        format = 0;
        if (cmd == 21268) {
            blocksize = 2352;
        }
        else if (cmd == 21261) {
            blocksize = 2048;
            format = 2;
        }
        else if (cmd == 21260) {
            blocksize = 2336;
        }
        tmp = copy_from_user((void *)(& msf), (void *)((struct cdrom_msf *)arg),
                (unsigned long )sizeof(msf));
        if (tmp) {
            return (-14);
        }
        lba = msf_to_lba((char )msf.cdmsf_min0, (char )msf.cdmsf_sec0, (char )msf.cdmsf_frame0);
        if (lba < 0) {
            return (-22);
        }
        cgc.buffer = (unsigned char *)mallocnull((unsigned int )blocksize);
        if ((unsigned int )cgc.buffer == (unsigned int )((void *)0)) {
            return (-12);
        }
        __p___0 = (void *)(& sense);
        __n___0 = sizeof(sense);
        if (__n___0 != 0U) {
            __memzero(__p___0, __n___0);
        }
        cgc.sense = & sense;
        cgc.data_direction = (unsigned char)2;
        ret = cdrom_read_block(cdi, & cgc, lba, 1, format, blocksize);
        if (ret) {
            if ((int )sense.sense_key == 5) {
                if ((int )sense.asc == 32) {
                    if ((int )sense.ascq == 0) {
                        ret = cdrom_switch_blocksize(cdi, blocksize);
                        if (ret) {
                            free((void *)cgc.buffer);
                            return (ret);
                        }
                        cgc.sense = (struct request_sense *)((void *)0);
                        ret = cdrom_read_cd(cdi, & cgc, lba, blocksize, 1);
                        tmp___1 = cdrom_switch_blocksize(cdi, blocksize);
                        ret |= tmp___1;
                    }
                }
            }
        }
        if (! ret) {
            tmp___2 = copy_to_user((void *)((char *)arg), (void *)cgc.buffer, (unsigned long )blocksize);
            if (tmp___2) {
                ret = -14;
            }
        }
        free((void *)cgc.buffer);
        return (ret);
    }
    else if (cmd == 21262) {
        tmp___3 = copy_from_user((void *)(& ra), (void *)((struct cdrom_read_audio *)arg),
                (unsigned long )sizeof(ra));
        if (tmp___3) {
            return (-14);
        }
        if ((int )ra.addr_format == 2) {
            lba___0 = msf_to_lba((char )ra.addr.msf.minute, (char )ra.addr.msf.second, (char )ra.addr.msf.frame);
        } else {
            if ((int )ra.addr_format == 1) {
                lba___0 = ra.addr.lba;
            } else {
                return (-22);
            }
        }
        if (lba___0 < 0) {
            return (-22);
        } else {
            if (ra.nframes <= 0) {
                return (-22);
            } else {
                if (ra.nframes > 75) {
                    return (-22);
                }
            }
        }
        tmp___4 = cdrom_read_cdda(cdi, ra.buf, lba___0, ra.nframes);
        return (tmp___4);
    }
    else if (cmd == 21259) {
        tmp___5 = copy_from_user((void *)(& q), (void *)((struct cdrom_subchnl *)arg),
                (unsigned long )sizeof(q));
        if (tmp___5) {
            return (-14);
        }
        requested = q.cdsc_format;
        if (! ((int )requested == 2)) {
            if (! ((int )requested == 1)) {
                return (-22);
            }
        }
        q.cdsc_format = (unsigned char)2;
        ret = cdrom_read_subchannel(cdi, & q, 0);
        if (ret) {
            return (ret);
        }
        back = q.cdsc_format;
        sanitize_format(& q.cdsc_absaddr, & back, requested);
        sanitize_format(& q.cdsc_reladdr, & q.cdsc_format, requested);
        tmp___6 = copy_to_user((void *)((struct cdrom_subchnl *)arg), (void *)(& q),
                (unsigned long )sizeof(q));
        if (tmp___6) {
            return (-14);
        }
        return (0);
    }
    else if (cmd == 21251) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROMPLAYMSF\n");
        }
        tmp___7 = copy_from_user((void *)(& msf___0), (void *)((struct cdrom_msf *)arg),
                (unsigned long )sizeof(msf___0));
        if (tmp___7) {
            return (-14);
        }
        cgc.cmd[0] = (unsigned char)71;
        cgc.cmd[3] = msf___0.cdmsf_min0;
        cgc.cmd[4] = msf___0.cdmsf_sec0;
        cgc.cmd[5] = msf___0.cdmsf_frame0;
        cgc.cmd[6] = msf___0.cdmsf_min1;
        cgc.cmd[7] = msf___0.cdmsf_sec1;
        cgc.cmd[8] = msf___0.cdmsf_frame1;
        cgc.data_direction = (unsigned char)3;

        tmp___8 = get_nondet_int();
        return (tmp___8);
    }
    else if (cmd == 21271) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROMPLAYBLK\n");
        }
        tmp___9 = copy_from_user((void *)(& blk), (void *)((struct cdrom_blk *)arg),
                (unsigned long )sizeof(blk));
        if (tmp___9) {
            return (-14);
        }
        cgc.cmd[0] = (unsigned char)69;
        cgc.cmd[2] = (unsigned char )((blk.from >> 24) & 255U);
        cgc.cmd[3] = (unsigned char )((blk.from >> 16) & 255U);
        cgc.cmd[4] = (unsigned char )((blk.from >> 8) & 255U);
        cgc.cmd[5] = (unsigned char )(blk.from & 255U);
        cgc.cmd[7] = (unsigned char )(((int )blk.len >> 8) & 255);
        cgc.cmd[8] = (unsigned char )((int )blk.len & 255);
        cgc.data_direction = (unsigned char)3;

        tmp___10 = get_nondet_int();
        return (tmp___10);
    }
    else if (cmd == 21258 || cmd == 21267) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROMVOLUME\n");
        }
        tmp___11 = copy_from_user((void *)(& volctrl), (void *)((struct cdrom_volctrl *)arg),
                (unsigned long )sizeof(volctrl));
        if (tmp___11) {
            return (-14);
        }
        cgc.buffer = buffer;
        cgc.buflen = 24U;
        ret = cdrom_mode_sense(cdi, & cgc, 14, 0);
        if (ret) {
            return (ret);
        }
        tmp___12 = __fswab16((*((unsigned short *)(buffer + 6))));
        offset = (unsigned short )(8 + (int )tmp___12);
        if ((unsigned int )((int )offset + 16) > sizeof(buffer)) {
            return (-7);
        }
        if ((unsigned int )((int )offset + 16) > cgc.buflen) {
            cgc.buflen = (unsigned int )((int )offset + 16);
            ret = cdrom_mode_sense(cdi, & cgc, 14, 0);
            if (ret) {
                return (ret);
            }
        }
        if (((int )buffer[offset] & 63) != 14) {
            return (-22);
        } else {
            if ((int )buffer[(int )offset + 1] < 14) {
                return (-22);
            }
        }
        if (cmd == 21267U) {
            volctrl.channel0 = buffer[(int )offset + 9];
            volctrl.channel1 = buffer[(int )offset + 11];
            volctrl.channel2 = buffer[(int )offset + 13];
            volctrl.channel3 = buffer[(int )offset + 15];
            tmp___13 = copy_to_user((void *)((struct cdrom_volctrl *)arg), (void *)(& volctrl),
                    (unsigned long )sizeof(volctrl));
            if (tmp___13) {
                return (-14);
            }
            return (0);
        }
        cgc.buffer = (unsigned char *)(mask);
        ret = cdrom_mode_sense(cdi, & cgc, 14, 1);
        if (ret) {
            return (ret);
        }
        buffer[(int )offset + 9] = (unsigned char )((int )volctrl.channel0 & (int )mask[(int )offset + 9]);
        buffer[(int )offset + 11] = (unsigned char )((int )volctrl.channel1 & (int )mask[(int )offset + 11]);
        buffer[(int )offset + 13] = (unsigned char )((int )volctrl.channel2 & (int )mask[(int )offset + 13]);
        buffer[(int )offset + 15] = (unsigned char )((int )volctrl.channel3 & (int )mask[(int )offset + 15]);
        cgc.buffer = (buffer + (int )offset) - 8;
        __p___1 = (void *)cgc.buffer;
        __n___1 = (size_t )8;
        if (__n___1 != 0U) {
            __memzero(__p___1, __n___1);
        }
        tmp___14 = cdrom_mode_select(cdi, & cgc);
        return (tmp___14);
    }
    else if (cmd == 21256 || cmd == 21255) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROMSTART/CDROMSTOP\n");
        }
        cgc.cmd[0] = (unsigned char)27;
        cgc.cmd[1] = (unsigned char)1;
        if (cmd == 21256U) {
            cgc.cmd[4] = (unsigned char)1;
        } else {
            cgc.cmd[4] = (unsigned char)0;
        }
        cgc.data_direction = (unsigned char)3;

        tmp___15 = get_nondet_int();
        return (tmp___15);
    }
    else if (cmd == 21249 || cmd == 21250) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROMPAUSE/CDROMRESUME\n");
        }
        cgc.cmd[0] = (unsigned char)75;
        if (cmd == 21250U) {
            cgc.cmd[8] = (unsigned char)1;
        } else {
            cgc.cmd[8] = (unsigned char)0;
        }
        cgc.data_direction = (unsigned char)3;

        tmp___16 = get_nondet_int();
        return (tmp___16);
    }
    else if (cmd == 21392) {
        size = (int )sizeof(dvd_struct );
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 32768)) {
            return (-38);
        }
        s = (dvd_struct *)mallocnull((unsigned int )size);
        if ((unsigned int )s == (unsigned int )((void *)0)) {
            return (-12);
        }
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering DVD_READ_STRUCT\n");
        }
        tmp___18 = copy_from_user((void *)s, (void *)((dvd_struct *)arg), (unsigned long )size);
        if (tmp___18) {
            free((void *)s);
            return (-14);
        }
        ret = dvd_read_struct(cdi, s);
        if (ret) {
            free((void *)s);
            return (ret);
        }
        tmp___19 = copy_to_user((void *)((dvd_struct *)arg), (void *)s, (unsigned long )size);
        if (tmp___19) {
            ret = -14;
        }
        free((void *)s);
        return (ret);
    }
    else if (cmd == 21394) {
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 32768)) {
            return (-38);
        }
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering DVD_AUTH\n");
        }
        tmp___20 = copy_from_user((void *)(& ai), (void *)((dvd_authinfo *)arg),
                (unsigned long )sizeof(ai));
        if (tmp___20) {
            return (-14);
        }
        ret = dvd_do_auth(cdi, & ai);
        if (ret) {
            return (ret);
        }
        tmp___21 = copy_to_user((void *)((dvd_authinfo *)arg), (void *)(& ai), (unsigned long )sizeof(ai));
        if (tmp___21) {
            return (-14);
        }
        return (0);
    }
    else if (cmd == 21396) {
        next = 0L;
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROM_NEXT_WRITABLE\n");
        }
        ret = cdrom_get_next_writable(cdi, & next);
        if (ret) {
            return (ret);
        }
        tmp___22 = copy_to_user((void *)((long *)arg), (void *)(& next), (unsigned long )sizeof(next));
        if (tmp___22) {
            return (-14);
        }
        return (0);
    }
    else if (cmd == 21397) {
        last = 0L;
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROM_LAST_WRITTEN\n");
        }
        ret = cdrom_get_last_written(cdi, & last);
        if (ret) {
            return (ret);
        }
        tmp___23 = copy_to_user((void *)((long *)arg), (void *)(& last), (unsigned long )sizeof(last));
        if (tmp___23) {
            return (-14);
        }
        return (0);
    }
    else {
        return (-25);
    }
}

static int media_changed(struct cdrom_device_info *cdi , int queue )
{
    unsigned int mask ;
    int ret ;
    int tmp ;

    mask = (unsigned int )(1 << (queue & 1));
    ret = ! (! (cdi->mc_flags & mask));
    if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 128)) {
        return (ret);
    }

    tmp = get_nondet_int();
    if (tmp) {
        cdi->mc_flags = 3U;
        ret |= 1;
        cdi->media_written = (unsigned char)0;
    }
    cdi->mc_flags &= ~ mask;
    return (ret);
}

static int cdrom_load_unload(struct cdrom_device_info *cdi , int slot )
{
    struct packet_command cgc ;
    int tmp ;

    if (debug == 1) {
        HsPrintk("<6>cdrom: entering cdrom_load_unload()\n");
    }
    if (cdi->sanyo_slot) {
        if (slot < 0) {
            return (0);
        }
    }
    init_cdrom_command(& cgc, (void *)0, 0, 3);
    cgc.cmd[0] = (unsigned char)166;
    cgc.cmd[4] = (unsigned char )(2 + (slot >= 0));
    cgc.cmd[8] = (unsigned char )slot;
    cgc.timeout = 6000;
    if (cdi->sanyo_slot) {
        if (-1 < slot) {
            cgc.cmd[0] = (unsigned char)0;
            cgc.cmd[7] = (unsigned char )slot;
            cgc.cmd[8] = (unsigned char)0;
            cgc.cmd[4] = cgc.cmd[8];
            if (slot) {
                cdi->sanyo_slot = (unsigned char )slot;
            } else {
                cdi->sanyo_slot = (unsigned char)3;
            }
        }
    }

    tmp = get_nondet_int();
    return (tmp);
}

static int cdrom_select_disc(struct cdrom_device_info *cdi , int slot )
{
    struct cdrom_changer_info *info ;
    int curslot ;
    int ret ;
    int tmp ;

    if (debug == 1) {
        HsPrintk("<6>cdrom: entering cdrom_select_disc()\n");
    }
    if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 16)) {
        return (-95);
    }

    if (slot == 2147483646) {
        cdi->mc_flags = 3U;
        tmp = cdrom_load_unload(cdi, -1);
        return (tmp);
    }
    info = (struct cdrom_changer_info *)mallocnull(sizeof((*info)));
    if (! info) {
        return (-12);
    }
    ret = cdrom_read_mech_status(cdi, info);
    if (ret) {
        free((void *)info);
        return (ret);
    }
    curslot = (int )info->hdr.curslot;
    free((void *)info);
    if (cdi->use_count > 1) {
        goto _L;
    } else {
        if (keeplocked) {
_L:
            if (slot == 2147483647) {
                return (curslot);
            } else {
                return (-16);
            }
        }
    }
    if (slot == 2147483647) {
        slot = curslot;
    }
    cdi->mc_flags = 3U;
    ret = cdrom_load_unload(cdi, slot);
    if (ret) {
        return (ret);
    }
    return (slot);
}

static int cdrom_slot_status(struct cdrom_device_info *cdi , int slot )
{
    struct cdrom_changer_info *info ;
    int ret ;
    if (debug == 1) {
        HsPrintk("<6>cdrom: entering cdrom_slot_status()\n");
    }
    if (cdi->sanyo_slot) {
        return (0);
    }
    info = (struct cdrom_changer_info *)mallocnull(sizeof((*info)));
    if (! info) {
        return (-12);
    }
    ret = cdrom_read_mech_status(cdi, info);
    if (ret) {
        goto out_free;
    }
    if (info->slots[slot].disc_present) {
        ret = 4;
    } else {
        ret = 1;
    }
out_free:
    free((void *)info);
    return (ret);
}

static int check_for_audio_disc(struct cdrom_device_info *cdi , struct cdrom_device_ops *cdo )
{
    int ret ;
    tracktype tracks ;

    if (debug == 1) {
        HsPrintk("<6>cdrom: entering check_for_audio_disc\n");
    }
    if (! (cdi->options & 16)) {
        return (0);
    }
    if ((unsigned int )cdo->drive_status != (unsigned int )((void *)0)) {

        ret = get_nondet_int();
        if (debug == 1) {
            HsPrintk("<6>cdrom: drive_status=%d\n", ret);
        }
        if (ret == 2) {
            if (debug == 1) {
                HsPrintk("<6>cdrom: the tray is open...\n");
            }
            if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 1) {
                if (cdi->options & 1) {
                    if (debug == 1) {
                        HsPrintk("<6>cdrom: trying to close the tray.\n");
                    }

                    ret = get_nondet_int();
                    if (ret) {
                        if (debug == 1) {
                            HsPrintk("<6>cdrom: bummer. tried to close tray but failed.\n");
                        }
                        return (-123);
                    }
                } else {
                    goto _L;
                }
            } else {
_L:
                if (debug == 1) {
                    HsPrintk("<6>cdrom: bummer. this driver can\'t close the tray.\n");
                }
                return (-123);
            }

            ret = get_nondet_int();
            if (ret == 1) {
                goto _L___0;
            } else {
                if (ret == 2) {
_L___0:
                    if (debug == 1) {
                        HsPrintk("<6>cdrom: bummer. the tray is still not closed.\n");
                    }
                    return (-123);
                }
            }
            if (ret != 4) {
                if (debug == 1) {
                    HsPrintk("<6>cdrom: bummer. disc isn\'t ready.\n");
                }
                return (-5);
            }
            if (debug == 1) {
                HsPrintk("<6>cdrom: the tray is now closed.\n");
            }
        }
    }
    cdrom_count_tracks(cdi, & tracks);
    if (tracks.error) {
        return ((int )tracks.error);
    }
    if (tracks.audio == 0) {
        return (-124);
    }
    return (0);
}

int cdrom_ioctl(struct file *file , struct cdrom_device_info *cdi , struct inode *ip ,
        unsigned int cmd , unsigned long arg )
{
    struct cdrom_device_ops *cdo ;
    int ret ;
    struct cdrom_multisession ms_info ;
    u_char requested_format ;
    unsigned long tmp ;
    unsigned long tmp___0 ;
    int tmp___1 ;
    int tmp___2 ;
    struct cdrom_changer_info *info ;
    int changed ;
    int tmp___3 ;
    int tmp___4 ;
    int tmp___5 ;
    int tmp___6 ;
    int tmp___7 ;
    int tmp___8 ;
    char *tmp___9 ;
    int tmp___10 ;
    int tmp___11 ;
    int tmp___12 ;
    char *tmp___13 ;
    struct cdrom_mcn mcn ;
    unsigned long tmp___14 ;
    int tmp___15 ;
    int tmp___16 ;
    int tmp___17 ;
    tracktype tracks ;
    struct cdrom_subchnl q ;
    u_char requested ;
    u_char back ;
    unsigned long tmp___18 ;
    unsigned long tmp___19 ;
    struct cdrom_tochdr header ;
    unsigned long tmp___20 ;
    unsigned long tmp___21 ;
    struct cdrom_tocentry entry ;
    u_char requested_format___0 ;
    unsigned long tmp___22 ;
    unsigned long tmp___23 ;
    struct cdrom_msf msf ;
    unsigned long tmp___24 ;
    int tmp___25 ;
    struct cdrom_ti ti ;
    unsigned long tmp___26 ;
    int tmp___27 ;
    struct cdrom_volctrl volume ;
    unsigned long tmp___28 ;
    int tmp___29 ;
    struct cdrom_volctrl volume___0 ;
    unsigned long tmp___30 ;
    int tmp___31 ;
    int tmp___32 ;

    cdo = cdi->ops;
    ret = scsi_cmd_ioctl(file, (ip->i_bdev)->bd_disk, cmd, (void *)arg);
    if (ret != -25) {
        return (ret);
    }
    if (cmd == 21264) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROMMULTISESSION\n");
        }
        if (! (cdo->capability & 32)) {
            return (-38);
        }
        tmp = copy_from_user((void *)(& ms_info), (void *)((struct cdrom_multisession *)arg),
                (unsigned long )sizeof(ms_info));
        if (tmp) {
            return (-14);
        }
        requested_format = ms_info.addr_format;
        if (! ((int )requested_format == 2)) {
            if (! ((int )requested_format == 1)) {
                return (-22);
            }
        }
        ms_info.addr_format = (unsigned char)1;

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
        sanitize_format(& ms_info.addr, & ms_info.addr_format, requested_format);
        tmp___0 = copy_to_user((void *)((struct cdrom_multisession *)arg), (void *)(& ms_info),
                (unsigned long )sizeof(ms_info));
        if (tmp___0) {
            return (-14);
        }
        if (debug == 1) {
            HsPrintk("<6>cdrom: CDROMMULTISESSION successful\n");
        }
        return (0);
    }
    else if (cmd == 21257) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROMEJECT\n");
        }
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 2)) {
            return (-38);
        }
        if (cdi->use_count != 1) {
            return (-16);
        } else {
            if (keeplocked) {
                return (-16);
            }
        }
        if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 4) {

            ret = get_nondet_int();
            if (ret) {
                return (ret);
            }
        }

        tmp___1 = get_nondet_int();
        return (tmp___1);
    }
    else if (cmd == 21273) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROMCLOSETRAY\n");
        }
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 1)) {
            return (-38);
        }

        tmp___2 = get_nondet_int();
        return (tmp___2);
    }
    else if (cmd == 21263) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROMEJECT_SW\n");
        }
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 2)) {
            return (-38);
        }
        if (keeplocked) {
            return (-16);
        }
        cdi->options &= -4;
        if (arg) {
            cdi->options |= 3;
        }
        return (0);
    }
    else if (cmd == 21285) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROM_MEDIA_CHANGED\n");
        }
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 128)) {
            return (-38);
        }
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 16)) {
            tmp___3 = media_changed(cdi, 1);
            return (tmp___3);
        } else {
            if (arg == 2147483647UL) {
                tmp___3 = media_changed(cdi, 1);
                return (tmp___3);
            }
        }
        if ((unsigned int )arg >= (unsigned int )cdi->capacity) {
            return (-22);
        }
        info = (struct cdrom_changer_info *)mallocnull(sizeof((*info)));
        if (! info) {
            return (-12);
        }
        ret = cdrom_read_mech_status(cdi, info);
        if (ret) {
            free((void *)info);
            return (ret);
        }
        changed = (int )info->slots[arg].change;
        free((void *)info);
        return (changed);
    }
    else if (cmd == 21280) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROM_SET_OPTIONS\n");
        }
        if (arg == 4 || arg == 16) {
        }
        else if (arg == 8) {
            if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 4)) {
                return (-38);
            }
        }
        else if (arg == 0) {
            return (cdi->options);
        }
        else {
            if (! ((unsigned long )((cdi->ops)->capability & (int )(~ cdi->mask)) & arg)) {
                return (-38);
            }
        }
        cdi->options |= (int )arg;
        return (cdi->options);
    }
    else if (cmd == 21281) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROM_CLEAR_OPTIONS\n");
        }
        cdi->options &= ~ ((int )arg);
        return (cdi->options);
    }
    else if (cmd == 21282) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROM_SELECT_SPEED\n");
        }
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 8)) {
            return (-38);
        }

        tmp___4 = get_nondet_int();
        return (tmp___4);
    }
    else if (cmd == 21283) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROM_SELECT_DISC\n");
        }
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 16)) {
            return (-38);
        }
        if (arg != 2147483647UL) {
            if (arg != 2147483646UL) {
                if ((int )arg >= cdi->capacity) {
                    return (-22);
                }
            }
        }
        if ((unsigned int )cdo->select_disc != (unsigned int )((void *)0)) {

            tmp___5 = get_nondet_int();
            return (tmp___5);
        }
        if (debug == 1) {
            HsPrintk("<6>cdrom: Using generic cdrom_select_disc()\n");
        }
        tmp___6 = cdrom_select_disc(cdi, (int )arg);
        return (tmp___6);
    }
    else if (cmd == 21266) {
        tmp___7 = capable(21);
        if (! tmp___7) {
            return (-13);
        }
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROM_RESET\n");
        }
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 512)) {
            return (-38);
        }
        invalidate_bdev(ip->i_bdev, 0);

        tmp___8 = get_nondet_int();
        return (tmp___8);
    }
    else if (cmd == 21289) {
        if (debug == 1) {
            if (arg) {
                tmp___9 = "L";
            } else {
                tmp___9 = "Unl";
            }
            HsPrintk("<6>cdrom: %socking door.\n", tmp___9);
        }
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 4)) {
            return (-95);
        }
        if (arg) {
            keeplocked = 1;
        } else {
            keeplocked = 0;
        }
        if (cdi->use_count != 1) {
            if (! arg) {
                tmp___10 = capable(21);
                if (! tmp___10) {
                    return (-16);
                }
            }
        }

        tmp___11 = get_nondet_int();
        return (tmp___11);
    }
    else if (cmd == 21296) {
        tmp___12 = capable(21);
        if (! tmp___12) {
            return (-13);
        }
        if (debug == 1) {
            if (arg) {
                tmp___13 = "En";
            } else {
                tmp___13 = "Dis";
            }
            HsPrintk("<6>cdrom: %sabling debug.\n", tmp___13);
        }
        if (arg) {
            debug = 1;
        } else {
            debug = 0;
        }
        return (debug);
    }
    else if (cmd == 21297) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROM_GET_CAPABILITY\n");
        }
        return ((int )(cdo->capability & (int )(~ cdi->mask)));
    }
    else if (cmd == 21265) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROM_GET_MCN\n");
        }
        if (! (cdo->capability & 64)) {
            return (-38);
        }

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
        tmp___14 = copy_to_user((void *)((struct cdrom_mcn *)arg), (void *)(& mcn),
                (unsigned long )sizeof(mcn));
        if (tmp___14) {
            return (-14);
        }
        if (debug == 1) {
            HsPrintk("<6>cdrom: CDROM_GET_MCN successful\n");
        }
        return (0);
    }
    else if (cmd == 21286) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROM_DRIVE_STATUS\n");
        }
        if (! (cdo->capability & 2048)) {
            return (-38);
        }
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 16)) {

            tmp___15 = get_nondet_int();
            return (tmp___15);
        }
        if (arg == 2147483647UL) {

            tmp___16 = get_nondet_int();
            return (tmp___16);
        } else {
            if (arg == 2147483646UL) {

                tmp___16 = get_nondet_int();
                return (tmp___16);
            }
        }
        if ((int )arg >= cdi->capacity) {
            return (-22);
        }
        tmp___17 = cdrom_slot_status(cdi, (int )arg);
        return (tmp___17);
    }
    else if (cmd == 21287) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROM_DISC_STATUS\n");
        }
        cdrom_count_tracks(cdi, & tracks);
        if (tracks.error) {
            return ((int )tracks.error);
        }
        if (tracks.audio > 0) {
            if (tracks.data == 0) {
                if (tracks.cdi == 0) {
                    if (tracks.xa == 0) {
                        return (100);
                    } else {
                        return (105);
                    }
                } else {
                    return (105);
                }
            } else {
                return (105);
            }
        }
        if (tracks.cdi > 0) {
            return (104);
        }
        if (tracks.xa > 0) {
            return (103);
        }
        if (tracks.data > 0) {
            return (101);
        }
        HsPrintk("<6>cdrom: This disc doesn\'t have any tracks I recognize!\n");
        return (0);
    }
    else if (cmd == 21288) {
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROM_CHANGER_NSLOTS\n");
        }
        return (cdi->capacity);
    }

    if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 4096) {
        ret = mmc_ioctl(cdi, cmd, arg);
        if (ret != -25) {
            return (ret);
        }
    }

    if (cmd == 21259) {
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 256)) {
            return (-38);
        }
        tmp___18 = copy_from_user((void *)(& q), (void *)((struct cdrom_subchnl *)arg),
                (unsigned long )sizeof(q));
        if (tmp___18) {
            return (-14);
        }
        requested = q.cdsc_format;
        if (! ((int )requested == 2)) {
            if (! ((int )requested == 1)) {
                return (-22);
            }
        }
        q.cdsc_format = (unsigned char)2;

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
        back = q.cdsc_format;
        sanitize_format(& q.cdsc_absaddr, & back, requested);
        sanitize_format(& q.cdsc_reladdr, & q.cdsc_format, requested);
        tmp___19 = copy_to_user((void *)((struct cdrom_subchnl *)arg), (void *)(& q),
                (unsigned long )sizeof(q));
        if (tmp___19) {
            return (-14);
        }
        return (0);
    }
    else if (cmd == 21253) {
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 256)) {
            return (-38);
        }
        tmp___20 = copy_from_user((void *)(& header), (void *)((struct cdrom_tochdr *)arg),
                (unsigned long )sizeof(header));
        if (tmp___20) {
            return (-14);
        }

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
        tmp___21 = copy_to_user((void *)((struct cdrom_tochdr *)arg), (void *)(& header),
                (unsigned long )sizeof(header));
        if (tmp___21) {
            return (-14);
        }
        return (0);
    }
    else if (cmd == 21254) {
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 256)) {
            return (-38);
        }
        tmp___22 = copy_from_user((void *)(& entry), (void *)((struct cdrom_tocentry *)arg),
                (unsigned long )sizeof(entry));
        if (tmp___22) {
            return (-14);
        }
        requested_format___0 = entry.cdte_format;
        if (! ((int )requested_format___0 == 2)) {
            if (! ((int )requested_format___0 == 1)) {
                return (-22);
            }
        }
        entry.cdte_format = (unsigned char)2;

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
        sanitize_format(& entry.cdte_addr, & entry.cdte_format, requested_format___0);
        tmp___23 = copy_to_user((void *)((struct cdrom_tocentry *)arg), (void *)(& entry),
                (unsigned long )sizeof(entry));
        if (tmp___23) {
            return (-14);
        }
        return (0);
    }
    else if (cmd == 21251) {
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 256)) {
            return (-38);
        }
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROMPLAYMSF\n");
        }
        tmp___24 = copy_from_user((void *)(& msf), (void *)((struct cdrom_msf *)arg),
                (unsigned long )sizeof(msf));
        if (tmp___24) {
            return (-14);
        }

        tmp___25 = get_nondet_int();
        return (tmp___25);
    }
    else if (cmd == 21252) {
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 256)) {
            return (-38);
        }
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROMPLAYTRKIND\n");
        }
        tmp___26 = copy_from_user((void *)(& ti), (void *)((struct cdrom_ti *)arg),
                (unsigned long )sizeof(ti));
        if (tmp___26) {
            return (-14);
        }
        ret = check_for_audio_disc(cdi, cdo);
        if (ret) {
            return (ret);
        }

        tmp___27 = get_nondet_int();
        return (tmp___27);
    }
    else if (cmd == 21258) {
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 256)) {
            return (-38);
        }
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROMVOLCTRL\n");
        }
        tmp___28 = copy_from_user((void *)(& volume), (void *)((struct cdrom_volctrl *)arg),
                (unsigned long )sizeof(volume));
        if (tmp___28) {
            return (-14);
        }

        tmp___29 = get_nondet_int();
        return (tmp___29);
    }
    else if (cmd == 21267) {
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 256)) {
            return (-38);
        }
        if (debug == 1) {
            HsPrintk("<6>cdrom: entering CDROMVOLREAD\n");
        }

        ret = get_nondet_int();
        if (ret) {
            return (ret);
        }
        tmp___30 = copy_to_user((void *)((struct cdrom_volctrl *)arg), (void *)(& volume___0),
                (unsigned long )sizeof(volume___0));
        if (tmp___30) {
            return (-14);
        }
        return (0);
    }
    else if (cmd == 21256 || cmd == 21255 || cmd == 21249 || cmd == 21250) {
        if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 256)) {
            return (-38);
        }
        if (debug == 1) {
            HsPrintk("<6>cdrom: doing audio ioctl (start/stop/pause/resume)\n");
        }
        ret = check_for_audio_disc(cdi, cdo);
        if (ret) {
            return (ret);
        }

        tmp___31 = get_nondet_int();
        return (tmp___31);
    }

    if (((cdi->ops)->capability & (int )(~ cdi->mask)) & 1024) {

        tmp___32 = get_nondet_int();
        return (tmp___32);
    }
    return (-38);
}

int cdrom_media_changed(struct cdrom_device_info *cdi )
{
    int tmp ;
    if ((unsigned int )cdi == (unsigned int )((void *)0)) {
        return (0);
    } else {
        if ((unsigned int )(cdi->ops)->media_changed == (unsigned int )((void *)0)) {
            return (0);
        }
    }
    if (! (((cdi->ops)->capability & (int )(~ cdi->mask)) & 128)) {
        return (0);
    }
    tmp = media_changed(cdi, 0);
    return (tmp);
}

struct cdrom_device_info *HsCreateCdromDeviceInfo(void)
{
    struct cdrom_device_info *cdi;

    cdi = (struct cdrom_device_info *)malloc(sizeof(struct cdrom_device_info));
    cdi->ops = (struct cdrom_device_ops *)malloc(sizeof(struct cdrom_device_ops));
    cdi->disk = (struct gendisk *)malloc(sizeof(struct gendisk));
    cdi->disk->queue = (struct request_queue *)malloc(sizeof(struct request_queue));
    cdi->disk->queue->boundary_rq = (struct request *)malloc(sizeof(struct request));
    cdi->disk->queue->boundary_rq->sense = (struct request_sense *)malloc(sizeof(struct request_sense));

    cdi->ops->generic_packet = 0;

    return cdi;
}

void HsFreeCdromDeviceInfo(struct cdrom_device_info *cdi)
{
    free(cdi->disk->queue->boundary_rq->sense);
    free(cdi->disk->queue->boundary_rq);
    free(cdi->disk->queue);
    free(cdi->disk);
    free(cdi->ops);
    free(cdi);

    return;
}

struct ctl_table *HsCreateCtlTable(int name)
{
    struct ctl_table *tbl;

    tbl = (struct ctl_table *)malloc(sizeof(struct ctl_table ));
    tbl->ctl_name = name;
    tbl->child = 0;
    tbl->de = (struct proc_dir_entry *)malloc(sizeof(struct proc_dir_entry ));
    tbl->de->next = 0;
    tbl->de->parent = 0;
    tbl->de->subdir = 0;

    return tbl;
}

void HsInitialize(void)
{
    struct ctl_table *tbl;
    struct cdrom_device_info *cdi;

    initialized = 0;

    cdrom_root_table = HsCreateCtlTable(0);
    while(__VERIFIER_nondet_int()) {
        tbl = HsCreateCtlTable(1);
        tbl->child = cdrom_root_table->child;
        cdrom_root_table->child = tbl;
    }

    topCdromPtr = 0;
    while(__VERIFIER_nondet_int()) {
        cdi = HsCreateCdromDeviceInfo();
        cdi->next = topCdromPtr;
        topCdromPtr = cdi;
    }
    return;
}

int main_sub(void)
{
    int tmp ;
    int size ;
    unsigned int cmd;
    unsigned long arg;
    struct cdrom_device_info *cdi;
    struct media_event_desc *med;
    struct file *fp;
    struct inode *ip;

    long *next_writable;

    HsInitialize();

    tmp = cdrom_init();
    if (tmp != 0) return(tmp);

    cdi = HsCreateCdromDeviceInfo();
    tmp = register_cdrom(cdi);
    if (tmp != 0) {
        HsFreeCdromDeviceInfo(cdi);
        return(tmp);
    }

    while (__VERIFIER_nondet_int()) {
        if (__VERIFIER_nondet_int()) {
            med = get_nondet_ptr();
            tmp = cdrom_get_media_event(cdi, med);
        }
        else if (__VERIFIER_nondet_int()) {
            fp = (struct file *)malloc(sizeof(struct file));
            ip = (struct inode *)malloc(sizeof(struct inode));
            ip->i_bdev = (struct block_device *)malloc(sizeof(struct block_device));
            cdrom_open(cdi, ip, fp);
            free(ip->i_bdev);
            free(fp);
            free(ip);
        }
        else if (__VERIFIER_nondet_int()) {
            fp = (struct file *)malloc(sizeof(struct file));
            cdrom_release(cdi, fp);
            free(fp);
        }
        else if (__VERIFIER_nondet_int()) {
            tmp = cdrom_number_of_slots(cdi);
        }
        else if (__VERIFIER_nondet_int()) {
            cmd = get_nondet_int();
            arg = get_nondet_int();
            tmp = mmc_ioctl(cdi, cmd, arg);
        }
        else if (__VERIFIER_nondet_int()) {
            fp = (struct file *)malloc(sizeof(struct file));
            ip = (struct inode *)malloc(sizeof(struct inode));
            ip->i_bdev = (struct block_device *)malloc(sizeof(struct block_device));
            cmd = get_nondet_int();
            arg = get_nondet_int();
            tmp = cdrom_ioctl(fp, cdi, ip, cmd, arg);
            free(ip->i_bdev);
            free(ip);
            free(fp);
        }
        else if (__VERIFIER_nondet_int()) {
            tmp = cdrom_media_changed(cdi);
        }
    }

    tmp = unregister_cdrom(cdi);
    if (tmp == 0) {
        HsFreeCdromDeviceInfo(cdi);
        return tmp;
    }
    cdrom_exit();
    return (0);
}

int main()
{
    return main_sub();
}
