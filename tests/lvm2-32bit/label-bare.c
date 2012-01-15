/* # 22 "label/label.c" 2 */
/* # 32 "label/label.c" */
struct labeller_i {
 struct dm_list list;

 struct labeller *l;
 char name[0];
};

static struct dm_list _labellers;

static struct labeller_i *_alloc_li(const char *name, struct labeller *l)
{
 struct labeller_i *li;
 size_t len;

 len = sizeof(*li) + strlen(name) + 1;

 if (!(li = dm_malloc_aux((len), "label/label.c", 48))) {
  print_log(3, "label/label.c", 49 , -1,"Couldn't allocate memory for labeller list object.");
  return ((void *)0);
 }

 li->l = l;
 strcpy(li->name, name);

 return li;
}

static void _free_li(struct labeller_i *li)
{
 free(li);
}

int label_init(void)
{
 dm_list_init(&_labellers);
 return 1;
}

void label_exit(void)
{
 struct dm_list *c, *n;
 struct labeller_i *li;

 for (c = _labellers.n; c && c != &_labellers; c = n) {
  n = c->n;
  li = ((struct labeller_i *)((const char *)((c)) - (const char *)&((struct labeller_i *) 0)->list));
  li->l->ops->destroy(li->l);
  _free_li(li);
 }

 dm_list_init(&_labellers);
}

int label_register_handler(const char *name, struct labeller *handler)
{
 struct labeller_i *li;

 if (!(li = _alloc_li(name, handler)))
  do { print_log(7, "label/label.c", 90 , 0,"<backtrace>"); return 0; } while (0);

 dm_list_add(&_labellers, &li->list);
 return 1;
}

struct labeller *label_get_handler(const char *name)
{
 struct labeller_i *li;

 for (li = ((__typeof__(*li) *)((const char *)(((&_labellers))->n) - (const char *)&((__typeof__(*li) *) 0)->list)); &li->list != ((&_labellers)); li = ((__typeof__(*li) *)((const char *)(li->list.n) - (const char *)&((__typeof__(*li) *) 0)->list)))
  if (!__extension__ ({ size_t __s1_len, __s2_len; (__builtin_constant_p (li->name) && __builtin_constant_p (name) && (__s1_len = strlen (li->name), __s2_len = strlen (name), (!((size_t)(const void *)((li->name) + 1) - (size_t)(const void *)(li->name) == 1) || __s1_len >= 4) && (!((size_t)(const void *)((name) + 1) - (size_t)(const void *)(name) == 1) || __s2_len >= 4)) ? __builtin_strcmp (li->name, name) : (__builtin_constant_p (li->name) && ((size_t)(const void *)((li->name) + 1) - (size_t)(const void *)(li->name) == 1) && (__s1_len = strlen (li->name), __s1_len < 4) ? (__builtin_constant_p (name) && ((size_t)(const void *)((name) + 1) - (size_t)(const void *)(name) == 1) ? __builtin_strcmp (li->name, name) : (__extension__ ({ __const unsigned char *__s2 = (__const unsigned char *) (__const char *) (name); register int __result = (((__const unsigned char *) (__const char *) (li->name))[0] - __s2[0]); if (__s1_len > 0 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (li->name))[1] - __s2[1]); if (__s1_len > 1 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (li->name))[2] - __s2[2]); if (__s1_len > 2 && __result == 0) __result = (((__const unsigned char *) (__const char *) (li->name))[3] - __s2[3]); } } __result; }))) : (__builtin_constant_p (name) && ((size_t)(const void *)((name) + 1) - (size_t)(const void *)(name) == 1) && (__s2_len = strlen (name), __s2_len < 4) ? (__builtin_constant_p (li->name) && ((size_t)(const void *)((li->name) + 1) - (size_t)(const void *)(li->name) == 1) ? __builtin_strcmp (li->name, name) : (__extension__ ({ __const unsigned char *__s1 = (__const unsigned char *) (__const char *) (li->name); register int __result = __s1[0] - ((__const unsigned char *) (__const char *) (name))[0]; if (__s2_len > 0 && __result == 0) { __result = (__s1[1] - ((__const unsigned char *) (__const char *) (name))[1]); if (__s2_len > 1 && __result == 0) { __result = (__s1[2] - ((__const unsigned char *) (__const char *) (name))[2]); if (__s2_len > 2 && __result == 0) __result = (__s1[3] - ((__const unsigned char *) (__const char *) (name))[3]); } } __result; }))) : __builtin_strcmp (li->name, name)))); }))
   return li->l;

 return ((void *)0);
}

