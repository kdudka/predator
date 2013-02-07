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

#ifndef FOLDING_H
#define FOLDING_H

// Standard library headers
#include <vector>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

// Forester headers
#include "abstractbox.hh"
#include "boxman.hh"
#include "config.h"
#include "connection_graph.hh"
#include "forestautext.hh"
#include "restart_request.hh"
#include "streams.hh"

class Folding
{
private:  // data types

	typedef TreeAut::Transition Transition;

private:  // data members

	FAE& fae;
	BoxMan& boxMan;

	std::vector<std::pair<bool, ConnectionGraph::StateToCutpointSignatureMap>> signatureMap;

protected:

	static void copyBox(
		std::vector<size_t>&                lhs,
		std::vector<const AbstractBox*>&    label,
		const AbstractBox*                  box,
		const std::vector<size_t>&          srcLhs,
		const size_t&                       srcOffset)
	{
		for (size_t i = 0; i < box->getArity(); ++i)
			lhs.push_back(srcLhs[srcOffset + i]);

		label.push_back(box);
	}

	static const ConnectionGraph::CutpointSignature& getSignature(
		size_t                                                 state,
		const ConnectionGraph::StateToCutpointSignatureMap&    signatures)
	{
		auto iter = signatures.find(state);

		assert(iter != signatures.end());

		return iter->second;
	}

	static bool isSignaturesCompatible(
		const ConnectionGraph::CutpointSignature&    s1,
		const ConnectionGraph::CutpointSignature&    s2);

	const ConnectionGraph::StateToCutpointSignatureMap& getSignatures(
		size_t        root);

	void invalidateSignatures(size_t root)
	{
		// Preconditions
		assert(root < this->signatureMap.size());

		this->signatureMap[root].first = false;
	}

	void componentCut(
		TreeAut&                                 res,
		TreeAut&                                 complement,
		ConnectionGraph::CutpointSignature&      complementSignature,
		size_t                                   root,
		size_t                                   state,
		size_t                                   target);

	std::pair<std::shared_ptr<TreeAut>, std::shared_ptr<TreeAut>> separateCutpoint(
		ConnectionGraph::CutpointSignature&            boxSignature,
		size_t                                         root,
		size_t                                         state,
		size_t                                         cutpoint);

	std::shared_ptr<TreeAut> relabelReferences(
		const                              TreeAut& ta,
		std::vector<size_t>&               index)
	{
		auto tmp = std::shared_ptr<TreeAut>(this->fae.allocTA());

		this->fae.relabelReferences(*tmp, ta, index);

		return tmp;
	}

	/**
	 * @brief  @todo
	 */
	std::shared_ptr<TreeAut> joinBox(
		const TreeAut&                               src,
		size_t                                       state,
		size_t                                       root,
		const Box*                                   box,
		const ConnectionGraph::CutpointSignature&    signature);

	static void updateSelectorMap(
		std::unordered_map<size_t, size_t>&          m,
		size_t                                       selector,
		const ConnectionGraph::CutpointSignature&    signature)
	{
		for (auto& cutpoint : signature)
		{
			auto p = m.insert(std::make_pair(cutpoint.root, selector));

			if (!p.second && p.first->second > selector)
				p.first->second = selector;
		}
	}

	// compute cutpoint-to-selector mapping, i.e. tell which selector one needs to take
	// in order to reach a given cutpoint
	static void computeSelectorMap(
		std::unordered_map<size_t, size_t>&                    selectorMap,
		const Transition&                                      t,
		const ConnectionGraph::StateToCutpointSignatureMap&    stateMap);

	bool checkSelectorMap(
		const std::unordered_map<size_t, size_t>&     selectorMap,
		size_t                                        root,
		size_t                                        state);

	bool computeSelectorMap(
		std::unordered_map<size_t, size_t>&      selectorMap,
		size_t                                   root,
		size_t                                   state);

	static size_t extractSelector(
		const std::unordered_map<size_t, size_t>&    selectorMap,
		size_t                                       target)
	{
		auto iter = selectorMap.find(target);

		assert(iter != selectorMap.end());

		return iter->second;
	}

