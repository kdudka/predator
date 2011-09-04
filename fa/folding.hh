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

#ifndef FOLDING_H
#define FOLDING_H

#include <vector>
#include <set>
#include <stdexcept>
#include <algorithm>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "forestautext.hh"
#include "abstractbox.hh"
#include "utils.hh"

typedef enum { mFail, mSuccess, mDunno } match_result_e;

template <class T, class F>
struct TAIsom {

	const TA<T>& ta1;
	const TA<T>& ta2;
	F f;
	boost::unordered_map<size_t, size_t> sMatching;
	std::vector<boost::unordered_map<size_t, size_t>::iterator> sStack;

	TAIsom(const TA<T>& ta1, const TA<T>& ta2, F f) : ta1(ta1), ta2(ta2), f(f) {}
/*
	bool matchLabels(const std::vector<const TT<label_type>*>& v1, const std::vector<const TT<label_type>*>& v2) {
		for (size_t i = 0; i < v1.size(); ++i) {
			if (!this->f(*v1[i], *v2[i]))
				return false;				
		}
		return true;
	}
*/
	bool matchLhs(const TT<T>& t1, const TT<T>& t2) {
		assert(t1.lhs().size() == t2.lhs().size());
		for  (size_t i = 0; i < t1.lhs().size(); ++i) {
			if (!this->matchStates(t1.lhs()[i], t2.lhs()[i]))
				return false;
		}
		return true;
	}

	bool matchStates(size_t s1, size_t s2) {
		switch (f.matchStates(s1, s2)) {
			case match_result_e::mSuccess: return true;
			case match_result_e::mFail: return false;
			default: break;
		}
		std::pair<typename boost::unordered_map<size_t, size_t>::iterator, bool> p
			= this->sMatching.insert(std::make_pair(s1, s2));
		if (!p.second)
			return p.first->second == s2;
		this->sStack.push_back(p.first);
		size_t mark = this->sStack.size();
		std::vector<const TT<label_type>*> v1, v2;
		for (typename TA<T>::iterator i = this->ta1.begin(s1); i != this->ta1.end(s1); ++i)
			v1.push_back(&*i);
		for (typename TA<T>::iterator i = this->ta2.begin(s2); i != this->ta2.end(s2); ++i)
			v2.push_back(&*i);
		if (v1.size() != v2.size())
			return false;
		std::sort(v2.begin(), v2.end());
		do {
			size_t i;
			for (i = 0; i < v1.size(); ++i) {
				switch (this->f.matchTransitions(*v1[i], *v2[i])) {
					case match_result_e::mSuccess:
						continue;
					case match_result_e::mDunno:
						if (this->matchLhs(*v1[i], *v2[i]))
							continue;
					default:
						break;	
				}
				break;
			}
			if (i == v1.size())
				return true;
			for (i = mark; i < this->sStack.size(); ++i)
				this->sMatching.erase(this->sStack[i]);
			this->sStack.resize(mark);
		} while (std::next_permutation(v2.begin(), v2.end()));
		return false;
	}

	bool run() {
		return this->matchStates(this->ta1.getFinalState(), this->ta2.getFinalState());
	}

};

struct RootSentry {
		
	FAE* fae;
	size_t expectedRoots;
	size_t stateOffset;

	RootSentry(FAE& fae) : fae(&fae), expectedRoots(fae.roots.size()), stateOffset(fae.nextState()) {}

	~RootSentry() {
		if (!this->fae)
			return;
		if (this->expectedRoots == this->fae->roots.size())
			return;
/*		for (size_t i = this->expectedRoots; i < this->fae->roots.size(); ++i)
			this->fae->taMan->release(this->fae->roots[i]);*/
		this->fae->roots.resize(this->expectedRoots);
		this->fae->rootMap.resize(this->expectedRoots);
		this->fae->setStateOffset(this->stateOffset);
	}

	void release() {
		this->fae = NULL;
	}

};

// matches root against box output while adding necessary roots on the fly!!!
struct IsomRootF {

	FAE& fae;
	size_t root;
	std::vector<size_t>& index;
	
