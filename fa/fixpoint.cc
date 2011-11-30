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
	bool operator()(const TT<label_type>& t1, const TT<label_type>& t2) {
		if (t1.label()->isNode() && t2.label()->isNode()) {
			if (t1.label()->getTag() != t2.label()->getTag())
				return false;
			std::vector<size_t> tmp;
			for (std::vector<size_t>::const_iterator i = t1.lhs().begin(); i != t1.lhs().end(); ++i) {
				if (FA::isData(*i))
					tmp.push_back(*i);
			}
			size_t i = 0;
			for (std::vector<size_t>::const_iterator j = t2.lhs().begin(); j != t2.lhs().end(); ++j) {
				if (FA::isData(*j)) {
					if ((i >= tmp.size()) || (*j != tmp[i++]))
						return false;
				}
			}
			return (i == tmp.size());
		}
		return t1.label() == t2.label();
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

inline void normalize(FAE& fae, const std::set<size_t>& forbidden, bool extended) {

	Normalization norm(fae);

	std::vector<size_t> order;
	std::vector<bool> marked;

	norm.scan(marked, order, forbidden, extended);
	norm.normalize(marked, order);

	CL_CDEBUG(3, "after normalization: " << std::endl << fae);

}

inline bool fold(FAE& fae, BoxMan& boxMan, const std::set<size_t>& forbidden, bool conditional) {

	std::vector<size_t> order;
	std::vector<bool> marked;

	Normalization(fae).scan(marked, order);

	Folding folding(fae, boxMan);

	bool matched = false;

	for (size_t i = 0; i < order.size(); ++i) {

		if (forbidden.count(order[i]))
			continue;

		assert(fae.roots[order[i]]);

		if (folding.discover(order[i], forbidden, conditional)) {

			fae.updateConnectionGraph();

			matched = true;

			continue;

		}

	}

	CL_CDEBUG(3, "after folding: " << std::endl << fae);

	return matched;

}

inline void learn(FAE& fae, BoxMan& boxMan) {

	fae.unreachableFree();
	fae.updateConnectionGraph();

	std::set<size_t> forbidden;

	computeForbiddenSet(forbidden, fae);

	fold(fae, boxMan, forbidden, false);

}

inline bool foldAndNormalize(FAE& fae, BoxMan& boxMan) {

	fae.unreachableFree();
	fae.updateConnectionGraph();

	std::set<size_t> forbidden;

	if (boxMan.getBoxes().empty()) {

		computeForbiddenSet(forbidden, fae);

		normalize(fae, forbidden, true);

		return false;

	}

	forbidden.insert(VirtualMachine(fae).varGet(ABP_INDEX).d_ref.root);

	bool matched = fold(fae, boxMan, forbidden, true);

	computeForbiddenSet(forbidden, fae);

	normalize(fae, forbidden, true);

	forbidden.clear();
	forbidden.insert(VirtualMachine(fae).varGet(ABP_INDEX).d_ref.root);

	return matched | fold(fae, boxMan, forbidden, true);

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

//	fae.fuse(target->fwdConf, FuseNonZeroF());

	CL_CDEBUG(3, "fused " << std::endl << fae);

	// abstract
//	CL_CDEBUG("abstracting ... " << 1);

	Abstraction abstraction(fae);

	for (size_t i = 1; i < fae.getRootCount(); ++i)
		abstraction.heightAbstraction(i, 1, SmartTMatchF());

	CL_CDEBUG(3, "after abstraction: " << std::endl << fae);

}

// FI_fix
void FI_abs::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

	learn(*fae, boxMan);

	foldAndNormalize(*fae, boxMan);

	do {

		abstract(*fae, this->fwdConf, this->taBackend, this->boxMan);

	} while (foldAndNormalize(*fae, boxMan));

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

	learn(*fae, boxMan);

	while (foldAndNormalize(*fae, boxMan)) {}

	// test inclusion
	if (testInclusion(*fae, this->fwdConf, this->fwdConfWrapper)) {

		CL_CDEBUG(3, "hit");

		execMan.traceFinished(state.second);

	} else {

		execMan.enqueue(state.second, state.first, fae, this->next_);

	}

}
