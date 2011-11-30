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

#ifndef SYM_STATE_H
#define SYM_STATE_H

#include <list>
#include <vector>
#include <set>
#include <memory>
#include <cassert>

#include <cl/cl_msg.hh>

#include "recycler.hh"
#include "types.hh"
#include "forestautext.hh"
#include "abstractinstruction.hh"
#include "integrity.hh"

struct SymState {

	friend std::ostream& operator<<(std::ostream& os, const AbstractInstruction::StateType& state) {

		os << "registers:";

		for (auto reg : *state.first)
			os << ' ' << reg;

		os << ", heap:" << std::endl << *state.second->fae;
		return os << "instruction (" << state.second->instr << "): " << *state.second->instr;

	}

	typedef std::list<AbstractInstruction::StateType> QueueType;

	SymState* parent;
	AbstractInstruction* instr;
	std::shared_ptr<const FAE> fae;
	std::set<SymState*> children;
	QueueType::iterator queueTag;
	void* payload;

	SymState() {}

	~SymState() {
		assert(this->fae == NULL);
		assert(this->children.empty());
	}

	void addChild(SymState* child) {
		if (!this->children.insert(child).second)
		{
			assert(false);
		}
	}

	void removeChild(SymState* child) {
		if (this->children.erase(child) != 1)
		{
			assert(false);
		}
	}

	void init(SymState* parent, AbstractInstruction* instr, const std::shared_ptr<const FAE>& fae, QueueType::iterator queueTag) {

		assert(Integrity(*fae).check());

		this->parent = parent;
		this->instr = instr;
		this->fae = fae;
		this->queueTag = queueTag;
		if (this->parent)
			this->parent->addChild(this);

	}

	void recycle(Recycler<SymState>& recycler) {

		if (this->parent)
			this->parent->removeChild(this);

		std::vector<SymState*> stack = { this };

		while (!stack.empty()) {

			SymState* state = stack.back();
			stack.pop_back();

			assert(state->fae);
			state->fae = NULL;

			for (auto s : state->children)
				stack.push_back(s);

			state->children.clear();
			recycler.recycle(state);

		}

	}

};

#endif
