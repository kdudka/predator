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

#include "forestautext.hh"
#include "ufae.hh"
#include "boxman.hh"

#include "fixpointinstruction.hh"

class FixpointBase : public FixpointInstruction {

protected:

	// configuration obtained in forward run
	TreeAut fwdConf;

	UFAE fwdConfWrapper;

	std::vector<std::shared_ptr<const FAE>> fixpoint;

	TreeAut::Backend& taBackend;

	BoxMan& boxMan;

public:

	virtual void extendFixpoint(const std::shared_ptr<const FAE>& fae) {
		this->fixpoint.push_back(fae);
	}

	virtual void clear() {

		this->fixpoint.clear();
		this->fwdConf.clear();
		this->fwdConfWrapper.clear();

	}

	void recompute() {
		this->fwdConf.clear();
		this->fwdConfWrapper.clear();
		TreeAut ta(*this->fwdConf.backend);
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

	FixpointBase(const CodeStorage::Insn* insn,
		TreeAut::Backend& fixpointBackend, TreeAut::Backend& taBackend,
		BoxMan& boxMan) :
		FixpointInstruction(insn), fwdConf(fixpointBackend),
		fwdConfWrapper(this->fwdConf, boxMan), fixpoint{}, taBackend(taBackend), boxMan(boxMan) {}

	virtual ~FixpointBase() {}

	virtual const TreeAut& getFixPoint() const {
		return this->fwdConf;
	}

};

class FI_abs : public FixpointBase {

public:

	FI_abs(const CodeStorage::Insn* insn,
		TreeAut::Backend& fixpointBackend, TreeAut::Backend& taBackend,
		BoxMan& boxMan) : FixpointBase(insn, fixpointBackend, taBackend, boxMan) {}

	virtual void execute(ExecutionManager& execMan, const ExecState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "abs   \t";
	}

};

class FI_fix : public FixpointBase {

public:

	FI_fix(const CodeStorage::Insn* insn,
		TreeAut::Backend& fixpointBackend, TreeAut::Backend& taBackend,
		BoxMan& boxMan)
		: FixpointBase(insn, fixpointBackend, taBackend, boxMan) {}

	virtual void execute(ExecutionManager& execMan, const ExecState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "fix   \t";
	}

};

#endif
