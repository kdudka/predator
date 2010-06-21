/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
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

/**
 * @file config.h
 * various compile-time options
 */

#include <signal.h>     /* needed for TRAP ... raise(SIGTRAP)       */
#include <stdio.h>      /* needed for TRAP ... fprintf(stderr, ...) */

/**
 * defined in version.c to avoid rebuild of all modules on git-commit
 */
extern const char *sl_git_sha1;

/**
 * jump to debugger by default in case anything interesting happens
 * @note this behavior may be subject for change in the future
 */
#define TRAP do { \
    fprintf(stderr, "%s:%d: killing self by SIGTRAP [SHA1 %s]\n", \
            __FILE__, __LINE__, sl_git_sha1); \
    raise(SIGTRAP); \
} while (0)

/**
 * if 1, print created/destroyed stack variables when running in verbose mode
 */
#define DEBUG_SE_STACK_FRAME                0

/**
 * if 1, define TObjId and TValueId is integers (makes debugging easier)
 */
#define DEBUG_SYMID_FORCE_INT               0

/**
 * if 1, comment out the body of SymHeap2::abstract
 */
#define SE_DISABLE_ABSTRACT                 0

/**
 * if 1, call optimization is not performed; it may introduce some compile-time
 * warnings and memory leaks as a side effect
 */
#define SE_DISABLE_CALL_CACHE               0

/**
 * if 1, do not use the @b symcut module at all
 */
#define SE_DISABLE_SYMCUT                   0

/**
 * maximal call depth
 * @attention Be careful here, setting it to big value may cause a stack
 * overflow because of the current suboptimal approach used in
 * SymExec::Private::execInsnCall()
 */
#define SE_MAX_CALL_DEPTH                   0x100

/**
 * if 1, SymHeapUnion uses (trivial) hash optimization
 */
#define SE_STATE_HASH_OPTIMIZATION          0

/**
 * if 1, turn on extensive debugging of hash optimization with significant
 * performance impact and a lot of noise in the debug output
 */
#define SE_STATE_HASH_OPTIMIZATION_DEBUG    0
