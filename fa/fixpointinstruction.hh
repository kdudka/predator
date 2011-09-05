/*
 * Copyright (C) 2011 Jiri Simacek
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

#ifndef FIXPOINT_INSTRUCTION_H
#define FIXPOINT_INSTRUCTION_H

#include "label.hh"
#include "treeaut.hh"
//#include "forestautext.hh"

#include "sequentialinstruction.hh"

class FixpointInstruction : public SequentialInstruction {

public:

	FixpointInstruction() : SequentialInstruction(e_fi_type::fiFix) {}

	virtual void extendFixpoint(const std::shared_ptr<const class FAE>& fae) = 0;

	virtual const TA<label_type>& getFixPoint() const = 0;

};

#endif
