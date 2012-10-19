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
#include "jump.hh"
#include "streams.hh"

void FI_jmp::execute(ExecutionManager& execMan, SymState& state)
{
	(void)execMan;
	(void)state;

	throw std::runtime_error("Reached unreachable operation: FI_jmp execution!");
}

void FI_jmp::finalize(
	const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex,
	std::vector<AbstractInstruction*>::const_iterator /* it */)
{
	this->next_ = this;

	while (this->next_->getType() == fi_type_e::fiJump)
	{
		this->next_ = (static_cast<FI_jmp*>(this->next_))->getTarget(codeIndex);
	}

	this->next_->setTarget();
}


SymState* FI_jmp::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)execMan;
	(void)fwdPred;
	(void)bwdSucc;

	throw std::runtime_error("Reached unreachable operation: FI_jmp reversal!");
}
