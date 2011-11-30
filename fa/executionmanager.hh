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

#ifndef EXECUTION_MANAGER_H
#define EXECUTION_MANAGER_H

#include <list>

#include "types.hh"
#include "recycler.hh"
#include "abstractinstruction.hh"
#include "fixpointinstruction.hh"
#include "symstate.hh"

class ExecutionManager {

	SymState* root_;

	SymState::QueueType queue_;

	size_t statesExecuted_;
	size_t tracesEvaluated_;

	Recycler<std::vector<Data>> registerRecycler_;
	Recycler<SymState> stateRecycler_;

	struct RecycleRegisterF {

		Recycler<std::vector<Data>>& recycler_;

		RecycleRegisterF(Recycler<std::vector<Data>>& recycler)
			: recycler_(recycler) {}

		void operator()(std::vector<Data>* x) {

			this->recycler_.recycle(x);

		}

	};
/*
public:

	struct DestroySimpleF {

		DestroySimpleF() {}

		void operator()(SymState* state) {

			AbstractInstruction* instr = state->instr;
			if (instr->computesFixpoint())
				((FixpointInstruction*)instr)->extendFixpoint(state->fae);
	//			else
	//				CFG_FROM_FAE(*node->fae)->invalidate(node->fae);

		}

	};
*/
public:

	ExecutionManager() : root_(NULL) {}

	~ExecutionManager() { this->clear(); }

	size_t statesEvaluated() const { return statesExecuted_; }

	size_t tracesEvaluated() const { return tracesEvaluated_; }

	void clear() {

		if (this->root_) {
			this->root_->recycle(this->stateRecycler_);
			this->root_ = NULL;
		}

		this->queue_.clear();

		this->stateRecycler_.clear();
		this->registerRecycler_.clear();

		this->statesExecuted_ = 0;
		this->tracesEvaluated_ = 0;

	}

	SymState* enqueue(SymState* parent, const std::shared_ptr<std::vector<Data>>& registers,
		const std::shared_ptr<const FAE>& fae, AbstractInstruction* instr) {

		SymState* state = this->stateRecycler_.alloc();

		state->init(
			parent,
			instr,
			fae,
			this->queue_.insert(this->queue_.end(), std::make_pair(registers, state))
		);

		return state;

	}

	SymState* enqueue(const AbstractInstruction::StateType& parent, AbstractInstruction* instr) {

		SymState* state = this->stateRecycler_.alloc();

		state->init(
			parent.second,
			instr,
			parent.second->fae,
			this->queue_.insert(this->queue_.end(), std::make_pair(parent.first, state))
		);

		return state;

	}

	bool dequeueBFS(AbstractInstruction::StateType& state) {

		if (this->queue_.empty())
			return false;

		state = this->queue_.front();

		this->queue_.pop_front();

		state.second->queueTag = this->queue_.end();

		return true;

	}

	bool dequeueDFS(AbstractInstruction::StateType& state) {

		if (this->queue_.empty())
			return false;

		state = this->queue_.back();

		this->queue_.pop_back();

		state.second->queueTag = this->queue_.end();

		return true;

	}

	std::shared_ptr<std::vector<Data>> allocRegisters(const std::vector<Data>& model) {

		std::vector<Data>* v = this->registerRecycler_.alloc();

		*v = model;

		return std::shared_ptr<std::vector<Data>>(v, RecycleRegisterF(this->registerRecycler_));

	}

	void init(const std::vector<Data>& registers, const std::shared_ptr<const FAE>& fae,
		AbstractInstruction* instr) {

		this->clear();
		this->root_ = this->enqueue(NULL, this->allocRegisters(registers), fae, instr);

	}

	void execute(AbstractInstruction::StateType& state) {

		++this->statesExecuted_;

		state.second->instr->execute(*this, state);

	}

//	template <class F>
	void traceFinished(SymState* state) {

		++this->tracesEvaluated_;

		this->destroyBranch(state);

	}

//	template <class F>
	void destroyBranch(SymState* state/*, F f*/) {

		assert(state);

		while (state->parent) {

			assert(state->parent->children.size());

			if (state->instr->getType() == fi_type_e::fiFix)
				((FixpointInstruction*)state->instr)->extendFixpoint(state->fae);
//			f(state);

			if (state->parent->children.size() > 1) {
				state->recycle(this->stateRecycler_);
				return;
			}

			state = state->parent;

		}

		assert(state == this->root_);

		this->root_->recycle(this->stateRecycler_);
		this->root_ = NULL;

	}

};

#endif
