/*
 * Copyright (C) 2011 Jiri Simacek
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

// Forester headers
#include "splitting.hh"

// anonymous namespace
namespace
{
/**
 * @brief  Functor that collects all selectors covered by a box
 */
struct RootEnumF
{
	size_t target;
	std::set<size_t>& selectors;

	RootEnumF(
		size_t                      target,
		std::set<size_t>&           selectors) :
		target(target),
		selectors(selectors)
	{ }

	bool operator()(const AbstractBox* aBox, size_t, size_t)
	{
		if (!aBox->isStructural())
			return true;

		const StructuralBox* sBox = static_cast<const StructuralBox*>(aBox);
		this->selectors.insert(
			sBox->outputCoverage().begin(),
			sBox->outputCoverage().end()
		);

		return true;
	}
};


struct LeafEnumF
{
	const FAE& fae;
	const Splitting::Transition& t;
	size_t target;
	std::set<size_t>& selectors;

	LeafEnumF(
		const FAE&                      fae,
		const Splitting::Transition&    t,
		size_t                          target,
		std::set<size_t>&               selectors) :
		fae(fae),
		t(t),
		target(target),
		selectors(selectors)
	{ }

	bool operator()(const AbstractBox* aBox, size_t, size_t offset)
	{
		if (!aBox->isType(box_type_e::bBox))
			return true;

		const Box* box = static_cast<const Box*>(aBox);
		for (size_t k = 0; k < box->getArity(); ++k, ++offset)
		{
			size_t ref;
			if (fae.getRef(this->t.lhs()[offset], ref) && ref == this->target)
			{
				this->selectors.insert(
					box->inputCoverage(k).begin(),
					box->inputCoverage(k).end()
				);
			}
		}
		return true;
	}
};


struct LeafScanF
{
	const FAE& fae;
	const Splitting::Transition& t;
	size_t selector;
	size_t target;
	const Box*& matched;

	LeafScanF(
		const FAE&                     fae,
		const Splitting::Transition&   t,
		size_t                         selector,
		size_t                         target,
		const Box*&                    matched) :
		fae(fae),
		t(t),
		selector(selector),
		target(target),
		matched(matched)
	{ }

	bool operator()(const AbstractBox* aBox, size_t, size_t offset)
	{
		if (!aBox->isType(box_type_e::bBox))
			return true;
		const Box* box = static_cast<const Box*>(aBox);
		for (size_t k = 0; k < box->getArity(); ++k, ++offset)
		{
			size_t ref;
			if (fae.getRef(this->t.lhs()[offset], ref) &&
				ref == this->target && box->inputCovers(k, this->selector))
			{
				this->matched = box;
				return false;
			}
		}
		return true;
	}
};


struct IsolateOneF
{
	size_t offset;

	IsolateOneF(size_t offset) : offset(offset) {}

	bool operator()(const StructuralBox* box) const {
		return box->outputCovers(this->offset);
	}

	friend std::ostream& operator<<(std::ostream& os, const IsolateOneF& f) {
		return os << f.offset;
	}
};


struct IsolateBoxF
{
	const Box* box;

	IsolateBoxF(const Box* box) : box(box) {}

	bool operator()(const StructuralBox* box) const {
		return this->box == box;
	}

	friend std::ostream& operator<<(std::ostream& os, const IsolateBoxF& f) {
		return os << *static_cast<const AbstractBox*>(f.box);
	}
};


struct IsolateSetF
{
	std::set<size_t> s;

	IsolateSetF(const std::vector<size_t>& v, size_t offset = 0) :
		s{}
	{
		for (size_t sel : v)
			this->s.insert(sel + offset);
	}

	bool operator()(const StructuralBox* box) const
	{
		return utils::checkIntersection(box->outputCoverage(), this->s);
	}

	friend std::ostream& operator<<(std::ostream& os, const IsolateSetF& f)
	{
		for (auto& x : f.s)
			os << x << ' ';

		return os;
	}
};


struct IsolateAllF
{
	bool operator()(const StructuralBox*) const
	{
		return true;
	}

