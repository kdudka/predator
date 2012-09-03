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

// Standard library headers
#include <list>

// Forester headers
#include "types.hh"
#include "recycler.hh"
#include "abstractinstruction.hh"
#include "fixpointinstruction.hh"
#include "symstate.hh"


/**
 * @brief  Class that carries out symbolic execution of the code
 *
 * This class performs symbolic execution of the code.
 */
class ExecutionManager
{
private:  // data members

	/// the root of the execution graph
	SymState* root_;

	/// the queue with the states to be processed
	SymState::QueueType queue_;

	/// counter of evaluated states
	size_t statesExecuted_;

	/// counter of evaluated paths
	size_t pathsEvaluated_;

	/// memory manager for registers
	Recycler<DataArray> registerRecycler_;
	/// memory manager for states
	Recycler<SymState> stateRecycler_;

	class RecycleRegisterF
	{
	private:  // data members

		Recycler<DataArray>& recycler_;

	public:   // methods

		RecycleRegisterF(Recycler<DataArray>& recycler) :
			recycler_(recycler)
		{ }

		void operator()(DataArray* x)
		{
			recycler_.recycle(x);
		}
	};

private:  // methods

	ExecutionManager(const ExecutionManager&);
	ExecutionManager& operator=(const ExecutionManager&);

public:

	ExecutionManager() :
		root_(nullptr),
		queue_{},
		statesExecuted_{},
		pathsEvaluated_{},
		registerRecycler_{},
		stateRecycler_{}
	{ }

	~ExecutionManager()
	{
		this->clear();
	}

	size_t statesEvaluated() const { return statesExecuted_; }

	size_t pathsEvaluated() const { return pathsEvaluated_; }

	void clear()
	{
		if (root_)
		{
			root_->recycle(stateRecycler_);
			root_ = nullptr;
		}

		queue_.clear();

		statesExecuted_ = 0;
		pathsEvaluated_ = 0;
	}

	SymState* enqueue(SymState* parent, const std::shared_ptr<DataArray>& registers,
		const std::shared_ptr<const FAE>& fae, AbstractInstruction* instr)
	{
		SymState* state = stateRecycler_.alloc();

		state->init(
			parent,
			instr,
			fae,
			queue_.insert(queue_.end(), ExecState(registers, state))
		);

		return state;
	}

	SymState* enqueue(const ExecState& parent, AbstractInstruction* instr)
	{
		SymState* state = stateRecycler_.alloc();

		state->init(
			parent.GetMem(),
			instr,
			parent.GetMem()->GetFAE(),
			queue_.insert(queue_.end(), ExecState(parent.GetRegsShPtr(), state))
		);

		return state;
	}

	bool dequeueBFS(ExecState& state)
	{
		if (queue_.empty())
			return false;

		state = queue_.front();
		queue_.pop_front();

		state.GetMem()->SetQueueTag(queue_.end());

		return true;
	}

	bool dequeueDFS(ExecState& state)
	{
		if (queue_.empty())
			return false;

		state = queue_.back();
		queue_.pop_back();

		state.GetMem()->SetQueueTag(queue_.end());

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

		state.GetMem()->GetInstr()->execute(*this, state);
	}

	void pathFinished(SymState* state)
	{
		++pathsEvaluated_;

		this->destroyBranch(state);
	}

	void destroyBranch(SymState* state)
	{
		// Assertions
		assert(state);

		while (state->GetParent())
		{
			// Assertions
			assert(state->GetParent()->GetChildren().size());

			if (state->GetInstr()->getType() == fi_type_e::fiFix)
				(static_cast<FixpointInstruction*>(state->GetInstr()))->extendFixpoint(state->GetFAE());

			if (state->GetParent()->GetChildren().size() > 1)
			{
				state->recycle(stateRecycler_);
				return;
			}

			state = state->GetParent();
		}

		// Assertions
		assert(state == root_);

		root_->recycle(stateRecycler_);
		root_ = nullptr;
	}
};

#endif
