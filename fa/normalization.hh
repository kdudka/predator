/*
 * Copyright (C) 2011 Jiri Simacek
 *
 * This file is part of forester.
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

#ifndef NORMALIZATION_H
#define NORMALIZATION_H

#include <vector>
#include <map>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "treeaut.hh"
#include "forestautext.hh"
#include "abstractbox.hh"
#include "utils.hh"
/*
struct NormInfo {

	struct RootInfo {

		size_t index;
		std::vector<std::pair<size_t, std::vector<size_t> > > mergedRoots;

		RootInfo(size_t index) : index(index) {}

		void initRoots(FAE& fae) const {
			fae.roots[this->index] = fae.taMan->alloc();
			for (std::vector<std::pair<size_t, std::vector<size_t> > >::const_iterator i = this->mergedRoots.begin(); i != this->mergedRoots.end(); ++i)
				fae.roots[i->first] = fae.taMan->alloc();				
		}

		friend std::ostream& operator<<(std::ostream& os, const RootInfo& rootInfo) {
			os << rootInfo.index << '|';
			for (std::vector<std::pair<size_t, std::vector<size_t> > >::const_iterator i = rootInfo.mergedRoots.begin(); i != rootInfo.mergedRoots.end(); ++i) {
				os << i->first << ':';
				for (std::vector<size_t>::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
					os << ' ' << *j;
			}
			return os;
		}

	};

	size_t rootCount;
//		std::vector<RootInfo> data;
	std::map<size_t, RootInfo> data;

	NormInfo() {}

	void clear() {
		this->rootCount = 0;
		this->data.clear();
	}

	void addRoot(size_t index) {
		bool b = this->data.insert(std::make_pair(index, RootInfo(index))).second;
		assert(b);
	}

	void mergeRoots(size_t dst, size_t src, const std::vector<size_t>& refStates) {
		std::map<size_t, RootInfo>::iterator i = this->data.find(dst);
		assert(i != this->data.end());
		std::map<size_t, RootInfo>::iterator j = this->data.find(src);
		assert(j != this->data.end());
		i->second.mergedRoots.push_back(std::make_pair(src, refStates));
		for (std::vector<std::pair<size_t, std::vector<size_t> > >::iterator k = j->second.mergedRoots.begin(); k != j->second.mergedRoots.end(); ++k)
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

	void check() const {
		size_t i = 0;
		for (std::map<size_t, RootInfo>::const_iterator j = this->data.begin(); j != this->data.end(); ++j)
			i += j->second.mergedRoots.size() + 1;
		assert(i == this->rootCount);			
	}
	
};*/

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
		if (this->fae.isData(t1->rhs(), data)) {
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
	std::set<std::pair<size_t, size_t> >& splitPoints;
	const TA<label_type>& src1;
	const TA<label_type>& src2;
	boost::unordered_map<size_t, size_t> rootMap;
/*	
	IntersectAndRelabelSpecialF(FAE& fae, TA<label_type>& dst, std::set<std::pair<size_t, size_t> >& splitPoints, const std::vector<size_t>& index, const TA<label_type>& src1, const TA<label_type>& src2, const NormInfo::RootInfo& rootInfo)
		: fae(fae), dst(dst), index(index), splitPoints(splitPoints), src1(src1), src2(src2) {
		for (std::vector<std::pair<size_t, std::vector<size_t> > >::const_iterator i = rootInfo.mergedRoots.begin(); i != rootInfo.mergedRoots.end(); ++i) {
			for (std::vector<size_t>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				bool b = this->rootMap.insert(std::make_pair(*j, i->first)).second;
				assert(b);
			}
		}
	}
*/
	void operator()(const TT<label_type>* t1, const TT<label_type>* t2, const std::vector<size_t>& lhs, size_t& rhs) {
		const Data* data;
		if (this->fae.isData(t1->rhs(), data)) {
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
			this->splitPoints.insert(std::make_pair(rhs, i->second));
	}

};

class Normalization {

	FAE& fae;

protected:

	TA<label_type>* mergeRoot(TA<label_type>& dst, size_t ref, TA<label_type>& src, std::vector<size_t>& joinStates) {
		assert(ref < this->fae.roots.size());
		TA<label_type>* ta = this->fae.allocTA();
		ta->addFinalStates(dst.getFinalStates());
		size_t refState = _MSB_ADD(this->fae.boxMan->getDataId(Data::createRef(ref)));
		boost::unordered_map<size_t, size_t> joinStatesMap;
		for (std::set<size_t>::const_iterator i = src.getFinalStates().begin(); i != src.getFinalStates().end(); ++i) {
			joinStates.push_back(this->fae.nextState());
			joinStatesMap.insert(std::make_pair(*i, this->fae.freshState()));
		}
		bool hit = false;
		for (TA<label_type>::iterator i = dst.begin(); i != dst.end(); ++i) {
			std::vector<size_t> tmp = i->lhs();
			std::vector<size_t>::iterator j = std::find(tmp.begin(), tmp.end(), refState);
			if (j != tmp.end()) {
				for (std::vector<size_t>::iterator k = joinStates.begin(); k != joinStates.end(); ++k) {
					*j = *k;
					ta->addTransition(tmp, i->label(), i->rhs());
				}
				hit = true;
			} else ta->addTransition(*i);
		}
//		std::cerr << joinState << std::endl;
		assert(hit);
		// avoid screwing up things
		src.unfoldAtRoot(*ta, joinStatesMap, false);
		return ta;
	}

	void visitDown(size_t c, std::vector<bool>& visited, std::vector<size_t>& order, std::vector<bool>& marked) const {
		if (visited[c]) {
			marked[c] = true;
			return;
		}
		visited[c] = true;
		order.push_back(c);
		for (std::vector<std::pair<size_t, bool> >::const_iterator i = this->fae.rootMap[c].begin(); i != this->fae.rootMap[c].end(); ++i) {
			this->visitDown(i->first, visited, order, marked);
			if (i->second)
				marked[i->first] = true;
		}
	}

	void visitDown(size_t c, std::vector<bool>& visited) const {
		if (visited[c])
			return;
		visited[c] = true;
		for (std::vector<std::pair<size_t, bool> >::const_iterator i = this->fae.rootMap[c].begin(); i != this->fae.rootMap[c].end(); ++i)
			this->visitDown(i->first, visited);
	}

	void traverse(std::vector<bool>& visited, std::vector<size_t>& order, std::vector<bool>& marked) const {
		// TODO: upward traversal
		visited = std::vector<bool>(this->fae.roots.size(), false);
		marked = std::vector<bool>(this->fae.roots.size(), false);
		order.clear();
		for (std::vector<Data>::const_iterator i = this->fae.variables.begin(); i != this->fae.variables.end(); ++i) {
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
		visited = std::vector<bool>(this->fae.roots.size(), false);
		for (std::vector<Data>::const_iterator i = this->fae.variables.begin(); i != this->fae.variables.end(); ++i) {
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

	void checkGarbage(const std::vector<bool>& visited) const {
		for (size_t i = 0; i < this->fae.roots.size(); ++i) {
			if (!visited[i] && (this->fae.roots[i] != NULL)) {
//				std::cerr << "the root " << i << " is not referenced anymore ... " << std::endl;
				throw ProgramError("garbage detected");
			}
		}
	}

public:

	// check garbage
	void check() const {
		// compute reachable roots
		std::vector<bool> visited(this->fae.roots.size(), false);
		this->traverse(visited);
		// check garbage
		this->checkGarbage(visited);
	}

	void normalizeRoot(/*NormInfo& normInfo, */std::vector<bool>& normalized, size_t root, const std::vector<bool>& marked) {
		if (normalized[root])
			return;
		normalized[root] = true;
		std::vector<std::pair<size_t, bool> > tmp = this->fae.rootMap[root];
//		normInfo.addRoot(root);
		for (std::vector<std::pair<size_t, bool> >::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			this->normalizeRoot(/*normInfo, */normalized, i->first, marked);
			if (!marked[i->first]) {
//				std::cerr << "merging " << *i << '(' << this->fae.roots[*i] << ')' << " into " << root << '(' << this->fae.roots[root] << ')' << std::endl;
				std::vector<size_t> refStates;
				TA<label_type>* ta = this->mergeRoot(*this->fae.roots[root], i->first, *this->fae.roots[i->first], refStates);
				this->fae.roots[root] = std::shared_ptr<TA<label_type>>(ta);
				this->fae.roots[i->first] = NULL;
				FAE::updateMap(this->fae.rootMap[root], i->first, this->fae.rootMap[i->first]);
//				normInfo.mergeRoots(root, i->first, refStates);
			}
		}
	}

	// normalize representation
	void normalize(/*NormInfo& normInfo, */const std::set<size_t>& forbidden = std::set<size_t>()) {

		// compute canonical root ordering
		std::vector<size_t> order;
		std::vector<bool> visited(this->fae.roots.size(), false), marked(this->fae.roots.size(), false);

		this->traverse(visited, order, marked);

		// check garbage
//		this->checkGarbage(visited);

		// prevent merging of forbidden roots
		for (std::set<size_t>::const_iterator i = forbidden.begin(); i != forbidden.end(); ++i)
			marked[*i] = true;

		bool normalizationNeeded = false;
		for (size_t i = 0; i < order.size(); ++i) {
			if (marked[i] && (order[i] == i))
				continue;
			normalizationNeeded = true;
			break;
		}

		if (!normalizationNeeded) {
			this->fae.roots.resize(order.size());
			this->fae.rootMap.resize(order.size());
			return;
		}

		// reindex roots
		std::vector<size_t> index(this->fae.roots.size(), (size_t)(-1));
		std::vector<bool> normalized(this->fae.roots.size(), false);
		std::vector<std::shared_ptr<TA<label_type>>> newRoots;
		std::vector<std::vector<std::pair<size_t, bool> > > newRootMap;
		size_t offset = 0;
		for (std::vector<size_t>::iterator i = order.begin(); i < order.end(); ++i) {
			this->normalizeRoot(/*normInfo, */normalized, *i, marked);
//			assert(marked[*i] || (this->fae.roots[*i] == NULL));
			if (!marked[*i])
				continue;
			newRoots.push_back(this->fae.roots[*i]);
			newRootMap.push_back(this->fae.rootMap[*i]);
			index[*i] = offset++;
//			normInfo.addRoot(*i);
		}
//		normInfo.rootCount = this->fae.roots.size();
//		normInfo.reindex(index);
		// update representation
		this->fae.roots = newRoots;
		this->fae.rootMap = newRootMap;
		for (size_t i = 0; i < this->fae.roots.size(); ++i) {
			this->fae.roots[i] = std::shared_ptr<TA<label_type>>(this->fae.relabelReferences(this->fae.roots[i].get(), index));
			FAE::renameVector(this->fae.rootMap[i], index);
		}
		// update variables
		for (std::vector<Data>::iterator i = this->fae.variables.begin(); i != this->fae.variables.end(); ++i) {
			if (i->isRef()) {
				assert(index[i->d_ref.root] != (size_t)(-1));
				i->d_ref.root = index[i->d_ref.root];
			}
		}
	}

	// single accepting in, (single accepting out?)
	void split(std::vector<TA<label_type>*>& dst, const TA<label_type>& src, size_t baseIndex, const std::set<std::pair<size_t, size_t> >& splitPoints) {

		Index<size_t> stateIndex;

		TA<label_type>::td_cache_type cache;
		src.buildTDCache(cache);

		boost::unordered_map<size_t, size_t> splitMap;

		for (std::set<size_t>::const_iterator i = src.getFinalStates().begin(); i != src.getFinalStates().end(); ++i)
			dst[baseIndex]->addFinalState(stateIndex.translateOTF(*i) + this->fae.stateOffset);
		for (std::set<std::pair<size_t, size_t> >::const_iterator i = splitPoints.begin(); i != splitPoints.end(); ++i) {
			splitMap.insert(std::make_pair(i->first, i->second));
			dst[i->second]->addFinalState(stateIndex.translateOTF(i->first) + this->fae.stateOffset);
		}

//		std::cerr << src;

		std::vector<std::pair<size_t, size_t> > stack;
		for (std::set<size_t>::const_iterator i = src.getFinalStates().begin(); i != src.getFinalStates().end(); ++i)
			stack.push_back(std::make_pair(*i, baseIndex));

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
						state = this->fae.addData(*dst[p.second], Data::createRef(l->second));
					} else {
						stack.push_back(std::make_pair(*k, p.second));
						if (FA::isData(*k))
							state = *k;
						else
							state = stateIndex.translateOTF(*k) + this->fae.stateOffset;
					}
					lhs.push_back(state);
				}
				size_t rhs = (*j)->rhs();
				if (!FA::isData(rhs))
					rhs = stateIndex.translateOTF(rhs) + this->fae.stateOffset;
				dst[p.second]->addTransition(lhs, (*j)->label(), rhs);
			}
		}

		this->fae.incrementStateOffset(stateIndex.size());
	}
/*
	bool denormalize(const FAE& fae, const NormInfo& normInfo) {
		assert(fae.roots.size() == normInfo.data.size());
		assert(this->fae.roots.size() == fae.roots.size());
		
		FAE tmp(this->fae);
		for (std::vector<TA<label_type>*>::iterator i = this->fae.roots.begin(); i != this->fae.roots.end(); ++i)
			this->fae.updateRoot(*i, NULL);

		this->fae.rootMap.resize(normInfo.rootCount);

		std::vector<size_t> index(normInfo.data.size(), (size_t)(-1));

		size_t ii = 0;

		for (std::map<size_t, NormInfo::RootInfo>::const_iterator i = normInfo.data.begin(); i != normInfo.data.end(); ++i, ++ii) {
			index[ii] = i->second.index;
			this->fae.rootMap[i->second.index] = tmp.rootMap[ii];
		}

		for (size_t i = 0; i < this->fae.rootMap.size(); ++i)
			FAE::renameVector(this->fae.rootMap[i], index);

		normInfo.initRoots(this->fae);

		for (size_t i = 0; i != normInfo.data.size(); ++i) {
			TA<label_type>::lt_cache_type cache1, cache2;
			tmp.roots[i]->buildLTCache(cache1);
			fae.roots[i]->buildLTCache(cache2);
			std::map<size_t, NormInfo::RootInfo>::const_iterator j = normInfo.data.find(i);
			assert(j != normInfo.data.end());
			size_t stateCount;
			if (j->second.mergedRoots.size() == 0) {
				stateCount = TA<label_type>::buProduct(
					cache1,
					cache2,
					IntersectAndRelabelF(
						this->fae, *this->fae.roots[index[i]], index, *tmp.roots[i], *fae.roots[i]
					),
					this->fae.nextState()
				);
			} else {
				TA<label_type> ta(this->fae.taMan->getBackend());
				std::set<std::pair<size_t, size_t> > splitPoints;
				stateCount = TA<label_type>::buProduct(
					cache1,
					cache2,
					IntersectAndRelabelSpecialF(
						this->fae, ta, splitPoints, index, *tmp.roots[i], *fae.roots[i], j->second
					),
					this->fae.nextState()
				);
				this->split(this->fae.roots, ta, index[i], splitPoints);
			}
			this->fae.incrementStateOffset(stateCount);
		}

		for (size_t i = 0; i < this->fae.roots.size(); ++i) {
			if (!this->fae.roots[i])
				continue;
			this->fae.updateRoot(this->fae.roots[i], &this->fae.roots[i]->uselessAndUnreachableFree(*this->fae.taMan->alloc()));
			if (this->fae.roots[i]->getFinalStates().empty())
				return false;
		}

		return true;

	}
*/
public:

	Normalization(FAE& fae) : fae(fae) {}

};

#endif
