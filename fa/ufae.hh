/*
 * Copyright (C) 2010 Jiri Simacek
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

#ifndef UFAE_H
#define UFAE_H

// Standard library headers
#include <vector>
#include <ostream>

// Forester headers
#include "utils.hh"
#include "label.hh"
#include "boxman.hh"
#include "forestautext.hh"

/**
 * @brief  Tree automaton representing a forest automaton
 *
 * This class is used to represent a forest automaton as a tree automaton. This
 * is done by merging transitions of all tree automata in the forest automaton
 * and adding a new root state, such that it can only make a transition into
 * all roots of the original tree automata over a special symbol.
 */
class UFAE
{
private:  // data members

	/// The tree automaton
	TreeAut& backend_;

	/// Offset of states of the original forest automaton
	size_t stateOffset_;

	/// Manager of boxes
	BoxMan& boxMan_;

public:   // methods

	UFAE(
		TreeAut&                    backend,
		BoxMan&                     boxMan) :
    backend_(backend),
		stateOffset_(1),
		boxMan_(boxMan)
	{
		// let 0 be the only accepting state
		backend_.addFinalState(0);
	}

	/**
	 * @brief  Clears the structure to the initial state
	 */
	void clear()
	{
		backend_.addFinalState(0);
		stateOffset_ = 1;
	}

	/**
	 * @brief  Retrievs the offset of states
	 *
	 * @returns  Offset of states from the original forest automaton
	 */
	size_t getStateOffset() const
	{
		return stateOffset_;
	}

	/**
	 * @brief  Sets the offset of states
	 *
	 * @param[in]  offset  The new offset of states
	 */
	void setStateOffset(size_t offset)
	{
		stateOffset_ = offset;
	}

	template <class T>
	struct Cursor {
		T begin;
		T end;
		T curr;
		Cursor(T begin, T end) : begin(begin), end(end), curr(begin) {}
		bool inc() {
			if (++this->curr != this->end)
				return true;
			this->curr = this->begin;
			return false;
		}
	};

	TreeAut& fae2ta(
		TreeAut&                   dst,
		Index<size_t>&             index,
		const FAE&                 src) const
	{
		dst.addFinalState(0);
		std::vector<Cursor<std::set<size_t>::const_iterator>> tmp;
		for (auto root : src.getRoots())
		{
			TreeAut::rename(
				dst,
				*root,
				FAE::RenameNonleafF(index, stateOffset_),
				false);

			assert(root->getFinalStates().size());
			tmp.push_back(Cursor<std::set<size_t>::const_iterator>(
				root->getFinalStates().begin(),
				root->getFinalStates().end()));
		}

		std::vector<size_t> lhs(tmp.size());
		label_type label = boxMan_.lookupLabel(tmp.size(), src.GetVariables());

		bool valid = true;
		while (valid)
		{
			for (size_t i = 0; i < lhs.size(); ++i)
			{
				lhs[i] = index[*tmp[i].curr] + stateOffset_;
			}
			dst.addTransition(lhs, label, 0);
			valid = false;

			for (auto it = tmp.begin(); !valid && it != tmp.end(); ++it)
			{
				valid = it->inc();
			}
		}
		return dst;
	}

	void join(const TreeAut& src, const Index<size_t>& index)
	{
		TreeAut::disjointUnion(backend_, src, false);
		stateOffset_ += index.size();
	}

	void adjust(const Index<size_t>& index)
	{
		stateOffset_ += index.size();
	}

	friend std::ostream& operator<<(std::ostream& os, const UFAE& ufae)
	{
		TAWriter<label_type>(os).writeOne(ufae.backend_);
		return os;
	}
};

#endif
