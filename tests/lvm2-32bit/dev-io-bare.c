struct winsize
  {
    unsigned short int ws_row;
    unsigned short int ws_col;
    unsigned short int ws_xpixel;
    unsigned short int ws_ypixel;
  };


struct termio
  {
    unsigned short int c_iflag;
    unsigned short int c_oflag;
    unsigned short int c_cflag;
    unsigned short int c_lflag;
    unsigned char c_line;
    unsigned char c_cc[8];
};
/* # 31 "/usr/include/sys/ioctl.h" 2 3 4 */






/* # 1 "/usr/include/sys/ttydefaults.h" 1 3 4 */
/* # 38 "/usr/include/sys/ioctl.h" 2 3 4 */




extern int ioctl (int __fd, unsigned long int __request, ...) __attribute__ ((__nothrow__));


/* # 29 "device/dev-io.c" 2 */





/* # 1 "/usr/include/linux/fs.h" 1 3 4 */
/* # 11 "/usr/include/linux/fs.h" 3 4 */
/* # 1 "/usr/include/linux/blk_types.h" 1 3 4 */
/* # 118 "/usr/include/linux/blk_types.h" 3 4 */
enum rq_flag_bits {

 __REQ_WRITE,
 __REQ_FAILFAST_DEV,
 __REQ_FAILFAST_TRANSPORT,
 __REQ_FAILFAST_DRIVER,

 __REQ_SYNC,
 __REQ_META,
 __REQ_DISCARD,
 __REQ_NOIDLE,


 __REQ_RAHEAD,
 __REQ_THROTTLED,



 __REQ_SORTED,
 __REQ_SOFTBARRIER,
 __REQ_FUA,
 __REQ_NOMERGE,
 __REQ_STARTED,
 __REQ_DONTPREP,
 __REQ_QUEUED,
 __REQ_ELVPRIV,
 __REQ_FAILED,
 __REQ_QUIET,
 __REQ_PREEMPT,
 __REQ_ALLOCED,
 __REQ_COPY_USER,
 __REQ_FLUSH,
 __REQ_FLUSH_SEQ,
 __REQ_IO_STAT,
 __REQ_MIXED_MERGE,
 __REQ_SECURE,
 __REQ_ON_PLUG,
 __REQ_NR_BITS,
};
/* # 12 "/usr/include/linux/fs.h" 2 3 4 */
/* # 36 "/usr/include/linux/fs.h" 3 4 */
struct fstrim_range {
 __u64 start;
 __u64 len;
 __u64 minlen;
};


struct files_stat_struct {
 unsigned long nr_files;
 unsigned long nr_free_files;
 unsigned long max_files;
};

struct inodes_stat_t {
 int nr_inodes;
 int nr_unused;
 int dummy[5];
};
/* # 35 "device/dev-io.c" 2 */
/* # 54 "device/dev-io.c" */
static struct dm_list _open_devices = { &(_open_devices), &(_open_devices) };





