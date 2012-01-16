/* # 27 "device/dev-cache.c" 2 */

struct dev_iter {
 struct btree_iter *current;
 struct dev_filter *filter;
};

struct dir_list {
 struct dm_list list;
 char dir[0];
};

static struct {
 struct dm_pool *mem;
 struct dm_hash_table *names;
 struct btree *devices;
 struct dm_regex *preferred_names_matcher;
 const char *dev_dir;

 int has_scanned;
 struct dm_list dirs;
 struct dm_list files;

} _cache;





static int _insert(const char *path, int rec);

struct device *dev_create_file(const char *filename, struct device *dev,
          struct str_list *alias, int use_malloc)
{
 int allocate = !dev;

 if (allocate) {
  if (use_malloc) {
   if (!(dev = dm_malloc_aux((sizeof(*dev)), "device/dev-cache.c", 64))) {
    print_log(3, "device/dev-cache.c", 65 , -1,"struct device allocation failed");
    return ((void *)0);
   }
   if (!(alias = dm_malloc_aux((sizeof(*alias)), "device/dev-cache.c", 68))) {
    print_log(3, "device/dev-cache.c", 69 , -1,"struct str_list allocation failed");
    free(dev);
    return ((void *)0);
   }
   if (!(alias->str = (__extension__ (__builtin_constant_p (filename) && ((size_t)(const void *)((filename) + 1) - (size_t)(const void *)(filename) == 1) ? (((__const char *) (filename))[0] == '\0' ? (char *) calloc ((size_t) 1, (size_t) 1) : ({ size_t __len = strlen (filename) + 1; char *__retval = (char *) malloc (__len); if (__retval != ((void *)0)) __retval = (char *) memcpy (__retval, filename, __len); __retval; })) : __strdup (filename))))) {
    print_log(3, "device/dev-cache.c", 74 , -1,"filename strdup failed");
    free(dev);
    free(alias);
    return ((void *)0);
   }
   dev->flags = 0x00000004;
  } else {
   if (!(dev = dm_pool_zalloc(_cache.mem, (sizeof(*dev))))) {
    print_log(3, "device/dev-cache.c", 82 , -1,"struct device allocation failed");
    return ((void *)0);
   }
   if (!(alias = dm_pool_zalloc(_cache.mem, (sizeof(*alias))))) {
    print_log(3, "device/dev-cache.c", 86 , -1,"struct str_list allocation failed");
    dm_pool_free(_cache.mem, (dev));
    return ((void *)0);
   }
   if (!(alias->str = dm_pool_strdup(_cache.mem, (filename)))) {
    print_log(3, "device/dev-cache.c", 91 , -1,"filename strdup failed");
    return ((void *)0);
   }
  }
 } else if (!(alias->str = (__extension__ (__builtin_constant_p (filename) && ((size_t)(const void *)((filename) + 1) - (size_t)(const void *)(filename) == 1) ? (((__const char *) (filename))[0] == '\0' ? (char *) calloc ((size_t) 1, (size_t) 1) : ({ size_t __len = strlen (filename) + 1; char *__retval = (char *) malloc (__len); if (__retval != ((void *)0)) __retval = (char *) memcpy (__retval, filename, __len); __retval; })) : __strdup (filename))))) {
  print_log(3, "device/dev-cache.c", 96 , -1,"filename strdup failed");
  return ((void *)0);
 }

 dev->flags |= 0x00000002;
 dm_list_init(&dev->aliases);
 dm_list_add(&dev->aliases, &alias->list);
 dev->end = 0ULL;
 dev->dev = 0;
 dev->fd = -1;
 dev->open_count = 0;
 dev->error_count = 0;
 dev->max_error_count = 0;
 dev->block_size = -1;
 dev->read_ahead = -1;
 memset(dev->pvid, 0, sizeof(dev->pvid));
 dm_list_init(&dev->open_list);

 return dev;
}

static struct device *_dev_create(dev_t d)
{
 struct device *dev;

 if (!(dev = dm_pool_zalloc(_cache.mem, (sizeof(*dev))))) {
  print_log(3, "device/dev-cache.c", 122 , -1,"struct device allocation failed");
  return ((void *)0);
 }
 dev->flags = 0;
 dm_list_init(&dev->aliases);
 dev->dev = d;
 dev->fd = -1;
 dev->open_count = 0;
 dev->max_error_count = dev_disable_after_error_count();
 dev->block_size = -1;
 dev->read_ahead = -1;
 dev->end = 0ULL;
 memset(dev->pvid, 0, sizeof(dev->pvid));
 dm_list_init(&dev->open_list);

 return dev;
}

void dev_set_preferred_name(struct str_list *sl, struct device *dev)
{



 if (_cache.preferred_names_matcher)
  return;

 print_log(7, "device/dev-cache.c", 148 , 0,"%s: New preferred name", sl->str);
 dm_list_del(&sl->list);
 dm_list_add_h(&dev->aliases, &sl->list);
}







