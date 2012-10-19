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
#include "call.hh"
#include "executionmanager.hh"
#include "streams.hh"

// FI_ret
void FI_ret::execute(ExecutionManager& execMan, SymState& state)
{
	// Assertions
	assert(state.GetReg(dst_).isNativePtr());
	assert(nullptr != static_cast<AbstractInstruction*>(state.GetReg(dst_).d_native_ptr));

	SymState* tmpState = execMan.createChildState(state,
		static_cast<AbstractInstruction*>(state.GetReg(dst_).d_native_ptr));

	execMan.enqueue(tmpState);
}


SymState* FI_ret::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)fwdPred;

	// Assertions
	assert(bwdSucc.GetReg(dst_).isNativePtr());
	assert(nullptr != static_cast<AbstractInstruction*>(bwdSucc.GetReg(dst_).d_native_ptr));
	assert(fwdPred.GetReg(dst_) == bwdSucc.GetReg(dst_));

	SymState* tmpState = execMan.copyState(bwdSucc);

	return tmpState;
}

