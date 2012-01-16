#include <verifier-builtins.h>

struct dm_list {
    struct dm_list *n, *p;
};

struct device {
    struct dm_list aliases;
    int dev;
    int fd;
    int open_count;
    int error_count;
    int max_error_count;
    int block_size;
    int read_ahead;
    unsigned flags;
    unsigned end;
    struct dm_list open_list;
    char pvid[32 + 1];
    char _padding[7];
};

struct str_list {
    struct dm_list list;
    const char *str;
};

const char *dev_name(const struct device *dev)
{
    return ((struct str_list *)
            ((const char *)((dev->aliases.n)) -
             (const char *)&((struct str_list *) 0)->list)
           )->str;
}

int main()
{
    struct device dev;
    return !!dev_name(&dev);
}
