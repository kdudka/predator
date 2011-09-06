/*
 * Copyright (C) 2011 Jiri Simacek
 *
 * This file is part of predator.
 *
 * predator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * predator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with predator.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ostream>

#include <cl/storage.hh>
#include <cl/cl_msg.hh>

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

// FI_fix
void FI_abs::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

	// normalize
	std::set<size_t> tmp;

	VirtualMachine vm(*fae);

	const Data& abp = vm.varGet(ABP_INDEX);

	vm.getNearbyReferences(abp.d_ref.root, tmp);

	Normalization norm(*fae);

	norm.normalize(tmp);

	bool matched = false;

	// fold
	Folding folding(*fae);

	// do not fold at 0
	for (size_t i = 1; i < fae->getRootCount(); ++i) {

		// we are normalized so we don't have to check whether root is not null
		for (auto box : this->boxes) {

			CL_CDEBUG(3, "trying " << *(const AbstractBox*)box << " at " << i);

			if (folding.foldBox(i, box)) {

				matched = true;
				CL_CDEBUG(3, "match");

			}

		}

	}

	if (matched) {

		tmp.clear();
		vm.getNearbyReferences(abp.d_ref.root, tmp);
		norm.normalize(tmp);

	}

	fae->unreachableFree();

	// merge fixpoint
	std::vector<FAE*> tmp2;

	ContainerGuard<std::vector<FAE*> > g(tmp2);

	FAE::loadCompatibleFAs(tmp2, this->fwdConf, this->taBackend, this->boxMan, fae.get(), 0, CompareVariablesF());

//	for (size_t i = 0; i < tmp.size(); ++i)
//		CL_CDEBUG("accelerator " << std::endl << *tmp[i]);
	fae->fuse(tmp2, FuseNonZeroF());
//	fae.fuse(target->fwdConf, FuseNonZeroF());

//	CL_CDEBUG("fused " << std::endl << *fae);

	// abstract
	Abstraction abstraction(*fae);

//	CL_CDEBUG("abstracting ... " << 1);
	for (size_t i = 1; i < fae->getRootCount(); ++i)
		abstraction.heightAbstraction(i, 1, SmartTMatchF());

	// test inclusion
	fae->unreachableFree();

	TA<label_type> ta(*this->fwdConf.backend);
	Index<size_t> index;

	this->fwdConfWrapper.fae2ta(ta, index, *fae);

//	CL_CDEBUG("challenge" << std::endl << ta);
//	CL_CDEBUG("response" << std::endl << this->fwdConf);

	if (TA<label_type>::subseteq(ta, this->fwdConf)) {

		CL_CDEBUG(3, "hit");
		execMan.traceFinished(state.second);
		return;

	}

//	CL_CDEBUG("extending fixpoint with:" << std::endl << fae);

	this->fwdConfWrapper.join(ta, index);
	ta.clear();
	this->fwdConf.minimized(ta);
	this->fwdConf = ta;

	CL_DEBUG_AT(2, "fixpoint at " << this->insn()->loc << std::endl << ta);

	execMan.enqueue(state.second, state.first, fae, this->next_);

}

// FI_fix
void FI_fix::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

	// normalize
	std::set<size_t> tmp;

	VirtualMachine vm(*fae);

	const Data& abp = vm.varGet(ABP_INDEX);

	vm.getNearbyReferences(abp.d_ref.root, tmp);

	Normalization norm(*fae);

	norm.normalize(tmp);
/*
	bool matched = false;

	// fold
	Folding folding(*fae);

	// do not fold at 0
	for (size_t i = 1; i < fae->getRootCount(); ++i) {

		// we are normalized so we don't have to check whether root is not null
		for (auto box : this->boxes) {

			CL_CDEBUG(3, "trying " << *(const AbstractBox*)box << " at " << i);

			if (folding.foldBox(i, box)) {

				matched = true;
				CL_CDEBUG(3, "match");

			}

		}

	}

	if (matched) {

		tmp.clear();
		vm.getNearbyReferences(abp.d_ref.root, tmp);
		norm.normalize(tmp);

	}

	fae->unreachableFree();

	// merge fixpoint
	std::vector<FAE*> tmp2;

	ContainerGuard<std::vector<FAE*> > g(tmp2);

	FAE::loadCompatibleFAs(tmp2, this->fwdConf, this->taBackend, this->boxMan, fae.get(), 0, CompareVariablesF());

//	for (size_t i = 0; i < tmp.size(); ++i)
//		CL_CDEBUG("accelerator " << std::endl << *tmp[i]);
	fae->fuse(tmp2, FuseNonZeroF());
//	fae.fuse(target->fwdConf, FuseNonZeroF());

//	CL_CDEBUG("fused " << std::endl << *fae);

	// abstract
	Abstraction abstraction(*fae);

//	CL_CDEBUG("abstracting ... " << 1);
	for (size_t i = 1; i < fae->getRootCount(); ++i)
		abstraction.heightAbstraction(i, 1, SmartTMatchF());
*/
	// test inclusion
	fae->unreachableFree();

	TA<label_type> ta(*this->fwdConf.backend);
	Index<size_t> index;

	this->fwdConfWrapper.fae2ta(ta, index, *fae);

//	CL_CDEBUG("challenge" << std::endl << ta);
//	CL_CDEBUG("response" << std::endl << this->fwdConf);

	if (TA<label_type>::subseteq(ta, this->fwdConf)) {

		CL_CDEBUG(3, "hit");
		execMan.traceFinished(state.second);
		return;

	}

//	CL_CDEBUG("extending fixpoint with:" << std::endl << fae);

	this->fwdConfWrapper.join(ta, index);
	ta.clear();
	this->fwdConf.minimized(ta);
	this->fwdConf = ta;

	CL_DEBUG_AT(2, "fixpoint at " << this->insn()->loc << std::endl << ta);

	execMan.enqueue(state.second, state.first, fae, this->next_);

}