static int _builtin_preference(const char *path0, const char *path1,
          size_t skip_prefix_count, const char *subpath)
{
 size_t subpath_len;
 int r0, r1;

 subpath_len = strlen(subpath);

 r0 = !(__extension__ (__builtin_constant_p (subpath_len) && ((__builtin_constant_p (path0 + skip_prefix_count) && strlen (path0 + skip_prefix_count) < ((size_t) (subpath_len))) || (__builtin_constant_p (subpath) && strlen (subpath) < ((size_t) (subpath_len)))) ? __extension__ ({ size_t __s1_len, __s2_len; (__builtin_constant_p (path0 + skip_prefix_count) && __builtin_constant_p (subpath) && (__s1_len = strlen (path0 + skip_prefix_count), __s2_len = strlen (subpath), (!((size_t)(const void *)((path0 + skip_prefix_count) + 1) - (size_t)(const void *)(path0 + skip_prefix_count) == 1) || __s1_len >= 4) && (!((size_t)(const void *)((subpath) + 1) - (size_t)(const void *)(subpath) == 1) || __s2_len >= 4)) ? __builtin_strcmp (path0 + skip_prefix_count, subpath) : (__builtin_constant_p (path0 + skip_prefix_count) && ((size_t)(const void *)((path0 + skip_prefix_count) + 1) - (size_t)(const void *)(path0 + skip_prefix_count) == 1) && (__s1_len = strlen (path0 + skip_prefix_count), __s1_len < 4) ? (__builtin_constant_p (subpath) && ((size_t)(const void *)((subpath) + 1) - (size_t)(const void *)(subpath) == 1) ? __builtin_strcmp (path0 + skip_prefix_count, subpath) : (__extension__ ({ __const unsigned char *__s2 = (__const unsigned char *) (__const char *) (subpath); register int __result = (((__const unsigned char *) (__const char *) (path0 + skip_prefix_count))[0] - __s2[0]); if (__s1_len > 0 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (path0 + skip_prefix_count))[1] - __s2[1]); if (__s1_len > 1 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (path0 + skip_prefix_count))[2] - __s2[2]); if (__s1_len > 2 && __result == 0) __result = (((__const unsigned char *) (__const char *) (path0 + skip_prefix_count))[3] - __s2[3]); } } __result; }))) : (__builtin_constant_p (subpath) && ((size_t)(const void *)((subpath) + 1) - (size_t)(const void *)(subpath) == 1) && (__s2_len = strlen (subpath), __s2_len < 4) ? (__builtin_constant_p (path0 + skip_prefix_count) && ((size_t)(const void *)((path0 + skip_prefix_count) + 1) - (size_t)(const void *)(path0 + skip_prefix_count) == 1) ? __builtin_strcmp (path0 + skip_prefix_count, subpath) : (__extension__ ({ __const unsigned char *__s1 = (__const unsigned char *) (__const char *) (path0 + skip_prefix_count); register int __result = __s1[0] - ((__const unsigned char *) (__const char *) (subpath))[0]; if (__s2_len > 0 && __result == 0) { __result = (__s1[1] - ((__const unsigned char *) (__const char *) (subpath))[1]); if (__s2_len > 1 && __result == 0) { __result = (__s1[2] - ((__const unsigned char *) (__const char *) (subpath))[2]); if (__s2_len > 2 && __result == 0) __result = (__s1[3] - ((__const unsigned char *) (__const char *) (subpath))[3]); } } __result; }))) : __builtin_strcmp (path0 + skip_prefix_count, subpath)))); }) : strncmp (path0 + skip_prefix_count, subpath, subpath_len)));
 r1 = !(__extension__ (__builtin_constant_p (subpath_len) && ((__builtin_constant_p (path1 + skip_prefix_count) && strlen (path1 + skip_prefix_count) < ((size_t) (subpath_len))) || (__builtin_constant_p (subpath) && strlen (subpath) < ((size_t) (subpath_len)))) ? __extension__ ({ size_t __s1_len, __s2_len; (__builtin_constant_p (path1 + skip_prefix_count) && __builtin_constant_p (subpath) && (__s1_len = strlen (path1 + skip_prefix_count), __s2_len = strlen (subpath), (!((size_t)(const void *)((path1 + skip_prefix_count) + 1) - (size_t)(const void *)(path1 + skip_prefix_count) == 1) || __s1_len >= 4) && (!((size_t)(const void *)((subpath) + 1) - (size_t)(const void *)(subpath) == 1) || __s2_len >= 4)) ? __builtin_strcmp (path1 + skip_prefix_count, subpath) : (__builtin_constant_p (path1 + skip_prefix_count) && ((size_t)(const void *)((path1 + skip_prefix_count) + 1) - (size_t)(const void *)(path1 + skip_prefix_count) == 1) && (__s1_len = strlen (path1 + skip_prefix_count), __s1_len < 4) ? (__builtin_constant_p (subpath) && ((size_t)(const void *)((subpath) + 1) - (size_t)(const void *)(subpath) == 1) ? __builtin_strcmp (path1 + skip_prefix_count, subpath) : (__extension__ ({ __const unsigned char *__s2 = (__const unsigned char *) (__const char *) (subpath); register int __result = (((__const unsigned char *) (__const char *) (path1 + skip_prefix_count))[0] - __s2[0]); if (__s1_len > 0 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (path1 + skip_prefix_count))[1] - __s2[1]); if (__s1_len > 1 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (path1 + skip_prefix_count))[2] - __s2[2]); if (__s1_len > 2 && __result == 0) __result = (((__const unsigned char *) (__const char *) (path1 + skip_prefix_count))[3] - __s2[3]); } } __result; }))) : (__builtin_constant_p (subpath) && ((size_t)(const void *)((subpath) + 1) - (size_t)(const void *)(subpath) == 1) && (__s2_len = strlen (subpath), __s2_len < 4) ? (__builtin_constant_p (path1 + skip_prefix_count) && ((size_t)(const void *)((path1 + skip_prefix_count) + 1) - (size_t)(const void *)(path1 + skip_prefix_count) == 1) ? __builtin_strcmp (path1 + skip_prefix_count, subpath) : (__extension__ ({ __const unsigned char *__s1 = (__const unsigned char *) (__const char *) (path1 + skip_prefix_count); register int __result = __s1[0] - ((__const unsigned char *) (__const char *) (subpath))[0]; if (__s2_len > 0 && __result == 0) { __result = (__s1[1] - ((__const unsigned char *) (__const char *) (subpath))[1]); if (__s2_len > 1 && __result == 0) { __result = (__s1[2] - ((__const unsigned char *) (__const char *) (subpath))[2]); if (__s2_len > 2 && __result == 0) __result = (__s1[3] - ((__const unsigned char *) (__const char *) (subpath))[3]); } } __result; }))) : __builtin_strcmp (path1 + skip_prefix_count, subpath)))); }) : strncmp (path1 + skip_prefix_count, subpath, subpath_len)));

 if (!r0 && r1)

  return 0;
 else if (r0 && !r1)

  return 1;
 else if (r0 && r1)

  return -1;


 return -2;
}

