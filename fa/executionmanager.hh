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
private:  // data types

	typedef std::list<SymState*> QueueType;

private:  // data members

	/// the root of the execution graph
	SymState* root_;

	/// the queue with the states to be processed
	QueueType queue_;

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
		if (nullptr != root_)
		{
			root_->recycle(stateRecycler_);
			root_ = nullptr;
		}

		queue_.clear();

		statesExecuted_ = 0;
		pathsEvaluated_ = 0;
	}

	SymState* createState()
	{
		SymState* state = stateRecycler_.alloc();
		assert(nullptr != state);
		return state;
	}


	SymState* createChildState(
		SymState&                          oldState,
		AbstractInstruction*               instr)
	{
		SymState* state = createState();
		state->initChildFrom(&oldState, instr);

		return state;
	}

	SymState* createChildStateWithNewRegs(
		SymState&                          oldState,
		AbstractInstruction*               instr)
	{
		SymState* state = createState();
		const std::shared_ptr<DataArray> regs = allocRegisters(oldState.GetRegs());
		state->initChildFrom(&oldState, instr, regs);

		return state;
	}

	SymState* copyState(
		const SymState&                    oldState)
	{
		SymState* state = createState();
		state->init(oldState);

		return state;
	}

	SymState* copyStateWithNewRegs(
		const SymState&                    oldState)
	{
		SymState* state = createState();
		const std::shared_ptr<DataArray> regs = allocRegisters(oldState.GetRegs());
		state->init(oldState, regs);

		return state;
	}

	SymState* copyStateWithNewRegs(
		const SymState&                    oldState,
		const AbstractInstruction*         insn)
	{
		SymState* state = createState();
		const std::shared_ptr<DataArray> regs = allocRegisters(oldState.GetRegs());
		state->init(oldState, regs, const_cast<AbstractInstruction*>(insn));

		return state;
	}

	SymState* enqueue(
		SymState*                           parent,
		const std::shared_ptr<DataArray>&   registers,
		const std::shared_ptr<const FAE>&   fae,
		AbstractInstruction*                instr)
	{
		SymState* state = createState();

		state->init(parent, instr, fae, registers);
		queue_.push_back(state);

		return state;
	}

	SymState* enqueue(
		SymState*                          state)
	{
		// Assertions
		assert(nullptr != state);

		queue_.push_back(state);
		return state;
	}

	SymState* dequeueBFS()
	{
		if (queue_.empty())
			return nullptr;

		SymState* state = queue_.front();
		assert(nullptr != state);

		queue_.pop_front();

		return state;
	}

	SymState* dequeueDFS()
	{
		if (queue_.empty())
			return nullptr;

		SymState* state = queue_.back();
		assert(nullptr != state);

		queue_.pop_back();

		return state;
	}

	std::shared_ptr<DataArray> allocRegisters(const DataArray& model)
	{
		DataArray* v = registerRecycler_.alloc();
		assert(nullptr != v);

		*v = model;

		return std::shared_ptr<DataArray>(v, RecycleRegisterF(registerRecycler_));
	}

	void init(const DataArray& registers, const std::shared_ptr<const FAE>& fae,
		AbstractInstruction* instr)
	{
		this->clear();
		root_ = this->enqueue(nullptr, this->allocRegisters(registers), fae, instr);
	}

	void execute(SymState& state)
	{
		// Assertions
		assert(nullptr != state.GetInstr());

		++statesExecuted_;

		state.GetInstr()->execute(*this, state);
	}

	void pathFinished(SymState* state)
	{
		++pathsEvaluated_;

		this->destroyBranch(state);
	}

	/**
	 * @brief  Recycles a state
	 *
	 * Recycles a state using the associated recycler.
	 *
	 * @param[in]  state  The state to be recycled
	 */
	void recycleState(
		SymState*                       state)
	{
		// Assertions
		assert(nullptr != state);

		state->recycle(stateRecycler_);
	}


	void destroyBranch(SymState* state)
	{
		// Assertions
		assert(nullptr != state);

		while (state->GetParent())
		{
			// Assertions
			assert(state->GetParent()->GetChildren().size());

			if (state->GetInstr()->getType() == fi_type_e::fiFix)
			{
				FixpointInstruction* fixpoint =
					static_cast<FixpointInstruction*>(state->GetInstr());
				fixpoint->extendFixpoint(state->GetFAE());
			}

			if (state->GetParent()->GetChildren().size() > 1)
			{
				state->recycle(stateRecycler_);
				return;
			}

			state = static_cast<SymState*>(state->GetParent());
		}

		// Assertions
		assert(state == root_);

		root_->recycle(stateRecycler_);
		root_ = nullptr;
	}
};

#endif