static int _io(struct device_area *where, char *buffer, int should_write)
{
 int fd = dev_fd(where->dev);
 ssize_t n = 0;
 size_t total = 0;

 if (fd < 0) {
  print_log(3,
 "device/dev-io.c"
/* # 67 "device/dev-io.c" */
  ,
 68
/* # 67 "device/dev-io.c" */
  , -1,"Attempt to read an unopened device (%s).", dev_name(where->dev))
                          ;
  return 0;
 }




 if (should_write && test_mode())
  return 1;

 if (where->size > 2147483647L) {
  print_log(3, "device/dev-io.c", 79 , -1,"Read size too large: %" "ll" "u", where->size);
  return 0;
 }

 if (lseek(fd, (off_t) where->start, 0) < 0) {
  print_log(3,

 "device/dev-io.c"
/* # 84 "device/dev-io.c" */
  ,

 86
/* # 84 "device/dev-io.c" */
  , -1,"%s: lseek %" "ll" "u" " failed: %s", dev_name(where->dev), (uint64_t) where->start, strerror((*__errno_location ())))

                     ;
  return 0;
 }

 while (total < (size_t) where->size) {
  do
   n = should_write ?
       write(fd, buffer, (size_t) where->size - total) :
       read(fd, buffer, (size_t) where->size - total);
  while ((n < 0) && (((*__errno_location ()) == 4) || ((*__errno_location ()) == 11)));

  if (n < 0)
   print_log(3 | 256,




 "device/dev-io.c"
/* # 98 "device/dev-io.c" */
   ,




 103
/* # 98 "device/dev-io.c" */
   , -1,"%s: %s failed after %" "ll" "u" " of %" "ll" "u" " at %" "ll" "u" ": %s", dev_name(where->dev), should_write ? "write" : "read", (uint64_t) total, (uint64_t) where->size, (uint64_t) where->start, strerror((*__errno_location ())))




                                                    ;

  if (n <= 0)
   break;

  total += n;
  buffer += n;
 }

 return (total == (size_t) where->size);
}
/* # 125 "device/dev-io.c" */
static int _get_block_size(struct device *dev, unsigned int *size)
{
 const char *name = dev_name(dev);

 if (dev->block_size == -1) {
  if (ioctl(dev_fd(dev), (((2U) << (((0 +8)+8)+14)) | (((0x12)) << (0 +8)) | (((112)) << 0) | ((((sizeof(size_t)))) << ((0 +8)+8))), &dev->block_size) < 0) {
   print_log(3, "device/dev-io.c", 131 , -1,"%s: %s failed: %s", name, "ioctl BLKBSZGET", strerror((*__errno_location ())));
   return 0;
  }
  print_log(7, "device/dev-io.c", 134 , 0,"%s: block size is %u bytes", name, dev->block_size);
 }

 *size = (unsigned int) dev->block_size;

 return 1;
}




static void _widen_region(unsigned int block_size, struct device_area *region,
     struct device_area *result)
{
 uint64_t mask = block_size - 1, delta;
 memcpy(result, region, sizeof(*result));


 delta = result->start & mask;
 if (delta) {
  result->start -= delta;
  result->size += delta;
 }


 delta = (result->start + result->size) & mask;
 if (delta)
  result->size += block_size - delta;
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

 if (!block_size)
  block_size = lvm_getpagesize();

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
 return r;
}

static int _dev_get_size_file(const struct device *dev, uint64_t *size)
{
 const char *name = dev_name(dev);
 struct stat info;

 if (stat(name, &info)) {
  print_log(3, "device/dev-io.c", 235 , -1,"%s: %s failed: %s", name, "stat", strerror((*__errno_location ())));
  return 0;
 }

 *size = info.st_size;
 *size >>= 9L;

 print_log(6, "device/dev-io.c", 242 , 0,"%s: size is %" "ll" "u" " sectors", name, *size);

 return 1;
}

static int _dev_get_size_dev(const struct device *dev, uint64_t *size)
{
 int fd;
 const char *name = dev_name(dev);

 if ((fd = open(name, 00)) < 0) {
  print_log(3, "device/dev-io.c", 253 , -1,"%s: %s failed: %s", name, "open", strerror((*__errno_location ())));
  return 0;
 }

 if (ioctl(fd, (((2U) << (((0 +8)+8)+14)) | (((0x12)) << (0 +8)) | (((114)) << 0) | ((((sizeof(size_t)))) << ((0 +8)+8))), size) < 0) {
  print_log(3, "device/dev-io.c", 258 , -1,"%s: %s failed: %s", name, "ioctl BLKGETSIZE64", strerror((*__errno_location ())));
  if (close(fd))
   print_log(3, "device/dev-io.c", 260 , -1,"%s: %s failed: %s", name, "close", strerror((*__errno_location ())));
  return 0;
 }

 *size >>= 9L;
 if (close(fd))
  print_log(3, "device/dev-io.c", 266 , -1,"%s: %s failed: %s", name, "close", strerror((*__errno_location ())));

 print_log(6, "device/dev-io.c", 268 , 0,"%s: size is %" "ll" "u" " sectors", name, *size);

 return 1;
}