	IsomRootF(FAE& fae, size_t root, std::vector<size_t>& index)
		: fae(fae), root(root), index(index) {
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);
	}

	match_result_e matchStates(size_t s1, size_t s2) {
		const Data* data1, * data2;
		if (this->fae.isData(s1, data1)) {
			if (!this->fae.isData(s2, data2))
				return match_result_e::mFail;
			if (!data1->isRef() || !data2->isRef())
				return (data1 == data2)?(match_result_e::mSuccess):(match_result_e::mFail);
			assert(data2->d_ref.root < this->index.size());
			size_t ref1 = data1->d_ref.root, ref2 = data2->d_ref.root;
			if ((this->index[ref2] != (size_t)(-1)) && (this->index[ref2] != ref1))
				return match_result_e::mFail;
			this->index[ref2] = ref1;
			return match_result_e::mSuccess;
		}
		if (!this->fae.isData(s2, data2))
			return match_result_e::mDunno;
		if (!data2->isRef())
			return match_result_e::mFail;
		size_t ref2 = data2->d_ref.root;
		assert(ref2 < this->index.size());
		if (this->index[ref2] != (size_t)(-1))
			return match_result_e::mFail;
		this->index[ref2] = this->fae.roots.size();
		this->fae.appendRoot(this->fae.allocTA());
		Index<size_t> stateIndex;
		size_t offset = this->fae.nextState();
		this->fae.unique(*this->fae.roots.back(), *this->fae.roots[this->root], stateIndex, false);
		this->fae.roots.back()->addFinalState(stateIndex[s1] + offset);
		fae.incrementStateOffset(stateIndex.size());
		this->fae.rootMap.push_back(std::vector<std::pair<size_t, bool> >());
		this->fae.updateRootMap(this->fae.rootMap.size() - 1);
		return match_result_e::mSuccess;
	}

	match_result_e matchTransitions(const TT<label_type>& t1, const TT<label_type>& t2) {
		return (t1.label() == t2.label())?(match_result_e::mDunno):(match_result_e::mFail);
	}

};

struct IsomF {

	const FAE& fae;
	std::vector<size_t>& index;
	
	IsomF(const FAE& fae, std::vector<size_t>& index) : fae(fae), index(index) {}

	match_result_e matchStates(size_t s1, size_t s2) {
		const Data* data1, * data2;
		if (this->fae.isData(s1, data1)) {
			if (!this->fae.isData(s2, data2))
				return match_result_e::mFail;
			if (!data1->isRef() || !data2->isRef())
				return (data1 == data2)?(match_result_e::mSuccess):(match_result_e::mFail);
			assert(data2->d_ref.root < this->index.size());
			size_t ref1 = data1->d_ref.root, ref2 = data2->d_ref.root;
			if ((this->index[ref2] != (size_t)(-1)) && (this->index[ref2] != ref1))
				return match_result_e::mFail;
			this->index[ref2] = ref1;
			return match_result_e::mSuccess;
		}
		if (!this->fae.isData(s2, data2))
			return match_result_e::mDunno;
		return match_result_e::mFail;
	}

	match_result_e matchTransitions(const TT<label_type>& t1, const TT<label_type>& t2) {
		return (t1.label() == t2.label())?(match_result_e::mDunno):(match_result_e::mFail);
	}

};

class Folding {

	FAE& fae;

protected:

	void boxMerge(TA<label_type>& dst, const TA<label_type>& src, const TA<label_type>& boxRoot, const Box* box, const std::vector<size_t>& rootIndex) {
		TA<label_type> tmp(*this->fae.backend), tmp2(*this->fae.backend);
//		this->fae.boxMan->adjustLeaves(tmp2, boxRoot);
		this->fae.relabelReferences(tmp, boxRoot, rootIndex);
		this->fae.unique(tmp2, tmp);
		src.copyTransitions(dst, TA<label_type>::NonAcceptingF(src));
		tmp2.copyTransitions(dst, TA<label_type>::NonAcceptingF(tmp2));
		dst.addFinalStates(tmp2.getFinalStates());
		for (std::set<size_t>::const_iterator j = src.getFinalStates().begin(); j != src.getFinalStates().end(); ++j) {
			for (TA<label_type>::iterator i = src.begin(*j); i != src.end(*j, i); ++i) {
				std::vector<size_t> lhs;
				std::vector<const AbstractBox*> label;
				size_t lhsOffset = 0;
				if (box) {
					bool found = false;
					for (std::vector<const AbstractBox*>::const_iterator j = i->label()->getNode().begin(); j != i->label()->getNode().end(); ++j) {
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
						lhsOffset += box->getArity();
						assert(!found);
						found = true;
					}
					assert(found);
				} else {
					lhs = i->lhs();
					label = i->label()->getNode();
				}
				for (TA<label_type>::iterator j = tmp2.accBegin(); j != tmp2.accEnd(j); ++j) {
					std::vector<size_t> lhs2 = lhs;
					std::vector<const AbstractBox*> label2 = label;
					lhs2.insert(lhs2.end(), j->lhs().begin(), j->lhs().end());
					label2.insert(label2.end(), j->label()->getNode().begin(), j->label()->getNode().end());
					FA::reorderBoxes(label2, lhs2);
					dst.addTransition(lhs2, this->fae.boxMan->lookupLabel(label2), j->rhs());
				}
			}
		}
	}

