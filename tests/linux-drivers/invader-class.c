# 1 "aclass.c"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "aclass.c"
void *malloc(int);
void free(void *);

int nondet;
# 15 "aclass.c"
enum kobject_action {
    KOBJ_ADD = 1,
    KOBJ_REMOVE = 2,
    KOBJ_CHANGE = 3,
    KOBJ_OFFLINE = 4,
    KOBJ_ONLINE = 5,
    KOBJ_MOVE = 6
};

enum module_state {
    MODULE_STATE_LIVE = 0,
    MODULE_STATE_COMING = 1,
    MODULE_STATE_GOING = 2
};

struct bus_type;
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
struct klist_node;
struct kobject;
struct kobj_type;
struct kset;
struct module;
struct module_param_attrs;
struct notifier_block;
struct task_struct;
struct vm_area_struct;

typedef unsigned int __kernel_mode_t;
typedef unsigned long __kernel_size_t;
typedef long __kernel_ssize_t;
typedef long long __kernel_loff_t;
typedef unsigned short __u16;
typedef int __s32;
typedef unsigned int __u32;
typedef unsigned long long __u64;
typedef unsigned long long u64;
typedef __u32 __kernel_dev_t;
typedef __kernel_dev_t dev_t;
typedef __kernel_mode_t mode_t;
typedef __kernel_loff_t loff_t;
typedef __kernel_size_t size_t;
typedef __kernel_ssize_t ssize_t;
typedef unsigned int gfp_t;
typedef __u64 Elf64_Addr;
typedef __u16 Elf64_Half;
typedef __u32 Elf64_Word;
typedef __u64 Elf64_Xword;



typedef int va_list;

struct bug_entry {
 unsigned long bug_addr ;
 char const *file ;
 unsigned short line ;
 unsigned short flags ;
};

struct list_head {
 struct list_head *next ;
 struct list_head *prev ;
};

struct __anonstruct_atomic_t_14 {
 int counter ;
};
typedef struct __anonstruct_atomic_t_14 atomic_t;

struct __anonstruct_atomic64_t_15 {
 long volatile counter ;
};
typedef struct __anonstruct_atomic64_t_15 atomic64_t;
typedef atomic64_t atomic_long_t;

struct attribute {
 char const *name ;
 struct module *owner ;
 mode_t mode ;
}
;
struct attribute_group {
 char const *name ;
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
 ssize_t (*store)(struct kobject * , struct attribute * , char const * , size_t ) ;
};

struct __anonstruct_raw_spinlock_t_16 {
 unsigned int slock ;
};
typedef struct __anonstruct_raw_spinlock_t_16 raw_spinlock_t;

struct __anonstruct_spinlock_t_18 {
 raw_spinlock_t raw_lock ;
};
typedef struct __anonstruct_spinlock_t_18 spinlock_t;

struct kref {
 atomic_t refcount ;
};

struct __wait_queue_head {
 spinlock_t lock ;
 struct list_head task_list ;
};
typedef struct __wait_queue_head wait_queue_head_t;

struct kobject {
 char const *k_name ;
 char name[20] ;
 struct kref kref ;
 struct list_head entry ;
 struct kobject *parent ;
 struct kset *kset ;
 struct kobj_type *ktype ;
 struct dentry *dentry ;
 wait_queue_head_t poll ;
};

struct kobj_type {
 void (*release)(struct kobject * ) ;
 struct sysfs_ops *sysfs_ops ;
 struct attribute **default_attrs ;
};

struct kset_uevent_ops {
 int (*filter)(struct kset *kset, struct class_device *cls_dev);
 char const *(*name)(struct kset *kset, struct class_device *cls_dev);
 int (*uevent)(struct kset *kset, struct class_device *cls_dev, char **envp, int num_envp,
    char *buffer, int buffer_size);
};

struct kset {
 struct kobj_type *ktype ;
 struct list_head list ;
 spinlock_t list_lock ;
 struct kobject kobj ;
 struct kset_uevent_ops *uevent_ops ;
};

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

struct elf64_sym {
 Elf64_Word st_name ;
 unsigned char st_info ;
 unsigned char st_other ;
 Elf64_Half st_shndx ;
 Elf64_Addr st_value ;
 Elf64_Xword st_size ;
};
typedef struct elf64_sym Elf64_Sym;

struct rw_semaphore {
 __s32 activity ;
 spinlock_t wait_lock ;
 struct list_head wait_list ;
};

struct notifier_block {
 int (*notifier_call)(struct notifier_block * , unsigned long , void * ) ;
 struct notifier_block *next ;
 int priority ;
};