static int _apply_builtin_path_preference_rules(const char *path0, const char *path1)
{
 size_t devdir_len;
 int r;

 devdir_len = strlen(_cache.dev_dir);

 if (!(__extension__ (__builtin_constant_p (devdir_len) && ((__builtin_constant_p (path0) && strlen (path0) < ((size_t) (devdir_len))) || (__builtin_constant_p (_cache.dev_dir) && strlen (_cache.dev_dir) < ((size_t) (devdir_len)))) ? __extension__ ({ size_t __s1_len, __s2_len; (__builtin_constant_p (path0) && __builtin_constant_p (_cache.dev_dir) && (__s1_len = strlen (path0), __s2_len = strlen (_cache.dev_dir), (!((size_t)(const void *)((path0) + 1) - (size_t)(const void *)(path0) == 1) || __s1_len >= 4) && (!((size_t)(const void *)((_cache.dev_dir) + 1) - (size_t)(const void *)(_cache.dev_dir) == 1) || __s2_len >= 4)) ? __builtin_strcmp (path0, _cache.dev_dir) : (__builtin_constant_p (path0) && ((size_t)(const void *)((path0) + 1) - (size_t)(const void *)(path0) == 1) && (__s1_len = strlen (path0), __s1_len < 4) ? (__builtin_constant_p (_cache.dev_dir) && ((size_t)(const void *)((_cache.dev_dir) + 1) - (size_t)(const void *)(_cache.dev_dir) == 1) ? __builtin_strcmp (path0, _cache.dev_dir) : (__extension__ ({ __const unsigned char *__s2 = (__const unsigned char *) (__const char *) (_cache.dev_dir); register int __result = (((__const unsigned char *) (__const char *) (path0))[0] - __s2[0]); if (__s1_len > 0 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (path0))[1] - __s2[1]); if (__s1_len > 1 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (path0))[2] - __s2[2]); if (__s1_len > 2 && __result == 0) __result = (((__const unsigned char *) (__const char *) (path0))[3] - __s2[3]); } } __result; }))) : (__builtin_constant_p (_cache.dev_dir) && ((size_t)(const void *)((_cache.dev_dir) + 1) - (size_t)(const void *)(_cache.dev_dir) == 1) && (__s2_len = strlen (_cache.dev_dir), __s2_len < 4) ? (__builtin_constant_p (path0) && ((size_t)(const void *)((path0) + 1) - (size_t)(const void *)(path0) == 1) ? __builtin_strcmp (path0, _cache.dev_dir) : (__extension__ ({ __const unsigned char *__s1 = (__const unsigned char *) (__const char *) (path0); register int __result = __s1[0] - ((__const unsigned char *) (__const char *) (_cache.dev_dir))[0]; if (__s2_len > 0 && __result == 0) { __result = (__s1[1] - ((__const unsigned char *) (__const char *) (_cache.dev_dir))[1]); if (__s2_len > 1 && __result == 0) { __result = (__s1[2] - ((__const unsigned char *) (__const char *) (_cache.dev_dir))[2]); if (__s2_len > 2 && __result == 0) __result = (__s1[3] - ((__const unsigned char *) (__const char *) (_cache.dev_dir))[3]); } } __result; }))) : __builtin_strcmp (path0, _cache.dev_dir)))); }) : strncmp (path0, _cache.dev_dir, devdir_len))) &&
     !(__extension__ (__builtin_constant_p (devdir_len) && ((__builtin_constant_p (path1) && strlen (path1) < ((size_t) (devdir_len))) || (__builtin_constant_p (_cache.dev_dir) && strlen (_cache.dev_dir) < ((size_t) (devdir_len)))) ? __extension__ ({ size_t __s1_len, __s2_len; (__builtin_constant_p (path1) && __builtin_constant_p (_cache.dev_dir) && (__s1_len = strlen (path1), __s2_len = strlen (_cache.dev_dir), (!((size_t)(const void *)((path1) + 1) - (size_t)(const void *)(path1) == 1) || __s1_len >= 4) && (!((size_t)(const void *)((_cache.dev_dir) + 1) - (size_t)(const void *)(_cache.dev_dir) == 1) || __s2_len >= 4)) ? __builtin_strcmp (path1, _cache.dev_dir) : (__builtin_constant_p (path1) && ((size_t)(const void *)((path1) + 1) - (size_t)(const void *)(path1) == 1) && (__s1_len = strlen (path1), __s1_len < 4) ? (__builtin_constant_p (_cache.dev_dir) && ((size_t)(const void *)((_cache.dev_dir) + 1) - (size_t)(const void *)(_cache.dev_dir) == 1) ? __builtin_strcmp (path1, _cache.dev_dir) : (__extension__ ({ __const unsigned char *__s2 = (__const unsigned char *) (__const char *) (_cache.dev_dir); register int __result = (((__const unsigned char *) (__const char *) (path1))[0] - __s2[0]); if (__s1_len > 0 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (path1))[1] - __s2[1]); if (__s1_len > 1 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (path1))[2] - __s2[2]); if (__s1_len > 2 && __result == 0) __result = (((__const unsigned char *) (__const char *) (path1))[3] - __s2[3]); } } __result; }))) : (__builtin_constant_p (_cache.dev_dir) && ((size_t)(const void *)((_cache.dev_dir) + 1) - (size_t)(const void *)(_cache.dev_dir) == 1) && (__s2_len = strlen (_cache.dev_dir), __s2_len < 4) ? (__builtin_constant_p (path1) && ((size_t)(const void *)((path1) + 1) - (size_t)(const void *)(path1) == 1) ? __builtin_strcmp (path1, _cache.dev_dir) : (__extension__ ({ __const unsigned char *__s1 = (__const unsigned char *) (__const char *) (path1); register int __result = __s1[0] - ((__const unsigned char *) (__const char *) (_cache.dev_dir))[0]; if (__s2_len > 0 && __result == 0) { __result = (__s1[1] - ((__const unsigned char *) (__const char *) (_cache.dev_dir))[1]); if (__s2_len > 1 && __result == 0) { __result = (__s1[2] - ((__const unsigned char *) (__const char *) (_cache.dev_dir))[2]); if (__s2_len > 2 && __result == 0) __result = (__s1[3] - ((__const unsigned char *) (__const char *) (_cache.dev_dir))[3]); } } __result; }))) : __builtin_strcmp (path1, _cache.dev_dir)))); }) : strncmp (path1, _cache.dev_dir, devdir_len)))) {






  if ((r = _builtin_preference(path0, path1, devdir_len, "block/")) >= -1)
   return r;


  if ((r = _builtin_preference(path0, path1, devdir_len, "dm-")) >= -1)
   return r;


  if ((r = _builtin_preference(path0, path1, devdir_len, "disk/")) >= -1)
   return r;


  if ((r = _builtin_preference(path0, path1, 0, dm_dir())) >= -1)
   return r;
 }

 return -1;
}


