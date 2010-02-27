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

#ifndef H_GUARD_CL_DOTGEN_H
#define H_GUARD_CL_DOTGEN_H

/**
 * @file cl_dotgen.hh
 * declaration of createClDotGenerator() aka @b "dotgen"
 */

class ICodeListener;

/**
 * create "dotgen" ICodeListener implementation
 * @param config_string Name of global dot file is the only configuration string
 * for now. It's optional, you can pass NULL to prevent the global dot file from
 * creating.
 * @todo proper documentation of the "dotgen" code listener
 */
ICodeListener* createClDotGenerator(const char *config_string);

#endif /* H_GUARD_CL_DOTGEN_H */
