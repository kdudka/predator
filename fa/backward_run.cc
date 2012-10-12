/*
 * Copyright (C) 2012  Ondrej Lengal
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
#include "backward_run.hh"


bool BackwardRun::isSpuriousCE(SymState::Trace& fwdTrace)
{
	// Assertions
	assert(!fwdTrace.empty());

	// iterator from the leaf of the forward run
	auto itFwdTrace = fwdTrace.cbegin();

	// the backward trace
	std::vector<SymState*> bwdTrace;

	// check that the iterator does not point to a nullptr
	assert(nullptr != *itFwdTrace);

	// copy the error state
	SymState* bwdState = execMan_.copyState(**itFwdTrace);

	bwdTrace.push_back(bwdState);

	while (++itFwdTrace != fwdTrace.cend())
	{	// until we hit the initial state of the execution trace
		const SymState* fwdState = *itFwdTrace;
		assert(nullptr != fwdState);

		const AbstractInstruction* instr = fwdState->GetInstr();
		assert(nullptr != instr);

//		instr->reverseAndIsect(execMan_, *fwdState, *bwdState);
	}

	for (SymState* st : bwdTrace)
	{	// recycle states on the trace
		assert(nullptr != st);

		execMan_.recycleState(st);
	}

	return true;
}
