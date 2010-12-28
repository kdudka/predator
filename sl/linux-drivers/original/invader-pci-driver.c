# 1 "apci-driver.c"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "apci-driver.c"






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
typedef unsigned char u8;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef long long s64;
typedef unsigned long long u64;
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
typedef unsigned long kernel_ulong_t;

struct audit_context;
struct backing_dev_info;
struct bus_attribute;
struct class;
struct class_attribute;
struct class_device;
struct class_device_attribute;
struct completion;
struct dentry;
struct device;
struct device_attribute;
struct device_driver;
struct dma_coherent_mem;
struct driver_attribute;
struct exception_table_entry;
struct exec_domain;
struct file;
struct files_struct;
struct hrtimer_base;
struct iattr;
struct io_context;
struct key;
struct kioctx;
struct klist_node;
struct kmem_cache;
struct kobject;
struct kobj_type;
struct kset;
struct linux_binfmt;
struct mm_struct;
struct module;
struct module_param_attrs;
struct namespace;
struct page;
struct pci_dev;
struct prio_array;
struct reclaim_state;
struct rw_semaphore;
struct siginfo;
struct subsystem;
struct task_struct;
struct thread_info;
struct timer_base_s;
struct tty_struct;
struct user_struct;
struct vfsmount;
struct vm_area_struct;
struct inode_operations;
struct file_operations;
struct file_lock;
struct cdev;
struct dnotify_struct;
struct address_space;
struct backing_dev_info;
struct dcookie_struct;
struct dentry_operations;
struct gendisk;
struct hd_struct;
struct inode;
struct iovec;
struct kiocb;
struct kstatfs;
struct module;
struct nameidata;
struct page;
struct pipe_inode_info;
struct poll_table_struct;
struct proc_dir_entry;
struct prio_tree_node;
struct pci_bus;
struct pci_dev;
struct pci_driver;
struct pci_ops;
struct quota_format_type;
struct radix_tree_node;
struct super_block;
struct vfsmount;
struct vm_area_struct;
struct writeback_control;

typedef int pci_power_t;
typedef unsigned int pci_channel_state_t;
typedef unsigned int pci_ers_result_t;
typedef __kernel_uid32_t qid_t;
typedef __u64 qsize_t;
typedef unsigned long mm_segment_t;
typedef struct kmem_cache kmem_cache_t;
typedef __u32 kernel_cap_t;
typedef unsigned long pgd_t[2];
typedef unsigned long pgprot_t;
typedef unsigned long cputime_t;
typedef void __signalfn_t(int );
typedef __signalfn_t *__sighandler_t;
typedef void __restorefn_t(void);
typedef __restorefn_t *__sigrestore_t;
typedef unsigned long mm_counter_t;
typedef struct prio_array prio_array_t;
typedef __u32 Elf32_Addr;
typedef __u16 Elf32_Half;
typedef __u32 Elf32_Word;