static int _compare_paths(const char *path0, const char *path1)
{
 int slash0 = 0, slash1 = 0;
 int m0, m1;
 const char *p;
 char p0[4096], p1[4096];
 char *s0, *s1;
 struct stat stat0, stat1;
 int r;




 if (_cache.preferred_names_matcher) {
  m0 = dm_regex_match(_cache.preferred_names_matcher, path0);
  m1 = dm_regex_match(_cache.preferred_names_matcher, path1);

  if (m0 != m1) {
   if (m0 < 0)
    return 1;
   if (m1 < 0)
    return 0;
   if (m0 < m1)
    return 1;
   if (m1 < m0)
    return 0;
  }
 }


 if ((r = _apply_builtin_path_preference_rules(path0, path1)) >= 0)
  return r;


 for (p = path0; p++; p = (const char *) (__extension__ (__builtin_constant_p ('/') && !__builtin_constant_p (p) && ('/') == '\0' ? (char *) __rawmemchr (p, '/') : __builtin_strchr (p, '/'))))
  slash0++;

 for (p = path1; p++; p = (const char *) (__extension__ (__builtin_constant_p ('/') && !__builtin_constant_p (p) && ('/') == '\0' ? (char *) __rawmemchr (p, '/') : __builtin_strchr (p, '/'))))
  slash1++;

 if (slash0 < slash1)
  return 0;
 if (slash1 < slash0)
  return 1;

 strncpy(p0, path0, 4096);
 strncpy(p1, path1, 4096);
 s0 = &p0[0] + 1;
 s1 = &p1[0] + 1;




 while (s0) {
  s0 = (__extension__ (__builtin_constant_p ('/') && !__builtin_constant_p (s0) && ('/') == '\0' ? (char *) __rawmemchr (s0, '/') : __builtin_strchr (s0, '/')));
  s1 = (__extension__ (__builtin_constant_p ('/') && !__builtin_constant_p (s1) && ('/') == '\0' ? (char *) __rawmemchr (s1, '/') : __builtin_strchr (s1, '/')));
  if (s0) {
   *s0 = '\0';
   *s1 = '\0';
  }
  if (lstat(p0, &stat0)) {
   print_log(6, "device/dev-cache.c", 280 , 0,"%s: %s failed: %s", p0, "lstat", strerror((*__errno_location ())));
   return 1;
  }
  if (lstat(p1, &stat1)) {
   print_log(6, "device/dev-cache.c", 284 , 0,"%s: %s failed: %s", p1, "lstat", strerror((*__errno_location ())));
   return 0;
  }
  if (((((stat0.st_mode)) & 0170000) == (0120000)) && !((((stat1.st_mode)) & 0170000) == (0120000)))
   return 0;
  if (!((((stat0.st_mode)) & 0170000) == (0120000)) && ((((stat1.st_mode)) & 0170000) == (0120000)))
   return 1;
  if (s0) {
   *s0++ = '/';
   *s1++ = '/';
  }
 }


 if (__extension__ ({ size_t __s1_len, __s2_len; (__builtin_constant_p (path0) && __builtin_constant_p (path1) && (__s1_len = strlen (path0), __s2_len = strlen (path1), (!((size_t)(const void *)((path0) + 1) - (size_t)(const void *)(path0) == 1) || __s1_len >= 4) && (!((size_t)(const void *)((path1) + 1) - (size_t)(const void *)(path1) == 1) || __s2_len >= 4)) ? __builtin_strcmp (path0, path1) : (__builtin_constant_p (path0) && ((size_t)(const void *)((path0) + 1) - (size_t)(const void *)(path0) == 1) && (__s1_len = strlen (path0), __s1_len < 4) ? (__builtin_constant_p (path1) && ((size_t)(const void *)((path1) + 1) - (size_t)(const void *)(path1) == 1) ? __builtin_strcmp (path0, path1) : (__extension__ ({ __const unsigned char *__s2 = (__const unsigned char *) (__const char *) (path1); register int __result = (((__const unsigned char *) (__const char *) (path0))[0] - __s2[0]); if (__s1_len > 0 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (path0))[1] - __s2[1]); if (__s1_len > 1 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (path0))[2] - __s2[2]); if (__s1_len > 2 && __result == 0) __result = (((__const unsigned char *) (__const char *) (path0))[3] - __s2[3]); } } __result; }))) : (__builtin_constant_p (path1) && ((size_t)(const void *)((path1) + 1) - (size_t)(const void *)(path1) == 1) && (__s2_len = strlen (path1), __s2_len < 4) ? (__builtin_constant_p (path0) && ((size_t)(const void *)((path0) + 1) - (size_t)(const void *)(path0) == 1) ? __builtin_strcmp (path0, path1) : (__extension__ ({ __const unsigned char *__s1 = (__const unsigned char *) (__const char *) (path0); register int __result = __s1[0] - ((__const unsigned char *) (__const char *) (path1))[0]; if (__s2_len > 0 && __result == 0) { __result = (__s1[1] - ((__const unsigned char *) (__const char *) (path1))[1]); if (__s2_len > 1 && __result == 0) { __result = (__s1[2] - ((__const unsigned char *) (__const char *) (path1))[2]); if (__s2_len > 2 && __result == 0) __result = (__s1[3] - ((__const unsigned char *) (__const char *) (path1))[3]); } } __result; }))) : __builtin_strcmp (path0, path1)))); }) < 0)
  return 0;
 else
  return 1;
}

