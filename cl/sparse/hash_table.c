/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of predator.
 *
 * predator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * predator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with predator.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hash_table.h"

#include <assert.h>
#include <stdlib.h>

#define HT_NEW(type) \
    (type *) malloc(sizeof(type))

struct ht_hash_item {
    void                    *data;
    struct ht_hash_item     *next;
};

struct ht_hash_slot {
    struct ht_hash_item     *head;
    struct ht_hash_item     **tail;
};

struct ht_hash_table {
    ht_hash_fnc_t           hash_fnc;
    ht_eq_fnc_t             eq_fnc;
    ht_free_fnc_t           free_fnc;
    size_t                  size;
    struct ht_hash_slot     *table;
};

static void ht_dummy_free_fnc(void *ptr)
{
    (void) ptr;
}

struct ht_hash_table* ht_create(
        size_t              htsize,
        ht_hash_fnc_t       hash_fnc,
        ht_eq_fnc_t         eq_fnc,
        ht_free_fnc_t       free_fnc)
{
    unsigned i;
    struct ht_hash_table *ht = HT_NEW(struct ht_hash_table);
    if (!ht)
        // out of memory
        return NULL;

    // initialize hash table
    ht->hash_fnc            = hash_fnc;
    ht->eq_fnc              = eq_fnc;
    ht->free_fnc            = (free_fnc)
                                ? free_fnc
                                : ht_dummy_free_fnc;
    ht->size                = htsize;
    ht->table               = (struct ht_hash_slot *)
        calloc(htsize, sizeof(struct ht_hash_slot));

    if (!ht->table) {
        // out of memory
        free(ht);
        return NULL;
    }

    // initialize all slots
    for(i = 0; i < htsize; ++i) {
        struct ht_hash_slot *slot = ht->table + i;
        slot->tail = &slot->head;
    }

    return ht;
}

void ht_destroy(struct ht_hash_table *ht)
{
    unsigned i;
    assert(ht && ht->table);

    // free all data managed by hash table
    for (i = 0; i < ht->size; ++i) {
        struct ht_hash_item *item = ht->table[i].head;
        while (item) {
            struct ht_hash_item *next = item->next;
            ht->free_fnc(item->data);
            free(item);
            item = next;
        }
    }

    // free hash table itself
    free(ht->table);
    free(ht);
}

static struct ht_hash_slot* ht_get_slot(struct ht_hash_table *ht,
                                        const void *what)
{
    ht_hash_t hash = ht->hash_fnc(what);
    unsigned idx = hash % ht->size;
    return ht->table + idx;
}

static struct ht_hash_item* ht_seek_item(struct ht_hash_table *ht,
                                         struct ht_hash_item *head,
                                         const void *what)
{
    struct ht_hash_item *item = head;
    for(; item; item = item->next)
        if (ht->eq_fnc(item->data, what))
            return item;

    // not found
    return NULL;
}

void* ht_lookup(struct ht_hash_table *ht, const void *what)
{
    struct ht_hash_slot *slot = ht_get_slot(ht, what);
    struct ht_hash_item *item = ht_seek_item(ht, slot->head, what);

    return (item)
        ? item->data
        : NULL;
}

void* ht_insert_if_needed(struct ht_hash_table *ht, void *what)
{
    struct ht_hash_slot *slot = ht_get_slot(ht, what);
    struct ht_hash_item *item = ht_seek_item(ht, slot->head, what);
    if (item)
        // item already hashed, return the original one
        return item->data;

    item = HT_NEW(struct ht_hash_item);
    if (!item)
        // out of memory
        return NULL;

    // initialize ht item
    item->data = what;
    item->next = NULL;

    // append the item at end of the list
    *slot->tail = item;
    slot->tail = &item->next;

    return what;
}
