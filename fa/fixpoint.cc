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

// Standard library headers
#include <ostream>

// Code Listener headers
#include <cl/storage.hh>

// Forester headers
#include "abstraction.hh"
#include "config.h"
#include "executionmanager.hh"
#include "fixpoint.hh"
#include "folding.hh"
#include "forestautext.hh"
#include "normalization.hh"
#include "regdef.hh"
#include "splitting.hh"
#include "streams.hh"
#include "ufae.hh"
#include "utils.hh"
#include "virtualmachine.hh"

// anonymous namespace
namespace
{
struct ExactTMatchF
{
	bool operator()(
		const TT<label_type>&              t1,
		const TT<label_type>&              t2)
	{
		return t1.label() == t2.label();
	}
};

struct SmartTMatchF
{
	bool operator()(
		const TT<label_type>&              t1,
		const TT<label_type>&              t2)
	{
		if (t1.label()->isNode() && t2.label()->isNode())
		{
			return t1.label()->getTag() == t2.label()->getTag();
		}

		return t1.label() == t2.label();
	}
};

class SmarterTMatchF
{
private:  // data members

	const FAE& fae_;

public:   // methods

	SmarterTMatchF(const FAE& fae) :
		fae_(fae)
	{ }

	bool operator()(
		const TT<label_type>&              t1,
		const TT<label_type>&              t2)
	{
		if (!t1.label()->isNode() || !t2.label()->isNode())
			return t1.label() == t2.label();

		if (t1.label()->getTag() != t2.label()->getTag())
			return false;

		if (&t1.lhs() == &t2.lhs())
			return true;

		if (t1.lhs().size() != t2.lhs().size())
			return false;

		for (size_t i = 0; i < t1.lhs().size(); ++i)
		{
			size_t s1 = t1.lhs()[i], s2 = t2.lhs()[i], ref;

			if (s1 == s2)
				continue;

			if (FA::isData(s1))
			{
				if (!this->fae_.getRef(s1, ref))
					return false;

				if (FA::isData(s2))
					return false;
			} else
			{
				if (FA::isData(s2) && !fae_.getRef(s2, ref))
					return false;
			}
		}

		return true;
	}
};

struct CompareVariablesF
{
	bool operator()(
		const FAE&            fae,
		size_t                i,
		const TreeAut&        ta1,
		const TreeAut&        ta2)
	{
		VirtualMachine vm(fae);

		bool isFixedComp = true;
		for (size_t j = 0; j < FIXED_REG_COUNT; ++j)
		{
			if (i == vm.varGet(j).d_ref.root)
			{
				isFixedComp = false;
				break;
			}
		}

		if (isFixedComp)
		{
			return true;
		}

		const TT<label_type>& t1 = ta1.getAcceptingTransition();
		const TT<label_type>& t2 = ta2.getAcceptingTransition();
		return (t1.label() == t2.label()) && (t1.lhs() == t2.lhs());
	}
};

struct FuseNonFixedF
{
	bool operator()(size_t root, const FAE* fae)
	{
		// Preconditions
		assert(nullptr != fae);

		VirtualMachine vm(*fae);

		for (size_t i = 0; i < FIXED_REG_COUNT; ++i)
		{
			if (root == vm.varGet(i).d_ref.root)
			{
				return false;
			}
		}

		return true;
	}
};


bool normalize(
	FAE&                      fae,
	const SymState*           state,
	const std::set<size_t>&   forbidden,
	bool                      extended)
{
	Normalization norm(fae, state);

	std::vector<size_t> order;
	std::vector<bool> marked;

	norm.scan(marked, order, forbidden, extended);

	bool result = norm.normalize(marked, order);

	FA_DEBUG_AT(3, "after normalization: " << std::endl << fae);

	return result;
}


/**
 * @brief  Folds a FA without learning
 *
 * This function folds a FA @p fae using boxes in the box manager @p boxMan, but
 * avoiding the folding of cutpoints from @p forbidden. Note than no new boxes
 * are learnt, only boxes already in @p boxMan are applied.
 *
 * @param[in]  fae        The forest automaton to be folded
 * @param[in]  boxMan     The database of boxes
 * @param[in]  forbidden  The set of cutpoints not allowed for folding
 *
 * @returns  @p true in the case something has been folded, @p false otherwise
 */
bool fold(
	FAE&                         fae,
	BoxMan&                      boxMan,
	const std::set<size_t>&      forbidden)
{
	std::vector<size_t> order;
	std::vector<bool> marked;

	Folding folding(fae, boxMan);

	bool matched = false;

	for (size_t i = 0; i < fae.getRootCount(); ++i)
	{
		if (forbidden.end() != forbidden.find(i))
		{	// in the case the cutpoint is not allowed for folding
			continue;
		}

		assert(nullptr != fae.getRoot(i));

		// Try to fold the 3 types of cutpoints starting from cutpoint 'i', but
		// _ONLY_ using boxes which are _ALREADY_ in 'boxMan'. No learning of new
		// boxes is allowed

		if (folding.discover1(i, forbidden, true))
		{
			matched = true;
		}

		if (folding.discover2(i, forbidden, true))
		{
			matched = true;
		}

		if (folding.discover3(i, forbidden, true))
		{
			matched = true;
		}
	}

	if (matched)
	{
		FA_DEBUG_AT(3, "after folding: " << std::endl << fae);
	}

	return matched;
}


/**
 * @brief  Reorders components into the canonical order
 *
 * This function reorders the components of the FA @p fae @e without @e merging
 * them together.
 *
 * @param[in]      state  The state of the symbolic execution
 * @param[in,out]  fae    The forest automaton to be reordered
 */
void reorder(
	const SymState*   state,
	FAE&              fae)
{
	fae.unreachableFree();

	Normalization norm(fae, state);

	std::vector<size_t> order;
	std::vector<bool> marked;

	norm.scan(marked, order, std::set<size_t>());

	// normalize without merging (we say that all components are referred more
	// than once), i.e. only reorder
	std::fill(marked.begin(), marked.end(), true);
	norm.normalize(marked, order);

	FA_DEBUG_AT(3, "after reordering: " << std::endl << fae);
}

bool testInclusion(
	FAE&                           fae,
	TreeAut&                       fwdConf,
	UFAE&                          fwdConfWrapper)
{
	TreeAut ta(*fwdConf.backend);

	Index<size_t> index;

	fae.unreachableFree();

	fwdConfWrapper.fae2ta(ta, index, fae);

	if (TreeAut::subseteq(ta, fwdConf))
		return true;

	fwdConfWrapper.join(ta, index);

	ta.clear();

	fwdConf.minimized(ta);
	fwdConf = ta;

	return false;
}

struct CopyNonZeroRhsF
{
	bool operator()(const TT<label_type>* transition) const
	{
		return transition->rhs() != 0;
	}
};


void getCandidates(
	std::set<size_t>&               candidates,
	const FAE&                      fae)
{
	std::unordered_map<
		std::vector<std::pair<int, size_t>>,
		std::set<size_t>,
		boost::hash<std::vector<std::pair<int, size_t>>>
	> partition;

	for (size_t i = 0; i < fae.getRootCount(); ++i)
	{
		std::vector<std::pair<int, size_t>> tmp;

		fae.connectionGraph.getRelativeSignature(tmp, i);

		partition.insert(std::make_pair(tmp, std::set<size_t>())).first->second.insert(i);
	}

	candidates.clear();

	for (auto& tmp : partition)
	{
		if (tmp.second.size() > 1)
			candidates.insert(tmp.second.begin(), tmp.second.end());
	}
}

void learn1(FAE& fae, BoxMan& boxMan)
{
	fae.unreachableFree();

	std::set<size_t> forbidden = Normalization::computeForbiddenSet(fae);

	Folding folding(fae, boxMan);

	for (size_t i = 0; i < fae.getRootCount(); ++i)
	{
		if (forbidden.count(i))
			continue;

		assert(fae.getRoot(i));

		folding.discover1(i, forbidden, false);
		folding.discover2(i, forbidden, false);
	}
}

void learn2(
	FAE&       fae,
	BoxMan&    boxMan)
{
	fae.unreachableFree();

	std::set<size_t> forbidden = Normalization::computeForbiddenSet(fae);

	Folding folding(fae, boxMan);

	for (size_t i = 0; i < fae.getRootCount(); ++i)
	{
		if (forbidden.count(i))
			continue;

		assert(fae.getRoot(i));

		folding.discover3(i, forbidden, false);
	}
}
} // namespace