static int _add_alias(struct device *dev, const char *path)
{
 struct str_list *sl = dm_pool_zalloc(_cache.mem, (sizeof(*sl)));
 struct str_list *strl;
 const char *oldpath;
 int prefer_old = 1;

 if (!sl)
  do { print_log(7, "device/dev-cache.c", 312 , 0,"<backtrace>"); return 0; } while (0);


 for (strl = ((__typeof__(*strl) *)((const char *)(((&dev->aliases))->n) - (const char *)&((__typeof__(*strl) *) 0)->list)); &strl->list != ((&dev->aliases)); strl = ((__typeof__(*strl) *)((const char *)(strl->list.n) - (const char *)&((__typeof__(*strl) *) 0)->list))) {
  if (!__extension__ ({ size_t __s1_len, __s2_len; (__builtin_constant_p (strl->str) && __builtin_constant_p (path) && (__s1_len = strlen (strl->str), __s2_len = strlen (path), (!((size_t)(const void *)((strl->str) + 1) - (size_t)(const void *)(strl->str) == 1) || __s1_len >= 4) && (!((size_t)(const void *)((path) + 1) - (size_t)(const void *)(path) == 1) || __s2_len >= 4)) ? __builtin_strcmp (strl->str, path) : (__builtin_constant_p (strl->str) && ((size_t)(const void *)((strl->str) + 1) - (size_t)(const void *)(strl->str) == 1) && (__s1_len = strlen (strl->str), __s1_len < 4) ? (__builtin_constant_p (path) && ((size_t)(const void *)((path) + 1) - (size_t)(const void *)(path) == 1) ? __builtin_strcmp (strl->str, path) : (__extension__ ({ __const unsigned char *__s2 = (__const unsigned char *) (__const char *) (path); register int __result = (((__const unsigned char *) (__const char *) (strl->str))[0] - __s2[0]); if (__s1_len > 0 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (strl->str))[1] - __s2[1]); if (__s1_len > 1 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (strl->str))[2] - __s2[2]); if (__s1_len > 2 && __result == 0) __result = (((__const unsigned char *) (__const char *) (strl->str))[3] - __s2[3]); } } __result; }))) : (__builtin_constant_p (path) && ((size_t)(const void *)((path) + 1) - (size_t)(const void *)(path) == 1) && (__s2_len = strlen (path), __s2_len < 4) ? (__builtin_constant_p (strl->str) && ((size_t)(const void *)((strl->str) + 1) - (size_t)(const void *)(strl->str) == 1) ? __builtin_strcmp (strl->str, path) : (__extension__ ({ __const unsigned char *__s1 = (__const unsigned char *) (__const char *) (strl->str); register int __result = __s1[0] - ((__const unsigned char *) (__const char *) (path))[0]; if (__s2_len > 0 && __result == 0) { __result = (__s1[1] - ((__const unsigned char *) (__const char *) (path))[1]); if (__s2_len > 1 && __result == 0) { __result = (__s1[2] - ((__const unsigned char *) (__const char *) (path))[2]); if (__s2_len > 2 && __result == 0) __result = (__s1[3] - ((__const unsigned char *) (__const char *) (path))[3]); } } __result; }))) : __builtin_strcmp (strl->str, path)))); })) {
   print_log(7, "device/dev-cache.c", 317 , 0,"%s: Already in device cache", path);
   return 1;
  }
 }

 if (!(sl->str = dm_pool_strdup(_cache.mem, path)))
  do { print_log(7, "device/dev-cache.c", 323 , 0,"<backtrace>"); return 0; } while (0);

 if (!dm_list_empty(&dev->aliases)) {
  oldpath = ((struct str_list *)((const char *)((dev->aliases.n)) - (const char *)&((struct str_list *) 0)->list))->str;
  prefer_old = _compare_paths(path, oldpath);
  print_log(7,
 "device/dev-cache.c"
/* # 328 "device/dev-cache.c" */
  ,
 329
/* # 328 "device/dev-cache.c" */
  , 0,"%s: Aliased to %s in device cache%s", path, oldpath, prefer_old ? "" : " (preferred name)")
                                                          ;

 } else
  print_log(7, "device/dev-cache.c", 332 , 0,"%s: Added to device cache", path);

 if (prefer_old)
  dm_list_add(&dev->aliases, &sl->list);
 else
  dm_list_add_h(&dev->aliases, &sl->list);

 return 1;
}