static int _dev_read_ahead_dev(struct device *dev, uint32_t *read_ahead)
{
 long read_ahead_long;

 if (dev->read_ahead != -1) {
  *read_ahead = (uint32_t) dev->read_ahead;
  return 1;
 }

 if (!dev_open(dev))
  do { print_log(7, "device/dev-io.c", 283 , 0,"<backtrace>"); return 0; } while (0);

 if (ioctl(dev->fd, (((0U) << (((0 +8)+8)+14)) | (((0x12)) << (0 +8)) | (((99)) << 0) | ((0) << ((0 +8)+8))), &read_ahead_long) < 0) {
  print_log(3, "device/dev-io.c", 286 , -1,"%s: %s failed: %s", dev_name(dev), "ioctl BLKRAGET", strerror((*__errno_location ())));
  if (!dev_close(dev))
   print_log(7, "device/dev-io.c", 288 , 0,"<backtrace>");
  return 0;
 }

 if (!dev_close(dev))
  print_log(7, "device/dev-io.c", 293 , 0,"<backtrace>");

 *read_ahead = (uint32_t) read_ahead_long;
 dev->read_ahead = read_ahead_long;

 print_log(6,
 "device/dev-io.c"
/* # 298 "device/dev-io.c" */
 ,
 299
/* # 298 "device/dev-io.c" */
 , 0,"%s: read_ahead is %u sectors", dev_name(dev), *read_ahead)
                               ;

 return 1;
}





int dev_get_size(const struct device *dev, uint64_t *size)
{
 if (!dev)
  return 0;

 if ((dev->flags & 0x00000002))
  return _dev_get_size_file(dev, size);
 else
  return _dev_get_size_dev(dev, size);
}

int dev_get_read_ahead(struct device *dev, uint32_t *read_ahead)
{
 if (!dev)
  return 0;

 if (dev->flags & 0x00000002) {
  *read_ahead = 0;
  return 1;
 }

 return _dev_read_ahead_dev(dev, read_ahead);
}
/* # 362 "device/dev-io.c" */
void dev_flush(struct device *dev)
{
 if (!(dev->flags & 0x00000002) && ioctl(dev->fd, (((0U) << (((0 +8)+8)+14)) | (((0x12)) << (0 +8)) | (((97)) << 0) | ((0) << ((0 +8)+8))), 0) >= 0)
  return;

 if (fsync(dev->fd) >= 0)
  return;

 sync();
}

