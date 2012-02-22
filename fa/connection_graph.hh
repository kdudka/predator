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

#include "config.h"

#define _MSBM			((~(size_t)0) >> 1)
#define _MSB			(~_MSBM)
#define _MSB_TEST(x)	(x & _MSB)
#define _MSB_GET(x)		(x & _MSBM)
#define _MSB_ADD(x)		(x | _MSB)

class ConnectionGraph {

public:

	struct CutpointInfo {

		size_t root; // cutpoint number
		size_t refCount; // number of references
		bool refInherited; // inherited from different state
//		size_t forwardSelector; // lowest selector which reaches the given cutpoint
		std::set<size_t> fwdSelectors; // a set of selectors which reach the given cutpoint
		size_t bwdSelector; // lowest selector of 'root' from which the state can be reached in the opposite direction
		std::set<size_t> defines; // set of selectors of the cutpoint hidden in the subtree (includes backwardSelector if exists)

		CutpointInfo(size_t root = 0) : root(root), /*joint(false), joinInherited(false),*/
			refCount(1), refInherited(false),
			fwdSelectors(), bwdSelector((size_t)(-1)) {

			this->fwdSelectors.insert((size_t)(-1));

		}

		bool operator==(const CutpointInfo& rhs) const {

			assert(!this->fwdSelectors.empty());
			assert(!rhs.fwdSelectors.empty());

			return this->root == rhs.root &&
				this->refCount == rhs.refCount &&
				*this->fwdSelectors.begin() == *rhs.fwdSelectors.begin() &&
				this->bwdSelector == rhs.bwdSelector &&
				this->defines == rhs.defines;

		}

		bool operator%(const CutpointInfo& rhs) const {

			return this->root == rhs.root &&
				this->refCount == rhs.refCount &&
//				this->fwdSelectors == rhs.fwdSelectors &&
				this->bwdSelector == rhs.bwdSelector &&
				this->defines == rhs.defines;

		}

		friend size_t hash_value(const CutpointInfo& info) {

			assert(!info.fwdSelectors.empty());

			size_t seed = 0;
			boost::hash_combine(seed, info.root);
			boost::hash_combine(seed, info.refCount);
			boost::hash_combine(seed, *info.fwdSelectors.begin());
			boost::hash_combine(seed, info.bwdSelector);
			boost::hash_combine(seed, info.defines);
			return seed;

		}

