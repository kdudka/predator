#ifndef FOREST_AUT_EXT_H
#define FOREST_AUT_EXT_H

#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <algorithm>

#include <boost/unordered_map.hpp>

#include "box.hh"
#include "labman.hh"
#include "forestaut.hh"
#include "utils.hh"

using std::vector;
using std::set;
using std::map;
using std::pair;
using std::make_pair;
using std::runtime_error;

class FAE : public FA {

	mutable BoxManager& boxMan;
	mutable LabMan& labMan;

	size_t stateOffset;

	vector<vector<size_t> > rootMap;
	
	// root -> (selector -> root)
	boost::unordered_map<size_t, map<size_t, size_t> > selectorMap;

	boost::unordered_map<size_t, size_t> rootReferenceIndex;
	boost::unordered_map<size_t, size_t> invRootReferenceIndex;

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
	
	size_t addRootReference(TA<label_type>& dst, size_t root) {
		pair<boost::unordered_map<size_t, size_t>::iterator, bool> p =
			this->rootReferenceIndex.insert(make_pair(root, this->nextState()));
		if (p.second) {
			this->invRootReferenceIndex.insert(make_pair(this->nextState(), root));
			this->newState();
		}
		vector<const Box*> label = { &this->boxMan.getReference(root) };
		dst.addTransition(vector<size_t>(), &this->labMan.lookup(label), p.first->second);
		return p.first->second;
	}

	bool isRootReference(size_t state, size_t& reference) const {
		boost::unordered_map<size_t, size_t>::const_iterator i = this->invRootReferenceIndex.find(state);
		if (i == this->invRootReferenceIndex.end())
			return false;
		reference = i->second;
		return true;		
	}
	
	static bool containsBox(label_type label) {
		for (vector<const Box*>::const_iterator i = label.dataB->begin(); i != label.dataB->end(); ++i) {
			if ((*i)->isBox())
				return true;
		}
		return false;
	}
/*
	void setRootReference(size_t state, size_t reference) {
		this->root_reference_index[state] = reference;
	}
*/
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
		TA<label_type>* ta = this->taMan.alloc();
		for (TA<label_type>::iterator i = src->begin(); i != src->end(); ++i) {
			if (i->label().head().isReference()) {
				size_t ref = i->label().head().getReference();
				if (index[ref] == ref) {
					ta->addTransition(*i);
				} else {
					assert(index[ref] != (size_t)(-1));
					vector<const Box*> label({ &this->boxMan.getReference(index[ref]) });
					ta->addTransition(
						vector<size_t>(), &this->labMan.lookup(label), i->rhs()
					);
				}
			} else {
				ta->addTransition(*i);
			}
		}
		return ta;
	}

	TA<label_type>* mergeRoot(TA<label_type>* dst, size_t refName, TA<label_type>* src) const {
		assert(refName < this->roots.size());
		TA<label_type>* ta = this->taMan.alloc();
		ta->addFinalState(dst->getFinalState());
		size_t refState = (size_t)(-1);
		for (TA<label_type>::iterator i = dst->begin(); i != dst->end(); ++i) {
			if (i->label().head().isReference(refName)) {
				assert(refState == (size_t)(-1));
				refState = i->rhs();
			} else {
				ta->addTransition(*i);
			}
		}
		assert(refState != (size_t)(-1));
		// avoid screwing things up
		src->unfoldAtRoot(*ta, refState, false);
		return ta;
	}
