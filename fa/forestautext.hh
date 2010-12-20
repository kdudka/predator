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
#include "boxman.hh"
#include "labman.hh"
#include "tatimint.hh"
#include "utils.hh"
#include "programerror.hh"

using std::vector;
using std::set;
using std::map;
using std::pair;
using std::make_pair;
using std::runtime_error;
/*
struct PG {

	template <class T>
	static void reset(std::vector<T>& p) {
		std::sort(p.begin(), p.end());
	}

	//--------------------------------------------------------
	// Generate next permutation (algorithm from Rosen p. 284)
	//--------------------------------------------------------
	template <class T>
	static bool next(std::vector<T>& p) {
		assert(p.size());

    	// find largest index j with p[j] < p[j+1]
	    int j;
    	for (j = p.size() - 2; j >= 0 && p[j] > p[j + 1]; --k);

    	if (j < 0)
    		return false;

		// find index k such that p[k] is smallest integer
		// greater than p[j] to the right of p[j]
		size_t k;
		for (k = p.size() - 1; p[j] > p[k]; --k);

		// interchange p[j] and p[k]
		T tmp = p[k];
		p[k] = p[j];
		p[j] = tmp;

		// put tail end of permutation after jth position in increasing order
		while (size_t r = a.size() - 1, s = j + 1; r > s; --r, ++s) {
			tmp = p[s];
			p[s] = p[r];
			p[r] = tmp;
		}

		return true;
	}

};
*/
template <class T, class F>
struct TAIsom {

	const TA<T>& ta1;
	const TA<T>& ta2;
	F f;
	boost::unordered_map<size_t, size_t> sMatching;
	std::vector<boost::unordered_map<size_t, size_t>::iterator> sStack;

	TAIsom(const TA<T>& ta1, const TA<T>& ta2, F f) : ta1(ta1), ta2(ta2), f(f) {}

	bool matchLabels(const std::vector<const TT<label_type>*>& v1, const std::vector<const TT<label_type>*>& v2) {
		for (size_t i = 0; i < v1.size(); ++i) {
			if (!this->f(*v1[i], *v2[i]))
				return false;				
		}
		return true;
	}

	bool matchLhss(const std::vector<const TT<label_type>*>& v1, const std::vector<const TT<label_type>*>& v2) {
		for (size_t i = 0; i < v1.size(); ++i) {
			for  (size_t j = 0; j < v1[i]->lhs().size(); ++j) {
				if (!this->matchStates(v1[i]->lhs()[j], v2[i]->lhs()[j]))
					return false;
			}
		}
		return true;
	}

	bool matchStates(size_t s1, size_t s2) {
		std::pair<typename boost::unordered_map<size_t, size_t>::iterator, bool> p
			= this->sMatching.insert(std::make_pair(s1, s2));
		if (!p.second)
			return p.first->second == s2;
		std::vector<const TT<label_type>*> v1, v2;
		for (typename TA<T>::iterator i = this->ta1.begin(s1); i != this->ta1.end(s1); ++i)
			v1.push_back(&*i);
		for (typename TA<T>::iterator i = this->ta2.begin(s2); i != this->ta2.end(s2); ++i)
			v2.push_back(&*i);
		if (v1.size() != v2.size())
			return false;
		this->sStack.push_back(p.first);
		size_t mark = this->sStack.size();
		std::sort(v2.begin(), v2.end());
		do {
			if (this->matchLabels(v1, v2) && this->matchLhss(v1, v2))
				return true;
			for (size_t i = mark; i < this->sStack.size(); ++i)
				this->sMatching.erase(this->sStack[i]);
			this->sStack.resize(mark);
		} while (std::next_permutation(v2.begin(), v2.end()));
		return false;
	}

	bool run() {
		return this->matchStates(this->ta1.getFinalState(), this->ta2.getFinalState());
	}

};

class FAE : public FA {

	friend std::ostream& operator<<(std::ostream& os, const FAE& fae) {
//		TA<label_type>::Backend backend;
//		TA<label_type> ta(backend);
/*		os << "vars";
		for (std::vector<Data>::const_iterator i = fae.variables.begin(); i != fae.variables.end(); ++i)
			os << ' ' << *i;
		os << std::endl;*/
		for (size_t i = 0; i < fae.roots.size(); ++i) {
			if (!fae.roots[i])
				continue;
			os << "===" << std::endl << "root " << i << " o=[";
			for (size_t j = 0; j < fae.rootMap[i].size(); ++j)
				os << fae.rootMap[i][j];
			os << ']';
			TA<label_type>& ta = *fae.roots[i];
//			ta.clear();
//			fae.roots[i]->minimized(ta);
			TAWriter<label_type> writer(os);
			for (std::set<size_t>::iterator j = ta.getFinalStates().begin(); j != ta.getFinalStates().end(); ++j)
				writer.writeState(*j);
			writer.endl();
			writer.writeTransitions(ta, FA::WriteStateF());
//			TAWriter<label_type>(os).writeOne(*ta, ss.str());
//			TAWriter<label_type>(os).writeOne(*fae.roots[i], ss.str());
		}
		return os;
	}

	mutable BoxMan* boxMan;
	mutable LabMan* labMan;

	size_t stateOffset;

	std::vector<vector<size_t> > rootMap;

