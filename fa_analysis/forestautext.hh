/*
 * Copyright (C) 2010 Jiri Simacek
 *
 * This file is part of predator.
 *
 * predator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * predator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with predator.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FOREST_AUT_EXT_H
#define FOREST_AUT_EXT_H

#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <algorithm>
//#include <sstream>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "forestaut.hh"
#include "box.hh"
#include "labman.hh"
#include "utils.hh"
#include "tatimint.hh"

using std::vector;
using std::set;
using std::map;
using std::pair;
using std::make_pair;
using std::runtime_error;

template <class T>
class LeafManager {

	boost::unordered_map<T, size_t> index;
	boost::unordered_map<size_t, T> invIndex;

public:

	void set(const T& leaf, size_t state) {
		pair<typename boost::unordered_map<T, size_t>::iterator, bool> p =
			this->index.insert(make_pair(leaf, state));
		if (!p.second)
			throw runtime_error("LeafManager::set(): leaf already registered!");
		if (!this->invIndex.insert(make_pair(state, leaf)).second)
			throw runtime_error("LeafManager::set(): state <-> leaf association corrupted!");
	}

	size_t find(const T& leaf) const {
		typename boost::unordered_map<T, size_t>::const_iterator i = this->index.find(leaf);
		if (i == this->index.end())
			throw runtime_error("LeafManager::find(): leaf not found!");
		return i->second;
	}

	bool add(const T& leaf, size_t& candidate) {
		pair<typename boost::unordered_map<T, size_t>::iterator, bool> p =
			this->index.insert(make_pair(leaf, candidate));
		if (p.second)
			this->invIndex.insert(make_pair(candidate, leaf));
		else
			candidate = p.first->second;
		return p.second;
	}

	bool isLeaf(size_t state) const {
		return (this->invIndex.find(state) != this->invIndex.end());
	}

	bool isLeaf(size_t state, T& leaf) const {
		typename boost::unordered_map<size_t, T>::const_iterator i = this->invIndex.find(state);
		if (i == this->invIndex.end())
			return false;
		leaf = i->second;
		return true;		
	}
	
	size_t remove(const T& leaf) {
		typename boost::unordered_map<T, size_t>::iterator i = this->index.find(leaf);
		if (i == this->index.end())
			throw runtime_error("LeafManager::remove(): leaf not found!");
		size_t state = i->second;
		typename boost::unordered_map<size_t, T>::iterator j = this->invIndex.find(state);
		if (j == this->invIndex.end())
			throw runtime_error("LeafManager::remove(): state <-> leaf association corrupted!");
		this->index.erase(i);
		this->invIndex.erase(j);
		return state;
	}

	void clear() {
		this->index.clear();
		this->invIndex.clear();
	}

};

class FAE : public FA {

	friend std::ostream& operator<<(std::ostream& os, const FAE& fae);

	mutable BoxManager* boxMan;
	mutable LabMan* labMan;

	size_t stateOffset;

	vector<vector<size_t> > rootMap;
	
	// root -> (selector -> root)
	boost::unordered_map<size_t, map<size_t, size_t> > selectorMap;

//	LeafManager<size_t> refMan;
	LeafManager<const Data*> dataMan;

//	boost::unordered_map<size_t, size_t> rootReferenceIndex;
//	boost::unordered_map<size_t, size_t> invRootReferenceIndex;

public:

	void clear() {
		FA::clear();
		this->stateOffset = 1;
		this->rootMap.clear();
		this->selectorMap.clear();
//		this->rootReferenceIndex.clear();
//		this->invRootReferenceIndex.clear();
		this->dataMan.clear();
	}
	
	void loadTA(const TA<label_type>& src, const TA<label_type>::td_cache_type& cache, const TT<label_type>* top, size_t stateOffset) {
		this->clear();
		this->variables = *top->label().data;
		this->stateOffset = stateOffset;
		for (vector<size_t>::const_iterator i = top->lhs().begin(); i != top->lhs().end(); ++i) {
			TA<label_type>* ta = this->taMan->alloc();
			this->roots.push_back(ta);
			// add reachable transitions
			for (TA<label_type>::td_iterator j = src.tdStart(cache, itov(*i)); j.isValid(); j.next()) {
				ta->addTransition(*j);
				if (j->lhs().empty()) {
					if (j->label().head().getType() == Box::dataID)
						this->dataMan.set(&j->label().head().getData(), j->rhs());
				}
			}
			ta->addFinalState(*i);
			// recompute 'o'
			boost::unordered_map<size_t, vector<size_t> > o;
			FAE::computeDownwardO(*ta, o);
			boost::unordered_map<size_t, vector<size_t> >::iterator j = o.find(*i);
			assert(j != o.end());
			this->rootMap.push_back(j->second);
			// TODO: evaluate selectorMap
		}
	}
	
protected:

	void newState() {
		++this->stateOffset;
	}
	
	size_t nextState() {
		return this->stateOffset;
	}
	
	size_t freshState() {
		return this->stateOffset++;
	}

	void incrementStateOffset(size_t amount) {
		this->stateOffset += amount;
	}
	
	size_t addData(TA<label_type>& dst, const Data& data) {
		const Box* b = &this->boxMan->getData(data);
		size_t state = this->nextState();
		if (this->dataMan.add(&b->getData(), state))
			this->newState();
		dst.addTransition(vector<size_t>(), &this->labMan->lookup(itov(b)), state);
		return state;
	}

	static bool containsBox(label_type label) {
		for (vector<const Box*>::const_iterator i = label.dataB->begin(); i != label.dataB->end(); ++i) {
			if ((*i)->isBox())
				return true;
		}
		return false;
	}

	static void removeMultOcc(vector<size_t>& x) {
		set<size_t> s;
		size_t offset = 0;
		for (size_t i = 0; i < x.size(); ++i) {
			if (s.insert(x[i]).second)
				x[offset++] = x[i];
		}
		x.resize(s.size());
	}

	static void reorderBoxes(vector<const Box*>& label, vector<size_t>& lhs) {
		vector<std::pair<const Box*, vector<size_t> > > tmp;
		std::vector<size_t>::iterator lhsBegin = lhs.end(), lhsEnd = lhs.begin();
		for (size_t i = 0; i < label.size(); ++i) {
			lhsBegin = lhsEnd;
			lhsEnd += label[i]->getArity();
			tmp.push_back(std::make_pair(label[i], std::vector<size_t>(lhsBegin, lhsEnd)));
		}
		std::sort(tmp.begin(), tmp.end());
		lhs.clear();
		for (size_t i = 0; i < tmp.size(); ++i) {
			label[i] = tmp[i].first;
			lhs.insert(lhs.end(), tmp[i].second.begin(), tmp[i].second.end());
		}
	}

	static void renameVector(vector<size_t>& dst, const vector<size_t>& index) {
		for (vector<size_t>::iterator i = dst.begin(); i != dst.end(); ++i) {
			assert(index[*i] != (size_t)(-1));
			*i = index[*i];
		}
	}

	static void updateMap(vector<size_t>& dst, size_t ref, const vector<size_t>& src) {
		vector<size_t> res;
		vector<size_t>::iterator i = std::find(dst.begin(), dst.end(), ref);
		assert(i != dst.end());
//		std::copy(dst.begin(), i, res.end());
//		std::copy(src.begin(), src.end(), res.end());
//		std::copy(i + 1, dst.end(), res.end());
		for (std::vector<size_t>::const_iterator j = dst.begin(); j != i; ++j)
			res.push_back(*j);
		for (std::vector<size_t>::const_iterator j = src.begin(); j != src.end(); ++j)
			res.push_back(*j);
		for (std::vector<size_t>::const_iterator j = i + 1; j != dst.end(); ++j)
			res.push_back(*j);
		FAE::removeMultOcc(res);
		std::swap(dst, res);
	}

	static void updateRoot(TA<label_type>*& root, TA<label_type>::Manager& manager, TA<label_type>* newRoot) {
		if (root)
			manager.release(root);
		root = newRoot;
	}

	TA<label_type>* relabelReferences(TA<label_type>* src, const vector<size_t>& index) {
		TA<label_type>* ta = this->taMan->alloc();
		ta->addFinalState(src->getFinalState());
		for (TA<label_type>::iterator i = src->begin(); i != src->end(); ++i) {
			vector<size_t> lhs;
			for (vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j) {
				const Data* data;
				if (!this->dataMan.isLeaf(*j, data) || !data->isRef() || index[data->d_ref.root] == data->d_ref.root) {
					lhs.push_back(*j);
				} else {
					if (index[data->d_ref.root] != (size_t)(-1))
						lhs.push_back(this->addData(*ta, Data::createRef(index[data->d_ref.root], data->d_ref.displ)));
					else
						lhs.push_back(this->addData(*ta, Data::createUndef()));
				}
			}
			ta->addTransition(lhs, i->label(), i->rhs());
		}
		return ta;
	}

	TA<label_type>* invalidateReference(TA<label_type>* src, size_t root) {
		TA<label_type>* ta = this->taMan->alloc();
		ta->addFinalState(src->getFinalState());
		for (TA<label_type>::iterator i = src->begin(); i != src->end(); ++i) {
			vector<size_t> lhs;
			for (vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j) {
				const Data* data;
				if (this->dataMan.isLeaf(*j, data) && data->isRef(root)) {
					lhs.push_back(this->addData(*ta, Data::createUndef()));
				} else {
					lhs.push_back(*j);
				}
			}
			if (!i->label().head().isData() || !i->label().head().getData().isRef(root))
				ta->addTransition(lhs, i->label(), i->rhs());
		}
		return ta;
	}

	static void invalidateReference(std::vector<size_t>& dst, size_t root) {
		std::vector<size_t>::iterator i = std::find(dst.begin(), dst.end(), root);
		if (i != dst.end())
			dst.erase(i);
	}

	TA<label_type>* mergeRoot(TA<label_type>& dst, size_t ref, TA<label_type>& src, size_t& joinState) {
		assert(ref < this->roots.size());
		TA<label_type> ta(this->taMan->getBackend());
		ta.addFinalState(dst.getFinalState());
		const Box* b = &this->boxMan->getData(Data::createRef(ref));
		size_t refState = this->dataMan.find(&b->getData());
		joinState = this->freshState();
		bool hit = false;
		for (TA<label_type>::iterator i = dst.begin(); i != dst.end(); ++i) {
//			const Data* data;
//			if (i->label().head().isData(data) && data->isRef(ref))
//				continue;
			std::vector<size_t> tmp = i->lhs();
			for (std::vector<size_t>::iterator j = tmp.begin(); j != tmp.end(); ++j) {
				if (*j == refState) {
					*j = joinState;
					hit = true;
//					break; ?
				}
			}
			ta.addTransition(tmp, i->label(), i->rhs());
		}
//		std::cerr << joinState << std::endl;
		assert(hit);
		// avoid screwing up things
		src.unfoldAtRoot(ta, joinState, false);
		TA<label_type>* ta2 = this->taMan->alloc();
		ta.unreachableFree(*ta2);
		return ta2;
	}

	typedef boost::unordered_map<size_t, vector<size_t> > o_map_type;

	static bool updateO(o_map_type& o, size_t state, const vector<size_t>& v) {
		pair<o_map_type::iterator, bool> p = o.insert(make_pair(state, v));
		if (p.second)
			return true;
		if (p.first->second.size() >= v.size())
			return false;
/*		if (p.first->second.size() > v.size())
			throw runtime_error("FAE::updateO(): Inconsistent update of 'o' (length mismatch)!");*/
