# 1 "ll_rw_blkk.c"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "ll_rw_blkk.c"



void *mallocnull(int);
void *malloc(int);
void free(void *);





int nondet;


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
typedef u32 dma_addr_t;
typedef __u32 __kernel_dev_t;
typedef __kernel_dev_t dev_t;
typedef __kernel_ino_t ino_t;
typedef __kernel_mode_t mode_t;
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
typedef unsigned long sector_t;
typedef unsigned int gfp_t;
struct thread_info;
struct task_struct;
struct siginfo;
struct mm_struct;
struct completion;
struct completion;
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
struct exec_domain;
struct exec_domain;
typedef unsigned long mm_segment_t;
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
struct rw_semaphore;
struct rw_semaphore;
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
typedef unsigned long pgd_t[2];
typedef unsigned long pgprot_t;
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
struct siginfo;
struct __anonstruct_sigset_t_12 {
 unsigned long sig[2] ;
};
typedef struct __anonstruct_sigset_t_12 sigset_t;
typedef void __signalfn_t(int );
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
struct siginfo;
struct user_struct;
struct sigpending {
 struct list_head list ;
 sigset_t signal ;
};
struct dentry;
struct dentry;
struct vfsmount;
struct vfsmount;
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
struct kmem_cache {
};
typedef struct kmem_cache kmem_cache_t;
struct page;
struct page;
struct file;
struct file;
struct vm_area_struct;
struct vm_area_struct;
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
struct exec_domain;
struct rlimit {
 unsigned long rlim_cur ;
 unsigned long rlim_max ;
};
struct timer_base_s;
struct timer_base_s;
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
struct hrtimer_base;
struct hrtimer_base;
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
struct namespace;
struct namespace;
struct workqueue_struct;
struct workqueue_struct;
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
struct kioctx;
struct kioctx;
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
struct mm_struct;
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
struct tty_struct;
struct key;
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
struct backing_dev_info;
struct backing_dev_info;
struct reclaim_state;
struct reclaim_state;
struct io_context;
struct group_info {
 int ngroups ;
 atomic_t usage ;
 gid_t small_block[32] ;
 int nblocks ;
 gid_t *blocks[0] ;
};
struct audit_context;
struct audit_context;
struct linux_binfmt;
struct files_struct;
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
struct key;
typedef __u32 Elf32_Addr;
typedef __u16 Elf32_Half;
typedef __u32 Elf32_Word;
struct elf32_sym {
 Elf32_Word st_name ;
 Elf32_Addr st_value ;
 Elf32_Word st_size ;
 unsigned char st_info ;
 unsigned char st_other ;
 Elf32_Half st_shndx ;
};
typedef struct elf32_sym Elf32_Sym;
struct kobject;
struct kobject;
struct module;
struct module;
struct attribute {
 char *name ;
 struct module *owner ;
 mode_t mode ;
};
struct attribute_group {
 char *name ;
 struct attribute **attrs ;
};
struct vm_area_struct;
struct sysfs_ops {
 ssize_t (*show)(struct kobject * , struct attribute * , char * ) ;
 ssize_t (*store)(struct kobject * , struct attribute * , char * , size_t ) ;
};
struct iattr;
struct kref {
 atomic_t refcount ;
};
struct kset;
struct kobj_type;
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
struct subsystem;
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
struct module;
struct task_struct;
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
struct module;
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
struct exception_table_entry;
struct exception_table_entry;
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
struct module_param_attrs;
struct module_param_attrs;
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
struct device_driver;
struct device_driver;
enum bdi_state {
    BDI_pdflush = 0,
    BDI_write_congested = 1,
    BDI_read_congested = 2,
    BDI_unused = 3
} ;
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
struct nameidata;
struct nameidata;
struct vfsmount;
struct qstr {
 unsigned int hash ;
 unsigned int len ;
 unsigned char *name ;
};
struct dcookie_struct;
struct dcookie_struct;
struct inode;
union __anonunion_d_u_33 {
 struct list_head d_child ;
 struct rcu_head d_rcu ;
};
struct dentry_operations;
struct super_block;
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
struct radix_tree_node;
struct radix_tree_root {
 unsigned int height ;
 gfp_t gfp_mask ;
 struct radix_tree_node *rnode ;
};
struct prio_tree_node;
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
struct hd_geometry;
struct hd_geometry;
struct iovec;
struct iovec;
struct nameidata;
struct kiocb;
struct pipe_inode_info;
struct pipe_inode_info;
struct poll_table_struct;
struct poll_table_struct;
struct kstatfs;
struct kstatfs;
struct vm_area_struct;
struct vfsmount;
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
typedef __kernel_uid32_t qid_t;
typedef __u64 qsize_t;
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
struct quota_format_type;
struct quota_format_type;
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
struct super_block;
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
struct page;
struct address_space;
struct address_space;
struct writeback_control;
struct writeback_control;
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
struct backing_dev_info;
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
struct hd_struct;
struct gendisk;
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
struct inode_operations;
struct file_operations;
struct file_lock;
struct cdev;
struct dnotify_struct;
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
struct nlm_lockowner;
struct nlm_lockowner;
struct nfs_lock_info {
 u32 state ;
 u32 flags ;
 struct nlm_lockowner *owner ;
};
struct nfs4_lock_state;
struct nfs4_lock_state;
struct nfs4_lock_info {
 struct nfs4_lock_state *owner ;
};
struct fasync_struct;
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
struct file_system_type;
struct super_operations;
struct export_operations;
struct xattr_handler;
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
 int (*readdir)(struct file * , void * , int (*)(void * , char * , int ,
             loff_t , ino_t , unsigned int ) ) ;
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
 ssize_t (*sendfile)(struct file * , loff_t * , size_t , int (*)(read_descriptor_t * ,
                  struct page * ,
                  unsigned long ,
                  unsigned long ) ,
      void * ) ;
 ssize_t (*sendpage)(struct file * , struct page * , int , size_t , loff_t * ,
      int ) ;
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
struct seq_file;
struct seq_file;
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
 struct super_block *(*get_sb)(struct file_system_type * , int , char * ,
          void * ) ;
 void (*kill_sb)(struct super_block * ) ;
 struct module *owner ;
 struct file_system_type *next ;
 struct list_head fs_supers ;
};
struct bio;
struct anon_vma;
struct mm_struct;
struct __anonstruct_vm_set_43 {
 struct list_head list ;
 void *parent ;
 struct vm_area_struct *head ;
};
union __anonunion_shared_42 {
 struct __anonstruct_vm_set_43 vm_set ;
 struct raw_prio_tree_node prio_tree_node ;
};
struct vm_operations_struct;
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
struct inode;
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
struct page_state {
 unsigned long nr_dirty ;
 unsigned long nr_writeback ;
 unsigned long nr_unstable ;
 unsigned long nr_page_table_pages ;
 unsigned long nr_mapped ;
 unsigned long nr_slab ;
 unsigned long pgpgin ;
 unsigned long pgpgout ;
 unsigned long pswpin ;
 unsigned long pswpout ;
 unsigned long pgalloc_high ;
 unsigned long pgalloc_normal ;
 unsigned long pgalloc_dma32 ;
 unsigned long pgalloc_dma ;
 unsigned long pgfree ;
 unsigned long pgactivate ;
 unsigned long pgdeactivate ;
 unsigned long pgfault ;
 unsigned long pgmajfault ;
 unsigned long pgrefill_high ;
 unsigned long pgrefill_normal ;
 unsigned long pgrefill_dma32 ;
 unsigned long pgrefill_dma ;
 unsigned long pgsteal_high ;
 unsigned long pgsteal_normal ;
 unsigned long pgsteal_dma32 ;
 unsigned long pgsteal_dma ;
 unsigned long pgscan_kswapd_high ;
 unsigned long pgscan_kswapd_normal ;
 unsigned long pgscan_kswapd_dma32 ;
 unsigned long pgscan_kswapd_dma ;
 unsigned long pgscan_direct_high ;
 unsigned long pgscan_direct_normal ;
 unsigned long pgscan_direct_dma32 ;
 unsigned long pgscan_direct_dma ;
 unsigned long pginodesteal ;
 unsigned long slabs_scanned ;
 unsigned long kswapd_steal ;
 unsigned long kswapd_inodesteal ;
 unsigned long pageoutrun ;
 unsigned long allocstall ;
 unsigned long pgrotated ;
 unsigned long nr_bounce ;
};
struct page;
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
struct request_queue;
struct sg_iovec;
struct klist_node;
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
struct device;
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
struct device_driver;
struct bus_attribute;
struct device_attribute;
struct driver_attribute;
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
struct dma_coherent_mem;
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
struct timer_rand_state;
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
struct exception_table_entry {
 unsigned long insn ;
 unsigned long fixup ;
};
struct scatterlist {
 struct page *page ;
 unsigned int offset ;
 dma_addr_t dma_address ;
 unsigned int length ;
};
struct elevator_queue;
typedef struct elevator_queue elevator_t;
struct request_pm_state;
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
struct cfq_queue;
struct cfq_queue;
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
struct request;
typedef void rq_end_io_fn(struct request * , int );
struct request_list {
 int count[2] ;
 int starved[2] ;
 int elvpriv ;
 mempool_t *rq_pool ;
 wait_queue_head_t wait[2] ;
};
struct blk_queue_tag;
struct request {

 struct request *next;
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
 struct request_queue *q ;
 struct request_list *rl ;
 struct completion *waiting ;
 void *special ;
 unsigned int cmd_len ;
 unsigned char cmd[16] ;
 unsigned int data_len ;
 void *data ;
 unsigned int sense_len ;
 void *sense ;
 unsigned int timeout ;
 int retries ;
 struct request_pm_state *pm ;
 rq_end_io_fn *end_io ;
 void *end_io_data ;
};
struct request_head {
 struct request* next;
};
struct request_pm_state {
 int pm_step ;
 u32 pm_state ;
 void *data ;
};
typedef int elevator_merge_fn(struct request_queue * , struct request ** , struct bio * );
typedef void elevator_merge_req_fn(struct request_queue * , struct request * , struct request * );
typedef void elevator_merged_fn(struct request_queue * , struct request * );
typedef int elevator_dispatch_fn(struct request_queue * , int );
typedef void elevator_add_req_fn(struct request_queue * , struct request * );
typedef int elevator_queue_empty_fn(struct request_queue * );
typedef struct request *elevator_request_list_fn(struct request_queue * , struct request * );
typedef void elevator_completed_req_fn(struct request_queue * , struct request * );
typedef int elevator_may_queue_fn(struct request_queue * , int , struct bio * );
typedef int elevator_set_req_fn(struct request_queue * , struct request * , struct bio * ,
                                gfp_t );
typedef void elevator_put_req_fn(struct request_queue * , struct request * );
typedef void elevator_activate_req_fn(struct request_queue * , struct request * );
typedef void elevator_deactivate_req_fn(struct request_queue * , struct request * );
typedef int elevator_init_fn(struct request_queue * , elevator_t * );
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
typedef int merge_request_fn(struct request_queue * , struct request * , struct bio * );
typedef int merge_requests_fn(struct request_queue * , struct request * , struct request * );
typedef void request_fn_proc(struct request_queue *q );
typedef int make_request_fn(struct request_queue *q , struct bio *bio );
typedef int prep_rq_fn(struct request_queue * , struct request * );
typedef void unplug_fn(struct request_queue * );
struct bio_vec;
typedef int merge_bvec_fn(struct request_queue * , struct bio * , struct bio_vec * );
typedef void activity_fn(void *data , int rw );
typedef int issue_flush_fn(struct request_queue * , struct gendisk * , sector_t * );
typedef void prepare_flush_fn(struct request_queue * , struct request * );
typedef void softirq_done_fn(struct request * );
struct blk_queue_tag {
 struct request **tag_index ;
 unsigned long *tag_map ;