struct blocking_notifier_head {
 struct rw_semaphore rwsem ;
 struct notifier_block *head ;
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

struct __anonstruct_local_t_78 {
 atomic_long_t a ;
};
typedef struct __anonstruct_local_t_78 local_t;

struct mod_arch_specific {
};

struct kernel_symbol {
 unsigned long value ;
 char const *name ;
};

struct module_attribute {
 struct attribute attr ;
 ssize_t (*show)(struct module_attribute * , struct module * , char * ) ;
 ssize_t (*store)(struct module_attribute * , struct module * , char const *, size_t count ) ;
 void (*setup)(struct module * , char const * ) ;
 int (*test)(struct module * ) ;
 void (*free)(struct module * ) ;
};

struct module_kobject {
 struct kobject kobj ;
 struct module *mod ;
 struct kobject *drivers_dir ;
};

struct module_ref {
 local_t count ;
};

struct module_sect_attr {
 struct module_attribute mattr ;
 char *name ;
 unsigned long address ;
};

struct module_sect_attrs {
 struct attribute_group grp ;
 int nsections ;
 struct module_sect_attr attrs[0] ;
};

struct module {
 enum module_state state ;
 struct list_head list ;
 char name[(int )(64U - sizeof(unsigned long ))] ;
 struct module_kobject mkobj ;
 struct module_param_attrs *param_attrs ;
 struct module_attribute *modinfo_attrs ;
 char const *version ;
 char const *srcversion ;
 struct kobject *holders_dir ;
 struct kernel_symbol const *syms ;
 unsigned int num_syms ;
 unsigned long const *crcs ;
 struct kernel_symbol const *gpl_syms ;
 unsigned int num_gpl_syms ;
 unsigned long const *gpl_crcs ;
 struct kernel_symbol const *unused_syms ;
 unsigned int num_unused_syms ;
 unsigned long const *unused_crcs ;
 struct kernel_symbol const *unused_gpl_syms ;
 unsigned int num_unused_gpl_syms ;
 unsigned long const *unused_gpl_crcs ;
 struct kernel_symbol const *gpl_future_syms ;
 unsigned int num_gpl_future_syms ;
 unsigned long const *gpl_future_crcs ;
 unsigned int num_exentries ;
 struct exception_table_entry const *extable ;
 int (*init)(void) ;
 void *module_init ;
 void *module_core ;
 unsigned long init_size ;
 unsigned long core_size ;
 unsigned long init_text_size ;
 unsigned long core_text_size ;
 void *unwind_info ;
 struct mod_arch_specific arch ;
 int unsafe ;
 unsigned int taints ;
 struct list_head bug_list ;
 struct bug_entry *bug_table ;
 unsigned int num_bugs ;
 struct module_ref ref[32] ;
 struct list_head modules_which_use_me ;
 struct task_struct *waiter ;
 void (*exit)(void) ;
 Elf64_Sym *symtab ;
 unsigned long num_symtab ;
 char *strtab ;
 struct module_sect_attrs *sect_attrs ;
 void *percpu ;
 char *args ;
};

struct semaphore {
 atomic_t count ;
 int sleepers ;
 wait_queue_head_t wait ;
};

struct dev_archdata {
 void *acpi_handle ;
};

struct bus_attribute {
 struct attribute attr ;
 ssize_t (*show)(struct bus_type * , char *buf ) ;
 ssize_t (*store)(struct bus_type * , char const *buf , size_t count ) ;
};

struct bus_type {
 char const *name ;
 struct module *owner ;
 struct kset subsys ;
 struct kset drivers ;
 struct kset devices ;
 struct klist klist_devices ;
 struct klist klist_drivers ;
 struct blocking_notifier_head bus_notifier ;
 struct bus_attribute *bus_attrs ;
 struct device_attribute *dev_attrs ;
 struct driver_attribute *drv_attrs ;
 struct bus_attribute drivers_autoprobe_attr ;
 struct bus_attribute drivers_probe_attr ;
 int (*match)(struct device *dev , struct device_driver *drv ) ;
 int (*uevent)(struct device *dev , char **envp , int num_envp , char *buffer ,
      int buffer_size ) ;
 int (*probe)(struct device *dev ) ;
 int (*remove)(struct device *dev ) ;
 void (*shutdown)(struct device *dev ) ;
 int (*suspend)(struct device *dev , pm_message_t state ) ;
 int (*suspend_late)(struct device *dev , pm_message_t state ) ;
 int (*resume_early)(struct device *dev ) ;
 int (*resume)(struct device *dev ) ;
 unsigned int drivers_autoprobe : 1 ;
};

struct device_driver {
 char const *name ;
 struct bus_type *bus ;
 struct kobject kobj ;
 struct klist klist_devices ;
 struct klist_node knode_bus ;
 struct module *owner ;
 char const *mod_name ;
 struct module_kobject *mkobj ;
 int (*probe)(struct device *dev ) ;
 int (*remove)(struct device *dev ) ;
 void (*shutdown)(struct device *dev ) ;
 int (*suspend)(struct device *dev , pm_message_t state ) ;
 int (*resume)(struct device *dev ) ;
};

struct driver_attribute {
 struct attribute attr ;
 ssize_t (*show)(struct device_driver * , char *buf ) ;
 ssize_t (*store)(struct device_driver * , char const *buf , size_t count ) ;
};

struct class {
 char const *name ;
 struct module *owner ;
 struct kset subsys ;

 struct class_device *node_next ;
 struct class_device *node_prev ;

 struct device *devices_next ;
 struct device *devices_prev ;

 struct class_interface *interfaces_next ;
 struct class_interface *interfaces_prev ;
 struct kset class_dirs ;
 struct semaphore sem ;
 struct class_attribute *class_attrs ;
 struct class_device_attribute *class_dev_attrs ;
 struct device_attribute *dev_attrs ;
 int (*uevent)(struct class_device *dev , char **envp , int num_envp , char *buffer ,
      int buffer_size ) ;
 int (*dev_uevent)(struct device *dev , char **envp , int num_envp , char *buffer ,
       int buffer_size ) ;
 void (*release)(struct class_device *dev ) ;
 void (*class_release)(struct class *class ) ;
 void (*dev_release)(struct device *dev ) ;
 int (*suspend)(struct device * , pm_message_t state ) ;
 int (*resume)(struct device * ) ;
};

struct class_attribute {
 struct attribute attr ;
 ssize_t (*show)(struct class * , char *buf ) ;
 ssize_t (*store)(struct class * , char const *buf , size_t count ) ;
};

struct class_device_attribute {
 struct attribute attr ;
 ssize_t (*show)(struct class_device * , char *buf ) ;
 ssize_t (*store)(struct class_device * , char const *buf , size_t count ) ;
};

struct class_device {

 struct class_device *node_next;
 struct class_device *node_prev;
 struct kobject kobj ;
 struct class *class ;
 dev_t devt ;
 struct class_device_attribute *devt_attr ;
 struct class_device_attribute uevent_attr ;
 struct device *dev ;
 void *class_data ;
 struct class_device *parent ;
 struct attribute_group **groups ;
 void (*release)(struct class_device *dev ) ;
 int (*uevent)(struct class_device *dev , char **envp , int num_envp , char *buffer ,
      int buffer_size ) ;
 char class_id[20] ;
};

struct class_interface {

