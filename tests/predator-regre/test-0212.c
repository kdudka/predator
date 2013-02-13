#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <verifier-builtins.h>

#include "list.h"

static const char text[] = "text";

struct interface {
    struct interface* (*clone)(void *);
    void (*destroy)(void *);
    void (*print)(void *);
};

struct data_item {
    struct interface vtab;
    struct list_head list;
    union {
        long num;
        char str[PATH_MAX];
    } data;
};

struct interface* data_item_clone(void *ptr)
{
    struct data_item *const item = malloc(sizeof *item);
    return item ?: memcpy(item, ptr, sizeof *item);
}

void data_item_print_num(void *ptr)
{
    struct data_item *item = ptr;
    printf("%li\n", item->data.num);
}

void data_item_print_str(void *ptr)
{
    struct data_item *item = ptr;
    printf("%s\n", item->data.str);
}

struct interface data_item_vtab = {
    .clone = data_item_clone,
    .destroy = free
};

struct data_item* data_item_from_num(long num)
{
    struct data_item *item = malloc(sizeof *item);
    if (!item)
        return NULL;

    item->vtab = data_item_vtab;
    item->vtab.print = data_item_print_num;
    item->data.num = num;

    return item;
}

struct data_item* data_item_from_str(const char *str)
{
    struct data_item *item = malloc(sizeof *item);
    if (!item)
        return NULL;

    item->vtab = data_item_vtab;
    item->vtab.print = data_item_print_str;
    strncpy(item->data.str, str, sizeof item->data.str);

    return item;
}

struct data_list {
    struct interface vtab;
    struct list_head head;
};

struct data_list* data_list_create(void);

void data_list_destroy(void *ptr)
{
    struct data_list *list = ptr;
    struct data_item *pos, *n;
    list_for_each_entry_safe(pos, n, &list->head, list)
        pos->vtab.destroy(pos);

    free(ptr);
}

struct interface* data_list_clone(void *ptr)
{
    struct data_list *dup_list = data_list_create();
    if (!dup_list)
        return NULL;

    struct data_list *list = ptr;
    struct data_item *pos;
    list_for_each_entry(pos, &list->head, list) {
        struct data_item *dup_item = (void *) pos->vtab.clone(pos);
        if (!dup_item) {
            /* OOM cleanup */
            data_list_destroy(dup_list);
            return NULL;
        }

        list_add(&dup_item->list, &dup_list->head);
    }
}

void data_list_print(void *ptr)
{
    struct data_list *list = ptr;
    struct data_item *pos;
    list_for_each_entry(pos, &list->head, list)
        pos->vtab.print(pos);
}

struct interface data_list_vtab = {
    .clone      = data_list_clone,
    .destroy    = data_list_destroy,
    .print      = data_list_print
};

struct data_list* data_list_create(void)
{
    struct data_list *list = malloc(sizeof *list);
    if (!list)
        return NULL;

    list->vtab = data_list_vtab;
    INIT_LIST_HEAD(&list->head);
    return list;
}

void data_list_append(struct data_list *list, struct data_item *item)
{
    list_add(&item->list, &list->head);
}

int main()
{
    int rv = EXIT_FAILURE;

    struct data_list *list = data_list_create();
    if (!list)
        return EXIT_FAILURE;

    while (__VERIFIER_nondet_int()) {
        struct data_item *num_item = data_item_from_num(0L);
        if (!num_item)
            goto fail;

        data_list_append(list, num_item);
    }

    while (__VERIFIER_nondet_int()) {
        struct data_item *str_item = data_item_from_str(text);
        if (!str_item)
            goto fail;

        data_list_append(list, str_item);
    }

    rv = EXIT_SUCCESS;

fail:
    __VERIFIER_plot(NULL);
    list->vtab.destroy(list);
    return rv;
}

/**
 * @file test-0212.c
 *
 * @brief playing with C-level polymorphism
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
