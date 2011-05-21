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

#define GIT_SHA1 sl_git_sha1
#include "trap.h"

/**
 * if 1, print amount of allocated memory on certain places when in verbose mode
 */
#define DEBUG_MEM_USAGE                     1

/**
 * if 1, plot heap graphs of branching by non-deterministic conditions
 */
#define DEBUG_SE_NONDET_COND                0

/**
 * if 1, print created/destroyed variables when running in verbose mode
 */
#define DEBUG_SE_STACK_FRAME                0

/**
 * if 1, plot each abstraction/concretization step to a separate heap graph
 */
#define DEBUG_SYMABSTRACT                   0

/**
 * if 1, the symcut module prints some extra debugging info
 */
#define DEBUG_SYMCUT                        0

/**
 * if 1, SymPlot prints some extra debugging info
 */
#define DEBUG_SYMPLOT                       0

/**
 * if 1, symjoin prints some extra debugging info
 */
#define DEBUG_SYMJOIN                       0

/**
 * if 1, SymState plots heap graphs per each heap comparison (expensive)
 */
#define DEBUG_SYMSTATE                      0

/**
 * if 1, SymState::insert() prints the effect of each invocation
 */
#define DEBUG_SYMSTATE_INSERT               0

/**
 * if 1, call cache is not used at all
 */
#define SE_DISABLE_CALL_CACHE               0

/**
 * if 1, perform abstraction after each just completed call on @b caller's side
 */
#define SE_ABSTRACT_ON_CALL_DONE            1

/**
 * if 1, local variables are destroyed as soon as they become dead
 */
#define SE_EARLY_VARS_DESTRUCTION           1

/**
 * if 1, program variables are created as needed; before a fnc call otherwise
 */
#define SE_LAZY_VARS_CREATION               1

/**
 * if 1, do not use DLS (Doubly-linked List Segment) abstracion
 */
#define SE_DISABLE_DLS                      0

/**
 * if 1, do not use SLS (Signly-linked List Segment) abstracion
 */
#define SE_DISABLE_SLS                      0

/**
 * if 1, do not use the @b symcut module at all
 */
#define SE_DISABLE_SYMCUT                   0

/**
 * if 1, do not use the @b symjoin module for symbolic state management
 */
#define SE_DISABLE_SYMJOIN_IN_SYMSTATE      0

/**
 * if 1, do not allow three-way join
 */
#define SE_DISABLE_THREE_WAY_JOIN           0

/**
 * if 1, abstraction path length is penalized by complexity of prototype join
 */
#define SE_PREFER_LOSSLESS_PROTOTYPES       1

/**
 * maximal call depth
 */
#define SE_MAX_CALL_DEPTH                   0x40

/**
 * if 1, support partial tracking of non-pointer values, which may result into
 * significant state explosion in some cases
 */
#define SE_TRACK_NON_POINTER_VALUES         1

/**
 * if more than zero, jump to debugger as soon as N graph of the same name has
 * been plotted
 */
#define SYMPLOT_STOP_AFTER_N_STATES         0

#if 0
#define SYMPLOT_STOP_CONDITION(name) \
    (!(name).compare("symabstract-0003-DLS-0001-0000"))
#endif
