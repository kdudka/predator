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

#ifdef PREDATOR
// declare built-ins
void ___sl_break(void);
void ___sl_plot(const char *name, ...);
int ___sl_get_nondet_int(void);
void ___sl_error(const char *msg);

#else
// dummy implementations of our built-ins, useful to run our test-cases on bare
// metal, other analyzers without (compatible) plotting interface, etc.

static /* inline */  void ___sl_break(void)
{
}

static /* inline */ void ___sl_plot(const char *name, ...)
{
    (void) name;
}

static /* inline */ int ___sl_get_nondet_int(void)
{
    int i;
    return i;
}

static /* inline */ void ___sl_error(const char *msg)
{
    (void) msg;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_GUARD_SL_H */
