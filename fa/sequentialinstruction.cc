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

// Forester headers
#include "executionmanager.hh"
#include "sequentialinstruction.hh"
#include "jump.hh"

void SequentialInstruction::finalize(
	const std::unordered_map<const CodeStorage::Block*,
		AbstractInstruction*>& codeIndex,
	std::vector<AbstractInstruction*>::const_iterator cur)
{
	this->next_ = *(cur + 1);

	while (this->next_->getType() == fi_type_e::fiJump)
	{ // shortcut jump instruction
		assert(dynamic_cast<FI_jmp*>(this->next_) != nullptr);

		this->next_ = static_cast<FI_jmp*>(this->next_)->getTarget(codeIndex);
	}
}

SymState* RegisterAssignment::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	// copy the previous value of register dstReg_ 
	SymState* tmpState = execMan.copyStateWithNewRegs(bwdSucc, fwdPred.GetInstr());
	tmpState->SetReg(dstReg_, fwdPred.GetReg(dstReg_));

	return tmpState;
}

SymState* VoidInstruction::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)fwdPred;

	return execMan.copyState(bwdSucc);
}