 struct class_interface *interfaces_next;
 struct class_interface *interfaces_prev;
 struct class *class ;
 int (*add)(struct class_device * , struct class_interface * ) ;
 void (*remove)(struct class_device * , struct class_interface * ) ;
 int (*add_dev)(struct device * , struct class_interface * ) ;
 void (*remove_dev)(struct device * , struct class_interface * ) ;
};

struct device_type {
 char const *name ;
 struct attribute_group **groups ;
 int (*uevent)(struct device *dev , char **envp , int num_envp , char *buffer ,
      int buffer_size ) ;
 void (*release)(struct device *dev ) ;
 int (*suspend)(struct device *dev , pm_message_t state ) ;
 int (*resume)(struct device *dev ) ;
};

struct device_attribute {
 struct attribute attr ;
 ssize_t (*show)(struct device *dev , struct device_attribute *attr , char *buf ) ;
 ssize_t (*store)(struct device *dev , struct device_attribute *attr , char const *buf ,
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
 struct device_type *type ;
 unsigned int is_registered : 1 ;
 unsigned int uevent_suppress : 1 ;
 struct device_attribute uevent_attr ;
 struct device_attribute *devt_attr ;
 struct semaphore sem ;
 struct bus_type *bus ;
 struct device_driver *driver ;
 void *driver_data ;
 void *platform_data ;
 struct dev_pm_info power ;
 int numa_node ;
 u64 *dma_mask ;
 u64 coherent_dma_mask ;
 struct list_head dma_pools ;
 struct dma_coherent_mem *dma_mem ;
 struct dev_archdata archdata ;
 spinlock_t devres_lock ;
 struct list_head devres_head ;

 struct device *devices_next;
 struct device *devices_prev;
 struct class *class ;
 dev_t devt ;
 struct attribute_group **groups ;
 void (*release)(struct device *dev ) ;
};

ssize_t class_attr_show(struct kobject *kobj, struct attribute *attr, char *buf);
ssize_t class_attr_store(struct kobject *kobj, struct attribute *attr, char const *buf, size_t count);

ssize_t class_device_attr_show(struct kobject *kobj, struct attribute *attr, char *buf);
ssize_t class_device_attr_store(struct kobject *kobj, struct attribute *attr, char const *buf, size_t count);

int class_uevent_filter(struct kset *kset, struct class_device *cls_dev);
char const *class_uevent_name(struct kset *kset, struct class_device *cls_dev);
int class_uevent(struct kset *kset, struct class_device *cls_dev, char **envp, int num_envp, char *buffer, int buffer_size);

void class_release(struct kobject *kobj);
void class_dev_release(struct kobject *kobj);
void class_create_release(struct class *cls);

ssize_t store_uevent(struct class_device *class_dev, char const *buf, size_t count);

ssize_t show_dev(struct class_device *class_dev, char *buf);





struct sysfs_ops class_sysfs_ops = {
 &class_attr_show,
 &class_attr_store
};

struct kobj_type ktype_class = {
 &class_release, &class_sysfs_ops, (struct attribute **)0
};

struct kset class_subsys = {
 &ktype_class,
 {(struct list_head *)0, (struct list_head *)0},
 {{0U}},
 {
  (char const *)0,
  {
   (char )'c', (char )'l', (char )'a',
   (char )'s', (char )'s', (char )'\000',
   (char)0, (char)0, (char)0,
   (char)0, (char)0, (char)0,
   (char)0, (char)0, (char)0,
   (char)0, (char)0, (char)0,
   (char)0, (char)0
  },
  {{0}},
  {(struct list_head *)0, (struct list_head *)0},
  (struct kobject *)0, (struct kset *)0,
  (struct kobj_type *)0, (struct dentry *)0,
  {{{0U}},
  {(struct list_head *)0, (struct list_head *)0}}
 },
 (struct kset_uevent_ops *)((void *)0)
};

struct sysfs_ops class_dev_sysfs_ops = {
 &class_device_attr_show,
 &class_device_attr_store
};

struct kobj_type ktype_class_device = {
 &class_dev_release,
 &class_dev_sysfs_ops,
 (struct attribute **)0
};

struct kset_uevent_ops class_uevent_ops = {
 &class_uevent_filter,
 &class_uevent_name,
 &class_uevent
};

struct kset class_obj_subsys = {
 &ktype_class_device,
 {(struct list_head *)0, (struct list_head *)0},
 {{0U}},
 {
  (char const *)0,
  {
   (char )'c', (char )'l', (char )'a',
   (char )'s', (char )'s', (char )'_',
   (char )'o', (char )'b', (char )'j',
   (char )'\000', (char)0, (char)0,
   (char)0, (char)0, (char)0, (char)0,
   (char)0, (char)0, (char)0, (char)0
  },
  {{0}},
  {(struct list_head *)0, (struct list_head *)0},
  (struct kobject *)0, (struct kset *)0,
  (struct kobj_type *)0, (struct dentry *)0,
  {{{0U}}, {(struct list_head *)0, (struct list_head *)0}}
 },
 &class_uevent_ops
};


void *get_nondet_ptr()
{
 void *a;
 return a;
}

int get_nondet_int()
{
 int a;
 return a;
}

char *get_nondet_char_ptr()
{
 char *a;
 return a;
}

dev_t get_nondet_dev_t()
{
 dev_t a;
 return a;
}

struct bin_attribute *get_nondet_bin_attribute()
{
 struct bin_attribute *a;
 return a;
}

void kfree(void *p)
{
 if (p==0) return;
 free(p);
}

struct device *HsCreateDevice()
{
 struct device *dev;

 dev = (struct device *)malloc(sizeof(struct device));
 dev->bus = (struct bus_type *)malloc(sizeof(struct bus_type));
 dev->driver = (struct device_driver *)malloc(sizeof(struct device_driver));

