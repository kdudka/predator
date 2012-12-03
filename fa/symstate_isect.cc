/*
 * Copyright (C) 2012 Ondrej Lengal
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

/**
 * @file symstate_isect.cc
 *
 * This file contains implementation of functions of SymState that perform
 * intersection of a pair of symbolic states.
 */

// Forester headers
#include "streams.hh"
#include "symstate.hh"


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

	friend std::ostream& operator<<(std::ostream& os, const RootState& rs)
	{
		os << "[" << rs.root << ", " << FA::writeState(rs.state) << "]";
		return os;
	}
};


typedef std::pair<RootState, RootState> ProdState;


void SymState::SubstituteRefs(
	const SymState&      src,
	const Data&          oldValue,
	const Data&          newValue)
{
	// Preconditions
	assert(oldValue.isRef() && newValue.isRef());

	const std::shared_ptr<const FAE> thisFAE = this->GetFAE();
	const std::shared_ptr<const FAE> srcFAE = src.GetFAE();
	assert((nullptr != thisFAE) && (nullptr != srcFAE));

	FAE* fae = new FAE(*thisFAE);
	fae->clear();

	for (size_t i = 0; i < thisFAE->getRootCount(); ++i)
	{	// allocate existing TA
		if (nullptr != thisFAE->getRoot(i))
		{
			TreeAut* ta = fae->allocTA();
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
		assert(0 == thisVar.d_ref.displ);

		const TreeAut* thisRoot = thisFAE->getRoot(thisVar.d_ref.root).get();
		const TreeAut* srcRoot  = srcFAE->getRoot(srcVar.d_ref.root).get();
		assert((nullptr != thisRoot) && (nullptr != srcRoot));

		ProdState initialState(
			RootState(thisVar.d_ref.root, thisRoot->getFinalState()),
			RootState(srcVar.d_ref.root, srcRoot->getFinalState())
		);

		if (processed.insert(initialState).second)
		{	// in case the state has not been processed before
			workset.push_back(initialState);
		}
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
		assert(0 == thisVar.d_ref.displ);

		const TreeAut* thisRoot = thisFAE->getRoot(thisVar.d_ref.root).get();
		const TreeAut* srcRoot  = srcFAE->getRoot(srcVar.d_ref.root).get();
		assert((nullptr != thisRoot) && (nullptr != srcRoot));

		ProdState initialState(
			RootState(thisVar.d_ref.root, thisRoot->getFinalState()),
			RootState(srcVar.d_ref.root, srcRoot->getFinalState())
		);

		if (processed.insert(initialState).second)
		{	// in case the state has not been processed before
			workset.push_back(initialState);
		}
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
						const Data* srcData = nullptr;
						if (srcFAE->isData(srcTrans.lhs()[i], srcData))
						{	// for data states
							assert(nullptr != srcData);

							const Data* thisData = nullptr;
							if (!thisFAE->isData(thisTrans.lhs()[i], thisData))
							{
								assert(false);       // fail gracefully
							}

							assert(nullptr != thisData);

							if (oldValue == *srcData)
							{	// in case we are at the right value
								assert(thisData->isUndef());

								// TODO: or may it reference itself?

								lhs.push_back(fae->addData(*fae->getRoot(thisRoot).get(), newValue));
							}
							else if (srcData->isRef())
							{	// for the case of other reference
								assert(thisData->isRef());
								assert(thisData->d_ref.displ == srcData->d_ref.displ);
								assert(0 == thisData->d_ref.displ);

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

							// TODO: add to lhs?
							assert(false);
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


// anonymous namespace
namespace
{
class IsectEngine
{
private:  // data members

	/// FAE to be the output
	FAE& fae_;

	/// Maps product states to states in the new automaton, e.g. (p,q) -> r
	std::map<ProdState, RootState> processed_;

	/// The work stack
	std::vector<std::pair<ProdState, RootState>> workstack_;

	/// Maps pairs of roots to a root in the new automaton
	std::map<std::pair<size_t, size_t>, size_t> rootMap_;

	/// Counter of roots in the new FAE
	size_t rootCnt_;

public:   // methods

	explicit IsectEngine(FAE& fae) :
		fae_(fae),
		processed_(),
		workstack_(),
		rootMap_(),
		rootCnt_(0)
	{ }

	/**
	 * @brief  Adds a state to the automaton if it does not exist
	 *
	 * Inserts a product state in the automaton in case it does not exist. In
	 * either case returns the number of the root and the number of the state in
	 * the target FAE (in the form of a pair). In case it is the first state of
	 * a new automaton (previously unknown combination of root numbers), it is set
	 * as the final state.
	 *
	 * @param[in]  lhsRoot   Root number of the LHS automaton
	 * @param[in]  lhsState  State in the LHS automaton
	 * @param[in]  rhsRoot   Root number of the RHS automaton
	 * @param[in]  rhsState  State in the RHS automaton
	 *
	 * @returns  Root in the new FAE and the number of the product state
	 */
	RootState makeProductState(
		const size_t&          lhsRoot,
		const size_t&          lhsState,
		const size_t&          rhsRoot,
		const size_t&          rhsState)
	{
		// the product state
		RootState lhsRootState(lhsRoot, lhsState);
		RootState rhsRootState(rhsRoot, rhsState);
		ProdState prodState(lhsRootState, rhsRootState);

		// find the root # in the aut (or create one)
		auto itBoolPairRootMap = rootMap_.insert(std::make_pair(
			std::make_pair(lhsRoot, rhsRoot), rootCnt_));
		bool isNewRoot = itBoolPairRootMap.second;
		if (isNewRoot)
		{
			FA_NOTE("Creating new root: " << rootCnt_ << " as the product of roots ("
				<< lhsRoot << ", " << rhsRoot << ")");
			++rootCnt_;

			TreeAut* ta = fae_.allocTA();
			fae_.appendRoot(ta);
			fae_.connectionGraph.newRoot();
		}

		// the actual number of the root
		const size_t& root = itBoolPairRootMap.first->second;

		RootState newState(root, fae_.nextState());
		auto itBoolPairProcessed = processed_.insert(std::make_pair(prodState, newState));
		bool isNewState = itBoolPairProcessed.second;

		// isNewRoot -> isNewState
		assert(!isNewRoot || isNewState);

		if (isNewState)
		{	// in case the state has not been processed before
			FA_NOTE("Creating new state: " << FA::writeState(fae_.nextState())
				<< " as the product of states (" <<  lhsRootState << ", "
				<< rhsRootState << ")");
			workstack_.push_back(std::make_pair(prodState, newState));
			fae_.newState();
		}

		// the actual number of the state
		const size_t& state = itBoolPairProcessed.first->second.state;

		if (isNewRoot)
		{	// set final state
			fae_.getRoot(root)->addFinalState(state);
		}

		return RootState(root, state);
	}


	/**
	 * @brief  Determines whether the work stack is empty
	 *
	 * This method returns @p true in case the work stack is empty, @p false
	 * otherwise.
	 *
	 * @returns  @p true in case the work stack is empty, @p false otherwise
	 */
	bool wsEmpty() const
	{
		return workstack_.empty();
	}

	/**
	 * @brief  Retrieves the next state to be processed
	 *
	 * This method retrieves the next state that has not been processed so far.
	 * Further, it removes it from the work stack.
	 *
	 * @returns  A pair of a product state and corresponding root and state in new
	 *           FAE
	 */
	std::pair<ProdState, RootState> getNextState()
	{
		// Preconditions
		assert(!wsEmpty());

		std::pair<ProdState, RootState> res = *workstack_.crbegin();
		workstack_.pop_back();
		return res;
	}

	std::vector<size_t> getRootOrderIndexForRHS() const
	{
		std::vector<size_t> index(fae_.getRootCount(), static_cast<size_t>(-1));

		for (const std::pair<const std::pair<size_t, size_t>, size_t>&
			productRootPair : rootMap_)
		{
			const size_t& oldState = productRootPair.first.second;
			const size_t& newState = productRootPair.second;
			assert(index[newState] == static_cast<size_t>(-1));
			index[newState] = oldState;
		}

		for (const size_t& i : index)
		{
			(void)i;
			assert(i != static_cast<size_t>(-1));
		}

		return index;
	}
};
} // namespace


void SymState::Intersect(
	const SymState&          fwd)
{
	const std::shared_ptr<const FAE> thisFAE = this->GetFAE();
	const std::shared_ptr<const FAE> fwdFAE = fwd.GetFAE();
	assert((nullptr != thisFAE) && (nullptr != fwdFAE));

	FAE* fae = new FAE(*thisFAE);
	fae->clear();
	this->SetFAE(std::shared_ptr<FAE>(fae));

	// engine that handles creation of new states etc.
	IsectEngine engine(*fae);

	if (this->GetRegCount() != fwd.GetRegCount())
	{	// if the number of local registers does not match
		FA_LOG("Number of local registers does not match -> creating empty intersection");
		return;      // empty FA
	}

	if (thisFAE->GetVarCount() != fwdFAE->GetVarCount())
	{	// if the number of input ports of the FAE does not match
		FA_LOG("Number of input ports does not match -> creating empty intersection");
		return;      // empty FA
	}

	assert(thisFAE->GetVarCount() == fwdFAE->GetVarCount());
	for (size_t i = 0; i < thisFAE->GetVarCount(); ++i)
	{	// check global variables
		const Data& thisVar = thisFAE->GetVar(i);
		const Data& fwdVar = fwdFAE->GetVar(i);

		if (!thisVar.isRef() && !fwdVar.isRef())
		{	// in case of non-references
			if (thisVar != fwdVar)
			{
				return;   // empty FA
			}
		}
		else
		{
			assert(thisVar.isRef() && fwdVar.isRef());
		}
	}

	assert(this->GetRegCount() == fwd.GetRegCount());
	for (size_t i = 0; i < this->GetRegCount(); ++i)
	{	// check local registers

		// NOTE: this needs to be done in order to also collect temporary TA
		// references for parts of heap which has not been so far connected so that
		// they would be reachable from global variables

		const Data& thisVar = this->GetReg(i);
		const Data& fwdVar = fwd.GetReg(i);

		if (!thisVar.isRef() && !fwdVar.isRef())
		{	// in case of non-references
			if (thisVar != fwdVar)
			{
				return;   // empty FA
			}
		}
		else
		{
			assert(thisVar.isRef() && fwdVar.isRef());
		}
	}

	assert(thisFAE->GetVarCount() == fwdFAE->GetVarCount());
	for (size_t i = 0; i < thisFAE->GetVarCount(); ++i)
	{	// add processing of all global variables
		const Data& thisVar = thisFAE->GetVar(i);
		const Data& fwdVar = fwdFAE->GetVar(i);

		if (!thisVar.isRef() || !fwdVar.isRef())
		{	// in case some of them is not a reference
			assert(fwdVar == thisVar);
			fae->PushVar(thisVar);
		}
		else
		{
			assert(thisVar.isRef() && fwdVar.isRef());
			assert(thisVar.d_ref.displ == fwdVar.d_ref.displ);
			assert(0 == thisVar.d_ref.displ);

			const size_t thisRootNum = thisVar.d_ref.root;
			const size_t fwdRootNum = fwdVar.d_ref.root;

			const TreeAut* thisRoot = thisFAE->getRoot(thisRootNum).get();
			const TreeAut* fwdRoot  = fwdFAE->getRoot(fwdRootNum).get();
			assert((nullptr != thisRoot) && (nullptr != fwdRoot));

			RootState rs = engine.makeProductState(
				thisRootNum, thisRoot->getFinalState(),
				fwdRootNum, fwdRoot->getFinalState()
			);

			fae->PushVar(Data::createRef(rs.root));
		}
	}

	while (!engine.wsEmpty())
	{
		const std::pair<ProdState, RootState> curProdState = engine.getNextState();

		const ProdState& curState = curProdState.first;
		const RootState& curNewState = curProdState.second;

		FA_NOTE("Processing product state (" << curState.first << ", " << curState.second << ")");

		const size_t& thisRoot = curState.first.root;
		const size_t& fwdRoot = curState.second.root;

		const std::shared_ptr<TreeAut> thisTA = thisFAE->getRoot(thisRoot);
		const std::shared_ptr<TreeAut> fwdTA = fwdFAE->getRoot(fwdRoot);
		assert((nullptr != thisTA) && (nullptr != fwdTA));

		const size_t& thisState = curState.first.state;
		const size_t& fwdState = curState.second.state;

		TreeAut::iterator thisIt = thisTA->begin(thisState);
		TreeAut::iterator thisEnd = thisTA->end(thisState, thisIt);
		TreeAut::iterator fwdIt = fwdTA->begin(fwdState);
		TreeAut::iterator fwdEnd = fwdTA->end(fwdState, fwdIt);

		for (; thisIt != thisEnd; ++thisIt)
		{
			for (; fwdIt != fwdEnd; ++fwdIt)
			{
				const Transition& thisTrans = *thisIt;
				const Transition& fwdTrans = *fwdIt;

				// we handle data one level up
				assert(!thisTrans.label()->isData() && !fwdTrans.label()->isData());
				assert(!thisTrans.lhs().empty() && !fwdTrans.lhs().empty());

				// TODO: so far, we are not doing unfolding!
				if (thisTrans.label() == fwdTrans.label())
				{
					FA_NOTE("Transition: " << thisTrans.label());

					assert(thisTrans.lhs().size() == fwdTrans.lhs().size());
					const size_t& transArity = thisTrans.lhs().size();

					if (thisTrans.label()->isData())
					{	// data are processed one level up
						assert(fwdTrans.label()->isData());
						continue;
					}

					// handle ordinary transitions
					std::vector<size_t> lhs;
					size_t i;
					for (i = 0; i < transArity; ++i)
					{	// for each pair of states that map to each other
						const Data* fwdData = nullptr, *thisData = nullptr;
						bool  fwdIsData =  fwdFAE->isData( fwdTrans.lhs()[i],  fwdData);
						bool thisIsData = thisFAE->isData(thisTrans.lhs()[i], thisData);

						if (!fwdIsData && !thisIsData)
						{	// ************* process internal states *************
							// This is the easiest case, when both states in the product are
							// internal. We do not create any new automaton.
							assert((nullptr == fwdData) && (nullptr == thisData));

							RootState rootState = engine.makeProductState(
								thisRoot, thisTrans.lhs()[i],
								fwdRoot, fwdTrans.lhs()[i]);

							// check that we have not created a new automaton
							assert(rootState.root == curNewState.root);

							lhs.push_back(rootState.state);
						}
						else if (fwdIsData && thisIsData &&
							!fwdData->isRef() && !thisData->isRef())
						{ // ************* process real data states (leaves) *************
							// This is the second easiest case, when both states are real data
							// (i.e. no references). In this case, we are simply doing
							// intersection of the data.
							assert(*thisData == *fwdData);

							lhs.push_back(fae->addData(
								*fae->getRoot(curNewState.root).get(), *thisData));
						}
						else if (fwdIsData && thisIsData &&
							fwdData->isRef() && thisData->isRef())
						{ // ************* process reference states (leaves) *************
							// This is another quite easy case, when both states are
							// references to another automata. In this case, we are jumping
							// from both automata into another product automaton.
							assert(thisData->d_ref.displ == fwdData->d_ref.displ);
							assert(0 == thisData->d_ref.displ);

							const size_t& thisNewRoot = thisData->d_ref.root;
							const size_t& fwdNewRoot  = fwdData->d_ref.root;

							const TreeAut* thisNewTA = thisFAE->getRoot(thisNewRoot).get();
							const TreeAut* fwdNewTA  = fwdFAE->getRoot(fwdNewRoot).get();
							assert((nullptr != thisNewTA) && (nullptr != fwdNewTA));

							RootState rootState = engine.makeProductState(
								thisNewRoot, thisNewTA->getFinalState(),
								fwdNewRoot, fwdNewTA->getFinalState());

							lhs.push_back(fae->addData(*fae->getRoot(curNewState.root).get(),
								Data::createRef(rootState.root)));
						}
						else if ((fwdIsData && !thisIsData && fwdData->isNull())
							|| (!fwdIsData && thisIsData && thisData->isNull())
							|| (fwdIsData && thisIsData && fwdData->isNull() && thisData->isRef())
							|| (fwdIsData && thisIsData && fwdData->isRef() && thisData->isNull()))
						{ // ************* process NULL pointers *************
							// This is the case when there is a NULL pointer and either an
							// internal state or a reference
							break;   // cut this branch of the intersection
						}
						else
						{	// we should not get here
							assert(false);
						}
					}

					if (transArity == i)
					{	// in case we have not interrupted the search, add the transition
						std::ostringstream osLhs;
						for (auto it = lhs.cbegin(); it != lhs.cend(); ++it)
						{
							if (lhs.cbegin() != it)
								osLhs << ",";

							osLhs << FA::writeState(*it);
						}

						FA_NOTE("TA " << curNewState.root << ": adding transition "
							<< FA::writeState(curNewState.state) << " -> "
							<< thisTrans.label() << "(" << osLhs.str() << ")");

						fae->getRoot(curNewState.root)->addTransition(
							lhs, thisTrans.label(), curNewState.state);
					}
				}
			}
		}
	}

	FA_NOTE("Result of intersection: " << *fae);

	// now, check whether there is some component with an empty language in the
	// result
	for (size_t i = 0; i < fae->getRootCount(); ++i)
	{
		TreeAut* ta = fae->allocTA();
		fae->getRoot(i)->uselessAndUnreachableFree(*ta);
		std::shared_ptr<TreeAut> pTa(ta);

		// check emptiness
		if (ta->getFinalStates().empty())
		{	// in case the language of an automaton is empty
			fae->clear();   // the language of the FA is empty
			return;
		}

		fae->setRoot(i, pTa);
	}

	// reorder the FAE to correspond to the original order
	std::vector<size_t> index = engine.getRootOrderIndexForRHS();
	std::vector<std::shared_ptr<TreeAut>> newRoots;

	for (size_t i : index)
	{
		newRoots.push_back(fae->getRoot(i));
	}

	// update representation
	fae->swapRoots(newRoots);

	for (size_t i = 0; i < fae->getRootCount(); ++i)
	{
		fae->setRoot(i, std::shared_ptr<TreeAut>(
			fae->relabelReferences(fae->getRoot(i).get(), index)
		));
	}

	fae->updateConnectionGraph();

	FA_WARN("Underapproximating intersection");
}
