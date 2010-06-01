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

//	boost::unordered_map<size_t, size_t> root_reference_index;

protected:

	size_t freshState() {
		return this->stateOffset++;
	}
/*
	bool isRootReference(size_t state, size_t& reference) {
		boost::unordered_map<size_t, size_t>::iterator i = this->root_reference_index.find(state);
		if (i == this->root_reference_index.end())
			return false;
		reference = i->second;
		return true;		
	}

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

	// replaces this->roots[src] by null
	TA<label_type>* mergeRoot(TA<label_type>* dst, size_t refName, TA<label_type>* src) {
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
		src->unfoldAtRoot(*ta, refState);
		return ta;
	}

	static void evaluateLhsOrder(const vector<const Box*>& label, vector<size_t>& order) {
		map<size_t, size_t> m;
		order.clear();
		size_t offset = 0;
		for (vector<const Box*>::const_iterator i = label.begin(); i != label.end(); ++i) {
			const vector<size_t>& selectors = (*i)->getDownwardCoverage(0).first;
			for (vector<size_t>::const_iterator j = selectors.begin(); j != selectors.end(); ++j) {
				if (m.insert(make_pair(*j, offset++)).second)
					throw runtime_error("FAE::evaluateLhsOrder(): A selector was defined more than once!");
			}
		}
		for (map<size_t, size_t>::iterator i = m.begin(); i != m.end(); ++i)
			order.push_back(i->second);
	}

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
					v = { i->label().head().getReference() };
				} else {
					vector<size_t> order;
					FAE::evaluateLhsOrder(*i->label().dataB, order);
					for (vector<size_t>::iterator j = order.begin(); j != order.end(); ++j) {
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
	void normalize() {
		vector<size_t> order, garbage;
		set<size_t> marked;
		this->traverse(order, marked, garbage);
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

	// try to indetify which roots to merge
	void collect() {
		
	}

	void reorderHeap(const vector<size_t>& variables) {
	}

public:

	FAE(TAManager<FA::label_type>& taMan, LabMan& labMan, BoxManager& boxMan)
	 : FA(taMan), boxMan(boxMan), labMan(labMan), stateOffset(1) {}

/* execution bits */
	size_t newVar() {
		size_t id = this->variables.size();
		this->variables.push_back(var_info(varUndef, 0));
		return id;
	}
	
	void dropVars(size_t count) {
		assert(count <= this->variables.size());
		while (count--) this->variables.pop_back();
	}
	
	bool x_eq_y(size_t x, size_t y) {
		return this->variables[x] == this->variables[y];
	}
	
	void x_ass_new(size_t x, size_t pointerSlots, size_t dataSlots) {
		if (dataSlots > 0)
			throw std::runtime_error("Data handling not implemented! (désolé)");
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
	
	void x_ass_null(size_t x) {
		// TODO: identify garbage
		this->variables[x] = var_info(varNull, 0);
		// TODO: reorder
	}
	
	void x_ass_y(size_t x, size_t y, int offset) {
		// TODO: identify garbage
		this->variables[x] = this->variables[y];
		this->variables[x].offset += offset;
		// TODO: reorder
	}
	
	void x_ass_y_next(size_t x, size_t y, size_t selector) {
		// TODO: identify garbage
		
	}	

};

#endif
