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
 * if 1, print created/destroyed stack variables when running in verbose mode
 */
#define DEBUG_SE_STACK_FRAME                0

/**
 * if 1, plot each abstraction step into a separate heap graph
 */
#define DEBUG_SYMABSTRACT                   1

/**
 * if 1, the symcut module prints some extra debugging info
 */
#define DEBUG_SYMCUT                        0

/**
 * if 1, SymPlot prints some extra debugging info
 */
#define DEBUG_SYMPLOT                       0

/**
 * if 1, define TObjId and TValueId is integers (makes debugging easier)
 */
#define DEBUG_SYMID_FORCE_INT               0

/**
 * if 1, SymHeapCore leaves a debug message when a value became unused
 */
#define DEBUG_UNUSED_VALUES                 0

/**
 * if 1, the garbage collector does not see objects pointed beyond the root as
 * garbage
 */
#define GC_ADMIT_LINUX_LISTS                1

/**
 * if 1, call optimization is not performed; it may introduce some compile-time
 * warnings and memory leaks as a side effect
 */
#define SE_DISABLE_CALL_CACHE               0

/**
 * if 1, perform abstraction after each just completed call on @b caller's side
 */
#define SE_ABSTRACT_ON_CALL_DONE            1

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
 * maximal call depth
 */
#define SE_MAX_CALL_DEPTH                   0x100

/**
 * if more than zero, jump to debugger as soon as N graph of the same name has
 * been plotted
 */
#define SYMPLOT_STOP_AFTER_N_STATES         0x40