	void updateRootMap(size_t root) {
		assert(root < this->roots.size());
		boost::unordered_map<size_t, std::vector<size_t> > o;
		FAE::computeDownwardO(*this->roots[root], o);
		this->rootMap[root] = o[this->roots[root]->getFinalState()];
	}

	// root x selector -> root
	boost::unordered_map<std::pair<size_t, size_t>, size_t> selectorMap;

/*
	struct RootInfo {
		
		struct BoxInfo {
			size_t root;
			const TT<label_type>* transition;
			const Box* box;
		};

		boost::unordered_map<const Box*, BoxInfo> boxMap;

	};
	
	// root -> RootInfo
	boost::unordered_map<size_t, RootInfo> boxMap;
*/

//	boost::unordered_map<size_t, size_t> rootReferenceIndex;
//	boost::unordered_map<size_t, size_t> invRootReferenceIndex;

public:

	struct RenameNonleafF {

		Index<size_t>& index;

		size_t offset;
		
		RenameNonleafF(Index<size_t>& index, size_t offset = 0)
			: index(index), offset(offset) {}

		size_t operator()(size_t s) {
			if (_MSB_TEST(s))
				return s;
			return this->index.translateOTF(s) + this->offset;
		}

	};

protected:

	void boxMerge(TA<label_type>& dst, const TA<label_type>& src, const TA<label_type>& boxRoot, const Box* box, const std::vector<size_t>& rootIndex) {
		TA<label_type> tmp(this->taMan->getBackend()), tmp2(this->taMan->getBackend());
		this->relabelReferences(tmp, boxRoot, rootIndex);
		Index<size_t> stateIndex;
		TA<label_type>::rename(tmp2, tmp, RenameNonleafF(stateIndex, this->nextState()));
		this->incrementStateOffset(stateIndex.size());
		tmp2.copyTransitions(dst, TA<label_type>::NonAcceptingF(tmp2));
		dst.addFinalState(tmp2.getFinalState());
		for (TA<label_type>::iterator i = src.accBegin(); i != src.accEnd(i); ++i) {
			std::vector<size_t> lhs;
			std::vector<const AbstractBox*> label;
			size_t lhsOffset = 0;
			if (box) {
				bool found = false;
				for (std::vector<const AbstractBox*>::const_iterator j = i->label().dataB->begin(); j != i->label().dataB->end(); ++j) {
					if (!(*j)->isStructural()) {
						label.push_back(*j);
						continue;
					}
					StructuralBox* b = (StructuralBox*)(*j);
					if (b != (const StructuralBox*)box) {
						// this box is not interesting
						for (size_t k = 0; k < b->getArity(); ++k, ++lhsOffset)
							lhs.push_back(i->lhs()[lhsOffset]);
						label.push_back(b);
						continue;
					}
					assert(!found);
					found = true;
				}
				assert(found);
			} else {
				lhs = i->lhs();
				label = *i->label().dataB;
			}
			for (TA<label_type>::iterator j = tmp2.accBegin(); j != tmp2.accEnd(j); ++j) {
				std::vector<size_t> lhs2 = lhs;
				std::vector<const AbstractBox*> label2 = label;
				lhs2.insert(lhs2.end(), j->lhs().begin(), j->lhs().end());
				label2.insert(label2.end(), j->label().dataB->begin(), j->label().dataB->end());
				FAE::reorderBoxes(label2, lhs2);
				dst.addTransition(lhs2, &this->labMan->lookup(label2), i->rhs());
			}
		}
	}

	void unfoldBox(size_t root, const Box* box) {
		assert(this->roots[root]);
		assert(box);
		const TT<label_type>& t = this->roots[root]->getAcceptingTransition();
		size_t lhsOffset = 0;
		std::vector<size_t> bSig = box->getO(0);
		std::vector<size_t> sig;
		for (std::vector<const AbstractBox*>::const_iterator i = t.label().dataB->begin(); i != t.label().dataB->end(); ++i) {
			if ((const AbstractBox*)box != *i) {
				lhsOffset += (*i)->getArity();
				continue;
			}
			for (size_t j = 0; j < box->getArity(); ++j) {
				size_t ref;
				bool b = this->getRef(t.lhs()[lhsOffset + j], ref);
				assert(b);
				sig.push_back(ref);
			}
			break;
		}
		// compute index
		std::vector<size_t> index(box->getArity() + 1, (size_t)(-1));
		index[0] = root;
		for (size_t i = 0; i < sig.size(); ++i) {
			assert(bSig[i] < index.size());
			index[bSig[i]] = sig[i];
		}
		TA<label_type>* ta = this->taMan->alloc();
		this->boxMerge(*ta, *this->roots[root], *box->getRoot(0), box, index);
		this->updateRoot(this->roots[root], ta);
		this->updateRootMap(root);
		for (size_t i = 0; i < box->getArity(); ++i) {
			ta = this->taMan->alloc();
			this->boxMerge(*ta, *this->roots[index[i]], *box->getRoot(i), NULL, index);
			this->updateRoot(this->roots[index[i]], ta);
			this->updateRootMap(index[i]);
		}
	}

	struct IsomF {

		std::vector<size_t>& index;
		
		IsomF(std::vector<size_t>& index) : index(index) {}

