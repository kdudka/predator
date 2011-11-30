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

#include <ostream>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <cassert>

#include "types.hh"
#include "treeaut.hh"
#include "label.hh"
#include "abstractbox.hh"

#define _MSBM			((~(size_t)0) >> 1)
#define _MSB			(~_MSBM)
#define _MSB_TEST(x)	(x & _MSB)
#define _MSB_GET(x)		(x & _MSBM)
#define _MSB_ADD(x)		(x | _MSB)

class ConnectionGraph {

public:

	struct CutpointInfo {

		size_t root; // cutpoint number
		bool joint; // multiple times
		size_t forwardSelector; // lowest selector which reaches the given cutpoint
		size_t backwardSelector; // lowest selector of 'root' from which the state can be reached in the opposite direction
		std::set<size_t> defines; // set of selectors of the cutpoint hidden in the subtree (includes backwardSelector if exists)

		CutpointInfo(size_t root = 0) : root(root), joint(false), forwardSelector((size_t)(-1)),
			backwardSelector((size_t)(-1)) {}

		bool operator==(const CutpointInfo& rhs) const {

			return this->root == rhs.root &&
				this->joint == rhs.joint &&
				this->backwardSelector == rhs.backwardSelector &&
				this->forwardSelector == rhs.forwardSelector &&
				this->defines == rhs.defines;

		}

		bool operator%(const CutpointInfo& rhs) const {

			return this->root == rhs.root &&
				this->joint == rhs.joint &&
				this->backwardSelector == rhs.backwardSelector &&
				this->forwardSelector == rhs.forwardSelector &&
				this->defines == rhs.defines;

		}

		friend size_t hash_value(const CutpointInfo& info) {

			size_t seed = 0;
			boost::hash_combine(seed, info.root);
			boost::hash_combine(seed, info.joint);
			boost::hash_combine(seed, info.forwardSelector);
			boost::hash_combine(seed, info.backwardSelector);
			boost::hash_combine(seed, info.defines);
			return seed;

		}

		/**
		 * @todo improve signature printing
		 */
		friend std::ostream& operator<<(std::ostream& os, const CutpointInfo& info) {

			os << info.root << '(' << info.joint << ',';

			if (info.forwardSelector == (size_t)(-1))
				os << '-';
			else
				os << info.forwardSelector;

			os << ',';

			if (info.backwardSelector == (size_t)(-1))
				os << '-';
			else
				os << info.backwardSelector;

			os << ',';

			for (auto& s : info.defines)
				os << " +" << s;

			return os << ')';

		}

	};

	typedef std::vector<CutpointInfo> CutpointSignature;
/*
	friend bool operator==(const CutpointSignature& lhs, const CutpointSignature& rhs) {

		if (lhs.size() != rhs.size())
			return false;

		for (size_t i = 0; i < lhs.size(); ++i) {

			if (!lhs[i].operator==(rhs[i]))
				return false;

		}

		return true;

	}
*/
	friend bool operator%(const CutpointSignature& lhs, const CutpointSignature& rhs) {

		if (lhs.size() != rhs.size())
			return false;

		for (size_t i = 0; i < lhs.size(); ++i) {

			if (!lhs[i].operator%(rhs[i]))
				return false;

		}

		return true;

	}

	typedef std::unordered_map<size_t, CutpointSignature> StateToCutpointSignatureMap;

	friend std::ostream& operator<<(std::ostream& os, const CutpointSignature& signature) {

		for (auto& cutpoint : signature)
			os << cutpoint;

		return os;

	}

	struct RootInfo {

		bool valid;
		CutpointSignature signature;
		std::map<size_t, size_t> backwardMap;

		RootInfo() : valid(), signature(), backwardMap() {}

		size_t backwardLookup(size_t selector) const {

			auto iter = this->backwardMap.find(selector);

			assert(iter != this->backwardMap.end());

			return iter->second;

		}

		friend std::ostream& operator<<(std::ostream& os, const RootInfo& info) {

			if (!info.valid)
				return os << "<invalid>";

			os << info.signature;

			for (auto& p : info.backwardMap)
				os << '|' << p.first << ':' << p.second;

			return os;

		}

	};

	typedef std::vector<RootInfo> ConnectionData;

public:

	static bool isData(size_t state) { return _MSB_TEST(state); }

	static bool containsCutpoint(const CutpointSignature& signature, size_t target) {

		for (auto& cutpoint : signature) {

			if (cutpoint.root == target)
				return true;

		}

		return false;

	}
/*
	static bool containsCutpoints(const CutpointSignature& signature, const std::set<size_t>& cutpoints) {

		for (auto& cutpoint : signature) {

			if (cutpoints.count(cutpoint.root))
				return true;

		}

		return false;

	}
*/
	static size_t getSelectorToTarget(const CutpointSignature& signature, size_t target) {

		for (auto& cutpoint : signature) {

			if (cutpoint.root == target) {

				assert(cutpoint.forwardSelector != (size_t)(-1));

				return cutpoint.forwardSelector;

			}

		}

		return (size_t)(-1);

	}