struct pci_device_id {
 __u32 vendor ;
 __u32 device ;
 __u32 subvendor ;
 __u32 subdevice ;
 __u32 class ;
 __u32 class_mask ;
 kernel_ulong_t driver_data ;
};
struct resource {
 char *name ;
 unsigned long start ;
 unsigned long end ;
 unsigned long flags ;
 struct resource *parent ;
 struct resource *sibling ;
 struct resource *child ;
};
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
struct hlist_node {
 struct hlist_node *next ;
 struct hlist_node **pprev ;
};
struct hlist_head {
 struct hlist_node *first ;
};
struct __anonstruct_atomic_t_4 {
 int counter ;
};
typedef struct __anonstruct_atomic_t_4 atomic_t;
struct attribute {
 char *name ;
 struct module *owner ;
 mode_t mode ;
};
struct attribute_group {
 char *name ;
 struct attribute **attrs ;
};
struct bin_attribute {
 struct attribute attr ;
 size_t size ;
 void *private ;
 ssize_t (*read)(struct kobject * , char * , loff_t , size_t ) ;
 ssize_t (*write)(struct kobject * , char * , loff_t , size_t ) ;
 int (*mmap)(struct kobject * , struct bin_attribute *attr , struct vm_area_struct *vma ) ;
};
struct sysfs_ops {
 ssize_t (*show)(struct kobject * , struct attribute * , char * ) ;
 ssize_t (*store)(struct kobject * , struct attribute * , char * , size_t ) ;
};
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
struct __anonstruct_raw_spinlock_t_5 {

};
typedef struct __anonstruct_raw_spinlock_t_5 raw_spinlock_t;
struct __anonstruct_raw_rwlock_t_6 {

};
typedef struct __anonstruct_raw_rwlock_t_6 raw_rwlock_t;
struct __anonstruct_spinlock_t_7 {
 raw_spinlock_t raw_lock ;
};
typedef struct __anonstruct_spinlock_t_7 spinlock_t;
struct __anonstruct_rwlock_t_8 {
 raw_rwlock_t raw_lock ;
};
typedef struct __anonstruct_rwlock_t_8 rwlock_t;
struct rw_semaphore {
 __s32 activity ;
 spinlock_t wait_lock ;
 struct list_head wait_list ;
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
struct completion {
 unsigned int done ;
 wait_queue_head_t wait ;
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
struct __anonstruct_cpumask_t_10 {
 unsigned long bits[1] ;
};
typedef struct __anonstruct_cpumask_t_10 cpumask_t;
struct semaphore {
 atomic_t count ;
 int sleepers ;
 wait_queue_head_t wait ;
};
struct __anonstruct_mm_context_t_12 {

};
typedef struct __anonstruct_mm_context_t_12 mm_context_t;
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
struct __anonstruct_sigset_t_13 {
 unsigned long sig[2] ;
};
typedef struct __anonstruct_sigset_t_13 sigset_t;
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
struct __anonstruct__kill_15 {
 pid_t _pid ;
 uid_t _uid ;
};
struct __anonstruct__timer_16 {
 timer_t _tid ;
 int _overrun ;
 char _pad[(int )(sizeof(uid_t ) - sizeof(int ))] ;
 sigval_t _sigval ;
 int _sys_private ;
};
struct __anonstruct__rt_17 {
 pid_t _pid ;
 uid_t _uid ;
 sigval_t _sigval ;
};
struct __anonstruct__sigchld_18 {
 pid_t _pid ;
 uid_t _uid ;
 int _status ;
 clock_t _utime ;
 clock_t _stime ;
};
struct __anonstruct__sigfault_19 {
 void *_addr ;
};
struct __anonstruct__sigpoll_20 {
 long _band ;
 int _fd ;
};
union __anonunion__sifields_14 {
 int _pad[(int )((128U - 3U * sizeof(int )) / sizeof(int ))] ;
 struct __anonstruct__kill_15 _kill ;
 struct __anonstruct__timer_16 _timer ;
 struct __anonstruct__rt_17 _rt ;
 struct __anonstruct__sigchld_18 _sigchld ;
 struct __anonstruct__sigfault_19 _sigfault ;
 struct __anonstruct__sigpoll_20 _sigpoll ;
};
struct siginfo {
 int si_signo ;
 int si_errno ;
 int si_code ;
 union __anonunion__sifields_14 _sifields ;
};
typedef struct siginfo siginfo_t;
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
struct pid {
 int nr ;
 struct hlist_node pid_chain ;
 struct list_head pid_list ;
};
struct cache_sizes {
 size_t cs_size ;
 kmem_cache_t *cs_cachep ;
 kmem_cache_t *cs_dmacachep ;
};
struct __anonstruct_seccomp_t_23 {

};
typedef struct __anonstruct_seccomp_t_23 seccomp_t;
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
struct __anonstruct_tv_25 {
 s32 nsec ;
 s32 sec ;
};
union __anonunion_ktime_t_24 {
 s64 tv64 ;
 struct __anonstruct_tv_25 tv ;
};
typedef union __anonunion_ktime_t_24 ktime_t;
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
union __anonunion_ki_obj_27 {
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
 union __anonunion_ki_obj_27 ki_obj ;
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
struct __anonstruct_local_t_33 {
 atomic_t a ;
};
typedef struct __anonstruct_local_t_33 local_t;
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
struct class {
 char *name ;
 struct module *owner ;
 struct subsystem subsys ;
 struct list_head children ;
 struct list_head interfaces ;
 struct semaphore sem ;
 struct class_attribute *class_attrs ;
 struct class_device_attribute *class_dev_attrs ;
 int (*uevent)(struct class_device *dev , char **envp , int num_envp , char *buffer ,
      int buffer_size ) ;
 void (*release)(struct class_device *dev ) ;
 void (*class_release)(struct class *class ) ;
};
struct class_attribute {
 struct attribute attr ;
 ssize_t (*show)(struct class * , char *buf ) ;
 ssize_t (*store)(struct class * , char *buf , size_t count ) ;
};
struct class_device_attribute {
 struct attribute attr ;
 ssize_t (*show)(struct class_device * , char *buf ) ;
 ssize_t (*store)(struct class_device * , char *buf , size_t count ) ;
};
struct class_device {
 struct list_head node ;
 struct kobject kobj ;
 struct class *class ;
 dev_t devt ;
 struct class_device_attribute *devt_attr ;
 struct class_device_attribute uevent_attr ;
 struct device *dev ;
 void *class_data ;
 struct class_device *parent ;
 void (*release)(struct class_device *dev ) ;
 int (*uevent)(struct class_device *dev , char **envp , int num_envp , char *buffer ,
      int buffer_size ) ;
 char class_id[20] ;
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
enum pci_channel_state {
    pci_channel_io_normal = 1,
    pci_channel_io_frozen = 2,
    pci_channel_io_perm_failure = 3
} ;
struct pci_dev {
 struct list_head global_list ;
 struct list_head bus_list ;
 struct pci_bus *bus ;
 struct pci_bus *subordinate ;
 void *sysdata ;
 struct proc_dir_entry *procent ;
 unsigned int devfn ;
 unsigned short vendor ;
 unsigned short device ;
 unsigned short subsystem_vendor ;
 unsigned short subsystem_device ;
 unsigned int class ;
 u8 hdr_type ;
 u8 rom_base_reg ;
 u8 pin ;
 struct pci_driver *driver ;
 u64 dma_mask ;
 pci_power_t current_state ;
 pci_channel_state_t error_state ;
 struct device dev ;
 unsigned short vendor_compatible[4] ;
 unsigned short device_compatible[4] ;
 int cfg_size ;
 unsigned int irq ;
 struct resource resource[12] ;
 unsigned int transparent : 1 ;
 unsigned int multifunction : 1 ;
 unsigned int is_enabled : 1 ;
 unsigned int is_busmaster : 1 ;
 unsigned int no_msi : 1 ;
 unsigned int block_ucfg_access : 1 ;
 u32 saved_config_space[16] ;
 struct bin_attribute *rom_attr ;
 int rom_attr_enabled ;
 struct bin_attribute *res_attr[12] ;
};
struct pci_bus {
 struct list_head node ;
 struct pci_bus *parent ;
 struct list_head children ;
 struct list_head devices ;
 struct pci_dev *self ;
 struct resource *resource[8] ;
 struct pci_ops *ops ;
 void *sysdata ;
 struct proc_dir_entry *procdir ;
 unsigned char number ;
 unsigned char primary ;
 unsigned char secondary ;
 unsigned char subordinate ;
 char name[48] ;
 unsigned short bridge_ctl ;
 unsigned short pad2 ;
 struct device *bridge ;
 struct class_device class_dev ;
 struct bin_attribute *legacy_io ;
 struct bin_attribute *legacy_mem ;
};
struct pci_ops {
 int (*read)(struct pci_bus *bus , unsigned int devfn , int where , int size , u32 *val ) ;
 int (*write)(struct pci_bus *bus , unsigned int devfn , int where , int size ,
     u32 val ) ;
};
struct pci_error_handlers {
 pci_ers_result_t (*error_detected)(struct pci_dev *dev , enum pci_channel_state error ) ;
 pci_ers_result_t (*mmio_enabled)(struct pci_dev *dev ) ;
 pci_ers_result_t (*link_reset)(struct pci_dev *dev ) ;
 pci_ers_result_t (*slot_reset)(struct pci_dev *dev ) ;
 void (*resume)(struct pci_dev *dev ) ;
};
struct pci_driver {
 struct list_head node ;
 char *name ;
 struct pci_device_id *id_table ;
 int (*probe)(struct pci_dev *dev , struct pci_device_id *id ) ;
 void (*remove)(struct pci_dev *dev ) ;
 int (*suspend)(struct pci_dev *dev , pm_message_t state ) ;
 int (*resume)(struct pci_dev *dev ) ;
 int (*enable_wake)(struct pci_dev *dev , pci_power_t state , int enable ) ;
 void (*shutdown)(struct pci_dev *dev ) ;
 struct pci_error_handlers *err_handler ;
 struct device_driver driver ;
 struct pci_dynid *next ;
 spinlock_t lock ;
 unsigned int use_driver_data : 1 ;
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
struct qstr {
 unsigned int hash ;
 unsigned int len ;
 unsigned char *name ;
};
union __anonunion_d_u_35 {
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
 union __anonunion_d_u_35 d_u ;
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
union __anonunion_u_36 {
 struct v1_mem_dqinfo v1_i ;
 struct v2_mem_dqinfo v2_i ;
};
struct mem_dqinfo {
 struct quota_format_type *dqi_format ;
 struct list_head dqi_dirty_list ;
 unsigned long dqi_flags ;
 unsigned int dqi_bgrace ;
 unsigned int dqi_igrace ;
 union __anonunion_u_36 u ;
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
union __anonunion_u_37 {
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
 union __anonunion_u_37 u ;
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
union __anonunion_f_u_38 {
 struct list_head fu_list ;
 struct rcu_head fu_rcuhead ;
};
struct file {
 union __anonunion_f_u_38 f_u ;
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
union __anonunion_fl_u_39 {
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
 union __anonunion_fl_u_39 fl_u ;
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
union __anonunion_arg_42 {
 char *buf ;
 void *data ;
};
struct __anonstruct_read_descriptor_t_41 {
 size_t written ;
 size_t count ;
 union __anonunion_arg_42 arg ;
 int error ;
};
typedef struct __anonstruct_read_descriptor_t_41 read_descriptor_t;
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
struct anon_vma;
struct anon_vma;
struct mm_struct;
struct mm_struct;
struct __anonstruct_vm_set_45 {
 struct list_head list ;
 void *parent ;
 struct vm_area_struct *head ;
};
union __anonunion_shared_44 {
 struct __anonstruct_vm_set_45 vm_set ;
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
 union __anonunion_shared_44 shared ;
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
struct __anonstruct____missing_field_name_47 {
 unsigned long private ;
 struct address_space *mapping ;
};
union __anonunion____missing_field_name_46 {
 struct __anonstruct____missing_field_name_47 __annonCompField1 ;
};
struct page {
 unsigned long flags ;
 atomic_t _count ;
 atomic_t _mapcount ;
 union __anonunion____missing_field_name_46 __annonCompField2 ;
 unsigned long index ;
 struct list_head lru ;
};
struct page;
struct vm_area_struct;
struct pci_dynid {
 struct pci_dynid *next;
 struct pci_device_id id ;
};

int HsPrintk(char *fmt , ...)
{
 int a;
 return a;
}
void __memzero(void *ptr , __kernel_size_t n )
{
 return;
}
int sscanf(char *x, char *y, ...)
{
 int a;
 return a;
}
int pci_uevent(struct device *dev , char **envp , int num_envp , char *buffer , int buffer_size )
{
 int a;
 return a;
}

ssize_t store_new_id(struct device_driver *driver, char *buf, size_t count);

static struct pci_driver pci_compat_driver;

struct bus_type pci_bus_type;

struct bus_type pci_bus_type ;

struct sysfs_ops pci_driver_sysfs_ops;

struct kobj_type pci_driver_kobj_type =
{
 (void (*)(struct kobject * ))0,
 & pci_driver_sysfs_ops,
 (struct attribute **)0
};

struct driver_attribute driver_attr_new_id =
{
 {"new_id", (struct module *)0, (mode_t )128},
 (ssize_t (*)(struct device_driver *, char *buf))((void *)0),
 & store_new_id
};



struct pci_driver *HsDrv;
struct pci_dev *HsDev;

struct pci_driver *HsCreatePciDriver()
{
 struct pci_driver *dri;
 struct pci_dynid *dynid;
 dri = (struct pci_driver *)malloc(sizeof(struct pci_driver));
 dri->id_table = (struct pci_device_id *)malloc(sizeof(struct pci_device_id));
 dri->next = (struct pci_dynid *)dri;
 while ((&nondet < 0)) {
  dynid = (struct pci_dynid *)malloc(sizeof(struct pci_dynid));
  dynid->next = dri->next;
  dri->next = dynid;
 }
 return dri;
}

void HsFreePciDriver(struct pci_driver *dri)
{
 struct pci_dynid *dynid, *next;
 dynid = dri->next;
 while (dynid != (struct pci_dynid *)dri) {
  next = dynid->next;
  free(dynid);
  dynid = next;
 }
 if (dri->id_table != 0) free(dri->id_table);
 free(dri);
 return;
}

struct pci_dev *HsCreatePciDevice()
{
 struct pci_dev *dev;
 dev = (struct pci_dev *)malloc(sizeof(struct pci_dev));
 return dev;
}

void HsFreePciDevice(struct pci_dev *dev)
{
 free(dev);
 return;
}

void HsInitialize()
{
 HsDrv = HsCreatePciDriver();
 HsDev = HsCreatePciDevice();
 if ((&nondet < 0)) {
  HsDev->driver = HsDrv;
 } else {
  HsDev->driver = 0;
 }
 return;
}

void HsFinalize()
{
 HsFreePciDriver(HsDrv);
 HsFreePciDevice(HsDev);
 HsDrv = 0;
 HsDev = 0;
 return;
}

void driver_unregister(struct device_driver *drv)
{
 return;
}

static void pci_free_dynids(struct pci_driver *drv)
{
 struct pci_dynid *dynid;
 struct pci_dynid *n ;

 dynid = drv->next;
 n = dynid->next;
 while (dynid != (struct pci_dynid *)drv) {
  drv->next = n;
  free(dynid);
  dynid = n;
  n = dynid->next;
 }
 return;
}

void pci_unregister_driver(struct pci_driver *drv)
{
 driver_unregister(& drv->driver);
 pci_free_dynids(drv);
 return;
}

void pci_device_shutdown(struct pci_dev *dev)
{
 struct pci_dev *pci_dev ;
 struct pci_driver *drv ;

 pci_dev = dev;
 drv = pci_dev->driver;
 if (drv) {
  if (drv->shutdown) {

  }
 }
 return;
}

int driver_register(struct device_driver *drv )
{
 int a; return a;
}

int sysfs_create_file(struct kobject *x, struct attribute *y)
{
 int a; return a;
}

int pci_create_newid_file(struct pci_driver *drv )
{
 int error ;
 error = 0;
 if ((unsigned int )drv->probe != (unsigned int )((void *)0)) {
  error = sysfs_create_file(&drv->driver.kobj, (struct attribute *)(&driver_attr_new_id.attr));
 }
 return (error);
}

int __pci_register_driver(struct pci_driver *drv , struct module *owner )
{
 int error ;
 spinlock_t __r_expr_0 ;

 drv->driver.name = (char *)drv->name;
 drv->driver.bus = & pci_bus_type;
 if (!drv->driver.shutdown) {
  drv->driver.shutdown = (void (*)(struct device *dev ))&pci_device_shutdown;
 } else {
  HsPrintk("<4>Warning: PCI driver %s has a struct device_driver shutdown method, please update!\n", drv->name);
 }
 drv->driver.owner = owner;
 drv->driver.kobj.ktype = &pci_driver_kobj_type;
 drv->lock = __r_expr_0;
 drv->next = (struct pci_dynid *)(void *)drv;
 error = driver_register(&drv->driver);
 if (! error) {
  error = pci_create_newid_file(drv);
 }
 return (error);
}

struct device_driver *get_driver(struct device_driver *drv)
{
 struct device_driver *a;
 return a;
}

void put_driver(struct device_driver *drv)
{
 return;
}

void driver_attach(struct device_driver *drv )
{
 return;
}

struct pci_driver *get_pci_driver(struct device_driver *drv)
{
 return HsDrv;
}

ssize_t store_new_id(struct device_driver *driver , char *buf , size_t count )
{
 struct pci_dynid *dynid, *prev, *curr;
 struct pci_driver *pdrv ;
 struct device_driver *__mptr ;
 __u32 vendor ;
 __u32 device ;
 __u32 subvendor ;
 __u32 subdevice ;
 __u32 class ;
 __u32 class_mask ;
 unsigned long driver_data ;
 int fields ;
 void *__p ;
 size_t __n ;
 struct device_driver *tmp ;

 pdrv = get_pci_driver(driver);

 vendor = (__u32 )(~ 0);
 device = (__u32 )(~ 0);
 subvendor = (__u32 )(~ 0);
 subdevice = (__u32 )(~ 0);
 class = (__u32 )0;
 class_mask = (__u32 )0;
 driver_data = 0UL;
 fields = 0;
 fields = sscanf(buf, "%x %x %x %x %x %x %lux", & vendor, & device, & subvendor,
     & subdevice, & class, & class_mask, & driver_data);
 if (fields < 0) {
  return (-22);
 }
 dynid = (struct pci_dynid *)mallocnull(sizeof((*dynid)));
 if (! dynid) {
  return (-12);
 }
 __p = (void *)dynid;
 __n = sizeof((*dynid));
 if (__n != 0U) {
  __memzero(__p, __n);
 }
 dynid->next = dynid;
 dynid->id.vendor = vendor;
 dynid->id.device = device;
 dynid->id.subvendor = subvendor;
 dynid->id.subdevice = subdevice;
 dynid->id.class = class;
 dynid->id.class_mask = class_mask;
 if (pdrv->use_driver_data) {
  dynid->id.driver_data = driver_data;
 } else {
  dynid->id.driver_data = 0UL;
 }
 if (pdrv->next == (struct pci_dynid *)pdrv) {
  dynid->next = (struct pci_dynid *)pdrv;
  pdrv->next = dynid;
 } else {
  prev = pdrv->next;
  curr = prev->next;
  while (curr != (struct pci_dynid *)pdrv) {
   prev = curr;
   curr = curr->next;
  }
  dynid->next = (struct pci_dynid *)pdrv;
  prev->next = dynid;
 }
 tmp = get_driver(& pdrv->driver);
 if (tmp) {
  driver_attach(& pdrv->driver);
  put_driver(& pdrv->driver);
 }
 return ((int )count);
}

ssize_t pci_driver_attr_store(struct device_driver *driver,
   struct driver_attribute *dattr,
   char *buf, size_t count)
{
 ssize_t ret ;
 struct device_driver *tmp ;

 tmp = get_driver(driver);
 if (! tmp) {
  return (-19);
 }
 if (dattr->store) {

  ret = store_new_id(driver, buf, count);
 } else {
  ret = -5;
 }
 put_driver(driver);
 return (ret);
}

ssize_t pci_driver_attr_show(struct device_driver *driver, struct driver_attribute *dattr, char *buf)
{
 ssize_t ret ;
 struct device_driver *tmp ;

 tmp = get_driver(driver);
 if (! tmp) {
  return (-19);
 }
 if (dattr->show) {

 } else {
  ret = -5;
 }
 put_driver(driver);
 return (ret);
}

struct pci_device_id *pci_match_one_device(struct pci_device_id *id, struct pci_dev *dev)
{
 if (id->vendor == 4294967295U) {
  goto _L___2;
 } else {
  if (id->vendor == (__u32 )dev->vendor) {
_L___2:
   if (id->device == 4294967295U) {
    goto _L___1;
   } else {
    if (id->device == (__u32 )dev->device) {
_L___1:
     if (id->subvendor == 4294967295U) {
      goto _L___0;
     } else {
      if (id->subvendor == (__u32 )dev->subsystem_vendor) {
_L___0:
       if (id->subdevice == 4294967295U) {
        goto _L;
       } else {
        if (id->subdevice == (__u32 )dev->subsystem_device) {
_L:
         if (! ((id->class ^ dev->class) & id->class_mask)) {
          return (id);
         }
        }
       }
      }
     }
    }
   }
  }
 }
 return ((struct pci_device_id *)((void *)0));
}

struct pci_device_id *pci_match_id(struct pci_device_id *ids , struct pci_dev *dev )
{
 struct pci_device_id *tmp ;
 if (ids) {
  while (1) {
   if (!ids->vendor) {
    if (! ids->subvendor) {
     if (! ids->class_mask) {
      break;
     }
    }
   }
   tmp = pci_match_one_device(ids, (struct pci_dev *)dev);
   if (tmp) {
    return (ids);
   }
   ids ++;
  }
 }
 return ((struct pci_device_id *)((void *)0));
}

void prefetch(void *x)
{
 return;
}

struct pci_device_id *pci_match_device(struct pci_driver *drv , struct pci_dev *dev )
{
 struct pci_device_id *id ;
 struct pci_dynid *dynid ;
 struct list_head *__mptr ;
 struct list_head *__mptr___0 ;
 struct pci_device_id *tmp ;