 return dev;
}

void HsFreeDevice(struct device *dev)
{
 if (dev->bus) free(dev->bus);
 if (dev->driver) free(dev->driver);
 free(dev);
}

struct class_device *HsCreateClsDev()
{
 struct class_device *cls_dev;
 cls_dev = (struct class_device *)malloc(sizeof(struct class_device));
 cls_dev->dev = HsCreateDevice();
 cls_dev->kobj.kset = &class_obj_subsys;
 cls_dev->devt_attr = (struct class_device_attribute *)malloc(sizeof(struct class_device_attribute));
 return cls_dev;
}

void HsFreeClsDev(struct class_device *cls_dev)
{
 if (cls_dev->devt_attr != 0) free(cls_dev->devt_attr);
 if (cls_dev->dev != 0) HsFreeDevice(cls_dev->dev);
 free(cls_dev);
 return;
}

char *kobject_get_path(struct kobject *x, gfp_t y)
{
 char *a;
 if ((&nondet > 0)) {
  return 0;
 } else {
  a = (char *)malloc(sizeof(char));
  return a;
 }
}

int add_uevent_var(char **envp, int num_envp, int *cur_index, char *buffer,
     int buffer_size, int *cur_len, char const *format, void *x)
{
 int a; return a;
}

void down(struct semaphore *sem )
{
 return;
}

void up(struct semaphore *sem )
{
 return;
}

int subsystem_register(struct kset *x)
{
 if ((&nondet > 0)) return 0;
 else return -22;
}

void subsystem_init(struct kset *x)
{
 return;
}

void kset_init(struct kset *k)
{
 return;
}

void init_waitqueue_head(wait_queue_head_t *q)
{
 return;
}

int kobject_set_name(struct kobject *x, char *y, char const *z)
{
 if ((&nondet > 0)) { return 0; }
 return (-22);
}

void kobject_put(struct kobject * kobj)
{
 if (kobj)
  kobj->kref.refcount.counter--;
}

int kobject_uevent(struct kobject *kobj, enum kobject_action action)
{
 int a; return a;
}

int sysfs_create_file(struct kobject *x, struct attribute const *y)
{
 if ((&nondet > 0)) return 0;
 else return -22;
}

void sysfs_remove_file(struct kobject *x, struct attribute const *y)
{
 return;
}

int sysfs_create_bin_file(struct kobject *kobj, struct bin_attribute *attr)
{
 if ((&nondet > 0)) return 0;
 else return -22;
}

void sysfs_remove_bin_file(struct kobject *kobj, struct bin_attribute *attr)
{
 return;
}

void subsystem_unregister(struct kset *x)
{
 return;
}

int vsnprintf(char *buf, size_t size, char const *fmt, va_list args)
{
 int a; return a;
}

int sprintf(char *buf, char const *fmt, ...)
{
 int a; return a;
}

void kobject_init(struct kobject *x)
{
 return;
}

unsigned long strlen(char const *s)
{
 unsigned long a; return a;
}

char *strcpy(char *dest, char const *src)
{
 char *x; return x;
}

char *strcat(char *dest, char const *src)
{
 char *x; return x;
}

int kobject_add(struct kobject *kobj)
{
 if ((&nondet > 0)) { return 0; }
        return -22;
}

void kobject_del(struct kobject *kobj)
{
 return;
}

int sysfs_create_link(struct kobject *kobj, struct kobject *target, char const *name)
{
 if ((&nondet > 0)) { return 0; }
 return -22;
}

void sysfs_remove_link(struct kobject *x, char const *name)
{
 return;
}

int sysfs_create_group(struct kobject *x, struct attribute_group const *y)
{
 if ((&nondet > 0)) return 0;
 else return -22;
}

void sysfs_remove_group(struct kobject *x, struct attribute_group const *y)
{
 return;
}

void kref_get(struct kref *kref)
{
        kref->refcount.counter++;
 return;
}

int classes_init(void)
{
 int retval;

 retval = subsystem_register(&class_subsys);
 if (retval) {
  return (retval);
 }
 subsystem_init(&class_obj_subsys);
 if (!class_obj_subsys.kobj.parent) {
  class_obj_subsys.kobj.parent = &class_obj_subsys.kobj;
 }
 return (0);
}

void sema_init(struct semaphore *sem, int val)
{
 sem->count.counter = val;
 sem->sleepers = 0;
 init_waitqueue_head(&sem->wait);
 return;
}

void init_MUTEX(struct semaphore *sem)
{
 sema_init(sem, 1);
 return;
}

int class_create_file(struct class *cls, struct class_attribute const *attr)
{
 int error;

 if (cls) {
  error = sysfs_create_file(&cls->subsys.kobj, &attr->attr);
 } else {
  error = -22;
 }
 return (error);
}

void class_remove_file(struct class *cls, struct class_attribute const *attr)
{
 if (cls) {
  sysfs_remove_file(&cls->subsys.kobj, &attr->attr);
 }
 return;
}

int add_class_attrs(struct class *cls )
{
 int i;
 int error;

 error = 0;
 if (cls->class_attrs) {
  i = 0;
  while ((cls->class_attrs + i)->attr.name) {
   error = class_create_file(cls, (struct class_attribute const *)(cls->class_attrs+i));
   if (error) {
    goto Err;
   }
   i++;
  }
 }
Done:
 return (error);
Err:
 while (1) {
  i --;
  if (! (i >= 0)) {
   break;
  }
  class_remove_file(cls, (struct class_attribute const *)(cls->class_attrs+i));
 }
 goto Done;
}

struct kset *to_kset(struct kobject *kobj)
{
 struct kset *tmp ;
 struct kobject const *__mptr___0 ;

 if (kobj) {
  __mptr___0 = (struct kobject const *)kobj;
  tmp = (struct kset *)((char *)__mptr___0 - (unsigned int)(&((struct kset *)0)->kobj));
 } else {
  tmp = (struct kset *)((void *)0);
 }
 return (tmp);
}

struct kobject *kobject_get(struct kobject *kobj)
{
 if (kobj)
  kref_get(&kobj->kref);
 return kobj;
}

struct kset *kset_get(struct kset *k)
{
 struct kset *tmp___1 ;
 struct kobject *tmp___2 ;

 if (k) {
  tmp___2 = kobject_get(&k->kobj);
  tmp___1 = to_kset(tmp___2);
  tmp___1 = k;
 } else {
  tmp___1 = (struct kset *)((void *)0);
 }
 return (tmp___1);
}

struct kset *subsys_get(struct kset *s)
{
 struct kset *tmp ;

 if (s) {
  tmp = kset_get(s);
  return (tmp);
 }
 return ((struct kset *)((void *)0));
}

struct class *class_get(struct class *cls)
{
 struct kset const *__mptr;
 struct kset const *tmp;