/*
	static void evaluateLhsOrder(const vector<const Box*>& label, vector<size_t>& order) {
		boost::unordered_map<size_t, size_t> m;
		order.clear();
		size_t offset = 0;
		for (vector<const Box*>::const_iterator i = label.begin(); i != label.end(); ++i) {
			const vector<size_t>& selectors = (*i)->getDownwardCoverage(0).first;
			for (vector<size_t>::const_iterator j = selectors.begin(); j != selectors.end(); ++j) {
				if (!m.insert(make_pair(*j, offset++)).second)
					throw runtime_error("FAE::evaluateLhsOrder(): A selector was defined more than once!");
			}
		}
		for (map<size_t, size_t>::iterator i = m.begin(); i != m.end(); ++i)
			order.push_back(i->second);
	}
*/
	static bool updateO(boost::unordered_map<size_t, vector<size_t> >& o, size_t state, const vector<size_t>& v) {
		pair<boost::unordered_map<size_t, vector<size_t> >::iterator, bool> p =
			o.insert(make_pair(state, v));
		if (p.second)
			return true;
		if (p.first->second.size() > v.size())
			throw runtime_error("FAE::updateO(): Inconsistent update of 'o' (length mismatch)!");
		for (size_t i = 0; i < p.first->second.size(); ++i) {
			if (v[i] != p.first->second[i])
				throw runtime_error("FAE::updateO(): Inconsistent update of 'o' (prefix mismatch)!");
		}
		if (p.first->second.size() != v.size()) {
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
					if (ref != varNull && ref != varUndef)
						v = { ref };
				} else {
//					vector<size_t> order;
//					FAE::evaluateLhsOrder(*i->label().dataB, order);
					for (vector<size_t>::iterator j = i->lhs().begin(); j != i->lhs().end(); ++j) {
						boost::unordered_map<size_t, vector<size_t> >::iterator k = o.find(i->lhs()[*j]);
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
	void normalize(const vector<size_t>& requiredGarbage) {
		vector<size_t> order, garbage;
		set<size_t> marked;
		this->traverse(order, marked, garbage);
		if (garbage != requiredGarbage) {
			// TODO: raise some reasonable exception here (instead of runtime_error)
			throw runtime_error("FAE::normalize(): garbage missmatch!");
		}
		for (vector<size_t>::iterator i = garbage.begin(); i != garbage.end(); ++i)
			this->taMan.release(this->roots[*i]);
		vector<size_t> index(this->roots.size(), (size_t)(-1));
		size_t offset = 0;
		vector<TA<label_type>*> newRoots;
		vector<vector<size_t> > newRootMap;
		for (vector<size_t>::iterator i = order.begin(); i < order.end(); ++i) {
			if (marked.count(*i)) {
				newRoots.push_back(this->taMan.addRef(this->roots[*i]));
				newRootMap.push_back(this->rootMap[*i]);
				index[*i] = offset++;
			} else {
				assert(newRoots.size());
				TA<label_type>* ta = this->mergeRoot(newRoots.back(), *i, this->roots[*i]);
				this->taMan.release(newRoots.back());
				newRoots.back() = ta;
				FAE::updateMap(newRootMap.back(), *i, this->rootMap[*i]);
			}
		}
		// update representation
		for (vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i) {
			if (*i)
				this->taMan.release(*i);
		}
		this->roots.resize(offset);
		this->rootMap.resize(offset);
		for (size_t i = 0; i < this->roots.size(); ++i) {
			this->roots[i] = this->relabelReferences(newRoots[i], index);
			this->taMan.release(newRoots[i]);
			FAE::renameVector(newRootMap[i], index);
			this->rootMap[i] = newRootMap[i];
		}
		
	}

	void decomposeAtRoot(vector<FAE*>& dst, size_t root, const vector<size_t>& selectors) const {
		throw runtime_error("FAE::decomposeAtRoot(): box decomposition not implemented! (désolé)");
	}
/*	
	void decomposeAtRoot(vector<FAE*>& dst, size_t root) const {
		throw runtime_error("FAE::decomposeAtRoot(): boxes not implemented! (désolé)");
	}

	void 
*/
	// ensures that the given selectors (or at least the boxes which contain them) become "free" in the result
	void isolateAtRoot(vector<FAE*>& dst, size_t root, const vector<size_t>& selectors) const {
		set<size_t> sSelectors(selectors.begin(), selector.end());
		boost::unordered_map<size_t, map<size_t, size_t> >::iterator i = this->selectorMap.find(root);
		if (i != this->selectorMap.end())
			throw runtime_error("FAE::isolateRoot(): some selectors are hidden somewhere else! (not implemented yet)");
		for (TA<label_type>::iterator i = this->roots[root]->begin(); i != this->roots[root]->end(); ++i) {
			if (i->rhs() == this->roots[root]->getFinalState()) {
				// TODO: check that the following value is properly released (in case f an exception)
				FAE* fae = new FAE(*this);
				TA<label_type>* ta = fae->taMan.clone(this->roots[root], false);
				vector<size_t> lhs;
				size_t lhsOffset = 0;
				bool needsDecomposition = false;
				for (vector<const Box*>::const_iterator j = i->label().dataB->begin(); j != i->label().dataB->end(); ++j) {
					if (!utils::checkIntersection((*j)->getDownwardCoverage(0).second, sSelectors)) {
						// this box is not interesting
						for (size_t k = 0; k < (*j)->getArity(); ++k, ++lhsOffset)
							lhs.push_back(i->lhs()[lhsOffsset]);
					} else {
						// we have to isolate
						for (size_t k = 0; k < (*j)->getArity(); ++k, ++lhsOffset) {
							if (this->isRootReference(i->lhs()[lhsOffset], reference)) {
								// no need to create a reference when it's already there
								lhs.push_back(i->lhs()[lhsOffset]);
							} else {
								// update new left-hand-side
								lhs.push_back(fae->addRootReference(*ta, fae->roots.size()));
								// prepare new root
								TA<label_type>* tmp = fae->taMan.clone(fae->roots[root], false);
								tmp->addFinalState(i->lhs()[lhsOffset]);
								TA<label_type>* tmp2 = fae->taMan.alloc();
								tmp->unreachableFree(*tmp2);
								fae->taMan.release(tmp);
								fae->roots.push_back(tmp2);
							}
						}
						needsDecomosition = (*j)->isBox();
					}
				}
				size_t newState = fae->freshState();
				ta->addTransition(lhs, i->label(), newState);
				ta->addFinalState(newState);
				// exchange the original automaton with a new one
				fae->taMan.release(fae->roots[root]);
				fae->roots[root] = ta;
				if (needsDecomposition) {
					fae->decomposeAtRoot(dst, root, selectors);
					delete fae;
				} else dst.push_back(fae);
			}
		}
	}
/*
	// returns configurations with isolated accepting rules, flag is set if we need to decompose further on
	void isolateRoot(vector<pair<FAE*, bool> >& dst, size_t root) const {
		boost::unordered_map<size_t, map<size_t, size_t> >::iterator i = this->selectorMap.find(root);
		if (i != this->selectorMap.end())
			throw runtime_error("FAE::isolateRoot(): some selectors are hidden somewhere else! (not implemented yet)");
		for (TA<label_type>::iterator i = this->roots[root]->begin(); i != this->roots[root]->end(); ++i) {
			if (i->rhs() == this->roots[root]->getFinalState()) {
				FAE* fae = new FAE(*this);
				TA<label_type>* ta = fae->taMan.alloc();
				vector<size_t> lhs;
				for (size_t j = 0; j < i->lhs().size(); ++j) {
					size_t reference;
					if (this->isRootReference(i->lhs()[j], reference)) {
						// update new left-hand-side
						lhs.push_back(fae->addRootReference(*ta, reference));
						continue;
					}
					// update new left-hand-side
					lhs.push_back(fae->addRootReference(*ta, fae->roots.size()));
					// prepare new root
					TA<label_type>* tmp = fae->taMan.clone(fae->roots[root]);
					tmp->clearFinalStates();
					tmp->addFinalState(i->lhs()[j]);
					TA<label_type>* tmp2 = fae->taMan.alloc();
					tmp->unreachableFree(*tmp2);
					fae->taMan.release(tmp);
					fae->roots.push_back(tmp2);
				}
				ta->addTransition(lhs, i->label(), i->rhs());
				ta->addFinalState(i->rhs());
				// exchange the original automaton with a new one
				fae->taMan.release(fae->roots[root]);
				fae->roots[root] = ta;
				dst.push_back(make_pair(fae, FAE::containsBox(i->label())));
			}
		}
	}

	void createRootpoint(vector<pair<FAE*, bool> >& dst, size_t root, size_t selector) const {
		for (TA<label_type>::iterator i = this->roots[root]->begin(); i != this->roots[root]->end(); ++i) {
			if (i->rhs() == this->roots[root]->getFinalState()) {
				FAE* fae = new FAE(*this);
				TA<label_type>* ta = fae->taMan.alloc();
				vector<size_t> lhs;
				for (size_t j = 0; j < i->lhs().size(); ++j) {
					size_t reference;
					if (this->isRootReference(i->lhs()[j], reference)) {
						// update new left-hand-side
						lhs.push_back(fae->addRootReference(*ta, reference));
						continue;
					}
					// update new left-hand-side
					lhs.push_back(fae->addRootReference(*ta, fae->roots.size()));
					// prepare new root
					TA<label_type>* tmp = fae->taMan.clone(fae->roots[root]);
					tmp->clearFinalStates();
					tmp->addFinalState(i->lhs()[j]);
					TA<label_type>* tmp2 = fae->taMan.alloc();
					tmp->unreachableFree(*tmp2);
					fae->taMan.release(tmp);
					fae->roots.push_back(tmp2);
				}
				ta->addTransition(lhs, i->label(), i->rhs());
				ta->addFinalState(i->rhs());
				// exchange the original automaton with a new one
				fae->taMan.release(fae->roots[root]);
				fae->roots[root] = ta;
				dst.push_back(make_pair(fae, FAE::containsBox(i->label())));
			}
		}
	}
*/
public:

	// state 0 should never be allocated by FAE
	FAE(TAManager<FA::label_type>& taMan, LabMan& labMan, BoxManager& boxMan)
	 : FA(taMan), boxMan(boxMan), labMan(labMan), stateOffset(1) {}

	FAE(const FAE& x)
	 : FA(x), boxMan(x.boxMan), labMan(x.labMan), stateOffset(x.stateOffset), rootMap(x.rootMap),
	 selectorMap(x.selectorMap), rootReferenceIndex(rootReferenceIndex), invRootReferenceIndex(invRootReferenceIndex) {}

/* execution bits */
	size_t newVar() {
		size_t id = this->variables.size();
		this->variables.push_back(var_info(varUndef, 0));
		return id;
	}
	
	void dropVars(size_t count) {
		assert(count <= this->variables.size());
		while (count-- > 0) this->variables.pop_back();
	}
	
	bool x_eq_y(size_t x, size_t y) {
		return this->variables[x] == this->variables[y];
	}
/*	
	void x_ass_new(vector<FAE*>& dst, size_t x, size_t pointerSlots, size_t dataSlots) const {
		if (dataSlots > 0)
			throw std::runtime_error("FAE::x_ass_new(): Data handling not implemented! (désolé)");
		// TODO: identify garbage
		TA<label_type>* ta = this->taMan.alloc();
		this->variables[x] = var_info(this->roots.size(), 0);
		this->roots.push_back(ta);
		set<pair<const Box*, size_t> > tmp;
		for (size_t i = 0; i < pointerSlots; ++i) {
			// create fresh state
			size_t s = this->freshState();
			// create leaf rule
//			vector<const Box*> label = { &this->boxMan.lookup(Box::createReference(varUndef)) };
//			ta->addTransition(vector<size_t>(), &this->labMan.lookup(label) , s);
//			this->setRootReference(s, varUndef);
			// prepare the rest
//			tmp.insert(make_pair(&this->boxMan.lookup(Box::createPrimitive(i)), s));
		}
		vector<size_t> lhs;
		vector<const Box*> label;
		for (set<pair<const Box*, size_t> >::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			lhs.push_back(i->second);
			label.push_back(i->first);
		}
		size_t f = this->freshState();
		ta->addTransition(lhs, &label, f);
		ta->addFinalState(f);		
		// TODO: reorder
	}
*/	
	void del_x(vector<FAE*>& dst, size_t x) const {
		dst.clear();
		size_t root = this->variables[x].index;
		vector<pair<FAE*, bool> > tmp;
		this->isolateRoot(tmp, root);
		for (vector<pair<FAE*, bool> >::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			if (i->second) {
				i->first->decomposeAtRoot(dst, root);
				delete i->first;
			} else {
				dst.push_back(i->first);
			}
		}
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
				(*i)->taMan.release((*i)->roots[j]);
				(*i)->roots[j] = ta;				
			}
			// normalize
			(*i)->normalize({root});
		}
	}
	
	void x_ass_null(vector<FAE*>& dst, size_t x) const {
		FAE* fae = new FAE(*this);
		fae->variables[x] = var_info(varNull, 0);
		fae->normalize({});
		dst.push_back(fae);
	}
	
	void x_ass_y(vector<FAE*>& dst, size_t x, size_t y, int offset) {
		FAE* fae = new FAE(*this);
		fae->variables[x] = fae->variables[y];
		fae->variables[x].offset += offset;
		fae->normalize({});
		dst.push_back(fae);
	}
	
	void x_ass_y_next(size_t x, size_t y, size_t selector) {
		// TODO: identify garbage
		
	}	

};

#endif
