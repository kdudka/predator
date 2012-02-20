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

#include <ostream>

#include <cl/storage.hh>
#include <cl/cl_msg.hh>
#include "../cl/ssd.h"

#include "treeaut.hh"
#include "forestautext.hh"
#include "ufae.hh"
#include "executionmanager.hh"
#include "virtualmachine.hh"
#include "folding.hh"
#include "abstraction.hh"
#include "normalization.hh"
#include "splitting.hh"
#include "utils.hh"
#include "regdef.hh"

#include "fixpoint.hh"

using namespace ssd;

struct ExactTMatchF {
	bool operator()(const TT<label_type>& t1, const TT<label_type>& t2) {
		return t1.label() == t2.label();
	}
};

struct SmartTMatchF {
	bool operator()(const TT<label_type>& t1, const TT<label_type>& t2) {
		if (t1.label()->isNode() && t2.label()->isNode())
			return t1.label()->getTag() == t2.label()->getTag();
		return t1.label() == t2.label();
	}
};

struct SmarterTMatchF {

	const FAE& fae;

	SmarterTMatchF(const FAE& fae) : fae(fae) {}

	bool operator()(const TT<label_type>& t1, const TT<label_type>& t2) {

		if (!t1.label()->isNode() || !t2.label()->isNode())
			return t1.label() == t2.label();

		if (t1.label()->getTag() != t2.label()->getTag())
			return false;

		if (&t1.lhs() == &t2.lhs())
			return true;

		if (t1.lhs().size() != t2.lhs().size())
			return false;

		for (size_t i = 0; i < t1.lhs().size(); ++i) {

			size_t s1 = t1.lhs()[i], s2 = t2.lhs()[i], ref;

			if (s1 == s2)
				continue;

			if (FA::isData(s1)) {

				if (!this->fae.getRef(s1, ref))
					return false;

				if (FA::isData(s2))
					return false;

			} else {

				if (FA::isData(s2) && !this->fae.getRef(s2, ref))
					return false;

			}

		}

		return true;

	}
};

struct CompareVariablesF {
	bool operator()(size_t i, const TA<label_type>& ta1, const TA<label_type>& ta2) {
		if (i)
			return true;
		const TT<label_type>& t1 = ta1.getAcceptingTransition();
		const TT<label_type>& t2 = ta2.getAcceptingTransition();
		return (t1.label() == t2.label()) && (t1.lhs() == t2.lhs());
	}
};

struct FuseNonZeroF {
	bool operator()(size_t root, const FAE*) {
		return root != 0;
	}
};

inline void computeForbiddenSet(std::set<size_t>& forbidden, FAE& fae) {

	assert(fae.roots.size() == fae.connectionGraph.data.size());

	VirtualMachine vm(fae);

	assert(fae.roots[vm.varGet(ABP_INDEX).d_ref.root]);

	// do not touch ABP
	forbidden.insert(vm.varGet(ABP_INDEX).d_ref.root);

	vm.getNearbyReferences(vm.varGet(ABP_INDEX).d_ref.root, forbidden);

/*
	for (size_t i = 0; i < fae.roots.size(); ++i) {

		if (!fae.roots[i])
			continue;

		if (!fae.connectionGraph.data[i].clean())
			forbidden.insert(i);
		else {

			CL_CDEBUG(3, "stationary root: " << i);

		}

	}
*/
}

inline bool normalize(FAE& fae, const std::set<size_t>& forbidden, bool extended) {

	Normalization norm(fae);

	std::vector<size_t> order;
	std::vector<bool> marked;

	norm.scan(marked, order, forbidden, extended);

	bool result = norm.normalize(marked, order);

	CL_CDEBUG(3, "after normalization: " << std::endl << fae);

	return result;

}

