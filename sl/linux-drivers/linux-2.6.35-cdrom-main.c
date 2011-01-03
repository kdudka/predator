// vim: tw=128
// simple driver for cdrom.c
// - inspired by invader cdrom.c example

// ===========================================================================================================================
#include "linux-2.6.35-cdrom.c"
// ===========================================================================================================================
#define NULL ((void*)0)

// generate nondeterministic int, bool, and pointer
int get_nondet_int(void)        { int a; return a; }
bool nondet(void)               { return get_nondet_int(); }
void *get_nondet_ptr(void)      { void *a; return a; }

// declaration of intrinsics
extern __attribute__((alloc_size(1))) void* malloc(size_t);
extern void free(const void*);
extern __attribute__((noreturn)) void abort(void);

#define NEW(type) ({                    \
    void *ptr = malloc(sizeof(type));   \
    if (!ptr)                           \
        abort();                        \
    (type *) ptr;                       \
});

// TODO: add details
struct cdrom_device_info *HsCreateCdromDeviceInfo(void)
{
 struct cdrom_device_info *cdi;

 cdi = NEW(struct cdrom_device_info);
 cdi->ops = NEW(struct cdrom_device_ops);
 cdi->disk = NEW(struct gendisk);
 cdi->disk->queue = NEW(struct request_queue);
 cdi->disk->queue->boundary_rq = NEW(struct request);
 cdi->disk->queue->boundary_rq->sense = NEW(struct request_sense);

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

void HsInitialize(void)
{
    while (nondet()) {
        struct cdrom_device_info *cdi;
        cdi = HsCreateCdromDeviceInfo();
        register_cdrom(cdi);
    }
    return;
}

int main_sub(void)
{
    int tmp;
    unsigned int cmd;
    unsigned long arg;
    struct cdrom_device_info *cdi;
    struct block_device *bdev;

    HsInitialize();

    tmp = cdrom_init();
    if (tmp != 0)
        return (tmp);

    cdi = HsCreateCdromDeviceInfo();
    tmp = register_cdrom(cdi);
    if (tmp != 0) {
        HsFreeCdromDeviceInfo(cdi);
        return (tmp);
    }


#if 0 // short test
            bdev = NEW(struct block_device);
            cdrom_open(cdi, bdev, (fmode_t)0);
            free(bdev);
            cdrom_release(cdi, (fmode_t)0);
#else
    while (nondet()) {
        if (nondet()) {
            struct media_event_desc *med;
            med = get_nondet_ptr();
            tmp = cdrom_get_media_event(cdi, med);
        } else if (nondet()) {
            bdev = NEW(struct block_device);
            cdrom_open(cdi, bdev, (fmode_t)0);
            free(bdev);
        } else if (nondet()) {
            cdrom_release(cdi, (fmode_t)0);
        } else if (nondet()) {
            tmp = cdrom_number_of_slots(cdi);
        } else if (nondet()) {
            cmd = get_nondet_int();
            arg = get_nondet_int();
            tmp = mmc_ioctl(cdi, cmd, arg);
        } else if (nondet()) {
            bdev = NEW(struct block_device);
            cmd = get_nondet_int();
            arg = get_nondet_int();
            tmp = cdrom_ioctl(cdi, bdev, (fmode_t)0, cmd, arg);
            free(bdev);
        } else if (nondet()) {
            tmp = cdrom_media_changed(cdi);
        }
    }
#endif

    unregister_cdrom(cdi);
    cdrom_exit();
    return (0);
}

int main(void)
{
    return main_sub();
}

// ===========================================================================================================================
#include "linux-2.6.35--common-functions.c"
// ===========================================================================================================================