 struct request *next;
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
struct work_struct;
struct reclaim_state {
 unsigned long reclaimed_slab ;
};
struct address_space;
struct writeback_control;
struct backing_dev_info;
enum writeback_sync_modes {
    WB_SYNC_NONE = 0,
    WB_SYNC_ALL = 1,
    WB_SYNC_HOLD = 2
} ;
struct writeback_control {
 struct backing_dev_info *bdi ;
 enum writeback_sync_modes sync_mode ;
 unsigned long *older_than_this ;
 long nr_to_write ;
 long pages_skipped ;
 loff_t start ;
 loff_t end ;
 unsigned int nonblocking : 1 ;
 unsigned int encountered_congestion : 1 ;
 unsigned int for_kupdate : 1 ;
 unsigned int for_reclaim : 1 ;
 unsigned int for_writepages : 1 ;
};
struct file;
struct softirq_action {
 void (*action)(struct softirq_action * ) ;
 void *data ;
};
struct request;
struct scatterlist;
struct queue_sysfs_entry {
 struct attribute attr ;
 ssize_t (*show)(struct request_queue * , char * ) ;
 ssize_t (*store)(struct request_queue * , char * , size_t ) ;
};





void *mempool_alloc_slab(gfp_t gfp_mask , void *pool_data )
{
 return;
}

void mempool_free_slab(void *element , void *pool_data )
{
 return;
}

int HsPrintk(char *fmt, ...)
{
 int a; return a;
}

int __make_request(struct request_queue *q , struct bio *bio )
{
 int a; return a;
}

void blk_backing_dev_unplug(struct backing_dev_info *bdi, struct page *page)
{
 return;
}

void blk_unplug_timeout(unsigned long data )
{
 return;
}

void blk_unplug_work(void *data )
{
 return;
}

int flush_dry_bio_endio(struct bio *bio, unsigned int bytes, int error)
{
 int a; return a;
}

int ll_front_merge_fn(struct request_queue *q , struct request *req , struct bio *bio )
{
 int a; return a;
}

int ll_merge_requests_fn(struct request_queue *q , struct request *req , struct request *next )
{
 int a; return a;
}

ssize_t queue_attr_show(struct kobject *kobj, struct attribute *attr, char *page)
{
 ssize_t a; return a;
}
size_t queue_attr_store(struct kobject *kobj, struct attribute *attr, char *page, size_t length)
{
 ssize_t a; return a;
}

ssize_t elv_iosched_show(struct request_queue *q, char *page)
{
 ssize_t a; return a;
}
ssize_t elv_iosched_store(struct request_queue *q, char *page, size_t count)
{
 ssize_t a; return a;
}

struct thread_info *HsThreadInfo;

unsigned long max_low_pfn;
unsigned long max_pfn;
unsigned long jiffies;
unsigned long blk_max_low_pfn;
unsigned long blk_max_pfn;

struct page *mem_map;
struct workqueue_struct *kblockd_workqueue;

kmem_cache_t *request_cachep;
kmem_cache_t *requestq_cachep;
kmem_cache_t *iocontext_cachep;

struct request_head per_cpu__blk_cpu_done;

struct sysfs_ops queue_sysfs_ops;





struct queue_sysfs_entry queue_requests_entry;






struct queue_sysfs_entry queue_ra_entry;






struct queue_sysfs_entry queue_max_hw_sectors_entry;







struct queue_sysfs_entry queue_max_sectors_entry;






struct queue_sysfs_entry queue_iosched_entry =
{
 {"scheduler", (struct module *)0, (mode_t )420},
 &elv_iosched_show, &elv_iosched_store
};

struct attribute *default_attrs[6] =
{
 &queue_requests_entry.attr,
 &queue_ra_entry.attr,
 &queue_max_hw_sectors_entry.attr,
 &queue_max_sectors_entry.attr,
 &queue_iosched_entry.attr,
 (struct attribute *)((void *)0)
};

struct kobj_type queue_ktype =
{
 (void (*)(struct kobject * ))0,
 &queue_sysfs_ops,
 default_attrs
};

char *rq_flags[26] =
{
 (char * )"REQ_RW",
 (char * )"REQ_FAILFAST",
 (char * )"REQ_SORTED",
 (char * )"REQ_SOFTBARRIER",
 (char * )"REQ_HARDBARRIER",
 (char * )"REQ_FUA",
 (char * )"REQ_CMD",
 (char * )"REQ_NOMERGE",
 (char * )"REQ_STARTED",
 (char * )"REQ_DONTPREP",
 (char * )"REQ_QUEUED",
 (char * )"REQ_ELVPRIV",
 (char * )"REQ_PC",
 (char * )"REQ_BLOCK_PC",
 (char * )"REQ_SENSE",
 (char * )"REQ_FAILED",
 (char * )"REQ_QUIET",
 (char * )"REQ_SPECIAL",
 (char * )"REQ_DRIVE_CMD",
 (char * )"REQ_DRIVE_TASK",
 (char * )"REQ_DRIVE_TASKFILE",
 (char * )"REQ_PREEMPT",
 (char * )"REQ_PM_SUSPEND",
 (char * )"REQ_PM_RESUME",
 (char * )"REQ_PM_SHUTDOWN",
 (char * )"REQ_ORDERED_COLOR"
};

wait_queue_head_t congestion_wqh[2] = {
 {{{}}, {& congestion_wqh[0].task_list, & congestion_wqh[0].task_list}},
 {{{}}, {& congestion_wqh[1].task_list, & congestion_wqh[1].task_list}}
};

struct thread_info *HsCreateThreadInfo()
{
 struct task_struct *task;
 struct thread_info *threadinfo;
 threadinfo = (struct thread_info *)malloc(sizeof(struct thread_info));
 task = (struct task_struct *)malloc(sizeof(struct task_struct));
 task->io_context = (struct io_context *)malloc(sizeof(struct io_context));
 threadinfo->task = task;
 return threadinfo;
}

void HsFreeThreadInfo(struct thread_info *ti)
{
 if (ti->task->io_context != 0) free(ti->task->io_context);
 free(ti->task);
 free(ti);
 return;
}

struct gendisk *HsCreateGendisk()
{
 struct gendisk *disk;

 disk = (struct gendisk *)malloc(sizeof(struct gendisk));
 disk->queue = (struct request_queue *)malloc(sizeof(struct request_queue));

 return disk;
}

void HsFreeGendisk(struct gendisk *disk)
{
 if (disk->queue != 0) free(disk->queue);
 free(disk);
 return;
}

struct block_device *HsCreateBdev(void)
{
        struct block_device *a;
        a = (struct block_device *)malloc(sizeof(struct block_device));
        a->bd_inode = (struct inode *)malloc(sizeof(struct inode));
        a->bd_disk = HsCreateGendisk();
        a->bd_part = 0;
        return a;
}

void HsFreeBdev(struct block_device *bdev)
{
        free(bdev->bd_inode);
        HsFreeGendisk(bdev->bd_disk);
        free(bdev);
        return;
}

struct bio *HsCreateBio()
{
 struct bio *bio;
 bio = (struct bio *)malloc(sizeof(struct bio));
 return bio;
}

void HsFreeBio(struct bio *bio)
{
 free(bio);
 return;
}

struct request *HsCreateRequest()
{
 struct request *rq;
 struct bio *bio;
 struct blk_queue_tag *bqt;

 rq = (struct request *)malloc(sizeof(struct request));

 rq->rq_disk = HsCreateGendisk();

 rq->bio = 0;
 while ((&nondet < 0)) {
  bio = HsCreateBio();
  bio->bi_next = rq->bio;
  rq->bio = bio;
 }

 rq->next = rq;

 while (1) { break; }

 return rq;
}

void HsFreeRequest(struct request *rq)
{
 struct bio *bio;
 struct blk_queue_tag *bqt;

 if (rq->rq_disk) HsFreeGendisk(rq->rq_disk);

 while (rq->bio != 0) {
  bio = rq->bio;
  rq->bio = bio->bi_next;
  HsFreeBio(bio);
 }

 free(rq);
 return;
}

struct blk_queue_tag *HsCreateBqt()
{
 struct blk_queue_tag *bqt;
 bqt = (struct blk_queue_tag *)malloc(sizeof(struct blk_queue_tag));
 bqt->tag_index = (struct blk_queue_tag *)malloc(sizeof(struct blk_queue_tag));
 bqt->tag_map = (struct blk_queue_tag *)malloc(sizeof(struct blk_queue_tag));
 bqt->next = bqt;
 return bqt;
}

void HsFreeBqt(struct blk_queue_tag *bqt)
{
 free((void *)bqt->tag_index);
 free((void *)bqt->tag_map);
 free((void *)bqt);
 return;
}

struct request_queue *HsCreateRequestQueue()
{
 struct request_queue *q;
 struct blk_queue_tag *bqt;
 struct request *rq;

 q = (struct request_queue *)malloc(sizeof(struct request_queue));

 bqt = HsCreateBqt();
 bqt->next = bqt;

 while ((&nondet < 0)) {
  rq = HsCreateRequest();


  rq->next = bqt->next;
  bqt->next = rq;
 }

 q->queue_tags = bqt;

