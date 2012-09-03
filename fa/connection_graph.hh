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


#define _MSBM			((~static_cast<size_t>(0)) >> 1)
#define _MSB			(~_MSBM)
#define _MSB_TEST(x)	(x & _MSB)
#define _MSB_GET(x)		(x & _MSBM)
#define _MSB_ADD(x)		(x | _MSB)


class ConnectionGraph
{

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
#ifdef FA_TRACK_SELECTORS
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

	friend bool operator%(const CutpointSignature& lhs, const CutpointSignature& rhs)
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

	friend std::ostream& operator<<(std::ostream& os, const CutpointSignature& signature)
	{
		for (auto& cutpoint : signature)
			os << cutpoint;

		return os;
	}

	struct RootInfo {

		bool valid;
		CutpointSignature signature;
		std::map<size_t, size_t> bwdMap;

		RootInfo() : valid(), signature(), bwdMap() {}

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

	static bool containsCutpoint(const CutpointSignature& signature, size_t target)
	{
		for (auto& cutpoint : signature)
		{
			if (cutpoint.root == target)
				return true;
		}

		return false;
	}

	static size_t getSelectorToTarget(const CutpointSignature& signature, size_t target)
	{
		for (auto& cutpoint : signature)
		{
			if (cutpoint.root == target)
			{
				assert(!cutpoint.fwdSelectors.empty());

				return *cutpoint.fwdSelectors.begin();
			}
		}

		return static_cast<size_t>(-1);
	}

	static void renameSignature(CutpointSignature& signature,
		const std::vector<size_t>& index)
	{
		for (auto& cutpoint : signature)
		{
			assert(cutpoint.root < index.size());

			cutpoint.root = index[cutpoint.root];
		}
	}

	static bool areDisjoint(const std::set<size_t>& s1, const std::set<size_t>& s2)
	{
		std::vector<size_t> v(s1.size());

		return std::set_intersection(s1.begin(), s1.end(),
			s2.begin(), s2.end(), v.begin()) == v.begin();
	}

	static bool isSubset(const std::set<size_t>& s1, const std::set<size_t>& s2)
	{
		return std::includes(s1.begin(), s1.end(), s2.begin(), s2.end());
	}

	static void normalizeSignature(CutpointSignature& signature);

	static void updateStateSignature(StateToCutpointSignatureMap& stateMap, size_t state,
		const CutpointSignature& v);

	static void processStateSignature(CutpointSignature& result,
		const StructuralBox* box, size_t input, size_t state,
		const CutpointSignature& signature);

	static bool processNode(CutpointSignature& result, const std::vector<size_t>& lhs,
		const label_type& label, const StateToCutpointSignatureMap& stateMap);

	// computes signature for all states of ta
	static void computeSignatures(StateToCutpointSignatureMap& stateMap,
		const TreeAut& ta);

	// computes signature for all states of ta
	static void fixSignatures(TreeAut& dst, const TreeAut& ta, size_t& offset);

public:

	ConnectionData data;

	bool isValid() const
	{
		for (auto& root : this->data)
		{
			if (!root.valid)
				return false;
		}

		return true;
	}

	void updateIfNeeded(const std::vector<std::shared_ptr<TreeAut>>& roots);

	void clear()
	{
		this->data.clear();
	}

	void invalidate(size_t root);

	void updateBackwardData(size_t root);

	void updateRoot(size_t root, const TreeAut& ta);

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

public:

	size_t climb(size_t c, const std::vector<bool>& visited,
		std::vector<bool>& mask) const;

	void visit(size_t c, std::vector<bool>& visited, std::vector<size_t>& order,
		std::vector<bool>& marked) const;

	void visit(size_t c, std::vector<bool>& visited) const;

public:

	ConnectionGraph(size_t size = 0) : data(size) {}

	void getRelativeSignature(std::vector<std::pair<int, size_t>>& signature,
		size_t root) const
	{
		signature.clear();

		for (auto& tmp : this->data[root].signature)
			signature.push_back(std::make_pair(tmp.root - root, tmp.refCount));
	}
};

#endif
