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

#ifndef FIXPOINT_H
#define FIXPOINT_H

#include <vector>
#include <memory>

#include "treeaut.hh"
#include "forestautext.hh"
#include "ufae.hh"
#include "boxman.hh"

#include "fixpointinstruction.hh"

class FI_fix : public FixpointInstruction {

	// configuration obtained in forward run
	TA<label_type> fwdConf;

	UFAE fwdConfWrapper;

	std::vector<std::shared_ptr<const FAE>> fixpoint;

	TA<label_type>::Backend& taBackend;

	BoxMan& boxMan;
	
	const std::vector<const Box*>& boxes;

public:

	virtual void extendFixpoint(const std::shared_ptr<const FAE>& fae) {
		this->fixpoint.push_back(fae);
	}

	void recompute() {
		this->fwdConfWrapper.clear();
		this->fwdConf.clear();
		TA<label_type> ta(*this->fwdConf.backend);
		Index<size_t> index;

		for (auto fae : this->fixpoint)
			this->fwdConfWrapper.fae2ta(ta, index, *fae);

		if (!ta.getTransitions().empty()) {
			this->fwdConfWrapper.adjust(index);
			ta.minimized(this->fwdConf);
		}		
//		this->fwdConfWrapper.setStateOffset(this->fixpointWrapper.getStateOffset());
//		this->fwdConf = this->fixpoint;
	}
/*
	bool testInclusion(FAE& fae) {

		TA<label_type> ta(*this->fwdConf.backend);
		Index<size_t> index;

		this->fwdConfWrapper.fae2ta(ta, index, fae);

//		CL_CDEBUG("challenge" << std::endl << ta);
//		CL_CDEBUG("response" << std::endl << this->fwdConf);

		if (TA<label_type>::subseteq(ta, this->fwdConf))
			return true;

//		CL_CDEBUG("extending fixpoint with:" << std::endl << fae);

		this->fwdConfWrapper.join(ta, index);
		ta.clear();
		this->fwdConf.minimized(ta);
		this->fwdConf = ta;

		return false;

	}
*/
/*
	void mergeFixpoint(FAE& fae) {
		std::vector<FAE*> tmp;
		ContainerGuard<std::vector<FAE*> > g(tmp);
		FAE::loadCompatibleFAs(tmp, this->fwdConf, this->taBackend, this->boxMan, &fae, 0, CompareVariablesF());
//		for (size_t i = 0; i < tmp.size(); ++i)
//			CL_CDEBUG("accelerator " << std::endl << *tmp[i]);
		fae.fuse(tmp, FuseNonZeroF());
//		fae.fuse(target->fwdConf, FuseNonZeroF());
		CL_CDEBUG("fused " << std::endl << fae);
	}
*/
public:

	FI_fix(TA<label_type>::Backend& fixpointBackend, TA<label_type>::Backend& taBackend,
		BoxMan& boxMan, const std::vector<const Box*>& boxes)
		: FixpointInstruction(), fwdConf(fixpointBackend), fwdConfWrapper(this->fwdConf, boxMan),
		taBackend(taBackend), boxMan(boxMan), boxes(boxes) {}

	virtual ~FI_fix() {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual const TA<label_type>& getFixPoint() const {
		return this->fwdConf;
	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "fix";
	}

};

#endif
