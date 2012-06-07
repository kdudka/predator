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

	Recycler<DataArray> registerRecycler_;
	Recycler<SymState> stateRecycler_;

	struct RecycleRegisterF {

		Recycler<DataArray>& recycler_;

		RecycleRegisterF(Recycler<DataArray>& recycler)
			: recycler_(recycler) {}

		void operator()(DataArray* x) {

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

private:  // methods

	ExecutionManager(const ExecutionManager&);
	ExecutionManager& operator=(const ExecutionManager&);

public:

	ExecutionManager() : root_(nullptr), queue_{}, statesExecuted_{}, tracesEvaluated_{}, registerRecycler_{}, stateRecycler_{} {}

	~ExecutionManager() { this->clear(); }

	size_t statesEvaluated() const { return statesExecuted_; }

	size_t tracesEvaluated() const { return tracesEvaluated_; }

	void clear() {

		if (this->root_) {
			this->root_->recycle(this->stateRecycler_);
			this->root_ = nullptr;
		}

		this->queue_.clear();

		this->statesExecuted_ = 0;
		this->tracesEvaluated_ = 0;

	}

	SymState* enqueue(SymState* parent, const std::shared_ptr<DataArray>& registers,
		const std::shared_ptr<const FAE>& fae, AbstractInstruction* instr) {

		SymState* state = this->stateRecycler_.alloc();

		state->init(
			parent,
			instr,
			fae,
			this->queue_.insert(this->queue_.end(), ExecState(registers, state))
		);

		return state;

	}

	SymState* enqueue(const ExecState& parent, AbstractInstruction* instr)
	{
		SymState* state = stateRecycler_.alloc();

		state->init(
			parent.GetMem(),
			instr,
			parent.GetMem()->fae,
			this->queue_.insert(this->queue_.end(), ExecState(parent.GetRegsShPtr(), state))
		);

		return state;
	}

	bool dequeueBFS(ExecState& state)
	{
		if (queue_.empty())
			return false;

		state = queue_.front();
		queue_.pop_front();

		state.GetMem()->queueTag = queue_.end();

		return true;
	}

	bool dequeueDFS(ExecState& state)
	{
		if (queue_.empty())
			return false;

		state = queue_.back();
		queue_.pop_back();

		state.GetMem()->queueTag = queue_.end();

		return true;
	}

	std::shared_ptr<DataArray> allocRegisters(const DataArray& model)
	{
		DataArray* v = registerRecycler_.alloc();

		*v = model;

		return std::shared_ptr<DataArray>(v, RecycleRegisterF(registerRecycler_));
	}

	void init(const DataArray& registers, const std::shared_ptr<const FAE>& fae,
		AbstractInstruction* instr)
	{
		this->clear();
		root_ = this->enqueue(nullptr, this->allocRegisters(registers), fae, instr);
	}

	void execute(ExecState& state)
	{
		++statesExecuted_;

		state.GetMem()->instr->execute(*this, state);
	}

//	template <class F>
	void traceFinished(SymState* state)
	{
		++tracesEvaluated_;

		this->destroyBranch(state);
	}

//	template <class F>
	void destroyBranch(SymState* state/*, F f*/)
	{
		// Assertions
		assert(state);

		while (state->parent)
		{
			// Assertions
			assert(state->parent->children.size());

			if (state->instr->getType() == fi_type_e::fiFix)
				(static_cast<FixpointInstruction*>(state->instr))->extendFixpoint(state->fae);
//			f(state);

			if (state->parent->children.size() > 1)
			{
				state->recycle(stateRecycler_);
				return;
			}

			state = state->parent;
		}

		// Assertions
		assert(state == root_);

		root_->recycle(stateRecycler_);
		root_ = nullptr;
	}
};

#endif
