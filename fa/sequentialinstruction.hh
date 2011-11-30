/*
 * Copyright (C) 2011 Jiri Simacek
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

#ifndef SEQUENTIAL_INSTRUCTION_H
#define SEQUENTIAL_INSTRUCTION_H

#include "abstractinstruction.hh"

class SequentialInstruction : public AbstractInstruction {

protected:

	AbstractInstruction* next_;

public:

	SequentialInstruction(fi_type_e fiType = fi_type_e::fiUnspec)
		: AbstractInstruction(fiType) {}

	virtual void finalize(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex,
		std::vector<AbstractInstruction*>::const_iterator cur
	);

	AbstractInstruction* next() const { return this->next_; }

};

#endif
