/*
 * Copyright (C) 2009-2011 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_CONFIG_H
#define H_GUARD_CONFIG_H

#define GIT_SHA1 sl_git_sha1
#include "trap.h"

/**
 * if 1, print amount of allocated memory on certain places when in verbose mode
 */
#ifndef NDEBUG
#   define DEBUG_MEM_USAGE                  1
#else
#   define DEBUG_MEM_USAGE                  0
#endif

/**
 * if 1, print block scheduler statistics whenever end of a fnc is not reached
 */
#define DEBUG_SE_END_NOT_REACHED            0

/**
 * - 0 ... do not debug branching by non-deterministic conditions
 * - 1 ... plot heap graphs of branching by non-deterministic abstract values
 * - 2 ... plot heap graphs of branching by any non-deterministic conditions
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
 * if 1, plot some interesting operations as being performed by symcall
 */
#define DEBUG_SYMCALL                       0

/**
 * if 1, the symcut module prints some extra debugging info
 */
#define DEBUG_SYMCUT                        0

/**
 * if 1, plot shape of the data structures being leaked
 */
#define DEBUG_SYMGC                         0

/**
 * if 1, symjoin prints some extra debugging info
 */
#define DEBUG_SYMJOIN                       0

/**
 * if 1, SymState plots heap graphs per each heap comparison (expensive)
 */
#define DEBUG_SYMSTATE                      0

/**
 * if 1, perform abstraction after each just completed call on @b caller's side
 */
#define SE_ABSTRACT_ON_CALL_DONE            1

/**
 * if 1, do not perform abstraction on each end of BB, but only when looping
 */
#define SE_ABSTRACT_ON_LOOP_EDGES_ONLY      1

/*
 * if non-zero, allow incomplete discovery paths with lower costs to apply
 */
#define SE_ALLOW_SUBPATH_RANKING            0

/**
 * - 0 ... avoid creation of a new integral range from two integral constants
 * - 1 ... allow to create integral ranges from integral constants if needed
 * - 2 ... same as above, additionally use widening in upward direction [broken]
 * - 3 ... same as above, additionally use widening in both directions [broken]
 */
#define SE_ALLOW_INT_RANGES                 0

/**
 * - 0 ... do not use values with offset specified by int ranges (VT_RANGE)
 * - 1 ... allow to use values with offset specified by int ranges (VT_RANGE)
 * - 2 ... same as above, additionally use widening in upward direction [broken]
 * - 3 ... same as above, additionally use widening in both directions [broken]
 */
#define SE_ALLOW_OFF_RANGES                 1

/**
 * how much do we allow to use three-way join
 * - 0 ... never ever
 * - 1 ... only when joining prototypes
 * - 2 ... also when joining states if the three-way join is considered useful
 * - 3 ... do not restrict the usage of three-way join at the level of symjoin
 */
#define SE_ALLOW_THREE_WAY_JOIN             2

/**
 * if 1, assume that the contents of static data is initialized on first access
 */
#define SE_ASSUME_FRESH_STATIC_DATA         1

/**
 * call cache miss count that will trigger function removal (0 means disabled)
 */
#define SE_CALL_CACHE_MISS_THR              0x10

/**
 * increase the cost of abstraction path consisting of concrete objects only by
 */
#define SE_COST_OF_SEG_INTRODUCTION         0

/**
 * if 1, do not use alignment tracking unless we get it for free
 */
#define SE_DISABLE_ALIGNMENT_TRACKING       0

/**
 * if 1, do not use DLS (Doubly-linked List Segment) abstraction
 */
#define SE_DISABLE_DLS                      0

/**
 * if 1, do not use SLS (Singly-linked List Segment) abstraction
 */
#define SE_DISABLE_SLS                      0

/**
 * if 1, do not use the @b symcut module at all
 */
#define SE_DISABLE_SYMCUT                   0

/**
 * - 0 ... do not dump trace graphs unless explicitly asked to do so
 * - 1 ... dump a single trace graph for all errors/warnings (may be huge)
 * - 2 ... dump a single trace graph for each error/warning
 */
#define SE_DUMP_TRACE_GRAPHS                1

/**
 * - 0 ... kill local variables only on stack frame destruction
 * - 1 ... kill local variables as soon as they become dead
 */
