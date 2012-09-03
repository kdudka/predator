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

// Standard library headers
#include <list>
#include <vector>
#include <set>
#include <memory>
#include <cassert>

// Forester headers
#include "recycler.hh"
#include "exec_state.hh"
#include "forestautext.hh"
#include "abstractinstruction.hh"

/**
 * @file symstate.hh
 * SymState - structure that represents symbolic state of the execution engine
 */


/**
 * @brief  Symbolic state of the execution
 */
class SymState
{
public:   // data types

	typedef std::list<ExecState> QueueType;

	/// Trace of symbolic states
	typedef std::vector<const SymState*> Trace;

private:  // data members

	/// Parent symbolic state
	SymState* parent_;

	/// Instruction that the symbolic state corresponds to
	AbstractInstruction* instr_;

	/// Forest automaton for the symbolic state
	std::shared_ptr<const FAE> fae_;

	/// Children symbolic states
	std::set<SymState*> children_;

	/// @todo: write dox
	QueueType::iterator queueTag_;

private:  // methods

	SymState(const SymState&);
	SymState& operator=(const SymState&);

public:   // methods

	/**
	 * @brief  Constructor
	 *
	 * Default constructor
	 */
	SymState() :
		parent_{},
		instr_{},
		fae_{},
		children_{},
		queueTag_{}
	{ }

	/**
	 * @brief  Destructor
	 *
	 * Destructor
	 */
	~SymState()
	{
		// Assertions
		assert(nullptr == fae_);
		assert(children_.empty());
	}

	std::shared_ptr<const FAE> GetFAE() const
	{
		return fae_;
	}

	const AbstractInstruction* GetInstr() const
	{
		return instr_;
	}

	AbstractInstruction* GetInstr()
	{
		return instr_;
	}

	SymState* GetParent()
	{
		return parent_;
	}

	const std::set<SymState*>& GetChildren() const
	{
		return children_;
	}

	void SetQueueTag(const QueueType::iterator tag)
	{
		queueTag_ = tag;
	}

	/**
	 * @brief  Add a child symbolic state
	 *
	 * Adds a new child symbolic state
	 *
	 * @param[in]  child  The child symbolic state to be added
	 */
	void addChild(SymState* child)
	{
		if (!children_.insert(child).second)
		{	// in case already present state was added
			assert(false);        // fail gracefully
		}
	}

	/**
	 * @brief  Remove a child symbolic state
	 *
	 * Removes a child symbolic state
	 *
	 * @param[in]  child  The child symbolic state to be removed
	 */
	void removeChild(SymState* child) {
		if (children_.erase(child) != 1)
		{	// in case the state to be removed is not present
			assert(false);        // fail gracefully
		}
	}


	/**
	 * @brief  Initializes the symbolic state
	 *
	 * Method that initializes the symbolic state.
	 *
	 * @param[in]  parent    The parent symbolic state
	 * @param[in]  instr     The instruction the state corresponds to
	 * @param[in]  fae       The forest automaton for the symbolic state
	 * @param[in]  queueTag  @todo write dox
	 */
	void init(
		SymState*                                      parent,
		AbstractInstruction*                           instr,
		const std::shared_ptr<const FAE>&              fae,
		QueueType::iterator                            queueTag);


	/**
	 * @brief  Recycles the symbolic state for further use
	 *
	 * This method recycles the symbolic state for further use. It also recycles
	 * recursively all children of the state.
	 *
	 * @param[in,out]  recycler  The Recycler object
	 */
	void recycle(Recycler<SymState>& recycler);


	/**
	 * @brief  Run a visitor on the instance
	 *
	 * This method is the @p accept method of the Visitor design pattern.
	 *
	 * @param[in]  visitor  The visitor of the type @p TVisitor
	 *
	 * @tparam  TVisitor  The type of the visitor
	 */
	template <class TVisitor>
	void accept(TVisitor& visitor) const
	{
		visitor(*this);
	}


	/**
	 * @brief  Retrieves the trace to this state
	 *
	 * This method retrieves the trace trace to this state in the reverse order
	 * (starting with this state).
	 *
	 * @returns  The trace
	 */
	Trace getTrace() const;
};

#endif
