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
public:   // data types

	/// Transition
	typedef TT<label_type> Transition;

private:  // data members

	/// The Forest Automaton that will be split
	FAE& fae_;

private:  // methods

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


	/**
	 * @brief  Adds redundant root points to allow further manipulation  
	 *
	 * Adds redundant root points into a FA in order to allow further
	 * manipulation only on roots. It does not unfold hierarchical boxes: that is
	 * the task of another @p isolateAtRoot method.
	 *
	 * @param[in]  root   Index of the desired tree automaton
	 * @param[in]  t      The transition to be handled
	 * @param[in]  f      Functor that returns @p true if a box contains any of the
	 *                    desired selectors
	 * @param[out] boxes  Hierarchical boxes that were encountered and need to be
	 *                    unfolded
	 */
	template <class F>
	void isolateAtRoot(
		size_t                             root,
		const Transition&                  t,
		F                                  f,
		std::set<const Box*>&              boxes);


	/**
	 * @brief  Unfolds for given offsets at desired root
	 *
	 * Unfolds the tree automaton with the index @p root, isolating selectors at
	 * given @p offsets.
	 *
	 * @param[out]  dst      The vector for storing results of the operation
	 * @param[in]   root     The index of the desired tree automaton
	 * @param[in]   offsets  Offsets to be isolated
	 */
	void isolateAtRoot(
		std::vector<FAE*>&                            dst,
		size_t                                        root,
		const std::vector<size_t>&                    offsets) const;

	/**
	 * @brief  TODO
	 *
	 * TODO
	 */
	// adds redundant root points to allow further manipulation
	void isolateAtLeaf(
		std::vector<FAE*>&                  dst,
		size_t                              root,
		size_t                              target,
		size_t                              selector) const;

public:   // methods

	Splitting(FAE& fae) : fae_(fae) {}
	Splitting(const FAE& fae) : fae_(const_cast<FAE&>(fae)) {}

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


};

#endif
