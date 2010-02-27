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

#ifndef H_GUARD_CLD_UNIREGS_H
#define H_GUARD_CLD_UNIREGS_H

/**
 * @file cld_uniregs.hh
 * declaration of createCldUniRegs() - code listener decorator @b
 * "unify_regs"
 */

class ICodeListener;

/**
 * create code listener decorator for register unification
 * @return on heap allocated instance of ICodeListener object
 */
ICodeListener* createCldUniRegs(ICodeListener *);

#endif /* H_GUARD_CLD_UNIREGS_H */