	friend std::ostream& operator<<(std::ostream& os, const IsolateAllF&)
	{
		return os << "<all>";
	}
};

} // namespace


void Splitting::enumerateSelectorsAtRoot(
	std::set<size_t>&            selectors,
	size_t                       target) const
{
	// Assertions
	assert(target < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(target));

	// the boxes of the accepting transition (note that there is exactly one
	// accepting transition in a normalised FA) are traversed and selectors
	// (even those inside boxes) are collected
	fae_.getRoot(target)->begin(
		*fae_.getRoot(target)->getFinalStates().begin()
	)->label()->iterate(RootEnumF(target, selectors));
}


void Splitting::enumerateSelectorsAtLeaf(
	std::set<size_t>&               selectors,
	size_t                          root,
	size_t                          target) const
{
	// Assertions
	assert(root < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(root));

	for (const Transition& trans : *fae_.getRoot(root))
	{
		if (trans.label()->isNode())
		{
			trans.label()->iterate(LeafEnumF(fae_, trans, target, selectors));
		}
	}
}


void Splitting::enumerateSelectorsAtLeaf(
	std::set<size_t>&           selectors,
	size_t                      target) const
{
	for (std::shared_ptr<TreeAut> ta : fae_.getRoots())
	{
		if (!ta)
			continue;

		for (const Transition& trans : *ta)
		{
			if (trans.label()->isNode())
			{
				trans.label()->iterate(LeafEnumF(fae_, trans, target, selectors));
			}
		}
	}
}


void Splitting::enumerateSelectors(
	std::set<size_t>&           selectors,
	size_t                      target) const
{
	// Assertions
	assert(target < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(target));

	this->enumerateSelectorsAtRoot(selectors, target);
	this->enumerateSelectorsAtLeaf(selectors, target);
}


void Splitting::isolateAtLeaf(
	std::vector<FAE*>&                  dst,
	size_t                              root,
	size_t                              target,
	size_t                              selector) const
{
	// Assertions
	assert(root < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(root));

	fae_.unreachableFree(fae_.getRoot(root));

	std::vector<std::pair<const Transition*, const Box*> > v;

	TreeAut ta(*fae_.backend);

	const Box* matched;
	for (TreeAut::iterator i = fae_.getRoot(root)->begin(); i != fae_.getRoot(root)->end(); ++i) {
		if (!i->label()->isNode()) {
			ta.addTransition(*i);
			continue;
		}
		matched = nullptr;
		i->label()->iterate(LeafScanF(fae_, *i, selector, target, matched));
		if (matched) {
			v.push_back(std::make_pair(&*i, matched));
		} else {
			ta.addTransition(*i);
		}
	}

	assert(v.size());

	for (std::vector<std::pair<const Transition*, const Box*> >::iterator i = v.begin(); i != v.end(); ++i) {
		FAE fae(fae_);
		Splitting splitting(fae);
		TreeAut ta2(*fae.backend);
		if (fae_.getRoot(root)->isFinalState(i->first->rhs()))
		{
			ta.copyTransitions(ta2);
			size_t state = fae.freshState();
			ta2.addFinalState(state);
			const Transition& t = ta2.addTransition(i->first->lhs(), i->first->label(), state)->first;
			fae.setRoot(root, std::shared_ptr<TreeAut>(&ta2.uselessAndUnreachableFree(*fae.allocTA())));
			fae.connectionGraph.invalidate(root);
			std::set<const Box*> boxes;
			splitting.isolateAtRoot(root, t, IsolateBoxF(i->second), boxes);
			assert(boxes.count(i->second));
			Unfolding(fae).unfoldBox(root, i->second);
			splitting.isolateOne(dst, target, selector);
			continue;
		}
		ta2.addFinalStates(fae_.getRoot(root)->getFinalStates());
		for (TreeAut::iterator j = ta.begin(); j != ta.end(); ++j)
		{
			ta2.addTransition(*j);
			std::vector<size_t> lhs = j->lhs();
			for (std::vector<size_t>::iterator k = lhs.begin(); k != lhs.end(); ++k)
			{
				if (*k == i->first->rhs())
				{
					*k = fae.addData(ta2, Data::createRef(fae.getRootCount()));
					ta2.addTransition(lhs, j->label(), j->rhs());
					*k = i->first->rhs();
				}
			}
		}

		TreeAut ta3(*fae.backend);

		// ha! we can get inconsistent signatures here
		size_t offset = fae.nextState();

		ConnectionGraph::fixSignatures(ta3, ta2, offset);

		assert(ta3.getFinalStates().size());

		fae.setStateOffset(offset);

		ta2.clear();

		size_t state = fae.freshState();

		ta2.addFinalState(state);

		const Transition& t = ta2.addTransition(i->first->lhs(), i->first->label(), state)->first;

		ta.copyTransitions(ta2);

		fae.appendRoot(&ta2.uselessAndUnreachableFree(*fae.allocTA()));

		fae.connectionGraph.newRoot();
		fae.connectionGraph.invalidate(root);

		for (auto& f : ta3.getFinalStates())
		{
			FAE fae2(fae);

			Splitting splitting2(fae2);

			ta2.clear();

			ta3.copyTransitions(ta2);

			ta2.addFinalState(f);

			fae2.setRoot(root, std::shared_ptr<TreeAut>(&ta2.uselessAndUnreachableFree(*fae2.allocTA())));

			std::set<const Box*> boxes;

			splitting2.isolateAtRoot(fae2.getRootCount() - 1, t, IsolateBoxF(i->second), boxes);

			assert(boxes.count(i->second));

			Unfolding(fae2).unfoldBox(fae2.getRootCount() - 1, i->second);

			splitting2.isolateOne(dst, target, selector);
		}
	}
}


