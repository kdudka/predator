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

#ifndef H_GUARD_SL_H
#define H_GUARD_SL_H

#ifdef __cplusplus
extern "C" {
#endif

#define ___SL_PLOT_STACK_FRAME(fnc, name) \
    ___sl_plot_stack_frame((void (*)()) fnc, name)

#define ___SL_PLOT_FNC(fnc) \
    ___SL_PLOT_STACK_FRAME(fnc, #fnc)

#ifdef PREDATOR
// declare built-ins
void ___sl_break(void);
void ___sl_plot(const char *name);
void ___sl_plot_stack_frame(void (*fnc)(), const char *name);
void ___sl_plot_by_ptr(const void *ptr, const char *name);

#else
// dummy implementations of our built-ins, useful to run our test-cases on bare
// metal, other analyzers without (compatible) plotting interface, etc.

static inline void ___sl_break(void)
{
}

static inline void ___sl_plot(const char *name)
{
    (void) name;
}

static inline void ___sl_plot_stack_frame(void (*fnc)(), const char *name)
{
    (void) fnc;
    (void) name;
}

static inline void ___sl_plot_by_ptr(const void *ptr, const char *name) {
    (void) ptr;
    (void) name;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_GUARD_SL_H */
