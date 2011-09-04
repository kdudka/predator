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

#ifndef SEQUENTIAL_INSTRUCTION_H
#define SEQUENTIAL_INSTRUCTION_H

#include <vector>
#include <unordered_map>

#include "abstractinstruction.hh"

class SequentialInstruction : public AbstractInstruction {

protected:

	AbstractInstruction* next_;

public:

	SequentialInstruction(bool computesFixpoint = false)
		: AbstractInstruction(computesFixpoint) {}

	virtual void finalize(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex,
		std::vector<AbstractInstruction*>::const_iterator cur
	);
	
};

#endif