template <class F>
void Splitting::isolateAtRoot(
	size_t                             root,
	const Transition&                  t,
	F                                  f,
	std::set<const Box*>&              boxes)
{
	// Assertions
	assert(root < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(root));

	size_t newState = fae_.freshState();

	TreeAut ta(
		/* original TA */ *fae_.getRoot(root),
		/* copy final states? */ false
	);

	ta.addFinalState(newState);

	// the new tuple of children states of the newly created transition
	std::vector<size_t> lhs;
	// pointer to the processed state in the original tuple of children states
	size_t lhsOffset = 0;

	for (const AbstractBox* aBox : t.label()->getNode())
	{	// traverse all boxes in the label
		assert(nullptr != aBox);

		if (!aBox->isStructural())
		{	// we are not interested in non-structural boxes
			continue;
		}

		const StructuralBox* b = static_cast<const StructuralBox*>(aBox);
		if (!f(b))
		{	// in case this box is not interesting
			for (size_t k = 0; k < aBox->getArity(); ++k, ++lhsOffset)
			{	// push all states covered by the selectors
				lhs.push_back(t.lhs()[lhsOffset]);
			}
			continue;
		}

		// in case we are interested in the box, we have to isolate here
		for (size_t k = 0; k < aBox->getArity(); ++k, ++lhsOffset)
		{	// iterate over the selectors of the box
			if (FA::isData(t.lhs()[lhsOffset]))
			{	// no need to create a leaf when it's already there
				lhs.push_back(t.lhs()[lhsOffset]);
				continue;
			}

			// We split a tree automaton into two for a given transition at a given
			// selector by creating a new tuple of children states where states at
			// given selectors are substituted for new states that correspond to
			// references to newly created tree automata. The new automaton is created
			// by copying the original automaton without accepting states and making
			// the original state in the tuple the only accepting state of the
			// automaton.

			// update new left-hand side - add reference to the new TA
			lhs.push_back(fae_.addData(ta, Data::createRef(fae_.getRootCount())));
			// prepare new root
			TreeAut tmp(*fae_.getRoot(root), false);
			tmp.addFinalState(t.lhs()[lhsOffset]);
			TreeAut* tmp2 = fae_.allocTA();
			tmp.unreachableFree(*tmp2);
			// update 'o'
			fae_.appendRoot(tmp2);
			fae_.connectionGraph.newRoot();
		}

		if (b->isType(box_type_e::bBox))
		{	// insert the hierarchical box into a list
			boxes.insert(static_cast<const Box*>(aBox));
		}
	}

	// insert the new transition
	ta.addTransition(lhs, t.label(), newState);

	TreeAut* tmp = fae_.allocTA();

	ta.unreachableFree(*tmp);

	// exchange the original automaton with the new one
	fae_.setRoot(root, std::shared_ptr<TreeAut>(tmp));
	fae_.connectionGraph.invalidate(root);
}