	static void renameSignature(CutpointSignature& signature, const std::vector<size_t>& index) {

		for (auto& cutpoint : signature) {

			assert(cutpoint.root < index.size());

			cutpoint.root = index[cutpoint.root];

		}

	}

	static bool areDisjoint(const std::set<size_t>& s1, const std::set<size_t>& s2) {

		std::vector<size_t> v(s1.size());

		return std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), v.begin()) == v.begin();

	}
/*
	static bool isSubset(const std::set<size_t>& s1, const std::set<size_t>& s2) {

		return std::includes(s1.begin(), s1.end(), s2.begin(), s2.end());

	}
*/
	static void normalizeSignature(CutpointSignature& signature) {

		std::unordered_map<size_t, CutpointInfo*> m;

		size_t offset = 0;

		for (size_t i = 0; i < signature.size(); ++i) {

			auto p = m.insert(std::make_pair(signature[i].root, &signature[offset]));

			if (p.second)

				signature[offset++] = signature[i];

			else {

				assert(p.first->second);

				p.first->second->joint = true;

				if (p.first->second->forwardSelector > signature[i].forwardSelector)
					p.first->second->forwardSelector = signature[i].forwardSelector;

				if (p.first->second->backwardSelector > signature[i].backwardSelector)
					p.first->second->backwardSelector = signature[i].backwardSelector;

				assert(
					ConnectionGraph::areDisjoint(p.first->second->defines, signature[i].defines)
				);

				p.first->second->defines.insert(
					signature[i].defines.begin(), signature[i].defines.end()
				);

			}

		}

		signature.resize(offset);

	}

	static void updateStateSignature(StateToCutpointSignatureMap& stateMap, size_t state,
		const CutpointSignature& v) {

		assert((stateMap.find(state) == stateMap.end()) || (stateMap[state] == v));

		stateMap.insert(std::make_pair(state, v));

	}

	static void processStateSignature(CutpointSignature& result, StructuralBox* box, size_t input,
		size_t state, const CutpointSignature& signature) {

		size_t selector = box->outputReachable(input);

		if (ConnectionGraph::isData(state)) {

			if (signature.empty())
				return;

			assert(signature.size() == 1);

			result.push_back(signature[0]);
			result.back().forwardSelector = box->selectorToInput(input);

			if (selector != (size_t)(-1))
				result.back().backwardSelector = selector;

			result.back().defines.insert(
				box->inputCoverage(input).begin(), box->inputCoverage(input).end()
			);

			return;

		}

		for (auto& cutpoint : signature) {

			result.push_back(cutpoint);
			result.back().forwardSelector = box->selectorToInput(input);

			if (selector == (size_t)(-1))
				result.back().backwardSelector = (size_t)(-1);

		}

	}

	static bool processNode(CutpointSignature& result, const TT<label_type>& t,
		const StateToCutpointSignatureMap& stateMap) {

		size_t lhsOffset = 0;

		for (auto box : t.label()->getNode()) {

			if (!box->isStructural())
				continue;

			StructuralBox* sBox = (StructuralBox*)box;

			for (size_t j = 0; j < sBox->getArity(); ++j) {

				auto k = stateMap.find(t.lhs()[lhsOffset + j]);

				if (k == stateMap.end())
					return false;

				ConnectionGraph::processStateSignature(
					result, sBox, j, t.lhs()[lhsOffset + j], k->second
				);

			}

			lhsOffset += sBox->getArity();

		}

		return true;

	}

	// computes signature for all states of ta
	static void computeSignatures(StateToCutpointSignatureMap& stateMap, const TA<label_type>& ta) {

		stateMap.clear();

		std::vector<const TT<label_type>*> transitions;

		CutpointSignature v(1);

		for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {

			const Data* data;

			if (i->label()->isData(data)) {

				if (data->isRef()){

					v[0] = CutpointInfo(data->d_ref.root);

					ConnectionGraph::updateStateSignature(stateMap, i->rhs(), v);

				} else {

					assert(stateMap.find(i->rhs()) == stateMap.end());

					ConnectionGraph::updateStateSignature(stateMap, i->rhs(), CutpointSignature());

				}

			} else {

				transitions.push_back(&*i);

			}

		}

		bool changed = true;

		while (transitions.size()/* && changed*/) {

			assert(changed);

			changed = false;

			for (auto i = transitions.begin(); i != transitions.end(); ) {

				const TT<label_type>& t = **i;

				assert(t.label()->isNode());

				v.clear();

				if (!processNode(v, t, stateMap)) {

					++i;

					continue;

				}

				ConnectionGraph::normalizeSignature(v);

				ConnectionGraph::updateStateSignature(stateMap, t.rhs(), v);

				changed = true;

				i = transitions.erase(i);

			}

		}

	}