static int _insert_dev(const char *path, dev_t d)
{
 struct device *dev;
 static dev_t loopfile_count = 0;
 int loopfile = 0;


 if (!d) {
  if (dm_hash_lookup(_cache.names, path))
   return 1;
  d = ++loopfile_count;
  loopfile = 1;
 }


 if (!(dev = (struct device *) btree_lookup(_cache.devices,
         (uint32_t) d))) {

  if (loopfile) {
   if (!(dev = dev_create_file(path, ((void *)0), ((void *)0), 0)))
    do { print_log(7, "device/dev-cache.c", 366 , 0,"<backtrace>"); return 0; } while (0);
  } else if (!(dev = _dev_create(d)))
   do { print_log(7, "device/dev-cache.c", 368 , 0,"<backtrace>"); return 0; } while (0);

  if (!(btree_insert(_cache.devices, (uint32_t) d, dev))) {
   print_log(3, "device/dev-cache.c", 371 , -1,"Couldn't insert device into binary tree.");
   dm_pool_free(_cache.mem, (dev));
   return 0;
  }
 }

 if (!loopfile && !_add_alias(dev, path)) {
  print_log(3, "device/dev-cache.c", 378 , -1,"Couldn't add alias to dev cache.");
  return 0;
 }

 if (!dm_hash_insert(_cache.names, path, dev)) {
  print_log(3, "device/dev-cache.c", 383 , -1,"Couldn't add name to hash in dev cache.");
  return 0;
 }

 return 1;
}

static char *_join(const char *dir, const char *name)
{
 size_t len = strlen(dir) + strlen(name) + 2;
 char *r = dm_malloc_aux((len), "device/dev-cache.c", 393);
 if (r)
  snprintf(r, len, "%s/%s", dir, name);

 return r;
}




static void _collapse_slashes(char *str)
{
 char *ptr;
 int was_slash = 0;

 for (ptr = str; *ptr; ptr++) {
  if (*ptr == '/') {
   if (was_slash)
    continue;

   was_slash = 1;
  } else
   was_slash = 0;
  *str++ = *ptr;
 }

 *str = *ptr;
}

static int _insert_dir(const char *dir)
{
 int n, dirent_count, r = 1;
 struct dirent **dirent;
 char *path;

 dirent_count = scandir(dir, &dirent, ((void *)0), alphasort);
 if (dirent_count > 0) {
  for (n = 0; n < dirent_count; n++) {
   if (dirent[n]->d_name[0] == '.') {
    free(dirent[n]);
    continue;
   }

   if (!(path = _join(dir, dirent[n]->d_name)))
    do { print_log(7, "device/dev-cache.c", 437 , 0,"<backtrace>"); return 0; } while (0);

   _collapse_slashes(path);
   r &= _insert(path, 1);
   free(path);

   free(dirent[n]);
  }
  free(dirent);
 }

 return r;
}

static int _insert_file(const char *path)
{
 struct stat info;

 if (stat(path, &info) < 0) {
  print_log(6, "device/dev-cache.c", 456 , 0,"%s: %s failed: %s", path, "stat", strerror((*__errno_location ())));
  return 0;
 }

 if (!((((info.st_mode)) & 0170000) == (0100000))) {
  print_log(7, "device/dev-cache.c", 461 , 0,"%s: Not a regular file", path);
  return 0;
 }

 if (!_insert_dev(path, 0))
  do { print_log(7, "device/dev-cache.c", 466 , 0,"<backtrace>"); return 0; } while (0);

 return 1;
}

static int _insert(const char *path, int rec)
{
 struct stat info;
 int r = 0;

 if (stat(path, &info) < 0) {
  print_log(6, "device/dev-cache.c", 477 , 0,"%s: %s failed: %s", path, "stat", strerror((*__errno_location ())));
  return 0;
 }

 if (((((info.st_mode)) & 0170000) == (0040000))) {

  if (lstat(path, &info) < 0) {
   print_log(6, "device/dev-cache.c", 484 , 0,"%s: %s failed: %s", path, "lstat", strerror((*__errno_location ())));
   return 0;
  }

  if (((((info.st_mode)) & 0170000) == (0120000))) {
   print_log(7, "device/dev-cache.c", 489 , 0,"%s: Symbolic link to directory", path);
   return 0;
  }

  if (rec)
   r = _insert_dir(path);

 } else {
  if (!((((info.st_mode)) & 0170000) == (0060000))) {
   print_log(7, "device/dev-cache.c", 498 , 0,"%s: Not a block device", path);
   return 0;
  }

  if (!_insert_dev(path, info.st_rdev))
   do { print_log(7, "device/dev-cache.c", 503 , 0,"<backtrace>"); return 0; } while (0);

  r = 1;
 }

 return r;
}

static void _full_scan(int dev_scan)
{
 struct dir_list *dl;

 if (_cache.has_scanned && !dev_scan)
  return;

 for (dl = ((__typeof__(*dl) *)((const char *)(((&_cache.dirs))->n) - (const char *)&((__typeof__(*dl) *) 0)->list)); &dl->list != ((&_cache.dirs)); dl = ((__typeof__(*dl) *)((const char *)(dl->list.n) - (const char *)&((__typeof__(*dl) *) 0)->list)))
  _insert_dir(dl->dir);

 for (dl = ((__typeof__(*dl) *)((const char *)(((&_cache.files))->n) - (const char *)&((__typeof__(*dl) *) 0)->list)); &dl->list != ((&_cache.files)); dl = ((__typeof__(*dl) *)((const char *)(dl->list.n) - (const char *)&((__typeof__(*dl) *) 0)->list)))
  _insert_file(dl->dir);

 _cache.has_scanned = 1;
 init_full_scan_done(1);
}