 if (cls) {
  tmp = (struct kset const *)subsys_get(&cls->subsys);
  __mptr = tmp;

  return cls;
 }
 return ((struct class *)((void *)0));
}

void kset_put(struct kset *k)
{
 kobject_put(&k->kobj);
 return;
}

void subsys_put(struct kset *s)
{
 kset_put(s);
 return;
}

void class_put(struct class *cls)
{
 if (cls) {
  subsys_put(&cls->subsys);
 }
 return;
}

int pr_debug(char const *fmt, ...)
{
 return (0);
}

int class_register(struct class *cls)
{
 int error;
 struct class *tmp;

 pr_debug("device class \'%s\': registering\n", cls->name);
 cls->node_next = (struct class_device *)cls;
 cls->node_prev = (struct class_device *)cls;
 cls->devices_next = (struct device *)cls;
 cls->devices_prev = (struct device *)cls;
 cls->interfaces_next = (struct class_interface *)cls;
 cls->interfaces_prev = (struct class_interface *)cls;
 kset_init(&cls->class_dirs);
 init_MUTEX(&cls->sem);
 error = kobject_set_name(&cls->subsys.kobj, "%s", cls->name);
 if (error) {
  return (error);
 }
 cls->subsys.kobj.kset = &class_subsys;
 error = subsystem_register(&cls->subsys);
 if (!error) {
  tmp = class_get(cls);
  error = add_class_attrs(tmp);
  class_put(cls);
 }
 return(error);
}

void *ERR_PTR(long error)
{
 return 0;

}

void class_device_create_release(struct class_device *class_dev )
{
 pr_debug("%s called for %s\n", "class_device_create_release", class_dev->class_id);
 kfree(class_dev);
 return;
}

struct class *class_create(struct module *owner, char const *name)
{
 struct class *cls;
 int retval;
 struct class *tmp;

 cls = (struct class *)mallocnull(sizeof((*cls)));
 if (!cls) {
  retval = -22;
  goto error;
 }

 cls->name = name;
 cls->owner = owner;
 cls->class_release = &class_create_release;
 cls->release = &class_device_create_release;
 retval = class_register(cls);
 if (retval) {
  goto error;
 }
 return (cls);
error:
 kfree(cls);
 tmp = (struct class *)ERR_PTR((long)retval);
 return (tmp);
}

void remove_class_attrs(struct class *cls)
{
 int i ;

 if (cls->class_attrs) {
  i = 0;
  while ((cls->class_attrs + i)->attr.name) {
   class_remove_file(cls, (struct class_attribute const *)(cls->class_attrs + i));
   i ++;
  }
 }
 return;
}

void class_unregister(struct class *cls)
{
 pr_debug("device class \'%s\': unregistering\n", cls->name);
 remove_class_attrs(cls);
 subsystem_unregister(&cls->subsys);
 return;
}

long IS_ERR(void const *ptr)
{
 long tmp ;


 tmp = (ptr == 0);
 return (tmp);
}

void class_destroy(struct class *cls )
{
 long tmp ;

 if ((unsigned int )cls == (unsigned int )((void *)0)) {
  return;
 } else {
  tmp = IS_ERR((void const *)cls);
  if (tmp) {
   return;
  }
 }
 class_unregister(cls);
 return;
}

int class_device_create_uevent(struct class_device *class_dev,
 char **envp, int num_envp, char *buffer, int buffer_size)
{
 pr_debug("%s called for %s\n", "class_device_create_uevent", class_dev->class_id);
 return (0);
}

void class_device_initialize(struct class_device *class_dev)
{
 class_dev->kobj.kset = &class_obj_subsys;
 kobject_init(&class_dev->kobj);
 class_dev->node_next = class_dev;
 class_dev->node_prev = class_dev;
 return;
}

struct class_device *to_class_dev(struct kobject *obj)
{
 struct kobject const *__mptr ;

 __mptr = (struct kobject const *)obj;
 return ((struct class_device *)((char *)__mptr - (unsigned int )(& ((struct class_device *)0)->kobj)));
}

struct class_device *class_device_get(struct class_device *class_dev)
{
 struct kobject *tmp ;
 struct class_device *tmp___0 ;