 id = pci_match_id(drv->id_table, dev);
 if (id) {
  return (id);
 }
 dynid = drv->next;
 while (1) {
  prefetch((void *)dynid->next);
  if (dynid == (struct pci_dynid *)drv) {
   break;
  }
  tmp = pci_match_one_device((struct pci_device_id *)(& dynid->id), (struct pci_dev *)dev);
  if (tmp) {
   return ((struct pci_device_id *)(&dynid->id));
  }
  dynid = dynid->next;
 }
 return ((struct pci_device_id *)((void *)0));
}

int pci_bus_match(struct pci_dev *pci_dev, struct pci_driver *pci_drv )
{
 struct pci_device_id *found_id ;

 found_id = pci_match_device(pci_drv, pci_dev);
 if (found_id) {
  return (1);
 }
 return (0);
}

int pci_restore_state(struct pci_dev *dev)
{
 int a; return a;
}

int pci_enable_device(struct pci_dev *dev)
{
 int a; return a;
}

void pci_set_master(struct pci_dev *dev )
{
 return;
}

void pci_default_resume(struct pci_dev *pci_dev )
{
 int retval ;
 pci_restore_state(pci_dev);
 if (pci_dev->is_enabled) {
  retval = pci_enable_device(pci_dev);
 }
 if (pci_dev->is_busmaster) {
  pci_set_master(pci_dev);
 }
 return;
}

int pci_device_resume(struct pci_dev *pci_dev)
{
 struct pci_driver *drv;

 drv = pci_dev->driver;
 if (drv) {
  if (drv->resume) {

   pci_default_resume(pci_dev);
  } else {
   pci_default_resume(pci_dev);
  }
 } else {
  pci_default_resume(pci_dev);
 }
 return (0);
}

int pci_save_state(struct pci_dev *dev)
{
 int a; return a;
}

int pci_device_suspend(struct pci_dev *pci_dev, int event)
{
 struct pci_driver *drv;
 int i;

 drv = pci_dev->driver;
 i = 0;
 if (drv) {
  if (drv->suspend) {

  } else {
   pci_save_state(pci_dev);
  }
 } else {
  pci_save_state(pci_dev);
 }
 return (i);
}

struct device *get_device(struct device *dev )
{
 struct device *a; return a;
}

struct pci_dev *pci_dev_get(struct pci_dev *dev )
{
 if (dev) {
  get_device(&dev->dev);
 }
 return (dev);
}

int pci_call_probe(struct pci_driver *drv , struct pci_dev *dev , struct pci_device_id *id)
{
 int error ;

 error = get_nondet_int();
 return (error);
}

int __pci_device_probe(struct pci_driver *drv , struct pci_dev *pci_dev )
{
 struct pci_device_id *id ;
 int error ;

 error = 0;
 if (!pci_dev->driver && drv) {
  if (drv->probe) {
   error = -19;
   id = pci_match_device(drv, pci_dev);
   if (id) {
    error = pci_call_probe(drv, pci_dev, id);
   }
   if (error >= 0) {
    pci_dev->driver = drv;
    error = 0;
   }
  }
 }
 return (error);
}

void put_device(struct device *dev)
{
 return;
}

void pci_dev_put(struct pci_dev *dev)
{
 if (dev) {
  put_device(& dev->dev);
 }
 return;
}

int pci_device_probe(struct pci_dev *pci_dev)
{
 int error ;
 struct pci_driver *drv ;

 error = 0;
 drv = pci_dev->driver;
 pci_dev_get(pci_dev);



 error = __pci_device_probe(drv, pci_dev);
 if (error) {
  pci_dev_put(pci_dev);
 }
 return (error);
}

int pci_device_remove(struct pci_dev *pci_dev)
{
 struct pci_driver *drv ;

 drv = pci_dev->driver;
 if (drv) {
  if (drv->remove) {

  }
  pci_dev->driver = (struct pci_driver *)((void *)0);
 }
 pci_dev_put(pci_dev);
 return (0);
}

struct pci_driver *pci_dev_driver(struct pci_dev *dev)
{
 int i ;