 return q;
}

void HsFreeRequestQueue(struct request_queue *q)
{
 struct blk_queue_tag *bqt;
 struct request *rq, *tmp;

 bqt = q->queue_tags;
 if (bqt != 0) {
  rq = bqt->next;

  while (rq != (struct request *)bqt) {
   tmp = rq;
   rq = rq->next;
   HsFreeRequest(tmp);
  }

  HsFreeBqt(bqt);
 }
 free(q);
 return;
}

void HsInitRequestQueue(struct request_queue *q)
{
 struct blk_queue_tag *bqt;
 struct request *rq;

 bqt = HsCreateBqt();
 bqt->next = bqt;

 while ((&nondet < 0)) {
  rq = HsCreateRequest();
  rq->next = bqt->next;
  bqt->next = rq;
 }

 q->queue_tags = bqt;

 return;
}

void HsInitialize()
{
 struct request *rq;

 max_low_pfn = 1;
 max_pfn = 1;
 HsThreadInfo = HsCreateThreadInfo();

 while ((&nondet < 0)) {
  rq = HsCreateRequest();
  rq->next = per_cpu__blk_cpu_done.next;
  per_cpu__blk_cpu_done.next = rq;
 }
 return;
}

void HsFinalize()
{
 HsFreeThreadInfo(HsThreadInfo);
 HsThreadInfo = 0;
 free(request_cachep);
 request_cachep = 0;
 free(requestq_cachep);
 requestq_cachep = 0;
 free(iocontext_cachep);
 iocontext_cachep = 0;
 return;
}

struct kobject *kobject_get(struct kobject *x)
{
 struct kobject *a; return a;
}

int HsSnprintf(char *buf, size_t size, char *fmt, ...)
{
 int a; return a;
}

int kobject_register(struct kobject *x)
{
 int a; return a;
}

void kobject_unregister(struct kobject *x)
{
 return;
}

int elv_register_queue(struct request_queue *q)
{
 int a; return a;
}

int blk_register_queue(struct gendisk *disk )
{
 int ret ;
 struct request_queue *q ;

 q = disk->queue;
 if (!q) {
  return (-6);
 } else {
  if (!q->request_fn) {
   return (-6);
  }
 }
 q->kobj.parent = kobject_get(& disk->kobj);
 if (!q->kobj.parent) {
  return (-16);
 }
 HsSnprintf(q->kobj.name, 20U, "%s", "queue");
 q->kobj.ktype = &queue_ktype;
 ret = kobject_register(&q->kobj);
 if (ret < 0) {
  return (ret);
 }
 ret = elv_register_queue(q);
 if (ret) {
  kobject_unregister(& q->kobj);
  return (ret);
 }
 return (0);
}

void kobject_put(struct kobject *x)
{
 return;
}

void elv_unregister_queue(struct request_queue *q)
{
 return;
}

void blk_unregister_queue(struct gendisk *disk)
{
 struct request_queue *q ;

 q = disk->queue;
 if (q) {
  if (q->request_fn) {
   elv_unregister_queue(q);
   kobject_unregister(& q->kobj);
   kobject_put(& disk->kobj);
  }
 }
 return;
}

void bio_endio(struct bio *x, unsigned int y, int z)
{
 return;
}

int ordered_bio_endio(struct request *rq, struct bio *bio, unsigned int nbytes, int error)
{
 struct request_queue *q;
 bio_end_io_t *endio;
 void *private;

 q = rq->q;
 if ((unsigned int )(& q->bar_rq) != (unsigned int )rq) {
  return (0);
 }
 if (error) {
  if (! q->orderr) {
   q->orderr = error;
  }
 }
 endio = bio->bi_end_io;
 private = bio->bi_private;
 bio->bi_end_io = & flush_dry_bio_endio;
 bio->bi_private = (void *)q;
 bio_endio(bio, nbytes, error);
 bio->bi_end_io = endio;
 bio->bi_private = private;
 return (1);
}

void blk_dump_rq_flags(struct request *rq, char *msg)
{
 int bit ;
 char *tmp ;

 if (rq->rq_disk) {
  tmp = (char *)((rq->rq_disk)->disk_name);
 } else {
  tmp = "?";
 }
 HsPrintk("%s: dev %s: flags = ", msg, tmp);
 bit = 0;
 while (1) {
  if (rq->flags & (unsigned long )(1 << bit)) {
   HsPrintk("%s ", rq_flags[bit]);
  }
  bit ++;
  if (! (bit < 26)) {
   break;
  }
 }
 HsPrintk("\nsector %llu, nr/cnr %lu/%u\n", (unsigned long long )rq->sector, rq->nr_sectors,
     rq->current_nr_sectors);
 HsPrintk("bio %p, biotail %p, buffer %p, data %p, len %u\n", rq->bio, rq->biotail,
     rq->buffer, rq->data, rq->data_len);
 if (rq->flags & 12288UL) {
  HsPrintk("cdb: ");
  bit = 0;
  while ((unsigned int )bit < sizeof(rq->cmd)) {
   HsPrintk("%02x ", rq->cmd[bit]);
   bit ++;
  }
  HsPrintk("\n");
 }
 return;
}

void *lowmem_page_address(struct page *page )
{
 return ((void *)((((unsigned long )(page - mem_map) << 12) - 0UL) + 3221225472UL));
}

void blk_recalc_rq_sectors(struct request *rq , int nsect)
{
 void *tmp ;

 if (rq->flags & 64UL) {
  rq->hard_sector += (sector_t )nsect;
  rq->hard_nr_sectors -= (unsigned long )nsect;
  if (rq->nr_sectors >= rq->hard_nr_sectors) {
   if (rq->sector <= rq->hard_sector) {
    rq->sector = rq->hard_sector;
    rq->nr_sectors = rq->hard_nr_sectors;
    rq->hard_cur_sectors = ((rq->bio)->bi_io_vec + (rq->bio)->bi_idx)->bv_len >> 9;
    rq->current_nr_sectors = rq->hard_cur_sectors;
    tmp = lowmem_page_address(((rq->bio)->bi_io_vec + (rq->bio)->bi_idx)->bv_page);
    rq->buffer = (char *)(tmp + ((rq->bio)->bi_io_vec + (rq->bio)->bi_idx)->bv_offset);
   }
  }
  if (rq->nr_sectors < (unsigned long )rq->current_nr_sectors) {
   HsPrintk("blk: request botched\n");
   rq->nr_sectors = (unsigned long )rq->current_nr_sectors;
  }
 }
 return;
}

int bio_phys_segments(struct request_queue *x, struct bio *y)
{
 int a; return a;
}

int bio_hw_segments(struct request_queue *x, struct bio *y)
{
 int a; return a;
}

int blk_phys_contig_segment(struct request_queue *q, struct bio *bio, struct bio *nxt)
{
 if (! (q->queue_flags & 1UL)) {
  return (0);
 }
 if (! ((((unsigned long )((bio->bi_io_vec + ((int )bio->bi_vcnt - 1))->bv_page - mem_map) << 12) + (unsigned long )(bio->bi_io_vec + ((int )bio->bi_vcnt - 1))->bv_offset) + (unsigned long )(bio->bi_io_vec + ((int )bio->bi_vcnt - 1))->bv_len == ((unsigned long )((nxt->bi_io_vec + nxt->bi_idx)->bv_page - mem_map) << 12) + (unsigned long )(nxt->bi_io_vec + nxt->bi_idx)->bv_offset)) {
  return (0);
 }
 if (bio->bi_size + nxt->bi_size > q->max_segment_size) {
  return (0);
 }
 if (((((unsigned long )((bio->bi_io_vec + ((int )bio->bi_vcnt - 1))->bv_page - mem_map) << 12) + (unsigned long )(bio->bi_io_vec + ((int )bio->bi_vcnt - 1))->bv_offset) | q->seg_boundary_mask) == ((((((unsigned long )((nxt->bi_io_vec + nxt->bi_idx)->bv_page - mem_map) << 12) + (unsigned long )(nxt->bi_io_vec + nxt->bi_idx)->bv_offset) + (unsigned long )(nxt->bi_io_vec + nxt->bi_idx)->bv_len) - 1UL) | q->seg_boundary_mask)) {
  return (1);
 }
 return (0);
}

void blk_recount_segments(struct request_queue *q , struct bio *bio )
{
 struct bio_vec *bv ;
 struct bio_vec *bvprv ;
 int i ;
 int nr_phys_segs ;
 int nr_hw_segs ;
 int seg_size ;
 int hw_seg_size ;
 int cluster ;
 int high ;
 int highprv ;
 long tmp ;

 bvprv = (struct bio_vec *)((void *)0);
 highprv = 1;

 tmp = get_nondet_int();
 if (tmp) {
  return;
 }
 cluster = (int )(q->queue_flags & 1UL);
 nr_hw_segs = 0;
 nr_phys_segs = nr_hw_segs;
 seg_size = nr_phys_segs;
 hw_seg_size = seg_size;
 bv = bio->bi_io_vec + bio->bi_idx;
 i = (int )bio->bi_idx;
 while (i < (int )bio->bi_vcnt) {
  high = (unsigned long )(bv->bv_page - mem_map) >= q->bounce_pfn;
  if (high) {
   goto new_hw_segment;
  } else {
   if (highprv) {
    goto new_hw_segment;
   }
  }
  if (cluster) {
   if ((unsigned int )seg_size + bv->bv_len > q->max_segment_size) {
    goto new_segment;
   }
   if (! ((((unsigned long )(bvprv->bv_page - mem_map) << 12) + (unsigned long )bvprv->bv_offset) + (unsigned long )bvprv->bv_len == ((unsigned long )(bv->bv_page - mem_map) << 12) + (unsigned long )bv->bv_offset)) {
    goto new_segment;
   }
   if (! (((((unsigned long )(bvprv->bv_page - mem_map) << 12) + (unsigned long )bvprv->bv_offset) | q->seg_boundary_mask) == ((((((unsigned long )(bv->bv_page - mem_map) << 12) + (unsigned long )bv->bv_offset) + (unsigned long )bv->bv_len) - 1UL) | q->seg_boundary_mask))) {
    goto new_segment;
   }
   seg_size = (int )((unsigned int )seg_size + bv->bv_len);
   hw_seg_size = (int )((unsigned int )hw_seg_size + bv->bv_len);
   bvprv = bv;
   goto __Cont;
  }
new_segment:
  if (((((((unsigned long )(bvprv->bv_page - mem_map) << 12) + (unsigned long )bvprv->bv_offset) + (unsigned long )bvprv->bv_len) | (((unsigned long )(bv->bv_page - mem_map) << 12) + (unsigned long )bv->bv_offset)) & 4294967295UL) == 0UL) {
   hw_seg_size = (int )((unsigned int )hw_seg_size + bv->bv_len);
  } else {
new_hw_segment:
   if ((unsigned int )hw_seg_size > bio->bi_hw_front_size) {
    bio->bi_hw_front_size = (unsigned int )hw_seg_size;
   }
   hw_seg_size = (int )bv->bv_len;
   nr_hw_segs ++;
  }
  nr_phys_segs ++;
  bvprv = bv;
  seg_size = (int )bv->bv_len;
  highprv = high;
__Cont:
  bv ++;
  i ++;
 }
 if ((unsigned int )hw_seg_size > bio->bi_hw_back_size) {
  bio->bi_hw_back_size = (unsigned int )hw_seg_size;
 }
 if (nr_hw_segs == 1) {
  if ((unsigned int )hw_seg_size > bio->bi_hw_front_size) {
   bio->bi_hw_front_size = (unsigned int )hw_seg_size;
  }
 }
 bio->bi_phys_segments = (unsigned short )nr_phys_segs;
 bio->bi_hw_segments = (unsigned short )nr_hw_segs;
 bio->bi_flags |= 8UL;
 return;
}

int blk_hw_contig_segment(struct request_queue *q, struct bio *bio, struct bio *nxt)
{
 long tmp ;
 long tmp___0 ;


 tmp = get_nondet_int();
 if (tmp) {
  blk_recount_segments(q, bio);
 }

 tmp___0 = get_nondet_int();
 if (tmp___0) {
  blk_recount_segments(q, nxt);
 }
 if (! (((((((unsigned long )((bio->bi_io_vec + ((int )bio->bi_vcnt - 1))->bv_page - mem_map) << 12) + (unsigned long )(bio->bi_io_vec + ((int )bio->bi_vcnt - 1))->bv_offset) + (unsigned long )(bio->bi_io_vec + ((int )bio->bi_vcnt - 1))->bv_len) | (((unsigned long )((nxt->bi_io_vec + nxt->bi_idx)->bv_page - mem_map) << 12) + (unsigned long )(nxt->bi_io_vec + nxt->bi_idx)->bv_offset)) & 4294967295UL) == 0UL)) {
  return (0);
 }
 if (bio->bi_size + nxt->bi_size > q->max_segment_size) {
  return (0);
 }
 return (1);
}

void blk_recalc_rq_segments(struct request *rq )
{
 struct bio *bio ;
 struct bio *prevbio ;
 int nr_phys_segs ;
 int nr_hw_segs ;
 unsigned int phys_size ;
 unsigned int hw_size ;
 struct request_queue *q ;
 int tmp ;
 int tmp___0 ;
 int pseg ;
 int hseg ;
 int tmp___1 ;
 int tmp___2 ;

 prevbio = (struct bio *)((void *)0);
 q = rq->q;
 if (! rq->bio) {
  return;
 }
 nr_hw_segs = 0;
 nr_phys_segs = nr_hw_segs;
 hw_size = (unsigned int )nr_phys_segs;
 phys_size = hw_size;
 if (rq->bio) {
  bio = rq->bio;
  while (bio) {
   bio->bi_flags &= 4294967287UL;
   tmp = bio_phys_segments(q, bio);
   nr_phys_segs += tmp;
   tmp___0 = bio_hw_segments(q, bio);
   nr_hw_segs += tmp___0;
   if (prevbio) {
    pseg = (int )((phys_size + prevbio->bi_size) + bio->bi_size);
    hseg = (int )((hw_size + prevbio->bi_size) + bio->bi_size);
    tmp___1 = blk_phys_contig_segment(q, prevbio, bio);
    if (tmp___1) {
     if ((unsigned int )pseg <= q->max_segment_size) {
      nr_phys_segs --;
      phys_size += prevbio->bi_size + bio->bi_size;
     } else {
      phys_size = 0U;
     }
    } else {
     phys_size = 0U;
    }
    tmp___2 = blk_hw_contig_segment(q, prevbio, bio);
    if (tmp___2) {
     if ((unsigned int )hseg <= q->max_segment_size) {
      nr_hw_segs --;
      hw_size += prevbio->bi_size + bio->bi_size;
     } else {
      hw_size = 0U;
     }
    } else {
     hw_size = 0U;
    }
   }
   prevbio = bio;
   bio = bio->bi_next;
  }
 }
 rq->nr_phys_segments = (unsigned short )nr_phys_segs;
 rq->nr_hw_segments = (unsigned short )nr_hw_segs;
 return;
}

void __bug(char *file, int line, void *data)
{
 return;
}

int __end_that_request_first(struct request *req , int uptodate , int nr_bytes )
{
 int total_bytes ;
 int bio_nbytes ;
 int error ;
 int next_idx ;
 struct bio *bio ;
 long tmp ;
 char *tmp___0 ;
 int rw ;
 int nbytes ;
 int tmp___1 ;
 int idx ;
 long tmp___2 ;
 long tmp___3 ;
 long tmp___4 ;
 long tmp___5 ;
 int tmp___6 ;

 next_idx = 0;
 error = 0;

 tmp = get_nondet_int();
 if (tmp) {
  if (! uptodate) {
   error = -5;
  } else {
   error = uptodate;
  }
 }
 if (! (req->flags & 8192UL)) {
  req->errors = 0;
 }
 if (! uptodate) {
  if (req->flags & 64UL) {
   if (! (req->flags & 65536UL)) {
    if (req->rq_disk) {
     tmp___0 = (char *)((req->rq_disk)->disk_name);
    } else {
     tmp___0 = "?";
    }
    HsPrintk("end_request: I/O error, dev %s, sector %llu\n", tmp___0, (unsigned long long )req->sector);
   }
  }
 }
 if (req->flags & 64UL) {
  if (req->rq_disk) {
   rw = (int )(req->flags & 1UL);
   (req->rq_disk)->dkstats.sectors[rw] += (unsigned int )(nr_bytes >> 9);
  }
 }
 bio_nbytes = 0;
 total_bytes = bio_nbytes;
 while (1) {
  bio = req->bio;
  if (bio == 0) {
   break;
  }
  if ((unsigned int )nr_bytes >= bio->bi_size) {
   req->bio = bio->bi_next;
   nbytes = (int )bio->bi_size;
   tmp___1 = ordered_bio_endio(req, bio, (unsigned int )nbytes, error);
   if (! tmp___1) {
    bio_endio(bio, (unsigned int )nbytes, error);
   }
   next_idx = 0;
   bio_nbytes = 0;

   // BUG:
   // Memory Leak
    HsFreeBio(bio);
  } else {
   idx = (int )bio->bi_idx + next_idx;

   tmp___2 = get_nondet_int();
   if (tmp___2) {
    blk_dump_rq_flags(req, (char *)"__end_that");
    HsPrintk("%s: bio idx %d >= vcnt %d\n", "__end_that_request_first", bio->bi_idx,
        bio->bi_vcnt);
    break;
   }
   nbytes = (int )(bio->bi_io_vec + idx)->bv_len;
   while (1) {

    tmp___3 = get_nondet_int();
    if (tmp___3) {
     __bug("ll_rw_blk.c", 3186, (void *)0);
    }
    break;
   }

   tmp___4 = get_nondet_int();
   if (tmp___4) {
    bio_nbytes += nr_bytes;
    total_bytes += nr_bytes;
    break;
   }
   next_idx ++;
   bio_nbytes += nbytes;
  }
  total_bytes += nbytes;
  nr_bytes -= nbytes;
  bio = req->bio;
  if (bio) {

   tmp___5 = get_nondet_int();
   if (tmp___5) {
    break;
   }
  }
 }
 if (! req->bio) {
  return (0);
 }
 if (bio_nbytes) {
  tmp___6 = ordered_bio_endio(req, bio, (unsigned int )bio_nbytes, error);
  if (! tmp___6) {
   bio_endio(bio, (unsigned int )bio_nbytes, error);
  }
  bio->bi_idx = (unsigned short )((int )bio->bi_idx + next_idx);
  (bio->bi_io_vec + bio->bi_idx)->bv_offset += (unsigned int )nr_bytes;
  (bio->bi_io_vec + bio->bi_idx)->bv_len -= (unsigned int )nr_bytes;
 }
 blk_recalc_rq_sectors(req, total_bytes >> 9);
 blk_recalc_rq_segments(req);
 return (1);
}

int end_that_request_first(struct request *req , int uptodate , int nr_sectors )
{
 int tmp ;
 tmp = __end_that_request_first(req, uptodate, nr_sectors << 9);
 return (tmp);
}

void add_disk_randomness(struct gendisk *disk)
{
 return;
}

void elv_dequeue_request(struct request_queue *x, struct request *y)
{
 return;
}

void blkdev_dequeue_request(struct request *req )
{
 elv_dequeue_request(req->q, req);
 return;
}

void laptop_io_completion()
{
 return;
}

void disk_round_stats(struct gendisk *disk )
{
 unsigned long now ;

 now = (unsigned long )jiffies;
 if (now == disk->stamp) {
  return;
 }
 if (disk->in_flight) {
  disk->dkstats.time_in_queue = (unsigned int )((unsigned long )disk->dkstats.time_in_queue + (unsigned long )disk->in_flight * (now - disk->stamp));
  disk->dkstats.io_ticks = (unsigned int )((unsigned long )disk->dkstats.io_ticks + (now - disk->stamp));
 }
 disk->stamp = now;
 return;
}

void elv_completed_request(struct request_queue *x, struct request *y)
{
 return;
}

unsigned long ffz(unsigned long word)
{
 int k ;
 word = ~ word;
 k = 31;
 if (word & 65535UL) {
  k -= 16;
  word <<= 16;
 }
 if (word & 16711680UL) {
  k -= 8;
  word <<= 8;
 }
 if (word & 251658240UL) {
  k -= 4;
  word <<= 4;
 }
 if (word & 805306368UL) {
  k -= 2;
  word <<= 2;
 }
 if (word & 1073741824UL) {
  k --;
 }
 return ((unsigned long )k);
}

unsigned int blk_ordered_cur_seq(struct request_queue *q)
{
 unsigned long tmp ;

 if (! q->ordseq) {
  return (0U);
 }
 tmp = ffz((unsigned long )q->ordseq);
 return ((unsigned int )(1 << tmp));
}

void __blk_put_request(struct request_queue *q , struct request *req );
void end_that_request_last(struct request *req, int uptodate);

void blk_ordered_complete_seq(struct request_queue *q , unsigned int seq , int error )
{
 struct request *rq ;
 int uptodate ;
 long tmp ;
 unsigned int tmp___0 ;

 if (error) {
  if (! q->orderr) {
   q->orderr = error;
  }
 }

 tmp = get_nondet_int();
 if (tmp) {
  __bug("ll_rw_blk.c", 400, (void *)0);
 }
 q->ordseq |= seq;
 tmp___0 = blk_ordered_cur_seq(q);
 if (tmp___0 != 32U) {
  return;
 }
 rq = q->orig_bar_rq;
 if (q->orderr) {
  uptodate = q->orderr;
 } else {
  uptodate = 1;
 }
 q->ordseq = 0U;
 end_that_request_first(rq, uptodate, (int )rq->hard_nr_sectors);
 end_that_request_last(rq, uptodate);
 if (rq->end_io) {

 } else {
  __blk_put_request(rq->q, rq);
 }
 return;
}

void bar_end_io(struct request *rq , int error )
{
 elv_completed_request(rq->q, rq);
 blk_ordered_complete_seq(rq->q, 8U, error);
 return;
}

void post_flush_end_io(struct request *rq , int error )
{
 elv_completed_request(rq->q, rq);
 blk_ordered_complete_seq(rq->q, 16U, error);
 return;
}

void pre_flush_end_io(struct request *rq , int error )
{
 elv_completed_request(rq->q, rq);
 blk_ordered_complete_seq(rq->q, 4U, error);
 return;
}

void elv_put_request(struct request_queue *x, struct request *y)
{
 return;
}

void mempool_free(void *element, mempool_t *pool)
{
 return;
}

void blk_free_request(struct request_queue *q, struct request *rq)
{
 if (rq->flags & 2048UL) {
  elv_put_request(q, rq);
 }
 mempool_free((void *)rq, q->rq.rq_pool);
 return;
}

int queue_congestion_off_threshold(struct request_queue *q)
{
 return ((int )q->nr_congestion_off);
}

void _clear_bit_le(int nr, unsigned long *p)
{
 return;
}

int waitqueue_active(wait_queue_head_t *q)
{
 int tmp ;
 int tmp___0 ;


 tmp = get_nondet_int();
 if (tmp) {
  tmp___0 = 0;
 } else {
  tmp___0 = 1;
 }
 return (tmp___0);
}

void __wake_up(wait_queue_head_t *q, unsigned int mode, int nr, void *key)
{
 return;
}

void clear_queue_congested(struct request_queue *q, int rw)
{
 enum bdi_state bit ;
 wait_queue_head_t *wqh ;
 int tmp ;

 wqh = & congestion_wqh[rw];
 if (rw == 1) {
  bit = 1;
 } else {
  bit = 2;
 }
 _clear_bit_le((int )bit, (unsigned long *)(&q->backing_dev_info.state));

 tmp = waitqueue_active(wqh);
 if (tmp) {
  __wake_up(wqh, 3U, 1, (void *)0);
 }
 return;
}

void ____atomic_clear_bit(unsigned int bit, unsigned long *p)
{
 unsigned long flags ;
 unsigned long mask ;
 unsigned long temp ;

 mask = 1UL << (bit & 31U);
 p += bit >> 5;




 (*p) &= (unsigned long )(~ mask);


 return;
}

void blk_clear_queue_full(struct request_queue *q, int rw)
{
 if (rw == 0) {
  ____atomic_clear_bit(3U, (unsigned long *)(&q->queue_flags));
 } else {
  ____atomic_clear_bit(4U, (unsigned long *)(&q->queue_flags));
 }
 return;
}

void __freed_request(struct request_queue *q, int rw)
{
 struct request_list *rl ;
 int tmp ;
 int tmp___0 ;

 rl = & q->rq;
 tmp = queue_congestion_off_threshold(q);
 if (rl->count[rw] < tmp) {
  clear_queue_congested(q, rw);
 }
 if ((unsigned long )(rl->count[rw] + 1) <= q->nr_requests) {
  tmp___0 = waitqueue_active(& rl->wait[rw]);
  if (tmp___0) {
   __wake_up(& rl->wait[rw], 3U, 1, (void *)0);
  }
  blk_clear_queue_full(q, rw);
 }
 return;
}

void freed_request(struct request_queue *q , int rw , int priv )
{
 struct request_list *rl ;
 long tmp ;

 rl = & q->rq;
 rl->count[rw] --;
 if (priv) {
  rl->elvpriv --;
 }
 __freed_request(q, rw);

 tmp = get_nondet_int();
 if (tmp) {
  __freed_request(q, rw ^ 1);
 }
 return;
}

void __blk_put_request(struct request_queue *q , struct request *req )
{
 struct request_list *rl ;
 long tmp ;
 int tmp___0 ;
 long tmp___1 ;
 int rw ;
 int priv ;
 int tmp___2 ;
 int tmp___3 ;
 int tmp___4 ;
 long tmp___5 ;

 rl = req->rl;

 tmp = get_nondet_int();
 if (tmp) {
  return;
 }
 req->ref_count --;
 if (req->ref_count) {
  tmp___0 = 1;
 } else {
  tmp___0 = 0;
 }

 tmp___1 = get_nondet_int();
 if (tmp___1) {
  return;
 }
 elv_completed_request(q, req);
 req->rq_status = -1;
 req->rl = (struct request_list *)((void *)0);
 if (rl) {
  rw = (int )(req->flags & 1UL);
  priv = (int )(req->flags & 2048UL);
  if (req->next == req) {
   tmp___3 = 0;
  } else {
   tmp___3 = 1;
  }
  if (tmp___3 != 0) {
   tmp___4 = 1;
  } else {
   tmp___4 = 0;
  }

  tmp___5 = get_nondet_int();
  if (tmp___5) {
   __bug("ll_rw_blk.c", 2608, (void *)0);
  }
  blk_free_request(q, req);
  freed_request(q, rw, priv);
 }
 return;
}

void end_that_request_last(struct request *req, int uptodate)
{
 struct gendisk *disk ;
 int error ;
 long tmp ;
 long tmp___0 ;
 unsigned long duration ;
 int rw ;

 disk = req->rq_disk;
 error = 0;

 tmp = get_nondet_int();
 if (tmp) {
  if (! uptodate) {
   error = -5;
  } else {
   error = uptodate;
  }
 }

 tmp___0 = get_nondet_int();
 if (tmp___0) {
  if (req->flags & 64UL) {
   laptop_io_completion();
  }
 }
 if (disk) {
  if (req->flags & 64UL) {
   duration = (unsigned long )(jiffies - (unsigned long )req->start_time);
   rw = (int )(req->flags & 1UL);
   disk->dkstats.ios[rw] ++;
   disk->dkstats.ticks[rw] = (unsigned int )((unsigned long )disk->dkstats.ticks[rw] + duration);
   disk_round_stats(disk);
   disk->in_flight --;
  }
 }
 return;
}

void end_request(struct request *req , int uptodate )
{
 int tmp ;
 int error ;

 tmp = end_that_request_first(req, uptodate, (int )req->hard_cur_sectors);
 if (!tmp) {
  add_disk_randomness(req->rq_disk);
  blkdev_dequeue_request(req);
  end_that_request_last(req, uptodate);
  if (req->end_io) {

   error = get_nondet_int();
   if ((&nondet < 0)) {
    bar_end_io(req, error);
   }
   else if ((&nondet < 0)) {
    post_flush_end_io(req, error);
   }
   else {
    pre_flush_end_io(req, error);
   }
  } else {
   __blk_put_request(req->q, req);
  }
 }
 return;
}

int end_that_request_chunk(struct request *req, int uptodate, int nr_bytes)
{
 int tmp ;

 tmp = __end_that_request_first(req, uptodate, nr_bytes);
 return (tmp);
}

void blk_put_request(struct request *req)
{
 unsigned long flags ;
 struct request_queue *q ;
 unsigned long temp ;

 q = req->q;
 if (q) {




  __blk_put_request(q, req);


 }
 return;
}

int __test_and_clear_bit(int nr, unsigned long *p)
{
 unsigned long oldval ;
 unsigned long mask ;

 mask = 1UL << (nr & 31);
 p += nr >> 5;
 oldval = (unsigned long )(*p);
 (*p) = (unsigned long )(oldval & ~ mask);
 return ((int )(oldval & mask));
}

void blk_queue_end_tag_sl(struct request_queue *q, struct request *prev, struct request *rq)
{
 struct blk_queue_tag *bqt ;
 int tag ;
 long tmp ;
 long tmp___0 ;
 int tmp___1 ;
 int tmp___2 ;
 long tmp___3 ;
 long tmp___4 ;

 bqt = q->queue_tags;
 tag = rq->tag;

 tmp = get_nondet_int();
 if (tmp) {
  __bug("ll_rw_blk.c", 1044, (void *)0);
 }

 tmp___0 = get_nondet_int();
 if (tmp___0) {
  return;
 }
 tmp___1 = __test_and_clear_bit(tag, (unsigned long *)bqt->tag_map);
 if (tmp___1) {
  tmp___2 = 0;
 } else {
  tmp___2 = 1;
 }

 tmp___3 = get_nondet_int();
 if (tmp___3) {
  HsPrintk("<3>%s: attempt to clear non-busy tag (%d)\n", "blk_queue_end_tag", tag);
  return;
 }
 prev->next = rq->next;
 rq->flags &= 4294966271UL;
 rq->tag = -1;

 tmp___4 = get_nondet_int();
 if (tmp___4) {
  HsPrintk("<3>%s: tag %d is missing\n", "blk_queue_end_tag", tag);
 }
 (*(bqt->tag_index + tag)) = (struct request *)((void *)0);
 bqt->busy --;
 return;
}

void __elv_add_request(struct request_queue *x, struct request *y, int z, int v)
{
 return;
}

void blk_queue_invalidate_tags(struct request_queue *q)
{
 struct blk_queue_tag *bqt ;
 struct request *rq, *n, *prev;

 bqt = q->queue_tags;
 prev = bqt;
 rq = bqt->next;
 n = rq->next;
 while (rq != (struct request *)bqt) {
  if (rq->tag == -1) {
   HsPrintk("<3>%s: bad tag found on list\n", "blk_queue_invalidate_tags");
   prev->next = n;
   rq->flags &= 4294966271UL;
  } else {
   blk_queue_end_tag_sl(q, prev, rq);
  }
  rq->flags &= 4294967039UL;
  __elv_add_request(q, rq, 2, 0);


  if (prev->next != rq) {
   HsFreeRequest(rq);
  } else {
   prev = rq;
  }

  rq = n;
  n = rq->next;
 }
 return;
}

int ____atomic_test_and_set_bit(unsigned int bit, unsigned long *p)
{
 unsigned long flags ;
 unsigned int res ;
 unsigned long mask ;
 unsigned long temp ;

 mask = 1UL << (bit & 31U);
 p += bit >> 5;




 res = (unsigned int )(*p);
 (*p) = (unsigned long )((unsigned long )res | mask);


 return ((int )((unsigned long )res & mask));
}

void dump_stack(void)
{
 return;
}

int mod_timer(struct timer_list *timer, unsigned long expires)
{
 return;
}

int __test_bit(int nr , unsigned long *p)
{
 return ((int )(((*(p + (nr >> 5))) >> (nr & 31)) & 1UL));
}

void blk_plug_device(struct request_queue *q )
{
 unsigned long flags ;
 int tmp ;
 int tmp___0 ;
 long tmp___1 ;
 int tmp___2 ;
 int tmp___3 ;



 if ((int )(flags & 128UL)) {
  tmp = 0;
 } else {
  tmp = 1;
 }
 if (tmp != 0) {
  tmp___0 = 1;
 } else {
  tmp___0 = 0;
 }

 tmp___1 = get_nondet_int();
 if (tmp___1) {
  HsPrintk("Badness in %s at %s:%d\n", "blk_plug_device", "ll_rw_blk.c", 1550);
  dump_stack();
 }
 tmp___2 = __test_bit(2, (unsigned long *)(& q->queue_flags));
 if (tmp___2) {
  return;
 }
 tmp___3 = ____atomic_test_and_set_bit(7U, (unsigned long *)(& q->queue_flags));
 if (! tmp___3) {
  mod_timer(&q->unplug_timer, (unsigned long)(jiffies + (unsigned long )q->unplug_delay));
 }
 return;
}

int queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
 int a; return a;
}