void Splitting::isolateAtRoot(
	std::vector<FAE*>&                            dst,
	size_t                                        root,
	const std::vector<size_t>&                    offsets) const
{
	// Assertions
	assert(root < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(root));

	for (size_t state : fae_.getRoot(root)->getFinalStates())
	{	// for all final states
		for (TreeAut::iterator i = fae_.getRoot(root)->begin(state),
			end = fae_.getRoot(root)->end(state, i); i != end ; ++i)
		{	// traverse accepting transitions
			FAE fae(fae_);
			Splitting splitting(fae);
			std::set<const Box*> boxes;
			splitting.isolateAtRoot(root, *i, IsolateSetF(offsets), boxes);

			if (!boxes.empty())
			{	// in case there were some hierarchical boxes, process further
				Unfolding(fae).unfoldBoxes(root, boxes);
				splitting.isolateSet(dst, root, 0, offsets);
			} else
			{	// in case there were no hierarchical boxes, simply take the result
				dst.push_back(new FAE(fae));
			}
		}
	}
}


void Splitting::isolateSet(
	std::vector<FAE*>&                 dst,
	size_t                             target,
	int                                base,
	const std::vector<size_t>&         offsets) const
{
	// Assertions
	assert(target < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(target));

	std::vector<size_t> offsD;
	std::set<size_t> tmpS, offsU;

	this->enumerateSelectorsAtRoot(tmpS, target);

	for (size_t i : offsets)
	{ // distinct the selectors
		if (tmpS.find(base + i) != tmpS.end())
			offsD.push_back(base + i);
		else
			offsU.insert(base + i);
	}

	if (offsU.empty())
	{ // in case there is no upward selector for isolation
		this->isolateAtRoot(dst, target, offsD);
		return;
	}

	// guarded vectors of forest automata (all FA they be deleted at the end of
	// the function)
	std::vector<FAE*> tmp, tmp2;
	ContainerGuard<std::vector<FAE*>> g(tmp), f(tmp2);

	if (!offsD.empty())
	{	// in case there are some downward selectors, generate possible FA
		this->isolateAtRoot(
			/* the vector for the results */ tmp,
			/* the root index */ target,
			/* offsets */ offsD
		);
	}
	else
	{	// in case there are no downward selectors
		tmp.push_back(new FAE(fae_));
	}

	// now, the FAE to be further processed are stored in 'tmp'

	for (size_t i : offsU)
	{	// upward selectors are isolated separately 
		for (FAE* aut : tmp)
		{	// each FAE is also processed separately
			assert(nullptr != aut);

			tmpS.clear();
			Splitting splitting(*aut);

			// get root selectors (from the new FA) into tmpS
			splitting.enumerateSelectorsAtRoot(tmpS, target);
			if (tmpS.count(i))
			{	// in case 
				tmp2.push_back(new FAE(*aut));
			}
			else
			{
				bool found = false;
				aut->updateConnectionGraph();
				for (size_t k = 0; k < aut->getRootCount(); ++k)
				{
					if (!aut->getRoot(k) || !aut->connectionGraph.hasReference(k, target))
						continue;
					tmpS.clear();
					splitting.enumerateSelectorsAtLeaf(tmpS, k, target);
					if (tmpS.count(i))
					{
						splitting.isolateAtLeaf(tmp2, k, target, i);
						found = true;
						break;
					}
				}

				if (!found)
				{
					assert(false);
					//throw ProgramError("isolateSet(): selector lookup failed!");
				}
			}
		}

		utils::erase(tmp);
		std::swap(tmp, tmp2);
	}

	assert(tmp.size());

	dst.insert(dst.end(), tmp.begin(), tmp.end());

	g.release();
	f.release();
}