/*		for (size_t i = 0; i < p.first->second.size(); ++i) {
			if (v[i] != p.first->second[i])
				throw runtime_error("FAE::updateO(): Inconsistent update of 'o' (prefix mismatch)!");
		}*/
		if (p.first->second != v) {
			p.first->second = v;
			return true;
		}
		return false;
	}

	// computes downward 'o' function
	static void computeDownwardO(const TA<label_type>& ta, o_map_type& o) {
		o.clear();
		bool changed = true;
		while (changed) {
			changed = false;
			for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {
				const Data* data;
				vector<size_t> v;
				if (i->label().head().isData(data) && data->isRef()) {
					v.push_back(data->d_ref.root);
				} else {
//					vector<size_t> order;
//					FAE::evaluateLhsOrder(*i->label().dataB, order);
					for (vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j) {
						o_map_type::iterator k = o.find(*j);
						if (k == o.end())
							break;
						v.insert(v.end(), k->second.begin(), k->second.end());
					}
					FAE::removeMultOcc(v);
				}
				if (FAE::updateO(o, i->rhs(), v))
					changed = true;
			}
		}
	}

	static bool isUniqueRef(const TA<label_type>& ta, size_t ref) {
		const Data* data;
		boost::unordered_map<size_t, size_t> index;
		for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {
			if (i->label().head().isData(data) && data->isRef(ref))
				index.insert(make_pair(i->rhs(), 1));
		}
		bool changed = true;
		while (changed) {
			changed = false;
			for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {
				if (!i->label().head().isData()) {
					size_t sum = 0;
					for (vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j)
						sum += index.insert(make_pair(*j, 0)).first->second;
					if (sum == 0)
						continue;
					// if everything is reachable, then we can safely conclude here
					if (sum > 1)
						return false;
					size_t& k = index.insert(make_pair(i->rhs(), 0)).first->second;
//					sum = std::min(sum, 2);
//					if (k < sum) {
//						k = sum;
//						changed = true;
//					}
					if (k == 0) {
						k = 1;
						changed = true;
					}
				}
			}
		}
		return true;
	}

	void visitDown(size_t c, std::vector<bool>& visited, std::vector<size_t>& order, std::vector<bool>& marked) const {
		if (visited[c]) {
			marked[c] = true;
			return;
		}
		visited[c] = true;
		order.push_back(c);
		for (vector<size_t>::const_iterator i = this->rootMap[c].begin(); i != this->rootMap[c].end(); ++i) {
			this->visitDown(*i, visited, order, marked);
			if (!marked[*i] && !FAE::isUniqueRef(*this->roots[c], *i))
				marked[*i] = true;
		}
	}

	void visitDown(size_t c, std::vector<bool>& visited) const {
		if (visited[c])
			return;
		visited[c] = true;
		for (vector<size_t>::const_iterator i = this->rootMap[c].begin(); i != this->rootMap[c].end(); ++i)
			this->visitDown(*i, visited);
	}

	void traverse(std::vector<bool>& visited, std::vector<size_t>& order, std::vector<bool>& marked) const {
		// TODO: upward traversal
		visited = std::vector<bool>(this->roots.size(), false);
		marked = std::vector<bool>(this->roots.size(), false);
		order.clear();
		for (vector<Data>::const_iterator i = this->variables.begin(); i != variables.end(); ++i) {
			// skip everything what is not a root reference
			if (!i->isRef())
				continue;
			size_t root = i->d_ref.root;
			// mark rootpoint pointed by a variable
			marked[root] = true;
			// check whether we traversed this one before
			if (visited[root])
				continue;
			this->visitDown(root, visited, order, marked);
		}
	}

	void traverse(std::vector<bool>& visited) const {
		// TODO: upward traversal
		visited = std::vector<bool>(this->roots.size(), false);
		for (vector<Data>::const_iterator i = this->variables.begin(); i != variables.end(); ++i) {
			// skip everything what is not a root reference
			if (!i->isRef())
				continue;
			size_t root = i->d_ref.root;
			// check whether we traversed this one before
			if (visited[root])
				continue;
			this->visitDown(root, visited);
		}
	}

	void checkGarbage(const std::vector<bool>& visited) {
		for (size_t i = 0; i < this->roots.size(); ++i) {
			if (!visited[i] && (this->roots[i] != NULL)) {
//				std::cerr << "the root " << i << " is not referenced anymore ... " << std::endl;
				throw std::runtime_error("FAE::checkGarbage(): garbage detected!");
			}
		}
	}