int kblockd_schedule_work(struct work_struct *work)
{
 int tmp ;
 tmp = queue_work(kblockd_workqueue, work);
 return (tmp);
}

void blk_start_queue(struct request_queue *q)
{
 int tmp ;

 ____atomic_clear_bit(2U, (unsigned long *)(& q->queue_flags));
 tmp = ____atomic_test_and_set_bit(6U, (unsigned long *)(& q->queue_flags));
 if (tmp) {
  blk_plug_device(q);
  kblockd_schedule_work(& q->unplug_work);
 } else {

  ____atomic_clear_bit(6U, (unsigned long *)(& q->queue_flags));
 }
 return;
}

int ____atomic_test_and_clear_bit(unsigned int bit, unsigned long *p)
{
 unsigned long flags ;
 unsigned int res ;
 unsigned long mask ;
 unsigned long temp ;

 mask = 1UL << (bit & 31U);
 p += bit >> 5;




 res = (unsigned int )(*p);
 (*p) = (unsigned long )((unsigned long )res & ~ mask);


 return ((int )((unsigned long )res & mask));
}

int del_timer(struct timer_list *timer)
{
 int a; return a;
}

int blk_remove_plug(struct request_queue *q )
{
 unsigned long flags ;
 int tmp ;
 int tmp___0 ;
 long tmp___1 ;
 int tmp___2 ;



 if ((int )(flags & 128UL)) {
  tmp = 0;
 } else {
  tmp = 1;
 }
 if (tmp != 0) {
  tmp___0 = 1;
 } else {
  tmp___0 = 0;
 }

 tmp___1 = get_nondet_int();
 if (tmp___1) {
  HsPrintk("Badness in %s at %s:%d\n", "blk_remove_plug", "ll_rw_blk.c", 1571);
  dump_stack();
 }
 tmp___2 = ____atomic_test_and_clear_bit(7U, (unsigned long *)(& q->queue_flags));
 if (! tmp___2) {
  return (0);
 }
 del_timer(& q->unplug_timer);
 return (1);
}

