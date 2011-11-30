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

#ifndef ABSTRACT_INSTRUCTION_H
#define ABSTRACT_INSTRUCTION_H

#include <ostream>
#include <vector>
#include <unordered_map>
#include <memory>

#include "types.hh"

namespace CodeStorage {
    struct Fnc;
    struct Storage;
    struct Block;
    struct Insn;
}

class ExecutionManager;

typedef enum { fiAbort, fiBranch, fiCheck, fiFix, fiJump, fiUnspec } fi_type_e;

class AbstractInstruction {

public:

	typedef std::pair<std::shared_ptr<std::vector<Data>>, struct SymState*> StateType;

private:

	fi_type_e fiType_;
	bool isTarget_;

	const CodeStorage::Insn* insn_;

public:

	AbstractInstruction(fi_type_e fiType = fi_type_e::fiUnspec)
		: fiType_(fiType), isTarget_(false), insn_(NULL) {}

	virtual ~AbstractInstruction() {}
	virtual void finalize(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex,
		std::vector<AbstractInstruction*>::const_iterator cur
	) = 0;
	virtual void execute(ExecutionManager& execMan, const StateType& state) = 0;

	virtual std::ostream& toStream(std::ostream&) const = 0;

	const CodeStorage::Insn* insn() const { return this->insn_; }
	void insn(const CodeStorage::Insn* insn) { this->insn_ = insn; }

	fi_type_e getType() const { return this->fiType_; }

	bool isTarget() const { return this->isTarget_; }

	void setTarget() { this->isTarget_ = true; }

	friend std::ostream& operator<<(std::ostream& os, const AbstractInstruction& instr) {

		return instr.toStream(os);

	}

};

#endif
