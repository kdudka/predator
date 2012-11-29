/*
 * Copyright (C) 2012 Jiri Simacek
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
#include "compiler.hh"
#include "integrity.hh"
#include "memplot.hh"
#include "regdef.hh"
#include "streams.hh"
#include "symstate.hh"
#include "virtualmachine.hh"


void SymState::init(
	SymState*                             parent,
	AbstractInstruction*                  instr,
	const std::shared_ptr<const FAE>&     fae,
	const std::shared_ptr<DataArray>&     regs)
{
	// Assertions
	assert(Integrity(*fae).check());

	instr_     = instr;
	fae_       = fae;
	regs_      = regs;

	this->setParent(parent);
}


void SymState::init(
	const SymState&                                oldState)
{
	instr_ = oldState.instr_;
	fae_   = oldState.fae_;
	regs_  = oldState.regs_;

	this->clearTree();
}


void SymState::init(
	const SymState&                                oldState,
	const std::shared_ptr<DataArray>               regs)
{
	instr_ = oldState.instr_;
	fae_   = oldState.fae_;
	regs_  = regs;

	this->clearTree();
}


void SymState::init(
	const SymState&                                oldState,
	const std::shared_ptr<DataArray>               regs,
	AbstractInstruction*                           insn)
{
	instr_ = insn;
	fae_   = oldState.fae_;
	regs_  = regs;

	this->clearTree();
}


void SymState::initChildFrom(
	SymState*                                      parent,
	AbstractInstruction*                           instr)
{
	// Assertions
	assert(nullptr != parent);
	assert(nullptr != instr);

	instr_  = instr;
	fae_    = parent->fae_;
	regs_   = parent->regs_;

	this->setParent(parent);
}


void SymState::initChildFrom(
	SymState*                                      parent,
	AbstractInstruction*                           instr,
	const std::shared_ptr<DataArray>               regs)
{
	// Assertions
	assert(nullptr != parent);
	assert(nullptr != instr);

	instr_  = instr;
	fae_    = parent->fae_;
	regs_   = regs;

	this->setParent(parent);
}


void SymState::recycle(Recycler<SymState>& recycler)
{
	if (nullptr != this->GetParent())
	{
		this->GetParent()->removeChild(this);
	}

	std::vector<SymState*> stack = { this };

	while (!stack.empty())
	{ // recycle recursively all children
		SymState* state = stack.back();
		stack.pop_back();

		assert(state->GetFAE());
		state->fae_ = nullptr;

		for (auto s : state->GetChildren())
		{
			stack.push_back(static_cast<SymState*>(s));
		}

		state->clearChildren();
		recycler.recycle(state);
	}
}


SymState::Trace SymState::getTrace() const
{
	Trace trace;

	const SymState* state = this;
	while (nullptr != state)
	{
		trace.push_back(state);
		state = static_cast<const SymState*>(state->GetParent());
	}

	return trace;
}


std::ostream& operator<<(std::ostream& os, const SymState& state)
{
	VirtualMachine vm(*state.GetFAE());

	// in case it changes, we should alter the printout
	assert(2 == FIXED_REG_COUNT);

	os << "{" << &state << "} global registers:";

	// there may be cases (at the beginning or end of a program) when ABP and GLOB
	// are not loaded
	os << " GLOB (gr" << GLOB_INDEX << ") = ";
	if (vm.varCount() <= GLOB_INDEX) { os << "(invld)"; }
	else { os << vm.varGet(GLOB_INDEX); }

	os << "  ABP (gr" <<  ABP_INDEX << ") = ";
	if (vm.varCount() <=  ABP_INDEX) { os << "(invld)"; }
	else { os << vm.varGet( ABP_INDEX); }

	for (size_t i = FIXED_REG_COUNT; i < vm.varCount(); ++i)
	{
		os << " gr" << i << '=' << vm.varGet(i);
	}
	os << "\n";

	os << "local registers: ";
	for (size_t i = 0; i < state.GetRegs().size(); ++i)
	{
		os << " r" << i << '=' << state.GetReg(i);
	}

	os << ", heap:" << std::endl << *state.GetFAE();

	return os << "instruction (" << state.GetInstr() << "): "
		<< *state.GetInstr();
}
