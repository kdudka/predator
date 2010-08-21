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

#ifndef H_GUARD_CLUTIL_H
#define H_GUARD_CLUTIL_H

/**
 * @file clutil.hh
 * some generic utilities working on top of code_listener/CodeStorage
 * @todo API documentation
 */

#include "config.h"

#include <cl/code_listener.h>

const struct cl_type* targetTypeOfPtr(const struct cl_type *clt);

bool seekRefAccessor(const struct cl_accessor *ac);

int intCstFromOperand(const struct cl_operand *op);

int varIdFromOperand(const struct cl_operand *op, const char **pName = 0);

#endif /* H_GUARD_CLUTIL_H */
