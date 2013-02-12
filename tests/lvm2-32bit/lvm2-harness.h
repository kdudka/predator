#include <verifier-builtins.h>

void *dm_malloc_aux(size_t s, const char *file, int line)
{
    (void) file;
    (void) line;
    return malloc(s);
}

void *dm_zalloc_aux(size_t s, const char *file, int line)
{
    (void) file;
    (void) line;
    return calloc(s, 1);
}

char *__strdup (__const char *__string)
{
    size_t len = 1UL + strlen(__string);
    char *dup = malloc(len);
    memmove(dup, __string, len);
    return dup;
}

char *strcpy (char *__restrict __dest, __const char *__restrict __src)
{
    size_t len = 1UL + strlen(__src);
    return memcpy(__dest, __src, len);
}

char *strncpy (char *__restrict __dest,
        __const char *__restrict __src, size_t __n)
{
    return memcpy(__dest, __src, __n);
}

int strncmp (__const char *__s1, __const char *__s2, size_t __n)
{
    strlen(__s1);
    strlen(__s2);
    (void) __n;
    return __VERIFIER_nondet_int();
}

int strcmp (__const char *__s1, __const char *__s2)
{
    strlen(__s1);
    strlen(__s2);
    return __VERIFIER_nondet_int();
}

int is_orphan_vg(const char *vg_name)
{
    strlen(vg_name);
    return __VERIFIER_nondet_int();
}

/* verbatim copy from uuid-prep.c */
int id_write_format(const struct id *id, char *buffer, size_t size)
{
 int i, tot;

 static const unsigned group_size[] = { 6, 4, 4, 4, 4, 4, 6 };

 ((32 == 32) ? (void) (0) : __assert_fail ("32 == 32", "uuid/uuid.c", 163, __PRETTY_FUNCTION__));


 if (size < (32 + 6 + 1)) {
  print_log(3, "uuid/uuid.c", 167 , -1,"Couldn't write uuid, buffer too small.");
  return 0;
 }

 for (i = 0, tot = 0; i < 7; i++) {
  memcpy(buffer, id->uuid + tot, group_size[i]);
  buffer += group_size[i];
  tot += group_size[i];
  *buffer++ = '-';
 }

 *--buffer = '\0';
 return 1;
}

#define NULL ((void *)0)

/*
 * Given the address v of an instance of 'struct dm_list' called 'head' 
 * contained in a structure of type t, return the containing structure.
 */
#define dm_list_struct_base(v, t, head) \
    ((t *)((const char *)(v) - (const char *)&((t *) 0)->head))

/*
 * Given the address v of an instance of 'struct dm_list list' contained in
 * a structure of type t, return the containing structure.
 */
#define dm_list_item(v, t) dm_list_struct_base((v), t, list)

/*
 * Given the address v of one known element e in a known structure of type t,
 * return another element f.
 */
#define dm_struct_field(v, t, e, f) \
    (((t *)((uintptr_t)(v) - (uintptr_t)&((t *) 0)->e))->f)

/*
 * Given the address v of a known element e in a known structure of type t,
 * return the list head 'list'
 */
#define dm_list_head(v, t, e) dm_struct_field(v, t, e, list)

/*
 * Walk a list, setting 'v' in turn to the containing structure of each item.
 * The containing structure should be the same type as 'v'.
 * The 'struct dm_list' variable within the containing structure is 'field'.
 */
#define dm_list_iterate_items_gen(v, head, field) \
	for (v = dm_list_struct_base((head)->n, __typeof__(*v), field); \
	     &v->field != (head); \
	     v = dm_list_struct_base(v->field.n, __typeof__(*v), field))

/*
 * Walk a list, setting 'v' in turn to the containing structure of each item.
 * The containing structure should be the same type as 'v'.
 * The list should be 'struct dm_list list' within the containing structure.
 */
#define dm_list_iterate_items(v, head) dm_list_iterate_items_gen(v, (head), list)

/* overridden simplified implementation of dm_hash_create() */
struct dm_hash_table *dm_hash_create(unsigned size_hint)
{
    (void) size_hint;

    void *ptr = malloc(sizeof(struct dm_list));
    if (!ptr)
        abort();

    /* we approximate hash table using an unordered list */
    dm_list_init(ptr);
    return ptr;
}

struct ht_node {
    void *data;
    void *owner;
    struct dm_list list;
};

/* overridden simplified implementation of dm_hash_lookup() */
void *dm_hash_lookup(struct dm_hash_table *t, const char *key)
{
    struct dm_list *head = (struct dm_list *) t;
    struct dm_list *pos;
    (void) key;

    /* return either random list node or NULL */
    for (pos = head->n; head != pos; pos = pos->n)
        if (__VERIFIER_nondet_int())
            return dm_list_item(pos, struct ht_node)->data;

    return NULL;
}

