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

#ifndef INTEGRITY_H
#define INTEGRITY_H

// Standard library headers
#include <vector>

// Forester headers
#include "forestautext.hh"


class Integrity
{
private:  // data types

	struct CheckIntegrityF;

private:  // data members

	/// The Forest Automaton checked for integrity
	const FAE& fae_;

private:  // methods


	/**
	 * @brief  Retrieve leaves that reference the target tree automaton 
	 *
	 * @param[out]  selectors  The leaves referencing the target tree automaton
	 * @param[in]   target     The index of the tree automaton
	 */
	void enumerateSelectorsAtLeaf(std::set<size_t>& selectors, size_t target) const;


	/**
	 * @brief  Checks the integrity of a single state of a tree automaton
	 *
	 * @param[in]      ta       The tree automaton into which the state belongs
	 * @param[in]      state    The state the integrity of which is to be checked
	 * @param[in]      defined  The set that contains leaf states referencing the root
	 *                          of the tree automaton
	 * @param[in,out]  bitmap   The set of already processed tree automata
	 * @param[in,out]  states   The map of states in tree automata to the set of
	 *                          leaves that reference them
	 *
	 * @returns  @p true if the integrity holds, @p false otherwise
	 */
	bool checkState(
		const TreeAut& ta,
		size_t state,
		const std::set<size_t>& defined,
		std::vector<bool>& bitmap,
		std::map<std::pair<const TreeAut*, size_t>, std::set<size_t>>& states) const;


	/**
	 * @brief  Checks integrity of a root of a forest automaton
	 *
	 * @param[in]      root    Number of the root of the forest automaton
	 * @param[in,out]  bitmap  Bitmap of processed roots
	 * @param[in,out]  states  @todo
	 *
	 * @returns  @p true if the integrity holds, @p false otherwise
	 */
	bool checkRoot(
		size_t root,
		std::vector<bool>& bitmap,
		std::map<std::pair<const TreeAut*, size_t>, std::set<size_t>>& states) const;


public:   // methods

	/**
	 * @brief  The constructor
	 *
	 * Constructs the @p Integrity object for a forest automaton
	 *
	 * @param[in]  fae  The forest automaton
	 */
	Integrity(const FAE& fae) :
		fae_(fae)
	{ }

	/**
	 * @brief  Checks integrity of a Forest Automaton
	 *
	 * @returns @p true if the integrity holds, @p false otherwise
	 */
	bool check() const;
};

#endif