static struct labeller *_find_labeller(struct device *dev, char *buf,
           uint64_t *label_sector,
           uint64_t scan_sector)
{
 struct labeller_i *li;
 struct labeller *r = ((void *)0);
 struct label_header *lh;
 struct lvmcache_info *info;
 uint64_t sector;
 int found = 0;
 char readbuf[(4L << 9L)] __attribute__((aligned(8)));

 if (!dev_read(dev, scan_sector << 9L,
        (4L << 9L), readbuf)) {
  print_log(7, "label/label.c", 121 , 0,"%s: Failed to read label area", dev_name(dev));
  goto out;
 }


 for (sector = 0; sector < 4L;
      sector += ( 1L << 9L ) >> 9L) {
  lh = (struct label_header *) (readbuf +
           (sector << 9L));

  if (!(__extension__ (__builtin_constant_p (sizeof(lh->id)) && ((__builtin_constant_p ((char *)lh->id) && strlen ((char *)lh->id) < ((size_t) (sizeof(lh->id)))) || (__builtin_constant_p ("LABELONE") && strlen ("LABELONE") < ((size_t) (sizeof(lh->id))))) ? __extension__ ({ size_t __s1_len, __s2_len; (__builtin_constant_p ((char *)lh->id) && __builtin_constant_p ("LABELONE") && (__s1_len = strlen ((char *)lh->id), __s2_len = strlen ("LABELONE"), (!((size_t)(const void *)(((char *)lh->id) + 1) - (size_t)(const void *)((char *)lh->id) == 1) || __s1_len >= 4) && (!((size_t)(const void *)(("LABELONE") + 1) - (size_t)(const void *)("LABELONE") == 1) || __s2_len >= 4)) ? __builtin_strcmp ((char *)lh->id, "LABELONE") : (__builtin_constant_p ((char *)lh->id) && ((size_t)(const void *)(((char *)lh->id) + 1) - (size_t)(const void *)((char *)lh->id) == 1) && (__s1_len = strlen ((char *)lh->id), __s1_len < 4) ? (__builtin_constant_p ("LABELONE") && ((size_t)(const void *)(("LABELONE") + 1) - (size_t)(const void *)("LABELONE") == 1) ? __builtin_strcmp ((char *)lh->id, "LABELONE") : (__extension__ ({ __const unsigned char *__s2 = (__const unsigned char *) (__const char *) ("LABELONE"); register int __result = (((__const unsigned char *) (__const char *) ((char *)lh->id))[0] - __s2[0]); if (__s1_len > 0 && __result == 0) { __result = (((__const unsigned char *) (__const char *) ((char *)lh->id))[1] - __s2[1]); if (__s1_len > 1 && __result == 0) { __result = (((__const unsigned char *) (__const char *) ((char *)lh->id))[2] - __s2[2]); if (__s1_len > 2 && __result == 0) __result = (((__const unsigned char *) (__const char *) ((char *)lh->id))[3] - __s2[3]); } } __result; }))) : (__builtin_constant_p ("LABELONE") && ((size_t)(const void *)(("LABELONE") + 1) - (size_t)(const void *)("LABELONE") == 1) && (__s2_len = strlen ("LABELONE"), __s2_len < 4) ? (__builtin_constant_p ((char *)lh->id) && ((size_t)(const void *)(((char *)lh->id) + 1) - (size_t)(const void *)((char *)lh->id) == 1) ? __builtin_strcmp ((char *)lh->id, "LABELONE") : (__extension__ ({ __const unsigned char *__s1 = (__const unsigned char *) (__const char *) ((char *)lh->id); register int __result = __s1[0] - ((__const unsigned char *) (__const char *) ("LABELONE"))[0]; if (__s2_len > 0 && __result == 0) { __result = (__s1[1] - ((__const unsigned char *) (__const char *) ("LABELONE"))[1]); if (__s2_len > 1 && __result == 0) { __result = (__s1[2] - ((__const unsigned char *) (__const char *) ("LABELONE"))[2]); if (__s2_len > 2 && __result == 0) __result = (__s1[3] - ((__const unsigned char *) (__const char *) ("LABELONE"))[3]); } } __result; }))) : __builtin_strcmp ((char *)lh->id, "LABELONE")))); }) : strncmp ((char *)lh->id, "LABELONE", sizeof(lh->id))))) {
   if (found) {
    print_log(3,

 "label/label.c"
/* # 133 "label/label.c" */
    ,

 135
/* # 133 "label/label.c" */
    , -1,"Ignoring additional label on %s at " "sector %" "ll" "u", dev_name(dev), sector + scan_sector)

                            ;
   }
   if ((lh->sector_xl) != sector + scan_sector) {
    print_log(6,



 "label/label.c"
/* # 138 "label/label.c" */
    ,



 142
/* # 138 "label/label.c" */
    , 0,"%s: Label for sector %" "ll" "u" " found at sector %" "ll" "u" " - ignoring", dev_name(dev), (uint64_t)(lh->sector_xl), sector + scan_sector)



                           ;
    continue;
   }
   if (calc_crc(0xf597a6cf, (uint8_t *)&lh->offset_xl, ( 1L << 9L ) -
         ((uint8_t *) &lh->offset_xl - (uint8_t *) lh)) !=
       (lh->crc_xl)) {
    print_log(6,
 "label/label.c"
/* # 148 "label/label.c" */
    ,
 149
/* # 148 "label/label.c" */
    , 0,"Label checksum incorrect on %s - " "ignoring", dev_name(dev))
                                ;
    continue;
   }
   if (found)
    continue;
  }

  for (li = ((__typeof__(*li) *)((const char *)(((&_labellers))->n) - (const char *)&((__typeof__(*li) *) 0)->list)); &li->list != ((&_labellers)); li = ((__typeof__(*li) *)((const char *)(li->list.n) - (const char *)&((__typeof__(*li) *) 0)->list))) {
   if (li->l->ops->can_handle(li->l, (char *) lh,
         sector + scan_sector)) {
    print_log(6,
 "label/label.c"
/* # 159 "label/label.c" */
    ,
 160
/* # 159 "label/label.c" */
    , 0,"%s: %s label detected", dev_name(dev), li->name)
                               ;
    if (found) {
     print_log(3,


 "label/label.c"
/* # 162 "label/label.c" */
     ,


 165
/* # 162 "label/label.c" */
     , -1,"Ignoring additional label " "on %s at sector %" "ll" "u", dev_name(dev), sector + scan_sector)


                             ;
     continue;
    }
    r = li->l;
    memcpy(buf, lh, ( 1L << 9L ));
    if (label_sector)
     *label_sector = sector + scan_sector;
    found = 1;
    break;
   }
  }
 }

      out:
 if (!found) {
  if ((info = info_from_pvid(dev->pvid, 0)))
   lvmcache_update_vgname_and_id(info, info->fmt->orphan_vg_name,
            info->fmt->orphan_vg_name,
            0, ((void *)0));
  print_log(6, "label/label.c", 184 , 0,"%s: No label detected", dev_name(dev));
 }

 return r;
}


