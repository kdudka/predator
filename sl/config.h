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

/**
 * if 1, check each code_listener decorator by the integrity checker
 */
#define CL_DEBUG_CLD                    0

/**
 * if 1, use ugly temporary workaround for missing type info of sparse args
 */
#define CLD_ARG_SUBST_KEEP_TYPE_REF     1

/**
 * type enumerator debug level; set to 0 to disable debugging
 */
#define DEBUG_TYPE_ENUMERATOR           0