		// TODO: revise
		bool operator()(const TT<label_type>& t1, const TT<label_type>& t2) {
			size_t ref1, ref2;
			if (!FAE::getRef(t1.label().head(), ref1))
				return (t1.label() == t2.label());
			if (!FAE::getRef(t2.label().head(), ref2))
				return false;
			if ((this->index[ref2] != (size_t)(-1)) && (this->index[ref2] == ref1))
				return false;
			assert(ref2 < this->index.size());
			this->index[ref2] = ref1;
			return true;
		}

	};

	static void copyBox(std::vector<size_t>& lhs, std::vector<const AbstractBox*>& label, const AbstractBox* box, const std::vector<size_t>& srcLhs, size_t srcOffset) {
		for (size_t i = 0; i < box->getArity(); ++i, ++srcOffset)
			lhs.push_back(srcLhs[srcOffset]);
		label.push_back(box);
	}

	bool boxCut(TA<label_type>& dst, TA<label_type>& complement, const TA<label_type>& src, const std::set<const AbstractBox*>& trigger) {
		dst.addFinalState(src.getFinalState());
		complement.addFinalState(src.getFinalState());
		src.copyTransitions(dst, TA<label_type>::NonAcceptingF(src));
		dst.copyTransitions(complement);
		for (TA<label_type>::Iterator i = src.accBegin(); i != src.accEnd(i); ++i) {
			std::vector<size_t> lhs, cLhs;
			std::vector<const AbstractBox*> label, cLabel;
			size_t lhsOffset = 0;
			const TT<label_type>& t = *i;
			size_t matched = 0;
			for (std::vector<const AbstractBox*>::const_iterator j = t.label().dataB->begin(); j != t.label().dataB->end(); ++j) {
				if (trigger.count(*j)) {
					FAE::copyBox(cLhs, cLabel, *j, t.lhs(), lhsOffset);
					++matched;
				} else {
					FAE::copyBox(lhs, label, *j, t.lhs(), lhsOffset);
				}
			}
			if (matched == trigger.size()) {
				FAE::reorderBoxes(label, lhs);
				dst.addTransition(lhs, &this->labMan->lookup(label), src.getFinalState());
				FAE::reorderBoxes(cLabel, cLhs);
				complement.addTransition(cLhs, &this->labMan->lookup(cLabel), src.getFinalState());
			} else return false;
		}
		return true;
	}

	bool foldBox(size_t root, const Box* box) {
		assert(this->roots[root]);
		assert(box);
		TA<label_type> tmp(this->taMan->getBackend()), cTmp(this->taMan->getBackend());
		if (!this->boxCut(tmp, cTmp, *this->roots[root], box->getTrigger(0)))
			return false;
		o_map_type o;
		FAE::computeDownwardO(cTmp, o);
		const std::vector<size_t>& sig = box->getO(0);
		std::vector<size_t> cSig = o[cTmp.getFinalState()];
		if (sig.size() != cSig.size())
			return false;

		// compute index
		std::vector<size_t> index(box->getArity() + 1, (size_t)(-1));
		index[root] = 0;
		for (size_t i = 0; i < sig.size(); ++i) {
			assert(sig[i] < index.size());
			index[sig[i]] = cSig[i];
		}

		// match inputs
		std::vector<std::pair<TA<label_type>, TA<label_type> > > iTmp;
		for (size_t i = 0; i < cSig.size(); ++i) {
			iTmp.push_back(std::make_pair(TA<label_type>(this->taMan->getBackend()), TA<label_type>(this->taMan->getBackend())));
			if (cSig[i] == root)
				continue;
			assert(this->roots[cSig[i]]);
			if (!this->boxCut(iTmp.back().first, iTmp.back().second, *this->roots[cSig[i]], box->getTrigger(sig[i])))
				return false;
			o.clear();
			FAE::computeDownwardO(iTmp.back().second, o);
			const std::vector<size_t>& iSig = box->getO(sig[i]);
			std::vector<size_t> cISig = o[iTmp.back().second.getFinalState()];
			if (iSig.size() != cISig.size())
				return false;
			for (size_t i = 0; i < sig.size(); ++i) {
				if (index[iSig[i]] != cISig[i])
					return false;
			}
		}

		// match automata
		if (!TAIsom<label_type, IsomF>(cTmp, *box->getRoot(0), IsomF(index)).run())
			return false;

		for (size_t i = 0; i < cSig.size(); ++i) {
			if (cSig[i] == root)
				continue;
			if (!TAIsom<label_type, IsomF>(iTmp[i].second, *box->getRoot(sig[i]), IsomF(index)).run())
				return false;
		}		

		// update selector map
/*		for (size_t i = 0; i < box->getArity(); ++i) {
			const std::set<size_t>& ic = box->inputCoverage(i);
			for (std::set<size_t>::const_iterator j = ic.begin(); j != ic.end(); ++j) {
				bool b = this->selectorMap.insert(std::make_pair(std::make_pair(cSig[i], *j), root)).second;
				assert(b);
			}
		}*/

		// append box
		TA<label_type>* ta = this->taMan->alloc();

		size_t state = tmp.getFinalState();

		for (TA<label_type>::Iterator i = tmp.begin(); i != tmp.end(); ++i) {
			if (i->rhs() != state) {
				ta->addTransition(*i);
				continue;
			}
			std::vector<const AbstractBox*> label(*i->label().dataB);
			std::vector<size_t> lhs(i->lhs());
			label.push_back(box);
			for (size_t j = 0; j < cSig.size(); ++j)
				lhs.push_back(this->addData(*ta, Data::createRef(cSig[j])));
			FAE::reorderBoxes(label, lhs);
			ta->addTransition(lhs, &this->labMan->lookup(label), state);
		}

		// replace
		this->updateRoot(this->roots[root], ta);
		this->updateRootMap(root);
		
		for (size_t i = 0; i < cSig.size(); ++i) {
			if (cSig[i] == root)
				continue;
			this->updateRoot(this->roots[cSig[i]], this->taMan->clone(&iTmp[i].first));
			this->updateRootMap(cSig[i]);
		}

		return true;
	}

