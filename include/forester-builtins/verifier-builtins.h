/*
 * Copyright (C) 2012  Ondrej Lengal
 *
 * This file is part of forester.
 *
 * forester is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * forester is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with forester.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef H_GUARD_FORESTER_BUILTINS_H
#define H_GUARD_FORESTER_BUILTINS_H

#ifdef __cplusplus
extern "C" {
#endif

#define __VERIFIER_assert(cond) do {                                        \
  if (!(cond))                                                              \
    ___fa_error("assertion failed: " #cond);                                \
} while (0)

#define __VERIFIER_nondet_int()  ___fa_get_nondet_int()

#ifdef FORESTER

/* declare built-ins */
void ___fa_error(const char *msg);
void __VERIFIER_plot(const char *name, ...);
void ___fa_plot_trace_now(const char *plot_name, const char *node_name);
void ___fa_plot_trace_once(const char *plot_name, const char *node_name);
int ___fa_get_nondet_int(void);

#else

/*
 * dummy implementations of our built-ins
 *
 * NOTE: the use of 'inline' is tricky -- if used, it breaks some C parsers that
 * are not C99 compliant; if not used, gcc give us the following list of errors:
 *
 * sl.h: In function ‘int ___sl_get_nondet_int()’:
 * sl.h:66:12: error: ‘i’ is used uninitialized in this function
 * sl.h: At global scope:
 * sl.h:47:27: error: ‘void ___sl_break(const char*)’ defined but not used
 * sl.h:52:26: error: ‘void ___sl_error(const char*)’ defined but not used
 * sl.h:57:26: error: ‘void ___sl_plot(const char*, ...)’ defined but not used
 * sl.h:62:25: error: ‘int ___sl_get_nondet_int()’ defined but not used
 */

static /* inline */ void ___fa_error(const char *msg)
{
	(void) msg;
}

static /* inline */ void __VERIFIER_plot(const char *name, ...)
{
	(void) name;
}

static /* inline */ void ___fa_plot_trace_now(
	const char *plot_name,
	const char *node_name)
{
	(void) plot_name;
	(void) node_name;
}

static /* inline */ void ___fa_plot_trace_once(
	const char *plot_name,
	const char *node_name)
{
	(void) plot_name;
	(void) node_name;
}

static /* inline */ int ___fa_get_nondet_int(void)
{
	int i;

	/* tools like valgrind will report a use of such value as a program error */
	return i;
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* H_GUARD_FORESTER_BUILTINS_H */
