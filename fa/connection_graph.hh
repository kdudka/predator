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

#ifndef CONNECTION_GRAPH_H
#define CONNECTION_GRAPH_H

// Standard library headers
#include <ostream>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <cassert>

// Forester headers
#include "types.hh"
#include "treeaut_label.hh"
#include "abstractbox.hh"
#include "config.h"


#define _MSBM         ((~static_cast<size_t>(0)) >> 1)
#define _MSB          (~_MSBM)
#define _MSB_TEST(x)  (x & _MSB)
#define _MSB_GET(x)   (x & _MSBM)
#define _MSB_ADD(x)   (x | _MSB)


class ConnectionGraph
{
private:  // data type

	typedef TreeAut::Transition Transition;

public:

	struct CutpointInfo
	{
		/// cutpoint number
		size_t root;

		/// number of references
		size_t refCount;

		/// number of real references
		size_t selCount;

		/// inherited from different state
		bool refInherited;

		/// a set of selectors which reach the given cutpoint
		std::set<size_t> fwdSelectors;

		/// lowest selector of 'root' from which the state can be reached in the
		/// opposite direction
		size_t bwdSelector;

		/// set of selectors of the cutpoint hidden in the subtree (includes
		/// backwardSelector if exists)
		std::set<size_t> defines;

		CutpointInfo(size_t root = 0) :
			root(root),
			refCount(1),
			selCount(1),
			refInherited(false),
			fwdSelectors(),
			bwdSelector(static_cast<size_t>(-1)),
			defines{}
		{
			this->fwdSelectors.insert(static_cast<size_t>(-1));
		}

		bool operator==(const CutpointInfo& rhs) const
		{
			// Assertions
			assert(!this->fwdSelectors.empty());
			assert(!rhs.fwdSelectors.empty());

			return this->root == rhs.root &&
				this->refCount == rhs.refCount &&
				this->selCount == rhs.selCount &&
				*this->fwdSelectors.begin() == *rhs.fwdSelectors.begin() &&
				this->bwdSelector == rhs.bwdSelector &&
				this->defines == rhs.defines;
		}

		bool operator%(const CutpointInfo& rhs) const
		{
			return this->root == rhs.root &&
				this->refCount == rhs.refCount &&
#if FA_TRACK_SELECTORS
				this->selCount == rhs.selCount &&
#endif
//				this->fwdSelectors == rhs.fwdSelectors &&
				this->bwdSelector == rhs.bwdSelector &&
				this->defines == rhs.defines;
		}

		friend size_t hash_value(const CutpointInfo& info)
		{
			// Assertions
			assert(!info.fwdSelectors.empty());

			size_t seed = 0;
			boost::hash_combine(seed, info.root);
			boost::hash_combine(seed, info.refCount);
			boost::hash_combine(seed, info.selCount);
			boost::hash_combine(seed, *info.fwdSelectors.begin());
			boost::hash_combine(seed, info.bwdSelector);
			boost::hash_combine(seed, info.defines);
			return seed;
		}

		/**
		 * @todo improve signature printing
		 */
		friend std::ostream& operator<<(std::ostream& os, const CutpointInfo& info);
	};

	typedef std::vector<CutpointInfo> CutpointSignature;

	friend bool operator%(
		const CutpointSignature&     lhs,
		const CutpointSignature&     rhs)
	{
		if (lhs.size() != rhs.size())
			return false;

		for (size_t i = 0; i < lhs.size(); ++i)
		{
			if (!lhs[i].operator%(rhs[i]))
				return false;
		}

		return true;
	}

	typedef std::unordered_map<size_t, CutpointSignature> StateToCutpointSignatureMap;

	friend std::ostream& operator<<(
		std::ostream&               os,
		const CutpointSignature&    signature)
	{
		for (auto& cutpoint : signature)
			os << cutpoint;

		return os;
	}

	struct RootInfo
	{
		bool valid;
		CutpointSignature signature;
		std::map<size_t, size_t> bwdMap;

		RootInfo() :
			valid(),
			signature(),
			bwdMap()
		{ }