int label_remove(struct device *dev)
{
 char buf[( 1L << 9L )] __attribute__((aligned(8)));
 char readbuf[(4L << 9L)] __attribute__((aligned(8)));
 int r = 1;
 uint64_t sector;
 int wipe;
 struct labeller_i *li;
 struct label_header *lh;

 memset(buf, 0, ( 1L << 9L ));

 print_log(6, "label/label.c", 203 , 0,"Scanning for labels to wipe from %s", dev_name(dev));

 if (!dev_open(dev))
  do { print_log(7, "label/label.c", 206 , 0,"<backtrace>"); return 0; } while (0);





 dev_flush(dev);

 if (!dev_read(dev, 0ULL, (4L << 9L), readbuf)) {
  print_log(7, "label/label.c", 215 , 0,"%s: Failed to read label area", dev_name(dev));
  goto out;
 }


 for (sector = 0; sector < 4L;
      sector += ( 1L << 9L ) >> 9L) {
  lh = (struct label_header *) (readbuf +
           (sector << 9L));

  wipe = 0;

  if (!(__extension__ (__builtin_constant_p (sizeof(lh->id)) && ((__builtin_constant_p ((char *)lh->id) && strlen ((char *)lh->id) < ((size_t) (sizeof(lh->id)))) || (__builtin_constant_p ("LABELONE") && strlen ("LABELONE") < ((size_t) (sizeof(lh->id))))) ? __extension__ ({ size_t __s1_len, __s2_len; (__builtin_constant_p ((char *)lh->id) && __builtin_constant_p ("LABELONE") && (__s1_len = strlen ((char *)lh->id), __s2_len = strlen ("LABELONE"), (!((size_t)(const void *)(((char *)lh->id) + 1) - (size_t)(const void *)((char *)lh->id) == 1) || __s1_len >= 4) && (!((size_t)(const void *)(("LABELONE") + 1) - (size_t)(const void *)("LABELONE") == 1) || __s2_len >= 4)) ? __builtin_strcmp ((char *)lh->id, "LABELONE") : (__builtin_constant_p ((char *)lh->id) && ((size_t)(const void *)(((char *)lh->id) + 1) - (size_t)(const void *)((char *)lh->id) == 1) && (__s1_len = strlen ((char *)lh->id), __s1_len < 4) ? (__builtin_constant_p ("LABELONE") && ((size_t)(const void *)(("LABELONE") + 1) - (size_t)(const void *)("LABELONE") == 1) ? __builtin_strcmp ((char *)lh->id, "LABELONE") : (__extension__ ({ __const unsigned char *__s2 = (__const unsigned char *) (__const char *) ("LABELONE"); register int __result = (((__const unsigned char *) (__const char *) ((char *)lh->id))[0] - __s2[0]); if (__s1_len > 0 && __result == 0) { __result = (((__const unsigned char *) (__const char *) ((char *)lh->id))[1] - __s2[1]); if (__s1_len > 1 && __result == 0) { __result = (((__const unsigned char *) (__const char *) ((char *)lh->id))[2] - __s2[2]); if (__s1_len > 2 && __result == 0) __result = (((__const unsigned char *) (__const char *) ((char *)lh->id))[3] - __s2[3]); } } __result; }))) : (__builtin_constant_p ("LABELONE") && ((size_t)(const void *)(("LABELONE") + 1) - (size_t)(const void *)("LABELONE") == 1) && (__s2_len = strlen ("LABELONE"), __s2_len < 4) ? (__builtin_constant_p ((char *)lh->id) && ((size_t)(const void *)(((char *)lh->id) + 1) - (size_t)(const void *)((char *)lh->id) == 1) ? __builtin_strcmp ((char *)lh->id, "LABELONE") : (__extension__ ({ __const unsigned char *__s1 = (__const unsigned char *) (__const char *) ((char *)lh->id); register int __result = __s1[0] - ((__const unsigned char *) (__const char *) ("LABELONE"))[0]; if (__s2_len > 0 && __result == 0) { __result = (__s1[1] - ((__const unsigned char *) (__const char *) ("LABELONE"))[1]); if (__s2_len > 1 && __result == 0) { __result = (__s1[2] - ((__const unsigned char *) (__const char *) ("LABELONE"))[2]); if (__s2_len > 2 && __result == 0) __result = (__s1[3] - ((__const unsigned char *) (__const char *) ("LABELONE"))[3]); } } __result; }))) : __builtin_strcmp ((char *)lh->id, "LABELONE")))); }) : strncmp ((char *)lh->id, "LABELONE", sizeof(lh->id))))) {
   if ((lh->sector_xl) == sector)
    wipe = 1;
  } else {
   for (li = ((__typeof__(*li) *)((const char *)(((&_labellers))->n) - (const char *)&((__typeof__(*li) *) 0)->list)); &li->list != ((&_labellers)); li = ((__typeof__(*li) *)((const char *)(li->list.n) - (const char *)&((__typeof__(*li) *) 0)->list))) {
    if (li->l->ops->can_handle(li->l, (char *) lh,
          sector)) {
     wipe = 1;
     break;
    }
   }
  }

  if (wipe) {
   print_log(6,
 "label/label.c"
/* # 241 "label/label.c" */
   ,
 242
/* # 241 "label/label.c" */
   , 0,"%s: Wiping label at sector %" "ll" "u", dev_name(dev), sector)
                           ;
   if (!dev_write(dev, sector << 9L, ( 1L << 9L ),
           buf)) {
    print_log(3,

 "label/label.c"
/* # 245 "label/label.c" */
    ,

 247
/* # 245 "label/label.c" */
    , -1,"Failed to remove label from %s at " "sector %" "ll" "u", dev_name(dev), sector)

              ;
    r = 0;
   }
  }
 }

      out:
 if (!dev_close(dev))
  print_log(7, "label/label.c", 255 , 0,"<backtrace>");

 return r;
}

