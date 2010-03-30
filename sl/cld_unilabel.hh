/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_CLD_UNILABEL_H
#define H_GUARD_CLD_UNILABEL_H

/**
 * @file cld_unilabel.hh
 * constructor createCldUniLabel() - of the @b "unify_labels_*" decorators
 */

#include "code_listener.h"

class ICodeListener;

/**
 * create code listener decorator for label unification
 * @param scope unification level - CL_SCOPE_GLOBAL, CL_SCOPE_STATIC or
 * CL_SCOPE_FUNCTION
 *
 * @return on heap allocated instance of ICodeListener object
 */
ICodeListener* createCldUniLabel(ICodeListener *, cl_scope_e scope);

#endif /* H_GUARD_CLD_UNILABEL_H */