void ____atomic_set_bit(unsigned int bit, unsigned long *p)
{
 unsigned long flags ;
 unsigned long mask ;
 unsigned long temp ;

 mask = 1UL << (bit & 31U);
 p += bit >> 5;




 (*p) |= (unsigned long )mask;


 return;
}

void blk_stop_queue(struct request_queue *q)
{
 blk_remove_plug(q);
 ____atomic_set_bit(2U, (unsigned long *)(& q->queue_flags));
 return;
}

void flush_workqueue(struct workqueue_struct *wq)
{
 return;
}

void kblockd_flush(void)
{
 flush_workqueue(kblockd_workqueue);
 return;
}

void blk_sync_queue(struct request_queue *q )
{
 del_timer(& q->unplug_timer);
 kblockd_flush();
 return;
}

void __memzero(void *ptr, __kernel_size_t n)
{
 return;
}

int blk_rq_map_sg(struct request_queue *q, struct request *rq, struct scatterlist *sg)
{
 struct bio_vec *bvec;
 struct bio_vec *bvprv;
 struct bio *bio;
 int nsegs;
 int i;
 int cluster;
 int nbytes;
 void *__p;
 size_t __n;

 nsegs = 0;
 cluster = (int )(q->queue_flags & 1UL);
 bvprv = (struct bio_vec *)((void *)0);
 if (rq->bio) {
  bio = rq->bio;
  while (bio) {
   bvec = bio->bi_io_vec + bio->bi_idx;
   i = (int )bio->bi_idx;
   while (i < (int )bio->bi_vcnt) {
    nbytes = (int )bvec->bv_len;
    if (bvprv) {
     if (cluster) {
      if ((sg + (nsegs - 1))->length + (unsigned int )nbytes > q->max_segment_size) {
       goto new_segment;
      }
      if (! ((((unsigned long )(bvprv->bv_page - mem_map) << 12) + (unsigned long )bvprv->bv_offset) + (unsigned long )bvprv->bv_len == ((unsigned long )(bvec->bv_page - mem_map) << 12) + (unsigned long )bvec->bv_offset)) {
       goto new_segment;
      }
      if (! (((((unsigned long )(bvprv->bv_page - mem_map) << 12) + (unsigned long )bvprv->bv_offset) | q->seg_boundary_mask) == ((((((unsigned long )(bvec->bv_page - mem_map) << 12) + (unsigned long )bvec->bv_offset) + (unsigned long )bvec->bv_len) - 1UL) | q->seg_boundary_mask))) {
       goto new_segment;
      }
      (sg + (nsegs - 1))->length += (unsigned int )nbytes;
     } else {
      goto new_segment;
     }
    } else {
new_segment:
     __p = (void *)(sg + nsegs);
     __n = sizeof(struct scatterlist);
     if (__n != 0U) {
      __memzero(__p, __n);
     }
     (sg + nsegs)->page = bvec->bv_page;
     (sg + nsegs)->length = (unsigned int )nbytes;
     (sg + nsegs)->offset = bvec->bv_offset;
     nsegs ++;
    }
    bvprv = bvec;
    bvec ++;
    i ++;
   }
   bio = bio->bi_next;
  }
 }
 return (nsegs);
}

int elv_queue_empty(struct request_queue *q)
{
 int a; return a;
}

void blk_run_queue(struct request_queue *q)
{
 unsigned long flags ;
 unsigned long temp ;
 int tmp ;





 blk_remove_plug(q);
 tmp = elv_queue_empty(q);
 if (! tmp) {

 }


 return;
}

int elv_may_queue(struct request_queue *x, int y, struct bio *z)
{
 int a; return a;
}

int queue_congestion_on_threshold(struct request_queue *q)
{
 return ((int )q->nr_congestion_on);
}

struct thread_info *current_thread_info(void)
{
 return (HsThreadInfo);
}

struct task_struct *get_current(void)
{
 struct thread_info *tmp ;

 tmp = current_thread_info();
 return (tmp->task);
}

struct io_context *kmem_cache_alloc_ioc(kmem_cache_t *x, gfp_t y)
{
 struct io_context *ioc;
 ioc = (struct io_context *)mallocnull(sizeof(struct io_context));
 return ioc;
}

struct io_context *current_io_context(gfp_t gfp_flags )
{
 struct task_struct *tsk ;
 struct task_struct *tmp ;
 struct io_context *ret ;
 long tmp___0 ;

 tmp = get_current();
 tsk = tmp;
 ret = tsk->io_context;

 if (ret) {
  return (ret);
 }
 ret = (struct io_context *)kmem_cache_alloc_ioc(iocontext_cachep, gfp_flags);
 if (ret) {
  ret->refcount.counter = (int )1;
  ret->task = get_current();
  ret->set_ioprio = (int (*)(struct io_context * , unsigned int ))((void *)0);
  ret->last_waited = (unsigned long )jiffies;
  ret->nr_batch_requests = 0;
  ret->aic = (struct as_io_context *)((void *)0);
  ret->cic = (struct cfq_io_context *)((void *)0);
  tsk->io_context = ret;
 }
 return (ret);
}

int ioc_batching(struct request_queue *q, struct io_context *ioc)
{
 int tmp ;

 if (! ioc) {
  return (0);
 }
 if ((unsigned int )ioc->nr_batch_requests == q->nr_batching) {
  tmp = 1;
 } else {
  if (ioc->nr_batch_requests > 0) {
   if ((long )jiffies - (long )(ioc->last_waited + 2UL) < 0L) {
    tmp = 1;
   } else {
    tmp = 0;
   }
  } else {
   tmp = 0;
  }
 }
 return (tmp);
}

int blk_queue_full(struct request_queue *q, int rw)
{
 if (rw == 0) return __test_bit(3,&q->queue_flags);

 return __test_bit(4,&q->queue_flags);
}

void ioc_set_batching(struct request_queue *q, struct io_context *ioc)
{
 if (!ioc || ioc_batching(q, ioc)) return;

 ioc->nr_batch_requests = q->nr_batching;
 ioc->last_waited = jiffies;
}

void _set_bit_le(int nr, unsigned long *p)
{
 return;
}

void blk_set_queue_full(struct request_queue *q, int rw)
{
 if (rw == 0) {
  if ((&nondet < 0)) {
   ____atomic_set_bit(3, &q->queue_flags);
  } else {
    _set_bit_le(3,&q->queue_flags);
  }
 } else {
  if ((&nondet < 0)) {
     ____atomic_set_bit(4, &q->queue_flags);
  } else {
   _set_bit_le(4,&q->queue_flags);
  }
 }
}

void set_queue_congested(struct request_queue *q, int rw)
{
 enum bdi_state bit ;

 if (rw == 1) {
  bit = 1;
 } else {
  bit = 2;
 }
 _set_bit_le((int )bit, (unsigned long *)(&q->backing_dev_info.state));
 return;
}

struct request *mempool_alloc_request(mempool_t *pool, gfp_t gfp_mask)
{
 struct request *rq;
 if ((&nondet < 0)) {
  rq = HsCreateRequest();
 } else {
  rq = 0;
 }
 return rq;
}

void mempool_free_request(struct request *element, mempool_t *pool)
{
 HsFreeRequest(element);
 return;
}

struct request *blk_alloc_request(struct request_queue *q, int rw, struct bio *bio, int priv, gfp_t gfp_mask)
{
 int tmp;
 struct request *rq = mempool_alloc_request(q->rq.rq_pool, gfp_mask);

 if (!rq) return ((void *)0);

 rq->flags = rw;

 if (priv) {
    if ((&nondet < 0)) {
   mempool_free_request(rq, q->rq.rq_pool);
   return ((void *)0);
    }
    rq->flags |= 2048UL;
  }

  return rq;
}