		size_t backwardLookup(size_t selector) const
		{
			auto iter = this->bwdMap.find(selector);

			assert(iter != this->bwdMap.end());

			return iter->second;
		}

		friend std::ostream& operator<<(std::ostream& os, const RootInfo& info);

		bool operator==(const RootInfo& rhs) const
		{
			if (!this->valid || !rhs.valid)
				return false;

			return this->signature == rhs.signature;
		}
	};

	typedef std::vector<RootInfo> ConnectionData;

public:

	static bool isData(size_t state) { return _MSB_TEST(state); }

	static bool containsCutpoint(
		const CutpointSignature&      signature,
		size_t                        target)
	{
		for (const CutpointInfo& cutpoint : signature)
		{
			if (cutpoint.root == target)
				return true;
		}

		return false;
	}


	/**
	 * @brief  Retrieves the selector going to a target
	 *
	 * Given a @p signature and a @p target cutpoint, this static method retrieves
	 * @p signature the offset of the lowest forward selector leading to @p target.
	 *
	 * @param[in]  signature  The signature of a state
	 * @param[in]  target     Index of the target cutpoint
	 *
	 * @returns  Offset of the lowest forward selector to @p target if there is
	 *           some, -1 otherwise
	 */
	static size_t getSelectorToTarget(
		const CutpointSignature&     signature,
		size_t                       target)
	{
		for (const CutpointInfo& cutpoint : signature)
		{
			if (cutpoint.root == target)
			{
				assert(!cutpoint.fwdSelectors.empty());

				return *cutpoint.fwdSelectors.begin();
			}
		}

		return static_cast<size_t>(-1);
	}


	static void renameSignature(
		CutpointSignature&           signature,
		const std::vector<size_t>&   index)
	{
		for (CutpointInfo& cutpoint : signature)
		{
			assert(cutpoint.root < index.size());

			cutpoint.root = index[cutpoint.root];
		}
	}

	static bool areDisjoint(
		const std::set<size_t>&    s1,
		const std::set<size_t>&    s2)
	{
		std::vector<size_t> v(s1.size());

		return std::set_intersection(s1.begin(), s1.end(),
			s2.begin(), s2.end(), v.begin()) == v.begin();
	}

	static bool isSubset(const std::set<size_t>& s1, const std::set<size_t>& s2)
	{
		return std::includes(s1.begin(), s1.end(), s2.begin(), s2.end());
	}


	/**
	 * @brief  Normalizes a signature by merging a root's records
	 *
	 * This function normalizes @p signature, i.e. it takes a look at all
	 * cutpoint information present and in case there are more elements for
	 * a single root, it merges them together into one record.
	 *
	 * @param[in,out]  signature  The signature to be normalized
	 */
	static void normalizeSignature(CutpointSignature& signature);


	/**
	 * @brief  Updates a state's signature
	 *
	 * This function updates a @p state's signature in the @p stateMap with the
	 * cutpoint signature given in @p v. In case @p state is not mapped to
	 * anything, the sole signature is inserted in the map, in the other case
	 * (there is already a signature for the state), the existing signature is
	 * altered.
	 *
	 * @param[in,out]  stateMap  The map of states to signatures
	 * @param[in]      state     The state the signature of which is to be
	 *                           updated
	 * @param[in]      v         The signature to be added to the state
	 */
	static void updateStateSignature(
		StateToCutpointSignatureMap&      stateMap,
		size_t                            state,
		const CutpointSignature&          v);


	/**
	 * @brief  Processes signature of a state
	 *
	 * This function processes signature of @p state. 
	 *
	 * @param[out]  result     Output signature of the state
	 * @param[in]   box        The box where the state is the @p input's selector
	 * @param[in]   input      The selector of the box corresponding to @p state
	 * @param[in]   state      The state
	 * @param[in]   signature  Previously computed signature
	 */
	static void processStateSignature(
		CutpointSignature&          result,
		const StructuralBox*        box,
		size_t                      input,
		size_t                      state,
		const CutpointSignature&    signature);


