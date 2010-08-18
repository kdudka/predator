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

#ifdef SE_OPTIMIZED_BUILD
#   define SE_SELF_TEST                     0
#else
#   define SE_SELF_TEST                     1
#endif

/**
 * defined in version.c to avoid rebuild of all modules on git-commit
 */
extern const char *sl_git_sha1;

/**
 * jump to debugger by default in case anything interesting happens
 * @note this behavior may be subject for change in the future
 * @note for comfortable source code browsing, it's recommended to tweak your
 * editor, in order to highlight SE_TRAP as as keyword, as well as SE_BREAK_IF
 */
#define SE_TRAP do {                                                        \
    fprintf(stderr, "%s:%d: killing self by SIGTRAP [SHA1 %s]\n",           \
            __FILE__, __LINE__, sl_git_sha1);                               \
                                                                            \
    raise(SIGTRAP);                                                         \
} while (0)

#if SE_SELF_TEST
/**
 * conditional variant of TRAP, do nothing as long as cond is not satisfied
 * @attention the macro suffer from the same flaw as std::assert - the given
 * expression is not evaluated at all unless you're running a debug build
 * @note the macro has exactly opposite semantic than std::assert
 * @note for comfortable source code browsing, it's recommended to tweak your
 * editor, in order to highlight SE_BREAK_IF as as keyword, as well as SE_TRAP
 */
#   define SE_BREAK_IF(cond) do {                                           \
        if (!(cond))                                                        \
            break;                                                          \
                                                                            \
        fprintf(stderr, "%s:%d: conditional breakpoint fired: "             \
                "SE_BREAK_IF(%s)\n", __FILE__, __LINE__, #cond);            \
                                                                            \
        SE_TRAP;                                                            \
    } while (0)

#else /* SE_SELF_TEST */
#   define SE_BREAK_IF(cond) do { } while (0)
#endif

/**
 * if 1, print created/destroyed stack variables when running in verbose mode
 */
#define DEBUG_SE_STACK_FRAME                0

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