/* overridden simplified implementation of dm_hash_insert() */
int dm_hash_insert(struct dm_hash_table *t, const char *key, void *data)
{
    void *head = t;
    struct dm_list *pos = head;
    (void) key;

    /* seek random list position */
    while (__VERIFIER_nondet_int())
        pos = pos->p;

    if (head != pos && __VERIFIER_nondet_int())
        /* simulate lookup success if we have at least one node in the list */
        return 0;

    /* allocate a new node */
    struct ht_node *node = malloc(sizeof *node);
    if (!node)
        return 0;

    node->data = data;
    node->owner = t;

    /* insert the new node at a random position in the list */
    dm_list_add(pos, &node->list);
    return 1;
}

/* overridden simplified implementation of dm_hash_remove() */
void dm_hash_remove(struct dm_hash_table *t, const char *key)
{
    void *head = t;
    struct dm_list *pos = head;
    (void) key;

    /* seek random list position */
    while (__VERIFIER_nondet_int())
        pos = pos->p;

    if (head == pos)
        /* not found*/
        return;

    dm_list_del(pos);
    free(dm_list_item(pos, struct ht_node));
}

/* overridden simplified implementation of dm_hash_destroy() */
void dm_hash_destroy(struct dm_hash_table *t)
{
    void *head = t;
    struct dm_list *pos = head;
    struct dm_list *next = pos->n;
    while (pos != head) {
        free(dm_list_item(pos, struct ht_node));
        pos = next;
    }

    free(head);
}

/* overridden simplified implementation of dm_hash_iter() */
void dm_hash_iter(struct dm_hash_table *t, dm_hash_iterate_fn f)
{
    struct dm_list *head = (struct dm_list *) t;
    struct dm_list *pos;

    for (pos = head->n; head != pos; pos = pos->n) {
        struct ht_node *node = dm_list_item(pos, struct ht_node);
        f(node->data);
    }
}

struct btree *btree_create(struct dm_pool *mem)
{
    (void) mem;
    return (void *) dm_hash_create(0);
}

struct btree_iter *btree_first(const struct btree *t)
{
    struct dm_list *head = (struct dm_list *) t;
    if (head->n == head)
        /* empty tree */
        return NULL;

    return (void *) head->n;
}

struct btree_iter *btree_next(const struct btree_iter *it)
{
    struct dm_list *head = (struct dm_list *) it;
    if (dm_list_item(head, struct ht_node)->owner == head->n)
        return NULL;

    return (void *) head->n;
}

void *btree_get_data(const struct btree_iter *it)
{
    struct dm_list *head = (struct dm_list *) it;
    return dm_list_item(head, struct ht_node)->data;
}

void *btree_lookup(const struct btree *t, uint32_t k)
{
    void *ht = (void *) t;
    return dm_hash_lookup(ht, ht);
}

int btree_insert(struct btree *t, uint32_t k, void *data)
{
    void *ht = (void *) t;
    return dm_hash_insert(ht, ht, data);
}

struct dirent {
    __ino64_t d_ino;
    __off64_t d_off;
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256];
};

extern int alphasort (__const struct dirent **__e1, __const struct dirent **__e2) __asm__ ("" "alphasort64") __attribute__ ((__nothrow__))
    __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

int scandir (__const char *__restrict __dir, struct dirent ***__restrict __namelist, int (*__selector) (__const struct dirent *), int (*__cmp) (__const struct dirent **, __const struct dirent **))
{
    return -1;
}

struct dm_pool *dm_pool_create(const char *name, size_t chunk_hint) {
    return malloc(1U);
}

char *dm_pool_strdup(struct dm_pool *p, const char *str) {
    return __strdup(str);
}

void *dm_pool_zalloc(struct dm_pool *p, size_t s) {
    return calloc(s, 1U);
}

const struct config_node *find_config_tree_node(struct cmd_context *cmd,
      const char *path)
{
    return NULL;
}

void init_full_scan_done(int level) {
    (void) level;
}

int stat (__const char *__restrict path, struct stat *__restrict buf)
{
    struct stat tmp;
    memcpy(&tmp, buf, sizeof tmp);
    memset(buf, '\0', sizeof tmp);
    memcpy(buf, &tmp, sizeof tmp);

    strlen(path);
    return 0;
}

static inline int _dev_is_valid(struct device *dev)
{
    return (dev->max_error_count == 0 ||
            dev->error_count < dev->max_error_count);
}

static int _get_block_size(struct device *dev, unsigned int *size)
{
    const char *name = dev_name(dev);

    if (dev->block_size == -1) {
        if (__VERIFIER_nondet_int() < 0) {
            print_log(3, "device/dev-io.c", 131 , -1,"%s: %s failed: %s", name, "ioctl BLKBSZGET", strerror((*__errno_location ())));
            return 0;
        }
        print_log(7, "device/dev-io.c", 134 , 0,"%s: block size is %u bytes", name, dev->block_size);
    }

    *size = (unsigned int) dev->block_size;

    return 1;
}