	void decomposeAtRoot(size_t root, const std::set<const Box*>& boxes) {
		for (std::set<const Box*>::const_iterator i = boxes.begin(); i != boxes.end(); ++i)
			this->unfoldBox(root, *i);
	}

public:

	void loadTA(const TA<label_type>& src, const TA<label_type>::td_cache_type& cache, const TT<label_type>* top, size_t stateOffset) {
		this->clear();
		this->variables = *top->label().data;
		this->stateOffset = stateOffset;
		for (vector<size_t>::const_iterator i = top->lhs().begin(); i != top->lhs().end(); ++i) {
			TA<label_type>* ta = this->taMan->alloc();
			this->roots.push_back(ta);
			// add reachable transitions
			for (TA<label_type>::td_iterator j = src.tdStart(cache, itov(*i)); j.isValid(); j.next())
				ta->addTransition(*j);
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
		const DataBox* b = this->boxMan->getData(data);
		size_t state = _MSB_ADD(b->getId());
		dst.addTransition(vector<size_t>(), &this->labMan->lookup(itov((const AbstractBox*)b)), state);
		return state;
	}

	static bool isData(size_t state) {
		return _MSB_TEST(state);
	}

	static bool isData(const AbstractBox* box, const Data*& data) {
		if (!box->isType(box_type_e::bData))
			return false;
		data = &((DataBox*)box)->getData();
		return true;
	}

	bool isData(size_t state, const Data*& data) const {
		if (!FAE::isData(state))
			return false;
		data = &this->boxMan->getData(_MSB_GET(state))->getData();
		return true;
	}

	bool getRef(size_t state, size_t& ref) const {
		if (!FAE::isData(state))
			return false;
		const Data& data = this->boxMan->getData(_MSB_GET(state))->getData();
		if (!data.isRef())
			return false;
		ref = data.d_ref.root;
		return true;
	}

	static bool isRef(const AbstractBox* box, size_t ref) {
		if (!box->isType(box_type_e::bData))
			return false;
		return ((DataBox*)box)->getData().isRef(ref);
	}

	static bool getRef(const AbstractBox* box, size_t& ref) {
		if (!box->isType(box_type_e::bData))
			return false;
		if (!((DataBox*)box)->getData().isRef())
			return false;
		ref = ((DataBox*)box)->getData().d_ref.root;
		return true;
	}

/*
	static bool containsBox(label_type label) {
		for (vector<const Box*>::const_iterator i = label.dataB->begin(); i != label.dataB->end(); ++i) {
			if ((*i)->isBox())
				return true;
		}
		return false;
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

	static void reorderBoxes(vector<const AbstractBox*>& label, vector<size_t>& lhs) {
		std::vector<std::pair<const AbstractBox*, std::vector<size_t> > > tmp;
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
		for (std::vector<size_t>::iterator i = dst.begin(); i != dst.end(); ++i) {
			assert(index[*i] != (size_t)(-1));
			*i = index[*i];
		}
	}

	static void updateMap(vector<size_t>& dst, size_t ref, const vector<size_t>& src) {
		std::vector<size_t> res;
		std::vector<size_t>::iterator i = std::find(dst.begin(), dst.end(), ref);
		assert(i != dst.end());
		res.insert(res.end(), dst.begin(), i);
		res.insert(res.end(), src.begin(), src.end());
		res.insert(res.end(), i + 1, dst.end());
		FAE::removeMultOcc(res);
		std::swap(dst, res);
	}
/*
	static void updateRoot(TA<label_type>*& root, TA<label_type>::Manager& manager, TA<label_type>* newRoot) {
		if (root)
			manager.release(root);
		root = newRoot;
	}
*/
	TA<label_type>& relabelReferences(TA<label_type>& dst, const TA<label_type>& src, const vector<size_t>& index) {
		dst.addFinalState(src.getFinalState());
		for (TA<label_type>::iterator i = src.begin(); i != src.end(); ++i) {
			std::vector<size_t> lhs;
			for (std::vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j) {
				const Data* data;
				if (!this->isData(*j, data) || !data->isRef() || index[data->d_ref.root] == data->d_ref.root) {
					lhs.push_back(*j);
				} else {
					if (index[data->d_ref.root] != (size_t)(-1))
						lhs.push_back(this->addData(dst, Data::createRef(index[data->d_ref.root], data->d_ref.displ)));
					else
						lhs.push_back(this->addData(dst, Data::createUndef()));
				}
			}
			dst.addTransition(lhs, i->label(), i->rhs());
		}
		return dst;
	}

	TA<label_type>* relabelReferences(TA<label_type>* src, const vector<size_t>& index) {
		return &this->relabelReferences(*this->taMan->alloc(), *src, index);
	}

	TA<label_type>& invalidateReference(TA<label_type>& dst, const TA<label_type>& src, size_t root) {
		dst.addFinalState(src.getFinalState());
		for (TA<label_type>::iterator i = src.begin(); i != src.end(); ++i) {
			vector<size_t> lhs;
			for (vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j) {
				const Data* data;
				if (FAE::isData(*j, data) && data->isRef(root)) {
					lhs.push_back(this->addData(dst, Data::createUndef()));
				} else {
					lhs.push_back(*j);
				}
			}
			if (!FAE::isRef(i->label().head(), root))
				dst.addTransition(lhs, i->label(), i->rhs());
		}
		return dst;
	}

	TA<label_type>* invalidateReference(TA<label_type>* src, size_t root) {
		return &this->invalidateReference(*this->taMan->alloc(), *src, root);
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
		const DataBox* b = this->boxMan->getData(Data::createRef(ref));
		size_t refState = _MSB_ADD(b->getId());
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
				std::vector<size_t> v;
				if (FAE::isData(i->label().head(), data) && data->isRef()) {
					v.push_back(data->d_ref.root);
				} else {
//					vector<size_t> order;
//					FAE::evaluateLhsOrder(*i->label().dataB, order);
					for (std::vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j) {
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
//		const Data* data;
		boost::unordered_map<size_t, size_t> index;
		for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {
			if (FAE::isRef(i->label().head(), ref))
				index.insert(make_pair(i->rhs(), 1));
		}
		bool changed = true;
		while (changed) {
			changed = false;
			for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {
				if (!i->label().head()->isStructural())
					continue;
				size_t sum = 0;
				for (vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j)
					sum += index.insert(make_pair(*j, 0)).first->second;
				if (sum == 0)
					continue;
				// if everything is reachable, then we can safely conclude here
				if (sum > 1)
					return false;
				size_t& k = index.insert(make_pair(i->rhs(), 0)).first->second;
//				sum = std::min(sum, 2);
//				if (k < sum) {
//					k = sum;
//					changed = true;
//				}
				if (k == 0) {
					k = 1;
					changed = true;
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
		for (std::vector<size_t>::const_iterator i = this->rootMap[c].begin(); i != this->rootMap[c].end(); ++i) {
			this->visitDown(*i, visited, order, marked);
			if (!marked[*i] && !FAE::isUniqueRef(*this->roots[c], *i))
				marked[*i] = true;
		}
	}

	void visitDown(size_t c, std::vector<bool>& visited) const {
		if (visited[c])
			return;
		visited[c] = true;
		for (std::vector<size_t>::const_iterator i = this->rootMap[c].begin(); i != this->rootMap[c].end(); ++i)
			this->visitDown(*i, visited);
	}

	void traverse(std::vector<bool>& visited, std::vector<size_t>& order, std::vector<bool>& marked) const {
		// TODO: upward traversal
		visited = std::vector<bool>(this->roots.size(), false);
		marked = std::vector<bool>(this->roots.size(), false);
		order.clear();
		for (std::vector<Data>::const_iterator i = this->variables.begin(); i != variables.end(); ++i) {
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
		for (std::vector<Data>::const_iterator i = this->variables.begin(); i != variables.end(); ++i) {
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
				throw ProgramError("garbage detected");
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
				this->updateRoot(this->roots[root], ta);
				this->updateRoot(this->roots[*i], NULL);
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
		std::vector<std::vector<size_t> > newRootMap;
		size_t offset = 0;
		for (std::vector<size_t>::iterator i = order.begin(); i < order.end(); ++i) {
			this->normalizeRoot(normInfo, normalized, *i, marked);
//			assert(marked[*i] || (this->roots[*i] == NULL));
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
			this->updateRoot(this->roots[i], this->relabelReferences(this->roots[i], index));
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
						if (FAE::isData(state))
							state = *k;
						else
							state = stateIndex.translateOTF(*k) + this->stateOffset;
					}
					lhs.push_back(state);
				}
				size_t rhs;
				if (FAE::isData((*j)->rhs()))
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
				this->splitPoints.push_back(std::make_pair(rhs, i->second));
		}

	};

	bool denormalize(const FAE& fae, const NormInfo& normInfo) {
		assert(fae.roots.size() == normInfo.data.size());
		assert(this->roots.size() == fae.roots.size());
		
		FAE tmp(*this);
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			this->updateRoot(*i, NULL);

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
				stateCount = TA<label_type>::buProduct(
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
				stateCount = TA<label_type>::buProduct(
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
			if (this->fae.isData(t2->rhs(), data))
				rhs = t2->rhs();
			this->dst.addTransition(lhs, t2->_label, rhs);
			if (this->src1.isFinalState(t1->rhs()) && this->src2.isFinalState(t2->rhs()))
				this->dst.addFinalState(rhs);
		}

	};

	void buildLTCacheExt(const TA<label_type>& ta, TA<label_type>::lt_cache_type& cache) {
		const DataBox* b = this->boxMan->getData(Data::createUndef());
		label_type lUndef = &this->labMan->lookup(itov((const AbstractBox*)b));
		for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {
			if (i->label().head()->isType(box_type_e::bData)) {
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
			this->updateRoot(this->roots.back(), NULL);
			this->roots.pop_back();
		}

		assert(this->roots.size() == fae.roots.size());

		FAE tmp(*this);
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			this->updateRoot(*i, NULL);

		for (size_t i = 0; i < this->roots.size(); ++i) {
			if (!tmp.roots[i]) {
				this->roots[i] = this->taMan->addRef(fae.roots[i]);
				continue;
			}
			TA<label_type>::lt_cache_type cache1, cache2;
			FAE::buildLTCacheExt(*tmp.roots[i], cache1);
			FAE::buildLTCacheExt(*fae.roots[i], cache2);

			this->roots[i] = this->taMan->alloc();
			
			size_t stateCount = TA<label_type>::buProduct(
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
			this->updateRoot(this->roots[i], &ta.collapsed(*this->taMan->alloc(), rel, stateIndex));
		}
	}

	struct IsolateOneF {
		size_t offset;

		IsolateOneF(size_t offset) : offset(offset) {}

		bool operator()(const StructuralBox* box) const {
			return box->outputCovers(this->offset);
		}
	};

	struct IsolateSetF {
		std::set<size_t> s;
		
		IsolateSetF(const std::vector<size_t>& v, size_t offset = 0) {
			for (std::vector<size_t>::const_iterator i = v.begin(); i != v.end(); ++i)
				this->s.insert(*i + offset);
		}
		
		bool operator()(const StructuralBox* box) const {
			return utils::checkIntersection(box->outputCoverage(), this->s);
		}
	};

	struct IsolateAllF {
		bool operator()(const StructuralBox*) const {
			return true;
		}
	};

	// adds redundant root points to allow further manipulation
	template <class F>
	void isolateAtRoot(std::vector<FAE*>& dst, size_t root, F f) const {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		for (TA<label_type>::iterator i = this->roots[root]->accBegin(); i != this->roots[root]->accEnd(i); ++i) {
			FAE fae(*this);
			TA<label_type> ta(*fae.roots[root], false);
			size_t newState = fae.freshState();
			ta.addFinalState(newState);
			vector<size_t> lhs;
			size_t lhsOffset = 0;
			std::set<const Box*> boxes;
			for (std::vector<const AbstractBox*>::const_iterator j = i->label().dataB->begin(); j != i->label().dataB->end(); ++j) {
				if (!(*j)->isStructural())
					continue;
				StructuralBox* b = (StructuralBox*)(*j);
				if (!f(b)) {
					// this box is not interesting
					for (size_t k = 0; k < (*j)->getArity(); ++k, ++lhsOffset)
						lhs.push_back(i->lhs()[lhsOffset]);
					continue;
				}
				// we have to isolate here
				for (size_t k = 0; k < (*j)->getArity(); ++k, ++lhsOffset) {
					if (FAE::isData(i->lhs()[lhsOffset])) {
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
					// update 'o'
					fae.roots.push_back(tmp2);
					fae.rootMap.push_back(std::vector<size_t>());
					fae.updateRootMap(fae.roots.size() - 1);
				}
				if (b->isType(box_type_e::bBox))
					boxes.insert((const Box*)*j);
			}
			ta.addTransition(lhs, i->label(), newState);
			TA<label_type>* tmp = fae.taMan->alloc();
			ta.unreachableFree(*tmp);
			// exchange the original automaton with the new one
			fae.updateRoot(fae.roots[root], tmp);
			fae.updateRootMap(root);
			if (!boxes.empty())
				fae.decomposeAtRoot(root, boxes);
			dst.push_back(new FAE(fae));
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

	static bool isSelector(const AbstractBox* box) {
		return box->isType(box_type_e::bSel);
	}

	static const SelData& readSelector(const AbstractBox* box) {
		return ((SelBox*)box)->getData();
	}

	static bool isSelector(const AbstractBox* box, size_t offset) {
		if (!FAE::isSelector(box))
			return false;
		return FAE::readSelector(box).offset == offset;
	}

	void transitionLookup(const TT<label_type>& transition, size_t offset, Data& data) const {
		bool found = false;
		size_t lhsOffset = 0;
		for (vector<const AbstractBox*>::const_iterator i = transition.label().dataB->begin(); i != transition.label().dataB->end(); ++i) {
			if (FAE::isSelector(*i, offset)) {
				assert(!found);
				found = true;
				const Data* tmp;
				if (!this->isData(transition.lhs()[lhsOffset], tmp))
					throw std::runtime_error("FAE::transitionLookup(): destination is not a leaf!");
				data = *tmp;
				FAE::displToData(FAE::readSelector(*i), data);
			}
			lhsOffset += (*i)->getArity();
		}
		if (!found)
			throw std::runtime_error("FAE::trasitionLookup(): selector not found!");
	}

	void transitionLookup(const TT<label_type>& transition, size_t base, const std::set<size_t>& offsets, Data& data) const {
		size_t lhsOffset = 0;
		data = Data::createStruct();
		for (vector<const AbstractBox*>::const_iterator i = transition.label().dataB->begin(); i != transition.label().dataB->end(); ++i) {
			if (FAE::isSelector(*i) && offsets.count(FAE::readSelector(*i).offset)) {
				const Data* tmp;
				if (!this->isData(transition.lhs()[lhsOffset], tmp))
					throw std::runtime_error("FAE::transitionLookup(): destination is not a leaf!");
				data.d_struct->push_back(Data::item_info(FAE::readSelector(*i).offset - base, *tmp));
				FAE::displToData(FAE::readSelector(*i), data.d_struct->back().second);
			} 
			lhsOffset += (*i)->getArity();
		}
		if (data.d_struct->size() != offsets.size())
			throw std::runtime_error("FAE::transitionLookup(): selectors missmatch!");
	}

	void transitionLookup(const TT<label_type>& transition, std::vector<std::pair<SelData, Data> >& nodeInfo) const {
		size_t lhsOffset = 0;
		for (vector<const AbstractBox*>::const_iterator i = transition.label().dataB->begin(); i != transition.label().dataB->end(); ++i) {
			assert(FAE::isSelector(*i));
			const Data* tmp;
			if (!this->isData(transition.lhs()[lhsOffset], tmp))
				throw std::runtime_error("FAE::transitionLookup(): destination is not a leaf!");
			nodeInfo.push_back(std::make_pair(FAE::readSelector(*i), *tmp));
			FAE::displToData(nodeInfo.back().first, nodeInfo.back().second);
			lhsOffset += (*i)->getArity();
		}
	}

	void transitionModify(TA<label_type>& dst, const TT<label_type>& transition, size_t offset, const Data& in, Data& out) {
		size_t state = this->freshState();
		dst.addFinalState(state);
		vector<size_t> lhs;
		vector<const AbstractBox*> label;
		size_t lhsOffset = 0;
		bool found = false;
		for (vector<const AbstractBox*>::const_iterator i = transition.label().dataB->begin(); i != transition.label().dataB->end(); ++i) {
			if (FAE::isSelector(*i, offset)) {
				assert(!found);
				found = true;
				const Data* tmp;
				if (!this->isData(transition.lhs()[lhsOffset], tmp))
					throw runtime_error("FAE::transitionModify(): destination is not a leaf!");
				out = *tmp;
				SelData s = FAE::readSelector(*i);
				FAE::displToData(s, out);
				Data d = in;
				FAE::displToSel(s, d);
				lhs.push_back(this->addData(dst, d));
				label.push_back(this->boxMan->getSelector(s));
			} else {
				lhs.insert(lhs.end(), transition.lhs().begin() + lhsOffset, transition.lhs().begin() + lhsOffset + (*i)->getArity());
				label.push_back(*i);
			}
			lhsOffset += (*i)->getArity();
		}
		if (!found)
			throw runtime_error("FAE::transitionModify(): selector not found!");
		FAE::reorderBoxes(label, lhs);
		dst.addTransition(lhs, &this->labMan->lookup(label), state);
	}

	void transitionModify(TA<label_type>& dst, const TT<label_type>& transition, size_t base, const std::map<size_t, Data>& in, Data& out) {
		size_t state = this->freshState();
		dst.addFinalState(state);
		vector<size_t> lhs;
		vector<const AbstractBox*> label;
		size_t lhsOffset = 0;
		out = Data::createStruct();
		for (vector<const AbstractBox*>::const_iterator i = transition.label().dataB->begin(); i != transition.label().dataB->end(); ++i) {
			if (FAE::isSelector(*i)) {
				std::map<size_t, Data>::const_iterator j = in.find(FAE::readSelector(*i).offset);
				if (j != in.end()) {
					const Data* tmp;
					if (!this->isData(transition.lhs()[lhsOffset], tmp))
						throw runtime_error("FAE::transitionModify(): destination is not a leaf!");
					out.d_struct->push_back(Data::item_info(FAE::readSelector(*i).offset - base, *tmp));
					SelData s = FAE::readSelector(*i);
					FAE::displToData(s, out.d_struct->back().second);
					Data d = j->second;
					FAE::displToSel(s, d);
					lhs.push_back(this->addData(dst, d));
					label.push_back(this->boxMan->getSelector(s));
				} else {
					lhs.insert(lhs.end(), transition.lhs().begin() + lhsOffset, transition.lhs().begin() + lhsOffset + (*i)->getArity());
					label.push_back(*i);
				}
			}
			lhsOffset += (*i)->getArity();
		}
		if (out.d_struct->size() != in.size())
			throw runtime_error("FAE::transitionModify(): selectors missmatch!");
		FAE::reorderBoxes(label, lhs);
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

	size_t nodeCreate(const std::vector<std::pair<SelData, Data> >& nodeInfo, const TypeBox* typeInfo = NULL) {
		size_t root = this->roots.size();
		TA<label_type>* ta = this->taMan->alloc();
		size_t f = this->freshState();
		ta->addFinalState(f);
		std::vector<const AbstractBox*> label;
		std::vector<size_t> lhs;
		std::vector<size_t> o;
		if (typeInfo)
			label.push_back(typeInfo);
		for (std::vector<std::pair<SelData, Data> >::const_iterator i = nodeInfo.begin(); i != nodeInfo.end(); ++i) {
			SelData sel = i->first;
			Data data = i->second;
			FAE::displToSel(sel, data);
			// label
			label.push_back(this->boxMan->getSelector(sel));
			// lhs
			lhs.push_back(this->addData(*ta, data));
			// o
			if (data.isRef())
				o.push_back(i->second.d_ref.root);
		}
		FAE::reorderBoxes(label, lhs);
		ta->addTransition(lhs, &this->labMan->lookup(label), f);
		this->roots.push_back(ta);
		this->rootMap.push_back(o);
		return root;
	}

	size_t nodeCreate(const std::vector<SelData>& nodeInfo, const TypeBox* typeInfo = NULL) {
		size_t root = this->roots.size();
		TA<label_type>* ta = this->taMan->alloc();
		size_t f = this->freshState();
		ta->addFinalState(f);
		// build label
		std::vector<const AbstractBox*> label;
		if (typeInfo)
			label.push_back(typeInfo);
		for (std::vector<SelData>::const_iterator i = nodeInfo.begin(); i != nodeInfo.end(); ++i)
			label.push_back(this->boxMan->getSelector(*i));
		// build lhs
		vector<size_t> lhs(nodeInfo.size(), this->addData(*ta, Data::createUndef()));
		// reorder
		FAE::reorderBoxes(label, lhs);
		// fill the rest
		ta->addTransition(lhs, &this->labMan->lookup(label), f);
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
		this->updateRoot(this->roots[root], NULL);
		// make all references to this rootpoint dangling
		size_t i = 0;
		for (; i < root; ++i) {
			this->updateRoot(this->roots[i], this->invalidateReference(this->roots[i], root));
			FAE::invalidateReference(this->rootMap[i], root);
		}
		// skip 'root'
		++i;
		for (; i < this->roots.size(); ++i) {
			this->updateRoot(this->roots[i], this->invalidateReference(this->roots[i], root));
			FAE::invalidateReference(this->rootMap[i], root);
		}
		
	}

	void unsafeNodeDelete(size_t root) {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		// erase node
		this->updateRoot(this->roots[root], NULL);
	}

	void nodeLookup(size_t root, size_t offset, Data& data) const {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		this->transitionLookup(this->roots[root]->getAcceptingTransition(), offset, data);
	}	

	void nodeLookup(size_t root, std::vector<std::pair<SelData, Data> >& data) const {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		this->transitionLookup(this->roots[root]->getAcceptingTransition(), data);
	}	

	void nodeLookupMultiple(size_t root, size_t base, const std::vector<size_t>& offsets, Data& data) const {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		std::set<size_t> s;
		for (std::vector<size_t>::const_iterator i = offsets.begin(); i != offsets.end(); ++i)
			s.insert(base + *i);
		this->transitionLookup(this->roots[root]->getAcceptingTransition(), base, s, data);
	}	

	void nodeModify(size_t root, size_t offset, const Data& in, Data& out) {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		TA<label_type> ta(this->taMan->getBackend());
		this->transitionModify(ta, this->roots[root]->getAcceptingTransition(), offset, in, out);
		this->roots[root]->copyTransitions(ta);
		TA<label_type>* tmp = this->taMan->alloc();
		ta.unreachableFree(*tmp);
		this->updateRoot(this->roots[root], tmp);
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
		this->transitionModify(ta, this->roots[root]->getAcceptingTransition(), offset, m, out);
		this->roots[root]->copyTransitions(ta);
		TA<label_type>* tmp = this->taMan->alloc();
		ta.unreachableFree(*tmp);
		this->updateRoot(this->roots[root], tmp);
		boost::unordered_map<size_t, vector<size_t> > o;
		FAE::computeDownwardO(*tmp, o);
		this->rootMap[root] = o[tmp->getFinalState()];
	}	

	void getNearbyReferences(size_t root, std::vector<size_t>& out) const {
		assert(root < this->roots.size());
		assert(this->roots[root]);
		const TT<label_type>& t = this->roots[root]->getAcceptingTransition();
		for (std::vector<size_t>::const_iterator i = t.lhs().begin(); i != t.lhs().end(); ++i) {
			const Data* data;
			if (this->isData(*i, data) && data->isRef())
				out.push_back(data->d_ref.root);
		}
	}	

public:

	// state 0 should never be allocated by FAE (?)
	FAE(TA<label_type>::Manager& taMan, LabMan& labMan, BoxMan& boxMan)
	 : FA(taMan), boxMan(&boxMan), labMan(&labMan), stateOffset(1) {
		 // init special root references (this is not required)
//		 this->registerRootReference(varNull, this->freshState());
//		 this->registerRootReference(varUndef, this->freshState());
	}

	FAE(const FAE& x)
		: FA(x), boxMan(x.boxMan), labMan(x.labMan), stateOffset(x.stateOffset), rootMap(x.rootMap)/*,
		selectorMap(x.selectorMap)*/ {
	}
//		rootReferenceIndex(x.rootReferenceIndex), invRootReferenceIndex(x.invRootReferenceIndex) {}

	~FAE() {
		this->clear();
	}
	
	FAE& operator=(const FAE& x) {
		((FA*)this)->operator=(x);
		this->boxMan = x.boxMan;
		this->labMan = x.labMan;
		this->stateOffset = x.stateOffset;
		this->rootMap = x.rootMap;
//		this->selectorMap = x.selectorMap;
		return *this;		
	}

	void clear() {
		((FA*)this)->clear();
		this->stateOffset = 1;
		this->rootMap.clear();
//		this->selectorMap.clear();
//		this->rootReferenceIndex.clear();
//		this->invRootReferenceIndex.clear();
//		this->dataMan.clear();
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
/*
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
*/
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