int dev_cache_has_scanned(void)
{
 return _cache.has_scanned;
}

void dev_cache_scan(int do_scan)
{
 if (!do_scan)
  _cache.has_scanned = 1;
 else
  _full_scan(1);
}

static int _init_preferred_names(struct cmd_context *cmd)
{
 const struct config_node *cn;
 const struct config_value *v;
 struct dm_pool *scratch = ((void *)0);
 const char **regex;
 unsigned count = 0;
 int i, r = 0;

 _cache.preferred_names_matcher = ((void *)0);

 if (!(cn = find_config_tree_node(cmd, "devices/preferred_names")) ||
     cn->v->type == CFG_EMPTY_ARRAY) {
  print_log(6,
 "device/dev-cache.c"
/* # 554 "device/dev-cache.c" */
  ,
 555
/* # 554 "device/dev-cache.c" */
  , 0,"devices/preferred_names not found in config file: " "using built-in preferences")
                                  ;
  return 1;
 }

 for (v = cn->v; v; v = v->next) {
  if (v->type != CFG_STRING) {
   print_log(3, "device/dev-cache.c", 561 , -1,"preferred_names patterns must be enclosed in quotes");
   return 0;
  }

  count++;
 }

 if (!(scratch = dm_pool_create("preferred device name matcher", 1024)))
  do { print_log(7, "device/dev-cache.c", 569 , 0,"<backtrace>"); return 0; } while (0);

 if (!(regex = dm_pool_alloc(scratch, sizeof(*regex) * count))) {
  print_log(3,
 "device/dev-cache.c"
/* # 572 "device/dev-cache.c" */
  ,
 573
/* # 572 "device/dev-cache.c" */
  , -1,"Failed to allocate preferred device name " "pattern list.")
                     ;
  goto out;
 }

 for (v = cn->v, i = count - 1; v; v = v->next, i--) {
  if (!(regex[i] = dm_pool_strdup(scratch, v->v.str))) {
   print_log(3,
 "device/dev-cache.c"
/* # 579 "device/dev-cache.c" */
   ,
 580
/* # 579 "device/dev-cache.c" */
   , -1,"Failed to allocate a preferred device name " "pattern.")
                 ;
   goto out;
  }
 }

 if (!(_cache.preferred_names_matcher =
  dm_regex_create(_cache.mem, regex, count))) {
  print_log(3, "device/dev-cache.c", 587 , -1,"Preferred device name pattern matcher creation failed.");
  goto out;
 }

 r = 1;

out:
 dm_pool_destroy(scratch);

 return r;
}

int dev_cache_init(struct cmd_context *cmd)
{
 _cache.names = ((void *)0);
 _cache.has_scanned = 0;

 if (!(_cache.mem = dm_pool_create("dev_cache", 10 * 1024)))
  do { print_log(7, "device/dev-cache.c", 605 , 0,"<backtrace>"); return 0; } while (0);

 if (!(_cache.names = dm_hash_create(128))) {
  dm_pool_destroy(_cache.mem);
  _cache.mem = 0;
  do { print_log(7, "device/dev-cache.c", 610 , 0,"<backtrace>"); return 0; } while (0);
 }

 if (!(_cache.devices = btree_create(_cache.mem))) {
  print_log(3, "device/dev-cache.c", 614 , -1,"Couldn't create binary tree for dev-cache.");
  goto bad;
 }

 if (!(_cache.dev_dir = dm_pool_strdup(_cache.mem, (cmd->dev_dir)))) {
  print_log(3, "device/dev-cache.c", 619 , -1,"strdup dev_dir failed.");
  goto bad;
 }

 dm_list_init(&_cache.dirs);
 dm_list_init(&_cache.files);

 if (!_init_preferred_names(cmd))
  do { print_log(7, "device/dev-cache.c", 627 , 0,"<backtrace>"); goto bad; } while (0);

 return 1;

      bad:
 dev_cache_exit();
 return 0;
}

static void _check_closed(struct device *dev)
{
 if (dev->fd >= 0)
  print_log(3, "device/dev-cache.c", 639 , -1,"Device '%s' has been left open.", dev_name(dev));
}

static void _check_for_open_devices(void)
{
 dm_hash_iter(_cache.names, (dm_hash_iterate_fn) _check_closed);
}

void dev_cache_exit(void)
{
 if (_cache.names)
  _check_for_open_devices();

 if (_cache.preferred_names_matcher)
  _cache.preferred_names_matcher = ((void *)0);

 if (_cache.mem) {
  dm_pool_destroy(_cache.mem);
  _cache.mem = ((void *)0);
 }

 if (_cache.names) {
  dm_hash_destroy(_cache.names);
  _cache.names = ((void *)0);
 }

 _cache.devices = ((void *)0);
 _cache.has_scanned = 0;
 dm_list_init(&_cache.dirs);
 dm_list_init(&_cache.files);
}

int dev_cache_add_dir(const char *path)
{
 struct dir_list *dl;
 struct stat st;

 if (stat(path, &st)) {
  print_log(3, "device/dev-cache.c", 677 , -1,"Ignoring %s: %s", path, strerror((*__errno_location ())));

  return 1;
 }

 if (!((((st.st_mode)) & 0170000) == (0040000))) {
  print_log(3, "device/dev-cache.c", 683 , -1,"Ignoring %s: Not a directory", path);
  return 1;
 }

 if (!(dl = dm_pool_zalloc(_cache.mem, (sizeof(*dl) + strlen(path) + 1)))) {
  print_log(3, "device/dev-cache.c", 688 , -1,"dir_list allocation failed");
  return 0;
 }

 strcpy(dl->dir, path);
 dm_list_add(&_cache.dirs, &dl->list);
 return 1;
}

