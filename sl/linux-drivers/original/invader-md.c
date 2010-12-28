# 1 "amd.c"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "amd.c"






void free(void *);
void *malloc(int);
void *mallocnull(int);






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

struct address_space;
struct anon_vma;
struct audit_context;
struct backing_dev_info;
struct bio;
struct bio_vec;
struct bitmap;
struct bootmem_data;
struct bus_attribute;
struct cdev;
struct cfq_queue;
struct completion;
struct ctl_table;
struct dcookie_struct;
struct dentry;
struct dentry_operations;
struct device;
struct device_attribute;
struct device_driver;
struct dma_coherent_mem;
struct dnotify_struct;
struct elevator_queue;
struct exception_table_entry;
struct exec_domain;
struct export_operations;
struct fasync_struct;
struct file;
struct files_struct;
struct file_lock;
struct file_operations;
struct file_system_type;
struct gendisk;
struct hd_geometry;
struct hd_struct;
struct hrtimer_base;
struct iattr;
struct inode;
struct inode_operations;
struct iovec;
struct io_context;
struct key;
struct kiocb;
struct kioctx;
struct klist_node;
struct kmem_cache;
struct kobject;
struct kobj_type;
struct kset;
struct kstatfs;
struct linux_binfmt;
struct mddev_s;
struct mdk_personality;
struct mdk_rdev_s;
struct mdk_thread_s;
struct mm_struct;
struct module;
struct module_param_attrs;
struct nameidata;
struct namespace;
struct nfs4_lock_state;
struct nlm_lockowner;
struct notifier_block;
struct page;
struct pglist_data;
struct pipe_inode_info;
struct poll_table_struct;
struct prio_array;
struct prio_tree_node;
struct quota_format_type;
struct radix_tree_node;
struct reclaim_state;
struct request;
struct request_pm_state;
struct request_queue;
struct rw_semaphore;
struct sem_undo;
struct seq_file;
struct seq_operations;
struct siginfo;
struct subsystem;
struct super_block;
struct super_operations;
struct task_struct;
struct thread_info;
struct timer_base_s;
struct timer_rand_state;
struct tty_struct;
struct user_struct;
struct vfsmount;
struct vm_area_struct;
struct vm_operations_struct;
struct work_struct;
struct writeback_control;
struct xattr_handler;
struct zone;

struct __anonstruct___kernel_fd_set_1 {
 unsigned long fds_bits[(int )(1024U / (8U * sizeof(unsigned long )))] ;
};
typedef struct __anonstruct___kernel_fd_set_1 __kernel_fd_set;
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
typedef __kernel_fd_set fd_set;
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
typedef unsigned long sector_t;
typedef unsigned int gfp_t;
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
struct __wait_queue {
 unsigned int flags ;
 void *private ;
 int (*func)(struct __wait_queue *wait , unsigned int mode , int sync , void *key ) ;
 struct list_head task_list ;
};
typedef struct __wait_queue wait_queue_t;
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
typedef unsigned long pgd_t[2];
typedef unsigned long pgprot_t;
struct __anonstruct_mm_context_t_11 {

};
typedef struct __anonstruct_mm_context_t_11 mm_context_t;
typedef unsigned long cputime_t;
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
struct free_area {
 struct list_head free_list ;
 unsigned long nr_free ;
};
struct per_cpu_pages {
 int count ;
 int high ;
 int batch ;
 struct list_head list ;
};
struct per_cpu_pageset {
 struct per_cpu_pages pcp[2] ;
};
struct zone {
 unsigned long free_pages ;
 unsigned long pages_min ;
 unsigned long pages_low ;
 unsigned long pages_high ;
 unsigned long lowmem_reserve[4] ;
 struct per_cpu_pageset pageset[1] ;
 spinlock_t lock ;
 struct free_area free_area[11] ;
 spinlock_t lru_lock ;
 struct list_head active_list ;
 struct list_head inactive_list ;
 unsigned long nr_scan_active ;
 unsigned long nr_scan_inactive ;
 unsigned long nr_active ;
 unsigned long nr_inactive ;
 unsigned long pages_scanned ;
 int all_unreclaimable ;
 atomic_t reclaim_in_progress ;
 unsigned long last_unsuccessful_zone_reclaim ;
 int temp_priority ;
 int prev_priority ;
 wait_queue_head_t *wait_table ;
 unsigned long wait_table_size ;
 unsigned long wait_table_bits ;
 struct pglist_data *zone_pgdat ;
 struct page *zone_mem_map ;
 unsigned long zone_start_pfn ;
 unsigned long spanned_pages ;
 unsigned long present_pages ;
 char *name ;
};
struct zonelist {
 struct zone *zones[5] ;
};
struct pglist_data {
 struct zone node_zones[4] ;
 struct zonelist node_zonelists[5] ;
 int nr_zones ;
 struct page *node_mem_map ;
 struct bootmem_data *bdata ;
 unsigned long node_start_pfn ;
 unsigned long node_present_pages ;
 unsigned long node_spanned_pages ;
 int node_id ;
 struct pglist_data *pgdat_next ;
 wait_queue_head_t kswapd_wait ;
 struct task_struct *kswapd ;
 int kswapd_max_order ;
};
struct notifier_block {
 int (*notifier_call)(struct notifier_block *self , unsigned long , void * ) ;
 struct notifier_block *next ;
 int priority ;
};
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
struct qstr {
 unsigned int hash ;
 unsigned int len ;
 unsigned char *name ;
};
union __anonunion_d_u_34 {
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
 union __anonunion_d_u_34 d_u ;
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
union __anonunion_u_35 {
 struct v1_mem_dqinfo v1_i ;
 struct v2_mem_dqinfo v2_i ;
};
struct mem_dqinfo {
 struct quota_format_type *dqi_format ;
 struct list_head dqi_dirty_list ;
 unsigned long dqi_flags ;
 unsigned int dqi_bgrace ;
 unsigned int dqi_igrace ;
 union __anonunion_u_35 u ;
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
union __anonunion_u_36 {
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
 union __anonunion_u_36 u ;
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
union __anonunion_f_u_37 {
 struct list_head fu_list ;
 struct rcu_head fu_rcuhead ;
};
struct file {
 union __anonunion_f_u_37 f_u ;
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
union __anonunion_fl_u_38 {
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
 union __anonunion_fl_u_38 fl_u ;
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
union __anonunion_arg_41 {
 char *buf ;
 void *data ;
};
struct __anonstruct_read_descriptor_t_40 {
 size_t written ;
 size_t count ;
 union __anonunion_arg_41 arg ;
 int error ;
};
typedef struct __anonstruct_read_descriptor_t_40 read_descriptor_t;
struct file_operations {
 struct module *owner ;
 loff_t (*llseek)(struct file * , loff_t , int ) ;
 ssize_t (*read)(struct file * , char * , size_t , loff_t * ) ;
 ssize_t (*aio_read)(struct kiocb * , char * , size_t , loff_t ) ;
 ssize_t (*write)(struct file * , char * , size_t , loff_t * ) ;
 ssize_t (*aio_write)(struct kiocb * , char * , size_t , loff_t ) ;
 int (*readdir)(struct file * , void * , int (*)(void * , char * , int , loff_t , ino_t , unsigned int ) ) ;
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
 struct super_block *(*get_sb)(struct file_system_type * , int , char * ,
          void * ) ;
 void (*kill_sb)(struct super_block * ) ;
 struct module *owner ;
 struct file_system_type *next ;
 struct list_head fs_supers ;
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
struct __anonstruct_vm_set_45 {
 struct list_head list ;
 void *parent ;
 struct vm_area_struct *head ;
};
union __anonunion_shared_44 {
 struct __anonstruct_vm_set_45 vm_set ;
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
struct exception_table_entry {
 unsigned long insn ;
 unsigned long fixup ;
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
struct hd_geometry {
 unsigned char heads ;
 unsigned char sectors ;
 unsigned short cylinders ;
 unsigned long start ;
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
struct seq_file {
 char *buf ;
 size_t size ;
 size_t from ;
 size_t count ;
 loff_t index ;
 loff_t version ;
 struct semaphore sem ;
 struct seq_operations *op ;
 void *private ;
};
struct seq_operations {
 void *(*start)(struct seq_file *m , loff_t *pos ) ;
 void (*stop)(struct seq_file *m , void *v ) ;
 void *(*next)(struct seq_file *m , void *v , loff_t *pos ) ;
 int (*show)(struct seq_file *m , void *v ) ;
};
struct mdp_device_descriptor_s {
 __u32 number ;
 __u32 major ;
 __u32 minor ;
 __u32 raid_disk ;
 __u32 state ;
 __u32 reserved[27] ;
};
typedef struct mdp_device_descriptor_s mdp_disk_t;
struct mdp_superblock_s {
 __u32 md_magic ;
 __u32 major_version ;
 __u32 minor_version ;
 __u32 patch_version ;
 __u32 gvalid_words ;
 __u32 set_uuid0 ;
 __u32 ctime ;
 __u32 level ;
 __u32 size ;
 __u32 nr_disks ;
 __u32 raid_disks ;
 __u32 md_minor ;
 __u32 not_persistent ;
 __u32 set_uuid1 ;
 __u32 set_uuid2 ;
 __u32 set_uuid3 ;
 __u32 gstate_creserved[16] ;
 __u32 utime ;
 __u32 state ;
 __u32 active_disks ;
 __u32 working_disks ;
 __u32 failed_disks ;
 __u32 spare_disks ;
 __u32 sb_csum ;
 __u32 events_lo ;
 __u32 events_hi ;
 __u32 cp_events_lo ;
 __u32 cp_events_hi ;
 __u32 recovery_cp ;
 __u32 gstate_sreserved[20] ;
 __u32 layout ;
 __u32 chunk_size ;
 __u32 root_pv ;
 __u32 root_block ;
 __u32 pstate_reserved[60] ;
 mdp_disk_t disks[27] ;
 __u32 reserved[0] ;
 mdp_disk_t this_disk ;
};
typedef struct mdp_superblock_s mdp_super_t;
struct mdp_superblock_1 {
 __u32 magic ;
 __u32 major_version ;
 __u32 feature_map ;
 __u32 pad0 ;
 __u8 set_uuid[16] ;
 char set_name[32] ;
 __u64 ctime ;
 __u32 level ;
 __u32 layout ;
 __u64 size ;
 __u32 chunksize ;
 __u32 raid_disks ;
 __u32 bitmap_offset ;
 __u8 pad1[28] ;
 __u64 data_offset ;
 __u64 data_size ;
 __u64 super_offset ;
 __u64 recovery_offset ;
 __u32 dev_number ;
 __u32 cnt_corrected_read ;
 __u8 device_uuid[16] ;
 __u8 devflags ;
 __u8 pad2[7] ;
 __u64 utime ;
 __u64 events ;
 __u64 resync_offset ;
 __u32 sb_csum ;
 __u32 max_dev ;
 __u8 pad3[32] ;
 __u16 dev_roles[0] ;
};
struct mdu_version_s {
 int major ;
 int minor ;
 int patchlevel ;
};
typedef struct mdu_version_s mdu_version_t;
struct mdu_array_info_s {
 int major_version ;
 int minor_version ;
 int patch_version ;
 int ctime ;
 int level ;
 int size ;
 int nr_disks ;
 int raid_disks ;
 int md_minor ;
 int not_persistent ;
 int utime ;
 int state ;
 int active_disks ;
 int working_disks ;
 int failed_disks ;
 int spare_disks ;
 int layout ;
 int chunk_size ;
};
typedef struct mdu_array_info_s mdu_array_info_t;
struct mdu_disk_info_s {
 int number ;
 int major ;
 int minor ;
 int raid_disk ;
 int state ;
};
typedef struct mdu_disk_info_s mdu_disk_info_t;
struct mdu_bitmap_file_s {
 char pathname[4096] ;
};
typedef struct mdu_bitmap_file_s mdu_bitmap_file_t;
struct mdu_param_s {
 int personality ;
 int chunk_size ;
 int max_fault ;
};
typedef struct mdu_param_s mdu_param_t;
typedef struct mddev_s mddev_t;
typedef struct mdk_rdev_s mdk_rdev_t;
struct mdk_rdev_s {

 mdk_rdev_t *disks;
 sector_t size ;
 mddev_t *mddev ;
 unsigned long last_events ;
 struct block_device *bdev ;
 struct page *sb_page ;
 int sb_loaded ;
 sector_t data_offset ;
 sector_t sb_offset ;
 int sb_size ;
 int preferred_minor ;
 struct kobject kobj ;
 unsigned long flags ;
 int desc_nr ;
 int raid_disk ;
 int saved_raid_disk ;
 atomic_t nr_pending ;
 atomic_t read_errors ;
 atomic_t corrected_errors ;
};
struct mdk_rdev_header {
 mdk_rdev_t *disks;
};
struct mddev_s {
 void *private ;
 struct mdk_personality *pers ;
 dev_t unit ;
 int md_minor ;

 mdk_rdev_t *disks;
 int sb_dirty ;
 int ro ;
 struct gendisk *gendisk ;
 struct kobject kobj ;
 int major_version ;
 int minor_version ;
 int patch_version ;
 int persistent ;
 int chunk_size ;
 time_t ctime ;
 time_t utime ;
 int level ;
 int layout ;
 char clevel[16] ;
 int raid_disks ;
 int max_disks ;
 sector_t size ;
 sector_t array_size ;
 __u64 events ;
 char uuid[16] ;
 struct mdk_thread_s *thread ;
 struct mdk_thread_s *sync_thread ;
 sector_t curr_resync ;
 unsigned long resync_mark ;
 sector_t resync_mark_cnt ;
 sector_t resync_max_sectors ;
 sector_t resync_mismatches ;
 int sync_speed_min ;
 int sync_speed_max ;
 int ok_start_degraded ;
 unsigned long recovery ;
 int in_sync ;
 struct semaphore reconfig_sem ;
 atomic_t active ;
 int changed ;
 int degraded ;
 int barriers_work ;
 struct bio *biolist ;
 atomic_t recovery_active ;
 wait_queue_head_t recovery_wait ;
 sector_t recovery_cp ;
 spinlock_t write_lock ;
 wait_queue_head_t sb_wait ;
 atomic_t pending_writes ;
 unsigned int safemode ;
 unsigned int safemode_delay ;
 struct timer_list safemode_timer ;
 atomic_t writes_pending ;
 request_queue_t *queue ;
 atomic_t write_behind ;
 unsigned int max_write_behind ;
 struct bitmap *bitmap ;
 struct file *bitmap_file ;
 long bitmap_offset ;
 long default_bitmap_offset ;

