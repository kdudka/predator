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

#ifndef CALL_H
#define CALL_H

#include "abstractinstruction.hh"

/**
 * @brief  Performs a return from a function
 */
class FI_ret : public AbstractInstruction
{
	/// Index of the register with the return address
	size_t dst_;

public:

	FI_ret(const CodeStorage::Insn* insn, size_t dst)
		: AbstractInstruction(insn), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual SymState* reverseAndIsect(
		ExecutionManager&                      execMan,
		const SymState&                        fwdPred,
		const SymState&                        bwdSucc) const;

	virtual void finalize(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>&,
		std::vector<AbstractInstruction*>::const_iterator)
	{ }

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "ret   \tr" << this->dst_;
	}

};

#endif