		/**
		 * @todo improve signature printing
		 */
		friend std::ostream& operator<<(std::ostream& os, const CutpointInfo& info) {

			os << info.root << "x" << info.refCount << "({";

			for (auto& s : info.fwdSelectors) {

				if (s == (size_t)(-1))
					continue;

				os << ' ' << s;

			}

			os << " }, ";

			if (info.bwdSelector == (size_t)(-1))
				os << '-';
			else
				os << info.bwdSelector;

			os << ", {";

			for (auto& s : info.defines)
				os << " +" << s;

			return os << " })";

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
		std::map<size_t, size_t> bwdMap;

		RootInfo() : valid(), signature(), bwdMap() {}

		size_t backwardLookup(size_t selector) const {

			auto iter = this->bwdMap.find(selector);

			assert(iter != this->bwdMap.end());

			return iter->second;

		}

		friend std::ostream& operator<<(std::ostream& os, const RootInfo& info) {

			if (!info.valid)
				return os << "<invalid>";

			os << info.signature;

			for (auto& p : info.bwdMap)
				os << '|' << p.first << ':' << p.second;

			return os;

		}

		bool operator==(const RootInfo& rhs) const {

			if (!this->valid || !rhs.valid)
				return false;

			return this->signature == rhs.signature;

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

				assert(!cutpoint.fwdSelectors.empty());

				return *cutpoint.fwdSelectors.begin();

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

	static bool isSubset(const std::set<size_t>& s1, const std::set<size_t>& s2) {

		return std::includes(s1.begin(), s1.end(), s2.begin(), s2.end());

	}

	static void normalizeSignature(CutpointSignature& signature) {

		std::unordered_map<size_t, CutpointInfo*> m;

		size_t offset = 0;

		for (size_t i = 0; i < signature.size(); ++i) {

			auto p = m.insert(std::make_pair(signature[i].root, &signature[offset]));

			if (p.second) {

				signature[offset] = signature[i];
				signature[offset].refInherited = signature[offset].refCount > 1;

				++offset;

			} else {

				assert(p.first->second);

				p.first->second->refCount = std::min(
					p.first->second->refCount + signature[i].refCount, (size_t)FA_REF_CNT_TRESHOLD
				);
				p.first->second->refInherited = false;
				p.first->second->fwdSelectors.insert(
					signature[i].fwdSelectors.begin(), signature[i].fwdSelectors.end()
				);

				if (p.first->second->bwdSelector > signature[i].bwdSelector)
					p.first->second->bwdSelector = signature[i].bwdSelector;

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
/*
		assert((stateMap.find(state) == stateMap.end()) || (stateMap[state] == v));

		stateMap.insert(std::make_pair(state, v));
*/
 		auto p = stateMap.insert(std::make_pair(state, v));

		if (!p.second) {

			assert(v.size() == p.first->second.size());

			for (size_t i = 0; i < v.size(); ++i) {

				assert(v[i].root == p.first->second[i].root);
//				assert(*v[i].fwdSelectors.begin() == *p.first->second[i].fwdSelectors.begin());
				assert(v[i].defines == p.first->second[i].defines);

				p.first->second[i].refCount = std::max(p.first->second[i].refCount, v[i].refCount);

				p.first->second[i].fwdSelectors.insert(v[i].fwdSelectors.begin(), v[i].fwdSelectors.end());

			}

		}

	}
/*
	static void updateStateSignature(StateToCutpointSignatureMap& stateMap,
		StateCutpointSignatureToStateMap& signatureMap,
		std::unordered_map<size_t, size_t>& invSignatureMap, TA<label_type>& ta,
		std::list<const TT<label_type>*>& transitions, size_t& offset, const TT<label_type>& t,
		const CutpointSignature& v) {

 		auto p = stateMap.insert(std::make_pair(t.rhs(), v));

		if (p.second)
			return;

		auto tmp = p.first->second;

		bool broken = (v.size() != tmp.size());

		if (!broken) {

			size_t i;

			for (i = 0; i < v.size(); ++i) {

				if (v[i].root != tmp[i].root)
					break;

				if (v[i].defines != tmp[i].defines)
					break;

				if (v[i].joint)
					tmp[i].joint = true;

				tmp[i].fwdSelectors.insert(v[i].fwdSelectors.begin(), v[i].fwdSelectors.end());

			}

			broken = (i == v.size());

		}

		if (!broken) {

			p.first->second = tmp;

			return;

		}

		p = stateMap.insert(std::make_pair(offset, v));

		assert(p.second);

		auto stateCutpointStatePairIterator = signatureMap.insert(std::make_pair(std::make_pair(t.rhs(), v), offset));

		transitions.push_back(&ta.addTransition(t.lhs(), t.label(), stateCutpointStatePairIterator.first->second)->first);

		if (!stateCutpointStatePairIterator.second)
			return;

		if (ta.isFinalState(t.rhs()))
			ta.addFinalState(offset);

		++offset;

		std::vector<const TT<label_type>*> tlist;

		for (auto i = ta.begin(); i != ta.end(); ++i) {

			for (auto& s : i->lhs()) {

				if (s == t.rhs()) {

					tlist.push_back(&*i);

					break;

				}

			}

		}

		for (auto& nt : tlist) {

			std::vector<size_t> lhs = nt->lhs();

			for (auto& s : lhs) {

				if (s == t.rhs())
					s = stateCutpointStatePairIterator.first->second;

			}

			transitions.push_back(&ta.addTransition(lhs, nt->label(), nt->rhs())->first);

		}

	}
*/
	static void processStateSignature(CutpointSignature& result, StructuralBox* box, size_t input,
		size_t state, const CutpointSignature& signature) {

		size_t selector = box->outputReachable(input);

		if (ConnectionGraph::isData(state)) {

			if (signature.empty())
				return;

			assert(signature.size() == 1);
			assert(signature[0].fwdSelectors.size() == 1);
			assert(*signature[0].fwdSelectors.begin() == (size_t)(-1));

			result.push_back(signature[0]);
			result.back().fwdSelectors.insert(box->selectorToInput(input));

			if (selector != (size_t)(-1))
				result.back().bwdSelector = selector;

			result.back().defines.insert(
				box->inputCoverage(input).begin(), box->inputCoverage(input).end()
			);

			return;

		}

		for (auto& cutpoint : signature) {

			result.push_back(cutpoint);
			result.back().fwdSelectors.clear();
			result.back().fwdSelectors.insert(box->selectorToInput(input));

			if (selector == (size_t)(-1))
				result.back().bwdSelector = (size_t)(-1);

		}

	}

	static bool processNode(CutpointSignature& result, const std::vector<size_t>& lhs,
		const label_type& label, const StateToCutpointSignatureMap& stateMap) {

		size_t lhsOffset = 0;

		for (auto box : label->getNode()) {

			if (!box->isStructural())
				continue;

			StructuralBox* sBox = (StructuralBox*)box;

			for (size_t j = 0; j < sBox->getArity(); ++j) {

				auto k = stateMap.find(lhs[lhsOffset + j]);

				if (k == stateMap.end())
					return false;

				ConnectionGraph::processStateSignature(
					result, sBox, j, lhs[lhsOffset + j], k->second
				);

			}

			lhsOffset += sBox->getArity();

		}

		return true;

	}

	// computes signature for all states of ta
	static void computeSignatures(StateToCutpointSignatureMap& stateMap, const TA<label_type>& ta) {

		stateMap.clear();

		std::list<const TT<label_type>*> transitions;

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

			if (!changed)
				assert(false);      // fail gracefully

			changed = false;

			for (auto i = transitions.begin(); i != transitions.end(); ) {

				const TT<label_type>& t = **i;

				assert(t.label()->isNode());

				v.clear();

				if (!processNode(v, t.lhs(), t.label(), stateMap)) {

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

	// computes signature for all states of ta
	static void fixSignatures(TA<label_type>& dst, const TA<label_type>& ta, size_t& offset) {

		typedef std::pair<size_t, CutpointSignature> StateCutpointSignaturePair;
		typedef std::unordered_map<StateCutpointSignaturePair, size_t, boost::hash<StateCutpointSignaturePair>> StateCutpointSignatureToStateMap;

		StateToCutpointSignatureMap stateMap;

		StateCutpointSignatureToStateMap signatureMap;

		typedef std::vector<CutpointSignature> CutpointSignatureList;

		std::unordered_map<size_t, CutpointSignatureList> invSignatureMap;

		struct ChoiceElement {

			CutpointSignatureList::const_iterator begin;
			CutpointSignatureList::const_iterator end;
			CutpointSignatureList::const_iterator iter;

			ChoiceElement(
				CutpointSignatureList::const_iterator begin,
				CutpointSignatureList::const_iterator end
			) : begin(begin), end(end), iter(begin) {}

		};

		typedef std::vector<ChoiceElement> ChoiceType;

		struct NextChoice {
			bool operator()(ChoiceType& choice) const {
				auto iter = choice.begin();
				for (; (iter != choice.end()) && (++(iter->iter) == iter->end); ++iter) {
					iter->iter = iter->begin;
				}
				return iter != choice.end();
			}
		};

		std::vector<const TT<label_type>*> transitions;

		CutpointSignature v;

		for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {

			const Data* data;

			if (i->label()->isData(data)) {

				if (data->isRef()) {

					v = { CutpointInfo(data->d_ref.root) };

				} else {

					v.clear();

				}

				auto p = signatureMap.insert(std::make_pair(std::make_pair(i->rhs(), v), i->rhs()));

				if (!p.second){
					assert(false);
				}

				invSignatureMap.insert(
					std::make_pair(i->rhs(), std::vector<CutpointSignature>())
				).first->second.push_back(v);

				ConnectionGraph::updateStateSignature(stateMap, i->rhs(), v);

				dst.addTransition(*i);

			} else {

				transitions.push_back(&*i);

			}

		}

		bool changed = true;

		while (changed) {

			changed = false;

			for (auto i = transitions.begin(); i != transitions.end(); ++i) {

				const TT<label_type>& t = **i;

				assert(t.label()->isNode());

				ChoiceType choice;

				size_t i;

				for (i = 0; i < t.lhs().size(); ++i) {

					auto iter = invSignatureMap.find(t.lhs()[i]);

					if (iter == invSignatureMap.end())
						break;

					assert(iter->second.begin() != iter->second.end());

					choice.push_back(ChoiceElement(iter->second.begin(), iter->second.end()));

				}

				if (i < t.lhs().size())
					continue;

				std::vector<std::pair<size_t, CutpointSignature>> buffer;

				do {

					std::vector<size_t> lhs(t.lhs().size());

					for (size_t i = 0; i < t.lhs().size(); ++i) {

						assert(i < choice.size());

						auto iter = signatureMap.find(std::make_pair(t.lhs()[i], *choice[i].iter));

						assert(iter != signatureMap.end());

						lhs[i] = iter->second;

					}

					v.clear();

					if (!processNode(v, lhs, t.label(), stateMap)) {
						assert(false);
					}

					ConnectionGraph::normalizeSignature(v);

					auto p = signatureMap.insert(std::make_pair(std::make_pair(t.rhs(), v), offset));

					if (p.second) {

						++offset;

						if (ta.isFinalState(t.rhs()))
							dst.addFinalState(p.first->second);

						ConnectionGraph::updateStateSignature(stateMap, p.first->second, v);

						buffer.push_back(std::make_pair(t.rhs(), v));

					}

					dst.addTransition(lhs, t.label(), p.first->second);

				} while (NextChoice()(choice));

				for (auto& item : buffer) {

					invSignatureMap.insert(
						std::make_pair(item.first, std::vector<CutpointSignature>())
					).first->second.push_back(item.second);

				}

				changed = changed || buffer.size();

			}

		}

	}

/*
	static void computeSplitPoints(std::unordered_set<size_t>& splitPoints, const TA<label_type>& ta) {

		std::unordered_map<size_t, std::unordered_set<std::set<size_t>>> stateInfo;

		std::vector<const TT<label_type>*> transitions;

		splitPoints.clear();

		for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {

			if (i->lhs().empty()) {

				stateInfo.insert(
					std::make_pair(i->rhs(), std::unordered_set<std::set<size_t>>())
				).first->second.insert(std::set<size_t>());

			} else {

				transitions.push_back(&*i);

				splitPoints.insert(i->rhs());

			}

		}

		bool changed = true;

		while (changed) {

			changed = false;

			for (auto i = transitions.begin(); i != transitions.end(); ++i) {

				const TT<label_type>& t = **i;

				std::vector<std::unordered_set<std::set<size_t>>*> tuple;

				auto j = t.lhs().begin();

				for (; j != t.lhs().end(); ++j) {

					auto iter = stateInfo.find(*j);

					if (iter == stateInfo.end())
						break;

					tuple.push(&iter->second);

				}

				if (j != stateInfo.end())
					break;

				std::vector<std::unordered_set<std::set<size_t>>::iterator> choice(tuple.size);

				for (size_t j = 0; j < choice.size(); ++j) {

					assert(tuple[j]->begin() != tuple[j]->end());

					choice[j] = tuple[j]->begin();

				}

				auto next = [&tuple, &choice]() -> bool {

					for (size_t i = 0; i < choice.size(); ++i) {

						if (++choice[i] != tuple[i]->end())
							return true;

						choice[i] == tuple[i]->begin();

					}

					return false;

				}

				do {

					std::set<size_t> tmp;

					tmp.insert(t.rhs());

					for (auto& v : choice)
						tmp.insert(v->begin(), v->end());

					if (stateInfo.insert(
							std::make_pair(t.rhs(), std::unordered_set<std::set<size_t>>())
						).first->second.insert(tmp).second)
						changed = true;

				} while (next());

			}

		}

		for (auto& f : ta.getFinalSates()) {

			auto iter =  stateInfo.find(f);

			assert(iter != stateInfo.end());

			for (auto& s : iter->second) {

				for (auto j = splitPoints.begin(); j != splitPoints.end(); ) {

					if (s.count(*j))
						++j;
					else
						j = splitPoints.erase(j);

				}

			}

		}

	}
*/
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

			if (cutpoint.bwdSelector != (size_t)(-1)) {

				assert(this->data[cutpoint.root].backwardLookup(cutpoint.bwdSelector) == root);

				this->data[cutpoint.root].bwdMap.erase(cutpoint.bwdSelector);

			}

		}

		this->data[root].valid = false;

	}

	void updateBackwardData(size_t root) {

		assert(root < this->data.size());
		assert(!this->data[root].valid);

		for (auto& cutpoint : this->data[root].signature) {

			assert(cutpoint.root < this->data.size());

			if (cutpoint.bwdSelector != (size_t)(-1)) {

				assert(
					this->data[cutpoint.root].bwdMap.find(
						cutpoint.bwdSelector
					) == this->data[cutpoint.root].bwdMap.end()
				);

				this->data[cutpoint.root].bwdMap.insert(std::make_pair(cutpoint.bwdSelector, root));

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

			for (auto& selectorRootPair : root.bwdMap) {

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

			if (cutpoint.bwdSelector == (size_t)(-1)) {

				// erase backward selector
				for (auto& tmp : srcSignature) {

					signature.push_back(tmp);
					signature.back().fwdSelectors = cutpoint.fwdSelectors;

					if (tmp.bwdSelector == (size_t)(-1))
						continue;

					signature.back().bwdSelector = (size_t)(-1);

					assert(tmp.root < this->data.size());

					auto iter = this->data[tmp.root].bwdMap.find(tmp.bwdSelector);

					assert(iter != this->data[tmp.root].bwdMap.end());
					assert(iter->second == src);

					this->data[tmp.root].bwdMap.erase(iter);

				}

			} else {

				// update backward selector
				for (auto& tmp : srcSignature) {

					signature.push_back(tmp);
					signature.back().fwdSelectors = cutpoint.fwdSelectors;

					if (tmp.bwdSelector == (size_t)(-1))
						continue;

					assert(tmp.root < this->data.size());

					auto iter = this->data[tmp.root].bwdMap.find(tmp.bwdSelector);

					assert(iter != this->data[tmp.root].bwdMap.end());
					assert(iter->second == src);

					auto i = this->data[tmp.root].bwdMap.begin();

					for (; i != this->data[tmp.root].bwdMap.end(); ++i) {

						if (i->second == dst)
							break;

					}

					if (i == this->data[tmp.root].bwdMap.end()) {

						iter->second = dst;

						continue;

					}

					if (i->first < iter->first) {

						this->data[tmp.root].bwdMap.erase(iter);

						continue;

					}

					this->data[tmp.root].bwdMap.erase(i);

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

		if (this->data[c].bwdMap.empty())
			return c;

		mask[c] = true;

		for (auto& selectorCutpointPair : this->data[c].bwdMap) {

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

			if (cutpoint.refCount > 1)
				marked[cutpoint.root] = true;

		}

		for (auto& selectorCutpointPair : this->data[c].bwdMap) {

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

		for (auto& selectorCutpointPair : this->data[c].bwdMap) {

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

	void getRelativeSignature(std::vector<std::pair<int, size_t>>& signature, size_t root) const {

		signature.clear();

		for (auto& tmp : this->data[root].signature)
			signature.push_back(std::make_pair(tmp.root - root, tmp.refCount));

	}

};

#endif
