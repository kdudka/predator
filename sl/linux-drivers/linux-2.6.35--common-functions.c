// vim: tw=128
// functions common to all drivers from Linux 2.6.35 kernel
// functions can be abstracted

void __memzero(void *x, size_t y)
{
 return;
}

// TODO:

//     U abort                  internal
//     U free                   internal
//     U malloc                 internal

//     U blk_execute_rq
//int blk_execute_rq(request_queue_t *q , struct gendisk *bd_disk , struct request *rq , int at_head )
//{
// int a; return a;
//}

void elv_put_request(struct request_queue *q, struct request *rq)
{
        struct elevator_queue *e = q->elevator;

        if (e->ops->elevator_put_req_fn)
                e->ops->elevator_put_req_fn(rq);
}

//     U blk_free_request
#define REQ_ELVPRIV     (1 << __REQ_ELVPRIV)

static inline void blk_free_request(struct request_queue *q, struct request *rq)
{
        if (rq->cmd_flags & REQ_ELVPRIV)
                elv_put_request(q, rq);
        mempool_free(rq, q->rq.rq_pool);
}

//     U blk_get_request
struct request *blk_get_request(struct request_queue *q , int rw , gfp_t gfp_mask)
{
 struct request *a; return a;
}
//     U blk_rq_map_user
int blk_rq_map_user(struct request_queue *q, struct request *rq, struct
                    rq_map_data *qqq, void *ubuf, unsigned long len, gfp_t ggg)
{
 int a; return a;
}
//     U blk_rq_unmap_user
int blk_rq_unmap_user(struct bio *bio)
{
 int a; return a;
}

//     U capable
int capable(int cap ) { int a; return a; }
//     U check_disk_change
int check_disk_change(struct block_device *x) { int a; return a; }
//     U _copy_from_user
//     U copy_from_user_overflow
//     U copy_to_user
//     U __copy_to_user_ll
//     U current_task
//     U elv_completed_request
//     U freed_request

void __wake_up(wait_queue_head_t *q, unsigned int mode, 
                        int nr_exclusive, void *key) 
{ 
//        unsigned long flags; 
// 
//        spin_lock_irqsave(&q->lock, flags); 
//        __wake_up_common(q, mode, nr_exclusive, 0, key); 
//        spin_unlock_irqrestore(&q->lock, flags); 
} 

#define wake_up(x)                      __wake_up(x, 3, 1, NULL)


static void __freed_request(struct request_queue *q, int sync)
{
        struct request_list *rl = &q->rq;

//        if (rl->count[sync] < queue_congestion_off_threshold(q))
//                blk_clear_queue_congested(q, sync);

        if (rl->count[sync] + 1 <= q->nr_requests) {
                if (waitqueue_active(&rl->wait[sync]))
                        wake_up(&rl->wait[sync]);

                blk_clear_queue_full(q, sync);
        }
}

static void freed_request(struct request_queue *q, int sync, int priv)
{
        struct request_list *rl = &q->rq;

        rl->count[sync]--;
        if (priv)
                rl->elvpriv--;

        __freed_request(q, sync);

        if ((rl->starved[sync ^ 1]))
                __freed_request(q, sync ^ 1);
}


//     U invalidate_bdev
void invalidate_bdev(struct block_device *x) { return ; }

//     U memset

//     U mutex_lock
void mutex_lock(struct mutex *lock) {}
//     U mutex_unlock
void mutex_unlock(struct mutex *lock) {}


//     U proc_dointvec
//     U proc_dostring

//     U _raw_spin_lock_irqsave

//     U scsi_cmd_ioctl
int scsi_cmd_ioctl(struct request_queue *x, struct gendisk *y, fmode_t ttt, unsigned int u, void *v)
{
 int a; return a;
}
//     U sysctl_set_parent
static void sysctl_set_parent(struct ctl_table *parent, struct ctl_table *table)
{
    for (; table->procname; table++) {
	table->parent = parent;
	if (table->child)
	    sysctl_set_parent(table, table->child);
    }
}


//     U try_attach
static struct ctl_table *is_branch_in(struct ctl_table *branch,
                                      struct ctl_table *table)
{
        struct ctl_table *p;
        const char *s = branch->procname;

        /* branch should have named subdirectory as its first element */
        if (!s || !branch->child)
                return NULL;

