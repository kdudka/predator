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

#ifndef H_GUARD_TYPE_ENUMERATOR_H
#define H_GUARD_TYPE_ENUMERATOR_H

#include "code_listener.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*typen_free_fnc)(struct cl_type *);

struct typen_data;
struct typen_data* typen_create(typen_free_fnc);
void typen_destroy(struct typen_data *);

struct cl_type* typen_insert_as_new(struct typen_data *, struct cl_type *type,
                                    void *key);

struct cl_type* typen_get_by_key(struct typen_data *, void *key);
struct cl_type* typen_get_by_uid(struct typen_data *, cl_type_uid_t);

#ifdef __cplusplus
}
#endif

#endif /* H_GUARD_TYPE_ENUMERATOR_H */