public:

	struct NormInfo {

		struct RootInfo {

			size_t index;
			std::vector<std::pair<size_t, size_t> > mergedRoots;

			RootInfo(size_t index) : index(index) {}

			void initRoots(FAE& fae) const {
				fae.roots[this->index] = fae.taMan->alloc();
				for (std::vector<std::pair<size_t, size_t> >::const_iterator i = this->mergedRoots.begin(); i != this->mergedRoots.end(); ++i)
					fae.roots[i->first] = fae.taMan->alloc();				
			}

			friend std::ostream& operator<<(std::ostream& os, const RootInfo& rootInfo) {
				os << rootInfo.index << '|';
				for (std::vector<std::pair<size_t, size_t> >::const_iterator i = rootInfo.mergedRoots.begin(); i != rootInfo.mergedRoots.end(); ++i)
					os << i->first << ':' << i->second << ' ';
				return os;
			}

		};

		size_t rootCount;
//		std::vector<RootInfo> data;
		std::map<size_t, RootInfo> data;

		NormInfo() {}

		void addRoot(size_t index) {
			bool b = this->data.insert(std::make_pair(index, RootInfo(index))).second;
			assert(b);			
		}

		void mergeRoots(size_t dst, size_t src, size_t refState) {
			std::map<size_t, RootInfo>::iterator i = this->data.find(dst);
			assert(i != this->data.end());
			std::map<size_t, RootInfo>::iterator j = this->data.find(src);
			assert(j != this->data.end());
			i->second.mergedRoots.push_back(std::make_pair(src, refState));
			for (std::vector<std::pair<size_t, size_t> >::iterator k = j->second.mergedRoots.begin(); k != j->second.mergedRoots.end(); ++k)
				i->second.mergedRoots.push_back(*k);
			this->data.erase(j);
		}

		void reindex(const std::vector<size_t>& index) {
			std::map<size_t, RootInfo> tmp(this->data);
			this->data.clear();
			for (std::map<size_t, RootInfo>::iterator i = tmp.begin(); i != tmp.end(); ++i)
				this->data.insert(std::make_pair(index[i->first], i->second));
		}

		void initRoots(FAE& fae) const {
			fae.roots.resize(this->rootCount, NULL);
			for (std::map<size_t, RootInfo>::const_iterator i = this->data.begin(); i != this->data.end(); ++i)
				i->second.initRoots(fae);
		}

		friend std::ostream& operator<<(std::ostream& os, const NormInfo& normInfo) {
			os << "roots " << normInfo.rootCount << std::endl;
			for (std::map<size_t, RootInfo>::const_iterator i = normInfo.data.begin(); i != normInfo.data.end(); ++i)
				os << i->first << ':' << i->second << std::endl;
			return os;
		}
	
	};

	// check consistency
	void check() {
		// compute reachable roots
		std::vector<bool> visited(this->roots.size(), false);
		this->traverse(visited);
		// check garbage
		this->checkGarbage(visited);
	}

	void normalizeRoot(NormInfo& normInfo, std::vector<bool>& normalized, size_t root, const std::vector<bool>& marked) {
		if (normalized[root])
			return;
		normalized[root] = true;
		std::vector<size_t> tmp = this->rootMap[root];
		normInfo.addRoot(root);
		for (std::vector<size_t>::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			this->normalizeRoot(normInfo, normalized, *i, marked);
			if (!marked[*i]) {
//				std::cerr << "merging " << *i << '(' << this->roots[*i] << ')' << " into " << root << '(' << this->roots[root] << ')' << std::endl;
				size_t refState;
				TA<label_type>* ta = this->mergeRoot(*this->roots[root], *i, *this->roots[*i], refState);
				FAE::updateRoot(this->roots[root], *this->taMan, ta);
				FAE::updateRoot(this->roots[*i], *this->taMan, NULL);
				FAE::updateMap(this->rootMap[root], *i, this->rootMap[*i]);
				normInfo.mergeRoots(root, *i, refState);
			}
		}
	}

	// normalize representation
	void normalize(NormInfo& normInfo, const std::vector<size_t>& forbidden = std::vector<size_t>()) {
		// compute canonical root ordering
		std::vector<size_t> order;
		std::vector<bool> visited(this->roots.size(), false), marked(this->roots.size(), false);
		this->traverse(visited, order, marked);
		// check garbage
		this->checkGarbage(visited);
		// prevent merging of forbidden roots
		for (std::vector<size_t>::const_iterator i = forbidden.begin(); i != forbidden.end(); ++i)
			marked[*i] = true;
		// reindex roots
		std::vector<size_t> index(this->roots.size(), (size_t)(-1));
		std::vector<bool> normalized(this->roots.size(), false);
		std::vector<TA<label_type>*> newRoots;
		std::vector<vector<size_t> > newRootMap;
		size_t offset = 0;
		for (std::vector<size_t>::iterator i = order.begin(); i < order.end(); ++i) {
			this->normalizeRoot(normInfo, normalized, *i, marked);
			assert(marked[*i] || (this->roots[*i] == NULL));
			if (!marked[*i])
				continue;
			newRoots.push_back(this->roots[*i]);
			newRootMap.push_back(this->rootMap[*i]);
			index[*i] = offset++;
//			normInfo.addRoot(*i);
		}
		normInfo.rootCount = this->roots.size();
		normInfo.reindex(index);
		// update representation
		this->roots = newRoots;
		this->rootMap = newRootMap;
		for (size_t i = 0; i < this->roots.size(); ++i) {
			FAE::updateRoot(this->roots[i], *this->taMan, this->relabelReferences(this->roots[i], index));
			FAE::renameVector(this->rootMap[i], index);
		}
		// update variables
		for (std::vector<Data>::iterator i = this->variables.begin(); i != this->variables.end(); ++i) {
			if (i->isRef()) {
				assert(index[i->d_ref.root] != (size_t)(-1));
				i->d_ref.root = index[i->d_ref.root];
			}
		}
	}

	// single accepting in, (single accepting out?)
	void split(std::vector<TA<label_type>*>& dst, const TA<label_type>& src, size_t baseIndex, const std::vector<std::pair<size_t, size_t> >& splitPoints) {

		Index<size_t> stateIndex;

		TA<label_type>::td_cache_type cache;
		src.buildTDCache(cache);

		boost::unordered_map<size_t, size_t> splitMap;

		dst[baseIndex]->addFinalState(stateIndex.translateOTF(src.getFinalState()) + this->stateOffset);
		for (size_t i = 0; i < splitPoints.size(); ++i) {
			splitMap.insert(std::make_pair(splitPoints[i].first, splitPoints[i].second));
			dst[splitPoints[i].second]->addFinalState(stateIndex.translateOTF(splitPoints[i].first) + this->stateOffset);
		}

		std::vector<std::pair<size_t, size_t> > stack = itov(std::make_pair(src.getFinalState(), baseIndex));
		
		boost::unordered_set<std::pair<size_t, size_t> > visited;
		while (!stack.empty()) {
			std::pair<size_t, size_t> p = stack.back();
			stack.pop_back();
			if (!visited.insert(p).second)
				continue;
			TA<label_type>::td_cache_type::iterator i = cache.find(p.first);
			assert(i != cache.end());
			for (std::vector<const TT<label_type>*>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				std::vector<size_t> lhs;
				for (std::vector<size_t>::const_iterator k = (*j)->lhs().begin(); k != (*j)->lhs().end(); ++k) {
					boost::unordered_map<size_t, size_t>::iterator l = splitMap.find(*k);
					size_t state;
					if (l != splitMap.end()) {
						stack.push_back(std::make_pair(*k, l->second));
						state = this->addData(*dst[p.second], Data::createRef(splitPoints[l->second].second));
					} else {
						stack.push_back(std::make_pair(*k, p.second));
						if (this->dataMan.isLeaf(state))
							state = *k;
						else
							state = stateIndex.translateOTF(*k) + this->stateOffset;
					}
					lhs.push_back(state);
				}
				size_t rhs;
				if (this->dataMan.isLeaf((*j)->rhs()))
					rhs = (*j)->rhs();
				else
					rhs = stateIndex.translateOTF((*j)->rhs()) + this->stateOffset;
				dst[p.second]->addTransition(lhs, (*j)->label(), rhs);
			}
		}
		this->incrementStateOffset(stateIndex.size());
	}

	struct IntersectAndRelabelF {

		FAE& fae;
		TA<label_type>& dst;
		const std::vector<size_t>& index;
		const TA<label_type>& src1;
		const TA<label_type>& src2;

		IntersectAndRelabelF(FAE& fae, TA<label_type>& dst, const std::vector<size_t>& index, const TA<label_type>& src1, const TA<label_type>& src2)
			: fae(fae), dst(dst), index(index), src1(src1), src2(src2) {}

		void operator()(const TT<label_type>* t1, const TT<label_type>* t2, const std::vector<size_t>& lhs, size_t& rhs) {
//			std::cerr << *t1 << " vs " << *t2 << std::endl;
			const Data* data;
			if (this->fae.dataMan.isLeaf(t1->rhs(), data)) {
				if (data->isRef()) {
					if (data->d_ref.root < this->index.size())
						rhs = fae.addData(this->dst, Data::createRef(this->index[data->d_ref.root], data->d_ref.displ));
					return;
				}
				rhs = t1->rhs();
			}
			this->dst.addTransition(lhs, t1->_label, rhs);
			if (this->src1.isFinalState(t1->rhs()) && this->src2.isFinalState(t2->rhs()))
				this->dst.addFinalState(rhs);
		}

	};

	struct IntersectAndRelabelSpecialF {

		FAE& fae;
		TA<label_type>& dst;
		const std::vector<size_t>& index;
		std::vector<std::pair<size_t, size_t> >& splitPoints;
		const TA<label_type>& src1;
		const TA<label_type>& src2;
		boost::unordered_map<size_t, size_t> rootMap;
		
		IntersectAndRelabelSpecialF(FAE& fae, TA<label_type>& dst, std::vector<std::pair<size_t, size_t> >& splitPoints, const std::vector<size_t>& index, const TA<label_type>& src1, const TA<label_type>& src2, const NormInfo::RootInfo& rootInfo)
			: fae(fae), dst(dst), index(index), splitPoints(splitPoints), src1(src1), src2(src2) {
			for (std::vector<std::pair<size_t, size_t> >::const_iterator i = rootInfo.mergedRoots.begin(); i != rootInfo.mergedRoots.end(); ++i) {
				bool b = this->rootMap.insert(std::make_pair(i->second, i->first)).second;
				assert(b);
			}
		}

		void operator()(const TT<label_type>* t1, const TT<label_type>* t2, const std::vector<size_t>& lhs, size_t& rhs) {
			const Data* data;
			if (this->fae.dataMan.isLeaf(t1->rhs(), data)) {
				if (data->isRef()) {
					if (data->d_ref.root < this->index.size())
						rhs = fae.addData(this->dst, Data::createRef(this->index[data->d_ref.root], data->d_ref.displ));
					return;
				}
				rhs = t1->rhs();
			}
			this->dst.addTransition(lhs, t1->_label, rhs);
			if (this->src1.isFinalState(t1->rhs()) && this->src2.isFinalState(t2->rhs()))
				this->dst.addFinalState(rhs);
			boost::unordered_map<size_t, size_t>::iterator i = rootMap.find(t2->rhs());
			if (i != this->rootMap.end())
				this->splitPoints.push_back(std::make_pair(rhs, i->second));
		}

	};

	bool denormalize(const FAE& fae, const NormInfo& normInfo) {
		assert(fae.roots.size() == normInfo.data.size());
		assert(this->roots.size() == fae.roots.size());
		
		FAE tmp(*this);
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			FAE::updateRoot(*i, *this->taMan, NULL);

		this->rootMap.resize(normInfo.rootCount);

		std::vector<size_t> index(normInfo.data.size(), (size_t)(-1));

		size_t ii = 0;

		for (std::map<size_t, NormInfo::RootInfo>::const_iterator i = normInfo.data.begin(); i != normInfo.data.end(); ++i, ++ii) {
			index[ii] = i->second.index;
			this->rootMap[i->second.index] = tmp.rootMap[ii];
		}

		for (size_t i = 0; i < this->rootMap.size(); ++i)
			FAE::renameVector(this->rootMap[i], index);

		normInfo.initRoots(*this);

		for (size_t i = 0; i != normInfo.data.size(); ++i) {
			TA<label_type>::lt_cache_type cache1, cache2;
			tmp.roots[i]->buildLTCache(cache1);
			fae.roots[i]->buildLTCache(cache2);
			std::map<size_t, NormInfo::RootInfo>::const_iterator j = normInfo.data.find(i);
			assert(j != normInfo.data.end());
			size_t stateCount;
			if (j->second.mergedRoots.size() == 0) {
				stateCount = TA<label_type>::computeProduct(
					cache1,
					cache2,
					IntersectAndRelabelF(
						*this, *this->roots[index[i]], index, *tmp.roots[i], *fae.roots[i]
					),
					this->nextState()
				);
			} else {
				TA<label_type> ta(this->taMan->getBackend());
				std::vector<std::pair<size_t, size_t> > splitPoints;
				stateCount = TA<label_type>::computeProduct(
					cache1,
					cache2,
					IntersectAndRelabelSpecialF(
						*this, ta, splitPoints, index, *tmp.roots[i], *fae.roots[i], j->second
					),
					this->nextState()
				);
				this->split(this->roots, ta, index[i], splitPoints);
			}
			this->incrementStateOffset(stateCount);
		}

		for (size_t i = 0; i < this->roots.size(); ++i) {
			if (this->roots[i] && this->roots[i]->getFinalStates().empty())
				return false;
		}

		return true;

	}

	struct CustomIntersectF {

		FAE& fae;
		TA<label_type>& dst;
		const TA<label_type>& src1;
		const TA<label_type>& src2;

		CustomIntersectF(FAE& fae, TA<label_type>& dst, const TA<label_type>& src1, const TA<label_type>& src2)
		 : fae(fae), dst(dst), src1(src1), src2(src2) {}

		void operator()(const TT<label_type>* t1, const TT<label_type>* t2, const std::vector<size_t>& lhs, size_t& rhs) {
			const Data* data;
			if (this->fae.dataMan.isLeaf(t2->rhs(), data))
				rhs = t2->rhs();
			this->dst.addTransition(lhs, t2->_label, rhs);
			if (this->src1.isFinalState(t1->rhs()) && this->src2.isFinalState(t2->rhs()))
				this->dst.addFinalState(rhs);
		}

	};

	void buildLTCacheExt(const TA<label_type>& ta, TA<label_type>::lt_cache_type& cache) {
		const Box* b = &this->boxMan->getData(Data::createUndef());
		label_type lUndef = &this->labMan->lookup(itov(b));
		for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {
			if (i->label().head().isData()) {
				cache.insert(
					make_pair(lUndef, std::vector<const TT<label_type>*>())
				).first->second.push_back(&*i);
			} else {
				cache.insert(
					make_pair(i->label(), std::vector<const TT<label_type>*>())
				).first->second.push_back(&*i);
			}
		}
	}

	bool reverse(const FAE& fae) {

		this->variables = fae.variables;
		this->rootMap = fae.rootMap;

		if (this->roots.size() > fae.roots.size()) {
			FAE::updateRoot(this->roots.back(), *this->taMan, NULL);
			this->roots.pop_back();
		}

		assert(this->roots.size() == fae.roots.size());

		FAE tmp(*this);
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			FAE::updateRoot(*i, *this->taMan, NULL);

		for (size_t i = 0; i < this->roots.size(); ++i) {
			if (!tmp.roots[i]) {
				this->roots[i] = this->taMan->addRef(fae.roots[i]);
				continue;
			}
			TA<label_type>::lt_cache_type cache1, cache2;
			FAE::buildLTCacheExt(*tmp.roots[i], cache1);
			FAE::buildLTCacheExt(*fae.roots[i], cache2);

			this->roots[i] = this->taMan->alloc();
			
			size_t stateCount = TA<label_type>::computeProduct(
				cache1,
				cache2,
				FAE::CustomIntersectF(*this, *this->roots[i], *tmp.roots[i], *fae.roots[i]),
				this->nextState()
			);

			this->incrementStateOffset(stateCount);
				
			if (this->roots[i]->getFinalStates().empty())
				return false;
		}

		return true;

	}

	void heightAbstraction(size_t height = 1) {
		for (size_t i = 0; i < this->roots.size(); ++i) {
			TA<label_type> ta(this->taMan->getBackend());
			this->roots[i]->minimized(ta);
			Index<size_t> stateIndex;
			ta.buildStateIndex(stateIndex);
			std::vector<std::vector<bool> > rel(stateIndex.size(), std::vector<bool>(stateIndex.size(), false));
			o_map_type o;
			FAE::computeDownwardO(ta, o);
			for (Index<size_t>::iterator j = stateIndex.begin(); j != stateIndex.end(); ++j) {
				rel[j->second][j->second] = true;
//				if (this->dataMan.isLeaf(j->first))
//					continue;
				for (Index<size_t>::iterator k = stateIndex.begin(); k != stateIndex.end(); ++k) {
					if (k == j) // || this->dataMan.isLeaf(k->first))
						continue;
					if (o[j->first] == o[k->first])
						rel[j->second][k->second] = true;
				}
			}
			ta.heightAbstraction(rel, height, stateIndex);
			FAE::updateRoot(this->roots[i], *this->taMan, &ta.collapsed(*this->taMan->alloc(), rel, stateIndex));
		}
	}

	void decomposeAtRoot(vector<FAE*>& dst, size_t root, const vector<const Box*>& boxes) const {
		throw runtime_error("FAE::decomposeAtRoot(): box decomposition not implemented! (désolé)");
	}

	struct IsolateOneF {
		size_t offset;

		IsolateOneF(size_t offset) : offset(offset) {}

		bool operator()(const Box* box) const {
			return box->getDownwardCoverage(0).second.count(this->offset);
		}
	};

	struct IsolateSetF {
		std::set<size_t> s;
		
		IsolateSetF(const std::vector<size_t>& v, size_t offset = 0) {
			for (std::vector<size_t>::const_iterator i = v.begin(); i != v.end(); ++i)
				this->s.insert(*i + offset);
		}
		
		bool operator()(const Box* box) const {
			return utils::checkIntersection(box->getDownwardCoverage(0).second, this->s);
		}
	};

	struct IsolateAllF {
		bool operator()(const Box* box) const {
			return !box->getDownwardCoverage(0).first.empty();
		}
	};

	// adds redundant root points to allow further manipulation
	template <class F>
	void isolateAtRoot(std::vector<FAE*>& dst, size_t root, F f) const {
		assert(root < this->roots.size());
		for (TA<label_type>::iterator i = this->roots[root]->begin(); i != this->roots[root]->end(); ++i) {
			if (i->rhs() == this->roots[root]->getFinalState()) {
				FAE fae(*this);
				TA<label_type> ta(*fae.roots[root], false);
				vector<size_t> lhs;
				size_t lhsOffset = 0;
				std::vector<const Box*> boxes;
				for (vector<const Box*>::const_iterator j = i->label().dataB->begin(); j != i->label().dataB->end(); ++j) {
					if (!f(*j)) {
						// this box is not interesting
						for (size_t k = 0; k < (*j)->getArity(); ++k, ++lhsOffset)
							lhs.push_back(i->lhs()[lhsOffset]);
						continue;
					}
					// we have to isolate here
					for (size_t k = 0; k < (*j)->getArity(); ++k, ++lhsOffset) {
						if (this->dataMan.isLeaf(i->lhs()[lhsOffset])) {
							// no need to create a leaf when it's already there
							lhs.push_back(i->lhs()[lhsOffset]);
							continue;
						}
						// update new left-hand side
						lhs.push_back(fae.addData(ta, Data::createRef(fae.roots.size())));
						// prepare new root
						TA<label_type> tmp(*fae.roots[root], false);
						tmp.addFinalState(i->lhs()[lhsOffset]);
						TA<label_type>* tmp2 = fae.taMan->alloc();
						tmp.unreachableFree(*tmp2);
						// compute 'o'
						boost::unordered_map<size_t, vector<size_t> > o;
						FAE::computeDownwardO(*tmp2, o);
						fae.roots.push_back(tmp2);
						fae.rootMap.push_back(o[tmp2->getFinalState()]);
					}
					if ((*j)->isBox())
						boxes.push_back(*j);
				}
				size_t newState = fae.freshState();
				ta.addTransition(lhs, i->label(), newState);
				ta.addFinalState(newState);
				TA<label_type>* tmp = fae.taMan->alloc();
				ta.unreachableFree(*tmp);
				// exchange the original automaton with the new one
				FAE::updateRoot(fae.roots[root], *fae.taMan, tmp);
				o_map_type o;
				FAE::computeDownwardO(*tmp, o);
				fae.rootMap[root] = o[tmp->getFinalState()];
				if (!boxes.empty()) {
					fae.decomposeAtRoot(dst, root, boxes);
				} else {
					dst.push_back(new FAE(fae));
				}
			}
		}
	}

	static void displToData(const SelData& sel, Data& data) {
		if (data.isRef())
			data.d_ref.displ = sel.displ;
	}

	static void displToSel(SelData& sel, Data& data) {
		if (data.isRef()) {
			sel.displ = data.d_ref.displ;
			data.d_ref.displ = 0;
		}
	}

	void transitionLookup(const TT<label_type>& transition, size_t offset, Data& data) const {
		bool found = false;
		size_t lhsOffset = 0;
		for (vector<const Box*>::const_iterator i = transition.label().dataB->begin(); i != transition.label().dataB->end(); ++i) {
			if ((*i)->isSelector(offset)) {
				assert(!found);
				found = true;
				const Data* tmp;
				if (!this->dataMan.isLeaf(transition.lhs()[lhsOffset], tmp))
					throw std::runtime_error("FAE::transitionLookup(): destination is not a leaf!");
				data = *tmp;
				FAE::displToData((*i)->getSelector(), data);
			}
			lhsOffset += (*i)->getArity();
		}
		if (!found)
			throw std::runtime_error("FAE::nodeLookup(): selector not found!");
	}

	void transitionLookup(const TT<label_type>& transition, size_t base, const std::set<size_t>& offsets, Data& data) const {
		size_t lhsOffset = 0;
		data = Data::createStruct();
		for (vector<const Box*>::const_iterator i = transition.label().dataB->begin(); i != transition.label().dataB->end(); ++i) {
			if ((*i)->isSelector() && offsets.count((*i)->getSelector().offset)) {
				const Data* tmp;
				if (!this->dataMan.isLeaf(transition.lhs()[lhsOffset], tmp))
					throw std::runtime_error("FAE::transitionLookup(): destination is not a leaf!");
				data.d_struct->push_back(Data::item_info((*i)->getSelector().offset - base, *tmp));
				FAE::displToData((*i)->getSelector(), data.d_struct->back().second);
			} 
			lhsOffset += (*i)->getArity();
		}
		if (data.d_struct->size() != offsets.size())
			throw std::runtime_error("FAE::nodeLookup(): selectors missmatch!");
	}

	void transitionLookup(const TT<label_type>& transition, std::vector<std::pair<SelData, Data> >& nodeInfo) const {
		size_t lhsOffset = 0;
		for (vector<const Box*>::const_iterator i = transition.label().dataB->begin(); i != transition.label().dataB->end(); ++i) {
			assert((*i)->isSelector());
			const Data* tmp;
			if (!this->dataMan.isLeaf(transition.lhs()[lhsOffset], tmp))
				throw std::runtime_error("FAE::transitionLookup(): destination is not a leaf!");
			nodeInfo.push_back(std::make_pair((*i)->getSelector(), *tmp));
			FAE::displToData(nodeInfo.back().first, nodeInfo.back().second);
			lhsOffset += (*i)->getArity();
		}
	}

	void transitionModify(TA<label_type>& dst, const TT<label_type>& transition, size_t offset, const Data& in, Data& out) {
		vector<size_t> lhs;
		vector<const Box*> label;
		size_t lhsOffset = 0;
		bool found = false;
		for (vector<const Box*>::const_iterator i = transition.label().dataB->begin(); i != transition.label().dataB->end(); ++i) {
			if ((*i)->isSelector(offset)) {
				assert(!found);
				found = true;
				const Data* tmp;
				if (!this->dataMan.isLeaf(transition.lhs()[lhsOffset], tmp))
					throw runtime_error("FAE::transitionModify(): destination is not a leaf!");
				out = *tmp;
				FAE::displToData((*i)->getSelector(), out);
				SelData s = (*i)->getSelector();
				Data d = in;
				FAE::displToSel(s, d);
				lhs.push_back(this->addData(dst, d));
				label.push_back(&this->boxMan->getSelector(s));
			} else {
				lhs.insert(lhs.end(), transition.lhs().begin() + lhsOffset, transition.lhs().begin() + lhsOffset + (*i)->getArity());
				label.push_back(*i);
			}
			lhsOffset += (*i)->getArity();
		}
		if (!found)
			throw runtime_error("FAE::nodeModify(): selector not found!");
		FAE::reorderBoxes(label, lhs);
		size_t state = this->freshState();
		dst.addFinalState(state);
		dst.addTransition(lhs, &this->labMan->lookup(label), state);
	}

	void transitionModify(TA<label_type>& dst, const TT<label_type>& transition, size_t base, const std::map<size_t, Data>& in, Data& out) {
		vector<size_t> lhs;
		vector<const Box*> label;
		size_t lhsOffset = 0;
		out = Data::createStruct();
		for (vector<const Box*>::const_iterator i = transition.label().dataB->begin(); i != transition.label().dataB->end(); ++i) {
			if ((*i)->isSelector()) {
				std::map<size_t, Data>::const_iterator j = in.find((*i)->getSelector().offset);
				if (j != in.end()) {
					const Data* tmp;
					if (!this->dataMan.isLeaf(transition.lhs()[lhsOffset], tmp))
						throw runtime_error("FAE::transitionModify(): destination is not a leaf!");
					out.d_struct->push_back(Data::item_info((*i)->getSelector().offset - base, *tmp));
					FAE::displToData((*i)->getSelector(), out.d_struct->back().second);
					SelData s = (*i)->getSelector();
					Data d = j->second;
					FAE::displToSel(s, d);
					lhs.push_back(this->addData(dst, d));
					label.push_back(&this->boxMan->getSelector(s));
				} else {
					lhs.insert(lhs.end(), transition.lhs().begin() + lhsOffset, transition.lhs().begin() + lhsOffset + (*i)->getArity());
					label.push_back(*i);
				}
			}
			lhsOffset += (*i)->getArity();
		}
		if (out.d_struct->size() != in.size())
			throw runtime_error("FAE::nodeModify(): selectors missmatch!");
		FAE::reorderBoxes(label, lhs);
		size_t state = this->freshState();
		dst.addFinalState(state);
		dst.addTransition(lhs, &this->labMan->lookup(label), state);
	}