int label_read(struct device *dev, struct label **result,
  uint64_t scan_sector)
{
 char buf[( 1L << 9L )] __attribute__((aligned(8)));
 struct labeller *l;
 uint64_t sector;
 struct lvmcache_info *info;
 int r = 0;

 if ((info = info_from_pvid(dev->pvid, 1))) {
  print_log(7, "label/label.c", 270 , 0,"Using cached label for %s", dev_name(dev));
  *result = info->label;
  return 1;
 }

 if (!dev_open(dev)) {
  print_log(7, "label/label.c", 276 , 0,"<backtrace>");

  if ((info = info_from_pvid(dev->pvid, 0)))
   lvmcache_update_vgname_and_id(info, info->fmt->orphan_vg_name,
            info->fmt->orphan_vg_name,
            0, ((void *)0));

  return r;
 }

 if (!(l = _find_labeller(dev, buf, &sector, scan_sector)))
  goto out;

 if ((r = (l->ops->read)(l, dev, buf, result)) && result && *result)
  (*result)->sector = sector;

      out:
 if (!dev_close(dev))
  print_log(7, "label/label.c", 294 , 0,"<backtrace>");

 return r;
}


int label_write(struct device *dev, struct label *label)
{
 char buf[( 1L << 9L )] __attribute__((aligned(8)));
 struct label_header *lh = (struct label_header *) buf;
 int r = 1;

 if (!label->labeller->ops->write) {
  print_log(3, "label/label.c", 307 , -1,"Label handler does not support label writes");
  return 0;
 }

 if ((( 1L << 9L ) + (label->sector << 9L)) > (4L << 9L)) {
  print_log(3,
 "label/label.c"
/* # 312 "label/label.c" */
  ,
 313
/* # 312 "label/label.c" */
  , -1,"Label sector %" "ll" "u" " beyond range (%ld)", label->sector, 4L)
                                       ;
  return 0;
 }

 memset(buf, 0, ( 1L << 9L ));

 strncpy((char *)lh->id, "LABELONE", sizeof(lh->id));
 lh->sector_xl = (label->sector);
 lh->offset_xl = (sizeof(*lh));

 if (!(label->labeller->ops->write)(label, buf))
  do { print_log(7, "label/label.c", 324 , 0,"<backtrace>"); return 0; } while (0);

 lh->crc_xl = (calc_crc(0xf597a6cf, (uint8_t *)&lh->offset_xl, ( 1L << 9L ) - ((uint8_t *) &lh->offset_xl - (uint8_t *) lh)))
                                                         ;

 if (!dev_open(dev))
  do { print_log(7, "label/label.c", 330 , 0,"<backtrace>"); return 0; } while (0);

 print_log(6,

 "label/label.c"
/* # 332 "label/label.c" */
 ,

 334
/* # 332 "label/label.c" */
 , 0,"%s: Writing label to sector %" "ll" "u" " with stored offset %" "u" ".", dev_name(dev), label->sector, (lh->offset_xl))

                          ;
 if (!dev_write(dev, label->sector << 9L, ( 1L << 9L ), buf)) {
  print_log(7, "label/label.c", 336 , 0,"Failed to write label to %s", dev_name(dev));
  r = 0;
 }

 if (!dev_close(dev))
  print_log(7, "label/label.c", 341 , 0,"<backtrace>");

 return r;
}


