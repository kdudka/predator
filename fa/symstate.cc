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


void SymState::SubstituteRefs(
	const SymState&      src,
	const Data&          oldValue,
	const Data&          newValue)
{
	// Preconditions
	assert(oldValue.isRef() && newValue.isRef());

	struct RootState
	{
		size_t root;
		size_t state;

		RootState(size_t pRoot, size_t pState) :
			root(pRoot),
			state(pState)
		{ }

		bool operator<(const RootState& rhs) const
		{
			if (root < rhs.root)
			{
				return true;
			}
			else if (root == rhs.root)
			{
				return state < rhs.state;
			}
			else
			{
				return false;
			}
		}
	};

	typedef std::pair<RootState, RootState> ProdState;

	const std::shared_ptr<const FAE> thisFAE = this->GetFAE();
	const std::shared_ptr<const FAE> srcFAE = src.GetFAE();
	assert((nullptr != thisFAE) && (nullptr != srcFAE));

	FAE* fae = new FAE(*thisFAE);
	fae->clear();

	for (size_t i = 0; i < thisFAE->getRootCount(); ++i)
	{	// allocate existing TA
		if (nullptr != thisFAE->getRoot(i))
		{
			TreeAut* ta =fae->allocTA();
			ta->addFinalStates(thisFAE->getRoot(i)->getFinalStates());
			fae->appendRoot(ta);
		}
		else
		{
			fae->appendRoot(nullptr);
		}

		fae->connectionGraph.newRoot();
	}

	std::set<ProdState> processed;
	std::vector<ProdState> workset;

	assert(thisFAE->GetVarCount() == srcFAE->GetVarCount());
	for (size_t i = 0; i < thisFAE->GetVarCount(); ++i)
	{	// copy global variables
		Data thisVar = thisFAE->GetVar(i);
		const Data& srcVar = srcFAE->GetVar(i);

		if (srcVar.isRef() && thisVar.isUndef())
		{	// in case we need to substitute at global variable
			assert(oldValue == srcVar);
			thisVar = newValue;
		}

		fae->PushVar(thisVar);

		if (!srcVar.isRef() || !thisVar.isRef())
		{	// in case some of them is not a reference
			assert(srcVar == thisVar);
			continue;
		}

		assert(thisVar.isRef() && srcVar.isRef());
		assert(thisVar.d_ref.displ == srcVar.d_ref.displ);

		const TreeAut* thisRoot = thisFAE->getRoot(thisVar.d_ref.root).get();
		const TreeAut* srcRoot  = srcFAE->getRoot(srcVar.d_ref.root).get();
		assert((nullptr != thisRoot) && (nullptr != srcRoot));

		ProdState initialState(
			RootState(thisVar.d_ref.root, thisRoot->getFinalState()),
			RootState(srcVar.d_ref.root, srcRoot->getFinalState())
		);

		workset.push_back(initialState);
		processed.insert(initialState);
	}

	assert(this->GetRegCount() == src.GetRegCount());
	for (size_t i = 0; i < this->GetRegCount(); ++i)
	{	// copy registers
		Data thisVar = this->GetReg(i);
		const Data& srcVar = src.GetReg(i);

		if (srcVar.isRef() && thisVar.isUndef())
		{	// in case we need to substitute at global variable
			assert(oldValue == srcVar);
			this->SetReg(i, thisVar);
			thisVar = newValue;
		}

		if (!srcVar.isRef() || !thisVar.isRef())
		{	// in case some of them is not a reference
			assert(srcVar == thisVar);
			continue;
		}

		assert(thisVar.isRef() && srcVar.isRef());
		assert(thisVar.d_ref.displ == srcVar.d_ref.displ);

		const TreeAut* thisRoot = thisFAE->getRoot(thisVar.d_ref.root).get();
		const TreeAut* srcRoot  = srcFAE->getRoot(srcVar.d_ref.root).get();
		assert((nullptr != thisRoot) && (nullptr != srcRoot));

		ProdState initialState(
			RootState(thisVar.d_ref.root, thisRoot->getFinalState()),
			RootState(srcVar.d_ref.root, srcRoot->getFinalState())
		);

		workset.push_back(initialState);
		processed.insert(initialState);
	}


	while (!workset.empty())
	{
		const ProdState curState = *workset.crbegin();
		workset.pop_back();

		const size_t& thisRoot = curState.first.root;
		const size_t& srcRoot = curState.second.root;

		const std::shared_ptr<TreeAut> thisTA = thisFAE->getRoot(thisRoot);
		const std::shared_ptr<TreeAut> srcTA = srcFAE->getRoot(srcRoot);
		assert((nullptr != thisTA) && (nullptr != srcTA));

		const size_t& thisState = curState.first.state;
		const size_t& srcState = curState.second.state;

		TreeAut::iterator thisIt = thisTA->begin(thisState);
		TreeAut::iterator thisEnd = thisTA->end(thisState, thisIt);
		TreeAut::iterator srcIt = srcTA->begin(srcState);
		TreeAut::iterator srcEnd = srcTA->end(srcState, srcIt);

		for (; thisIt != thisEnd; ++thisIt)
		{
			for (; srcIt != srcEnd; ++srcIt)
			{
				const Transition& thisTrans = *thisIt;
				const Transition& srcTrans = *srcIt;

				if (thisTrans.label() == srcTrans.label())
				{
					assert(thisTrans.lhs().size() == srcTrans.lhs().size());

					if (thisTrans.label()->isData())
					{	// data are processed one level up
						assert(srcTrans.label()->isData());
						continue;
					}

					std::vector<size_t> lhs;

					for (size_t i = 0; i < thisTrans.lhs().size(); ++i)
					{	// for each pair of states that map to each other
						const Data* srcData;
						if (srcFAE->isData(srcTrans.lhs()[i], srcData))
						{	// for data states
							assert(nullptr != srcData);

							const Data* thisData;
							if (!thisFAE->isData(thisTrans.lhs()[i], thisData))
							{
								assert(false);       // fail gracefully
							}

							assert(nullptr != thisData);

							if (oldValue == *srcData)
							{	// in case we are at the right value
								assert(thisData->isUndef());
								// TODO: or may it reference itself?

								assert(false);
								// TODO: we should now change the undef to newValue

								// add sth to 'lhs'
							}
							else if (srcData->isRef())
							{	// for the case of other reference
								assert(thisData->isRef());
								assert(thisData->d_ref.displ == srcData->d_ref.displ);

								const size_t& thisNewRoot = thisData->d_ref.root;
								const size_t& srcNewRoot  = srcData->d_ref.root;

								const TreeAut* thisNewTA = thisFAE->getRoot(thisData->d_ref.root).get();
								const TreeAut* srcNewTA  = srcFAE->getRoot(srcData->d_ref.root).get();
								assert((nullptr != thisNewTA) && (nullptr != srcNewTA));

								ProdState jumpState(
									RootState(thisNewRoot, thisNewTA->getFinalState()),
									RootState(srcNewRoot, srcNewTA->getFinalState())
								);

								if (processed.insert(jumpState).second)
								{	// in case the state has not been processed before
									workset.push_back(jumpState);
								}

								lhs.push_back(fae->addData(*fae->getRoot(thisRoot).get(), *thisData));
							}
							else
							{	// for other data
								lhs.push_back(fae->addData(*fae->getRoot(thisRoot).get(), *thisData));
							}
						}
						else
						{
							const ProdState newState(std::make_pair(
								RootState(thisRoot, thisTrans.lhs()[i]),
								RootState(srcRoot, srcTrans.lhs()[i])
							));

							if (processed.insert(newState).second)
							{	// in case the state has not been processed before
								workset.push_back(newState);
							}
						}
					}

					fae->getRoot(thisRoot)->addTransition(lhs, thisTrans.label(), thisTrans.rhs());
				}
				else
				{
					FA_LOG("not-matching: " << *thisIt << ", " << *srcIt);
				}
			}
		}
	}

	fae->updateConnectionGraph();
	this->SetFAE(std::shared_ptr<FAE>(fae));
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
