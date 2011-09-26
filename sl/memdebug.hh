/*
 * Copyright (C) 2011 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_MEM_DEBUG_H
#define H_GUARD_MEM_DEBUG_H

#include <string>

/**
 * @file memdebug.hh
 * @todo some dox
 */

/// provide the raw amount of currently allocated memory (as glibc reports it)
bool rawMemUsage(size_t *pDst);

/// initialize memory debugging, taking the current memory state as state zero
bool initMemDrif();

/// provide relative amount of currently allocated memory (subtracting drift)
bool currentMemUsage(size_t *pDst);

/// print the current amount of allocated memory
bool printMemUsage(const char *justCompletedFncName);

#endif /* H_GUARD_MEM_DEBUG_H */