public:

	void selfCheck() const {
		for (size_t i = 0; i < this->roots.size(); ++i)
			assert(this->taMan->isAlive(this->roots[i]));
	}

	/* execution bits */

	size_t varCount() const {
		return this->variables.size();
	}

	size_t varAdd(const Data& data) {
		size_t id = this->variables.size();
		this->variables.push_back(data);
		return id;
	}

	void varPopulate(size_t count) {
		this->variables.resize(this->variables.size() + count, Data::createUndef());
	}

	void varRemove(size_t count) {
		assert(count <= this->variables.size());
		while (count-- > 0) this->variables.pop_back();
	}

	const Data& varGet(size_t id) const {
		assert(id < this->variables.size());
		return this->variables[id];
	}

	void varSet(size_t id, const Data& data) {
		assert(id < this->variables.size());
		this->variables[id] = data;
	}

	size_t nodeCreate(const std::vector<std::pair<SelData, Data> >& nodeInfo, const Box* typeInfo = NULL) {
		size_t root = this->roots.size();
		TA<label_type>* ta = this->taMan->alloc();
		std::vector<const Box*> label;
		std::vector<size_t> lhs;
		std::vector<size_t> o;
		if (typeInfo)
			label.push_back(typeInfo);
		for (std::vector<std::pair<SelData, Data> >::const_iterator i = nodeInfo.begin(); i != nodeInfo.end(); ++i) {
			SelData sel = i->first;
			Data data = i->second;
			FAE::displToSel(sel, data);
			// label
			label.push_back(&this->boxMan->getSelector(sel));
			// lhs
			lhs.push_back(this->addData(*ta, data));
			// o
			if (data.isRef())
				o.push_back(i->second.d_ref.root);
		}
		FAE::reorderBoxes(label, lhs);
		size_t f = this->freshState();
		ta->addTransition(lhs, &this->labMan->lookup(label), f);
		ta->addFinalState(f);
		this->roots.push_back(ta);
		this->rootMap.push_back(o);
		return root;
	}

	size_t nodeCreate(const std::vector<SelData>& nodeInfo, const Box* typeInfo = NULL) {
		size_t root = this->roots.size();
		TA<label_type>* ta = this->taMan->alloc();
		// build label
		std::vector<const Box*> label;
		if (typeInfo)
			label.push_back(typeInfo);
		for (std::vector<SelData>::const_iterator i = nodeInfo.begin(); i != nodeInfo.end(); ++i)
			label.push_back(&this->boxMan->getSelector(*i));
		// build lhs
		vector<size_t> lhs(nodeInfo.size(), this->addData(*ta, Data::createUndef()));
		// reorder
		FAE::reorderBoxes(label, lhs);
		// fill the rest
		size_t f = this->freshState();
		ta->addTransition(lhs, &this->labMan->lookup(label), f);
		ta->addFinalState(f);
		this->roots.push_back(ta);
		this->rootMap.push_back(std::vector<size_t>());
		return root;
	}

	void nodeDelete(size_t root) {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		// update content of the variables
		for (std::vector<Data>::iterator i = this->variables.begin(); i != this->variables.end(); ++i) {
			if (i->isRef(root))
				*i = Data::createUndef();
		}
		// erase node
		FAE::updateRoot(this->roots[root], *this->taMan, NULL);
		// make all references to this rootpoint dangling
		size_t i = 0;
		for (; i < root; ++i) {
			FAE::updateRoot(this->roots[i], *this->taMan, this->invalidateReference(this->roots[i], root));
			FAE::invalidateReference(this->rootMap[i], root);
		}
		// skip 'root'
		++i;
		for (; i < this->roots.size(); ++i) {
			FAE::updateRoot(this->roots[i], *this->taMan, this->invalidateReference(this->roots[i], root));
			FAE::invalidateReference(this->rootMap[i], root);
		}
		
	}

	void unsafeNodeDelete(size_t root) {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		// erase node
		FAE::updateRoot(this->roots[root], *this->taMan, NULL);
	}

	void nodeLookup(size_t root, size_t offset, Data& data) const {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		for (TA<label_type>::iterator i = this->roots[root]->begin(); i != this->roots[root]->end(); ++i) {
			if (i->rhs() == this->roots[root]->getFinalState()) {
				// only one accepting rule is exppected
				this->transitionLookup(*i, offset, data);
				break;
			}				
		}		
	}	

	void nodeLookup(size_t root, std::vector<std::pair<SelData, Data> >& data) const {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		for (TA<label_type>::iterator i = this->roots[root]->begin(); i != this->roots[root]->end(); ++i) {
			if (i->rhs() == this->roots[root]->getFinalState()) {
				// only one accepting rule is exppected
				this->transitionLookup(*i, data);
				break;
			}				
		}		
	}	

	void nodeLookupMultiple(size_t root, size_t base, const std::vector<size_t>& offsets, Data& data) const {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		std::set<size_t> s;
		for (std::vector<size_t>::const_iterator i = offsets.begin(); i != offsets.end(); ++i)
			s.insert(base + *i);
		for (TA<label_type>::iterator i = this->roots[root]->begin(); i != this->roots[root]->end(); ++i) {
			if (i->rhs() == this->roots[root]->getFinalState()) {
				// only one accepting rule is exppected
				this->transitionLookup(*i, base, s, data);
				break;
			}				
		}		
	}	

	void nodeModify(size_t root, size_t offset, const Data& in, Data& out) {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		TA<label_type> ta(this->taMan->getBackend());
		for (TA<label_type>::iterator i = this->roots[root]->begin(); i != this->roots[root]->end(); ++i) {
			ta.addTransition(*i);
			if (i->rhs() == this->roots[root]->getFinalState()) {
				// only one accepting rule is exppected
				this->transitionModify(ta, *i, offset, in, out);
			}
		}
		TA<label_type>* tmp = this->taMan->alloc();
		ta.unreachableFree(*tmp);
		FAE::updateRoot(this->roots[root], *this->taMan, tmp);
		boost::unordered_map<size_t, vector<size_t> > o;
		FAE::computeDownwardO(*tmp, o);
		this->rootMap[root] = o[tmp->getFinalState()];
	}	

	void nodeModifyMultiple(size_t root, size_t offset, const Data& in, Data& out) {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		assert(in.isStruct());
		std::map<size_t, Data> m;
		for (std::vector<Data::item_info>::const_iterator i = in.d_struct->begin(); i != in.d_struct->end(); ++i)
			m.insert(std::make_pair(i->first + offset, i->second));
		TA<label_type> ta(this->taMan->getBackend());
		for (TA<label_type>::iterator i = this->roots[root]->begin(); i != this->roots[root]->end(); ++i) {
			ta.addTransition(*i);
			if (i->rhs() == this->roots[root]->getFinalState()) {
				// only one accepting rule is exppected
				this->transitionModify(ta, *i, offset, m, out);
			}
		}
		TA<label_type>* tmp = this->taMan->alloc();
		ta.unreachableFree(*tmp);
		FAE::updateRoot(this->roots[root], *this->taMan, tmp);
		boost::unordered_map<size_t, vector<size_t> > o;
		FAE::computeDownwardO(*tmp, o);
		this->rootMap[root] = o[tmp->getFinalState()];
	}	

	void getNearbyReferences(size_t root, std::vector<size_t>& out) const {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		for (TA<label_type>::iterator i = this->roots[root]->begin(); i != this->roots[root]->end(); ++i) {
			if (i->rhs() == this->roots[root]->getFinalState()) {
				for (std::vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j) {
					const Data* data;
					if (this->dataMan.isLeaf(*j, data) && data->isRef())
						out.push_back(data->d_ref.root);
				}
				// only one accepting rule is exppected
				break;
			}				
		}		
	}	