	/**
	 * @brief  Processes a label, i.e. a memory node
	 *
	 * This function processes @p label (a memory node in the heap terminology).
	 * It merges information stored in @p stateMap from downward states given in
	 * @p lhs into @p result. In case there is a state in @p lhs such that it has
	 * no corresponding record in @p stateMap, the function returns @p false,
	 * otherwise (everything was OK) it returns @p true. Note that the content of
	 * @p result is undefined for the case when the function returns @p false.
	 *
	 * @param[out]  result    Signature of the label
	 * @param[in]   lhs       The tuple of downward states
	 * @param[in]   label     The label
	 * @param[in]   stateMap  Mapping of states to cutpoint signatures
	 *
	 * @returns  @p true if the update was OK (and @p result is valid), @p false
	 *           in the case some downward state signature is missing (@p result
	 *           is invalid in this case)
	 */
	static bool processNode(
		CutpointSignature&                    result,
		const std::vector<size_t>&            lhs,
		const label_type&                     label,
		const StateToCutpointSignatureMap&    stateMap);

	// computes signature for all states of ta
	static void computeSignatures(
		StateToCutpointSignatureMap&    stateMap,
		const TreeAut&                  ta);

	// TODO: I don't know what this method does
	// (computes signature for all states of ta) ???
	static void fixSignatures(
		TreeAut&           dst,
		const TreeAut&     ta,
		size_t&            offset);

public:

	ConnectionData data;

	bool isValid() const
	{
		for (const RootInfo& root : this->data)
		{
			if (!root.valid)
			{
				return false;
			}
		}

		return true;
	}


	/**
	 * @brief  Updates the needed components of the connection graph
	 *
	 * This method updates the components of the connection graph that need an
	 * update.
	 *
	 * @param[in]  roots  The tree automata from the forest automaton
	 */
	void updateIfNeeded(
		const std::vector<std::shared_ptr<TreeAut>>& roots);


	void clear()
	{
		this->data.clear();
	}

	void invalidate(size_t root);

	void updateBackwardData(size_t root);

	/**
	 * @brief  Updates the info about a single root
	 *
	 * This method updates information about a single root of the forest.
	 *
	 * @param[in]  root  Index of the tree automaton in the forest automaton
	 * @param[in]  ta    The tree automaton at the index @p root
	 */
	void updateRoot(
		size_t              root,
		const TreeAut&      ta);

	void newRoot()
	{
		this->data.push_back(RootInfo());
	}

	bool hasReference(size_t root, size_t target) const
	{
		// Assertions
		assert(root < this->data.size());
		assert(this->data[root].valid);

		return ConnectionGraph::containsCutpoint(this->data[root].signature, target);
	}

	void finishNormalization(size_t size, const std::vector<size_t>& index);

	void mergeCutpoint(size_t dst, size_t src);

public:   // methods

	size_t climb(
		size_t                           c,
		const std::vector<bool>&         visited,
		std::vector<bool>&               mask) const;

	/**
	 * @brief  Computes the order of components in a FA
	 *
	 * Traverses the root interconnection graph from given root and computes the
	 * order of components in a FA according to the depth-first traversal. Note
	 * that only the root interconnection graph (based on signatures of
	 * rootpoints) is traversed and not the actual tree automata.
	 *
	 * @param[in]      c        The index of the component to be traversed
	 * @param[in,out]  visited  Vector where visited roots are set to true
	 * @param[out]     order    To be filled with the order of root points
	 * @param[out]     marked   Vector where roots referenced more than once are
	 *                          set to true
	 */
	void visit(
		size_t                       c,
		std::vector<bool>&           visited,
		std::vector<size_t>&         order,
		std::vector<bool>&           marked) const;


	void visit(
		size_t                       c,
		std::vector<bool>&           visited) const;


	ConnectionGraph(size_t size = 0) :
		data(size)
	{ }

	void getRelativeSignature(
		std::vector<std::pair<int, size_t>>&    signature,
		size_t                                  root) const
	{
		signature.clear();

		for (const CutpointInfo& tmp : this->data[root].signature)
			signature.push_back(std::make_pair(tmp.root - root, tmp.refCount));
	}
};

#endif