 if (dev->driver) {
  return ((struct pci_driver *)dev->driver);
 } else {
  i = 0;
  while (i <= 6) {
   if (dev->resource[i].flags & 2147483648UL) {
    return (& pci_compat_driver);
   }
   i ++;
  }
 }
 return ((struct pci_driver *)((void *)0));
}

void main_sub()
{
 struct pci_dev *dev;
 struct device_driver *driver;
 struct driver_attribute *dattr;
 char *buf;
 size_t count;
 int event;

 HsInitialize();
 while ((&nondet < 0)) {
  if ((&nondet < 0)) {
   pci_unregister_driver(HsDrv);
  }
  else if ((&nondet < 0)) {
   dev = (struct pci_dev *)malloc(sizeof(struct pci_dev));
   dev->driver = HsDrv;
   pci_device_shutdown(dev);
   free(dev);
  }
  else if ((&nondet < 0)) {
   driver = (struct device_driver *)malloc(sizeof(struct device_driver));
   dattr = (struct driver_attribute *)malloc(sizeof(struct driver_attribute));
   buf = (char *)malloc(sizeof(char));
   count = get_nondet_int();
   pci_driver_attr_store(driver, dattr, buf, count);
   free(driver);
   free(dattr);
   free(buf);
  }
  else if ((&nondet < 0)) {
   driver = (struct device_driver *)malloc(sizeof(struct device_driver));
   dattr = (struct driver_attribute *)malloc(sizeof(struct driver_attribute));
   buf = (char *)malloc(sizeof(char));
   pci_driver_attr_show(driver, dattr, buf);
   free(driver);
   free(dattr);
   free(buf);
  }
  else if ((&nondet < 0)) {
   pci_bus_match(HsDev, HsDrv);
  }
  else if ((&nondet < 0)) {
   pci_device_resume(HsDev);
  }
  else if ((&nondet < 0)) {
   event = get_nondet_int();
   pci_device_suspend(HsDev, event);
  }
  else if ((&nondet < 0)) {
   pci_device_probe(HsDev);
  }
  else if ((&nondet < 0)) {
   pci_device_remove(HsDev);
  }
  else if ((&nondet < 0)) {
   pci_dev_driver(HsDev);
  }
 }
 pci_unregister_driver(HsDrv);
 HsFinalize();
 return;
}

int main()
{
 main_sub();
 return 0;
}
