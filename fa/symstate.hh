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

// Code Listener headers
#include <cl/cl_msg.hh>

// Forester headers
#include "recycler.hh"
#include "exec_state.hh"
#include "types.hh"
#include "forestautext.hh"
#include "abstractinstruction.hh"
#include "integrity.hh"

/**
 * @file symstate.hh
 * SymState - structure that represents symbolic state of the execution engine
 */


struct SymState
{
private:

	SymState(const SymState&);
	SymState& operator=(const SymState&);

public:


	typedef std::list<ExecState> QueueType;

	/// Parent symbolic state
	SymState* parent;

	/// Instruction that the symbolic state corresponds to
	AbstractInstruction* instr;

	/// Forest automaton for the symbolic state
	std::shared_ptr<const FAE> fae;

	/// Children symbolic states
	std::set<SymState*> children;

	/// @todo: write dox
	QueueType::iterator queueTag;

	/// @todo: write dox
	void* payload;

	/**
	 * @brief  Constructor
	 *
	 * Default constructor
	 */
	SymState() :
		parent{},
		instr{},
		fae{},
		children{},
		queueTag{},
		payload{}
	{ }

	/**
	 * @brief  Destructor
	 *
	 * Destructor
	 */
	~SymState() {
		// Assertions
		assert(this->fae == nullptr);
		assert(this->children.empty());
	}

	/**
	 * @brief  Add a child symbolic state
	 *
	 * Adds a new child symbolic state
	 *
	 * @param[in]  child  The child symbolic state to be added
	 */
	void addChild(SymState* child) {
		if (!this->children.insert(child).second)
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
		if (this->children.erase(child) != 1)
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
	void init(SymState* parent, AbstractInstruction* instr,
		const std::shared_ptr<const FAE>& fae, QueueType::iterator queueTag) {

		// Assertions
		assert(Integrity(*fae).check());

		this->parent = parent;
		this->instr = instr;
		this->fae = fae;
		this->queueTag = queueTag;
		if (this->parent)
			this->parent->addChild(this);
	}

	/**
	 * @brief  Recycles the symbolic state for further use
	 *
	 * This method recycles the symbolic state for further use. It also recycles
	 * recursively all children of the state.
	 *
	 * @param[in,out]  recycler  The Recycler object
	 */
	void recycle(Recycler<SymState>& recycler) {

		if (this->parent) {
			this->parent->removeChild(this);
		}

		std::vector<SymState*> stack = { this };

		while (!stack.empty()) {
			// recycle recursively all children

			SymState* state = stack.back();
			stack.pop_back();

			assert(state->fae);
			state->fae = nullptr;

			for (auto s : state->children) {
				stack.push_back(s);
			}

			state->children.clear();
			recycler.recycle(state);
		}
	}

};

#endif