static int _aligned_io(struct device_area *where, char *buffer,
        int should_write)
{
    char *bounce, *bounce_buf;
    unsigned int block_size = 0;
    uintptr_t mask;
    struct device_area widened;
    int r = 0;

    if (!(where->dev->flags & 0x00000002) &&
            !_get_block_size(where->dev, &block_size))
        do { print_log(7, "device/dev-io.c", 175 , 0,"<backtrace>"); return 0; } while (0);

    /* TODO */
#if 0
    if (!block_size)
        block_size = lvm_getpagesize() __VERIFIER_nondet_int();

    _widen_region(block_size, where, &widened);


    mask = block_size - 1;
    if (!memcmp(where, &widened, sizeof(widened)) &&
            !((uintptr_t) buffer & mask))
        return _io(where, buffer, should_write);


    if (!(bounce_buf = bounce = dm_malloc_aux(((size_t) widened.size + block_size), "device/dev-io.c", 189))) {
        print_log(3, "device/dev-io.c", 190 , -1,"Bounce buffer malloc failed");
        return 0;
    }




    if (((uintptr_t) bounce) & mask)
        bounce = (char *) ((((uintptr_t) bounce) + mask) & ~mask);


    if (!_io(&widened, bounce, 0)) {
        if (!should_write)
            do { print_log(7, "device/dev-io.c", 203 , 0,"<backtrace>"); goto out; } while (0);

        memset(bounce, '\n', widened.size);
    }

    if (should_write) {
        memcpy(bounce + (where->start - widened.start), buffer,
                (size_t) where->size);


        if (!(r = _io(&widened, bounce, 1)))
            print_log(7, "device/dev-io.c", 214 , 0,"<backtrace>");

        goto out;
    }

    memcpy(buffer, bounce + (where->start - widened.start),
            (size_t) where->size);

    r = 1;

out:
    free(bounce_buf);
#endif
    return r;
}

static void _dev_inc_error_count(struct device *dev)
{
    if (++dev->error_count == dev->max_error_count)
        print_log(4 | 128,

                "device/dev-io.c"
                /* # 615 "device/dev-io.c" */
                ,

                617
                /* # 615 "device/dev-io.c" */
                , 0,"WARNING: Error counts reached a limit of %d. " "Device %s was disabled", dev->max_error_count, dev_name(dev))

            ;
}

int dev_open(struct device *dev)
{
    if (__VERIFIER_nondet_int())
        return 0;

    dev->open_count ++;
    return 1;
}

int dev_read(struct device *dev, uint64_t offset, size_t len, void *buffer)
{
    struct device_area where;
    int ret;

    if (!dev->open_count)
        do { print_log(7, "device/dev-io.c", 626 , 0,"<backtrace>"); return 0; } while (0);

    if (!_dev_is_valid(dev))
        return 0;

    where.dev = dev;
    where.start = offset;
    where.size = len;

    ret = _aligned_io(&where, buffer, 0);
    if (!ret)
        _dev_inc_error_count(dev);

    return ret;
}

static int _dev_close(struct device *dev, int immediate)
{
    struct lvmcache_info *info;

    if (dev->fd < 0) {
        print_log(3,
                "device/dev-io.c"
                /* # 556 "device/dev-io.c" */
                ,
                557
                /* # 556 "device/dev-io.c" */
                , -1,"Attempt to close device '%s' " "which is not open.", dev_name(dev))
            ;
        return 0;
    }






    if (dev->open_count > 0)
        dev->open_count--;

    if (immediate && dev->open_count)
        print_log(7,
                "device/dev-io.c"
                /* # 570 "device/dev-io.c" */
                ,
                571
                /* # 570 "device/dev-io.c" */
                , 0,"%s: Immediate close attempt while still referenced", dev_name(dev))
            ;


    if (immediate ||
            (dev->open_count < 1 &&
             (!(info = info_from_pvid(dev->pvid, 0)) ||
              !info->vginfo ||
              !vgname_is_locked(info->vginfo->vgname))))
        _close(dev);

    return 1;
}

int dev_close(struct device *dev)
{
    return _dev_close(dev, 0);
}

#define MDA_IGNORED 0x00000001

unsigned mda_is_ignored(struct metadata_area *mda)
{
    return (mda->status & MDA_IGNORED);
}

int mdas_empty_or_ignored(struct dm_list *mdas)
{
    struct metadata_area *mda;

    if (!dm_list_size(mdas))
        return 1;
    dm_list_iterate_items(mda, mdas) {
        if (mda_is_ignored(mda))
            return 1;
    }
    return 0;
}
