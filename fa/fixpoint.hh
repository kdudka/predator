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

#ifndef FIXPOINT_H
#define FIXPOINT_H

#include <vector>
#include <memory>

#include "treeaut.hh"
#include "forestautext.hh"
#include "ufae.hh"
#include "boxman.hh"

#include "fixpointinstruction.hh"

class FI_abs : public FixpointInstruction {

	// configuration obtained in forward run
	TA<label_type> fwdConf;

	UFAE fwdConfWrapper;

	std::vector<std::shared_ptr<const FAE>> fixpoint;

	TA<label_type>::Backend& taBackend;

	BoxMan& boxMan;

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

public:

	FI_abs(TA<label_type>::Backend& fixpointBackend, TA<label_type>::Backend& taBackend,
		BoxMan& boxMan)
		: FixpointInstruction(), fwdConf(fixpointBackend), fwdConfWrapper(this->fwdConf, boxMan),
		taBackend(taBackend), boxMan(boxMan) {}

	virtual ~FI_abs() {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual const TA<label_type>& getFixPoint() const {
		return this->fwdConf;
	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "abs   ";
	}

};

class FI_fix : public FixpointInstruction {

	// configuration obtained in forward run
	TA<label_type> fwdConf;

	UFAE fwdConfWrapper;

	std::vector<std::shared_ptr<const FAE>> fixpoint;

	TA<label_type>::Backend& taBackend;

	BoxMan& boxMan;

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

public:

	FI_fix(TA<label_type>::Backend& fixpointBackend, TA<label_type>::Backend& taBackend,
		BoxMan& boxMan)
		: FixpointInstruction(), fwdConf(fixpointBackend), fwdConfWrapper(this->fwdConf, boxMan),
		taBackend(taBackend), boxMan(boxMan) {}

	virtual ~FI_fix() {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual const TA<label_type>& getFixPoint() const {
		return this->fwdConf;
	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "fix   ";
	}

};

#endif