void rq_init(struct request_queue *q, struct request *rq)
{
  rq->next = rq;


 rq->errors = 0;
 rq->rq_status = 1;

 rq->ioprio = 0;
 rq->buffer = ((void *)0);
 rq->ref_count = 1;
 rq->q = q;
 rq->waiting = ((void *)0);
 rq->special = ((void *)0);
 rq->data_len = 0;
 rq->data = ((void *)0);
 rq->nr_phys_segments = 0;
 rq->sense = ((void *)0);
 rq->end_io = ((void *)0);
 rq->end_io_data = ((void *)0);
 rq->completion_data = ((void *)0);
}

struct request *get_request(struct request_queue *q, int rw, struct bio *bio, gfp_t gfp_mask)
{
 struct request *rq ;
 struct request_list *rl ;
 struct io_context *ioc ;
 int may_queue ;
 int priv ;
 int tmp ;
 int tmp___0 ;
 int tmp___1 ;
 int tmp___2 ;
 int tmp___3 ;
 unsigned long temp ;
 unsigned long temp___0 ;
 long tmp___4 ;
 long tmp___5 ;
 int tmp___6 ;

 rq = (struct request *)((void *)0);
 rl = & q->rq;
 ioc = (struct io_context *)((void *)0);
 may_queue = elv_may_queue(q, rw, bio);
 if (may_queue == 1) {
  goto rq_starved;
 }
 tmp___1 = queue_congestion_on_threshold(q);
 if (rl->count[rw] + 1 >= tmp___1) {
  if ((unsigned long)(rl->count[rw] + 1) >= q->nr_requests) {
   ioc = current_io_context(32U);
   tmp___0 = blk_queue_full(q, rw);
   if (tmp___0) {
    if (may_queue != 2) {
     tmp = ioc_batching(q, ioc);
     if (! tmp) {
      goto out;
     }
    }
   } else {
    ioc_set_batching(q, ioc);
    blk_set_queue_full(q, rw);
   }
  }
  set_queue_congested(q, rw);
 }
 if ((unsigned long )rl->count[rw] >= (3UL * q->nr_requests) / 2UL) {
  goto out;
 }
 rl->count[rw] ++;
 rl->starved[rw] = 0;
 tmp___2 = __test_bit(8, (unsigned long *)(& q->queue_flags));
 if (tmp___2) {
  tmp___3 = 0;
 } else {
  tmp___3 = 1;
 }
 priv = tmp___3;
 if (priv) {
  rl->elvpriv ++;
 }



 rq = blk_alloc_request(q, rw, bio, priv, gfp_mask);
 if (!rq) {



  freed_request(q, rw, priv);
rq_starved:

  tmp___4 = get_nondet_int();
  if (tmp___4) {
   rl->starved[rw] = 1;
  }
  goto out;
 }
 tmp___6 = ioc_batching(q, ioc);
 if (tmp___6) {
  ioc->nr_batch_requests --;
 }
 rq_init(q, rq);
 rq->rl = rl;
out:
 return (rq);
}

void prepare_to_wait_exclusive(wait_queue_head_t *q, wait_queue_t *wait, int state)
{
 return;
}

void __generic_unplug_device(struct request_queue *q)
{
 int tmp ;
        int tmp___0 ;
        long tmp___1 ;
        int tmp___2 ;

 tmp = __test_bit(2, (unsigned long *)(& q->queue_flags));
 if (tmp) {
  tmp___0 = 1;
 } else {
  tmp___0 = 0;
 }

 tmp___1 = get_nondet_int();
 if (tmp___1) {
  return;
 }
 tmp___2 = blk_remove_plug(q);
 if (! tmp___2) {
  return;
 }

 return;
}

void generic_unplug_device(struct request_queue *q)
{
 unsigned long temp ;
 unsigned long temp___0 ;




 __generic_unplug_device(q);



 return;
}

int ll_back_merge_fn(struct request_queue *q , struct request *req , struct bio *bio )
{
 int a; return a;
}


void io_schedule(void)
{
 return;
}

void finish_wait(wait_queue_head_t *q, wait_queue_t *wait)
{
 return;
}

int autoremove_wake_function(wait_queue_t *wait, unsigned int mode, int sync, void *key)
{
 int a; return a;
}

struct request *get_request_wait(struct request_queue *q , int rw , struct bio *bio)
{
 struct request *rq ;
 wait_queue_t wait ;
 void *tmp ;
 struct request_list *rl ;
 struct io_context *ioc ;
 unsigned long temp ;
 unsigned long temp___0 ;

 rq = get_request(q, rw, bio, 16U);
 while (! rq) {
  tmp = (void *)get_current();
  wait.flags = 0U;
  wait.private = tmp;
  wait.func = & autoremove_wake_function;
  wait.task_list.next = &wait.task_list;
  wait.task_list.prev = &wait.task_list;
  rl = & q->rq;
  prepare_to_wait_exclusive(& rl->wait[rw], & wait, 2);
  rq = get_request(q, rw, bio, 16U);
  if (! rq) {
   __generic_unplug_device(q);



   io_schedule();
   ioc = current_io_context(16U);
   ioc_set_batching(q, ioc);



  }
  finish_wait(& rl->wait[rw], & wait);
 }
 return (rq);
}

struct request *blk_get_request(struct request_queue *q , int rw , gfp_t gfp_mask )
{
 struct request *rq ;
 int tmp ;
 int tmp___0 ;
 long tmp___1 ;
 unsigned long temp ;
 unsigned long temp___0 ;

 if (rw != 0) {
  if (rw != 1) {
   tmp = 1;
  } else {
   tmp = 0;
  }
 } else {
  tmp = 0;
 }
 if (tmp != 0) {
  tmp___0 = 1;
 } else {
  tmp___0 = 0;
 }

 tmp___1 = get_nondet_int();
 if (tmp___1) {
  __bug("ll_rw_blk.c", 2165, (void *)0);
 }



 if (gfp_mask & 16U) {
  rq = get_request_wait(q, rw, (struct bio *)((void *)0));
 } else {
  rq = get_request(q, rw, (struct bio *)((void *)0), gfp_mask);
  if (! rq) {



  }
 }
 return (rq);
}

struct workqueue_struct *__create_workqueue(char *name, int singlethread)
{
 struct workqueue_struct *a; return a;
}

kmem_cache_t *kmem_cache_create(char *x, size_t y, size_t z, unsigned long u,
   void (*f)(void *, kmem_cache_t *, unsigned long),
   void (*g)(void *, kmem_cache_t *, unsigned long))
{
 kmem_cache_t *a;
 a = (kmem_cache_t *)malloc(sizeof(kmem_cache_t));
 return a;
}

void open_softirq(int nr, void (*action)(struct softirq_action *), void *data)
{
 return;
}

void blk_done_softirq(struct softirq_action *h )
{
 struct request *tmp;
 struct request_head local_list;

 local_list.next = &local_list;
 while (per_cpu__blk_cpu_done.next != (struct request *)&per_cpu__blk_cpu_done) {
  tmp = per_cpu__blk_cpu_done.next;
  per_cpu__blk_cpu_done.next = tmp->next;

  tmp->next = local_list.next;
  local_list.next = tmp;
 }

 while (local_list.next != (struct request *)&local_list) {
  tmp = local_list.next;
  local_list.next = tmp->next;

  HsFreeRequest(tmp);
 }
 return;
}

void panic(char *x)
{
 return;
}

int blk_dev_init(void)
{
 int i ;
 kblockd_workqueue = __create_workqueue("kblockd", 0);
 if (!kblockd_workqueue) {
  panic("Failed to create kblockd\n");
 }
 request_cachep = kmem_cache_create("blkdev_requests", sizeof(struct request),
  0U, 262144UL,
  (void (*)(void *, kmem_cache_t *, unsigned long))((void *)0),
     (void (*)(void *, kmem_cache_t *, unsigned long))((void *)0));
 requestq_cachep = kmem_cache_create("blkdev_queue", sizeof(struct request_queue),
  0U, 262144UL,
  (void (*)(void *, kmem_cache_t *, unsigned long))((void *)0),
  (void (*)(void *, kmem_cache_t *, unsigned long))((void *)0));
 iocontext_cachep = kmem_cache_create("blkdev_ioc", sizeof(struct io_context),
  0U, 262144UL,
  (void (*)(void *, kmem_cache_t *, unsigned long))((void *)0),
  (void (*)(void *, kmem_cache_t *, unsigned long))((void *)0));

 per_cpu__blk_cpu_done.next = &per_cpu__blk_cpu_done;
 open_softirq(4, & blk_done_softirq, (void *)0);
 blk_max_low_pfn = max_low_pfn;
 blk_max_pfn = max_pfn;
 return (0);
}

ssize_t queue_var_show(unsigned int var, char *page)
{
 ssize_t tmp ;


 tmp = get_nondet_int();
 return (tmp);
}

ssize_t queue_max_hw_sectors_show(struct request_queue *q, char *page)
{
 int max_hw_sectors_kb;
 ssize_t tmp;

 max_hw_sectors_kb = (int )(q->max_hw_sectors >> 1);
 tmp = queue_var_show((unsigned int)max_hw_sectors_kb, page);
 return (tmp);
}

unsigned long simple_strtoul(char *x, char **y, unsigned int z)
{
 unsigned long a; return a;
}

ssize_t queue_var_store(unsigned long *var, char *page, size_t count)
{
 char *p ;

 p = (char *)page;
 (*var) = simple_strtoul((char *)p, & p, 10U);
 return ((int )count);
}

ssize_t queue_max_sectors_store(struct request_queue *q, char *page, size_t count)
{
 unsigned long max_sectors_kb ;
 unsigned long max_hw_sectors_kb ;
 unsigned long page_kb ;
 ssize_t ret ;
 ssize_t tmp ;
 int ra_kb ;
 unsigned long temp ;
 unsigned long temp___0 ;

 max_hw_sectors_kb = (unsigned long)(q->max_hw_sectors >> 1);
 page_kb = 4UL;
 tmp = queue_var_store(&max_sectors_kb, page, count);
 ret = tmp;
 if (max_sectors_kb > max_hw_sectors_kb) {
  return (-22);
 } else {
  if (max_sectors_kb < page_kb) {
   return (-22);
  }
 }



 ra_kb = (int )(q->backing_dev_info.ra_pages << 2);
 if ((unsigned long )ra_kb > max_sectors_kb) {
  q->backing_dev_info.ra_pages = max_sectors_kb >> 2;
 }
 q->max_sectors = (unsigned int )(max_sectors_kb << 1);



 return (ret);
}

ssize_t queue_max_sectors_show(struct request_queue *q, char *page)
{
 int max_sectors_kb ;
 ssize_t tmp ;

 max_sectors_kb = (int )(q->max_sectors >> 1);
 tmp = queue_var_show((unsigned int )max_sectors_kb, page);
 return (tmp);
}

ssize_t queue_ra_store(struct request_queue *q, char *page, size_t count)
{
 unsigned long ra_kb ;
 ssize_t ret ;
 ssize_t tmp ;
 unsigned long temp ;
 unsigned long temp___0 ;

 tmp = queue_var_store(& ra_kb, page, count);
 ret = tmp;



 if (ra_kb > (unsigned long )(q->max_sectors >> 1)) {
  ra_kb = (unsigned long )(q->max_sectors >> 1);
 }
 q->backing_dev_info.ra_pages = ra_kb >> 2;



 return (ret);
}

ssize_t queue_ra_show(struct request_queue *q, char *page)
{
 int ra_kb ;
 ssize_t tmp ;

 ra_kb = (int )(q->backing_dev_info.ra_pages << 2);
 tmp = queue_var_show((unsigned int )ra_kb, page);
 return (tmp);
}

void blk_queue_congestion_threshold(struct request_queue *q)
{
 int nr ;

 nr = (int)((q->nr_requests - q->nr_requests / 8UL) + 1UL);
 if ((unsigned long)nr > q->nr_requests) {
  nr = (int)q->nr_requests;
 }
 q->nr_congestion_on = (unsigned int)nr;
 nr = (int )(((q->nr_requests - q->nr_requests / 8UL) - q->nr_requests / 16UL) - 1UL);
 if (nr < 1) {
  nr = 1;
 }
 q->nr_congestion_off = (unsigned int)nr;
 return;
}