SymState* FixpointBase::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)fwdPred;

	SymState* tmpState = execMan.copyStateWithNewRegs(bwdSucc, fwdPred.GetInstr());

	// perform intersection
	tmpState->Intersect(fwdPred);

	FA_WARN("Executing !!VERY!! suspicious reverse operation FixpointBase");
	return tmpState;
}


void FI_abs::abstract(
	FAE&                 fae)
{
	fae.unreachableFree();

	FA_DEBUG_AT(3, "before abstraction: " << std::endl << fae);

	if (FA_FUSION_ENABLED)
	{
		// merge fixpoint
		std::vector<FAE*> tmp;

		ContainerGuard<std::vector<FAE*>> g(tmp);

		FAE::loadCompatibleFAs(
			/* the result */ tmp,
			fwdConf_,
			taBackend_,
			boxMan_,
			fae,
			0,
			CompareVariablesF()
		);

		for (size_t i = 0; i < tmp.size(); ++i)
		{
			FA_DEBUG_AT(3, "accelerator " << std::endl << *tmp[i]);
		}

		fae.fuse(tmp, FuseNonFixedF());
		FA_DEBUG_AT(3, "fused " << std::endl << fae);
	}

	// abstract
	Abstraction abstraction(fae);

	if (FA_USE_PREDICATE_ABSTRACTION)
	{	// for predicate abstraction
		abstraction.predicateAbstraction(this->getPredicates());
	}
	else
	{	// for finite height abstraction

		// the roots that will be excluded from abstraction
		std::vector<bool> excludedRoots(fae.getRootCount(), false);
		for (size_t i = 0; i < FIXED_REG_COUNT; ++i)
		{
			excludedRoots[VirtualMachine(fae).varGet(i).d_ref.root] = true;
		}

		for (size_t i = 0; i < fae.getRootCount(); ++i)
		{
			if (!excludedRoots[i])
			{
				abstraction.heightAbstraction(i, FA_ABS_HEIGHT, SmartTMatchF());
//				abstraction.heightAbstraction(i, FA_ABS_HEIGHT, SmarterTMatchF(fae));
			}
		}
	}

	FA_DEBUG_AT(3, "after abstraction: " << std::endl << fae);
}


