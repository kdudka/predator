/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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

#include <stdio.h>      /* needed for TRAP ... fprintf(stderr, ...) */

/**
 * defined in version.c to avoid rebuild of all modules on git-commit
 */
extern const char *GIT_SHA1;

#ifdef USE_INT3_AS_BRK
#   define _CL_BREAK                        __asm__("int3")
#   define _CL_BREAK_MECH                   "INT3"
#else
#   include <signal.h>
#   define _CL_BREAK                        raise(SIGTRAP)
#   define _CL_BREAK_MECH                   "SIGTRAP"
#endif

/**
 * jump to debugger by default in case anything interesting happens
 * @note this behavior may be subject for change in the future
 * @note for comfortable source code browsing, it's recommended to tweak your
 * editor, in order to highlight CL_TRAP as as keyword, as well as CL_BREAK_IF
 */
#define CL_TRAP do {                                                        \
    fprintf(stderr, "%s:%d: killing self by %s [SHA1 %s]\n",                \
            __FILE__, __LINE__, _CL_BREAK_MECH, GIT_SHA1);                  \
                                                                            \
    _CL_BREAK;                                                              \
} while (0)

#ifndef NDEBUG
/**
 * conditional variant of CL_TRAP, do nothing as long as cond is not satisfied
 * @attention the macro suffer from the same flaw as std::assert - the given
 * expression is not evaluated at all unless you're running a debug build
 * @note the macro has exactly opposite semantic than std::assert
 * @note for comfortable source code browsing, it's recommended to tweak your
 * editor, in order to highlight CL_BREAK_IF as as keyword, as well as CL_TRAP
 */
#   define CL_BREAK_IF(cond) do {                                           \
        if (!(cond))                                                        \
            break;                                                          \
                                                                            \
        fprintf(stderr, "%s:%d: conditional breakpoint fired: "             \
                "CL_BREAK_IF(%s)\n", __FILE__, __LINE__, #cond);            \
                                                                            \
        CL_TRAP;                                                            \
    } while (0)

#else /* NDEBUG */
#   define CL_BREAK_IF(cond) do { } while (0)
#endif
