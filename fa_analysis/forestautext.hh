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

#include <boost/unordered_map.hpp>

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

std::ostream& operator<<(std::ostream& os, const TA<FA::label_type>& ta);

class FAE : public FA {

	mutable BoxManager* boxMan;
	mutable LabMan* labMan;

	size_t stateOffset;

	vector<vector<size_t> > rootMap;
	
	// root -> (selector -> root)
	boost::unordered_map<size_t, map<size_t, size_t> > selectorMap;

	boost::unordered_map<size_t, size_t> rootReferenceIndex;
	boost::unordered_map<size_t, size_t> invRootReferenceIndex;

public:

	void clear() {
		FA::clear();
		this->stateOffset = 1;
		this->rootMap.clear();
		this->selectorMap.clear();
		this->rootReferenceIndex.clear();
		this->invRootReferenceIndex.clear();
	}
	
	void loadTA(const TA<label_type>& src, const TA<label_type>::td_cache_type& cache, const TT<label_type>* top, size_t stateOffset) {
		this->clear();
		this->variables = *top->label().data;
		this->stateOffset = stateOffset;
		for (vector<size_t>::const_iterator i = top->lhs().begin(); i != top->lhs().end(); ++i) {
			TA<label_type>* ta = this->taMan->alloc();
			this->roots.push_back(ta);
			// add reachable transitions
			for (TA<label_type>::dfs_iterator j = src.dfsStart(cache, {*i}); j.isValid(); j.next()) {
				ta->addTransition(*j);
				if (j->lhs().empty())
					this->registerRootReference(j->label().head().getReference(), j->rhs());
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
	
	void registerRootReference(size_t root, size_t state) {
		if (!this->rootReferenceIndex.insert(make_pair(root, state)).second)
			throw runtime_error("FAE::registerRootReference(): root reference already registered!");
		if (!this->invRootReferenceIndex.insert(make_pair(state, root)).second)
			throw runtime_error("FAE::registerRootReference(): state <-> root reference association corrupted!");
	}

	size_t addRootReference(TA<label_type>& dst, size_t root) {
		pair<boost::unordered_map<size_t, size_t>::iterator, bool> p =
			this->rootReferenceIndex.insert(make_pair(root, this->nextState()));
		if (p.second) {
			this->invRootReferenceIndex.insert(make_pair(this->nextState(), root));
			this->newState();
		}
		vector<const Box*> label = { &this->boxMan->getReference(root) };
		dst.addTransition(vector<size_t>(), &this->labMan->lookup(label), p.first->second);
		return p.first->second;
	}

	bool isRootReference(size_t state, size_t& reference) const {
		boost::unordered_map<size_t, size_t>::const_iterator i = this->invRootReferenceIndex.find(state);
		if (i == this->invRootReferenceIndex.end())
			return false;
		reference = i->second;
		return true;		
	}
	
	size_t removeRootReference(size_t reference) {
		boost::unordered_map<size_t, size_t>::iterator i = this->rootReferenceIndex.find(reference);
		if (i == this->rootReferenceIndex.end())
			throw runtime_error("FAE::removeRootReference(): root reference not found!");
		size_t state = i->second;
		boost::unordered_map<size_t, size_t>::iterator j = this->invRootReferenceIndex.find(state);
		if (j == this->invRootReferenceIndex.end())
			throw runtime_error("FAE::registerRootReference(): state <-> root reference association corrupted!");
		this->rootReferenceIndex.erase(i);
		this->invRootReferenceIndex.erase(j);
		return state;
	}
	
	static bool isNullOrUndef(size_t root) {
		return root == varNull || root == varUndef;
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

	static void renameVector(vector<size_t>& dst, vector<size_t>& index) {
		for (vector<size_t>::iterator i = dst.begin(); i != dst.end(); ++i) {
			assert(index[*i] != (size_t)(-1));
			*i = index[*i];
		}
	}

	static void updateMap(vector<size_t>& dst, size_t ref, const vector<size_t>& src) {
		vector<size_t> res;
		vector<size_t>::iterator i = std::find(dst.begin(), dst.end(), ref);
		assert(i != dst.end());
		std::copy(dst.begin(), i, res.end());
		std::copy(src.begin(), src.end(), res.end());
		std::copy(i + 1, dst.end(), res.end());
		FAE::removeMultOcc(res);
		std::swap(dst, res);
	}

	TA<label_type>* relabelReferences(TA<label_type>* src, const vector<size_t>& index) {
		TA<label_type>* ta = this->taMan->alloc();
		ta->addFinalState(src->getFinalState());
		for (TA<label_type>::iterator i = src->begin(); i != src->end(); ++i) {
			vector<size_t> lhs;
			for (vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j) {
				size_t ref;
				if (!this->isRootReference(*j, ref) || FAE::isNullOrUndef(ref) || index[ref] == ref) {
					lhs.push_back(*j);
				} else {
					lhs.push_back(this->addRootReference(*ta, index[ref]));
				}
			}
			ta->addTransition(
				lhs, i->label(), i->rhs()
			);
		}
		return ta;
	}

	TA<label_type>* mergeRoot(TA<label_type>* dst, size_t refName, TA<label_type>* src) {
		assert(refName < this->roots.size());
		TA<label_type>* ta = this->taMan->alloc();
		ta->addFinalState(dst->getFinalState());
		size_t refState = (size_t)(-1);
		for (TA<label_type>::iterator i = dst->begin(); i != dst->end(); ++i) {
			if (i->label().head().isReference(refName)) {
				assert(refState == (size_t)(-1));
				refState = this->removeRootReference(refName);
				assert(refState == i->rhs());
			} else {
				ta->addTransition(*i);
			}
		}
		assert(refState != (size_t)(-1));
		// avoid screwing things up
		src->unfoldAtRoot(*ta, refState, false);
		return ta;
	}

	static bool updateO(boost::unordered_map<size_t, vector<size_t> >& o, size_t state, const vector<size_t>& v) {
		pair<boost::unordered_map<size_t, vector<size_t> >::iterator, bool> p =
			o.insert(make_pair(state, v));
		if (p.second)
			return true;
		if (p.first->second.size() > v.size())
			throw runtime_error("FAE::updateO(): Inconsistent update of 'o' (length mismatch)!");
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
	static void computeDownwardO(const TA<label_type>& ta, boost::unordered_map<size_t, vector<size_t> >& o) {
		o.clear();
		bool changed = true;
		while (changed) {
			changed = false;
			for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {
				vector<size_t> v;
				if (i->label().head().isReference()) {
					size_t ref = i->label().head().getReference();
					if (!FAE::isNullOrUndef(ref)) {
						v = { ref };
					}
				} else {
//					vector<size_t> order;
//					FAE::evaluateLhsOrder(*i->label().dataB, order);
					for (vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j) {
						boost::unordered_map<size_t, vector<size_t> >::iterator k = o.find(*j);
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
		boost::unordered_map<size_t, size_t> index;
		for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {
			if (i->label().head().isReference(ref))
				index.insert(make_pair(i->rhs(), 1));
		}
		bool changed = true;
		while (changed) {
			changed = false;
			for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {
				if (!i->label().head().isReference()) {
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

	void visitDown(size_t c, set<size_t>& visited, vector<size_t>& order, set<size_t>& marked) {
		if (!visited.insert(c).second) {
			marked.insert(c);
			return;
		}
		order.push_back(c);
		for (vector<size_t>::iterator i = this->rootMap[c].begin(); i != this->rootMap[c].end(); ++i) {
			this->visitDown(*i, visited, order, marked);
			if ((marked.count(*i) == 0) && (!FAE::isUniqueRef(*this->roots[c], *i)))
				marked.insert(*i);
		}
	}

	void traverse(vector<size_t>& order, set<size_t>& marked, vector<size_t>& garbage) {
		// TODO: upward traversal
		order.clear();
		marked.clear();
		set<size_t> visited;
		for (vector<var_info>::iterator i = this->variables.begin(); i != variables.end(); ++i) {
			size_t root = i->index;
			// skip nulls and undefined values
			if (FAE::isNullOrUndef(root))
				continue;
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

	// normalize representation
	void normalize(const vector<size_t>& requiredGarbage = vector<size_t>()) {
		vector<size_t> order, garbage;
		set<size_t> marked;
		this->traverse(order, marked, garbage);
		if (garbage != requiredGarbage) {
			// TODO: raise some reasonable exception here (instead of runtime_error)
			throw runtime_error("FAE::normalize(): garbage missmatch!");
		}
		for (vector<size_t>::iterator i = garbage.begin(); i != garbage.end(); ++i)
			this->taMan->release(this->roots[*i]);
		vector<size_t> index(this->roots.size(), (size_t)(-1));
		size_t offset = 0;
		vector<TA<label_type>*> newRoots;
		vector<vector<size_t> > newRootMap;
		for (vector<size_t>::iterator i = order.begin(); i < order.end(); ++i) {
			if (marked.count(*i)) {
				newRoots.push_back(this->taMan->addRef(this->roots[*i]));
				newRootMap.push_back(this->rootMap[*i]);
				index[*i] = offset++;
			} else {
				assert(newRoots.size());
				TA<label_type>* ta = this->mergeRoot(newRoots.back(), *i, this->roots[*i]);
				this->taMan->release(newRoots.back());
				newRoots.back() = ta;
				FAE::updateMap(newRootMap.back(), *i, this->rootMap[*i]);
			}
		}
		// update representation
		for (vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i) {
			if (*i)
				this->taMan->release(*i);
		}
		this->roots.resize(offset);
		this->rootMap.resize(offset);
		for (size_t i = 0; i < this->roots.size(); ++i) {
			this->roots[i] = this->relabelReferences(newRoots[i], index);
			this->taMan->release(newRoots[i]);
			FAE::renameVector(newRootMap[i], index);
			this->rootMap[i] = newRootMap[i];
		}
		// update variables
		for (std::vector<var_info>::iterator i = this->variables.begin(); i != this->variables.end(); ++i) {
			if (!FAE::isNullOrUndef(i->index))
				i->index = index[i->index];
		}
	}

	void decomposeAtRoot(vector<FAE*>& dst, size_t root, const vector<size_t>& selectors) const {
		throw runtime_error("FAE::decomposeAtRoot(): box decomposition not implemented! (désolé)");
	}

	// ensures that the given selectors (or at least the boxes which contain them) become "free" in the result
	void isolateAtRoot(vector<FAE*>& dst, size_t root, const vector<size_t>& selectors = vector<size_t>()) const {
		set<size_t> sSelectors(selectors.begin(), selectors.end());
		boost::unordered_map<size_t, map<size_t, size_t> >::const_iterator i = this->selectorMap.find(root);
		if (i != this->selectorMap.end())
			throw runtime_error("FAE::isolateRoot(): some selectors are hidden somewhere else! (not implemented yet)");
		assert(root < this->roots.size());
		for (TA<label_type>::iterator i = this->roots[root]->begin(); i != this->roots[root]->end(); ++i) {
			if (i->rhs() == this->roots[root]->getFinalState()) {
				FAE* fae = new FAE(*this);
				Guard<FAE> guard(fae);
				TA<label_type>* ta = fae->taMan->clone(this->roots[root], false);
				vector<size_t> lhs;
				size_t lhsOffset = 0;
				bool needsDecomposition = false;
				for (vector<const Box*>::const_iterator j = i->label().dataB->begin(); j != i->label().dataB->end(); ++j) {
					// if called with selectors being empty then it isolates everything
					if (!selectors.empty() && !utils::checkIntersection((*j)->getDownwardCoverage(0).second, sSelectors)) {
						// this box is not interesting
						for (size_t k = 0; k < (*j)->getArity(); ++k, ++lhsOffset)
							lhs.push_back(i->lhs()[lhsOffset]);
					} else {
						// we have to isolate here
						for (size_t k = 0; k < (*j)->getArity(); ++k, ++lhsOffset) {
							size_t reference;
							if (this->isRootReference(i->lhs()[lhsOffset], reference)) {
								// no need to create a reference when it's already there
								lhs.push_back(i->lhs()[lhsOffset]);
							} else {
								// update new left-hand side
								lhs.push_back(fae->addRootReference(*ta, fae->roots.size()));
								// prepare new root
								TA<label_type>* tmp = fae->taMan->clone(fae->roots[root], false);
								tmp->addFinalState(i->lhs()[lhsOffset]);
								TA<label_type>* tmp2 = fae->taMan->alloc();
								tmp->unreachableFree(*tmp2);
								fae->taMan->release(tmp);
								// compute 'o'
								boost::unordered_map<size_t, vector<size_t> > o;
								FAE::computeDownwardO(*tmp2, o);
								fae->roots.push_back(tmp2);
								fae->rootMap.push_back(o[tmp2->getFinalState()]);
							}
						}
						needsDecomposition = (*j)->isBox();
					}
				}
				size_t newState = fae->freshState();
				ta->addTransition(lhs, i->label(), newState);
				ta->addFinalState(newState);
				// exchange the original automaton with a new one
				fae->taMan->release(fae->roots[root]);
				fae->roots[root] = ta;
				boost::unordered_map<size_t, vector<size_t> > o;
				FAE::computeDownwardO(*ta, o);
				fae->rootMap[root] = o[ta->getFinalState()];
				if (needsDecomposition) {
					fae->decomposeAtRoot(dst, root, selectors);
				} else {
					dst.push_back(fae);
					guard.release();
				}
			}
		}
	}
	
	void findSelectorDestination(const TT<label_type>& transition, size_t selector, size_t& dest, size_t& offset) const {
		dest = varUndef;
		offset = 0;
		size_t lhsOffset = 0;
		bool found = false;
		for (vector<const Box*>::const_iterator i = transition.label().dataB->begin(); i != transition.label().dataB->end(); ++i) {
			if ((*i)->isSelector(selector)) {
				assert(!found);
				if (!this->isRootReference(transition.lhs()[lhsOffset], dest))
					throw runtime_error("FAE::findSelectorDestination(): destination is not a reference!");
				offset = (*i)->getSelectorOffset();
				found = true;
			}
			lhsOffset += (*i)->getArity();
		}
		// TODO: emit some warning here
	}

	void changeSelectorDestination(TA<label_type>& dst, const TT<label_type>& transition, size_t selector, size_t dest, size_t offset) {
		vector<size_t> lhs;
		vector<const Box*> label;
		size_t lhsOffset = 0;
		bool found = false;
		for (vector<const Box*>::const_iterator i = transition.label().dataB->begin(); i != transition.label().dataB->end(); ++i) {
			if ((*i)->isSelector(selector)) {
				assert(!found);
				lhs.push_back(this->addRootReference(dst, dest));
				label.push_back(&this->boxMan->getSelector(selector, offset));
				found = true;
			} else {
				lhs.insert(lhs.end(), transition.lhs().begin() + lhsOffset, transition.lhs().begin() + lhsOffset + (*i)->getArity());
				label.push_back(*i);
			}
			lhsOffset += (*i)->getArity();
		}
		if (!found)
			throw runtime_error("FAE::changeSelectorDestination(): pointer slot not defined!");
		FAE::reorderBoxes(label, lhs);
		dst.addTransition(lhs, &this->labMan->lookup(label), transition.rhs());
	}

public:

	// state 0 should never be allocated by FAE
	FAE(TA<label_type>::Manager& taMan, LabMan& labMan, BoxManager& boxMan)
	 : FA(taMan), boxMan(&boxMan), labMan(&labMan), stateOffset(1) {
		 // init special root references (this is not required)
		 this->registerRootReference(varNull, varNull);
		 this->registerRootReference(varUndef, varUndef);
	}

	FAE(const FAE& x)
	 : FA(x), boxMan(x.boxMan), labMan(x.labMan), stateOffset(x.stateOffset), rootMap(x.rootMap),
	 selectorMap(x.selectorMap), rootReferenceIndex(x.rootReferenceIndex), invRootReferenceIndex(x.invRootReferenceIndex) {}

	~FAE() {
		this->clear();
	}
	
	FAE& operator=(const FAE& x) {
		((FA*)this)->operator=(x);
		this->boxMan = x.boxMan;
		this->labMan = x.labMan;
		this->stateOffset = x.stateOffset;
		this->rootMap = x.rootMap;
		this->selectorMap = x.selectorMap;
		this->rootReferenceIndex = x.rootReferenceIndex;
		this->invRootReferenceIndex = x.invRootReferenceIndex;
		return *this;		
	}

/* execution bits */
	void addVar(vector<FAE*>& dst, size_t& id) const {
		FAE* fae = new FAE(*this);
		dst.push_back(fae);
		id = fae->variables.size();
		fae->variables.push_back(var_info(varUndef, 0));
	}
	
	void dropVars(vector<FAE*>& dst, size_t count) const {
		FAE* fae = new FAE(*this);
		dst.push_back(fae);
		assert(count <= fae->variables.size());
		while (count-- > 0) fae->variables.pop_back();
		fae->normalize();
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
		vector<size_t> lhs(pointerSlots, fae->addRootReference(*ta, varUndef));
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
		fae->normalize();
	}

	void del_x(vector<FAE*>& dst, size_t x) const {
		assert(x < this->variables.size());
		// raise some reasonable exception here
		switch (this->variables[x].index) {
			case varNull: throw runtime_error("FAE::x_ass_y_next(): destination variable contains NULL!");
			case varUndef: throw runtime_error("FAE::x_ass_y_next(): destination variable undefined!");
		}
		size_t root = this->variables[x].index;
		this->isolateAtRoot(dst, root);
		for (vector<FAE*>::iterator i = dst.begin(); i != dst.end(); ++i) {
			if ((*i)->variables[x].offset != 0) {
				// TODO: raise some reasonable exception here (instead of runtime_error)
				throw runtime_error("FAE::del_x(): call on a variable pointing inside allocated block!");
			}
			(*i)->variables[x].index = varUndef;
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
			(*i)->normalize({root});
		}
	}
	
	void x_ass_null(vector<FAE*>& dst, size_t x) const {
		assert(x < this->variables.size());
		FAE* fae = new FAE(*this);
		dst.push_back(fae);
		fae->variables[x] = var_info(varNull, 0);
		fae->normalize();
	}
	
	void x_ass_y(vector<FAE*>& dst, size_t x, size_t y, size_t offset) const {
		assert(x < this->variables.size());
		assert(y < this->variables.size());
		FAE* fae = new FAE(*this);
		dst.push_back(fae);
		fae->variables[x] = fae->variables[y];
		fae->variables[x].offset += offset;
		fae->normalize();
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
		this->isolateAtRoot(dst, root, {selector});
		for (vector<FAE*>::iterator i = dst.begin(); i != dst.end(); ++i) {
			// find the destination of the selector
			for (TA<label_type>::iterator j = (*i)->roots[root]->begin(); j != (*i)->roots[root]->end(); ++j) {
				if (j->rhs() == (*i)->roots[root]->getFinalState()) {
					// only one accepting rule is exppected
					(*i)->findSelectorDestination(*j, selector, (*i)->variables[x].index, (*i)->variables[x].offset);
					break;
				}				
			}		
			(*i)->normalize();
		}
	}	

	void x_next_ass_y(vector<FAE*>& dst, size_t x, size_t y, size_t selector) const {
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
		this->isolateAtRoot(dst, root, {selector});
		for (vector<FAE*>::iterator i = dst.begin(); i != dst.end(); ++i) {
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
			(*i)->normalize();
		}
	}	

	friend std::ostream& operator<<(std::ostream& os, const TA<label_type>& ta) {
		TAWriter<label_type>(os).writeOne(ta);
		return os;
	}

	friend std::ostream& operator<<(std::ostream& os, const FAE& fae) {
		os << "variables:";
		for (std::vector<var_info>::const_iterator i = fae.variables.begin(); i != fae.variables.end(); ++i)
			os << ' ' << *i;
		os << std::endl << "roots:" << std::endl;
		for (size_t i = 0; i < fae.roots.size(); ++i) {
			std::ostringstream ss;
			ss << "root" << i << '_';
			for (size_t j = 0; j < fae.rootMap[i].size(); ++j)
				ss << fae.rootMap[i][j];
			TAWriter<FA::label_type>(os).writeOne(*fae.roots[i], ss.str());
		}
		return os;
	}

};

#endif