public:

	ConnectionData data;

	bool isValid() const {

		for (auto& root : this->data) {

			if (!root.valid)
				return false;

		}

		return true;

	}

	void updateIfNeeded(const std::vector<std::shared_ptr<TA<label_type>>>& roots) {

		assert(this->data.size() == roots.size());

		for (size_t i = 0; i < this->data.size(); ++i) {

			if (this->data[i].valid)
				continue;

			if (!roots[i]) {

				this->data[i].valid = true;

				continue;

			}

			this->updateRoot(i, *roots[i]);

		}

	}

	void clear() {

		this->data.clear();

	}

	void invalidate(size_t root) {

		assert(root < this->data.size());

		if (!this->data[root].valid)
			return;

		for (auto& cutpoint : this->data[root].signature) {

			assert(cutpoint.root < this->data.size());

			if (cutpoint.backwardSelector != (size_t)(-1)) {

				assert(this->data[cutpoint.root].backwardLookup(cutpoint.backwardSelector) == root);

				this->data[cutpoint.root].backwardMap.erase(cutpoint.backwardSelector);

			}

		}

		this->data[root].valid = false;

	}

	void updateBackwardData(size_t root) {

		assert(root < this->data.size());
		assert(!this->data[root].valid);

		for (auto& cutpoint : this->data[root].signature) {

			assert(cutpoint.root < this->data.size());

			if (cutpoint.backwardSelector != (size_t)(-1)) {

				assert(
					this->data[cutpoint.root].backwardMap.find(
						cutpoint.backwardSelector
					) == this->data[cutpoint.root].backwardMap.end()
				);

				this->data[cutpoint.root].backwardMap.insert(
					std::make_pair(cutpoint.backwardSelector, root)
				);

			}

		}

		this->data[root].valid = true;

	}

	void updateRoot(size_t root, const TA<label_type>& ta) {

		assert(root < this->data.size());
		assert(!this->data[root].valid);
		assert(ta.getFinalStates().size());

		StateToCutpointSignatureMap stateMap;

		ConnectionGraph::computeSignatures(stateMap, ta);

		auto iter = ta.getFinalStates().begin();

		assert(stateMap.find(*iter) != stateMap.end());

		this->data[root].signature = stateMap[*iter];

		for (++iter; iter != ta.getFinalStates().end(); ++iter) {

			assert(stateMap.find(*iter) != stateMap.end());
			assert(this->data[root].signature == stateMap[*iter]);

		}

		this->updateBackwardData(root);

	}

	void newRoot() {

		this->data.push_back(RootInfo());

	}

	bool hasReference(size_t root, size_t target) const {

		assert(root < this->data.size());
		assert(this->data[root].valid);

		return ConnectionGraph::containsCutpoint(this->data[root].signature, target);

	}

	void finishNormalization(size_t size, const std::vector<size_t>& index) {

		assert(size <= this->data.size());
		assert(index.size() == this->data.size());
		assert(this->isValid());

		ConnectionData tmp(size);

		for (size_t i = 0; i < this->data.size(); ++i) {

			if (index[i] == (size_t)(-1))
				continue;

			assert(index[i] < tmp.size());

			tmp[index[i]] = this->data[i];

		}

		std::swap(tmp, this->data);

		for (auto& root : this->data) {

			assert(root.valid);

			ConnectionGraph::renameSignature(root.signature, index);

			for (auto& selectorRootPair : root.backwardMap) {

				assert(selectorRootPair.second < index.size());

				selectorRootPair.second = index[selectorRootPair.second];

			}

		}

	}