inline bool fold(FAE& fae, BoxMan& boxMan, const std::set<size_t>& forbidden) {

	std::vector<size_t> order;
	std::vector<bool> marked;

	Folding folding(fae, boxMan);

	bool matched = false;

	for (size_t i = 0; i < fae.roots.size(); ++i) {

		if (forbidden.count(i))
			continue;

		assert(fae.roots[i]);

		if (folding.discover1(i, forbidden, true))
			matched = true;

		if (folding.discover2(i, forbidden, true))
			matched = true;

		if (folding.discover3(i, forbidden, true))
			matched = true;

	}

	if (matched) {
		CL_CDEBUG(3, "after folding: " << std::endl << fae);
	}

	return matched;

}

inline void reorder(FAE& fae) {

	fae.unreachableFree();

	Normalization norm(fae);

	std::vector<size_t> order;
	std::vector<bool> marked;

	norm.scan(marked, order, std::set<size_t>());

	std::fill(marked.begin(), marked.end(), true);

	norm.normalize(marked, order);

	CL_CDEBUG(3, "after reordering: " << std::endl << fae);

}

inline bool testInclusion(FAE& fae, TA<label_type>& fwdConf, UFAE& fwdConfWrapper) {

	TA<label_type> ta(*fwdConf.backend);

	Index<size_t> index;

	fae.unreachableFree();

	fwdConfWrapper.fae2ta(ta, index, fae);

//	CL_CDEBUG(3, "challenge:" << std::endl << ta);
//	CL_CDEBUG(3, "response:" << std::endl << fwdConf);

	if (TA<label_type>::subseteq(ta, fwdConf))
		return true;

	CL_CDEBUG(1, "extending fixpoint with:" << std::endl << fae);

	fwdConfWrapper.join(ta, index);

	ta.clear();

	fwdConf.minimized(ta);
	fwdConf = ta;

	return false;

}

struct CopyNonZeroRhsF {
	bool operator()(const TT<label_type>* transition) const {

		return transition->rhs() != 0;

	}
};

inline void abstract(FAE& fae, TA<label_type>& fwdConf, TA<label_type>::Backend& backend, BoxMan& boxMan) {

	fae.unreachableFree();

//	CL_CDEBUG(1, SSD_INLINE_COLOR(C_LIGHT_GREEN, "after normalization:" ) << std::endl << *fae);

	// merge fixpoint
	std::vector<FAE*> tmp;

	ContainerGuard<std::vector<FAE*> > g(tmp);

	FAE::loadCompatibleFAs(
		tmp, fwdConf, backend, boxMan, &fae, 0, CompareVariablesF()
	);

	for (size_t i = 0; i < tmp.size(); ++i)
		CL_CDEBUG(3, "accelerator " << std::endl << *tmp[i]);

	fae.fuse(tmp, FuseNonZeroF());

//	fae.fuse(fwdConf, FuseNonZeroF(), CopyNonZeroRhsF());

	CL_CDEBUG(3, "fused " << std::endl << fae);

	// abstract
//	CL_CDEBUG("abstracting ... " << 1);

	Abstraction abstraction(fae);

	for (size_t i = 1; i < fae.getRootCount(); ++i)
//		abstraction.heightAbstraction(i, 1, SmarterTMatchF(fae));
		abstraction.heightAbstraction(i, 1, SmartTMatchF());

	CL_CDEBUG(3, "after abstraction: " << std::endl << fae);

}

inline void getCandidates(std::set<size_t>& candidates, const FAE& fae) {

	std::unordered_map<
		std::vector<std::pair<int, size_t>>,
		std::set<size_t>,
		boost::hash<std::vector<std::pair<int, size_t>>>
	> partition;

	for (size_t i = 0; i < fae.roots.size(); ++i) {

		std::vector<std::pair<int, size_t>> tmp;

		fae.connectionGraph.getRelativeSignature(tmp, i);

		partition.insert(std::make_pair(tmp, std::set<size_t>())).first->second.insert(i);

	}

	candidates.clear();

	for (auto& tmp : partition) {

		if (tmp.second.size() > 1)
			candidates.insert(tmp.second.begin(), tmp.second.end());

	}

}