	// transform
	static void extractInputMap(
		std::vector<size_t>&                         inputMap,
		const std::unordered_map<size_t, size_t>&    selectorMap,
		size_t                                       root,
		const std::vector<size_t>&                   index);

/*
	static bool checkSingular(const TreeAut& ta, bool result,
		const ConnectionGraph::StateToCutpointSignatureMap& stateMap) {

		for (auto& state : ta.getFinalStates()) {

			auto iter = stateMap.find(state);

			assert(iter != stateMap.end());

			if (iter->second.empty() != result)
				return false;

		}

		return true;

	}

	static bool isSingular(const TreeAut& ta) {

		ConnectionGraph::StateToCutpointSignatureMap stateMap;

		ConnectionGraph::computeSignatures(stateMap, ta);

		assert(ta.getFinalStates().size());

		auto iter = stateMap.find(*ta.getFinalStates().begin());

		assert(iter != stateMap.end());

		bool result = iter->second.empty();

		assert(Folding::checkSingular(ta, result, stateMap));

		return result;

	}
*/
	const Box* getBox(const Box& box, bool conditional)
	{
		return (conditional)?(this->boxMan.lookupBox(box)):(this->boxMan.getBox(box));
	}

	const Box* makeType1Box(
		size_t                        root,
		size_t                        state,
		size_t                        aux,
		const std::set<size_t>&       forbidden,
		bool                          conditional = true,
		bool                          test = false);

	const Box* makeType2Box(
		size_t                      root,
		size_t                      aux,
		const std::set<size_t>&     forbidden,
		bool                        conditional = true,
		bool                        test = false);

public:

	bool discover1(
		size_t                       root,
		const std::set<size_t>&      forbidden,
		bool                         conditional);

	bool discover2(
		size_t                       root,
		const std::set<size_t>&      forbidden,
		bool                         conditional);

	bool discover3(
		size_t                      root,
		const std::set<size_t>&     forbidden,
		bool                        conditional);

/*
	bool discover(size_t root, const std::set<size_t>& forbidden) {

		assert(this->fae.connectionGraph.isValid());
		assert(this->fae.roots.size() == this->fae.connectionGraph.data.size());
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);

		if (forbidden.count(root))
			return false;

		const Box* boxPtr;

		FA_DEBUG_AT(3, "analysing: " << this->fae);

		// save state offset
		this->fae.pushStateOffset();

		bool found = false;

		size_t selectorToRoot;
start:
		this->fae.updateConnectionGraph();

		for (auto& cutpoint : this->fae.connectionGraph.data[root].signature) {

			if (cutpoint.root == root) {

				FA_DEBUG_AT(3, "type 1 cutpoint detected at root " << root);

				boxPtr = this->makeType1Box(
					root, this->fae.roots[root]->getFinalState(), root, forbidden, true
				);

				if (boxPtr)
					goto box_found;

				this->fae.popStateOffset();

				continue;

			}

			if (cutpoint.joint) {

				auto& signatures = this->getSignatures(root);

				for (auto& stateSignaturePair : signatures) {

					for (auto& tmp : stateSignaturePair.second) {

						if (!tmp.joint || tmp.joinInherited || (tmp.root != cutpoint.root))
							continue;

						FA_DEBUG_AT(3, "type 2 cutpoint detected inside component " << root << " at state q" << stateSignaturePair.first);

						boxPtr = this->makeType1Box(
							root, stateSignaturePair.first, cutpoint.root, forbidden, true
						);

						if (boxPtr)
							goto box_found;

						this->fae.popStateOffset();

					}

				}

				continue;

			}

			if (forbidden.count(cutpoint.root))
				continue;

			selectorToRoot = ConnectionGraph::getSelectorToTarget(
				this->fae.connectionGraph.data[cutpoint.root].signature, root
			);

			if (selectorToRoot == (size_t)(-1))
				continue;
*//*
			if (selectorToRoot == cutpoint.forwardSelector)
				continue;
*//*
			assert(!cutpoint.fwdSelectors.empty());

			FA_DEBUG_AT(3, "type 3 cutpoint detected at roots " << root << " and " << cutpoint.root);

			boxPtr = this->makeType2Box(root, cutpoint.root, forbidden, true);

			if (boxPtr)
				goto box_found;

			this->fae.popStateOffset();

			continue;
box_found:
			FA_DEBUG_AT(3, (AbstractBox*)boxPtr << " found");

			found = true;

			if (!this->fae.connectionGraph.data[root].valid)
				goto start;

		}

		return found;

	}
*/
public:

	Folding(
		FAE&           fae,
		BoxMan&        boxMan) :
		fae(fae),
		boxMan(boxMan),
		signatureMap(fae.getRootCount())
	{ }
};

#endif