        /* ... and nothing else */
        if (branch[1].procname) 
                return NULL;    
        
        /* table should contain subdirectory with the same name */
        for (p = table; p->procname; p++) {
                if (!p->child)  
                        continue;
                if (p->procname && strcmp(p->procname, s) == 0)
                        return p;
        }
        return NULL;
}

/* see if attaching q to p would be an improvement */
static void try_attach(struct ctl_table_header *p, struct ctl_table_header *q)
{
        struct ctl_table *to = p->ctl_table, *by = q->ctl_table;
        struct ctl_table *next;
        int is_better = 0;
        int not_in_parent = !p->attached_by;

        while ((next = is_branch_in(by, to)) != NULL) {
                if (by == q->attached_by)
                        is_better = 1;
                if (to == p->attached_by)
                        not_in_parent = 1;
                by = by->child;
                to = next->child;
        }

        if (is_better && not_in_parent) {
                q->attached_by = by;
                q->attached_to = to;
                q->parent = p;
        }
}

//     U unregister_sysctl_table
//     U warn_slowpath_null


__attribute__((regparm(0))) int printk(const char * fmt, ...) { int a; return a; }

void *memcpy(void *to, const void *from, size_t len) { void *a; return a; }


// from current.h
////PP warning: extra simple function body
//struct task_struct *get_current(void)
//{
//        static struct task_struct x;
//        return &x;
//}
//__inline static struct thread_info *( __attribute__((__always_inline__)) current_thread_info)(void)
//{
//}
//__inline static struct task_struct *( __attribute__((__always_inline__)) get_current)(void)
//{
// struct thread_info *tmp ;
// tmp = current_thread_info();
// return (tmp->task);
//}



// simple kmalloc
void *  __attribute__((alloc_size(1))) __kmalloc(size_t size, gfp_t flags) {
    return malloc(size);
}
void kfree(const void *objp) { free(objp); }

#pragma GCC diagnostic ignored "-Wuninitialized"

int param_set_bool(const char *val, struct kernel_param *kp) { int r; return r; }
int param_get_bool(char *buffer, struct kernel_param *kp) { int r; return r; }

int scnprintf(char *buf, size_t size, const char *fmt, ...) {
    int r; return r;
}

//unsigned long copy_from_user(void *to, const void *from, unsigned long n) {}
//unsigned long copy_to_user(void *to, const void *from, unsigned long n) {}







static struct ctl_table root_table[1];
static struct ctl_table_root sysctl_table_root;
static struct ctl_table_header root_table_header = {
 .count = 1,
 .ctl_table = root_table,
 .ctl_entry = { &(sysctl_table_root.default_set.list), &(sysctl_table_root.default_set.list) },
 .root = &sysctl_table_root,
 .set = &sysctl_table_root.default_set,
};
static struct ctl_table_root sysctl_table_root = {
 .root_list = { &(sysctl_table_root.root_list), &(sysctl_table_root.root_list) },
 .default_set.list = { &(root_table_header.ctl_entry), &(root_table_header.ctl_entry) },
};


// 
static struct ctl_table_set *lookup_header_set(struct ctl_table_root *root,
                                               struct nsproxy *namespaces)
{
    struct ctl_table_set *set = &root->default_set;
    if (root->lookup)
        set = root->lookup(root, namespaces);
    return set;
}


//# 1825 "kernel/sysctl.c"
struct ctl_table_header *__register_sysctl_paths(
            struct ctl_table_root *root,
            struct nsproxy *namespaces,
            const struct ctl_path *path,
            struct ctl_table *table)
{
    struct ctl_table_header *header;
    struct ctl_table *new, **prevp;
    unsigned int n, npath;
    struct ctl_table_set *set;


    for (npath = 0; path[npath].procname; ++npath);
//# 1847 "kernel/sysctl.c"
    header = kzalloc(sizeof(struct ctl_table_header) +
                     (2 * npath * sizeof(struct ctl_table)),
                     (((gfp_t) 0x10u) | ((gfp_t) 0x40u) |
                      ((gfp_t) 0x80u)));
    if (!header)
        return ((void *) 0);

    new = (struct ctl_table *) (header + 1);


    prevp = &header->ctl_table;
    for (n = 0; n < npath; ++n, ++path) {

        new->procname = path->procname;
        new->mode = 0555;

        *prevp = new;
        prevp = &new->child;

        new += 2;
    }
    *prevp = table;
    header->ctl_table_arg = table;

