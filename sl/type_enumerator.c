/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of sl.
 *
 * sl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sl.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "type_enumerator.h"

#include "hash_table.h"

#include <assert.h>
#include <stdlib.h>

#define TE_NEW(type) \
    (type *) malloc(sizeof(type))

enum {
    TE_CHUNK = 0x100
};

struct te_item {
    void                    *key;
    struct cl_type          *type;
};

struct typen_data {
    typen_free_fnc          free_fnc;
    struct ht_hash_table    *htab;
    cl_type_uid_t           last_uid;
    size_t                  cnt_allocated;
    struct te_item          *items;
};

static ht_hash_t te_hash_fnc(const void *p)
{
    struct te_item *item = (struct te_item *) p;
    void *key = item->key;
    return /* FIXME: not optimal hashing */ ((long) key) >> sizeof(long);
}

static bool te_eq_fnc(const void *i1, const void *i2)
{
    const void *key1 = ((const struct te_item *) i1)->key;
    const void *key2 = ((const struct te_item *) i2)->key;
    return key1 == key2;
}

static void te_dummy_free_fnc(struct cl_type *clt)
{
    (void) clt;
}

struct typen_data* typen_create(typen_free_fnc free_fnc)
{
    struct typen_data *te = TE_NEW(struct typen_data);
    if (!te)
        return NULL;

    te->free_fnc = (free_fnc)
        ? free_fnc
        : te_dummy_free_fnc;

    te->htab = ht_create(/* FIXME: hardcoded for now */ 0x100,
                         te_hash_fnc,
                         te_eq_fnc,
                         /* te_free_fnc */ NULL);

    if (!te->htab) {
        free(te);
        return NULL;
    }

    te->last_uid        = 0;
    te->cnt_allocated   = 0;
    te->items           = NULL;
    return te;
}

void typen_destroy(struct typen_data *te)
{
    int i;
    for(i = 0; i < te->last_uid - 1; ++i)
        te->free_fnc(te->items[i].type);
    free(te->items);

    ht_destroy(te->htab);
    free(te);
}

static void* te_realloc_if_needed(struct typen_data *te)
{
    if (te->cnt_allocated <= (size_t) te->last_uid) {
        te->cnt_allocated += TE_CHUNK;
        te->items = (struct te_item *)
            realloc(te->items, te->cnt_allocated * sizeof(struct te_item));
    }

    return te->items;
}

struct cl_type* typen_insert_as_new(struct typen_data *te, struct cl_type *type,
                                    void *key)
{
    cl_type_uid_t uid = te->last_uid;
    struct te_item *item;
    struct te_item *item_new;
    if (!te_realloc_if_needed(te))
        // OOM
        return NULL;

    type->uid = uid + 1;
    te->last_uid = type->uid;

    item = te->items + uid;
    item->key = key;
    item->type = type;

    item_new = ht_insert_if_needed(te->htab, item);
    assert(item == item_new);
    return type;
}

struct cl_type* typen_get_by_key(struct typen_data *te, void *key)
{
    struct te_item *item;
    struct te_item tpl;
    tpl.key = key;
    item = (struct te_item *) ht_lookup(te->htab, &tpl);
    return (item)
        ? item->type
        : NULL;
}

struct cl_type* typen_get_by_uid(struct typen_data *te, cl_type_uid_t uid)
{
    assert(0 < uid);
    if (te->last_uid < uid)
        return NULL;

    return te->items[uid - 1].type;
}
