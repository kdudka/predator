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
	typedef std::shared_ptr<TreeAut> TreeAutShPtr;

private:  // data members

	FAE& fae_;
	BoxMan& boxMan_;

	std::vector<std::pair<bool, ConnectionGraph::StateToCutpointSignatureMap>> signatureMap_;

protected:

	/**
	 * @brief  Copies a box and corresponding states into a label
	 *
	 * This static method copies a desired box into an output label, together with
	 * corresponding states.
	 *
	 * @param[out]  lhs       The target vector where the states that correspond
	 *                        to the box are to be appended
	 * @param[out]  label     The label into which the box is to be copied
	 * @param[in]   box       The box to be copied
	 * @param[in]   srcLhs    The source vector of states from which the relevant
	 *                        one will be picked to be put into @p lhs
	 * @param[in]   srcOffset The offset of the first state in @p srcLhs
	 *                        corresponding to @p box
	 */
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

	/**
	 * @brief  Given a map of signatures, gets the signature of a particular state
	 *
	 * This static method is given a map of signatures and it returns the
	 * signature of a particular state.
	 *
	 * @param[in]  state       The state the signature of which is desired
	 * @param[in]  signatures  The map of signatures
	 *
	 * @returns  The signature of @p state
	 */
	static const ConnectionGraph::CutpointSignature& getSignature(
		size_t                                                 state,
		const ConnectionGraph::StateToCutpointSignatureMap&    signatures)
	{
		auto iter = signatures.find(state);

		assert(iter != signatures.end());

		return iter->second;
	}


	/**
	 * @brief  Returns valid signatures for given root
	 *
	 * This method returns valid signatures for given root.
	 *
	 * @param[in]  root  The root the signatures of which are to be obtained
	 *
	 * @returns  Signatures of @p root
	 */
	const ConnectionGraph::StateToCutpointSignatureMap& getSignatures(
		size_t        root);


	/**
	 * @brief  Invalidates the signature of given root
	 *
	 * Invalidates the signature of given root.
	 *
	 * @param[in]  root  Index of the root to be invalidated
	 */
	void invalidateSignatures(size_t root)
	{
		// Preconditions
		assert(root < signatureMap_.size());

		signatureMap_[root].first = false;
	}


	/**
	 * @brief  Splits a tree automaton into two tree automata
	 *
	 * This method splits a tree automaton at index @p root in the FA into two
	 * tree automata at given @p state at the selectors that have the @p target
	 * component in their signature.
	 *
	 * @param[out]  res                  The resulting automaton @p root without
	 *                                   the part under @p state going to
	 *                                   reference @p target
	 * @param[out]  complement           The tree automaton for the part under
	 *                                   @p state going to reference @p target
	 * @param[out]  complementSignature  Signature of @p complement
	 * @param[in]   root                 Index of the source tree automaton
	 * @param[in]   state                State in the tree automaton at @p root
	 * @param[in]   target               Index of the target tree automaton, the
	 *                                   references to which are to be found
	 */
	void componentCut(
		TreeAut&                                 res,
		TreeAut&                                 complement,
		ConnectionGraph::CutpointSignature&      complementSignature,
		size_t                                   root,
		size_t                                   state,
		size_t                                   target);


	/**
	 * @brief  Splits a tree automaton under given state leading to a cutpoint
	 *
	 * This method splits a tree automaton at index @p root at the FA under @p state
	 * in the path leading to the @p cutpoint and returns the split part together
	 * with the original automaton without the split part.
	 *
	 * @param[out]  boxSignature  Signature of the split part
	 * @param[in]   root          Index of the tree automaton where the splitting
	 *                            will occur
	 * @param[in]   state         State in the TA at @p root under which the
	 *                            splitting will occur
	 * @param[in]   cutpoint      Index of the component such that the split path
	 *                            goes to the reference to this component
	 *
	 * @returns  The pair of tree automata such that the first tree automaton is
	 *           the TA at index @p root in the FA without the split part and the
	 *           second automaton is the split part. Together, they represent the
	 *           original TA
	 */
	std::pair<TreeAutShPtr, TreeAutShPtr> separateCutpoint(
		ConnectionGraph::CutpointSignature&            boxSignature,
		size_t                                         root,
		size_t                                         state,
		size_t                                         cutpoint);


	/**
	 * @brief  Relabels references in given tree automaton
	 *
	 * Relabels references in the tree automaton @p ta according to the @p index.
	 *
	 * @param[in]  ta     The tree automaton where the references are to be
	 *                    relabelled 
	 * @param[in]  index  The index according to which the references are to be
	 *                    relabelled
	 *
	 * @returns  The tree automaton with relabelled references
	 */
	TreeAutShPtr relabelReferences(
		const TreeAut&                     ta,
		std::vector<size_t>&               index)
	{
		TreeAutShPtr tmp = TreeAutShPtr(fae_.allocTA());

		fae_.relabelReferences(*tmp, ta, index);

		return tmp;
	}


	/**
	 * @brief  @todo
	 */
	TreeAutShPtr joinBox(
		const TreeAut&                               src,
		size_t                                       state,
		size_t                                       root,
		const Box*                                   box,
		const ConnectionGraph::CutpointSignature&    signature);


	/**
	 * @brief  Checks whether a selector map is correct for a state
	 *
	 * This method checks whether the provided cutpoint-to-selector map @p
	 * selectorMap is correct for the @p state in the tree automaton at the index
	 * @p root in the FA, i.e. whether all transitions respect this map.
	 *
	 * @param[in]  selectorMap  The cutpoint-to-selector map
	 * @param[in]  root         The index of the tree automaton in FA
	 * @param[in]  state        The state in the tree automaton at index @p root
	 *
	 * @returns  @p true if all transition from @p state respect the
	 *           cutpoint-to-selector map @p selectorMap, @p false otherwise
	 */
	bool checkSelectorMap(
		const std::unordered_map<size_t, size_t>&     selectorMap,
		size_t                                        root,
		size_t                                        state);


	/**
	 * @brief  Computes cutpoint-to-selector mapping
	 *
	 * This function computes for a @p state in the tree automaton at index @p
	 * root in the FA the cutpoint-to-selector mapping, i.e. a map telling for
	 * a given cutpoint which selector (or, more precisely, the selector at which
	 * @e offset) one needs to take in order to reach the cutpoint.
	 *
	 * @param[out]  selectorMap  The resulting cutpoint-to-selector map
	 * @param[in]   root         The index of the tree automaton in the FA
	 * @param[in]   state        The state in the automaton at index @p root for
	 *                           which the selector map is to be obtained
	 *
	 * @returns  @p true if all transitions from @p state respect the output
	 *           cutpoint-to-selector map @p selectorMap, @p false otherwise
	 */
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

	const Box* getBox(const Box& box, bool conditional)
	{
		return (conditional)?(boxMan_.lookupBox(box)):(boxMan_.getBox(box));
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

public:

	Folding(
		FAE&           fae,
		BoxMan&        boxMan) :
		fae_(fae),
		boxMan_(boxMan),
		signatureMap_(fae.getRootCount())
	{ }
};

#endif
