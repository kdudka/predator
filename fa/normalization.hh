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


	void traverse(
		std::vector<bool>&                visited) const;


	void checkGarbage(
		const std::vector<bool>&          visited) const;


public:


	// check garbage
	void check() const;


	void normalizeRoot(
		std::vector<bool>&                normalized,
		size_t                            root,
		const std::vector<bool>&          marked);


	bool selfReachable(
		size_t                            root,
		size_t                            self,
		const std::vector<bool>&          marked);


	void scan(
		std::vector<bool>&                marked,
		std::vector<size_t>&              order,
		const std::set<size_t>&           forbidden = std::set<size_t>(),
		bool                              extended = false);


	// normalize representation
	bool normalize(
		const std::vector<bool>&          marked,
		const std::vector<size_t>&        order);


public:   // methods

	Normalization(FAE& fae, const SymState* state) :
		fae(fae), state_{state}
	{ }
};

#endif
