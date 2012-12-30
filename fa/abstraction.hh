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

/**
 * @brief  The class that performs abstraction
 *
 * This class peforms abstraction over tree automata.
 */
class Abstraction
{
private:  // data members

	FAE& fae_;

public:   // methods

	/**
	 * @brief  Performs finite height abstraction
	 *
	 * This method performs the finite height abtraction over a tree automaton at
	 * index @p root of the forest automaton. The abstraction is for @p height.
	 *
	 * @param[in]  root    The root on which the abstraction is to be applied
	 * @param[in]  height  The height of the abstraction
	 * @param[in]  f       Functor for matching transitions
	 */
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
		std::vector<std::vector<bool>> rel(stateIndex.size(),
			std::vector<bool>(stateIndex.size(), true));

		// compute the abstraction (i.e. which states are to be merged)
		fae_.getRoot(root)->heightAbstraction(rel, height, f, stateIndex);

		ConnectionGraph::StateToCutpointSignatureMap stateMap;
		ConnectionGraph::computeSignatures(stateMap, *fae_.getRoot(root));
		for (Index<size_t>::iterator j = stateIndex.begin(); j != stateIndex.end(); ++j)
		{	// go through the matrix
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


	/**
	 * @brief  Performs predicate abstraction
	 *
	 * This method performs predicate abstraction of a forest automaton using
	 * a set of predicates (currently only one predicate).
	 *
	 * @param[in]  predicates  The predicates used for the abstraction
	 */
	void predicateAbstraction(
		const std::vector<std::shared_ptr<const FAE>>&    predicates)
	{
		FA_NOTE("Predicate abstraction input: " << fae_);

		Index<size_t> faeStateIndex;
		for (size_t i = 0; i < fae_.getRootCount(); ++i)
		{
			assert(nullptr != fae_.getRoot(i));
			fae_.getRoot(i)->buildStateIndex(faeStateIndex);
		}

		size_t numStates = faeStateIndex.size();

		FA_NOTE("Index: " << faeStateIndex);

		// create the initial relation
		// TODO: use boost::dynamic_bitset
		std::vector<std::vector<bool>> rel;

		if (!predicates.empty())
		{
			FA_NOTE("Predicate: " << *predicates.back());

			std::set<std::pair<size_t, size_t>> product;
			FAE::makeProduct(fae_, *predicates.back(), product);

			// create a map of states of 'fae_' on sets of states of 'predicate'
			std::vector<std::set<size_t>> matchWith(numStates, std::set<size_t>());
			for (const std::pair<size_t, size_t>& statePair : product)
			{
				matchWith[faeStateIndex[statePair.first]].insert(statePair.second);
			}

			std::ostringstream oss;
			for (size_t i = 0; i < matchWith.size(); ++i)
			{
				oss << ", " << i << " -> ";
				utils::printCont(oss, matchWith[i]);
			}

			FA_NOTE("matchWith: " << oss.str());

			// create the relation
			rel.assign(numStates, std::vector<bool>(numStates, false));
			for (size_t i = 0; i < numStates; ++i)
			{
				rel[i][i] = true;

				for (size_t j = 0 ; j < i; ++j)
				{
					if (matchWith[i] == matchWith[j])
					{
						rel[i][j] = true;
						rel[j][i] = true;
					}
				}
			}
		}
		else
		{
			// create universal relation
			rel.assign(numStates, std::vector<bool>(numStates, true));
		}

		for (size_t i = 0; i < fae_.getRootCount(); ++i)
		{
			assert(nullptr != fae_.getRoot(i));

			// refine the relation according to cutpoints etc.
			ConnectionGraph::StateToCutpointSignatureMap stateMap;
			ConnectionGraph::computeSignatures(stateMap, *fae_.getRoot(i));
			for (auto j = faeStateIndex.begin(); j != faeStateIndex.end(); ++j)
			{	// go through the matrix
				for (auto k = faeStateIndex.begin(); k != faeStateIndex.end(); ++k)
				{
					if (k == j)
						continue;

					if (fae_.getRoot(i)->isFinalState(j->first)
						|| fae_.getRoot(i)->isFinalState(k->first))
					{
						rel[j->second][k->second] = false;
						continue;
					}

					// load data if present
					const Data* jData;
					bool jIsData = fae_.isData(j->first, jData);
					assert((!jIsData || (nullptr != jData)) && (!(nullptr == jData) || !jIsData));
					const Data* kData;
					bool kIsData = fae_.isData(k->first, kData);
					assert((!kIsData || (nullptr != kData)) && (!(nullptr == kData) || !kIsData));

					if (jIsData || kIsData)
					{
						rel[j->second][k->second] = false;
						continue;
					}

					if (stateMap[j->first] % stateMap[k->first])
						continue;

					rel[j->second][k->second] = false;
				}
			}
		}

		std::ostringstream oss;
		utils::relPrint(oss, rel);
		FA_NOTE("Relation: \n" << oss.str());

		std::ostringstream ossInd;
		ossInd << '[';
		for (auto it = faeStateIndex.begin(); it != faeStateIndex.end(); ++it)
		{
			ossInd << '(' << FA::writeState(it->first) << ',' << it->second << ')';
		}

		ossInd << ']';
		FA_NOTE("Index: " << ossInd.str());

		// TODO: label states of fae_ by states of predicate

		for (size_t i = 0; i < fae_.getRootCount(); ++i)
		{
			TreeAut ta(*fae_.backend);
			fae_.getRoot(i)->collapsed(ta, rel, faeStateIndex);
			fae_.setRoot(i, std::shared_ptr<TreeAut>(fae_.allocTA()));
			ta.uselessAndUnreachableFree(*fae_.getRoot(i));
		}

		FA_NOTE("Predicate abstraction output: " << fae_);
	}


public:

	Abstraction(FAE& fae) :
		fae_(fae)
	{ }
};

#endif