#define SE_EARLY_VARS_DESTRUCTION           1

/**
 * - 0 ... call cache completely disabled (saves a lot of memory)
 * - 1 ... call cache enabled, use graph isomorphism for lookup (hungry on mem)
 * - 2 ... call cache enabled, use join operator for lookup [experimental]
 */
#define SE_ENABLE_CALL_CACHE                1

/**
 * - 0 no error recovery, stop the analysis as soon as an error is detected
 * - 1 low-cost error recovery (stop analyzing paths with errors already caught)
 * - 2 full error recovery (keep analyzing until a non-recoverable error occurs)
 */
#define SE_ERROR_RECOVERY_MODE              1

/**
 * the highest integral number we can count to (only partial implementation atm)
 */
#define SE_INT_ARITHMETIC_LIMIT             8

/**
 * - 0 ... join states on each basic block entry
 * - 1 ... join only when traversing a loop-closing edge, entailment otherwise
 * - 2 ... join only when traversing a loop-closing edge, isomorphism otherwise
 */
#define SE_JOIN_ON_LOOP_EDGES_ONLY          2

/**
 * maximal call depth
 */
#define SE_MAX_CALL_DEPTH                   0x40

/**
 * if non-zero, plot each state that caused an error to be reported
 */
#define SE_PLOT_ERROR_STATES                0

/**
 * cost of merged prototype where one case was more generic than the other case
 */
#define SE_PROTO_COST_ASYM                  1

/**
 * cost of merged prototype that is generalisation of both input prototypes
 * @note only values >= SE_PROTO_COST_ASYM make sense
 */
#define SE_PROTO_COST_THREEWAY              4

/**
 * upper bound of DLS minimal length (zero means unlimited)
 */
#define SE_RESTRICT_DLS_MINLEN              2

/**
 * upper bound of SLS minimal length (zero means unlimited)
 */
#define SE_RESTRICT_SLS_MINLEN              2

/**
 * - 0 ... keep state info for all basic blocks of a function (safe default)
 * - 1 ... keep state info for all basic blocks with more than one ingoing edge
 * - 2 ... keep state info for all basic blocks that a CFG loop starts with
 */
#define SE_STATE_PRUNING_MODE               0

/**
 * if 1, the symcut module allows generic minimal lengths to survive a function
 * call/return.  @b Not recommended unless SymCallCache has been rewritten to
 * use symjoin to maintain cache entries.
 */
#define SE_SYMCUT_PRESERVES_MIN_LENGTHS     1

/**
 * - 0 ... disable tracking non-pointer values
 * - 1 ... basic tracking of non-pointer values
 * - 2 ... expensive tracking of pointer values
 * - 3 ... track also code pointers
 */
#define SE_TRACK_NON_POINTER_VALUES         2

/**
 * if 1, use a DFS scheduler at the level of basic blocks; if 0, use BFS
 */
#define SE_USE_DFS_SCHEDULER                1

/**
 * if 1, do not make deep copy on copy of SymHeap [experimental]
 */
#define SH_COPY_ON_WRITE                    1

/**
 * if 1, do not destroy objects immediately as they become unused
 */
#define SH_DELAYED_OBJECTS_DESTRUCTION      1

/**
 * if 1, allow to assign unused heap IDs to newly created heap entities
 */
#define SH_REUSE_FREE_IDS                   0

/**
 * if 1, write the contents of both parts of a DLS pair
 */
#define SYMPLOT_DEBUG_DLS                   0

/**
 * if 1, do not plot allocated areas as clusters
 */
#define SYMPLOT_FLAT_MODE                   0

/**
 * if 1, skip plotting of "neq" edges (makes the result more readable by humans
 * in certain cases)
 */
#define SYMPLOT_OMIT_NEQ_EDGES              1

/**
 * if more than zero, jump to debugger as soon as N graph of the same name has
 * been plotted
 */
#define SYMPLOT_STOP_AFTER_N_STATES         0

#if 0
#define SYMPLOT_STOP_CONDITION(name) \
    (!(name).compare("symabstract-0003-DLS-0001-0000"))
#endif

#endif /* H_GUARD_CONFIG_H */