int dev_cache_add_loopfile(const char *path)
{
 struct dir_list *dl;
 struct stat st;

 if (stat(path, &st)) {
  print_log(3, "device/dev-cache.c", 703 , -1,"Ignoring %s: %s", path, strerror((*__errno_location ())));

  return 1;
 }

 if (!((((st.st_mode)) & 0170000) == (0100000))) {
  print_log(3, "device/dev-cache.c", 709 , -1,"Ignoring %s: Not a regular file", path);
  return 1;
 }

 if (!(dl = dm_pool_zalloc(_cache.mem, (sizeof(*dl) + strlen(path) + 1)))) {
  print_log(3, "device/dev-cache.c", 714 , -1,"dir_list allocation failed for file");
  return 0;
 }

 strcpy(dl->dir, path);
 dm_list_add(&_cache.files, &dl->list);
 return 1;
}





const char *dev_name_confirmed(struct device *dev, int quiet)
{
 struct stat buf;
 const char *name;
 int r;

 if ((dev->flags & 0x00000002))
  return dev_name(dev);

 while ((r = stat(name = ((struct str_list *)((const char *)((dev->aliases.n)) - (const char *)&((struct str_list *) 0)->list))
                       ->str, &buf)) ||
        (buf.st_rdev != dev->dev)) {
  if (r < 0) {
   if (quiet)
    print_log(7, "device/dev-cache.c", 741 , 0,"%s: %s failed: %s", name, "stat", strerror((*__errno_location ())));
   else
    print_log(3, "device/dev-cache.c", 743 , -1,"%s: %s failed: %s", name, "stat", strerror((*__errno_location ())));
  }
  if (quiet)
   print_log(7,

 "device/dev-cache.c"
/* # 746 "device/dev-cache.c" */
   ,

 748
/* # 746 "device/dev-cache.c" */
   , 0,"Path %s no longer valid for device(%d,%d)", name, (int) ((dev->dev & 0xfff00) >> 8), (int) ((dev->dev & 0xff) | ((dev->dev >> 12) & 0xfff00)))

                            ;
  else
   print_log(3,

 "device/dev-cache.c"
/* # 750 "device/dev-cache.c" */
   ,

 752
/* # 750 "device/dev-cache.c" */
   , -1,"Path %s no longer valid for device(%d,%d)", name, (int) ((dev->dev & 0xfff00) >> 8), (int) ((dev->dev & 0xff) | ((dev->dev >> 12) & 0xfff00)))

                            ;


  dm_hash_remove(_cache.names, name);




  if (dm_list_size(&dev->aliases) > 1) {
   dm_list_del(dev->aliases.n);
   if (!r)
    _insert(name, 0);
   continue;
  }


  print_log(7,
 "device/dev-cache.c"
/* # 768 "device/dev-cache.c" */
  ,
 769
/* # 768 "device/dev-cache.c" */
  , 0,"Aborting - please provide new pathname for what " "used to be %s", name)
                           ;
  return ((void *)0);
 }

 return dev_name(dev);
}

struct device *dev_cache_get(const char *name, struct dev_filter *f)
{
 struct stat buf;
 struct device *d = (struct device *) dm_hash_lookup(_cache.names, name);

 if (d && (d->flags & 0x00000002))
  return d;


 if (d && (stat(name, &buf) || (buf.st_rdev != d->dev))) {
  dm_hash_remove(_cache.names, name);
  d = ((void *)0);
 }

 if (!d) {
  _insert(name, 0);
  d = (struct device *) dm_hash_lookup(_cache.names, name);
  if (!d) {
   _full_scan(0);
   d = (struct device *) dm_hash_lookup(_cache.names, name);
  }
 }

 return (d && (!f || (d->flags & 0x00000002) ||
        f->passes_filter(f, d))) ? d : ((void *)0);
}

struct dev_iter *dev_iter_create(struct dev_filter *f, int dev_scan)
{
 struct dev_iter *di = dm_malloc_aux((sizeof(*di)), "device/dev-cache.c", 805);

 if (!di) {
  print_log(3, "device/dev-cache.c", 808 , -1,"dev_iter allocation failed");
  return ((void *)0);
 }

 if (dev_scan && !trust_cache()) {

  if (!full_scan_done())
   persistent_filter_wipe(f);
 } else
  _full_scan(0);

 di->current = btree_first(_cache.devices);
 di->filter = f;
 di->filter->use_count++;

 return di;
}

void dev_iter_destroy(struct dev_iter *iter)
{
 iter->filter->use_count--;
 free(iter);
}

static struct device *_iter_next(struct dev_iter *iter)
{
 struct device *d = btree_get_data(iter->current);
 iter->current = btree_next(iter->current);
 return d;
}

struct device *dev_iter_get(struct dev_iter *iter)
{
 while (iter->current) {
  struct device *d = _iter_next(iter);
  if (!iter->filter || (d->flags & 0x00000002) ||
      iter->filter->passes_filter(iter->filter, d))
   return d;
 }

 return ((void *)0);
}

void dev_reset_error_count(struct cmd_context *cmd)
{
 struct dev_iter iter;

 if (!_cache.devices)
  return;

 iter.current = btree_first(_cache.devices);
 while (iter.current)
  _iter_next(&iter)->error_count = 0;
}

int dev_fd(struct device *dev)
{
 return dev->fd;
}

const char *dev_name(const struct device *dev)
{
 return (dev) ? ((struct str_list *)((const char *)((dev->aliases.n)) - (const char *)&((struct str_list *) 0)->list))->str :
     "unknown device";
}