 if (class_dev) {
  tmp = kobject_get(&class_dev->kobj);
  tmp___0 = to_class_dev(tmp);
  tmp___0 = class_dev;
  return (tmp___0);
 }
 return ((struct class_device *)((void *)0));
}

void class_device_put(struct class_device *class_dev)
{
 if (class_dev) {
  kobject_put(&class_dev->kobj);
 }
 return;
}

int class_device_create_file(struct class_device *class_dev, struct class_device_attribute const *attr)
{
 int error ;

 error = -22;
 if (class_dev) {
  error = sysfs_create_file(&class_dev->kobj, &attr->attr);
 }
 return (error);
}

void class_device_remove_file(struct class_device *class_dev, struct class_device_attribute const *attr)
{
 if (class_dev) {
  sysfs_remove_file(&class_dev->kobj, &attr->attr);
 }
 return;
}

int class_device_add_attrs(struct class_device *cd)
{
 int i;
 int error;
 struct class *cls;

 error = 0;
 cls = cd->class;
 if (cls->class_dev_attrs) {
  i = 0;
  while ((cls->class_dev_attrs + i)->attr.name) {
   error = class_device_create_file(cd, (struct class_device_attribute const *)(cls->class_dev_attrs + i));
   if (error) {
    goto Err;
   }
   i ++;
  }
 }
Done:
 return (error);
Err:
 while (1) {
  i --;
  if (! (i >= 0)) {
   break;
  }
  class_device_remove_file(cd, (struct class_device_attribute const *)(cls->class_dev_attrs + i));
 }
 goto Done;
}

void class_device_remove_attrs(struct class_device *cd)
{
 int i ;
 struct class *cls ;

 cls = cd->class;
 if (cls->class_dev_attrs) {
  i = 0;
  while ((cls->class_dev_attrs + i)->attr.name) {
   class_device_remove_file(cd, (struct class_device_attribute const *)(cls->class_dev_attrs + i));
   i ++;
  }
 }
 return;
}

int class_device_add_groups(struct class_device *cd )
{
 int i ;
 int error ;

 error = 0;
 if (cd->groups) {
  i = 0;
  while ((*(cd->groups + i))) {
   error = sysfs_create_group(& cd->kobj, (struct attribute_group const *)(*(cd->groups + i)));
   if (error) {
    while (1) {
     i --;
     if (! (i >= 0)) {
      break;
     }
     sysfs_remove_group(& cd->kobj, (struct attribute_group const *)(*(cd->groups + i)));
    }
    goto out;
   }
   i ++;
  }
 }
out:
 return (error);
}

char const *kobject_name(struct kobject const *kobj )
{
 return ((char const *)kobj->k_name);
}

char *make_class_name(char const *name, struct kobject *kobj)
{
 char *class_name ;
 int size ;
 unsigned long tmp ;
 char const *tmp___0 ;
 unsigned long tmp___1 ;
 char const *tmp___2 ;

 tmp = strlen(name);
 tmp___0 = kobject_name((struct kobject const *)kobj);
 tmp___1 = strlen(tmp___0);
 size = (int )((tmp + tmp___1) + 2UL);
 class_name = (char *)mallocnull(size);
 if (!class_name) {
  return ((char *)((void *)0));
 }
 strcpy(class_name, name);
 strcat(class_name, ":");
 tmp___2 = kobject_name((struct kobject const *)kobj);
 strcat(class_name, tmp___2);
 return (class_name);
}

int make_deprecated_class_device_links(struct class_device *class_dev)
{
 char *class_name ;
 int error ;

 if (! class_dev->dev) {
  return (0);
 }
 class_name = make_class_name((class_dev->class)->name, &class_dev->kobj);
 if (class_name) {
  error = sysfs_create_link(&(class_dev->dev)->kobj, &class_dev->kobj, (char const *)class_name);
 } else {
  error = -22;
 }
 kfree(class_name);
 return (error);
}

void class_device_remove_groups(struct class_device *cd)
{
 int i ;

 if (cd->groups) {
  i = 0;
  while ((*(cd->groups + i))) {
   sysfs_remove_group(&cd->kobj, (struct attribute_group const *)(*(cd->groups + i)));
   i ++;
  }
 }
 return;
}

void prefetch(void *x)
{

 return;
}

int class_device_add(struct class_device *class_dev)
{
 struct class *parent_class;
 struct class_device *parent_class_dev;
 struct class_interface *class_intf;
 int error;
 unsigned long tmp;
 struct class_device_attribute *attr;
 struct list_head const *__mptr ;
 struct list_head const *__mptr___0 ;

 parent_class = (struct class *)((void *)0);
 parent_class_dev = (struct class_device *)((void *)0);
 error = -22;
 class_dev = class_device_get(class_dev);
 if (!class_dev) {
  return (-22);
 }
 tmp = strlen((char const *)(class_dev->class_id));
 if (!tmp) {
  goto out1;
 }
 parent_class = class_get(class_dev->class);
 if (!parent_class) {
  goto out1;
 }
 parent_class_dev = class_device_get(class_dev->parent);
 pr_debug("CLASS: registering class device: ID = \'%s\'\n", class_dev->class_id);
 error = kobject_set_name(&class_dev->kobj, "%s", class_dev->class_id);
 if (error) {
  goto out2;
 }
 if (parent_class_dev) {
  class_dev->kobj.parent = &parent_class_dev->kobj;
 } else {
  class_dev->kobj.parent = &parent_class->subsys.kobj;
 }
 error = kobject_add(&class_dev->kobj);
 if (error) {
  goto out2;
 }
 error = sysfs_create_link(&class_dev->kobj, &parent_class->subsys.kobj, "subsystem");
 if (error) {
  goto out3;
 }
 class_dev->uevent_attr.attr.name = "uevent";
 class_dev->uevent_attr.attr.mode = 128U;
 class_dev->uevent_attr.attr.owner = parent_class->owner;
 class_dev->uevent_attr.store = &store_uevent;
 error = class_device_create_file(class_dev, (struct class_device_attribute const *)(&class_dev->uevent_attr));
 if (error) {
  goto out3;
 }
 if (class_dev->devt >> 20) {
  attr = (struct class_device_attribute *)mallocnull(sizeof((*attr)));
  if (!attr) {
   error = -22;
   goto out4;
  }
  attr->attr.name = "dev";
  attr->attr.mode = 292U;
  attr->attr.owner = parent_class->owner;
  attr->show = &show_dev;
  error = class_device_create_file(class_dev, (struct class_device_attribute const *)attr);
  if (error) {
   kfree(attr);
   goto out4;
  }
  class_dev->devt_attr = attr;
 } else {
  error = -22;
  goto out4;
 }
 error = class_device_add_attrs(class_dev);
 if (error) {
  goto out5;
 }
 if (class_dev->dev) {
  error = sysfs_create_link(&class_dev->kobj, &(class_dev->dev)->kobj, "device");
  if (error) {
   goto out6;
  }
 }
 error = class_device_add_groups(class_dev);
 if (error) {
  goto out7;
 }
 error = make_deprecated_class_device_links(class_dev);
 if (error) {
  goto out8;
 }
 kobject_uevent(&class_dev->kobj, 1);
 down(&parent_class->sem);
 class_dev->node_prev = parent_class->node_prev;
 class_dev->node_next = (struct class_device *)parent_class;
 parent_class->node_prev->node_next = class_dev;
 parent_class->node_prev = class_dev;
 class_intf = (struct class_interface *)parent_class->interfaces_next;
 while (1) {
  prefetch((void *)class_intf->interfaces_next);
  if (class_intf == (struct class_interface *)parent_class) {
   break;
  }
  if (class_intf->add) {

  }
  class_intf = class_intf->interfaces_next;
 }
 up(&parent_class->sem);
 goto out1;
out8:
 class_device_remove_groups(class_dev);
out7:
 if (class_dev->dev) {
  sysfs_remove_link(& class_dev->kobj, "device");
 }
out6:
 class_device_remove_attrs(class_dev);
out5:
 if (class_dev->devt_attr) {
  class_device_remove_file(class_dev, (struct class_device_attribute const *)class_dev->devt_attr);


  kfree(class_dev->devt_attr);
  class_dev->devt_attr = 0;
 }
out4:
 class_device_remove_file(class_dev, (struct class_device_attribute const *)(&class_dev->uevent_attr));
out3:
 kobject_del(&class_dev->kobj);
out2:
 if (parent_class_dev) {
  class_device_put(parent_class_dev);
 }
 class_put(parent_class);
out1:
 class_device_put(class_dev);
 return (error);
}

int class_device_register(struct class_device *class_dev)
{
 int tmp;

 class_device_initialize(class_dev);
 tmp = class_device_add(class_dev);
 return (tmp);
}

struct class_device *class_device_create(struct class *cls,
  struct class_device *parent, dev_t devt,
  struct device *device, char const *fmt, va_list args)
{
 struct class_device *class_dev;
 int retval;
 long tmp;
 struct class_device *tmp___0;

 class_dev = (struct class_device *)((void *)0);
 retval = -22;
 if ((unsigned int )cls == (unsigned int )((void *)0)) {
  goto error;
 } else {
  tmp = IS_ERR((void const *)cls);
  if (tmp) {
   goto error;
  }
 }

