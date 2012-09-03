/*
 * Copyright (C) 2012 Jiri Simacek
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


// Forester headers
#include "compiler.hh"
#include "integrity.hh"
#include "memplot.hh"
#include "symstate.hh"


void SymState::init(
	SymState*                             parent,
	AbstractInstruction*                  instr,
	const std::shared_ptr<const FAE>&     fae,
	QueueType::iterator                   queueTag)
{
	// Assertions
	assert(Integrity(*fae).check());

	parent_ = parent;
	instr_ = instr;
	fae_ = fae;
	queueTag_ = queueTag;
	if (parent_)
		parent_->addChild(this);
}


void SymState::recycle(Recycler<SymState>& recycler)
{
	if (parent_)
	{
		parent_->removeChild(this);
	}

	std::vector<SymState*> stack = { this };

	while (!stack.empty()) {
		// recycle recursively all children

		SymState* state = stack.back();
		stack.pop_back();

		assert(state->GetFAE());
		state->fae_ = nullptr;

		for (auto s : state->GetChildren())
		{
			stack.push_back(s);
		}

		state->children_.clear();
		recycler.recycle(state);
	}
}


SymState::Trace SymState::getTrace() const
{
	Trace trace;

	const SymState* state = this;
	while (nullptr != state)
	{
		trace.push_back(state);
		state = state->parent_;
	}

	return trace;
}
