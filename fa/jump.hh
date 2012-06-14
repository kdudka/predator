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

#ifndef JUMP_H
#define JUMP_H

#include "abstractinstruction.hh"

class FI_jmp : public AbstractInstruction {

	const CodeStorage::Block* target_;
	AbstractInstruction* next_;

private:  // methods

	FI_jmp(const FI_jmp&);
	FI_jmp& operator=(const FI_jmp&);

public:

	FI_jmp(const CodeStorage::Insn* insn, const CodeStorage::Block* target)
		: AbstractInstruction(insn, fi_type_e::fiJump), target_(target), next_(nullptr) {}

	static AbstractInstruction* getTarget(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex,
		const CodeStorage::Block* target
	) {
		auto tmp = codeIndex.find(target);
		assert(tmp != codeIndex.end());
		return tmp->second;
	}

	AbstractInstruction* getTarget(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex
	) {
		return getTarget(codeIndex, this->target_);
	}

	virtual void execute(ExecutionManager&, const ExecState&);

	virtual void finalize(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex,
		std::vector<AbstractInstruction*>::const_iterator
	);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "jmp   \t" << this->next_;
	}

};

#endif