	static void copyBox(std::vector<size_t>& lhs, std::vector<const AbstractBox*>& label, const AbstractBox* box, const std::vector<size_t>& srcLhs, size_t& srcOffset) {
		for (size_t i = 0; i < box->getArity(); ++i, ++srcOffset)
			lhs.push_back(srcLhs[srcOffset]);
		label.push_back(box);
	}

	bool boxCut(TA<label_type>& dst, TA<label_type>& complement, const TA<label_type>& src, const std::set<const AbstractBox*>& trigger) {
		for (std::set<size_t>::const_iterator k = src.getFinalStates().begin(); k != src.getFinalStates().end(); ++k) {
			for (TA<label_type>::Iterator i = src.begin(*k); i != src.end(*k, i); ++i) {
				std::vector<size_t> lhs, cLhs;
				std::vector<const AbstractBox*> label, cLabel;
				size_t lhsOffset = 0;
				const TT<label_type>& t = *i;
				size_t matched = 0;
				for (std::vector<const AbstractBox*>::const_iterator j = t.label()->getNode().begin(); j != t.label()->getNode().end(); ++j) {
					if (trigger.count(*j)) {
						Folding::copyBox(cLhs, cLabel, *j, t.lhs(), lhsOffset);
						++matched;
					} else {
						Folding::copyBox(lhs, label, *j, t.lhs(), lhsOffset);
					}
				}
				if (matched == trigger.size()) {
					FAE::reorderBoxes(label, lhs);
					dst.addTransition(lhs, this->fae.boxMan->lookupLabel(label), *k);
					FAE::reorderBoxes(cLabel, cLhs);
					complement.addTransition(cLhs, this->fae.boxMan->lookupLabel(cLabel), this->fae.nextState());
				} else return false;
			}
		}
		dst.addFinalStates(src.getFinalStates());
		complement.addFinalState(this->fae.nextState());
		src.copyTransitions(dst, TA<label_type>::NonAcceptingF(src));
		src.copyTransitions(complement, TA<label_type>::NonAcceptingF(src));
		return true;
	}

public:

