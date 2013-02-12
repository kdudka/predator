#include <verifier-builtins.h>
#include <stdlib.h>
#include <stdio.h>

static void *malloc_wrap(unsigned size)
{
    __VERIFIER_plot(NULL);
    void *ptr = malloc(size);
    if (ptr)
        return ptr;
    else
        abort();
}

#define NEW(type) ((type *)malloc_wrap(sizeof(type)))

#ifdef PREDATOR
#   define printf(...)
#endif

struct cdrom_device_info {
    struct cdrom_device_ops *ops;
    struct cdrom_device_info *next;
    struct gendisk *disk;
    int mask;
    int options : 30;
    unsigned int mc_flags : 2;
    char name[20];
    int cdda_method;
    int (*exit)(struct cdrom_device_info *);
};

struct cdrom_device_ops {
    int (*open)(struct cdrom_device_info *, int);
    void (*release)(struct cdrom_device_info *);
    int (*drive_status)(struct cdrom_device_info *, int);
    int (*media_changed)(struct cdrom_device_info *, int);
    int (*tray_move)(struct cdrom_device_info *, int);
    int (*lock_door)(struct cdrom_device_info *, int);
    int (*select_speed)(struct cdrom_device_info *, int);
    int (*get_last_session)(struct cdrom_device_info *, void *);
    int (*get_mcn)(struct cdrom_device_info *, void *);
    int (*reset)(struct cdrom_device_info *);
    int (*audio_ioctl)(struct cdrom_device_info *, unsigned int, void *);
    int (*dev_ioctl)(struct cdrom_device_info *, unsigned int, unsigned long);
    int capability;
    int n_minors;
    int (*generic_packet)(struct cdrom_device_info *, void *);
};

typedef struct ctl_table ctl_table;
struct ctl_table {
    int ctl_name;
    ctl_table *child;
    struct proc_dir_entry *de;
};

struct ctl_table_header {
    ctl_table *ctl_table;
};

struct proc_dir_entry {
    struct module *owner;
    struct proc_dir_entry *next;
    struct proc_dir_entry *parent;
    struct proc_dir_entry *subdir;
};

struct request {
    struct request_sense *sense;
};

struct cdrom_sysctl_settings {
    int autoclose;
    int autoeject;
    int debug;
    int lock;
    int check;
};

static int debug;
static int autoclose = 1;
static int autoeject;
static int lockdoor = 1;
static int check_media_type;

static struct cdrom_sysctl_settings cdrom_sysctl_settings;
static ctl_table *cdrom_root_table;
static struct ctl_table_header *cdrom_sysctl_header;
static int initialized;

static struct cdrom_device_info *topCdromPtr;
static char banner_printed;

struct ctl_table_header *register_sysctl_table(ctl_table *table, int insert_at_head)
{
    struct ctl_table_header *cdrom_sysctl_header = NEW(struct ctl_table_header);
    (void) insert_at_head;

    if (cdrom_sysctl_header)
        cdrom_sysctl_header->ctl_table = table;

    return cdrom_sysctl_header;
}

static int cdrom_sysctl_register(void)
{
    if (initialized == 1)
        return 0;

    cdrom_sysctl_header = register_sysctl_table(cdrom_root_table, 1);
    if (cdrom_sysctl_header == 0)
        return -2;

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

static int cdrom_dummy_generic_packet(struct cdrom_device_info *cdi, void *cgc)
{
    (void) cdi;
    (void) cgc;
    return -5;
}

static int cdrom_mrw_exit(struct cdrom_device_info *cdi)
{
    (void) cdi;
    return -1;
}

int register_cdrom(struct cdrom_device_info *cdi)
{
    struct cdrom_device_ops *cdo = cdi->ops;
    int *change_capability = (int *)(& cdo->capability);

    if (debug == 1) {
        printf("<6>cdrom: entering register_cdrom\n");
    }

    if (cdo->open == ((void *)0)) {
        return -2;
    } else {
        if (cdo->release == ((void *)0)) {
            return -2;
        }
    }

    if (! banner_printed) {
        printf("<6>Uniform CD-ROM driver Revision: 3.20\n");
        banner_printed = (char)1;
        cdrom_sysctl_register();
    }

    if (cdo->drive_status == ((void *)0)) {
        (*change_capability) &= -2049;
    }
    if (cdo->media_changed == ((void *)0)) {
        (*change_capability) &= -129;
    }
    if (cdo->tray_move == ((void *)0)) {
        (*change_capability) &= -4;
    }
    if (cdo->lock_door == ((void *)0)) {
        (*change_capability) &= -5;
    }
    if (cdo->select_speed == ((void *)0)) {
        (*change_capability) &= -9;
    }
    if (cdo->get_last_session == ((void *)0)) {
        (*change_capability) &= -33;
    }
    if (cdo->get_mcn == ((void *)0)) {
        (*change_capability) &= -65;
    }
    if (cdo->reset == ((void *)0)) {
        (*change_capability) &= -513;
    }
    if (cdo->audio_ioctl == ((void *)0)) {
        (*change_capability) &= -257;
    }
    if (cdo->dev_ioctl == ((void *)0)) {
        (*change_capability) &= -1025;
    }
    if (cdo->generic_packet == ((void *)0)) {
        (*change_capability) &= -4097;
    }

    cdi->mc_flags = 0U;
    cdo->n_minors = 0;
    cdi->options = 4;

    if (autoclose == 1) {
        if (((cdi->ops)->capability & (int)(~ cdi->mask)) & 1) {
            cdi->options |= 1;
        }
    }
    if (autoeject == 1) {
        if (((cdi->ops)->capability & (int)(~ cdi->mask)) & 2) {
            cdi->options |= 2;
        }
    }
    if (lockdoor == 1) {
        cdi->options |= 8;
    }
    if (check_media_type == 1) {
        cdi->options |= 16;
    }
    if (((cdi->ops)->capability & (int)(~ cdi->mask)) & 1048576) {
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
        printf("<6>cdrom: drive \"/dev/%s\" registered\n", cdi->name);
    }

    cdi->next = topCdromPtr;
    topCdromPtr = cdi;
    return 0;
}

struct cdrom_device_info *HsCreateCdromDeviceInfo(void)
{
    struct cdrom_device_info *cdi = NEW(struct cdrom_device_info);

    cdi->ops = NEW(struct cdrom_device_ops);
    cdi->ops->generic_packet = 0;

    return cdi;
}

struct ctl_table *HsCreateCtlTable(int name)
{
    struct ctl_table *tbl = NEW(struct ctl_table);

    tbl->ctl_name = name;
    tbl->child = 0;
    tbl->de = NEW(struct proc_dir_entry);
    tbl->de->next = 0;
    tbl->de->parent = 0;
    tbl->de->subdir = 0;

    return tbl;
}

int get_nondet_int(void)
{
    __VERIFIER_plot(NULL);
    int a;
    return a;
}

int main()
{
    cdrom_root_table = HsCreateCtlTable(0);
    while((get_nondet_int() > 0)) {
        struct ctl_table *tbl = HsCreateCtlTable(1);
        tbl->child = cdrom_root_table->child;
        cdrom_root_table->child = tbl;
    }

    while((get_nondet_int() > 0)) {
        struct cdrom_device_info *cdi = HsCreateCdromDeviceInfo();
        cdi->next = topCdromPtr;
        topCdromPtr = cdi;
    }

    struct cdrom_device_info *cdi = HsCreateCdromDeviceInfo();
    return register_cdrom(cdi);
}
