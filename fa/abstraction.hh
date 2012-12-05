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

#ifndef ABSTRACTION_H
#define ABSTRACTION_H

// Forester headers
#include "forestautext.hh"
#include "streams.hh"

class Abstraction
{
private:  // data members

	FAE& fae_;

public:   // methods

	template <class F>
	void heightAbstraction(
		size_t                root,
		size_t                height,
		F                     f)
	{
		// Preconditions
		assert(root < fae_.getRootCount());
		assert(nullptr != fae_.getRoot(root));

		Index<size_t> stateIndex;
		fae_.getRoot(root)->buildStateIndex(stateIndex);
		std::vector<std::vector<bool> > rel(stateIndex.size(), std::vector<bool>(stateIndex.size(), true));
		fae_.getRoot(root)->heightAbstraction(rel, height, f, stateIndex);
		ConnectionGraph::StateToCutpointSignatureMap stateMap;
		ConnectionGraph::computeSignatures(stateMap, *fae_.getRoot(root));
		for (Index<size_t>::iterator j = stateIndex.begin(); j != stateIndex.end(); ++j)
		{
			for (Index<size_t>::iterator k = stateIndex.begin(); k != stateIndex.end(); ++k)
			{
				if (k == j)
					continue;

				if (stateMap[j->first] % stateMap[k->first])
					continue;

				rel[j->second][k->second] = false;
			}
		}

		TreeAut ta(*fae_.backend);
		fae_.getRoot(root)->collapsed(ta, rel, stateIndex);
		fae_.setRoot(root, std::shared_ptr<TreeAut>(fae_.allocTA()));
		ta.uselessAndUnreachableFree(*fae_.getRoot(root));
	}

public:

	Abstraction(FAE& fae) :
		fae_(fae)
	{ }
};

#endif