	void unfoldBox(size_t root, const Box* box) {
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);
		assert(box);
		CL_CDEBUG("efore unfolding box " << *(AbstractBox*)box << " at " << root << ":" << std::endl << this->fae.roots[root]);
		const TT<label_type>& t = this->fae.roots[root]->getAcceptingTransition();
		size_t lhsOffset = 0;
		std::vector<size_t> bSig = box->getSig(0);
		std::vector<size_t> sig;
		for (std::vector<const AbstractBox*>::const_iterator i = t.label()->getNode().begin(); i != t.label()->getNode().end(); ++i) {
			if ((const AbstractBox*)box != *i) {
				lhsOffset += (*i)->getArity();
				continue;
			}
			for (size_t j = 0; j < box->getArity(); ++j) {
				size_t ref;
				bool b = this->fae.getRef(t.lhs()[lhsOffset + j], ref);
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
		TA<label_type> ta(*this->fae.backend);
//		TA<label_type>* ta = this->fae.taMan->alloc();
		this->boxMerge(ta, *this->fae.roots[root], *box->getRoot(0), box, index);
		this->fae.roots[root] = std::shared_ptr<TA<label_type>>(&ta.unreachableFree(*this->fae.allocTA()));
		this->fae.updateRootMap(root);
		for (size_t i = 0; i < box->getArity(); ++i) {
			TA<label_type> tmp(*this->fae.backend);
			this->fae.roots[index[i + 1]]->unfoldAtRoot(tmp, this->fae.freshState());
			ta.clear();
			this->boxMerge(ta, tmp, *box->getRoot(i + 1), NULL, index);
			this->fae.roots[index[i + 1]] = std::shared_ptr<TA<label_type>>(&ta.unreachableFree(*this->fae.allocTA()));
			this->fae.updateRootMap(index[i + 1]);
		}
	}

	bool foldBox(size_t root, const Box* box) {
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);
		assert(box);

		TA<label_type> tmp(*this->fae.backend), cTmp(*this->fae.backend);
		if (!this->boxCut(tmp, cTmp, *this->fae.roots[root], box->getTrigger(0)))
			return false;

		RootSentry rs(this->fae);

		std::vector<size_t> index(box->getArity() + 1, (size_t)(-1));
		index[0] = root;

		// match automata
		if (!TAIsom<label_type, IsomRootF>(cTmp, *box->getRoot(0), IsomRootF(this->fae, root, index)).run())
			return false;

		const std::vector<size_t>& sig = box->getSig(0);
		std::vector<size_t> cSig(sig.size());
		for (size_t i = 0; i < sig.size(); ++i) {
			assert(sig[i] < index.size());
			cSig[i] = index[sig[i]];
			if (sig[i] != 0 && cSig[i] == root)
				return false;
		}

		FA::o_map_type o;

		// match inputs
		std::vector<std::pair<TA<label_type>, TA<label_type> > > iTmp;
		for (size_t i = 0; i < cSig.size(); ++i) {
			iTmp.push_back(std::make_pair(TA<label_type>(*this->fae.backend), TA<label_type>(*this->fae.backend)));
			if (cSig[i] == root)
				continue;
			assert(this->fae.roots[cSig[i]]);
			if (!this->boxCut(iTmp.back().first, iTmp.back().second, *this->fae.roots[cSig[i]], box->getTrigger(sig[i])))
				return false;
			o.clear();
			FA::computeDownwardO(iTmp.back().second, o);
			const std::vector<size_t>& iSig = box->getSig(sig[i]);
			std::vector<std::pair<size_t, bool> > cISig = o[iTmp.back().second.getFinalState()];
			if (iSig.size() != cISig.size())
				return false;
			for (size_t i = 0; i < sig.size(); ++i) {
				if (index[iSig[i]] != cISig[i].first)
					return false;
			}
		}

		for (size_t i = 0; i < cSig.size(); ++i) {
			if (cSig[i] == root)
				continue;
			if (!TAIsom<label_type, IsomF>(iTmp[i].second, *box->getRoot(sig[i]), IsomF(this->fae, index)).run())
				return false;
		}		

		rs.release();

		// append box
		TA<label_type> ta(*this->fae.backend);
		size_t state = tmp.getFinalState();

		ta.addFinalState(state);

		for (TA<label_type>::Iterator i = tmp.begin(); i != tmp.end(); ++i) {
			if (i->rhs() != state) {
				ta.addTransition(*i);
				continue;
			}
			std::vector<const AbstractBox*> label(i->label()->getNode());
			std::vector<size_t> lhs(i->lhs());
			label.push_back(box);
			for (size_t j = 0; j < cSig.size(); ++j)
				lhs.push_back(this->fae.addData(ta, Data::createRef(cSig[j])));
			FAE::reorderBoxes(label, lhs);
			ta.addTransition(lhs, this->fae.boxMan->lookupLabel(label), state);
		}

		// replace
		this->fae.roots[root] = std::shared_ptr<TA<label_type>>(this->fae.allocTA());
		ta.unreachableFree(*this->fae.roots[root]);
		this->fae.updateRootMap(root);
		
		for (size_t i = 0; i < cSig.size(); ++i) {
			if (cSig[i] == root) {
				if (sig[i] == 0)
					continue;
				bool b = this->boxCut(iTmp[i].first, iTmp[i].second, *this->fae.roots[cSig[i]], box->getTrigger(sig[i]));
				assert(b);
			}
			this->fae.roots[cSig[i]] = std::shared_ptr<TA<label_type>>(this->fae.allocTA());
			iTmp[i].first.unreachableFree(*this->fae.roots[cSig[i]]);
			this->fae.updateRootMap(cSig[i]);
		}

		return true;
	}

	void unfoldBoxes(size_t root, const std::set<const Box*>& boxes) {
		for (std::set<const Box*>::const_iterator i = boxes.begin(); i != boxes.end(); ++i)
			this->unfoldBox(root, *i);
	}

public:

	Folding(FAE& fae) : fae(fae) {}

};

#endif
