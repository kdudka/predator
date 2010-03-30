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

#ifndef H_GUARD_HASH_TABLE_H
#define H_GUARD_HASH_TABLE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file hash_table.h
 * generic hash table implementation written in pure C
 */

/**
 * type used for hash value
 *
 * It is guaranteed to be an integral (signed or unsigned) type.
 */
typedef unsigned ht_hash_t;

/**
 * type of function evaluating hash value for a given (hashable) item
 */
typedef ht_hash_t (*ht_hash_fnc_t)(const void *item);

/**
 * type of function comparing two (hashable) items for equality
 *
 * @param i1 The first item for comparison.
 * @param i2 The second item for comparison.
 * @return Return true if the given items are equal, false otherwise.
 */
typedef bool (*ht_eq_fnc_t)(const void *i1, const void *i2);

/**
 * type of function used to free a hashed item
 */
typedef void (*ht_free_fnc_t)(void *item);

/**
 * data internally used by hash table implementation
 *
 * You only need to keep pointer to this structure.
 */
struct ht_hash_table;

/**
 * create (empty) hash table
 *
 * @param htsize Size of hash table; will stay unchanged for the whole hash
 * table life time.
 * @param hash_fnc A function used to compute hash value for a given item.
 * @param eq_fnc A function used to compare given items for equality.
 * @param free_fnc A function used to free hashed item (for now only on hash
 * table destruction). If NULL is given as free_fnc, the hashed items will not
 * be freed.
 *
 * @return Return an initialized hash table object, NULL if memory is exhausted.
 */
struct ht_hash_table* ht_create(
        size_t              htsize,
        ht_hash_fnc_t       hash_fnc,
        ht_eq_fnc_t         eq_fnc,
        ht_free_fnc_t       free_fnc);

/**
 * destroy hash table
 *
 * This also calls a formerly given free_fnc function for all hashed items.
 *
 * @param ht Pointer to hash table internal data.
 */
void ht_destroy(struct ht_hash_table *ht);

/**
 * look for a given item within the hash table
 *
 * @param ht Pointer to hash table internal data.
 * @param item An item (or its template) to look for.
 * @return Returns the found item if any, NULL otherwise.
 */
void* ht_lookup(struct ht_hash_table *ht, const void *item);

/**
 * insert an item into the hash table if the item is not there already
 *
 * @param ht Pointer to hash table internal data.
 * @param item Item to look for and then eventually insert into hash table.
 * @return Returns the found item if it was already hashed. If not, it tries to
 * insert the item into the hash table and return the new item. It may also
 * return NULL, if memory is exhausted.
 */
void* ht_insert_if_needed(struct ht_hash_table *ht, void *item);

#ifdef __cplusplus
}
#endif

#endif /* H_GUARD_HASH_TABLE_H */