// FI_abs
void FI_abs::execute(ExecutionManager& execMan, SymState& state)
{
	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(state.GetFAE())));

	fae->updateConnectionGraph();

	std::set<size_t> forbidden;
#if FA_ALLOW_FOLDING
	// reorder components into the canonical form (no merging!)
	reorder(&state, *fae);

	if (!boxMan_.boxDatabase().empty())
	{	// in the case there are some boxes, try to fold immediately before
		// normalization
		for (size_t i = 0; i < FIXED_REG_COUNT; ++i)
		{
			forbidden.insert(VirtualMachine(*fae).varGet(i).d_ref.root);
		}

		// fold already discovered boxes
		fold(*fae, boxMan_, forbidden);
	}

	learn2(*fae, boxMan_);
#endif
	forbidden = Normalization::computeForbiddenSet(*fae);

	normalize(*fae, &state, forbidden, true);

	abstract(*fae);
#if FA_ALLOW_FOLDING
	learn1(*fae, boxMan_);

	if (boxMan_.boxDatabase().size())
	{
		FAE old(*fae->backend, boxMan_);

		do
		{
			forbidden = Normalization::computeForbiddenSet(*fae);

			normalize(*fae, &state, forbidden, true);

			abstract(*fae);

			forbidden.clear();
			for (size_t i = 0; i < FIXED_REG_COUNT; ++i)
			{
				forbidden.insert(VirtualMachine(*fae).varGet(i).d_ref.root);
			}

			old = *fae;

		} while (fold(*fae, boxMan_, forbidden) && !FAE::subseteq(*fae, old));

	}
#endif
	// test inclusion
	if (testInclusion(*fae, fwdConf_, fwdConfWrapper_))
	{
		FA_DEBUG_AT(3, "hit");

		execMan.pathFinished(&state);
	} else
	{
		FA_DEBUG_AT_MSG(1, &this->insn()->loc, "extending fixpoint\n" << *fae);

		SymState* tmpState = execMan.createChildState(state, next_);
		tmpState->SetFAE(fae);

		execMan.enqueue(tmpState);
	}
}

// FI_fix
void FI_fix::execute(ExecutionManager& execMan, SymState& state)
{
	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(state.GetFAE())));

	fae->updateConnectionGraph();

	std::set<size_t> forbidden;
#if FA_ALLOW_FOLDING
	reorder(&state, *fae);

	if (!boxMan_.boxDatabase().size())
	{
		for (size_t i = 0; i < FIXED_REG_COUNT; ++i)
		{
			forbidden.insert(VirtualMachine(*fae).varGet(i).d_ref.root);
		}

		fold(*fae, boxMan_, forbidden);
	}
#endif
	forbidden = Normalization::computeForbiddenSet(*fae);

	normalize(*fae, &state, forbidden, true);
#if FA_ALLOW_FOLDING
	if (boxMan_.boxDatabase().size())
	{
		forbidden.clear();

		for (size_t i = 0; i < FIXED_REG_COUNT; ++i)
		{
			forbidden.insert(VirtualMachine(*fae).varGet(i).d_ref.root);
		}

		while (fold(*fae, boxMan_, forbidden))
		{
			forbidden = Normalization::computeForbiddenSet(*fae);

			normalize(*fae, &state, forbidden, true);

			forbidden.clear();

			for (size_t i = 0; i < FIXED_REG_COUNT; ++i)
			{
				forbidden.insert(VirtualMachine(*fae).varGet(i).d_ref.root);
			}
		}
	}
#endif
	// test inclusion
	if (testInclusion(*fae, fwdConf_, fwdConfWrapper_))
	{
		FA_DEBUG_AT(3, "hit");

		execMan.pathFinished(&state);
	} else
	{
		FA_DEBUG_AT_MSG(1, &this->insn()->loc, "extending fixpoint\n" << *fae);

		SymState* tmpState = execMan.createChildState(state, next_);
		tmpState->SetFAE(fae);

		execMan.enqueue(tmpState);
	}
}