 class_dev = (struct class_device *)mallocnull(sizeof((*class_dev)));
 if (!class_dev) {
  retval = -22;
  goto error;
 }
 class_dev->devt = devt;
 class_dev->dev = device;
 class_dev->class = cls;
 class_dev->parent = parent;
 class_dev->release = &class_device_create_release;
 class_dev->uevent = &class_device_create_uevent;

 // BUG:
 // field devt_attr is not initialized.
 // Without this initialization, the devt_attr field of class_dev
 // can be dangling or point to an allocated struct. This can later 
 // cause other problems, such as memory leaks or dereference errors.
 class_dev->devt_attr = 0;

 0;
 vsnprintf(class_dev->class_id, 20UL, fmt, args);
 0;
 retval = class_device_register(class_dev);
 if (retval) {
  goto error;
 }
 return (class_dev);
error:
 kfree(class_dev);
 tmp___0 = (struct class_device *)ERR_PTR((long )retval);
 return (tmp___0);
}

void remove_deprecated_class_device_links(struct class_device *class_dev)
{
 char *class_name;

 if (!class_dev->dev) {
  return;
 }
 class_name = make_class_name((class_dev->class)->name, &class_dev->kobj);
 if (class_name) {
  sysfs_remove_link(& (class_dev->dev)->kobj, (char const *)class_name);
 }
 kfree(class_name);
 return;
}

void class_device_del(struct class_device *class_dev)
{
 struct class *parent_class;
 struct class_device *parent_device;
 struct class_interface *class_intf;

 parent_class = class_dev->class;
 parent_device = class_dev->parent;
 if (parent_class) {
  down(& parent_class->sem);
  class_dev->node_prev->node_next = class_dev->node_next;
  class_dev->node_next->node_prev = class_dev->node_prev;
  class_intf = parent_class->interfaces_next;
  while (1) {
   prefetch((void *)class_intf->interfaces_next);
   if (class_intf == (struct class_interface *)parent_class) {
    break;
   }
   if (class_intf->remove) {

   }
   class_intf = class_intf->interfaces_next;
  }
  up(&parent_class->sem);
 }
 if (class_dev->dev) {
  remove_deprecated_class_device_links(class_dev);
  sysfs_remove_link(&class_dev->kobj, "device");
 }
 sysfs_remove_link(& class_dev->kobj, "subsystem");
 class_device_remove_file(class_dev, (struct class_device_attribute const *)(& class_dev->uevent_attr));
 if (class_dev->devt_attr) {
  class_device_remove_file(class_dev, (struct class_device_attribute const *)class_dev->devt_attr);
 }
 class_device_remove_attrs(class_dev);
 class_device_remove_groups(class_dev);
 kobject_uevent(&class_dev->kobj, 2);
 kobject_del(&class_dev->kobj);
 class_device_put(parent_device);
 class_put(parent_class);
 return;
}

void class_device_unregister(struct class_device *class_dev)
{
 pr_debug("CLASS: Unregistering class device. ID = \'%s\'\n", class_dev->class_id);
 class_device_del(class_dev);
 class_device_put(class_dev);




 HsFreeClsDev(class_dev);

 return;
}

void class_device_destroy(struct class *cls, dev_t devt)
{
 struct class_device *class_dev;
 struct class_device *class_dev_tmp;

 class_dev = (struct class_device *)((void *)0);
 down(&cls->sem);
 class_dev_tmp = cls->node_next;
 while (1) {
  prefetch((void *)class_dev_tmp->node_next);
  if (class_dev_tmp == (struct class_device *)cls) {
   break;
  }
  if (class_dev_tmp->devt == devt) {
   class_dev = class_dev_tmp;
   break;
  }
  class_dev_tmp = class_dev_tmp->node_next;
 }
 up(& cls->sem);
 if (class_dev) {
  class_device_unregister(class_dev);
 }
 return;
}

int class_device_create_bin_file(struct class_device *class_dev, struct bin_attribute *attr)
{
 int error ;
 error = -22;
 if (class_dev) {
  error = sysfs_create_bin_file(& class_dev->kobj, attr);
 }
 return (error);
}

void class_device_remove_bin_file(struct class_device *class_dev , struct bin_attribute *attr)
{
 if (class_dev) {
  sysfs_remove_bin_file(& class_dev->kobj, attr);
 }
 return;
}

int class_interface_register(struct class_interface *class_intf)
{
 struct class *parent;
 struct class_device *class_dev;
 struct device *dev;

 if (!class_intf) {
  return (-22);
 } else {
  if (!class_intf->class) {
   return (-22);
  }
 }
 parent = class_get(class_intf->class);
 if (!parent) {
  return (-22);
 }
 down(&parent->sem);
 class_intf->interfaces_next = (struct class_interface *)parent;
 class_intf->interfaces_prev = parent->interfaces_prev;
 parent->interfaces_prev->interfaces_next = class_intf;
 parent->interfaces_prev = class_intf;

 if (class_intf->add) {
  class_dev = parent->node_next;
  while (1) {
   prefetch((void *)class_dev->node_next);
   if (class_dev == (struct class_device *)parent) {
    break;
   }

   class_dev = class_dev->node_next;
  }
 }
 if (class_intf->add_dev) {
  dev = parent->devices_next;
  while (1) {
   prefetch((void *)dev->devices_next);
   if (dev == (struct device *)parent) {
    break;
   }

   dev = dev->devices_next;
  }
 }
 up(& parent->sem);
 return (0);
}

void class_interface_unregister(struct class_interface *class_intf)
{
 struct class *parent;
 struct class_device *class_dev;
 struct device *dev;

 parent = class_intf->class;
 if (!parent) {
  return;
 }
 down(&parent->sem);

 class_intf->interfaces_next->interfaces_prev = class_intf->interfaces_prev;
 class_intf->interfaces_prev->interfaces_next = class_intf->interfaces_next;

 if (class_intf->remove) {
  class_dev = parent->node_next;
  while (1) {
   prefetch((void *)class_dev->node_next);
   if (class_dev == (struct class_device *)parent) {
    break;
   }

   class_dev = class_dev->node_next;
  }
 }
 if (class_intf->remove_dev) {
  dev = parent->devices_next;
  while (1) {
   prefetch((void *)dev->devices_next);
   if (dev == (struct device *)parent) {
    break;
   }

   dev = dev->devices_next;
  }
 }
 up(& parent->sem);
 class_put(parent);
 return;
}

ssize_t store_uevent(struct class_device *class_dev, char const *buf, size_t count)
{
 kobject_uevent(&class_dev->kobj, 1);
 return ((long )count);
}

ssize_t show_dev(struct class_device *class_dev, char *buf)
{
 ssize_t tmp ;

 tmp = (ssize_t )sprintf(buf, "%u:%u\n", class_dev->devt >> 20, class_dev->devt & 1048575U);
 return (tmp);
}

char const *class_uevent_name(struct kset *kset, struct class_device *class_dev)
{
 return ((class_dev->class)->name);
}

struct kobj_type *get_ktype(struct kobject *k)
{
 if (k->kset) {
  if ((k->kset)->ktype) {
   return ((k->kset)->ktype);
  } else {
   return (k->ktype);
  }
 } else {
  return (k->ktype);
 }
}

int class_uevent_filter(struct kset *kset, struct class_device *class_dev)
{
 struct kobj_type *ktype ;
 struct kobj_type *tmp ;

 tmp = get_ktype(&class_dev->kobj);
 ktype = tmp;
 if ((unsigned int )ktype == (unsigned int )(& ktype_class_device)) {

  if (class_dev->class) {
   return (1);
  }

 }
 return (0);
}

struct class* HsCreateModel()
{
 struct module *owner;
 char *name;