int dev_open_flags(struct device *dev, int flags, int direct, int quiet)
{
 struct stat buf;
 const char *name;
 int need_excl = 0, need_rw = 0;

 if ((flags & 0003) == 02)
  need_rw = 1;

 if ((flags & 0200))
  need_excl = 1;

 if (dev->fd >= 0) {
  if (((dev->flags & 0x00000008) || !need_rw) &&
      ((dev->flags & 0x00000010) || !need_excl)) {
   dev->open_count++;
   return 1;
  }

  if (dev->open_count && !need_excl) {

   print_log(3,
 "device/dev-io.c"
/* # 394 "device/dev-io.c" */
   ,
 395
/* # 394 "device/dev-io.c" */
   , -1,"Internal error: " "%s already opened read-only", dev_name(dev))
                   ;
   dev->open_count++;
  }

  dev_close_immediate(dev);
 }

 if (critical_section())

  print_log(5,
 "device/dev-io.c"
/* # 404 "device/dev-io.c" */
  ,
 405
/* # 404 "device/dev-io.c" */
  , 0,"dev_open(%s) called while suspended", dev_name(dev))
                  ;

 if (dev->flags & 0x00000002)
  name = dev_name(dev);
 else if (!(name = dev_name_confirmed(dev, quiet)))
  do { print_log(7, "device/dev-io.c", 410 , 0,"<backtrace>"); return 0; } while (0);

 if (!(dev->flags & 0x00000002)) {
  if (stat(name, &buf) < 0) {
   print_log(3, "device/dev-io.c", 414 , -1,"%s: %s failed: %s", name, "%s: stat failed", strerror((*__errno_location ())));
   return 0;
  }
  if (buf.st_rdev != dev->dev) {
   print_log(3, "device/dev-io.c", 418 , -1,"%s: device changed", name);
   return 0;
  }
 }


 if (direct) {
  if (!(dev->flags & 0x00000040))
   dev->flags |= 0x00000020;

  if ((dev->flags & 0x00000020))
   flags |= 040000;
 }




 if (!(dev->flags & 0x00000002))
  flags |= 01000000;


 if ((dev->fd = open(name, flags, 0777)) < 0) {

  if (direct && !(dev->flags & 0x00000040)) {
   flags &= ~040000;
   if ((dev->fd = open(name, flags, 0777)) >= 0) {
    dev->flags &= ~0x00000020;
    print_log(7, "device/dev-io.c", 445 , 0,"%s: Not using O_DIRECT", name);
    goto opened;
   }
  }

  if (quiet)
   print_log(7, "device/dev-io.c", 451 , 0,"%s: %s failed: %s", name, "open", strerror((*__errno_location ())));
  else
   print_log(3, "device/dev-io.c", 453 , -1,"%s: %s failed: %s", name, "open", strerror((*__errno_location ())));
  return 0;
 }


      opened:
 if (direct)
  dev->flags |= 0x00000040;

 dev->open_count++;
 dev->flags &= ~0x00000001;

 if (need_rw)
  dev->flags |= 0x00000008;
 else
  dev->flags &= ~0x00000008;

 if (need_excl)
  dev->flags |= 0x00000010;
 else
  dev->flags &= ~0x00000010;

 if (!(dev->flags & 0x00000002) &&
     ((fstat(dev->fd, &buf) < 0) || (buf.st_rdev != dev->dev))) {
  print_log(3, "device/dev-io.c", 477 , -1,"%s: fstat failed: Has device name changed?", name);
  dev_close_immediate(dev);
  return 0;
 }






 if ((flags & 0100) && !(flags & 01000))
  dev->end = lseek(dev->fd, (off_t) 0, 2);

 dm_list_add(&_open_devices, &dev->open_list);

 print_log(7,


 "device/dev-io.c"
/* # 492 "device/dev-io.c" */
 ,


 495
/* # 492 "device/dev-io.c" */
 , 0,"Opened %s %s%s%s", dev_name(dev), dev->flags & 0x00000008 ? "RW" : "RO", dev->flags & 0x00000010 ? " O_EXCL" : "", dev->flags & 0x00000020 ? " O_DIRECT" : "")


                                                 ;

 return 1;
}

int dev_open_quiet(struct device *dev)
{
 int flags;

 flags = vg_write_lock_held() ? 02 : 00;

 return dev_open_flags(dev, flags, 1, 1);
}

int dev_open(struct device *dev)
{
 int flags;

 flags = vg_write_lock_held() ? 02 : 00;

 return dev_open_flags(dev, flags, 1, 0);
}

int dev_test_excl(struct device *dev)
{
 int flags;
 int r;

 flags = vg_write_lock_held() ? 02 : 00;
 flags |= 0200;

 r = dev_open_flags(dev, flags, 1, 1);
 if (r)
  dev_close_immediate(dev);

 return r;
}

