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

#ifndef NORMALIZATION_H
#define NORMALIZATION_H

// Standard library headers
#include <vector>
#include <map>

// Forester headers
#include "forestautext.hh"
#include "abstractbox.hh"
#include "streams.hh"
#include "symstate.hh"
#include "utils.hh"

class Normalization
{
private:  // data members

	FAE& fae;

	/// the corresponding symbolic state
	const SymState* state_;

protected:

	TreeAut* mergeRoot(
		TreeAut&                          dst,
		size_t                            ref,
		TreeAut&                          src,
		std::vector<size_t>&              joinStates);


	void traverse(
		std::vector<bool>&                visited,
		std::vector<size_t>&              order,
		std::vector<bool>&                marked) const;


	/**
	 * @brief  Traverse the forest automaton and mark visited components
	 *
	 * This method traverses the forest automaton and marks visited components
	 * in the passed bitmap.
	 *
	 * @param[out]  visited  Bitmap into which visited components are marked
	 */
	void traverse(
		std::vector<bool>&                visited) const;


	void checkGarbage(
		const std::vector<bool>&          visited) const;


	/**
	 * @brief  Normalizes given root recursively
	 *
	 * This method performs recursively normalization of all components reachable
	 * from given root. This only removes redundant root points (and preserves
	 * only root points which are real cutpoints), reordering of roots is
	 * performed on a higher level.
	 *
	 * @param[in,out]  normalized  Vector marking root points which are normalized
	 * @param[in]      root        The root to be normalized
	 * @param[in]      marked      Bitmap telling which root points are referenced
	 *                             more than once
	 */
	void normalizeRoot(
		std::vector<bool>&                normalized,
		size_t                            root,
		const std::vector<bool>&          marked);


public:


	/**
	 * @brief  Checks for garbage
	 *
	 * Checks for garbage, i.e. components unreachable from program variables.
	 *
	 * @todo  This method fails for backward-only reachable components
	 */
	void check() const;


	bool selfReachable(
		size_t                            root,
		size_t                            self,
		const std::vector<bool>&          marked);


	void scan(
		std::vector<bool>&                marked,
		std::vector<size_t>&              order,
		const std::set<size_t>&           forbidden = std::set<size_t>(),
		bool                              extended = false);


	/**
	 * @brief  Transforms the forest automaton into a canonicity-respecting form
	 *
	 * This method transforms the corresponding forest automaton into
	 * a canonicity-respecting form. This means that root points correspond to
	 * real cutpoints (other are merged) and the order of the root points is
	 * according to the depth-first traversal.
	 *
	 * @param[in]  marked  Vector marking roots which are referred more than once
	 * @param[in]  order   Vector with root indices in the right order
	 *
	 * @returns  @p true in case some components were merged, @p false otherwise
	 */
	bool normalize(
		const std::vector<bool>&          marked,
		const std::vector<size_t>&        order);


	/**
	 * @brief  Computes the indices of components which are not to be merged
	 *
	 * This function computes the set of indices of components of the forest
	 * automaton @p fae which are not to be merged or folded
	 *
	 * @param[in]  fae  The forest automaton
	 *
	 * @returns  The set with indices of components not to be merged or folded
	 */
	static std::set<size_t> computeForbiddenSet(FAE& fae);

public:   // methods

	Normalization(FAE& fae, const SymState* state) :
		fae(fae), state_{state}
	{ }
};

#endif