 struct class *res;
 struct class_device *curr;
 struct device *dev;
 struct class_interface *intf;

 res = class_create(owner, name);
 if (res == 0) return 0;

 while((&nondet > 0)) {
  curr = HsCreateClsDev();
  curr->parent = 0;
  curr->class = res;
  curr->node_next = res->node_next;
  curr->node_prev = (struct class_device *)res;
  res->node_next->node_prev = curr;
  res->node_next = curr;
 }

 while((&nondet > 0)) {
  dev = (struct device *)malloc(sizeof(struct device));
  dev->devices_next = res->devices_next;
  dev->devices_prev = (struct device *)res;
  res->devices_next->devices_prev = dev;
  res->devices_next = dev;
 }

 while((&nondet > 0)) {
  intf = (struct class_interface *)malloc(sizeof(struct class_interface));
  intf->class = res;
  intf->interfaces_next = res->interfaces_next;
  intf->interfaces_prev = (struct class_interface *)res;
  res->interfaces_next->interfaces_prev = intf;
  res->interfaces_next = intf;
 }

 return res;
}

void class_create_release(struct class *cls)
{
 pr_debug("%s called for %s\n", "class_create_release", cls->name);
 kfree(cls);
 return;
}

void HsDisposeModel(struct class *cls)
{
 struct class_device *curr;
 struct device *dev;
 struct class_interface *intf;

 while (cls->node_next != (struct class_device *)cls) {
  curr = cls->node_next;
  cls->node_next->node_prev = (struct class_device *)cls;
  cls->node_next = curr->node_next;
  HsFreeClsDev(curr);
 }

 while (cls->interfaces_next != (struct class_interface *)cls) {
  intf = cls->interfaces_next;
  cls->interfaces_next->interfaces_prev = (struct class_interface *)cls;
  cls->interfaces_next = intf->interfaces_next;
  free(intf);
 }

 while (cls->devices_next != (struct device *)cls) {
  dev = cls->devices_next;
  cls->devices_next->devices_prev = (struct device *)cls;
  cls->devices_next = dev->devices_next;
  free(dev);
 }

 class_destroy(cls);
 class_create_release(cls);
}

struct class *main_sub()
{
 struct class *model;

 char *fmt, *buf;
 dev_t devt;
 struct device *device;
 va_list args;
 size_t count;

 struct class_device *cls_dev;
 struct class_interface *intf;
 struct bin_attribute *attr;

 struct kset *kset;
 char **envp;
 int num_envp;

 model = HsCreateModel();
 if (model == 0) { return 0; }

 while ((&nondet > 0)) {
  if ((&nondet > 0)) {
   classes_init();
  }
  else if ((&nondet > 0)) {
   device = HsCreateDevice();
   cls_dev = class_device_create(model, 0, devt, device, fmt, args);
   if (cls_dev == 0) { HsFreeDevice(device); }
  }
  else if ((&nondet > 0)) {
   devt = get_nondet_dev_t();
   class_device_destroy(model, devt);
  }
  else if ((&nondet > 0)) {
   device = HsCreateDevice();
   cls_dev = class_device_create(model, 0, devt, device, fmt, args);
   attr = get_nondet_bin_attribute();
   if (cls_dev != 0) {
    class_device_create_bin_file(cls_dev, attr);
   } else {
    HsFreeDevice(device);
   }
  }
  else if ((&nondet > 0)) {
   device = HsCreateDevice();
   cls_dev = class_device_create(model, 0, devt, device, fmt, args);
   attr = get_nondet_bin_attribute();
   if (cls_dev != 0) {
    class_device_remove_bin_file(cls_dev , attr);
   } else {
    HsFreeDevice(device);
   }
  }
  else if ((&nondet > 0)) {
   intf = (struct class_interface *)malloc(sizeof(struct class_interface));
   if ((&nondet > 0)) {
    intf->class = model;
    class_interface_register(intf);
   }
   else {
    intf->class = 0;
    class_interface_register(intf);
    free(intf);
   }
  }
  else if ((&nondet > 0)) {
   intf = model->interfaces_next;
   while (intf != (struct class_interface *)model) {
    if ((&nondet > 0)) {
     class_interface_unregister(intf);
     free(intf);
     break;
    }
    intf = intf->interfaces_next;
   }
  }
  else if ((&nondet > 0)) {
   cls_dev = model->node_next;
   while (cls_dev != (struct class_device *)model) {
    if ((&nondet > 0)) {
     count = get_nondet_int();
     num_envp = get_nondet_int();
     buf = get_nondet_char_ptr();
     envp = get_nondet_ptr();
     kset = get_nondet_ptr();
     if ((&nondet > 0)) store_uevent(cls_dev, buf, count);
     if ((&nondet > 0)) show_dev(cls_dev, buf);
     if ((&nondet > 0)) class_uevent_filter(kset, cls_dev);
     if ((&nondet > 0)) class_uevent_name(kset, cls_dev);
    }
    cls_dev = cls_dev->node_next;
   }
  }
 }

 HsDisposeModel(model);
 return model;
}

int main()
{
 struct class *model;
 model = main_sub();
 return 0;
}