/*
	bool isMergable(size_t dst, size_t src) {

		assert(dst < this->data.size());
		assert(src < this->data.size());

		assert(this->isValid());

		const CutpointSignature& dstSignature = this->data[dst].signature;
		const CutpointSignature& srcSignature = this->data[src].signature;

		assert(
			std::find_if(
				dstSignature.begin(),
				dstSignature.end(),
				[&src](const CutpointInfo& cutpoint) { return cutpoint.root == src; }
			) != dstSignature.end()
		);

		for (auto& cutpoint : dstSignature) {

			if (cutpoint.root != src)
				continue;

			if (cutpoint.backwardSelector == (size_t)(-1)) {

				for (auto& tmp : srcSignature) {

					if (tmp.backwardSelector != (size_t)(-1))
						return false;

				}

			}

		}

		return true;

	}
*/
	void mergeCutpoint(size_t dst, size_t src) {

		assert(dst < this->data.size());
		assert(src < this->data.size());

		assert(this->isValid());

		const CutpointSignature& dstSignature = this->data[dst].signature;
		const CutpointSignature& srcSignature = this->data[src].signature;

		assert(
			std::find_if(
				dstSignature.begin(),
				dstSignature.end(),
				[&src](const CutpointInfo& cutpoint) { return cutpoint.root == src; }
			) != dstSignature.end()
		);

		CutpointSignature signature;

		for (auto& cutpoint : dstSignature) {

			if (cutpoint.root != src) {

				// copy original signature
				signature.push_back(cutpoint);

				continue;

			}

			if (cutpoint.backwardSelector == (size_t)(-1)) {

				// erase backward selector
				for (auto& tmp : srcSignature) {

					signature.push_back(tmp);
					signature.back().forwardSelector = cutpoint.forwardSelector;

					if (tmp.backwardSelector == (size_t)(-1))
						continue;

					signature.back().backwardSelector = (size_t)(-1);

					assert(tmp.root < this->data.size());

					auto iter = this->data[tmp.root].backwardMap.find(tmp.backwardSelector);

					assert(iter != this->data[tmp.root].backwardMap.end());
					assert(iter->second == src);

					this->data[tmp.root].backwardMap.erase(iter);

				}

			} else {

				// update backward selector
				for (auto& tmp : srcSignature) {

					signature.push_back(tmp);
					signature.back().forwardSelector = cutpoint.forwardSelector;

					if (tmp.backwardSelector == (size_t)(-1))
						continue;

					assert(tmp.root < this->data.size());

					auto iter = this->data[tmp.root].backwardMap.find(tmp.backwardSelector);

					assert(iter != this->data[tmp.root].backwardMap.end());
					assert(iter->second == src);

					auto i = this->data[tmp.root].backwardMap.begin();

					for (; i != this->data[tmp.root].backwardMap.end(); ++i) {

						if (i->second == dst)
							break;

					}

					if (i == this->data[tmp.root].backwardMap.end()) {

						iter->second = dst;

						continue;

					}

					if (i->first < iter->first) {

						this->data[tmp.root].backwardMap.erase(iter);

						continue;

					}

					this->data[tmp.root].backwardMap.erase(i);

					iter->second = dst;

				}

			}

		}

		ConnectionGraph::normalizeSignature(signature);

		std::swap(this->data[dst].signature, signature);

	}

public:

	size_t climb(size_t c, const std::vector<bool>& visited, std::vector<bool>& mask) const {

		assert(c < this->data.size());

		if (mask[c])
			return c;

		if (this->data[c].backwardMap.empty())
			return c;

		mask[c] = true;

		for (auto& selectorCutpointPair : this->data[c].backwardMap) {

			assert(selectorCutpointPair.second < visited.size());

			if (!visited[selectorCutpointPair.second])
				return this->climb(selectorCutpointPair.second, visited, mask);

		}

		return c;

	}

	void visit(size_t c, std::vector<bool>& visited, std::vector<size_t>& order, std::vector<bool>& marked) const {

		assert(c < visited.size());

		if (visited[c]) {

			marked[c] = true;

			return;

		}

		visited[c] = true;

		order.push_back(c);

		for (auto& cutpoint : this->data[c].signature) {

			this->visit(cutpoint.root, visited, order, marked);

			if (cutpoint.joint)
				marked[cutpoint.root] = true;

		}

		for (auto& selectorCutpointPair : this->data[c].backwardMap) {

			if (visited[selectorCutpointPair.second])
				continue;

			std::vector<bool> mask(this->data.size(), false);

			mask[c] = true;

			size_t tmp = this->climb(selectorCutpointPair.second, visited, mask);

			if (tmp == c)
				continue;

			marked[tmp] = true;

			this->visit(tmp, visited, order, marked);

		}

	}

	void visit(size_t c, std::vector<bool>& visited) const {

		assert(c < visited.size());

		if (visited[c])
			return;

		visited[c] = true;

		for (auto& cutpoint : this->data[c].signature)
			this->visit(cutpoint.root, visited);

		for (auto& selectorCutpointPair : this->data[c].backwardMap) {

			if (visited[selectorCutpointPair.second])
				continue;

			std::vector<bool> mask(this->data.size(), false);

			mask[c] = true;

			size_t tmp = this->climb(selectorCutpointPair.second, visited, mask);

			if (tmp == c)
				continue;

			this->visit(tmp, visited);

		}

	}

public:

	ConnectionGraph(size_t size = 0) : data(size) {}

};

#endif