ssize_t queue_requests_store(struct request_queue *q, char *page, size_t count)
{
 struct request_list *rl ;
 int ret ;
 int tmp ;
 int tmp___0 ;
 int tmp___1 ;
 int tmp___2 ;
 int tmp___3 ;

 rl = & q->rq;
 tmp = queue_var_store(& q->nr_requests, page, count);
 ret = tmp;
 if (q->nr_requests < 4UL) {
  q->nr_requests = 4UL;
 }
 blk_queue_congestion_threshold(q);
 tmp___1 = queue_congestion_on_threshold(q);
 if (rl->count[0] >= tmp___1) {
  set_queue_congested(q, 0);
 } else {
  tmp___0 = queue_congestion_off_threshold(q);
  if (rl->count[0] < tmp___0) {
   clear_queue_congested(q, 0);
  }
 }
 tmp___3 = queue_congestion_on_threshold(q);
 if (rl->count[1] >= tmp___3) {
  set_queue_congested(q, 1);
 } else {
  tmp___2 = queue_congestion_off_threshold(q);
  if (rl->count[1] < tmp___2) {
   clear_queue_congested(q, 1);
  }
 }
 if ((unsigned long )rl->count[0] >= q->nr_requests) {
  blk_set_queue_full(q, 0);
 } else {
  if ((unsigned long )(rl->count[0] + 1) <= q->nr_requests) {
   blk_clear_queue_full(q, 0);
   __wake_up(& rl->wait[0], 3U, 1, (void *)0);
  }
 }
 if ((unsigned long )rl->count[1] >= q->nr_requests) {
  blk_set_queue_full(q, 1);
 } else {
  if ((unsigned long )(rl->count[1] + 1) <= q->nr_requests) {
   blk_clear_queue_full(q, 1);
   __wake_up(& rl->wait[1], 3U, 1, (void *)0);
  }
 }
 return (ret);
}

ssize_t queue_requests_show(struct request_queue *q, char *page)
{
 ssize_t tmp ;
 tmp = queue_var_show((unsigned int )q->nr_requests, page);
 return (tmp);
}

int queue_dma_alignment(struct request_queue *q)
{
 int retval ;
 retval = 511;
 if (q) {
  if (q->dma_alignment) {
   retval = (int )q->dma_alignment;
  }
 }
 return (retval);
}

struct bio *bio_copy_user(struct request_queue *x, unsigned long y, unsigned int z, int w)
{
 struct bio *a;
 if ((&nondet < 0)) {
  a = HsCreateBio();
 } else {
  a = 0;
 }
 return a;
}

struct bio *bio_map_user(struct request_queue *x, struct block_device *y, unsigned long z, unsigned int u, int v)
{
 struct bio *a;
 if ((&nondet < 0)) {
  a = HsCreateBio();
 } else {
  a = 0;
 }
 return a;
}

void blk_rq_bio_prep(struct request_queue *q, struct request *rq, struct bio *bio)
{
 void *tmp ;

 rq->flags |= bio->bi_rw & 7UL;
 rq->nr_phys_segments = (unsigned short )bio_phys_segments(q, bio);
 rq->nr_hw_segments = (unsigned short )bio_hw_segments(q, bio);
 rq->current_nr_sectors = (bio->bi_io_vec + bio->bi_idx)->bv_len >> 9;
 rq->hard_cur_sectors = rq->current_nr_sectors;
 rq->nr_sectors = (unsigned long )(bio->bi_size >> 9);
 rq->hard_nr_sectors = rq->nr_sectors;
 tmp = lowmem_page_address((bio->bi_io_vec + bio->bi_idx)->bv_page);
 rq->buffer = (char *)(tmp + (bio->bi_io_vec + bio->bi_idx)->bv_offset);


 return;
}

int blk_rq_map_user(struct request_queue *q, struct request *rq, void *ubuf, unsigned int len)
{
 unsigned long uaddr ;
 struct bio *bio ;
 int reading ;
 int tmp ;
 int tmp___0 ;
 long tmp___1 ;
 int tmp___2 ;

 if (len > q->max_hw_sectors << 9) {
  return (-22);
 }
 if (! len) {
  return (-22);
 } else {
  if (! ubuf) {
   return (-22);
  }
 }
 reading = (rq->flags & 1UL) == 0UL;
 uaddr = (unsigned long )ubuf;
 tmp = queue_dma_alignment(q);
 if (uaddr & (unsigned long )tmp) {
  bio = bio_copy_user(q, uaddr, len, reading);
 } else {
  tmp___0 = queue_dma_alignment(q);
  if (len & (unsigned int )tmp___0) {
   bio = bio_copy_user(q, uaddr, len, reading);
  } else {
   bio = bio_map_user(q, (struct block_device *)((void *)0), uaddr, len, reading);
  }
 }
 if (bio) {


  bio->bi_next = rq->bio;
  rq->bio = bio;
  blk_rq_bio_prep(q, rq, bio);
  rq->data = (void *)0;
  rq->buffer = (char *)rq->data;
  rq->data_len = len;
  return (0);
 }
 return (-1);
}

void raise_softirq_irqoff(unsigned int nr)
{
 return;
}

void blk_complete_request(struct request *req)
{
 struct list_head *cpu_list ;
 unsigned long flags ;
 long tmp ;
 unsigned long temp ;


 tmp = get_nondet_int();
 if (tmp) {
  __bug("ll_rw_blk.c", 3349, (void *)0);
 }




 req->next = per_cpu__blk_cpu_done.next;
 per_cpu__blk_cpu_done.next = req;
 raise_softirq_irqoff(4U);


 return;
}

struct request_queue *bdev_get_queue(struct block_device *bdev)
{
 return ((bdev->bd_disk)->queue);
}

struct backing_dev_info *blk_get_backing_dev_info(struct block_device *bdev)
{
 struct backing_dev_info *ret;
 struct request_queue *q;
 struct request_queue *tmp;

 ret = (struct backing_dev_info *)((void *)0);
 tmp = bdev_get_queue(bdev);
 q = tmp;
 if (q) {
  ret = & q->backing_dev_info;
 }
 return (ret);
}

void init_timer(struct timer_list *timer)
{
 return;
}

struct request_queue *blk_alloc_queue_node(gfp_t gfp_mask, int node_id)
{
 struct request_queue *q ;
 void *__p ;
 size_t __n ;


 q = (struct request_queue *)mallocnull(sizeof(struct request_queue));
 if (!q) {
  return ((struct request_queue *)((void *)0));
 }
 __p = (void *)q;
 __n = sizeof((*q));
 if (__n != 0U) {
  __memzero(__p, __n);
 }
 q->queue_tags = 0;
 init_timer(& q->unplug_timer);
 q->refcnt.counter = (int )1;
 q->backing_dev_info.unplug_io_fn = &blk_backing_dev_unplug;
 q->backing_dev_info.unplug_io_data = (void *)q;
 return (q);
}

mempool_t *mempool_create_node(int min_nr, mempool_alloc_t *alloc_fn,
   mempool_free_t *free_fn,
   void *pool_data , int nid )
{
 mempool_t *a; return a;
}

int blk_init_free_list(struct request_queue *q)
{
 struct request_list *rl ;

 rl = & q->rq;
 rl->count[1] = 0;
 rl->count[0] = rl->count[1];
 rl->starved[1] = 0;
 rl->starved[0] = rl->starved[1];
 rl->elvpriv = 0;
 (rl->wait[0]).task_list.next = &(rl->wait[0]).task_list;
 (rl->wait[0]).task_list.prev = &(rl->wait[0]).task_list;
 (rl->wait[1]).task_list.next = &(rl->wait[1]).task_list;
 (rl->wait[1]).task_list.prev = &(rl->wait[1]).task_list;
 rl->rq_pool = mempool_create_node(4, & mempool_alloc_slab, & mempool_free_slab,
       (void *)request_cachep, q->node);
 if (! rl->rq_pool) {
  return (-12);
 }
 return (0);
}

void blk_queue_segment_boundary(struct request_queue *q, unsigned long mask)
{
 if (mask < 4095UL) {
  mask = 4095UL;
  HsPrintk("%s: set to minimum %lx\n", "blk_queue_segment_boundary", mask);
 }
 q->seg_boundary_mask = mask;
 return;
}

void blk_queue_max_phys_segments(struct request_queue *q , unsigned short max_segments )
{
 if (! max_segments) {
  max_segments = (unsigned short)1;
  HsPrintk("%s: set to minimum %d\n", "blk_queue_max_phys_segments", max_segments);
 }
 q->max_phys_segments = max_segments;
 return;
}

void blk_queue_max_hw_segments(struct request_queue *q , unsigned short max_segments )
{
 if (! max_segments) {
  max_segments = (unsigned short)1;
  HsPrintk("%s: set to minimum %d\n", "blk_queue_max_hw_segments", max_segments);
 }
 q->max_hw_segments = max_segments;
 return;
}

void blk_queue_max_sectors(struct request_queue *q , unsigned int max_sectors )
{
 if ((unsigned long )(max_sectors << 9) < 4096UL) {
  max_sectors = 8U;
  HsPrintk("%s: set to minimum %d\n", "blk_queue_max_sectors", max_sectors);
 }
 if (1024U > max_sectors) {
  q->max_sectors = max_sectors;
  q->max_hw_sectors = q->max_sectors;
 } else {
  q->max_sectors = 1024U;
  q->max_hw_sectors = max_sectors;
 }
 return;
}

void blk_queue_hardsect_size(struct request_queue *q , unsigned short size )
{
 q->hardsect_size = size;
 return;
}

void blk_queue_dma_alignment(struct request_queue *q , int mask )
{
 q->dma_alignment = (unsigned int )mask;
 return;
}

int init_emergency_isa_pool(void)
{
 int a; return a;
}

void blk_queue_bounce_limit(struct request_queue *q, u64 dma_addr)
{
 unsigned long bounce_pfn ;
 int dma ;

 bounce_pfn = (unsigned long )(dma_addr >> 12);
 dma = 0;
 q->bounce_gfp = 16U;
 if (bounce_pfn < blk_max_low_pfn) {
  dma = 1;
 }
 q->bounce_pfn = bounce_pfn;
 if (dma) {
  init_emergency_isa_pool();
  q->bounce_gfp = 17U;
  q->bounce_pfn = bounce_pfn;
 }
 return;
}

void blk_queue_activity_fn(struct request_queue *q, activity_fn *fn, void *data)
{
 q->activity_fn = fn;
 q->activity_data = data;
 return;
}

void blk_queue_make_request(struct request_queue *q, make_request_fn *mfn)
{
 q->nr_requests = 128UL;
 blk_queue_max_phys_segments(q, (unsigned short)128);
 blk_queue_max_hw_segments(q, (unsigned short)128);
 q->make_request_fn = mfn;
 q->backing_dev_info.ra_pages = 32UL;
 q->backing_dev_info.state = 0UL;
 q->backing_dev_info.capabilities = 4U;
 blk_queue_max_sectors(q, 255U);
 blk_queue_hardsect_size(q, (unsigned short)512);
 blk_queue_dma_alignment(q, 511);
 blk_queue_congestion_threshold(q);
 q->nr_batching = 32U;
 q->unplug_thresh = 4;
 q->unplug_delay = 0UL;
 if (q->unplug_delay == 0UL) {
  q->unplug_delay = 1UL;
 }
 q->unplug_work.entry.next = &q->unplug_work.entry;
 q->unplug_work.entry.prev = &q->unplug_work.entry;
 q->unplug_work.pending = 0UL;
 q->unplug_work.func = & blk_unplug_work;
 q->unplug_work.data = (void *)q;
 init_timer(& q->unplug_work.timer);
 q->unplug_timer.function = & blk_unplug_timeout;
 q->unplug_timer.data = (unsigned long )q;
 blk_queue_bounce_limit(q, (unsigned long long )blk_max_low_pfn << 12);
 blk_queue_activity_fn(q, (activity_fn *)((void *)0), (void *)0);
 return;
}

void blk_queue_max_segment_size(struct request_queue *q , unsigned int max_size )
{
 if ((unsigned long )max_size < 4096UL) {
  max_size = 4096U;
  HsPrintk("%s: set to minimum %d\n", "blk_queue_max_segment_size", max_size);
 }
 q->max_segment_size = max_size;
 return;
}

int atomic_sub_return(int i, atomic_t *v)
{
 unsigned long flags ;
 int val ;
 unsigned long temp ;





 val = (int )v->counter;
 val -= i;
 v->counter = (int )val;


 return (val);
}

void elevator_exit(elevator_t *x)
{
 return;
}

void mempool_destroy(mempool_t *pool)
{
 return;
}

void __blk_queue_free_tags(struct request_queue *q )
{
 struct blk_queue_tag *bqt;
 long tmp ;
 int tmp___0 ;
 int tmp___1 ;
 int tmp___2 ;
 long tmp___3 ;
 int tmp___4 ;

 bqt = q->queue_tags;
 if (! bqt) {
  return;
 }
 tmp___4 = atomic_sub_return(1, & bqt->refcnt);
 if (tmp___4 == 0) {

  tmp = get_nondet_int();
  if (tmp) {
   __bug("ll_rw_blk.c", 864, (void *)0);
  }
  if (bqt->next == (struct request *)bqt) {
   tmp___1 = 0;
  } else {
   tmp___1 = 1;
  }
  if (tmp___1 != 0) {
   tmp___2 = 1;
  } else {
   tmp___2 = 0;
  }

  tmp___3 = get_nondet_int();
  if (tmp___3) {
   __bug("ll_rw_blk.c", 865, (void *)0);
  }
  free((void *)bqt->tag_index);
  bqt->tag_index = (struct request **)((void *)0);
  free((void *)bqt->tag_map);
  bqt->tag_map = (unsigned long *)((void *)0);
  free((void *)bqt);
 }
 q->queue_tags = (struct blk_queue_tag *)((void *)0);
 q->queue_flags &= 4294967293UL;
 return;
}

