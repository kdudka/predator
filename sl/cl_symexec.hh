/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of sl.
 *
 * sl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sl.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef H_GUARD_CL_SYMEXEC_H
#define H_GUARD_CL_SYMEXEC_H

/**
 * @file cl_symexec.hh
 * constructor createClSymExec() of the @b "symexec" code listener - see SymExec
 */

class ICodeListener;

/**
 * create "symexec" ICodeListener implementation - see SymExec
 * @param config_string If "fast" is given, some computationally expensive
 * analysis will be omitted.
 * @todo proper documentation of the "symexec" code listener
 */
ICodeListener* createClSymExec(const char *config_string);

#endif /* H_GUARD_CL_SYMEXEC_H */
