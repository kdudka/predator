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

#ifndef H_GUARD_CL_PP_H
#define H_GUARD_CL_PP_H

/**
 * @file cl_pp.hh
 * declaration of createClPrettyPrint() aka @b "pp"
 */

class ICodeListener;

/**
 * create "pp" (pretty print) ICodeListener implementation
 * @param config_string Name of a file to write to.  If NULL is given, stdout is
 * used.  If the output device is terminal the output may be colorized.
 * @param showTypes If true, dump type information as well.
 * @todo proper documentation of the "pp" code listener
 */
ICodeListener* createClPrettyPrint(const char *config_string, bool showTypes);

#endif /* H_GUARD_CL_PP_H */
