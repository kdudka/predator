/*
 * Copyright (C) 2010-2011 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_PREDATOR_BUILTINS_H
#define H_GUARD_PREDATOR_BUILTINS_H

#define __VERIFIER_assert(cond) do {                                        \
    if (!(cond))                                                            \
        ___sl_error("assertion failed: " #cond);                            \
} while (0)

#define ___SL_BREAK_IF(cond) do {                                           \
    if (cond)                                                               \
        ___sl_break("conditional breakpoint fired: " #cond);                \
} while (0)

enum ___sl_module_id {
    ___SL_EVERYTHING,
    ___SL_SYMABSTRACT,
    ___SL_SYMJOIN,
    ___SL_GARBAGE_COLLECTOR
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PREDATOR
/* declare built-ins */
void ___sl_break(const char *msg);
void ___sl_error(const char *msg);
void __VERIFIER_plot(const char *name, ...);
void ___sl_plot_trace_now(const char *plot_name, const char *node_name);
void ___sl_plot_trace_once(const char *plot_name, const char *node_name);
void ___sl_enable_debugging_of(enum ___sl_module_id module, int enable);
int __VERIFIER_nondet_int(void);

#else
/*
 * dummy implementations of our built-ins
 *
 * NOTE: the use of 'inline' is tricky -- if used, it breaks some C parsers that
 * are not C99 compliant; if not used, gcc give us the following list of errors:
 *
 * sl.h: In function ‘int __VERIFIER_nondet_int()’:
 * sl.h:66:12: error: ‘i’ is used uninitialized in this function
 * sl.h: At global scope:
 * sl.h:47:27: error: ‘void ___sl_break(const char*)’ defined but not used
 * sl.h:52:26: error: ‘void ___sl_error(const char*)’ defined but not used
 * sl.h:57:26: error: ‘void ___sl_plot(const char*, ...)’ defined but not used
 * sl.h:62:25: error: ‘int __VERIFIER_nondet_int()’ defined but not used
 */

static /* inline */  void ___sl_break(const char *msg)
{
    (void) msg;
}

static /* inline */ void ___sl_error(const char *msg)
{
    (void) msg;
}

static /* inline */ void __VERIFIER_plot(const char *name, ...)
{
    (void) name;
}

static /* inline */ void ___sl_plot_trace_now(
        const char *plot_name,
        const char *node_name)
{
    (void) plot_name;
    (void) node_name;
}

static /* inline */ void ___sl_plot_trace_once(
        const char *plot_name,
        const char *node_name)
{
    (void) plot_name;
    (void) node_name;
}

static /* inline */ void
___sl_enable_debugging_of(enum ___sl_module_id module, int enable)
{
    (void) module;
    (void) enable;
}

static /* inline */ int __VERIFIER_nondet_int(void)
{
    int i;

    /* tools like valgrind will report a use of such value as a program error */
    return i;
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* H_GUARD_PREDATOR_BUILTINS_H */
