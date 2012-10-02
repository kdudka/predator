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

#ifndef SPLITTING_H
#define SPLITTING_H

// Standard library headers
#include <vector>
#include <set>

// Forester headers
#include "forestautext.hh"
#include "unfolding.hh"
#include "programerror.hh"
#include "utils.hh"

/**
 * @brief  Performs splitting of a tree automaton into several
 *
 * This class performs splitting of a tree automaton (or, more precisely,
 * a forest automaton) into several tree automata, e.g. during isolation of
 * a certain selector.
 */
class Splitting
{
private:  // data members

	FAE& fae_;

public:

	/**
	 * @brief  Gets offsets of direct selectors of given root
	 *
	 * This method returns offsets of all direct selectors (even those hidden
	 * inside boxes) of a tree automaton root state.
	 *
	 * @param[out]  selectors  Set into which the selectors will be added
	 * @param[in]   target     Index of the tree automaton in the FA
	 */
	void enumerateSelectorsAtRoot(
		std::set<size_t>&            selectors,
		size_t                       target) const;

	// enumerates upwards selectors
	void enumerateSelectorsAtLeaf(
		std::set<size_t>&               selectors,
		size_t                          root,
		size_t                          target) const;

	// enumerates upwards selectors
	void enumerateSelectorsAtLeaf(
		std::set<size_t>&           selectors,
		size_t                      target) const;

	void enumerateSelectors(
		std::set<size_t>&           selectors,
		size_t                      target) const;

	// adds redundant root points to allow further manipulation
	void isolateAtLeaf(
		std::vector<FAE*>&                  dst,
		size_t                              root,
		size_t                              target,
		size_t                              selector) const;


	// adds redundant root points to allow further manipulation
	template <class F>
	void isolateAtRoot(
		size_t                             root,
		const TT<label_type>&              t,
		F                                  f,
		std::set<const Box*>&              boxes)
	{
		// Assertions
		assert(root < fae_.roots.size());
		assert(fae_.roots[root]);

		size_t newState = fae_.freshState();

		TreeAut ta(*fae_.roots[root], false);

		ta.addFinalState(newState);

		std::vector<size_t> lhs;

		size_t lhsOffset = 0;

		for (auto j = t.label()->getNode().begin(); j != t.label()->getNode().end(); ++j)
		{
			if (!(*j)->isStructural())
				continue;

			const StructuralBox* b = static_cast<const StructuralBox*>(*j);
			if (!f(b))
			{
				// this box is not interesting
				for (size_t k = 0; k < (*j)->getArity(); ++k, ++lhsOffset)
					lhs.push_back(t.lhs()[lhsOffset]);
				continue;
			}

			// we have to isolate here
			for (size_t k = 0; k < (*j)->getArity(); ++k, ++lhsOffset)
			{
				if (FA::isData(t.lhs()[lhsOffset])) {
					// no need to create a leaf when it's already there
					lhs.push_back(t.lhs()[lhsOffset]);
					continue;
				}
				// update new left-hand side
				lhs.push_back(fae_.addData(ta, Data::createRef(fae_.roots.size())));
				// prepare new root
				TreeAut tmp(*fae_.roots[root], false);
				tmp.addFinalState(t.lhs()[lhsOffset]);
				TreeAut* tmp2 = fae_.allocTA();
				tmp.unreachableFree(*tmp2);
				// update 'o'
				fae_.appendRoot(tmp2);
				fae_.connectionGraph.newRoot();
			}
			if (b->isType(box_type_e::bBox))
				boxes.insert(static_cast<const Box*>(*j));
		}

		ta.addTransition(lhs, t.label(), newState);

		TreeAut* tmp = fae_.allocTA();

		ta.unreachableFree(*tmp);

		// exchange the original automaton with the new one
		fae_.roots[root] = std::shared_ptr<TreeAut>(tmp);
		fae_.connectionGraph.invalidate(root);
	}

	// adds redundant root points to allow further manipulation
	void isolateAtRoot(
		std::vector<FAE*>&                            dst,
		size_t                                        root,
		const std::vector<size_t>&                    offsets) const;

	/**
	 * @brief  Isolates a single selector from a root
	 *
	 * Isolates one selector at given offset directly under the root of a given
	 * tree automaton.
	 *
	 * @param[out]  dst     Vector into which the resulting FAs will be pushed
	 * @param[in]   target  Index of the desired tree automaton
	 * @param[in]   offset  Offset of the selector to be isolated
	 */
	void isolateOne(
		std::vector<FAE*>&                     dst,
		size_t                                 target,
		size_t                                 offset) const
	{
		this->isolateSet(dst, target, 0, { offset });
	}

	/**
	 * @brief  Isolates a set of selectors from a root
	 *
	 * Isolates a set of selectors at given offsets directly under the root of
	 * a given tree automaton.
	 *
	 * @param[out]  dst      Vector into which the resulting FAs will be pushed
	 * @param[in]   target   Index of the desired tree automaton
	 * @param[in]   base     Base to which the @p offsets relate
	 * @param[in]   offsets  Offsets of the selectors to be isolated
	 */
	void isolateSet(
		std::vector<FAE*>&                 dst,
		size_t                             target,
		int                                base,
		const std::vector<size_t>&         offsets) const;

public:

	Splitting(FAE& fae) : fae_(fae) {}
	Splitting(const FAE& fae) : fae_(const_cast<FAE&>(fae)) {}

};

#endif