void kmem_cache_free(kmem_cache_t *x, void * y)
{
 free(y);
}

void blk_cleanup_queue(struct request_queue *q )
{
 struct request_list *rl ;
 int tmp ;

 rl = & q->rq;
 tmp = atomic_sub_return(1, & q->refcnt);
 if (! (tmp == 0)) {

  // BUG:
  // Memory Leak
  free(q);
  return;
 }
 if (q->elevator) {
  elevator_exit(q->elevator);
 }
 blk_sync_queue(q);
 if (rl->rq_pool) {
  mempool_destroy(rl->rq_pool);
 }
 if (q->queue_tags) {
  __blk_queue_free_tags(q);
 }
 kmem_cache_free(requestq_cachep, (void *)q);
 return;
}

int elevator_init(struct request_queue *x, char *y)
{
 int a; return a;
}

struct request_queue *blk_init_queue_node(request_fn_proc *rfn, spinlock_t *lock, int node_id)
{
 struct request_queue *q ;
 int tmp___0 ;
 int tmp___1 ;

 q = blk_alloc_queue_node(208U, node_id);
 if (!q) {
  return ((struct request_queue *)((void *)0));
 }
 q->node = node_id;
 tmp___0 = blk_init_free_list(q);
 if (tmp___0) {
  goto out_init;
 }
 if (! lock) {
  lock = & q->__queue_lock;
 }
 q->request_fn = rfn;
 q->back_merge_fn = & ll_back_merge_fn;
 q->front_merge_fn = & ll_front_merge_fn;
 q->merge_requests_fn = & ll_merge_requests_fn;
 q->prep_rq_fn = (prep_rq_fn *)((void *)0);
 q->unplug_fn = & generic_unplug_device;
 q->queue_flags = 1UL;
 q->queue_lock = lock;
 blk_queue_segment_boundary(q, 4294967295UL);
 blk_queue_make_request(q, & __make_request);
 blk_queue_max_segment_size(q, 65536U);
 blk_queue_max_hw_segments(q, (unsigned short)128);
 blk_queue_max_phys_segments(q, (unsigned short)128);
 tmp___1 = elevator_init(q, (char *)((void *)0));
 if (! tmp___1) {
  blk_queue_congestion_threshold(q);
  return (q);
 }
 blk_cleanup_queue(q);

 return ((struct request_queue *)((void *)0));
out_init:
 kmem_cache_free(requestq_cachep, (void *)q);
 return ((struct request_queue *)((void *)0));
}

struct request_queue *blk_init_queue(request_fn_proc *rfn, spinlock_t *lock)
{
 struct request_queue *tmp ;

 tmp = blk_init_queue_node(rfn, lock, -1);
 return (tmp);
}

int atomic_add_return(int i, atomic_t *v)
{
 unsigned long flags ;
 int val ;
 unsigned long temp ;





 val = (int )v->counter;
 val += i;
 v->counter = (int )val;


 return (val);
}

int blk_get_queue(struct request_queue *q)
{
 int tmp ;
 int tmp___0 ;
 long tmp___1 ;

 tmp = __test_bit(5, (unsigned long *)(& q->queue_flags));
 if (tmp) {
  tmp___0 = 0;
 } else {
  tmp___0 = 1;
 }

 tmp___1 = get_nondet_int();
 if (tmp___1) {
  atomic_add_return(1, & q->refcnt);
  return (0);
 }
 return (1);
}

void elv_requeue_request(struct request_queue *x, struct request *y)
{
 return;
}

void blk_requeue_request(struct request_queue *q , struct request *rq )
{
 if (rq->flags & 1024UL) {
  blk_queue_end_tag_sl(q, rq, rq);
 }
 elv_requeue_request(q, rq);
 return;
}

void drive_stat_acct(struct request *rq , int nr_sectors , int new_io )
{
 int rw ;

 rw = (int )(rq->flags & 1UL);
 if (! (rq->flags & 64UL)) {
  return;
 } else {
  if (! rq->rq_disk) {
   return;
  }
 }
 if (! new_io) {
  (rq->rq_disk)->dkstats.merges[rw] ++;
 } else {
  disk_round_stats(rq->rq_disk);
  (rq->rq_disk)->in_flight ++;
 }
 return;
}

void blk_insert_request(struct request_queue *q , struct request *rq , int at_head , void *data)
{
 int where ;
 int tmp ;
 unsigned long flags ;
 unsigned long temp ;
 int tmp___0 ;

 if (at_head) {
  tmp = 1;
 } else {
  tmp = 2;
 }
 where = tmp;
 rq->flags |= 131080UL;
 rq->special = data;




 if (rq->flags & 1024UL) {
  blk_queue_end_tag_sl(q, rq, rq);
 }
 drive_stat_acct(rq, (int )rq->nr_sectors, 1);
 __elv_add_request(q, rq, where, 0);
 tmp___0 = __test_bit(7, (unsigned long *)(& q->queue_flags));
 if (tmp___0) {
  __generic_unplug_device(q);
 } else {

 }


 return;
}

struct bio *bio_map_user_iov(struct request_queue *x, struct block_device *y, struct sg_iovec *z, int u, int v)
{
 struct bio *a;
 if ((&nondet < 0)) {
  a = HsCreateBio();
 } else {
  a = 0;
 }
 return a;
}

int blk_rq_map_user_iov(struct request_queue *q, struct request *rq, struct sg_iovec *iov, int iov_count)
{
 struct bio *bio ;
 int tmp ;
 long tmp___0 ;

 if (! iov) {
  return (-22);
 } else {
  if (iov_count <= 0) {
   return (-22);
  }
 }
 bio = bio_map_user_iov(q, (struct block_device *)((void *)0), iov, iov_count, (rq->flags & 1UL) == 0UL);
 if (!bio) {
  return (-22);
 }


 bio->bi_next = rq->bio;
 rq->bio = bio;
 blk_rq_bio_prep(q, rq, bio);
 rq->data = (void *)0;
 rq->buffer = (char *)rq->data;
 rq->data_len = bio->bi_size;
 return (0);
}

void elv_add_request(struct request_queue *x, struct request *y, int z, int u)
{
 return;
}

void blk_execute_rq_nowait(struct request_queue *q, struct gendisk *bd_disk,
   struct request *rq, int at_head, rq_end_io_fn *done)
{
 int where ;
 int tmp ;

 if (at_head) {
  tmp = 1;
 } else {
  tmp = 2;
 }
 where = tmp;

 if (rq->rq_disk) HsFreeGendisk(rq->rq_disk);
 rq->rq_disk = bd_disk;
 rq->flags |= 128UL;
 rq->end_io = done;
 elv_add_request(q, rq, where, 1);
 generic_unplug_device(q);
 return;
}

void wait_for_completion(struct completion *x)
{
 return;
}

void blk_end_sync_rq(struct request *rq, int error)
{
 return;
}

int blk_execute_rq(struct request_queue *q, struct gendisk *bd_disk, struct request *rq, int at_head )
{
 struct completion wait ;
 char sense[96] ;
 int err ;
 void *__p ;
 size_t __n ;

 wait.done = 0U;
 wait.wait.task_list.next = & wait.wait.task_list;
 wait.wait.task_list.prev = & wait.wait.task_list;
 err = 0;
 rq->ref_count ++;
 if (! rq->sense) {
  __p = (void *)(sense);
  __n = sizeof(sense);
  if (__n != 0U) {
   __memzero(__p, __n);
  }
  rq->sense = (void *)(sense);
  rq->sense_len = 0U;
 }
 rq->waiting = & wait;
 blk_execute_rq_nowait(q, bd_disk, rq, at_head, & blk_end_sync_rq);
 wait_for_completion(& wait);
 rq->waiting = (struct completion *)((void *)0);
 if (rq->errors) {
  err = -5;
 }
 return (err);
}

void main_sub()
{
 struct gendisk *disk, *disk_tmp;
 struct request *req, *req_tmp;
 struct request_queue *q;
 struct scatterlist *sg;
 struct sg_iovec *iov;
 struct softirq_action *h;
 struct block_device *bdev;

 void *ubuf;
 char *page;

 int arg0;
 int arg1;

 blk_dev_init();
 q = blk_init_queue(0, 0);
 if (!q) return;

 HsInitialize();
 disk = HsCreateGendisk();
 req = HsCreateRequest();
 bdev = HsCreateBdev();

 HsInitRequestQueue(q);

 req->q = q;
 req->rl = &q->rq;
 q->orig_bar_rq = req;

 while ((&nondet < 0)) {
  if ((&nondet < 0)) {
   blk_register_queue(disk);
  }
  else if ((&nondet < 0)) {
   blk_unregister_queue(disk);
  }
  else if ((&nondet < 0)) {
   arg0 = get_nondet_int();
   end_request(req, arg0);
  }
  else if ((&nondet < 0)) {
   arg0 = get_nondet_int();
   arg1 = get_nondet_int();
   end_that_request_chunk(req, arg0, arg1);
  }
  else if ((&nondet < 0)) {
   blk_put_request(req);
  }
  else if ((&nondet < 0)) {
   blk_queue_invalidate_tags(q);
  }
  else if ((&nondet < 0)) {
   blk_start_queue(q);
  }
  else if ((&nondet < 0)) {
   blk_stop_queue(q);
  }
  else if ((&nondet < 0)) {
   blk_sync_queue(q);
  }
  else if ((&nondet < 0)) {
   sg = (struct scatterlist *)malloc(sizeof(struct scatterlist));
   blk_rq_map_sg(q, req, sg);
   free(sg);
  }
  else if ((&nondet < 0)) {
   blk_run_queue(q);
  }
  else if ((&nondet < 0)) {
   arg0 = get_nondet_int();
   arg1 = get_nondet_int();
   req_tmp = blk_get_request(q, arg0, arg1);
   if (req_tmp) HsFreeRequest(req_tmp);
  }
  else if ((&nondet < 0)) {
   page = get_nondet_ptr();
   queue_max_hw_sectors_show(q, page);
  }
  else if ((&nondet < 0)) {
   page = get_nondet_ptr();
   arg0 = get_nondet_int();
   queue_max_sectors_store(q, page, arg0);
  }
  else if ((&nondet < 0)) {
   page = get_nondet_ptr();
   queue_max_sectors_show(q, page);
  }
  else if ((&nondet < 0)) {
   page = get_nondet_ptr();
   arg0 = get_nondet_int();
   queue_ra_store(q, page, arg0);
  }
  else if ((&nondet < 0)) {
   page = get_nondet_ptr();
   queue_ra_show(q, page);
  }
  else if ((&nondet < 0)) {
   page = get_nondet_ptr();
   arg0 = get_nondet_int();
   queue_requests_store(q, page, arg0);
  }
  else if ((&nondet < 0)) {
   page = get_nondet_ptr();
   queue_requests_show(q, page);
  }
  else if ((&nondet < 0)) {
   ubuf = get_nondet_ptr();
   arg0 = get_nondet_int();
   blk_rq_map_user(q, req, ubuf, arg0);
  }
  else if ((&nondet < 0)) {
   req_tmp = HsCreateRequest();
   blk_complete_request(req_tmp);
  }
  else if ((&nondet < 0)) {
   blk_get_backing_dev_info(bdev);
  }
  else if ((&nondet < 0)) {
   blk_get_queue(q);
  }
  else if ((&nondet < 0)) {
   req_tmp = HsCreateRequest();
   blk_requeue_request(q, req_tmp);
   HsFreeRequest(req_tmp);
  }
  else if ((&nondet < 0)) {
   req_tmp = HsCreateRequest();
   ubuf = get_nondet_ptr();
   arg0 = get_nondet_int();
   blk_insert_request(q, req_tmp, arg0, ubuf);
   HsFreeRequest(req_tmp);
  }
  else if ((&nondet < 0)) {
   iov = get_nondet_ptr();
   arg0 = get_nondet_int();
   blk_rq_map_user_iov(q, req, iov, arg0);
  }
  else if ((&nondet < 0)) {
   arg0 = get_nondet_int();
   disk_tmp = HsCreateGendisk();
   blk_execute_rq(q, disk_tmp, req, arg0);
  }
 }

 blk_done_softirq(h);

 HsFreeBdev(bdev);
 HsFreeRequestQueue(q);
 HsFreeRequest(req);
 HsFreeGendisk(disk);
 HsFinalize();
 return;
}

int main()
{
 main_sub();
 return 0;
}