public:

	// state 0 should never be allocated by FAE (?)
	FAE(TA<label_type>::Manager& taMan, LabMan& labMan, BoxManager& boxMan)
	 : FA(taMan), boxMan(&boxMan), labMan(&labMan), stateOffset(1) {
		 // init special root references (this is not required)
//		 this->registerRootReference(varNull, this->freshState());
//		 this->registerRootReference(varUndef, this->freshState());
	}

	FAE(const FAE& x)
		: FA(x), boxMan(x.boxMan), labMan(x.labMan), stateOffset(x.stateOffset), rootMap(x.rootMap),
		selectorMap(x.selectorMap), dataMan(x.dataMan) {
	}
//		rootReferenceIndex(x.rootReferenceIndex), invRootReferenceIndex(x.invRootReferenceIndex) {}

	~FAE() { this->clear();	}
	
	FAE& operator=(const FAE& x) {
		((FA*)this)->operator=(x);
		this->boxMan = x.boxMan;
		this->labMan = x.labMan;
		this->stateOffset = x.stateOffset;
		this->rootMap = x.rootMap;
		this->selectorMap = x.selectorMap;
//		this->refMan = x.refMan;
		this->dataMan = x.dataMan;
		return *this;		
	}

/* WIP */
protected:

/*
	void doAbstraction() {
		this->heightAbstraction(abstract();
	}
*/
/*
	bool x_not_null(size_t x) const {
		assert(x < this->variables.size());
		return this->variables[x].index != varNull;
	}

	bool x_eq_y(size_t x, size_t y) const {
		assert(x < this->variables.size());
		assert(y < this->variables.size());
		if ((this->variables[x].index == varUndef) || (this->variables[y].index == varUndef))
			throw runtime_error("FAE:x_eq_y(): comparing undefined value!");
		return this->variables[x] == this->variables[y];
	}
	
	void x_ass_new(vector<FAE*>& dst, size_t x, size_t pointerSlots, size_t dataSlots) const {
		assert(x < this->variables.size());
		if (dataSlots > 0)
			throw std::runtime_error("FAE::x_ass_new(): Data handling not implemented! (désolé)");
		FAE* fae = new FAE(*this);
		dst.push_back(fae);
		size_t root = fae->roots.size();
		TA<label_type>* ta = fae->taMan->alloc();
		fae->roots.push_back(ta);
		fae->variables[x] = var_info(root, 0);
		// build lhs out of undefs
		vector<size_t> lhs(pointerSlots, fae->addReference(*ta, varUndef));
		// build label out of selectors
		vector<const Box*> label;
		for (size_t i = 0; i < pointerSlots; ++i)
			label.push_back(&fae->boxMan->getSelector(i));
		FAE::reorderBoxes(label, lhs);
		size_t f = fae->freshState();
		ta->addTransition(lhs, &fae->labMan->lookup(label), f);
		ta->addFinalState(f);
		// udate rootMap
		fae->rootMap.push_back(std::vector<size_t>());
		NormInfo normInfo;
		fae->normalize(normInfo);
	}

	void del_x(vector<FAE*>& dst, size_t x) const {
		assert(x < this->variables.size());
		// raise some reasonable exception here
		switch (this->variables[x].index) {
			case varNull: throw runtime_error("FAE::del_x(): destination variable contains NULL!");
			case varUndef: throw runtime_error("FAE::del_x(): destination variable undefined!");
		}
		size_t root = this->variables[x].index;
		this->isolateAtRoot(dst, root);
		for (vector<FAE*>::iterator i = dst.begin(); i != dst.end(); ++i) {
			if ((*i)->variables[x].offset != 0) {
				// TODO: raise some reasonable exception here (instead of runtime_error)
				throw runtime_error("FAE::del_x(): call on a variable pointing inside an allocated block!");
			}
			// update variable content
			for (std::vector<var_info>::iterator j = (*i)->variables.begin(); j != (*i)->variables.end(); ++j) {
				if (j->index == root)
					j->index = varUndef;
			}
			// make all references to this rootpoint dangling
			vector<size_t> index((*i)->roots.size());
			for (size_t j = 0; j < (*i)->roots.size(); ++j)
				index[j] = (j == root)?(varUndef):(j);
			for (size_t j = 0; j < (*i)->roots.size(); ++j) {
				TA<label_type>* ta = (*i)->relabelReferences((*i)->roots[j], index);
				(*i)->taMan->release((*i)->roots[j]);
				(*i)->roots[j] = ta;				
			}
			// normalize
			NormInfo normInfo;
			(*i)->normalize(normInfo, itov(root));
		}
	}

	void x_ass_null(vector<FAE*>& dst, size_t x) const {
		assert(x < this->variables.size());
		FAE* fae = new FAE(*this);
		dst.push_back(fae);
		fae->variables[x] = var_info(varNull, 0);
		NormInfo normInfo;
		fae->normalize(normInfo);
	}
	
	void x_ass_y(vector<FAE*>& dst, size_t x, size_t y, size_t offset) const {
		assert(x < this->variables.size());
		assert(y < this->variables.size());
		FAE* fae = new FAE(*this);
		dst.push_back(fae);
		fae->variables[x] = fae->variables[y];
		fae->variables[x].offset += offset;
		NormInfo normInfo;
		fae->normalize(normInfo);
	}
	
	void x_ass_y_next(vector<FAE*>& dst, size_t x, size_t y, size_t selector) const {
		assert(x < this->variables.size());
		assert(y < this->variables.size());
		// TODO: raise some reasonable exception here (instead of runtime_error)
		switch (this->variables[y].index) {
			case varNull: throw runtime_error("FAE::x_ass_y_next(): source variable contains NULL!");
			case varUndef: throw runtime_error("FAE::x_ass_y_next(): source variable undefined!");
		}
		size_t root = this->variables[y].index;
		selector += this->variables[y].offset;
		assert(root < this->roots.size());
		this->isolateAtRoot(dst, root, itov(selector));
		for (vector<FAE*>::iterator i = dst.begin(); i != dst.end(); ++i) {
			// find the destination of the selector
			for (TA<label_type>::iterator j = (*i)->roots[root]->begin(); j != (*i)->roots[root]->end(); ++j) {
				if (j->rhs() == (*i)->roots[root]->getFinalState()) {
					// only one accepting rule is exppected
					(*i)->findSelectorDestination(*j, selector, (*i)->variables[x].index, (*i)->variables[x].offset);
					break;
				}				
			}		
			NormInfo normInfo;
			(*i)->normalize(normInfo);
		}
	}	

	void x_next_ass_y(std::vector<FAE*>& dst, size_t x, size_t y, size_t selector) const {
		assert(x < this->variables.size());
		assert(y < this->variables.size());
		// TODO: raise some reasonable exception here (instead of runtime_error)
		switch (this->variables[x].index) {
			case varNull: throw runtime_error("FAE::x_ass_y_next(): destination variable contains NULL!");
			case varUndef: throw runtime_error("FAE::x_ass_y_next(): destination variable undefined!");
		}
		size_t root = this->variables[x].index;
		selector += this->variables[x].offset;
		assert(root < this->roots.size());
		std::vector<FAE*> tmp;
		this->isolateAtRoot(tmp, root, itov(selector));
		try {
			for (vector<FAE*>::iterator i = tmp.begin(); i != tmp.end(); ++i) {
				TA<label_type>* ta = (*i)->taMan->alloc();
				for (TA<label_type>::iterator j = (*i)->roots[root]->begin(); j != (*i)->roots[root]->end(); ++j) {
					if (j->rhs() == (*i)->roots[root]->getFinalState()) {
						// only one accepting rule is exppected
						(*i)->changeSelectorDestination(*ta, *j, selector, (*i)->variables[y].index, (*i)->variables[y].offset);
					} else {
						ta->addTransition(*j);
					}
				}		
				ta->addFinalState((*i)->roots[root]->getFinalState());
				(*i)->taMan->release((*i)->roots[root]);
				(*i)->roots[root] = ta;
				boost::unordered_map<size_t, vector<size_t> > o;
				FAE::computeDownwardO(*ta, o);
				(*i)->rootMap[root] = o[ta->getFinalState()];
				NormInfo normInfo;
				(*i)->normalize(normInfo);
			}
		} catch (...) {
			for (vector<FAE*>::iterator i = tmp.begin(); i != tmp.end(); ++i)
				delete *i;
			throw;
		}
		dst.insert(dst.end(), tmp.begin(), tmp.end());
	}	
*/
	// ensures the existence of 'o' (can create more accepting states)
	TA<label_type>* normalizeTA(TA<label_type>* src, o_map_type& o) {
		TA<label_type>* ta = this->taMan->alloc();
		TA<label_type>::bu_cache_type buCache;
		src->buildBUCache(buCache);
		o.clear();
		// <original state, 'o' vector>
		boost::unordered_map<std::pair<size_t, std::vector<size_t> >, size_t> newStates;
		// new state -> original state
		boost::unordered_map<size_t, std::vector<size_t> > aux;
		std::vector<std::pair<size_t, size_t> > stack;
		for (TA<label_type>::iterator i = src->begin(); i != src->end(); ++i) {
			const Data* data;
			if (!i->label().head().isData(data))
				continue;
			vector<size_t> v;
			if (data->isRef())
				v = itov(data->d_ref.root);
			std::pair<boost::unordered_map<std::pair<size_t, std::vector<size_t> >, size_t>::iterator, bool> p =
				newStates.insert(std::make_pair(std::make_pair(i->rhs(), v), this->nextState()));
			ta->addTransition(i->lhs(), i->label(), p.first->second);
			if (!p.second)
				continue;
			this->newState();
			stack.push_back(std::make_pair(i->rhs(), p.first->second));
			aux.insert(std::make_pair(i->rhs(), std::vector<size_t>())).first->second.push_back(p.first->second);
			o[p.first->second] = v;
		}
		while (!stack.empty()) {
			std::pair<size_t, size_t> x = stack.back();
			stack.pop_back();
			TA<label_type>::bu_cache_type::iterator i = buCache.find(x.first);
			if (i == buCache.end())
				continue;
			for (std::vector<const TT<label_type>*>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
				std::vector<std::pair<std::vector<size_t>::const_iterator, std::vector<size_t>::const_iterator> > pool;
				std::vector<std::vector<size_t>::const_iterator> current;
				for (std::vector<size_t>::const_iterator k = (*j)->lhs().begin(); k != (*j)->lhs().end(); ++k) {
					boost::unordered_map<size_t, std::vector<size_t> >::iterator l = aux.find(*k);
					if (l == aux.end())
						break;
					pool.push_back(std::make_pair(l->second.begin(), l->second.end()));
					current.push_back(l->second.begin());
				}
				if (pool.size() != (*j)->lhs().size())
					continue;
				while (current.back() != pool.back().second) {
					vector<size_t> v;
					for (std::vector<std::vector<size_t>::const_iterator>::iterator k = current.begin(); k != current.end(); ++k) {
						std::vector<size_t>& tmp = o[**k];
						v.insert(v.end(), tmp.begin(), tmp.end());
					}
					FAE::removeMultOcc(v);
					std::pair<boost::unordered_map<std::pair<size_t, std::vector<size_t> >, size_t>::iterator, bool> p =
						newStates.insert(std::make_pair(std::make_pair((*j)->rhs(), v), this->nextState()));
					ta->addTransition((*j)->lhs(), (*j)->label(), p.first->second);
					if (!p.second)
						continue;
					this->newState();
					stack.push_back(std::make_pair((*j)->rhs(), p.first->second));
					aux.insert(std::make_pair((*j)->rhs(), std::vector<size_t>())).first->second.push_back(p.first->second);
					o[p.first->second] = v;
					for (size_t k = 0; (++current[k] == pool[k].second) && (k + 1 < pool.size()); ++k)
						current[k] = pool[k].first;
				}
			}
		}
		boost::unordered_map<size_t, std::vector<size_t> >::iterator i = aux.find(src->getFinalState());
		for (std::vector<size_t>::iterator j = i->second.begin(); j != i->second.end(); ++j)
			ta->addFinalState(*j);
		return ta;
	}

	// ensures the given state appears exactly once in each run
	TA<label_type>* propagateReference(TA<label_type>* src, size_t refState) {
		std::vector<std::pair<size_t, size_t> > stack = itov(std::make_pair(refState, refState));
		boost::unordered_map<size_t, size_t> newStates;
		TA<label_type>* ta = this->taMan->clone(src, false);
		TA<label_type>::bu_cache_type buCache;
		ta->buildBUCache(buCache);
		while (!stack.empty()) {
			std::pair<size_t, size_t> x = stack.back();
			stack.pop_back();
			TA<label_type>::bu_cache_type::iterator i = buCache.find(x.first);
			if (i == buCache.end())
				continue;
			for (std::vector<const TT<label_type>*>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
				std::vector<size_t> tmp = (*j)->lhs();
				std::pair<boost::unordered_map<size_t, size_t>::iterator, bool> p =
					newStates.insert(std::make_pair((*j)->rhs(), this->nextState()));
				if (p.second) {
					this->newState();
					stack.push_back(*p.first);
				}
				for (size_t k = 0; k < (*j)->lhs().size(); ++k) {
					if (tmp[k] != x.first)
						continue;
					tmp[k] = x.second;
					ta->addTransition(tmp, (*j)->label(), p.first->second);
					tmp[k] = x.first;
				}
			}
		}
		boost::unordered_map<size_t, size_t>::iterator i = newStates.find(src->getFinalState());
		if (i == newStates.end())
			throw std::runtime_error("FAE::propagateState(): no final state reached in bottom-up traversal!");
		ta->addFinalState(i->second);
		return ta;
	}

	template <class F>
	void split(vector<FAE*>& dst, size_t root, F f) const {
		assert(root < this->roots.size());
//		size_t refState = this->findRootReference(reference);
		TA<label_type>::td_cache_type dfsCache;
		this->roots[root]->buildTDCache(dfsCache);
		for (TA<label_type>::iterator i = this->roots[root]->begin(); i != this->roots[root]->end(); ++i) {
			// is it interesting?
			if (!f(*i))
				continue;
			FAE* fae = new FAE(*this);
			// lower part (renamed)
			TA<label_type>* ta = fae->taMan->alloc();
			Index<size_t> index;
			ta->addFinalState(index.translateOTF(i->rhs()) + fae->nextState());
			for (
				TA<label_type>::td_iterator j = fae->roots[root]->tdStart(dfsCache, itov(i->rhs()));
				j.isValid();
				j.next()
			) {
				const Data* data;
				if (j->label().head().isData(data)) {
					ta->addTransition(*j);
					continue;
				}
				std::vector<size_t> tmp;
				index.translateOTF(tmp, j->lhs(), fae->nextState());
				ta->addTransition(tmp, j->label(), index.translateOTF(j->rhs()) + fae->nextState());
			}
			fae->incrementStateOffset(index.size());
			// the rest
			TA<label_type>* ta2 = fae->taMan->clone(fae->roots[root]);
			size_t state = fae->addData(*ta2, Data::createRef(fae->roots.size()));
			for (TA<label_type>::iterator j = fae->roots[root]->begin(); j != fae->roots[root]->end(); ++j) {
				std::vector<size_t> tmp = j->lhs();
				for (size_t k = 0; k < j->lhs().size(); ++k) {
					if (tmp[k] != i->rhs())
						continue;
					tmp[k] = state;
					ta2->addTransition(tmp, j->label(), j->rhs());
					tmp[k] = i->rhs();
				}
			}
			o_map_type o;
			FAE::computeDownwardO(*ta, o);
			o_map_type::iterator j = o.find(ta->getFinalState());
			assert(j != o.end());
			fae->rootMap[fae->roots.size()] = *j;
			o.clear();
			fae->roots.push_back(ta);
			ta = fae->propagateReference(ta2, state);
			fae->taMan->release(ta2);
			ta2 = fae->normalizeTA(ta, o);
			fae->taMan->release(ta);
			if (ta2->getFinalStates().size() == 1) {
				fae->taMan->release(fae->roots[root]);
				fae->roots[root] = ta2;
				j = o.find(ta2->getFinalState());
				assert(j != o.end());
				fae->rootMap[root] = *j;
				dst.push_back(fae);
				continue;
			}
			// for more accepting states
			for (std::set<size_t>::const_iterator k = ta2->getFinalStates().begin(); k != ta2->getFinalStates().end(); ++k) {
				FAE* fae2 = new FAE(*fae);
				ta = fae2->taMan->clone(ta2, false);
				ta->addFinalState(*k);
				fae->taMan->release(ta2);
				ta2 = fae2->taMan->alloc();
				ta->minimized(*ta2);
				fae2->taMan->release(ta);
				fae2->taMan->release(fae2->roots[root]);
				fae2->roots[root] = ta2;
				j = o.find(ta2->getFinalState());
				assert(j != o.end());
				fae2->rootMap[root] = *j;
				dst.push_back(fae2);
			}
			delete fae;
		}		
	}	