static void _close(struct device *dev)
{
 if (close(dev->fd))
  print_log(3, "device/dev-io.c", 536 , -1,"%s: %s failed: %s", dev_name(dev), "close", strerror((*__errno_location ())));
 dev->fd = -1;
 dev->block_size = -1;
 dm_list_del(&dev->open_list);

 print_log(7, "device/dev-io.c", 541 , 0,"Closed %s", dev_name(dev));

 if (dev->flags & 0x00000004) {
  free((void *) ((struct str_list *)((const char *)((dev->aliases.n)) - (const char *)&((struct str_list *) 0)->list))-> str)
        ;
  free(dev->aliases.n);
  free(dev);
 }
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

int dev_close_immediate(struct device *dev)
{
 return _dev_close(dev, 1);
}

void dev_close_all(void)
{
 struct dm_list *doh, *doht;
 struct device *dev;

 for (doh = (&_open_devices)->n, doht = doh->n; doh != &_open_devices; doh = doht, doht = doh->n) {
  dev = ((struct device *)((const char *)(doh) - (const char *)&((struct device *) 0)->open_list));
  if (dev->open_count < 1)
   _close(dev);
 }
}

static inline int _dev_is_valid(struct device *dev)
{
 return (dev->max_error_count == 0 ||
  dev->error_count < dev->max_error_count);
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






int dev_read_circular(struct device *dev, uint64_t offset, size_t len,
        uint64_t offset2, size_t len2, char *buf)
{
 if (!dev_read(dev, offset, len, buf)) {
  print_log(3, "device/dev-io.c", 651 , -1,"Read from %s failed", dev_name(dev));
  return 0;
 }





 if (!len2)
  return 1;

 if (!dev_read(dev, offset2, len2, buf + len)) {
  print_log(3,
 "device/dev-io.c"
/* # 663 "device/dev-io.c" */
  ,
 664
/* # 663 "device/dev-io.c" */
  , -1,"Circular read from %s failed", dev_name(dev))
                   ;
  return 0;
 }

 return 1;
}






int dev_append(struct device *dev, size_t len, char *buffer)
{
 int r;

 if (!dev->open_count)
  do { print_log(7, "device/dev-io.c", 681 , 0,"<backtrace>"); return 0; } while (0);

 r = dev_write(dev, dev->end, len, buffer);
 dev->end += (uint64_t) len;




 return r;
}

int dev_write(struct device *dev, uint64_t offset, size_t len, void *buffer)
{
 struct device_area where;
 int ret;

 if (!dev->open_count)
  do { print_log(7, "device/dev-io.c", 698 , 0,"<backtrace>"); return 0; } while (0);

 if (!_dev_is_valid(dev))
  return 0;

 where.dev = dev;
 where.start = offset;
 where.size = len;

 dev->flags |= 0x00000001;

 ret = _aligned_io(&where, buffer, 1);
 if (!ret)
  _dev_inc_error_count(dev);

 return ret;
}

int dev_set(struct device *dev, uint64_t offset, size_t len, int value)
{
 size_t s;
 char buffer[4096] __attribute__((aligned(8)));

 if (!dev_open(dev))
  do { print_log(7, "device/dev-io.c", 722 , 0,"<backtrace>"); return 0; } while (0);

 if ((offset % ( 1L << 9L )) || (len % ( 1L << 9L )))
  print_log(7,
 "device/dev-io.c"
/* # 725 "device/dev-io.c" */
  ,
 726
/* # 725 "device/dev-io.c" */
  , 0,"Wiping %s at %" "ll" "u" " length %" "zu", dev_name(dev), offset, len)
                                ;
 else
  print_log(7,

 "device/dev-io.c"
/* # 728 "device/dev-io.c" */
  ,

 730
/* # 728 "device/dev-io.c" */
  , 0,"Wiping %s at sector %" "ll" "u" " length %" "zu" " sectors", dev_name(dev), offset >> 9L, len >> 9L)

                         ;

 memset(buffer, value, sizeof(buffer));
 while (1) {
  s = len > sizeof(buffer) ? sizeof(buffer) : len;
  if (!dev_write(dev, offset, s, buffer))
   break;

  len -= s;
  if (!len)
   break;

  offset += s;
 }

 dev->flags |= 0x00000001;

 if (!dev_close(dev))
  print_log(7, "device/dev-io.c", 748 , 0,"<backtrace>");

 return (len == 0);
}
