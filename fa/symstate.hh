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

#ifndef SYM_STATE_H
#define SYM_STATE_H

#include <list>
#include <vector>
#include <set>
#include <memory>
#include <cassert>

#include "recycler.hh"
#include "types.hh"
#include "forestautext.hh"
#include "abstractinstruction.hh"

struct SymState {

	typedef std::list<AbstractInstruction::StateType> QueueType;

	SymState* parent;
	AbstractInstruction* instr;
//	std::vector<Data>* registers;
	std::shared_ptr<const FAE> fae;
	std::set<SymState*> children;
	QueueType::iterator queueTag;
	void* payload;

	SymState() {}
//	SymState(size_t regFileSize) : registers(regFileSize) {}
/*
	SymState(SymState* parent, const FAE* fae, std::list<SymState*>::iterator queueTag) {
		this->init(parent, fae, queueTag);
	}
*/
	~SymState() {
		assert(this->fae == NULL);
		assert(this->children.empty());
//		delete this->fae;
//		this->releaseChildren();
	}

	void addChild(SymState* child) {
		bool b = this->children.insert(child).second;
		assert(b);
	}

	void removeChild(SymState* child) {
		size_t s = this->children.erase(child);
		assert(s == 1);
	}

	void init(SymState* parent, AbstractInstruction* instr, const std::shared_ptr<const FAE>& fae, QueueType::iterator queueTag) {

		this->parent = parent;
		this->instr = instr;
		this->fae = fae;
		this->queueTag = queueTag;
		if (this->parent)
			this->parent->addChild(this);

	}
/*
	struct RecycleF {
		void operator()(Recycler<SymState>& recycler, SymState* obj) {
			obj->recycle(recycler);
		}
	};
*/

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

/*
	void releaseChildren() {
		for (SymState* child : this->children)
			delete child;
		this->children.clear();
	}
*/
};

#endif