    INIT_LIST_HEAD(&header->ctl_entry);
    header->used = 0;
    header->unregistering = ((void *) 0);
    header->root = root;
    sysctl_set_parent(((void *) 0), header->ctl_table);
    header->count = 1;

// spin_lock(&sysctl_lock);
    header->set = lookup_header_set(root, namespaces);
    header->attached_by = header->ctl_table;
    header->attached_to = root_table;
    header->parent = &root_table_header;
    for (set = header->set; set; set = set->parent) {
        struct ctl_table_header *p;
        for (p = ( { const typeof(((typeof(*p) *) 0)->ctl_entry) * __mptr = ((&set->list)->next);
                  (typeof(*p) *) ((char *) __mptr - __builtin_offsetof(typeof(*p), ctl_entry));});

             __builtin_prefetch(p->ctl_entry.next), &p->ctl_entry != (&set->list);

             p = ( { const typeof(((typeof(*p) *) 0)->ctl_entry) * __mptr = (p->ctl_entry.next);
                    (typeof(*p) *) ((char *) __mptr - __builtin_offsetof (typeof (*p), ctl_entry));}))
        {
            if (p->unregistering)
                continue;
            try_attach(p, header);
        }
    }
    header->parent->count++;
    list_add_tail(&header->ctl_entry, &header->set->list);
// spin_unlock(&sysctl_lock);

    return header;
}
//# 1911 "kernel/sysctl.c"
struct ctl_table_header *register_sysctl_paths(const struct ctl_path *path,
      struct ctl_table *table)
{
    return __register_sysctl_paths(
                &sysctl_table_root,
                get_current()->nsproxy,
                path,
                table);
}
//# 1927 "kernel/sysctl.c"
struct ctl_table_header *register_sysctl_table(struct ctl_table *table)
{
 static const struct ctl_path null_path[] = { {} };

 return register_sysctl_paths(null_path, table);
}


// from blk-core.c
void __blk_put_request(struct request_queue *q, struct request *req)
{
     if (__builtin_expect(!!(!q), 0))
           return;
            if (__builtin_expect(!!(--req->ref_count), 0))
                  return;

                   elv_completed_request(q, req);


                    ({ int __ret_warn_on = !!(req->bio != ((void *)0)); if (__builtin_expect(!!(__ret_warn_on), 0))
                    warn_slowpath_null("block/blk-core.c", 1108); __builtin_expect(!!(__ret_warn_on), 0); });





                     if (req->cmd_flags & (1 << __REQ_ALLOCED)) {
                           int is_sync = rq_is_sync(req) != 0;
                             int priv = req->cmd_flags & (1 << __REQ_ELVPRIV);

                               do { if (__builtin_expect(!!(!list_empty(&req->queuelist)), 0)) do { asm volatile("1:\tud2\n"
                                   ".pushsection __bug_table,\"a\"\n" "2:\t.long 1b - 2b, %c0 - 2b\n" "\t.word %c1, 0\n" "\t.org 2b+%c2\n" ".popsection" : : "i" ("block/blk-core.c"), "i" (1118), "i" (sizeof(struct
                                   bug_entry))); __builtin_unreachable(); } while (0); } while(0);
                                     do { if (__builtin_expect(!!(!hlist_unhashed(&req->hash)), 0)) do { asm
                                         volatile("1:\tud2\n" ".pushsection __bug_table,\"a\"\n" "2:\t.long 1b - 2b, %c0 - 2b\n"
                                         "\t.word %c1, 0\n" "\t.org 2b+%c2\n" ".popsection" : : "i" ("block/blk-core.c"), "i"
                                         (1119), "i" (sizeof(struct bug_entry))); __builtin_unreachable(); } while (0); }
                                         while(0);

                                           blk_free_request(q, req);
                                             freed_request(q, is_sync, priv);
                                              }
}
;

#if 1
//invader: 
void blk_put_request(struct request *req) { return; }
#else
void blk_put_request(struct request *req)
{
     unsigned long flags;
      struct request_queue *q = req->q;

       do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); flags =
           _raw_spin_lock_irqsave(spinlock_check(q->queue_lock)); } while (0); } while (0);
            __blk_put_request(q, req);
             spin_unlock_irqrestore(q->queue_lock, flags);
}
#endif