int label_verify(struct device *dev)
{
 struct labeller *l;
 char buf[( 1L << 9L )] __attribute__((aligned(8)));
 uint64_t sector;
 struct lvmcache_info *info;
 int r = 0;

 if (!dev_open(dev)) {
  if ((info = info_from_pvid(dev->pvid, 0)))
   lvmcache_update_vgname_and_id(info, info->fmt->orphan_vg_name,
            info->fmt->orphan_vg_name,
            0, ((void *)0));

  do { print_log(7, "label/label.c", 361 , 0,"<backtrace>"); return 0; } while (0);
 }

 if (!(l = _find_labeller(dev, buf, &sector, 0ULL)))
  goto out;

 r = l->ops->verify ? l->ops->verify(l, buf, sector) : 1;

      out:
 if (!dev_close(dev))
  print_log(7, "label/label.c", 371 , 0,"<backtrace>");

 return r;
}

void label_destroy(struct label *label)
{
 label->labeller->ops->destroy_label(label->labeller, label);
 free(label);
}

struct label *label_create(struct labeller *labeller)
{
 struct label *label;

 if (!(label = dm_zalloc_aux((sizeof(*label)), "label/label.c", 386))) {
  print_log(3, "label/label.c", 387 , -1,"label allocaction failed");
  return ((void *)0);
 }

 label->labeller = labeller;

 labeller->ops->initialise_label(labeller, label);

 return label;
}