 struct mddev_s *next;
};
struct mddev_head {
 struct mddev_s *next;
};
struct mdk_personality {
 char *name ;
 int level ;
 struct mdk_personality *next;
 struct module *owner ;
 int (*make_request)(request_queue_t *q , struct bio *bio ) ;
 int (*run)(mddev_t *mddev ) ;
 int (*stop)(mddev_t *mddev ) ;
 void (*status)(struct seq_file *seq , mddev_t *mddev ) ;
 void (*error_handler)(mddev_t *mddev , mdk_rdev_t *rdev ) ;
 int (*hot_add_disk)(mddev_t *mddev , mdk_rdev_t *rdev ) ;
 int (*hot_remove_disk)(mddev_t *mddev , int number ) ;
 int (*spare_active)(mddev_t *mddev ) ;
 sector_t (*sync_request)(mddev_t *mddev , sector_t sector_nr , int *skipped , int go_faster ) ;
 int (*resize)(mddev_t *mddev , sector_t sectors ) ;
 int (*reshape)(mddev_t *mddev , int raid_disks ) ;
 int (*reconfig)(mddev_t *mddev , int layout , int chunk_size ) ;
 void (*quiesce)(mddev_t *mddev , int state ) ;
};
struct mdk_personality_head {
 struct mdk_personality *next;
};
struct md_sysfs_entry {
 struct attribute attr ;
 ssize_t (*show)(mddev_t * , char * ) ;
 ssize_t (*store)(mddev_t * , char * , size_t ) ;
};
struct mdk_thread_s {
 void (*run)(mddev_t *mddev ) ;
 mddev_t *mddev ;
 wait_queue_head_t wqueue ;
 unsigned long flags ;
 struct task_struct *tsk ;
 unsigned long timeout ;
};
typedef struct mdk_thread_s mdk_thread_t;
struct bitmap_page {
 char *map ;
 unsigned int hijacked : 1 ;
 unsigned int count : 31 ;
};
struct bitmap {
 struct bitmap_page *bp ;
 unsigned long pages ;
 unsigned long missing_pages ;
 mddev_t *mddev ;
 int counter_bits ;
 unsigned long chunksize ;
 unsigned long chunkshift ;
 unsigned long chunks ;
 unsigned long syncchunk ;
 __u64 events_cleared ;
 spinlock_t lock ;
 long offset ;
 struct file *file ;
 struct page *sb_page ;
 struct page **filemap ;
 unsigned long *filemap_attr ;
 unsigned long file_pages ;
 unsigned long flags ;
 unsigned long max_write_behind ;
 atomic_t behind_writes ;
 unsigned long daemon_lastrun ;
 unsigned long daemon_sleep ;
 mdk_thread_t *writeback_daemon ;
 spinlock_t write_lock ;
 wait_queue_head_t write_wait ;
 struct list_head complete_pages ;
 mempool_t *write_pool ;
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
struct reclaim_state {
 unsigned long reclaimed_slab ;
};
struct poll_table_struct {
 void (*qproc)(struct file * , wait_queue_head_t * , struct poll_table_struct * ) ;
};
typedef struct poll_table_struct poll_table;
struct fdtable {
 unsigned int max_fds ;
 int max_fdset ;
 int next_fd ;
 struct file **fd ;
 fd_set *close_on_exec ;
 fd_set *open_fds ;
 struct rcu_head rcu ;
 struct files_struct *free_files ;
 struct fdtable *next ;
};
struct files_struct {
 atomic_t count ;
 struct fdtable *fdt ;
 struct fdtable fdtab ;
 fd_set close_on_exec_init ;
 fd_set open_fds_init ;
 struct file *fd_array[32] ;
 spinlock_t file_lock ;
};
struct super_type {
 char *name ;
 struct module *owner ;
 int (*load_super)(mdk_rdev_t *rdev , mdk_rdev_t *refdev , int minor_version ) ;
 int (*validate_super)(mddev_t *mddev , mdk_rdev_t *rdev ) ;
 void (*sync_super)(mddev_t *mddev , mdk_rdev_t *rdev ) ;
};
struct rdev_sysfs_entry {
 struct attribute attr ;
 ssize_t (*show)(mdk_rdev_t * , char * ) ;
 ssize_t (*store)(mdk_rdev_t * , char * , size_t ) ;
};
struct mdstat_info {
 int event ;
};

int bd_claim(struct block_device *x, void *y)
{
 int a; return a;
}
char *__bdevname(dev_t x, char *buffer)
{
 char *a; return a;
}
void bd_release(struct block_device *x)
{
 return;
}
void bitmap_print_sb(struct bitmap *bitmap)
{
 return;
}
void blk_cleanup_queue(request_queue_t *x)
{
 return;
}
void blk_register_region(dev_t dev, unsigned long range, struct module *module, struct kobject *(*probe)(dev_t, int *, void *), int (*lock)(dev_t, void *), void *data)
{
 return;
}
void blk_unregister_region(dev_t dev, unsigned long range)
{
 return;
}
int register_blkdev(unsigned int x, char *y)
{
 int a; return a;
}
int unregister_blkdev(unsigned int x, char *y)
{
 int a; return a;
}
int check_disk_change(struct block_device *x)
{
 int a; return a;
}
void del_gendisk(struct gendisk *gp)
{
 free(gp);
 return;
}
void kobject_init(struct kobject *x)
{
 return;
}
void kobject_del(struct kobject *x)
{
 return;
}
void kobject_unregister(struct kobject *x)
{
 return;
}
void kobject_put(struct kobject *x)
{
 return;
}
void __memzero(void *ptr, __kernel_size_t n)
{
 return;
}
void mutex_lock(struct mutex *lock)
{
 return;
}
void mutex_unlock(struct mutex *lock)
{
 return;
}
int proc_dointvec(ctl_table *x, int y, struct file *z, void *u, size_t *v, loff_t *w)
{
 int a; return a;
}
int register_reboot_notifier(struct notifier_block *x)
{
 int a; return a;
}
int unregister_reboot_notifier(struct notifier_block *x)
{
 int a; return a;
}
loff_t seq_lseek(struct file *x, loff_t y, int z)
{
 loff_t a; return a;
}
ssize_t seq_read(struct file *x, char *y, size_t z, loff_t *u)
{
 ssize_t a; return a;
}
int HsSprintf(char *buf, char *fmt, ...)
{
 int a; return a;
}
int HsPrintk(char *fmt, ...)
{
 int a; return a;
}
int HsStrcmp(char *x, char *y)
{
 int a; return a;
}
char *HsStrchr(char *s, int c)
{
 char *a; return a;
}
size_t HsStrlcpy(char *x, char *y, size_t z)
{
 size_t a; return a;
}
int autoremove_wake_function(wait_queue_t *wait , unsigned int mode , int sync , void *key )
{
 int a; return a;
}

int _atomic_dec_and_lock(atomic_t *atomic, spinlock_t *lock)
{
 int a; return a;
}
int capable(int cap)
{
 int a; return a;
}
void __bug(char *file, int line, void *data)
{
 return;
}

struct page *mem_map ;
struct pglist_data contig_page_data ;
unsigned long jiffies ;

struct __anonstruct_spinlock_t_5 all_mddevs_lock = { {} };





int md_open(struct inode *inode , struct file *file);
int md_release(struct inode *inode , struct file *file);
int md_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);

int md_media_changed(struct gendisk *disk)
{
 int a; return a;
}
int md_revalidate(struct gendisk *disk)
{
 int a; return a;
}
int md_getgeo(struct block_device *bdev , struct hd_geometry *geo)
{
 int a; return a;
}
void md_free(struct kobject *ko)
{
 return;
}

unsigned int mdstat_poll(struct file *filp , poll_table *wait )
{
 unsigned int a; return a;
}
int md_notify_reboot(struct notifier_block *this , unsigned long code , void *x )
{
 int a; return a;
}
int md_seq_open(struct inode *inode , struct file *file )
{
 int a; return a;
}
int md_seq_release(struct inode *inode , struct file *file )
{
 int a; return a;
}

int super_written_barrier(struct bio *bio , unsigned int bytes_done , int error )
{
 int a; return a;
}
int super_written(struct bio *bio , unsigned int bytes_done , int error )
{
 int a; return a;
}

int super_90_load(mdk_rdev_t *rdev , mdk_rdev_t *refdev , int minor_version )
{
 int a; return a;
}
int super_90_validate(mddev_t *mddev , mdk_rdev_t *rdev )
{
 int a; return a;
}
void super_90_sync(mddev_t *mddev , mdk_rdev_t *rdev )
{
 return;
}

int super_1_load(mdk_rdev_t *rdev , mdk_rdev_t *refdev , int minor_version )
{
 int a; return a;
}
int super_1_validate(mddev_t *mddev , mdk_rdev_t *rdev )
{
 int a; return a;
}
void super_1_sync(mddev_t *mddev , mdk_rdev_t *rdev )
{
 return;
}

ssize_t sync_completed_show(mddev_t *mddev , char *page )
{
 ssize_t a; return a;
}

ssize_t level_show(mddev_t *mddev , char *page )
{
 ssize_t a; return a;
}
ssize_t level_store(mddev_t *mddev , char *buf , size_t len )
{
 ssize_t a; return a;
}

ssize_t raid_disks_show(mddev_t *mddev , char *page )
{
 ssize_t a; return a;
}
ssize_t raid_disks_store(mddev_t *mddev , char *buf , size_t len )
{
 ssize_t a; return a;
}

ssize_t chunk_size_show(mddev_t *mddev , char *page )
{
 ssize_t a; return a;
}
ssize_t chunk_size_store(mddev_t *mddev , char *buf , size_t len )
{
 ssize_t a; return a;
}

ssize_t size_show(mddev_t *mddev , char *page )
{
 ssize_t a; return a;
}
ssize_t size_store(mddev_t *mddev , char *buf , size_t len )
{
 ssize_t a; return a;
}

ssize_t metadata_show(mddev_t *mddev , char *page )
{
 ssize_t a; return a;
}
ssize_t metadata_store(mddev_t *mddev , char *buf , size_t len )
{
 ssize_t a; return a;
}

ssize_t null_show(mddev_t *mddev , char *page )
{
 ssize_t a; return a;
}
ssize_t new_dev_store(mddev_t *mddev , char *buf , size_t len )
{
 ssize_t a; return a;
}

ssize_t action_show(mddev_t *mddev , char *page )
{
 ssize_t a; return a;
}
ssize_t action_store(mddev_t *mddev , char *buf , size_t len )
{
 ssize_t a; return a;
}

ssize_t mismatch_cnt_show(mddev_t *mddev , char *page );

ssize_t sync_min_show(mddev_t *mddev , char *page )
{
 ssize_t a; return a;
}
ssize_t sync_min_store(mddev_t *mddev , char *buf , size_t len )
{
 ssize_t a; return a;
}

ssize_t sync_max_show(mddev_t *mddev , char *page )
{
 ssize_t a; return a;
}
ssize_t sync_max_store(mddev_t *mddev , char *buf , size_t len )
{
 ssize_t a; return a;
}

ssize_t sync_speed_show(mddev_t *mddev , char *page )
{
 ssize_t a; return a;
}

ssize_t md_attr_show(struct kobject *kobj , struct attribute *attr , char *page)
{
 ssize_t a; return a;
}
ssize_t md_attr_store(struct kobject *kobj , struct attribute *attr , char *page , size_t length)
{
 ssize_t a; return a;
}

int mdp_major;
int dev_cnt;
int cnt;
int sysctl_speed_limit_min;
int sysctl_speed_limit_max;
int start_readonly;
unsigned int __invalid_size_argument_for_IOC ;
int start_dirty_degraded;

dev_t detected_devices[128];

struct notifier_block md_notifier = {&md_notify_reboot, (struct notifier_block *)((void *)0), 2147483647};

ctl_table raid_table[3] =
{
 {1, "speed_limit_min", (void *)(& sysctl_speed_limit_min), (int)sizeof(int),
  (mode_t)420, (ctl_table *)0, & proc_dointvec, (ctl_handler *)0, (struct proc_dir_entry *)0,
  (void *)0, (void *)0},
 {2, "speed_limit_max", (void *)(& sysctl_speed_limit_max), (int)sizeof(int),
  (mode_t)420, (ctl_table *)0, & proc_dointvec, (ctl_handler *)0, (struct proc_dir_entry *)0,
  (void *)0, (void *)0},
 {0, (char *)0, (void *)0, 0, (unsigned short)0, (ctl_table *)0, (proc_handler *)0,
  (ctl_handler *)0, (struct proc_dir_entry *)0, (void *)0, (void *)0}
};
ctl_table raid_dir_table[2] =
{
 {4, "raid", (void *)0, 0, (mode_t )365, raid_table, (proc_handler *)0, (ctl_handler *)0,
  (struct proc_dir_entry *)0, (void *)0, (void *)0},
 {0, (char *)0, (void *)0, 0, (unsigned short)0, (ctl_table *)0, (proc_handler *)0,
  (ctl_handler *)0, (struct proc_dir_entry *)0, (void *)0, (void *)0}
};
ctl_table raid_root_table[2] =
{
 {7, "dev", (void *)0, 0, (mode_t)365, raid_dir_table, (proc_handler *)0, (ctl_handler *)0,
  (struct proc_dir_entry *)0, (void *)0, (void *)0},
 {0, (char *)0, (void *)0, 0, (unsigned short)0, (ctl_table *)0, (proc_handler *)0,
        (ctl_handler *)0, (struct proc_dir_entry *)0, (void *)0, (void *)0}
};

struct file_operations md_seq_fops =
{
 (struct module *)0,
 &seq_lseek,
 &seq_read, (ssize_t (*)(struct kiocb *, char *, size_t, loff_t))0,
 (ssize_t (*)(struct file *, char *, size_t, loff_t *))0,
 (ssize_t (*)(struct kiocb *, char *, size_t, loff_t))0,
 (int (*)(struct file *, void *, int (*)(void *, char *, int, loff_t, ino_t, unsigned int)))0,
 &mdstat_poll,
 (int (*)(struct inode *, struct file *, unsigned int, unsigned long))0,
 (long (*)(struct file *, unsigned int, unsigned long))0,
 (long (*)(struct file *, unsigned int, unsigned long))0,
 (int (*)(struct file *, struct vm_area_struct *))0,
 &md_seq_open, (int (*)(struct file *))0,
 &md_seq_release, (int (*)(struct file * , struct dentry *, int datasync))0,
 (int (*)(struct kiocb *, int datasync))0, (int (*)(int, struct file *, int))0,
 (int (*)(struct file *, int, struct file_lock *))0,
 (ssize_t (*)(struct file *, struct iovec *, unsigned long, loff_t *))0,
 (ssize_t (*)(struct file *, struct iovec *, unsigned long, loff_t *))0,
 (ssize_t (*)(struct file *, loff_t *, size_t,
  int (*)(read_descriptor_t *, struct page *, unsigned long, unsigned long), void *))0,
 (ssize_t (*)(struct file *, struct page *, int, size_t, loff_t *, int))0,
 (unsigned long (*)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long))0,
 (int (*)(int))0, (int (*)(struct file *filp, unsigned long arg))0,
 (int (*)(struct file *, int, struct file_lock *))0
};


struct kobj_type rdev_ktype;

struct super_type super_types[2] =
{
 {(char *)"0.90.0", (struct module *)0, & super_90_load, & super_90_validate, & super_90_sync},
 {(char *)"md-1", (struct module *)0, & super_1_load, & super_1_validate, & super_1_sync}
};

struct md_sysfs_entry md_sync_completed =
{
 {"sync_completed", (struct module *)0, (mode_t )292},
 &sync_completed_show,
 (ssize_t (*)(mddev_t *, char *, size_t))0
};
struct md_sysfs_entry md_level =
{
 {"level", (struct module *)0, (mode_t )420},
 & level_show, & level_store
};
struct md_sysfs_entry md_raid_disks =
{
 {"raid_disks", (struct module *)0, (mode_t )420},
 & raid_disks_show, & raid_disks_store
};
struct md_sysfs_entry md_chunk_size =
{
 {"chunk_size", (struct module *)0, (mode_t )420},
 & chunk_size_show, & chunk_size_store
};
struct md_sysfs_entry md_size =
{
 {"component_size", (struct module *)0, (mode_t )420},
 & size_show, & size_store
};
struct md_sysfs_entry md_metadata =
{
 {"metadata_version", (struct module *)0, (mode_t )420},
 & metadata_show, & metadata_store
};
struct md_sysfs_entry md_new_device =
{
 {"new_dev", (struct module *)0, (mode_t )128},
 & null_show, & new_dev_store
};
struct md_sysfs_entry md_scan_mode =
{
 {"sync_action", (struct module *)0, (mode_t )420},
 & action_show, & action_store
};
struct md_sysfs_entry md_mismatches =
{
 {"mismatch_cnt", (struct module *)0, (mode_t )292},
 & mismatch_cnt_show, (ssize_t (*)(mddev_t *, char *, size_t))0
};
struct md_sysfs_entry md_sync_min =
{
 {"sync_speed_min", (struct module *)0, (mode_t )420},
 & sync_min_show, & sync_min_store
};
struct md_sysfs_entry md_sync_max =
{
 {"sync_speed_max", (struct module *)0, (mode_t )420},
 & sync_max_show, & sync_max_store
};
struct md_sysfs_entry md_sync_speed =
{
 {"sync_speed", (struct module *)0, (mode_t )292},
 & sync_speed_show, (ssize_t (*)(mddev_t *, char *, size_t))0
};

struct attribute *md_default_attrs[7] =
{
 &md_level.attr, &md_raid_disks.attr, &md_chunk_size.attr, &md_size.attr,
 &md_metadata.attr, &md_new_device.attr, (struct attribute *)((void *)0)
};
struct attribute *md_redundancy_attrs[7] =
{
 &md_scan_mode.attr, &md_mismatches.attr, &md_sync_min.attr, &md_sync_max.attr,
 &md_sync_speed.attr, &md_sync_completed.attr, (struct attribute *)((void *)0)
};
struct attribute_group md_redundancy_group =
{
 (char *)((void *)0), md_redundancy_attrs
};

struct __wait_queue_head md_event_waiters =
{
 {{}},
 {& md_event_waiters.task_list, & md_event_waiters.task_list}
};

struct semaphore disks_sem =
{
 {(int )1},
 0,
 {{{}}, {& disks_sem.wait.task_list, & disks_sem.wait.task_list}}
};

struct block_device_operations md_fops =
{
 & md_open, & md_release, & md_ioctl,
 (long (*)(struct file * , unsigned int , unsigned long ))0,
 (long (*)(struct file * , unsigned int , unsigned long ))0,
 (int (*)(struct block_device * , sector_t , unsigned long * ))0,
 & md_media_changed,
 & md_revalidate,
 & md_getgeo,
 (struct module *)0
};

struct sysfs_ops md_sysfs_ops =
{
 & md_attr_show,
 & md_attr_store
};

struct kobj_type md_ktype =
{
 & md_free,
 & md_sysfs_ops,
 md_default_attrs
};

struct ctl_table_header *raid_table_header;

struct mdk_rdev_header pending_raid_disks;
struct mdk_personality_head pers_list;
struct mddev_head all_mddevs;
struct timespec xtime ;
atomic_t md_event_count;

struct proc_dir_entry *HsProc;
struct mdk_thread_s HsThread;
struct thread_info *HsThreadInfo;
struct module HsModule;

struct gendisk *HsCreateGendisk(void)
{
 struct gendisk *disk;
 disk = (struct gendisk *)malloc(sizeof(struct gendisk));
 return disk;
}

void HsFreeGendisk(struct gendisk *disk)
{
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

mdk_rdev_t *HsCreateRdev(void)
{
 mdk_rdev_t *rdev;
 rdev = (mdk_rdev_t *)malloc(sizeof(mdk_rdev_t));
 rdev->bdev = HsCreateBdev();
 rdev->sb_page = (struct page *)malloc(sizeof(struct page));
 rdev->disks = 0;
 return rdev;
}

void HsFreeRdev(mdk_rdev_t *rdev)
{
 if (rdev->bdev != 0) HsFreeBdev(rdev->bdev);
 if (rdev->sb_page != 0) free(rdev->sb_page);
 free(rdev);
 return;
}

struct inode *HsCreateInode(void)
{
 struct inode *i;
 i = (struct inode *)malloc(sizeof(struct inode));
 i->i_bdev = HsCreateBdev();
 return i;
}

void HsFreeInode(struct inode *i)
{
 if (i->i_bdev != 0) HsFreeBdev(i->i_bdev);
 free(i);
 return;
}

struct file *HsCreateFile(void)
{
 struct file *f;
 f = (struct file *)malloc(sizeof(struct file));
 f->f_mapping = (struct address_space *)malloc(sizeof(struct address_space));
 f->f_mapping->host = (struct inode *)malloc(sizeof(struct inode));
 f->f_dentry = (struct dentry *)malloc(sizeof(struct dentry));
 f->f_dentry->d_inode = (struct inode *)malloc(sizeof(struct inode));
 return f;
}


void HsFreeFile(struct file *f)
{
 if (f->f_mapping != 0) {
  if (f->f_mapping->host != 0) {
   free(f->f_mapping->host);
  }
  free(f->f_mapping);
 }
 if (f->f_dentry != 0) {
  if (f->f_dentry->d_inode != 0) {
   free(f->f_dentry->d_inode);
  }
  free(f->f_dentry);
 }
 free(f);
 return;
}

mddev_t *HsCreateDev(void)
{
 mddev_t *dev;
 struct bio *bio;
 struct mdk_personality *pers;

 dev = (mddev_t *)malloc(sizeof(mddev_t));
 dev->queue = (request_queue_t *)malloc(sizeof(request_queue_t));
 dev->bitmap = 0;
 dev->sync_thread = (struct mdk_thread_s *)malloc(sizeof(struct mdk_thread_s));
 dev->gendisk = HsCreateGendisk();
 dev->bitmap_file = HsCreateFile();

 dev->disks = (mdk_rdev_t *)dev;
 dev->next = dev;
 dev->default_bitmap_offset = dev->bitmap_offset;
 dev->ok_start_degraded = start_dirty_degraded;

 (dev->queue)->queuedata = (void *)dev;
 dev->thread = &HsThread;

 dev->biolist = 0;
 while ((&nondet < 0)) {
  bio = (struct bio*)malloc(sizeof(struct bio));
  bio->bi_next = dev->biolist;
  dev->biolist = bio;
 }

 dev->pers = 0;
 dev->pers = (struct mdk_personality *)malloc(sizeof(struct mdk_personality));
 dev->pers->owner = &HsModule;

 while (1) { break; }
 return dev;
}

mddev_t *HsCreateDevGeneral(void)
{
 mddev_t *dev;
 struct bio *bio;
 struct mdk_personality *pers;

 dev = (mddev_t *)malloc(sizeof(mddev_t));
 dev->bitmap = 0;
 dev->queue = (request_queue_t *)malloc(sizeof(request_queue_t));
 dev->sync_thread = (struct mdk_thread_s *)malloc(sizeof(struct mdk_thread_s));

 if ((&nondet < 0)) {
  dev->gendisk = HsCreateGendisk();
 } else {
  dev->gendisk = 0;
 }
 if ((&nondet < 0)) {
  dev->bitmap_file = HsCreateFile();
 } else {
  dev->bitmap_file = 0;
 }
 if ((&nondet < 0)) {
  dev->pers = (struct mdk_personality *)malloc(sizeof(struct mdk_personality));
  dev->pers->owner = &HsModule;
 } else {
  dev->pers = 0;
 }

 dev->disks = (mdk_rdev_t *)dev;
 dev->next = dev;
 dev->default_bitmap_offset = dev->bitmap_offset;
 dev->ok_start_degraded = start_dirty_degraded;

 (dev->queue)->queuedata = (void *)dev;
 dev->thread = &HsThread;

 dev->biolist = 0;
 while ((&nondet < 0)) {
  bio = (struct bio*)malloc(sizeof(struct bio));
  bio->bi_next = dev->biolist;
  dev->biolist = bio;
 }

 while(1) { break; }
 return dev;
}

mddev_t *HsCreateDevNull(void)
{
 if ((&nondet < 0)) {
  return (HsCreateDev());
 } else {
  return 0;
 }
}

void HsFreeDev(mddev_t *dev)
{
 struct bio *bio;
 mdk_rdev_t *rdev;

 if (dev->pers != 0) free(dev->pers);
 if (dev->queue != 0) free(dev->queue);
 if (dev->bitmap != 0) free(dev->bitmap);
 if (dev->sync_thread != 0) free(dev->sync_thread);
 if (dev->gendisk != 0) HsFreeGendisk(dev->gendisk);
 if (dev->bitmap_file != 0) HsFreeFile(dev->bitmap_file);

 while (dev->biolist) {
  bio = dev->biolist;
  dev->biolist = bio->bi_next;
  free(bio);
 }
 while (dev->disks != (mdk_rdev_t *)dev) {
  rdev = dev->disks;
  dev->disks = rdev->disks;
  HsFreeRdev(rdev);
 }

 free(dev);
 return;
}

ssize_t mismatch_cnt_show(mddev_t *mddev , char *page )
{
 ssize_t tmp ;
 tmp = HsSprintf(page, "%llu\n", (unsigned long long )mddev->resync_mismatches);
 return (tmp);
}

struct block_device *bdget(dev_t x)
{
 struct block_device *a;
 a = HsCreateBdev();
 return a;
}

void bdput(struct block_device *x)
{
 HsFreeBdev(x);
 return;
}

void blkdev_put(struct block_device *x)
{
 HsFreeBdev(x);
 return;
}

void put_disk(struct gendisk *disk)
{
 return;
}

void put_page(struct page *page)
{
 free(page);
 return;
}

unsigned long __arch_copy_from_user(void *to, void *from, unsigned long n)
{
 unsigned long a; return a;
}

unsigned long __arch_copy_to_user(void *to, void *from, unsigned long n)
{
 unsigned long a; return a;
}

struct proc_dir_entry *create_proc_entry(char *name, mode_t mode, struct proc_dir_entry *parent)
{
 struct proc_dir_entry* res;
 res = mallocnull(sizeof(struct proc_dir_entry));
 return res;
}

void remove_proc_entry(char *name, struct proc_dir_entry *parent)
{
 if (HsProc != 0) free(HsProc);
 return;
}

struct ctl_table_header *register_sysctl_table(ctl_table *table , int insert_at_head)
{
        struct ctl_table_header *tmp ;
        tmp = (struct ctl_table_header *)mallocnull(sizeof(struct ctl_table_header ));

        if (tmp == 0) return tmp;

        tmp->ctl_table = table;
        return tmp;
}

void md_geninit(void)
{
 struct proc_dir_entry *p ;
 p = create_proc_entry("mdstat", (unsigned short)292, (struct proc_dir_entry *)((void *)0));
 if (p) {
  HsProc = p;
  p->proc_fops = & md_seq_fops;
 }
 return;
}

void prefetch(void *x)
{
 return;
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

mddev_t *mddev_get(mddev_t *mddev)
{
 atomic_add_return(1, &mddev->active);
 return (mddev);
}

void init_waitqueue_head(wait_queue_head_t *q )
{
 return;
}

void sema_init(struct semaphore *sem , int val )
{
 sem->count.counter = (int )val;
 sem->sleepers = 0;
 init_waitqueue_head(& sem->wait);
 return;
}

void init_MUTEX(struct semaphore *sem )
{
 sem->count.counter = (int )1;
 sem->sleepers = 0;
 init_waitqueue_head(& sem->wait);
 return;
}

void init_timer(struct timer_list *timer)
{
 return;
}

request_queue_t *blk_alloc_queue(gfp_t x)
{
 request_queue_t *a; return a;
}

void blk_queue_make_request(request_queue_t *x, make_request_fn *y)
{
 return;
}

void bio_endio(struct bio *x, unsigned int y, int z)
{
 return;
}

int md_fail_request(request_queue_t *q , struct bio *bio )
{
 bio_endio(bio, bio->bi_size, -5);
 return (0);
}

mddev_t *mddev_find(dev_t unit)
{
 mddev_t *mddev ;
 mddev_t *new ;
 mddev_t *__mptr ;
 mddev_t *__mptr___0 ;
 spinlock_t __r_expr_0 ;

 new = (mddev_t *)((void *)0);
retry:
 mddev = (mddev_t *)all_mddevs.next;
 while (1) {
  prefetch((void *)mddev->next);
  if (mddev == (mddev_t *)&all_mddevs) {
   break;
  }
  if (mddev->unit == unit) {
   mddev_get(mddev);

   // BUG: 
   // Null pointer dereference error
   // kfree((void  *)new);
   if (new) HsFreeDev(new);

   return (mddev);
  }
  mddev = mddev->next;
 }
 if (new) {
  new->next = all_mddevs.next;
  all_mddevs.next = new;
  return (new);
 }

 new = HsCreateDevNull();
 if (! new) {
  return ((mddev_t *)((void *)0));
 }
 new->unit = unit;
 if (unit >> 20 == 9U) {
  new->md_minor = (int )(unit & 1048575U);
 } else {
  new->md_minor = (int )((unit & 1048575U) >> 6);
 }
 init_MUTEX(& new->reconfig_sem);
 new->disks = new;
 new->next = new;
 init_timer(& new->safemode_timer);
 new->active.counter = (int )1;
 while (1) {


  break;
 }
 init_waitqueue_head(& new->sb_wait);


 if (! new->queue) {

  HsFreeDev(new);
  return ((mddev_t *)((void *)0));
 }
 blk_queue_make_request(new->queue, & md_fail_request);
 goto retry;
}

void down(struct semaphore *sem )
{
# 3460 "amd.c"
 return;
}

void up(struct semaphore* disks_sem)
{
 return;
}

void mddev_put(mddev_t *mddev);

struct gendisk *alloc_disk(int minors)
{
 if ((&nondet < 0)) {
  return (HsCreateGendisk());
 } else {
  return 0;
 }
}

void add_disk(struct gendisk *disk)
{
 return;
}

int kobject_register(struct kobject *x)
{
 int a; return a;
}

int HsSnprintf(char *buf, size_t size, char *fmt, ...)
{
 int a; return a;
}

struct kobject *md_probe(dev_t dev , int *part , void *data )
{
 mddev_t *mddev ;
 mddev_t *tmp ;
 struct gendisk *disk ;
 int partitioned ;
 int shift ;
 int tmp___0 ;
 int unit ;

 tmp = mddev_find(dev);
 mddev = tmp;
 partitioned = dev >> 20 != 9U;
 if (partitioned) {
  tmp___0 = 6;
 } else {
  tmp___0 = 0;
 }
 shift = tmp___0;
 unit = (int )((dev & 1048575U) >> shift);
 if (! mddev) {
  return ((struct kobject *)((void *)0));
 }
 down(& disks_sem);
 if (mddev->gendisk) {
  up(& disks_sem);
  mddev_put(mddev);
  return ((struct kobject *)((void *)0));
 }
 disk = alloc_disk(1 << shift);
 if (! disk) {
  up(& disks_sem);
  mddev_put(mddev);
  return ((struct kobject *)((void *)0));
 }
 disk->major = (int )(dev >> 20);
 disk->first_minor = unit << shift;
 if (partitioned) {
  HsSprintf(disk->disk_name, "md_d%d", unit);
  HsSprintf(disk->devfs_name, "md/d%d", unit);
 } else {
  HsSprintf(disk->disk_name, "md%d", unit);
  HsSprintf(disk->devfs_name, "md/%d", unit);
 }
 disk->fops = & md_fops;
 disk->private_data = (void *)mddev;
 disk->queue = mddev->queue;
 add_disk(disk);
 mddev->gendisk = disk;
 up(&disks_sem);
 mddev->kobj.parent = & disk->kobj;
 mddev->kobj.k_name = (char *)((void *)0);
 HsSnprintf(mddev->kobj.name, 20U, "%s", "md");
 mddev->kobj.ktype = & md_ktype;
 kobject_register(& mddev->kobj);
 return ((struct kobject *)((void *)0));
}

int devfs_mk_bdev(dev_t dev , umode_t mode , char *fmt , ...)
{
 return (0);
}

int devfs_mk_dir(char *fmt, ...)
{
 return (0);
}

int md_init(void)
{
 int minor ;
 int tmp ;

 HsPrintk("<6>md: md driver %d.%d.%d MAX_MD_DEVS=%d, MD_SB_DISKS=%d\n", 0, 90, 3, 256, 27);
 HsPrintk("<6>md: bitmap version %d.%d\n", 4, 39);
 tmp = register_blkdev(9U, "md");
 if (tmp) {
  return (-1);
 }
 mdp_major = register_blkdev(0U, "mdp");
 if (mdp_major <= 0) {
  unregister_blkdev(9U, "md");
  return (-1);
 }
 devfs_mk_dir("md");
 blk_register_region(9437184U, 256UL, (struct module *)0, & md_probe,
     (int (*)(dev_t, void *))((void *)0),
     (void *)0);
 blk_register_region((unsigned int )(mdp_major << 20), 16384UL, (struct module *)0, & md_probe,
      (int (*)(dev_t, void *))((void *)0),
      (void *)0);
 minor = 0;
 while (minor < 256) {
  devfs_mk_bdev((unsigned int )(9437184 | minor),
    (unsigned short)24960, "md/%d", minor);
  minor ++;
 }
 minor = 0;
 while (minor < 256) {
  devfs_mk_bdev((unsigned int )((mdp_major << 20) | (minor << 6)),
    (unsigned short)24960, "md/mdp%d", minor);
  minor ++;
 }
 register_reboot_notifier(&md_notifier);
 raid_table_header = register_sysctl_table(raid_root_table, 1);
 md_geninit();
 return (0);
}





void mddev_put(mddev_t *mddev)
{
 int tmp ;
 tmp = _atomic_dec_and_lock(&mddev->active, &all_mddevs_lock);
 if (!tmp) {
  return;
 }
 if (!mddev->raid_disks) {
  if (mddev->disks == (mdk_rdev_t *)mddev) {
   blk_cleanup_queue(mddev->queue);
   kobject_unregister(& mddev->kobj);
  }
 }
 return;
}

int mddev_put_remove(mddev_t *prev, mddev_t *mddev)
{
 int tmp ;
 tmp = _atomic_dec_and_lock(&mddev->active, &all_mddevs_lock);
 if (!tmp) {
  return 0;
 }
 if (!mddev->raid_disks && mddev->disks == (mdk_rdev_t *)mddev) {
  prev->next = mddev->next;
  return 1;
 }
 return 0;
}

int __test_bit(int nr , unsigned long *p)
{
 return ((int )(((*(p + (nr >> 5))) >> (nr & 31)) & 1UL));
}

void *lowmem_page_address(struct page *page)
{
 return ((void *)((((unsigned long )(page - mem_map) << 12) - 0UL) + 3221225472UL));
}

void print_sb(mdp_super_t *sb)
{
 return;
}

char *bdevname(struct block_device *bdev , char *buffer)
{
 char *a; return a;
}

void print_rdev(mdk_rdev_t *rdev )
{
 char b[32] ;
 int tmp ;
 int tmp___0 ;
 char *tmp___1 ;
 void *tmp___2 ;

 tmp = __test_bit(2, (unsigned long *)(& rdev->flags));
 tmp___0 = __test_bit(1, (unsigned long *)(& rdev->flags));
 tmp___1 = bdevname(rdev->bdev, b);
 HsPrintk("<6>md: rdev %s, SZ:%08llu F:%d S:%d DN:%u\n", tmp___1, (unsigned long long )rdev->size,
     tmp___0, tmp, rdev->desc_nr);

 if ((&nondet < 0)) {
  HsPrintk("<6>md: rdev superblock:\n");
  tmp___2 = lowmem_page_address(rdev->sb_page);
  print_sb((mdp_super_t *)tmp___2);
 } else {
  HsPrintk("<6>md: no rdev superblock!\n");
 }
 return;
}

char *mdname(mddev_t *mddev )
{
 char *tmp ;
 if (mddev->gendisk) {
  tmp = (char *)((mddev->gendisk)->disk_name);
 } else {
  tmp = "mdX";
 }
 return ((char *)tmp);
}


void md_print_devices(void)
{
 mdk_rdev_t *rdev, *tmp2;
 mddev_t *mddev, *tmp;
 char b[32] ;
 char *tmp___0 ;
 char *tmp___1 ;

 int removed;

 HsPrintk("\n");
 HsPrintk("md:\t**********************************\n");
 HsPrintk("md:\t* <COMPLETE RAID STATE PRINTOUT> *\n");
 HsPrintk("md:\t**********************************\n");
 tmp = all_mddevs.next;
 mddev = (mddev_t *)((void *)0);
 while (1) {
  if ((unsigned int )tmp != (unsigned int )(&all_mddevs)) {
   mddev_get(tmp);
  }
  if (mddev) {
   mddev_put(mddev);
  }
  mddev = tmp;
  if ((unsigned int )tmp == (unsigned int )(& all_mddevs)) {
   break;
  }
  if (mddev->bitmap) {
   bitmap_print_sb(mddev->bitmap);
  } else {
   tmp___0 = mdname(mddev);
   HsPrintk("%s: ", tmp___0);
  }
  tmp2 = mddev->disks;
  while (1) {
   rdev = tmp2;
   tmp2 = tmp2->disks;
   if ((unsigned int )tmp2 == (unsigned int )mddev->disks) {
    break;
   }
   tmp___1 = bdevname(rdev->bdev, b);
   HsPrintk("<%s>", tmp___1);
  }
  HsPrintk("\n");
  tmp2 = mddev->disks;
  while (1) {
   rdev = tmp2;
   tmp2 = tmp2->disks;
   if ((unsigned int )tmp2 == (unsigned int )mddev->disks) {
    break;
   }
   print_rdev(rdev);
  }
  tmp = tmp->next;
 }
 HsPrintk("md:\t**********************************\n");
 HsPrintk("\n");
 return;
}

void sysfs_remove_link(struct kobject *x, char *name)
{
 return;
}

void unbind_rdev_from_array(mdk_rdev_t *rdev)
{
 char b[32] ;
 char *tmp ;

 if (!rdev->mddev) {
  HsPrintk("md: bug in file %s, line %d\n", "md.c", 1308);
  md_print_devices();
  return;
 }
 tmp = bdevname(rdev->bdev, b);
 HsPrintk("<6>md: unbind<%s>\n", tmp);
 rdev->mddev = (mddev_t *)((void *)0);
 sysfs_remove_link(&rdev->kobj, "block");
 kobject_del(&rdev->kobj);
 return;
}

void free_disk_sb(mdk_rdev_t *rdev )
{
 if (rdev->sb_page) {
  put_page(rdev->sb_page);

  rdev->sb_loaded = get_nondet_int();
  rdev->sb_page = (struct page *)((void *)0);
  rdev->sb_offset = 0UL;
  rdev->size = 0UL;
 }
 return;
}

void md_autodetect_dev(dev_t dev)
{
 int tmp ;
 if (dev_cnt >= 0) {
  if (dev_cnt < 127) {
   tmp = dev_cnt;
   dev_cnt ++;
   detected_devices[tmp] = dev;
  }
 }
 return;
}

void unlock_rdev(mdk_rdev_t *rdev )
{
 struct block_device *bdev ;
 bdev = rdev->bdev;
 rdev->bdev = (struct block_device *)((void *)0);
 if (! bdev) {
  HsPrintk("md: bug in file %s, line %d\n", "md.c", 1351);
  md_print_devices();
 }
 bd_release(bdev);
 blkdev_put(bdev);
 return;
}

void export_rdev_sl(mdk_rdev_t *prev, mdk_rdev_t *rdev)
{
 char b[32];
 char *tmp;

 tmp = bdevname(rdev->bdev, b);
 HsPrintk("<6>md: export_rdev(%s)\n", tmp);
 if (rdev->mddev) {
  HsPrintk("md: bug in file %s, line %d\n", "md.c", 1364);
  md_print_devices();
 }
 prev->disks = rdev->disks;
 rdev->disks = rdev;
 free_disk_sb(rdev);
 md_autodetect_dev((rdev->bdev)->bd_dev);
 unlock_rdev(rdev);
 kobject_put(& rdev->kobj);


 free(rdev);
 return;
}

void kick_rdev_from_array_sl(mdk_rdev_t *prev, mdk_rdev_t *rdev)
{
 unbind_rdev_from_array(rdev);
 export_rdev_sl(prev,rdev);
 return;
}

void export_array(mddev_t *mddev)
{
 mdk_rdev_t *rdev, *tmp, *prev;
 int tmp___0 ;

 rdev = (mdk_rdev_t *)mddev;
 prev = rdev;
 tmp = mddev->disks;
 while (1) {
  rdev = tmp;
  tmp = tmp->disks;
  if ((unsigned int )tmp == (unsigned int )(mddev->disks)) {
   break;
  }
  if (!rdev->mddev) {
   HsPrintk("md: bug in file %s, line %d\n", "md.c", 1387);
   md_print_devices();
   prev = rdev;
   goto __Cont;
  }
  kick_rdev_from_array_sl(prev,rdev);
__Cont:
  ;
 }
 if (mddev->disks == (mdk_rdev_t *)mddev) {
  HsPrintk("md: bug in file %s, line %d\n", "md.c", 1393);
  md_print_devices();
 }
 mddev->raid_disks = 0;
 mddev->major_version = 0;
 return;
}

void unregister_sysctl_table(struct ctl_table_header *table_header) {
 return;
}

void devfs_remove(char *fmt, ...)
{
 return;
}

void md_exit(void)
{
 mddev_t *mddev;
 mddev_t *tmp, *prev;
 int i;
 int removed;
 struct gendisk *disk;

 blk_unregister_region(9437184U, 256UL);
 blk_unregister_region((unsigned int )(mdp_major << 20), 16384UL);
 i = 0;
 while (i < 256) {
  devfs_remove("md/%d", i);
  i ++;
 }
 i = 0;
 while (i < 256) {
  devfs_remove("md/d%d", i);
  i ++;
 }
 devfs_remove("md");
 unregister_blkdev(9U, "md");
 unregister_blkdev((unsigned int )mdp_major, "mdp");
 unregister_reboot_notifier(&md_notifier);
 unregister_sysctl_table(raid_table_header);
 remove_proc_entry("mdstat", (struct proc_dir_entry *)((void *)0));
 tmp = all_mddevs.next;
 prev = (mddev_t *)&all_mddevs;
 mddev = (mddev_t *)((void *)0);
 while (1) {
  if ((unsigned int )tmp != (unsigned int )(&all_mddevs)) {
   mddev_get(tmp);
  }
  if (mddev) {
   mddev_put(mddev);


   removed = mddev_put_remove(prev,mddev);
   if (removed) {
    HsFreeDev(mddev);
   }
   else {
    prev = mddev;
   }
  }
  mddev = tmp;
  if ((unsigned int )tmp == (unsigned int )(& all_mddevs)) {
   break;
  }
  disk = mddev->gendisk;
  if (!disk) {
   goto __Cont;
  }
  export_array(mddev);
  del_gendisk(disk);
  put_disk(disk);
  mddev->gendisk = (struct gendisk *)((void *)0);
  mddev_put(mddev);

__Cont:
  tmp = tmp->next;
 }
 return;
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

void __wake_up(wait_queue_head_t *q, unsigned int mode, int nr, void *key)
{
 return;
}


void md_wakeup_thread(mdk_thread_t *thread)
{
 if (thread) {
  ____atomic_set_bit(0U, (unsigned long *)(& thread->flags));
  __wake_up(& thread->wqueue, 3U, 1, (void *)0);
 }
 return;
}

void mddev_unlock(mddev_t *mddev)
{
 up(&mddev->reconfig_sem);
 md_wakeup_thread(mddev->thread);
 return;
}

struct thread_info *current_thread_info(void)
{
 return HsThreadInfo;



}

unsigned long copy_to_user(void *to, void *from, unsigned long n)
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

int get_version(void *arg)
{
 mdu_version_t ver ;
 unsigned long tmp ;

 ver.major = 0;
 ver.minor = 90;
 ver.patchlevel = 3;
 tmp = copy_to_user(arg, (void *)(& ver), (unsigned long)sizeof(ver));
 if (tmp) {
  return (-14);
 }
 return (0);
}

int gfp_zone(gfp_t gfp )
{
 int zone ;
 long tmp ;

 zone = 7 & (int )gfp;
 while (1) {
  tmp = get_nondet_int();
  if (tmp) {
   __bug("../../include/linux/gfp.h", 81, (void *)0);
  }
  break;
 }
 return (zone);
}

struct page *__alloc_pages(gfp_t x, unsigned int y, struct zonelist *z)
{
 struct page *p = (struct page *)mallocnull(sizeof(struct page));
 return p;
}

struct page *alloc_pages_node(int nid, gfp_t gfp_mask, unsigned int order)
{
 long tmp ;
 int tmp___0 ;
 struct page *tmp___1 ;

 tmp = get_nondet_int();
 if (tmp) {
  return ((struct page *)((void *)0));
 }
 if (nid < 0) {
  nid = 0;
 }
 tmp___0 = gfp_zone(gfp_mask);
 tmp___1 = __alloc_pages(gfp_mask, order, contig_page_data.node_zonelists + tmp___0);
 return (tmp___1);
}

int alloc_disk_sb(mdk_rdev_t *rdev)
{
 if (rdev->sb_page) {
  HsPrintk("md: bug in file %s, line %d\n", "md.c", 350);
  md_print_devices();
 }
 rdev->sb_page = alloc_pages_node(0, 208U, 0U);
 if (!rdev->sb_page) {
  HsPrintk("<1>md: out of memory.\n");
  return (-22);
 }
 return (0);
}

struct block_device *open_by_devnum(dev_t x, unsigned int y)
{
 struct block_device *a;
 if ((&nondet < 0)) {
  a = HsCreateBdev();
  return a;
 }
 else {
  return 0;
 }
}

int lock_rdev(mdk_rdev_t *rdev, dev_t dev)
{
 int err ;
 struct block_device *bdev ;
 char b[32] ;
 char *tmp ;
 int tmp___0 ;
 char *tmp___2 ;

 err = 0;
 bdev = open_by_devnum(dev, 3U);
 if (!bdev) {
  tmp = __bdevname(dev, b);
  HsPrintk("<3>md: could not open %s.\n", tmp);




  return 1;
 }
 bdev->bd_inode->i_size = 0;
 err = bd_claim(bdev, (void *)rdev);
 if (err) {
  tmp___2 = bdevname(bdev, b);
  HsPrintk("<3>md: could not bd_claim %s.\n", tmp___2);
  blkdev_put(bdev);
  return 1;
 }
 rdev->bdev = bdev;
 return (err);
}

mdk_rdev_t *md_import_device(dev_t newdev, int super_format, int super_minor)
{
 char b[32] ;
 int err ;
 mdk_rdev_t *rdev ;
 sector_t size ;
 mdk_rdev_t *tmp ;
 char *tmp___0 ;
 char *tmp___1 ;
 char *tmp___2 ;
 mdk_rdev_t *tmp___3 ;


 rdev = (mdk_rdev_t *)mallocnull(sizeof((*rdev)));

 if (!rdev) {
  HsPrintk("<3>md: could not alloc mem for new device!\n");

  return (0);
 }



 rdev->sb_page = 0;
 rdev->bdev = 0;

 err = alloc_disk_sb(rdev);
 if (err) {
  goto abort_free;
 }
 err = lock_rdev(rdev, newdev);
 if (err) {
  goto abort_free;
 }
 rdev->kobj.parent = (struct kobject *)((void *)0);
 rdev->kobj.ktype = & rdev_ktype;
 kobject_init(&rdev->kobj);
 rdev->desc_nr = -1;
 rdev->flags = 0UL;
 rdev->data_offset = 0UL;
 rdev->nr_pending.counter = (int )0;
 rdev->read_errors.counter = (int )0;
 rdev->corrected_errors.counter = (int )0;
 size = (unsigned long )(((rdev->bdev)->bd_inode)->i_size >> 10);
 if (! size) {
  tmp___0 = bdevname(rdev->bdev, b);
  HsPrintk("<4>md: %s has zero or unknown size, marking faulty!\n", tmp___0);
  err = -22;
  goto abort_free;
 }
 if (super_format >= 0) {

  err = get_nondet_int();
  if (err == -22) {
   tmp___1 = bdevname(rdev->bdev, b);
   HsPrintk("<4>md: %s has invalid sb, not importing!\n", tmp___1);
   goto abort_free;
  }
  if (err < 0) {
   tmp___2 = bdevname(rdev->bdev, b);
   HsPrintk("<4>md: could not read %s\'s sb, not importing!\n", tmp___2);
   goto abort_free;
  }
 }
 rdev->disks = (mdk_rdev_t *)rdev;
 return (rdev);
abort_free:
 if (rdev->sb_page) {
  if (rdev->bdev) {


   unlock_rdev(rdev);
  }
  free_disk_sb(rdev);
 }
 free((void *)rdev);

 tmp___3 = (mdk_rdev_t *)0;
 return (tmp___3);
}

void autorun_devices(int x)
{
 return;
}

void autostart_arrays(int part)
{
 mdk_rdev_t *rdev ;
 int i ;
 dev_t dev ;
 long tmp ;
 int tmp___0 ;

 HsPrintk("<6>md: Autodetecting RAID arrays.\n");
 i = 0;
 while (i < dev_cnt) {
  dev = detected_devices[i];
  rdev = md_import_device(dev, 0, 0);
  if (!rdev) {
   goto __Cont;
  }
  tmp___0 = __test_bit(1, (unsigned long *)(& rdev->flags));
  if (tmp___0) {
   HsPrintk("md: bug in file %s, line %d\n", "md.c", 4969);
   md_print_devices();


   unlock_rdev(rdev);
   free_disk_sb(rdev);
   free(rdev);

   goto __Cont;
  }
  rdev->disks = pending_raid_disks.disks;
  pending_raid_disks.disks = rdev;
__Cont:
      i ++;
 }
 dev_cnt = 0;
 autorun_devices(part);
 return;
}

struct task_struct *get_current(void)
{
 struct thread_info *tmp ;
 tmp = current_thread_info();
 return (tmp->task);
}

dev_t new_decode_dev(u32 dev)
{
 unsigned int major ;
 unsigned int minor ;

 major = (dev & 1048320U) >> 8;
 minor = (dev & 255U) | ((dev >> 12) & 1048320U);
 return ((major << 20) | minor);
}

int down_interruptible(struct semaphore *sem )
{
 unsigned int ret;
# 4290 "amd.c"
 return ((int )ret);
}

int mddev_lock(mddev_t *mddev)
{
 int tmp;
 tmp = down_interruptible(& mddev->reconfig_sem);
 return (tmp);
}

sector_t get_capacity(struct gendisk *disk )
{
 return (disk->capacity);
}

struct block_device *bdget_disk(struct gendisk *disk , int index )
{
 struct block_device *tmp ;
 tmp = bdget((unsigned int )(((disk->major << 20) | disk->first_minor) + index));
 return (tmp);
}

void i_size_write(struct inode *inode, loff_t i_size )
{
 inode->i_size = i_size;
 return;
}

int update_size(mddev_t *mddev, unsigned long size)
{
 mdk_rdev_t *rdev, *tmp, *prev;
 int rv ;
 sector_t avail ;
 int fit ;
 sector_t tmp___0 ;
 struct block_device *bdev;

 if ((unsigned int )(mddev->pers)->resize == (unsigned int )((void *)0)) {
  return (-22);
 }
 if (mddev->sync_thread) {
  return (-16);
 }
 prev = (mdk_rdev_t *)mddev;
 tmp = mddev->disks;
 while (1) {
  rdev = tmp;
  tmp = tmp->disks;
  if (tmp == mddev->disks) {
   break;
  }
  fit = size == 0UL;
  if (rdev->sb_offset > rdev->data_offset) {
   avail = rdev->sb_offset * 2UL - rdev->data_offset;
  } else {
   tmp___0 = get_capacity((rdev->bdev)->bd_disk);
   avail = tmp___0 - rdev->data_offset;
  }
  if (fit) {
   if (size == 0UL) {
    size = avail / 2UL;
   } else {
    if (size > avail / 2UL) {
     size = avail / 2UL;
    }
   }
  }
  if (avail < size << 1) {
   return (-28);
  }
 }

 rv = get_nondet_int();
 if (! rv) {

  if (!mddev->gendisk) return(rv);

  bdev = bdget_disk(mddev->gendisk, 0);
  if (bdev) {
   mutex_lock(& (bdev->bd_inode)->i_mutex);
   i_size_write(bdev->bd_inode, (long long )mddev->array_size << 10);
   mutex_unlock(& (bdev->bd_inode)->i_mutex);
   bdput(bdev);
  }
 }
 return (rv);
}

int update_raid_disks(mddev_t *mddev , int raid_disks )
{
 int rv ;

 if ((unsigned int )(mddev->pers)->reshape == (unsigned int )((void *)0)) {
  return (-22);
 }
 if (raid_disks <= 0) {
  return (-22);
 } else {
  if (raid_disks >= mddev->max_disks) {
   return (-22);
  }
 }
 if (mddev->sync_thread) {
  return (-16);
 }

 rv = get_nondet_int();
 return (rv);
}

unsigned long get_seconds(void)
{
 return ((unsigned long )xtime.tv_sec);
}

void sync_sbs(mddev_t *mddev )
{
 mdk_rdev_t *rdev,*tmp;

 tmp = mddev->disks;
 while (1) {
  rdev = tmp;
  tmp = tmp->disks;
  if (tmp == mddev->disks) {
   break;
  }


  rdev->sb_loaded = get_nondet_int();
 }
 return;
}

int bitmap_update_sb(struct bitmap *bitmap)
{
 int a; return a;
}

struct bio *bio_alloc(gfp_t x, int y)
{
 struct bio* a;
 a = (struct bio*)malloc(sizeof(struct bio));
 return a;
}

int bio_add_page(struct bio *x, struct page *y, unsigned int z, unsigned int w)
{
 int a; return a;
}

void submit_bio(int x, struct bio *y)
{
 free(y);
 return;
}

struct bio *bio_clone(struct bio *x, gfp_t y)
{
 return x;
}

void md_super_write(mddev_t *mddev , mdk_rdev_t *rdev , sector_t sector , int size , struct page *page )
{
 struct bio *bio ;
 struct bio *tmp ;
 int rw ;
 struct bio *rbio ;
 int tmp___0 ;

 tmp = bio_alloc(16U, 1);
 bio = tmp;
 rw = 17;
 bio->bi_bdev = rdev->bdev;
 bio->bi_sector = sector;
 bio_add_page(bio, page, (unsigned int )size, 0U);
 bio->bi_private = (void *)rdev;
 bio->bi_end_io = & super_written;
 bio->bi_rw = (unsigned long )rw;
 atomic_add_return(1, & mddev->pending_writes);
 tmp___0 = __test_bit(5, (unsigned long *)(& rdev->flags));
 if (tmp___0) {
  submit_bio(rw, bio);
 } else {
  rw |= 4;
  rbio = bio_clone(bio, 16U);
  rbio->bi_private = (void *)bio;
  rbio->bi_end_io = & super_written_barrier;
  submit_bio(rw, rbio);
 }
 return;
}

void prepare_to_wait(wait_queue_head_t *q, wait_queue_t *wait, int state)
{
 return;
}

void schedule(void)
{
 return;
}

void finish_wait(wait_queue_head_t *q, wait_queue_t *wait)
{
 return;
}

void md_super_wait(mddev_t *mddev )
{
 wait_queue_t wq ;
 void *tmp ;
 struct bio *bio ;
 unsigned long temp ;
 unsigned long temp___0 ;

 tmp = (void *)get_current();
 wq.flags = 0U;
 wq.private = tmp;
 wq.func = & autoremove_wake_function;
 wq.task_list.next = & wq.task_list;
 wq.task_list.prev = & wq.task_list;
 while (1) {
  prepare_to_wait(& mddev->sb_wait, & wq, 2);
  if (mddev->pending_writes.counter == 0) {
   break;
  }
  while (mddev->biolist) {



   bio = mddev->biolist;
   mddev->biolist = bio->bi_next;
   bio->bi_next = (struct bio *)((void *)0);



   submit_bio((int )bio->bi_rw, bio);
  }
  schedule();
 }
 finish_wait(& mddev->sb_wait, & wq);
 return;
}

void md_update_sb(mddev_t *mddev )
{
 int err ;
 mdk_rdev_t *rdev, *tmp;
 int sync_req ;
 unsigned long temp ;
 unsigned long temp___0 ;
 unsigned long temp___1 ;
 int tmp___4 ;
 int tmp___10 ;
 unsigned long temp___2 ;
 unsigned long temp___3 ;
 unsigned long temp___4 ;

repeat:



 sync_req = mddev->in_sync;
 mddev->utime = (time_t )get_seconds();
 mddev->events ++;

 if (! mddev->events) {
  HsPrintk("md: bug in file %s, line %d\n", "md.c", 1512);
  md_print_devices();
  mddev->events --;
 }
 mddev->sb_dirty = 2;
 sync_sbs(mddev);
 if (! mddev->persistent) {
  mddev->sb_dirty = 0;



  __wake_up(& mddev->sb_wait, 3U, 1, (void *)0);
  return;
 }



 err = bitmap_update_sb(mddev->bitmap);
 tmp = mddev->disks;
 while (1) {
  rdev = tmp;
  tmp = tmp->disks;
  if (tmp == mddev->disks) {
   break;
  }
  tmp___4 = __test_bit(1, (unsigned long *)(& rdev->flags));
  if (tmp___4) {

  }
  tmp___10 = __test_bit(1, (unsigned long *)(& rdev->flags));
  if (! tmp___10) {
   md_super_write(mddev, rdev, rdev->sb_offset << 1, rdev->sb_size, rdev->sb_page);
  }
  if (mddev->level == -4) {
   break;
  }
 }
 md_super_wait(mddev);



 if (mddev->in_sync != sync_req) {
  goto _L;
 } else {
  if (mddev->sb_dirty == 1) {
_L:



   goto repeat;
  }
 }
 mddev->sb_dirty = 0;



 __wake_up(& mddev->sb_wait, 3U, 1, (void *)0);
 return;
}

int bitmap_create(mddev_t *mddev)
{
 int a; return a;
}

void bitmap_destroy(mddev_t *mddev)
{
 return;
}

int update_array_info(mddev_t *mddev , mdu_array_info_t *info )
{
 int rv ;
 int cnt ;
 int state ;
 int tmp ;

 rv = 0;
 cnt = 0;
 state = 0;
 if (mddev->bitmap) {
  if (mddev->bitmap_offset) {
   state |= 256;
  }
 }
 if (mddev->major_version != info->major_version) {
  return (-22);
 } else {
  if (mddev->minor_version != info->minor_version) {
   return (-22);
  } else {
   if (mddev->ctime != (time_t )info->ctime) {
    return (-22);
   } else {
    if (mddev->level != info->level) {
     return (-22);
    } else {
     if (! mddev->persistent != info->not_persistent) {
      return (-22);
     } else {
      if (mddev->chunk_size != info->chunk_size) {
       return (-22);
      } else {
       if ((unsigned int )(state ^ info->state) & 4294966784U) {
        return (-22);
       }
      }
     }
    }
   }
  }
 }
 if (info->size >= 0) {
  if (mddev->size != (sector_t )info->size) {
   cnt ++;
  }
 }
 if (mddev->raid_disks != info->raid_disks) {
  cnt ++;
 }
 if (mddev->layout != info->layout) {
  cnt ++;
 }
 if ((state ^ info->state) & 256) {
  cnt ++;
 }
 if (cnt == 0) {
  return (0);
 }
 if (cnt > 1) {
  return (-22);
 }
 if (mddev->layout != info->layout) {
  if ((unsigned int )(mddev->pers)->reconfig == (unsigned int )((void *)0)) {
   return (-22);
  } else {

   tmp = get_nondet_int();
   return (tmp);
  }
 }
 if (info->size >= 0) {
  if (mddev->size != (sector_t )info->size) {
   rv = update_size(mddev, (unsigned long )info->size);
  }
 }
 if (mddev->raid_disks != info->raid_disks) {
  rv = update_raid_disks(mddev, info->raid_disks);
 }
 if ((state ^ info->state) & 256) {
  if ((unsigned int )(mddev->pers)->quiesce == (unsigned int )((void *)0)) {
   return (-22);
  }
  if (mddev->recovery) {
   return (-16);
  } else {
   if (mddev->sync_thread) {
    return (-16);
   }
  }
  if (info->state & 256) {
   if (mddev->bitmap) {
    return (-17);
   }
   if (mddev->default_bitmap_offset == 0L) {
    return (-22);
   }
   mddev->bitmap_offset = mddev->default_bitmap_offset;

   rv = bitmap_create(mddev);
   if (rv) {
    bitmap_destroy(mddev);
   }

  } else {
   if (! mddev->bitmap) {
    return (-2);
   }
   if ((mddev->bitmap)->file) {
    return (-22);
   }

   bitmap_destroy(mddev);

   mddev->bitmap_offset = 0L;
  }
 }
 md_update_sb(mddev);
 return (rv);
}

void get_random_bytes(void *buf, int nbytes)
{
 return;
}

int set_array_info(mddev_t *mddev , mdu_array_info_t *info )
{
 if (info->raid_disks == 0) {
  if (info->major_version < 0) {
   HsPrintk("<6>md: superblock version %d not known\n", info->major_version);
   return (-22);
  } else {
   if ((unsigned int )info->major_version >= sizeof(super_types) / sizeof(super_types[0])) {
    HsPrintk("<6>md: superblock version %d not known\n", info->major_version);
    return (-22);
   } else {
    if ((unsigned int )super_types[info->major_version].name == (unsigned int )((void *)0)) {
     HsPrintk("<6>md: superblock version %d not known\n", info->major_version);
     return (-22);
    }
   }
  }
  mddev->major_version = info->major_version;
  mddev->minor_version = info->minor_version;
  mddev->patch_version = info->patch_version;
  return (0);
 }
 mddev->major_version = 0;
 mddev->minor_version = 90;
 mddev->patch_version = 3;
 mddev->ctime = (time_t )get_seconds();
 mddev->level = info->level;
 mddev->clevel[0] = (char)0;
 mddev->size = (unsigned long )info->size;
 mddev->raid_disks = info->raid_disks;
 if (info->state & 1) {
  mddev->recovery_cp = ~ 0UL;
 } else {
  mddev->recovery_cp = 0UL;
 }
 mddev->persistent = ! info->not_persistent;
 mddev->layout = info->layout;
 mddev->chunk_size = info->chunk_size;
 mddev->max_disks = 27;
 mddev->sb_dirty = 1;
 mddev->default_bitmap_offset = 8L;
 mddev->bitmap_offset = 0L;
 get_random_bytes((void *)(mddev->uuid), 16);
 return (0);
}

int get_array_info(mddev_t *mddev , void *arg )
{
 mdu_array_info_t info ;
 int nr ;
 int working ;
 int active ;
 int failed ;
 int spare ;
 mdk_rdev_t *rdev, *tmp ;
 int tmp___0 ;
 int tmp___1 ;
 unsigned long tmp___2 ;

 spare = 0;
 failed = spare;
 active = failed;
 working = active;
 nr = working;
 tmp = mddev->disks;
 while (1) {
  rdev = tmp;
  tmp = tmp->disks;
  if (tmp == mddev->disks) {
   break;
  }
  nr ++;
  tmp___1 = __test_bit(1, (unsigned long *)(& rdev->flags));
  if (tmp___1) {
   failed ++;
  } else {
   working ++;
   tmp___0 = __test_bit(2, (unsigned long *)(& rdev->flags));
   if (tmp___0) {
    active ++;
   } else {
    spare ++;
   }
  }
 }
 info.major_version = mddev->major_version;
 info.minor_version = mddev->minor_version;
 info.patch_version = 3;
 info.ctime = (int )mddev->ctime;
 info.level = mddev->level;
 info.size = (int )mddev->size;
 if ((sector_t )info.size != mddev->size) {
  info.size = -1;
 }
 info.nr_disks = nr;
 info.raid_disks = mddev->raid_disks;
 info.md_minor = mddev->md_minor;
 info.not_persistent = ! mddev->persistent;
 info.utime = (int )mddev->utime;
 info.state = 0;
 if (mddev->in_sync) {
  info.state = 1;
 }
 if (mddev->bitmap) {
  if (mddev->bitmap_offset) {
   info.state = 256;
  }
 }
 info.active_disks = active;
 info.working_disks = working;
 info.failed_disks = failed;
 info.spare_disks = spare;
 info.layout = mddev->layout;
 info.chunk_size = mddev->chunk_size;
 tmp___2 = copy_to_user(arg, (void *)(& info), (unsigned long )sizeof(info));
 if (tmp___2) {
  return (-14);
 }
 return (0);
}

char *HsStrcpy(char *x, char *y)
{
 char *a; return a;
}

char *file_path(struct file *file , char *buf , int count )
{
 char *a; return a;
}

int get_bitmap_file(mddev_t *mddev , void *arg )
{
 mdu_bitmap_file_t *file ;
 char *ptr ;
 char *buf ;
 int err ;
 unsigned long tmp ;

 file = (mdu_bitmap_file_t *)((void *)0);
 buf = (char *)((void *)0);
 err = -12;
 file = (mdu_bitmap_file_t *)mallocnull(sizeof((*file)));
 if (! file) {
  goto out;
 }
 if (! mddev->bitmap) {
  file->pathname[0] = (char )'\000';
  goto copy_out;
 } else {
  if (! (mddev->bitmap)->file) {
   file->pathname[0] = (char )'\000';
   goto copy_out;
  }
 }
 buf = (char *)mallocnull(sizeof(file->pathname));
 if (! buf) {
  goto out;
 }
 ptr = file_path((mddev->bitmap)->file, buf, (int )sizeof(file->pathname));
 if (! ptr) {
  goto out;
 }
 HsStrcpy(file->pathname, (char *)ptr);
copy_out:
 err = 0;
 tmp = copy_to_user(arg, (void *)file, (unsigned long )sizeof((*file)));
 if (tmp) {
  err = -14;
 }
out:


 if (buf != 0) free((void *)buf);
 if (file != 0) free((void *)file);
 return (err);
}

unsigned long copy_from_user(void *to, void *from, unsigned long n)
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

mdk_rdev_t *find_rdev_nr(mddev_t *mddev , int nr )
{
 mdk_rdev_t *rdev, *tmp;

 tmp = mddev->disks;
 while (1) {
  rdev = tmp;
  tmp = tmp->disks;
  if (tmp == mddev->disks) {
   break;
  }
  if (rdev->desc_nr == nr) {
   return (rdev);
  }
 }
 return ((mdk_rdev_t *)((void *)0));
}

int get_disk_info(mddev_t *mddev , void *arg )
{
 mdu_disk_info_t info ;
 unsigned int nr ;
 mdk_rdev_t *rdev ;
 unsigned long tmp ;
 int tmp___0 ;
 int tmp___1 ;
 int tmp___2 ;
 unsigned long tmp___3 ;

 tmp = copy_from_user((void *)(& info), (void *)arg, (unsigned long )sizeof(info));
 if (tmp) {
  return (-14);
 }
 nr = (unsigned int )info.number;
 rdev = find_rdev_nr(mddev, (int )nr);
 if (rdev) {
  info.major = (int )((rdev->bdev)->bd_dev >> 20);
  info.minor = (int )((rdev->bdev)->bd_dev & 1048575U);
  info.raid_disk = rdev->raid_disk;
  info.state = 0;
  tmp___1 = __test_bit(1, (unsigned long *)(& rdev->flags));
  if (tmp___1) {
   info.state |= 1;
  } else {
   tmp___0 = __test_bit(2, (unsigned long *)(& rdev->flags));
   if (tmp___0) {
    info.state |= 2;
    info.state |= 4;
   }
  }
  tmp___2 = __test_bit(4, (unsigned long *)(& rdev->flags));
  if (tmp___2) {
   info.state |= 512;
  }
 } else {
  info.minor = 0;
  info.major = info.minor;
  info.raid_disk = -1;
  info.state = 8;
 }
 tmp___3 = copy_to_user(arg, (void *)(& info), (unsigned long )sizeof(info));
 if (tmp___3) {
  return (-14);
 }
 return (0);
}

void set_disk_ro(struct gendisk *disk, int flag)
{
 return;
}

int restart_array(mddev_t *mddev )
{
 struct gendisk *disk ;
 int err ;
 int tmp ;
 char *tmp___0 ;
 char *tmp___1 ;

 disk = mddev->gendisk;
 err = -6;
 if (mddev->disks == mddev) {
  goto out;
 }
 if (mddev->pers) {
  err = -16;
  if (! mddev->ro) {
   goto out;
  }
  mddev->safemode = 0U;
  mddev->ro = 0;
  set_disk_ro(disk, 0);
  tmp___0 = mdname(mddev);
  HsPrintk("<6>md: %s switched to read-write mode.\n", tmp___0);
  ____atomic_set_bit(5U, (unsigned long *)(& mddev->recovery));
  md_wakeup_thread(mddev->thread);
  err = 0;
 } else {
  tmp___1 = mdname(mddev);
  HsPrintk("<3>md: %s has no personality assigned.\n", tmp___1);
  err = -22;
 }
out:
 return (err);
}

int kthread_stop(struct task_struct *k )
{
 int a; return a;
}

void md_unregister_thread(mdk_thread_t *thread )
{
 kthread_stop(thread->tsk);
 free((void *)thread);
 return;
}

int del_timer(struct timer_list *timer)
{
 int a; return a;
}

int invalidate_partition(struct gendisk *x, int y)
{
 int a; return a;
}

void bitmap_flush(mddev_t *mddev)
{
 return;
}

void sysfs_remove_group(struct kobject *x, struct attribute_group *y)
{
 return;
}

int atomic_sub_return(int i , atomic_t *v )
{
 unsigned long flags ;
 int val ;
 unsigned long temp ;





 val = (int )v->counter;
 val -= i;
 v->counter = (int )val;


 return (val);
}

int module_is_live(struct module *mod )
{
 return ((int )mod->state != 2);
}

int wake_up_process(struct task_struct *tsk)
{
 int a; return a;
}

void module_put(struct module *module )
{
 unsigned int cpu ;
 int tmp ;
 int tmp___0 ;
 long tmp___1 ;

 if (module) {
  cpu = 0U;
  atomic_sub_return(1, & module->ref[cpu].count.a);
  tmp = module_is_live(module);
  if (tmp) {
   tmp___0 = 0;
  } else {
   tmp___0 = 1;
  }
  if (tmp___0 == 0) {
   wake_up_process(module->waiter);
  }
 }
 return;
}

void md_new_event(mddev_t *mddev )
{
 atomic_add_return(1, & md_event_count);
 __wake_up(& md_event_waiters, 3U, 1, (void *)0);
 return;
}

void set_capacity(struct gendisk *disk , sector_t size )
{
 disk->capacity = size;
 return;
}

void fput(struct file *f)
{
 HsFreeFile(f);
 return;
}

int do_md_stop(mddev_t *mddev , int ro )
{
 int err ;
 struct gendisk *disk ;
 char *tmp ;
 mdk_rdev_t *rdev, *tmp___0 ;
 struct gendisk *disk___0 ;
 char *tmp___1 ;
 char nm[20] ;
 char *tmp___2 ;

 err = 0;
 disk = mddev->gendisk;
 if (mddev->pers) {
  if (mddev->active.counter > 2) {
   tmp = mdname(mddev);
   HsPrintk("md: %s still in use.\n", tmp);
   return (-16);
  }
  if (mddev->sync_thread) {
   ____atomic_set_bit(3U, (unsigned long *)(& mddev->recovery));
   md_unregister_thread(mddev->sync_thread);
   mddev->sync_thread = (struct mdk_thread_s *)((void *)0);
  }
  del_timer(& mddev->safemode_timer);
  invalidate_partition(disk, 0);
  if (ro) {
   err = -6;
   if (mddev->ro == 1) {
    goto out;
   }
   mddev->ro = 1;
  } else {
   bitmap_flush(mddev);
   md_super_wait(mddev);
   if (mddev->ro) {
    set_disk_ro(disk, 0);
   }
   blk_queue_make_request(mddev->queue, & md_fail_request);

   if ((mddev->pers)->sync_request) {
    sysfs_remove_group(& mddev->kobj, (struct attribute_group *)(& md_redundancy_group));
   }
   module_put((mddev->pers)->owner);

   // BUG: 
   // Memory Leak
   free(mddev->pers);
   mddev->pers = (struct mdk_personality *)((void *)0);

   if (mddev->ro) {
    mddev->ro = 0;
   }
  }
  if (! mddev->in_sync) {
   mddev->in_sync = 1;
   md_update_sb(mddev);
  }
  if (ro) {
   set_disk_ro(disk, 1);
  }
 }
 if (! ro) {
  tmp___1 = mdname(mddev);
  HsPrintk("<6>md: %s stopped.\n", tmp___1);
  bitmap_destroy(mddev);
  if (mddev->bitmap_file) {
   (((mddev->bitmap_file)->f_dentry)->d_inode)->i_writecount.counter = (int )1;
   fput(mddev->bitmap_file);
   mddev->bitmap_file = (struct file *)((void *)0);
  }
  mddev->bitmap_offset = 0L;
  tmp___0 = mddev->disks;
  while (1) {
   rdev = tmp___0;
   tmp___0 = tmp___0->disks;
   if (tmp___0 == mddev->disks) {
    break;
   }
   if (rdev->raid_disk >= 0) {
    HsSprintf(nm, "rd%d", rdev->raid_disk);
    sysfs_remove_link(& mddev->kobj, (char *)(nm));
   }
  }
  export_array(mddev);
  mddev->array_size = 0UL;
  disk___0 = mddev->gendisk;
  if (disk___0) {
   set_capacity(disk___0, 0UL);
  }
  mddev->changed = 1;
 } else {
  tmp___2 = mdname(mddev);
  HsPrintk("<6>md: %s switched to read-only mode.\n", tmp___2);
 }
 err = 0;
 md_new_event(mddev);
out:
 return (err);
}

mdk_rdev_t *match_dev_unit(mddev_t *mddev , mdk_rdev_t *dev )
{
 mdk_rdev_t *rdev, *tmp ;

 tmp = mddev->disks;
 while (1) {
  rdev = tmp;
  tmp = tmp->disks;
  if (tmp == mddev->disks) {
   break;
  }
  if ((rdev->bdev)->bd_contains == (dev->bdev)->bd_contains) {
   return (rdev);
  }
 }
 return ((mdk_rdev_t *)((void *)0));
}

int kobject_set_name(struct kobject *x, char *y, ...)
{
 int a; return a;
}

int kobject_add(struct kobject *x)
{
 int a; return a;
}

int sysfs_create_link(struct kobject *kobj, struct kobject *target, char *name)
{
 int a; return a;
}

int bind_rdev_to_array(mdk_rdev_t *rdev , mddev_t *mddev )
{
 mdk_rdev_t *same_pdev ;
 char b[32] ;
 char b2[32] ;
 struct kobject *ko ;
 char *s ;
 char *tmp ;
 char *tmp___0 ;
 char *tmp___1 ;
 int choice ;
 mdk_rdev_t *tmp___2 ;
 mdk_rdev_t *tmp___3 ;
 int tmp___4 ;

 if (rdev->mddev) {
  HsPrintk("md: bug in file %s, line %d\n", "md.c", 1249);
  md_print_devices();
  return (-1);
 }
 if (rdev->size) {
  if (mddev->size == 0UL) {
   goto _L;
  } else {
   if (rdev->size < mddev->size) {
_L:
    if (mddev->pers) {
     return (-1);
    } else {
     mddev->size = rdev->size;
    }
   }
  }
 }
 same_pdev = match_dev_unit(mddev, rdev);
 if (same_pdev) {
  tmp = bdevname(same_pdev->bdev, b2);
  tmp___0 = bdevname(rdev->bdev, b);
  tmp___1 = mdname(mddev);
  HsPrintk("<4>%s: WARNING: %s appears to be on the same physical disk as %s. True\n     protection against single-disk failure might be compromised.\n", tmp___1, tmp___0, tmp);
 }
 if (rdev->desc_nr < 0) {
  choice = 0;
  if (mddev->pers) {
   choice = mddev->raid_disks;
  }
  while (1) {
   tmp___2 = find_rdev_nr(mddev, choice);
   if (! tmp___2) {
    break;
   }
   choice ++;
  }
  rdev->desc_nr = choice;
 } else {
  tmp___3 = find_rdev_nr(mddev, rdev->desc_nr);
  if (tmp___3) {
   return (-1);
  }
 }
 bdevname(rdev->bdev, b);
 tmp___4 = kobject_set_name(& rdev->kobj, "dev-%s", b);
 if (tmp___4 < 0) {
  return (-1);
 }
 while (1) {
  s = HsStrchr(rdev->kobj.k_name, '/');
  if (! ((unsigned int )s != (unsigned int )((void *)0))) {
   break;
  }
  (*(s+0)) = (char )'!';
 }
 rdev->disks = mddev->disks;
 mddev->disks = rdev;
 rdev->mddev = mddev;
 HsPrintk("<6>md: bind<%s>\n", b);

 rdev->kobj.parent = get_nondet_ptr();
 kobject_add(& rdev->kobj);
 if ((rdev->bdev)->bd_part) {
  ko = & ((rdev->bdev)->bd_part)->kobj;
 } else {
  ko = & ((rdev->bdev)->bd_disk)->kobj;
 }
 sysfs_create_link(& rdev->kobj, ko, "block");
 return (0);
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

sector_t calc_dev_sboffset(struct block_device *bdev )
{
 sector_t size ;
 size = (sector_t )((bdev->bd_inode)->i_size >> 10);
 return ((size & 4294967232UL) - 64UL);
}

sector_t calc_dev_size(mdk_rdev_t *rdev , unsigned int chunk_size )
{
 sector_t size ;
 size = rdev->sb_offset;
 if (chunk_size) {
  size &= ~ ((unsigned long )chunk_size / 1024UL - 1UL);
 }
 return (size);
}

int add_new_disk(mddev_t *mddev , mdu_disk_info_t *info )
{
 char b[32] ;
 char b2[32] ;
 mdk_rdev_t *rdev ;
 dev_t dev ;
 int err ;
 long tmp ;
 int tmp___0 ;
 long tmp___1 ;
 mdk_rdev_t *rdev0 ;
 struct list_head *__mptr ;
 int err___0 ;
 int tmp___2 ;
 char *tmp___3 ;
 char *tmp___4 ;
 int tmp___5 ;
 int err___1 ;
 char *tmp___6 ;
 long tmp___7 ;
 int tmp___8 ;
 long tmp___9 ;
 char *tmp___10 ;
 int err___2 ;
 long tmp___11 ;
 int tmp___12 ;
 long tmp___13 ;

 dev = (dev_t )((info->major << 20) | info->minor);
 if ((unsigned int )info->major != dev >> 20) {
  return (-75);
 } else {
  if ((unsigned int )info->minor != (dev & 1048575U)) {
   return (-75);
  }
 }
 if (! mddev->raid_disks) {
  rdev = md_import_device(dev, mddev->major_version, mddev->minor_version);
  if (!rdev) {
   tmp = rdev;
   HsPrintk("<4>md: md_import_device returned %ld\n", tmp);
   tmp___0 = -1;
   return (tmp___0);
  }
  if (mddev->disks != mddev) {
   rdev0 = mddev->disks;

   tmp___2 = get_nondet_int();
   err___0 = tmp___2;
   if (err___0 < 0) {
    tmp___3 = bdevname(rdev0->bdev, b2);
    tmp___4 = bdevname(rdev->bdev, b);
    HsPrintk("<4>md: %s has different UUID to %s\n", tmp___4, tmp___3);
    export_rdev_sl(rdev,rdev);
    return (-22);
   }
  }
  err = bind_rdev_to_array(rdev, mddev);
  if (err) {
   export_rdev_sl(rdev,rdev);
  }
  return (err);
 }
 if (mddev->pers) {
  if (! (mddev->pers)->hot_add_disk) {
   tmp___6 = mdname(mddev);
   HsPrintk("<4>%s: personality does not support diskops!\n", tmp___6);
   return (-22);
  }
  if (mddev->persistent) {
   rdev = md_import_device(dev, mddev->major_version, mddev->minor_version);
  } else {
   rdev = md_import_device(dev, -1, -1);
  }
  if (!rdev) {
   tmp___7 = rdev;
   HsPrintk("<4>md: md_import_device returned %ld\n", tmp___7);

   tmp___8 = -1;
   return (tmp___8);
  }
  if (! mddev->persistent) {
   if (info->state & 4) {
    if (info->raid_disk < mddev->raid_disks) {
     rdev->raid_disk = info->raid_disk;
    } else {
     rdev->raid_disk = -1;
    }
   } else {
    rdev->raid_disk = -1;
   }
  } else {

  }
  rdev->saved_raid_disk = rdev->raid_disk;
  ____atomic_clear_bit(2U, (unsigned long *)(& rdev->flags));
  if (info->state & 512) {
   ____atomic_set_bit(4U, (unsigned long *)(& rdev->flags));
  }
  rdev->raid_disk = -1;
  err___1 = bind_rdev_to_array(rdev, mddev);
  if (err___1) {
   export_rdev_sl(rdev,rdev);
  }
  ____atomic_set_bit(5U, (unsigned long *)(& mddev->recovery));
  md_wakeup_thread(mddev->thread);
  return (err___1);
 }
 if (mddev->major_version != 0) {
  tmp___10 = mdname(mddev);
  HsPrintk("<4>%s: ADD_NEW_DISK not supported\n", tmp___10);
  return (-22);
 }
 if (! (info->state & 1)) {
  rdev = md_import_device(dev, -1, 0);
  if (!rdev) {
   tmp___11 = rdev;
   HsPrintk("<4>md: error, md_import_device() returned %ld\n", tmp___11);

   tmp___12 = -1;
   return (tmp___12);
  }
  rdev->desc_nr = info->number;
  if (info->raid_disk < mddev->raid_disks) {
   rdev->raid_disk = info->raid_disk;
  } else {
   rdev->raid_disk = -1;
  }
  rdev->flags = 0UL;
  if (rdev->raid_disk < mddev->raid_disks) {
   if (info->state & 4) {
    ____atomic_set_bit(2U, (unsigned long *)(& rdev->flags));
   }
  }
  if (info->state & 512) {
   ____atomic_set_bit(4U, (unsigned long *)(& rdev->flags));
  }
  if (! mddev->persistent) {
   HsPrintk("<6>md: nonpersistent superblock ...\n");
   rdev->sb_offset = (unsigned long )(((rdev->bdev)->bd_inode)->i_size >> 10);
  } else {
   rdev->sb_offset = calc_dev_sboffset(rdev->bdev);
  }
  rdev->size = calc_dev_size(rdev, (unsigned int )mddev->chunk_size);
  err___2 = bind_rdev_to_array(rdev, mddev);
  if (err___2) {
   export_rdev_sl(rdev,rdev);
   return (err___2);
  }
 }
 return (0);
}

mdk_rdev_t *find_rdev(mddev_t *mddev , dev_t dev )
{
 mdk_rdev_t *rdev, *tmp ;

 tmp = mddev->disks;
 while (1) {
  rdev = tmp;
  tmp = tmp->disks;
  if (tmp == mddev->disks) {
   break;
  }
  if ((rdev->bdev)->bd_dev == dev) {
   return (rdev);
  }
 }
 return ((mdk_rdev_t *)((void *)0));
}

int hot_remove_disk(mddev_t *mddev , dev_t dev )
{
 char b[32] ;
 mdk_rdev_t *rdev, *rdev1, *tmp1, *prev;
 char *tmp ;
 char *tmp___0 ;

 if (! mddev->pers) {
  return (-19);
 }

 prev = (mdk_rdev_t *)mddev;
 tmp1 = mddev->disks;
 while (1) {
  rdev1 = tmp1;
  tmp1 = tmp1->disks;
  if (tmp1 == mddev->disks) {
   rdev = 0;
   break;
  }
  if ((rdev1->bdev)->bd_dev == dev) {
   rdev = rdev1;
   break;
  }
  prev = rdev1;
 }

 if (! rdev) {
  return (-6);
 }
 if (rdev->raid_disk >= 0) {
  goto busy;
 }
 kick_rdev_from_array_sl(prev,rdev);
 md_update_sb(mddev);
 md_new_event(mddev);
 return (0);
busy:
 tmp = mdname(mddev);
 tmp___0 = bdevname(rdev->bdev, b);
 HsPrintk("<4>md: cannot remove active disk %s from %s ... \n", tmp___0, tmp);
 return (-16);
}

int hot_add_disk(mddev_t *mddev , dev_t dev )
{
 char b[32] ;
 int err ;
 unsigned int size ;
 mdk_rdev_t *rdev ;
 char *tmp ;
 char *tmp___0 ;
 long tmp___1 ;
 long tmp___2 ;
 char *tmp___3 ;
 char *tmp___4 ;
 int tmp___5 ;
 char *tmp___6 ;

 if (! mddev->pers) {
  return (-19);
 }
 if (mddev->major_version != 0) {
  tmp = mdname(mddev);
  HsPrintk("<4>%s: HOT_ADD may only be used with version-0 superblocks.\n", tmp);
  return (-22);
 }
 if (! (mddev->pers)->hot_add_disk) {
  tmp___0 = mdname(mddev);
  HsPrintk("<4>%s: personality does not support diskops!\n", tmp___0);
  return (-22);
 }
 rdev = md_import_device(dev, -1, 0);
 if (!rdev) {
  tmp___1 = (void *)rdev;
  HsPrintk("<4>md: error, md_import_device() returned %ld\n", tmp___1);
  return (-22);
 }
 if (mddev->persistent) {
  rdev->sb_offset = calc_dev_sboffset(rdev->bdev);
 } else {
  rdev->sb_offset = (unsigned long )(((rdev->bdev)->bd_inode)->i_size >> 10);
 }
 size = (unsigned int )calc_dev_size(rdev, (unsigned int )mddev->chunk_size);
 rdev->size = (unsigned long )size;
 tmp___5 = __test_bit(1, (unsigned long *)(& rdev->flags));
 if (tmp___5) {
  tmp___3 = mdname(mddev);
  tmp___4 = bdevname(rdev->bdev, b);
  HsPrintk("<4>md: can not hot-add faulty %s disk to %s!\n", tmp___4, tmp___3);
  err = -22;
  goto abort_export;
 }
 ____atomic_clear_bit(2U, (unsigned long *)(& rdev->flags));
 rdev->desc_nr = -1;
 err = bind_rdev_to_array(rdev, mddev);
 if (err) {
  goto abort_export;
 }
 if (rdev->desc_nr == mddev->max_disks) {
  tmp___6 = mdname(mddev);
  HsPrintk("<4>%s: can not hot-add to full array!\n", tmp___6);
  err = -16;
  goto abort_unbind_export;
 }
 rdev->raid_disk = -1;
 md_update_sb(mddev);
 ____atomic_set_bit(5U, (unsigned long *)(& mddev->recovery));
 md_wakeup_thread(mddev->thread);
 md_new_event(mddev);
 return (0);
abort_unbind_export:





 return (err);
abort_export:
 export_rdev_sl(rdev,rdev);
 return (err);
}

void md_error(mddev_t *mddev , mdk_rdev_t *rdev )
{
 int tmp ;

 if (! mddev) {
  HsPrintk("md: bug in file %s, line %d\n", "md.c", 3994);
  md_print_devices();
  return;
 }
 if (! rdev) {
  return;
 } else {
  tmp = __test_bit(1, (unsigned long *)(& rdev->flags));
  if (tmp) {
   return;
  }
 }
 if (! (mddev->pers)->error_handler) {
  return;
 }

 ____atomic_set_bit(3U, (unsigned long *)(& mddev->recovery));
 ____atomic_set_bit(5U, (unsigned long *)(& mddev->recovery));
 md_wakeup_thread(mddev->thread);
 md_new_event(mddev);
 return;
}

int set_disk_faulty(mddev_t *mddev , dev_t dev )
{
 mdk_rdev_t *rdev ;

 if ((unsigned int )mddev->pers == (unsigned int )((void *)0)) {
  return (-19);
 }
 rdev = find_rdev(mddev, dev);
 if (! rdev) {
  return (-19);
 }
 md_error(mddev, rdev);
 return (0);
}

void analyze_sbs(mddev_t *mddev )
{
 int i ;
 mdk_rdev_t *rdev, *tmp, *prev ;
 mdk_rdev_t *freshest ;
 char b[32] ;
 int tmp___0 ;
 char *tmp___1 ;
 struct list_head *__mptr ;
 char *tmp___2 ;
 int tmp___3 ;
 int tmp___4 ;
 struct list_head *__mptr___0 ;
 char *tmp___5 ;

 freshest = (mdk_rdev_t *)((void *)0);
 prev = (mdk_rdev_t *)mddev;
 tmp = mddev->disks;
 while (1) {
  rdev = tmp;
  tmp = tmp->disks;
  if (tmp == mddev->disks) {
   break;
  }

  tmp___0 = get_nondet_int();
  if (tmp___0 == 1) {
   freshest = rdev;
   prev = rdev;
  }
  else if (tmp___0 == 0) {
   prev = rdev;
  }
  else {
   tmp___1 = bdevname(rdev->bdev, b);
   HsPrintk("<3>md: fatal superblock inconsistency in %s -- removing from array\n", tmp___1);
   kick_rdev_from_array_sl(prev,rdev);
  }
 }

 i = 0;
 prev = (mdk_rdev_t *)mddev;
 tmp = mddev->disks;
 while (1) {
  rdev = tmp;
  tmp = tmp->disks;
  if (tmp == mddev->disks) {
   break;
  }
  if ((unsigned int )rdev != (unsigned int )freshest) {

   tmp___3 = get_nondet_int();
   if (tmp___3) {
    tmp___2 = bdevname(rdev->bdev, b);
    HsPrintk("<4>md: kicking non-fresh %s from array!\n", tmp___2);
    kick_rdev_from_array_sl(prev,rdev);
    goto __Cont;
   }
   else {
    prev = rdev;
   }
  }
  else {
   prev = rdev;
  }
  if (mddev->level == -4) {
   tmp___4 = i;
   i ++;
   rdev->desc_nr = tmp___4;
   rdev->raid_disk = rdev->desc_nr;
   ____atomic_set_bit(2U, (unsigned long *)(& rdev->flags));
  }
__Cont:
  ;
 }
 if (mddev->recovery_cp != 4294967295UL) {
  if (mddev->level >= 1) {
   tmp___5 = mdname(mddev);
   HsPrintk("<3>md: %s: raid array is not clean -- starting background reruction\n", tmp___5);
  }
 }
 return;
}

unsigned long ffz(unsigned long word )
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

int request_module(char *name, ...)
{
 int a; return a;
}

int sync_blockdev(struct block_device *bdev)
{
 int a; return a;
}

void invalidate_bdev(struct block_device *x, int y)
{
 return;
}

struct mdk_personality *find_pers(int level , char *clevel )
{
 struct mdk_personality *pers,*prev;
 int tmp ;

 prev = &pers_list;
 pers = pers_list.next;
 while (1) {
  prefetch((void *)pers->next);
  if (pers == &pers_list) {
   break;
  }
  if (level != -1000000) {
   if (pers->level == level) {
    prev->next = pers->next;
    pers->next = get_nondet_ptr();
    return (pers);
   }
  }
  tmp = HsStrcmp((char *)pers->name, (char *)clevel);
  if (tmp == 0) {
   prev->next = pers->next;
   pers->next = get_nondet_ptr();
   return (pers);
  }
  prev = pers;
  pers = pers->next;
 }
 return ((struct mdk_personality *)((void *)0));
}

int try_module_get(struct module *module )
{
 int ret ;
 unsigned int cpu ;
 int tmp ;
 int tmp___0 ;
 long tmp___1 ;

 ret = 1;
 if (module) {
  cpu = 0U;
  tmp = module_is_live(module);
  if (tmp) {
   tmp___0 = 1;
  } else {
   tmp___0 = 0;
  }

  tmp___1 = get_nondet_int();
  if (tmp___1) {
   atomic_add_return(1, & module->ref[cpu].count.a);
  } else {
   ret = 0;
  }
 }
 return (ret);
}

int sysfs_create_group(struct kobject *x, struct attribute_group *y)
{
 int a; return a;
}

void md_safemode_timeout(unsigned long data )
{
 mddev_t *mddev ;

 mddev = (mddev_t *)data;
 mddev->safemode = 1U;
 md_wakeup_thread(mddev->thread);
 return;
}

int do_md_run(mddev_t *mddev )
{
 int err ;
 int chunk_size ;
 mdk_rdev_t *rdev, *tmp ;
 struct gendisk *disk ;
 struct mdk_personality *pers ;
 char b[32] ;
 int tmp___0 ;
 unsigned long tmp___1 ;
 int tmp___2 ;
 char *tmp___3 ;
 struct list_head *__mptr ;
 int tmp___4 ;
 struct list_head *__mptr___0 ;
 int tmp___5 ;
 char *tmp___6 ;
 char nm[20] ;
 struct list_head *__mptr___1 ;

 if (mddev->disks == (mdk_rdev_t *)mddev) {
  return (-22);
 }
 if (mddev->pers) {
  return (-16);
 }
 if (! mddev->raid_disks) {
  analyze_sbs(mddev);
 }
 chunk_size = mddev->chunk_size;
 if (chunk_size) {
  if (chunk_size > 4194304) {
   HsPrintk("<3>too big chunk_size: %d > %d\n", chunk_size, 4194304);
   return (-22);
  }
  tmp___1 = ffz((unsigned long )(~ chunk_size));
  if (1 << tmp___1 != chunk_size) {
   HsPrintk("<3>chunk_size of %d not valid\n", chunk_size);
   return (-22);
  }
  if ((unsigned long )chunk_size < 4096UL) {
   HsPrintk("<3>too small chunk_size: %d < %ld\n", chunk_size, 4096UL);
   return (-22);
  }
  tmp = mddev->disks;
  while (1) {
   rdev = tmp;
   tmp = tmp->disks;
   if (tmp == mddev->disks) {
    break;
   }
   tmp___2 = __test_bit(1, (unsigned long *)(& rdev->flags));
   if (tmp___2) {
    goto __Cont;
   }
   if (rdev->size < (sector_t )(chunk_size / 1024)) {
    tmp___3 = bdevname(rdev->bdev, b);
    HsPrintk("<4>md: Dev %s smaller than chunk_size: %lluk < %dk\n", tmp___3, (unsigned long long )rdev->size, chunk_size / 1024);
    return (-22);
   }
__Cont:
   ;
  }
 }
 if (mddev->level != -1000000) {
  request_module("md-level-%d", mddev->level);
 } else {
  if (mddev->clevel[0]) {
   request_module("md-%s", mddev->clevel);
  }
 }
 tmp = mddev->disks;
 while (1) {
  rdev = tmp;
  tmp = tmp->disks;
  if (tmp == mddev->disks) {
   break;
  }
  tmp___4 = __test_bit(1, (unsigned long *)(& rdev->flags));
  if (tmp___4) {
   goto __Cont___0;
  }
  sync_blockdev(rdev->bdev);
  invalidate_bdev(rdev->bdev, 0);
__Cont___0:
  ;
 }
 md_probe(mddev->unit, (int *)((void *)0), (void *)0);
 disk = mddev->gendisk;
 if (! disk) {
  return (-12);
 }
 pers = find_pers(mddev->level, mddev->clevel);
 if (! pers) {
  goto _L;
 } else {
  tmp___5 = try_module_get(pers->owner);
  if (! tmp___5) {
   free(pers);
_L:
   if (mddev->level != -1000000) {
    HsPrintk("<4>md: personality for level %d is not loaded!\n", mddev->level);
   } else {
    HsPrintk("<4>md: personality for level %s is not loaded!\n", mddev->clevel);
   }
   return (-22);
  }
 }
 mddev->pers = pers;
 mddev->level = pers->level;
 HsStrlcpy(mddev->clevel, (char *)pers->name, sizeof(mddev->clevel));
 mddev->recovery = 0UL;
 mddev->resync_max_sectors = mddev->size << 1;
 mddev->barriers_work = 1;
 mddev->ok_start_degraded = start_dirty_degraded;
 if (start_readonly) {
  mddev->ro = 2;
 }

 err = get_nondet_int();
 if (! err) {
  if ((mddev->pers)->sync_request) {
   err = bitmap_create(mddev);
   if (err) {
    tmp___6 = mdname(mddev);
    HsPrintk("<3>%s: failed to create bitmap (%d)\n", tmp___6, err);

   }
  }
 }
 else {
  HsPrintk("<3>md: pers->run() failed ...\n");
  module_put((mddev->pers)->owner);
  mddev->pers = (struct mdk_personality *)((void *)0);
  bitmap_destroy(mddev);
  free(pers);
  return (err);
 }
 if ((mddev->pers)->sync_request) {
  sysfs_create_group(& mddev->kobj, (struct attribute_group *)(& md_redundancy_group));
 } else {
  if (mddev->ro == 2) {
   mddev->ro = 0;
  }
 }
 mddev->writes_pending.counter = (int )0;
 mddev->safemode = 0U;
 mddev->safemode_timer.function = & md_safemode_timeout;
 mddev->safemode_timer.data = (unsigned long )mddev;
 mddev->safemode_delay = 3U;
 mddev->in_sync = 1;
 tmp = mddev->disks;
 while (1) {
  rdev = tmp;
  tmp = tmp->disks;
  if (tmp == mddev->disks) {
   break;
  }
  if (rdev->raid_disk >= 0) {
   HsSprintf(nm, "rd%d", rdev->raid_disk);
   sysfs_create_link(& mddev->kobj, & rdev->kobj, (char *)(nm));
  }
 }
 ____atomic_set_bit(5U, (unsigned long *)(& mddev->recovery));
 md_wakeup_thread(mddev->thread);
 if (mddev->sb_dirty) {
  md_update_sb(mddev);
 }
 set_capacity(disk, mddev->array_size << 1);
 (mddev->queue)->queuedata = (void *)mddev;
 (mddev->queue)->make_request_fn = (mddev->pers)->make_request;
 mddev->changed = 1;
 md_new_event(mddev);
 return (0);
}

struct file *fget(unsigned int fd)
{
 struct file *f;
 f = HsCreateFile();
 return f;
}

int deny_bitmap_write_access(struct file *file )
{
 struct inode *inode ;
 inode = (file->f_mapping)->host;
 if (inode->i_writecount.counter > 1) {
  return (-26);
 }
 inode->i_writecount.counter = (int )-1;
 return (0);
}

int set_bitmap_file(mddev_t *mddev , int fd )
{
 int err ;
 char *tmp ;
 char *tmp___0 ;

 if (mddev->pers) {
  if (! (mddev->pers)->quiesce) {
   return (-16);
  }
  if (mddev->recovery) {
   return (-16);
  } else {
   if (mddev->sync_thread) {
    return (-16);
   }
  }
 }
 if (fd >= 0) {
  if (mddev->bitmap) {
   return (-17);
  }


  if (mddev->bitmap_file) fput(mddev->bitmap_file);

  mddev->bitmap_file = fget((unsigned int )fd);
  if ((unsigned int )mddev->bitmap_file == (unsigned int )((void *)0)) {
   tmp = mdname(mddev);
   HsPrintk("<3>%s: error: failed to get bitmap file\n", tmp);
   return (-9);
  }
  err = deny_bitmap_write_access(mddev->bitmap_file);
  if (err) {
   tmp___0 = mdname(mddev);
   HsPrintk("<3>%s: error: bitmap file is already in use\n", tmp___0);
   fput(mddev->bitmap_file);
   mddev->bitmap_file = (struct file *)((void *)0);
   return (err);
  }
  mddev->bitmap_offset = 0L;
 } else {
  if ((unsigned int )mddev->bitmap == (unsigned int )((void *)0)) {
   return (-2);
  }
 }
 err = 0;
 if (mddev->pers) {

  if (fd >= 0) {
   err = bitmap_create(mddev);
  }
  if (fd < 0) {
   bitmap_destroy(mddev);
  } else {
   if (err) {
    bitmap_destroy(mddev);
   }
  }

 } else {
  if (fd < 0) {
   if (mddev->bitmap_file) {
    fput(mddev->bitmap_file);
   }
   mddev->bitmap_file = (struct file *)((void *)0);
  }
 }
 return (err);
}

int md_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg )
{
 int err ;
 void *argp ;
 mddev_t *mddev ;
 int tmp ;
 struct task_struct *tmp___0 ;
 struct task_struct *tmp___1 ;
 dev_t tmp___2 ;
 mdu_array_info_t info ;
 void *__p ;
 size_t __n ;
 unsigned long tmp___3 ;
 char *tmp___4 ;
 int tmp___5 ;
 char *tmp___6 ;
 unsigned int tmp___7 ;
 unsigned int tmp___8 ;
 unsigned int tmp___9 ;
 mdu_disk_info_t info___0 ;
 unsigned long tmp___10 ;
 dev_t tmp___11 ;
 dev_t tmp___12 ;
 dev_t tmp___13 ;
 struct task_struct *tmp___14 ;
 struct task_struct *tmp___15 ;

 err = 0;
 argp = (void *)arg;
 mddev = (mddev_t *)((void *)0);
 tmp = capable(21);
 if (! tmp) {
  return (-13);
 }

 if (cmd == 2147485968U | (sizeof(mdu_version_t ) << 16)) {
  err = get_version(argp);
  goto done;
 }
 else if (cmd == 2323U) {
  err = 0;
  md_print_devices();
  goto done;
 }
 else if (cmd == 2324U) {
  err = 0;
  autostart_arrays((int )arg);
  goto done;
 }

 mddev = (mddev_t *)((inode->i_bdev)->bd_disk)->private_data;
 if (! mddev) {
  __bug("md.c", 3652, (void *)0);
  goto abort;
 }
 if (cmd == 2353U) {
  if (cnt > 0) {
   tmp___0 = get_current();
   tmp___1 = get_current();
   HsPrintk("<4>md: %s(pid %d) used deprecated START_ARRAY ioctl. This will not be supported beyond July 2006\n", tmp___1->comm, tmp___0->pid);
   cnt --;
  }
  tmp___2 = new_decode_dev((unsigned int )arg);
  err = autostart_array(tmp___2);
  if (err) {
   HsPrintk("<4>md: autostart failed!\n");
   goto abort;
  }
  goto done;
 }
 err = mddev_lock(mddev);
 if (err) {
  HsPrintk("<6>md: ioctl lock interrupted, reason %d, cmd %d\n", err, cmd);
  goto abort;
 }

 if (cmd == 1073744163U | (sizeof(mdu_array_info_t ) << 16)) {
  if (! arg) {
   __p = (void *)(& info);
   __n = sizeof(info);
   if (__n != 0U) {
    __memzero(__p, __n);
   }
  } else {
   tmp___3 = copy_from_user((void *)(& info), (void *)argp, (unsigned long )sizeof(info));
   if (tmp___3) {
    err = -14;
    goto done_unlock;
   }
  }
  if (mddev->pers) {
   err = update_array_info(mddev, & info);
   if (err) {
    HsPrintk("<4>md: couldn\'t update array info. %d\n", err);
    goto done_unlock;
   }
   goto done_unlock;
  }
  if (mddev->disks == (mdk_rdev_t *)mddev) {
   tmp___4 = mdname(mddev);
   HsPrintk("<4>md: array %s already has disks!\n", tmp___4);
   err = -16;
   goto done_unlock;
  }
  if (mddev->raid_disks) {
   tmp___6 = mdname(mddev);
   HsPrintk("<4>md: array %s already initialised!\n", tmp___6);
   err = -16;
   goto done_unlock;
  }
  err = set_array_info(mddev, & info);
  if (err) {
   HsPrintk("<4>md: couldn\'t set array info. %d\n", err);
   goto done_unlock;
  }
  goto done_unlock;
 }

 if (! mddev->raid_disks) {
  if (sizeof(mdu_disk_info_t ) == sizeof(mdu_disk_info_t [1])) {
   if (sizeof(mdu_disk_info_t ) < 16384U) {
    tmp___7 = sizeof(mdu_disk_info_t );
   } else {
    tmp___7 = __invalid_size_argument_for_IOC;
   }
  } else {
   tmp___7 = __invalid_size_argument_for_IOC;
  }
  if (cmd != (1073744161U | (tmp___7 << 16))) {
   if (cmd != 2354U) {
    if (sizeof(mdu_param_t ) == sizeof(mdu_param_t [1])) {
     if (sizeof(mdu_param_t ) < 16384U) {
      tmp___8 = sizeof(mdu_param_t );
     } else {
      tmp___8 = __invalid_size_argument_for_IOC;
     }
    } else {
     tmp___8 = __invalid_size_argument_for_IOC;
    }
    if (cmd != (1073744176U | (tmp___8 << 16))) {
     if (sizeof(int ) == sizeof(int [1])) {
      if (sizeof(int ) < 16384U) {
       tmp___9 = sizeof(int );
      } else {
       tmp___9 = __invalid_size_argument_for_IOC;
      }
     } else {
      tmp___9 = __invalid_size_argument_for_IOC;
     }
     if (cmd != (1073744171U | (tmp___9 << 16))) {
      err = -19;
      goto done_unlock;
     }
    }
   }
  }
 }

 if (cmd == 2147485969U | (sizeof(mdu_array_info_t ) << 16)) {
  err = get_array_info(mddev, argp);
  goto done_unlock;
 }
 else if (cmd == 2147485973U | (sizeof(mdu_bitmap_file_t ) << 16)) {
  err = get_bitmap_file(mddev, argp);
  goto done_unlock;
 }
 else if (cmd == 2147485970U | (sizeof(mdu_disk_info_t ) << 16)) {
  err = get_disk_info(mddev, argp);
  goto done_unlock;
 }
 else if (cmd == 2356U) {
  err = restart_array(mddev);
  goto done_unlock;
 }
 else if (cmd == 2354U) {
  err = do_md_stop(mddev, 0);
  goto done_unlock;
 }
 else if (cmd == 2355U) {
  err = do_md_stop(mddev, 1);
  goto done_unlock;
 }

 if (((cmd >> 8) & 255U) == 9U) {
  if (mddev->ro) {
   if (mddev->pers) {
    if (mddev->ro == 2) {
     mddev->ro = 0;
     ____atomic_set_bit(5U, (unsigned long *)(& mddev->recovery));
     md_wakeup_thread(mddev->thread);
    } else {
     err = -30;
     goto done_unlock;
    }
   }
  }
 }

 if (cmd == 1073744161U | (sizeof(mdu_disk_info_t ) << 16)) {
  tmp___10 = copy_from_user((void *)(& info___0), (void *)argp, (unsigned long )sizeof(info___0));
  if (tmp___10) {
   err = -14;
  } else {
   err = add_new_disk(mddev, & info___0);
  }
  goto done_unlock;
 }
 else if (cmd == 2338U) {
  tmp___11 = new_decode_dev((unsigned int )arg);
  err = hot_remove_disk(mddev, tmp___11);
  goto done_unlock;
 }
 else if (cmd == 2344U) {
  tmp___12 = new_decode_dev((unsigned int )arg);
  err = hot_add_disk(mddev, tmp___12);
  goto done_unlock;
 }
 else if (cmd == 2345U) {
  tmp___13 = new_decode_dev((unsigned int )arg);
  err = set_disk_faulty(mddev, tmp___13);
  goto done_unlock;
 }
 else if (cmd == 1073744176U | (sizeof(mdu_param_t) << 16)) {
  err = do_md_run(mddev);
  goto done_unlock;
 }
 else if (cmd == 1073744171U | (sizeof(int ) << 16)) {
  err = set_bitmap_file(mddev, (int )arg);
  goto done_unlock;
 }
 else {
  if (((cmd >> 8) & 255U) == 9U) {
   tmp___14 = get_current();
   tmp___15 = get_current();
   HsPrintk("<4>md: %s(pid %d) used obsolete MD ioctl, upgrade your software to use new ictls.\n",
       tmp___15->comm, tmp___14->pid);
  }
  err = -22;
  goto done_unlock;
 }

done_unlock:
done_unock:
 mddev_unlock(mddev);
 return (err);
done:
 if (err) {
  HsPrintk("md: bug in file %s, line %d\n", "md.c", 3848);
  md_print_devices();
 }
abort:
 return (err);
}

int md_open(struct inode *inode , struct file *file )
{
 mddev_t *mddev ;
 int err ;

 mddev = (mddev_t *)((inode->i_bdev)->bd_disk)->private_data;
 err = mddev_lock(mddev);
 if (err) {
  goto out;
 }
 err = 0;
 mddev_get(mddev);
 mddev_unlock(mddev);
 check_disk_change(inode->i_bdev);
out:
 return (err);
}

int md_release(struct inode *inode , struct file *file )
{
 mddev_t *mddev ;

 mddev = (mddev_t *)((inode->i_bdev)->bd_disk)->private_data;
 if (! mddev) {
  __bug("md.c", 3879, (void *)0);
 }
 mddev_put(mddev);
 HsFreeDev(mddev);
 return (0);
}

void HsInitialize(void)
{
 mddev_t *dev;
 mdk_rdev_t *rdev;
 struct mdk_personality *pers;

 HsProc = 0;
 HsThreadInfo = (struct thread_info *)malloc(sizeof(struct thread_info));
 HsThreadInfo->task = (struct task_struct *)malloc(sizeof(struct task_struct));
 HsModule.state = 1;

 cnt = 3;
 mdp_major = 0;
 sysctl_speed_limit_min = 1000;
 sysctl_speed_limit_max = 200000;
 start_dirty_degraded = 0;

 raid_table_header = 0;

 pers_list.next = (struct mdk_personality *)&pers_list;
 while ((&nondet < 0)) {
  pers = (struct mdk_personality *)malloc(sizeof(struct mdk_personality));
  pers->owner = &HsModule;
  pers->next = pers_list.next;
  pers_list.next = pers;
 }

 all_mddevs.next = (mddev_t *)&all_mddevs;
 while ((&nondet < 0)) {
  dev = HsCreateDev();
  dev->next = all_mddevs.next;
  do {
   rdev = HsCreateRdev();
   rdev->mddev = dev;
   rdev->disks = dev->disks;
   dev->disks = rdev;
  } while ((&nondet < 0));
  all_mddevs.next = dev;
 }

 pending_raid_disks.disks = (mdk_rdev_t *)&pending_raid_disks;
 while ((&nondet < 0)) {
  rdev = HsCreateRdev();
  rdev->disks = pending_raid_disks.disks;
  pending_raid_disks.disks = rdev;
 }

 return;
}

void HsFinalize()
{
 mdk_rdev_t *rdev;
 struct mdk_personality *pers;

 while (pers_list.next != &pers_list) {
  pers = pers_list.next;
  pers_list.next = pers->next;
  free(pers);
 }

 while (pending_raid_disks.disks != &pending_raid_disks) {
  rdev = pending_raid_disks.disks;
  pending_raid_disks.disks = rdev->disks;
  HsFreeRdev(rdev);
 }

 free(HsThreadInfo->task);
 free(HsThreadInfo);
 HsThreadInfo = 0;
 HsProc = 0;

 mdp_major = 0;
 cnt = 0;
}

int main_sub(void)
{
 int tmp;
 unsigned int cmd;
 unsigned long arg;
 struct inode *inode;
 struct file *file;

 mddev_t *mddev;
 mdk_rdev_t *rdev;

 HsInitialize();

 tmp = md_init();
 if (tmp != 0) {
  return tmp;
 }

 while((&nondet < 0)) {
  mddev = HsCreateDevGeneral();
  while ((&nondet < 0)) {
    rdev = HsCreateRdev();
    rdev->mddev = mddev;
    rdev->disks = mddev->disks;
    mddev->disks = rdev;
  }
  file = HsCreateFile();
  inode = HsCreateInode();
  ((inode->i_bdev)->bd_disk)->private_data = mddev;

  md_open(inode, file);

  cmd = get_nondet_int();
  arg = get_nondet_int();

  md_ioctl(inode, file, cmd, arg);

  md_release(inode, file);
  HsFreeInode(inode);
  HsFreeFile(file);
 }

 md_exit();
 HsFinalize();

 return 0;
}

int main(void)
{
 int tmp;
 tmp = main_sub();
 return tmp;
}