/*
	void traverse(vector<size_t>& order, set<size_t>& marked, vector<size_t>& garbage) {
		// TODO: upward traversal
		order.clear();
		marked.clear();
		set<size_t> visited;
		for (vector<Data>::iterator i = this->variables.begin(); i != variables.end(); ++i) {
			// skip everything what is not a root reference
			if (!i->isPtr())
				continue;
			size_t root = i->d_ptr.root;
			// mark rootpoint pointed by a variable
			marked.insert(root);
			// check whether we traversed this one before
			if (visited.count(root))
				continue;
			this->visitDown(root, visited, order, marked);
		}
		garbage.clear();
		for (size_t i = 0; i < this->roots.size(); ++i) {
			if (!visited.count(i))
				garbage.push_back(i);
		}
	}
*/
/*
	void computeIndex(const std::vector<size_t>& order, std::vector<size_t>& index) {
		index = std::vector<size_t>(order.size(), (size_t)(-1));
		size_t offset = 0;
		for (std::vector<size_t>::const_iterator i = order.begin(); i < order.end(); ++i) {
			assert(*i < index.size());
			index[*i] = offset++;
		}
	}

	// reorder roots according to the order (do not relabel leaves)
	void reorderRoots(const std::vector<size_t>& index) {
		vector<TA<label_type>*> oldRoots = this->roots;
		vector<vector<size_t> > oldRootMap = this->rootMap;
		// update representation
		for (size_t i = 0; i < this->roots.size(); ++i) {
			this->roots[i] = oldRoots[index[i]];
			this->rootMap[i] = oldRootMap[index[i]];
		}
		// update variables
		for (std::vector<var_info>::iterator i = this->variables.begin(); i != this->variables.end(); ++i) {
			if (!FAE::isNullOrUndef(i->index))
				i->index = index[i->index];
		}
	}

	// relabel references
	void relabelReferences(const std::vector<size_t>& index) {
		for (size_t i = 0; i < this->roots.size(); ++i) {
			TA<label_type>* ta = this->roots[i];
			this->roots[i] = this->relabelReferences(ta, index);
			this->taMan->release(ta);
			FAE::renameVector(this->rootMap[i], index);
		}
	}

	// merge roots (reordered, but not renamed)
	void mergeRoots(const std::vector<size_t>& order, const std::set<size_t>& marked, std::vector<std::vector<size_t> >& revInfo) {
		for (size_t i = 0, last = (size_t)(-1); i < this->roots.size(); ++i) {
			if (marked.count(order[i]) == 1) {
				this->roots[++last] = this->roots[i];
				revInfo.push_back(itov(last));
			} else {
				assert(last != (size_t)(-1));
				size_t refState;
				TA<label_type>* ta = this->mergeRoot(this->roots[last], order[i], this->roots[i], refState);
				this->taMan->release(this->roots[last]);
				this->roots[last] = ta;
				FAE::updateMap(this->rootMap[last], order[i], this->rootMap[i]);
				revInfo.back().push_back(refState);
			}
		}
	}
*/
	
};

#endif
