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

// Forester headers
#include "abstractinstruction.hh"

/**
 * @file sequentialinstruction.hh
 * SequentialInstruction - abstract base class for sequential instructions
 */


/**
 * @brief  An abstract base class that represents a sequential instruction
 *
 * Abstract class representing a sequential instruction. This class serves as
 * the base class for all sequential instructions.
 */
class SequentialInstruction : public AbstractInstruction {

protected:

	/// the following instruction
	AbstractInstruction* next_;


private: // methods

	SequentialInstruction(const SequentialInstruction&);
	SequentialInstruction& operator=(const SequentialInstruction&);

public:

	/**
	 * @brief  The constructor
	 *
	 * ``Default'' constructor, creates a new sequential instruction of given type
	 * for given instruction in CL's code storage.
	 *
	 * @param[in]  insn    Source instruction in the CL's code storage
	 * @param[in]  fiType  The type of the instruction (from #fi_type_e)
	 */
	SequentialInstruction(const CodeStorage::Insn* insn = nullptr,
		fi_type_e fiType = fi_type_e::fiUnspec)
		: AbstractInstruction(insn, fiType),
		next_{nullptr}
	{ }

	/**
	 * @copydoc AbstractInstruction::finalize
	 */
	virtual void finalize(
		const std::unordered_map<const CodeStorage::Block*,
		AbstractInstruction*>& codeIndex,
		std::vector<AbstractInstruction*>::const_iterator cur
	);

	/**
	 * @brief  Gets the next instruction
	 *
	 * Method that retrieves the next instruction in the sequence.
	 * 
	 * @returns  The next instruction in the sequence
	 */
	AbstractInstruction* next() const { return this->next_; }
};

#endif