inline void learn1(FAE& fae, BoxMan& boxMan) {

	fae.unreachableFree();

	std::set<size_t> forbidden;

	Folding folding(fae, boxMan);

	computeForbiddenSet(forbidden, fae);
//	forbidden.insert(VirtualMachine(fae).varGet(ABP_INDEX).d_ref.root);
/*
	std::set<size_t> candidates;

	getCandidates(candidates, fae);

	for (auto& candidate : candidates) {
*/
	for (size_t i = 0; i < fae.roots.size(); ++i) {

		if (forbidden.count(i))
			continue;

		assert(fae.roots[i]);

		folding.discover1(i, forbidden, false);
		folding.discover2(i, forbidden, false);

	}

}

inline void learn2(FAE& fae, BoxMan& boxMan) {

	fae.unreachableFree();

	std::set<size_t> forbidden;

	Folding folding(fae, boxMan);

	computeForbiddenSet(forbidden, fae);
//	forbidden.insert(VirtualMachine(fae).varGet(ABP_INDEX).d_ref.root);
/*
	std::set<size_t> candidates;

	getCandidates(candidates, fae);

	for (auto& candidate : candidates) {
*/
	for (size_t i = 0; i < fae.roots.size(); ++i) {

		if (forbidden.count(i))
			continue;

		assert(fae.roots[i]);

		folding.discover3(i, forbidden, false);

	}

}

// FI_fix
void FI_abs::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

	fae->updateConnectionGraph();

	reorder(*fae);

	std::set<size_t> forbidden;

	if (boxMan.getBoxes().size()) {

		forbidden.insert(VirtualMachine(*fae).varGet(ABP_INDEX).d_ref.root);

		fold(*fae, this->boxMan, forbidden);

	}

	learn2(*fae, this->boxMan);

	forbidden.clear();

	computeForbiddenSet(forbidden, *fae);

	normalize(*fae, forbidden, true);

	abstract(*fae, this->fwdConf, this->taBackend, this->boxMan);

	learn1(*fae, this->boxMan);

	if (boxMan.getBoxes().size()) {

		FAE old(*fae->backend, this->boxMan);

		do {

			forbidden.clear();
			computeForbiddenSet(forbidden, *fae);

			normalize(*fae, forbidden, true);

			abstract(*fae, this->fwdConf, this->taBackend, this->boxMan);

			forbidden.clear();
			forbidden.insert(VirtualMachine(*fae).varGet(ABP_INDEX).d_ref.root);

			old = *fae;

		} while (fold(*fae, this->boxMan, forbidden) && !FAE::subseteq(*fae, old));

	}

	// test inclusion
	if (testInclusion(*fae, this->fwdConf, this->fwdConfWrapper)) {

		CL_CDEBUG(3, "hit");

		execMan.traceFinished(state.second);

	} else {

		execMan.enqueue(state.second, state.first, fae, this->next_);

	}

}

// FI_fix
void FI_fix::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

	fae->updateConnectionGraph();

	reorder(*fae);

	std::set<size_t> forbidden;

	if (boxMan.getBoxes().size()) {

		forbidden.insert(VirtualMachine(*fae).varGet(ABP_INDEX).d_ref.root);

		fold(*fae, this->boxMan, forbidden);

	}

	forbidden.clear();

	computeForbiddenSet(forbidden, *fae);

	normalize(*fae, forbidden, true);

	if (boxMan.getBoxes().size()) {

		forbidden.clear();

		forbidden.insert(VirtualMachine(*fae).varGet(ABP_INDEX).d_ref.root);

		while (fold(*fae, this->boxMan, forbidden)) {

			forbidden.clear();

			computeForbiddenSet(forbidden, *fae);

			normalize(*fae, forbidden, true);

			forbidden.clear();

			forbidden.insert(VirtualMachine(*fae).varGet(ABP_INDEX).d_ref.root);

		}

	}

	// test inclusion
	if (testInclusion(*fae, this->fwdConf, this->fwdConfWrapper)) {

		CL_CDEBUG(3, "hit");

		execMan.traceFinished(state.second);

	} else {

		execMan.enqueue(state.second, state.first, fae, this->next_);

	}

}
