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


/**
 * @brief  Functor checking for the presence of a backward box reference 
 *
 * This functor checks the presence of a backward box reference into the given
 * tree automaton.
 *
 * To be used in NodeLabel::iterate().
 *
 */
struct LeafScanF
{
	const FAE& fae;
	const Splitting::Transition& trans;
	size_t selector;
	size_t target;
	const Box*& matched;

	/**
	 * @brief  
	 *
	 * TODO
	 *
	 * @param[in]  fae       The forest automaton
	 * @param[in]  trans     The source transition
	 * @param[in]  selector  The backward selector to be checked for presence
	 * @param[in]  target    The target root index
	 * @param[out] matched   The box that will contain given selector
	 */
	LeafScanF(
		const FAE&                     fae,
		const Splitting::Transition&   trans,
		size_t                         selector,
		size_t                         target,
		const Box*&                    matched) :
		fae(fae),
		trans(trans),
		selector(selector),
		target(target),
		matched(matched)
	{ }

	bool operator()(const AbstractBox* aBox, size_t, size_t offset)
	{
		if (!aBox->isType(box_type_e::bBox))
		{	// in case the box is not a nested FA
			return true;
		}

		const Box* box = static_cast<const Box*>(aBox);
		for (size_t k = 0; k < box->getArity(); ++k, ++offset)
		{	// traverse the box
			size_t ref;
			if (fae.getRef(this->trans.lhs()[offset], ref) &&
				ref == this->target && box->inputCovers(k, this->selector))
			{	// if the state at 'offset' is a reference to the 'target' automaton and
				// the input of the box covers the 'selector'
				this->matched = box;

				// stop the iteration
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

	bool operator()(const StructuralBox* box) const
	{
		return box->outputCovers(this->offset);
	}

	friend std::ostream& operator<<(std::ostream& os, const IsolateOneF& f)
	{
		return os << f.offset;
	}
};


struct IsolateBoxF
{
	const Box* box;

	IsolateBoxF(const Box* box) : box(box) {}

	bool operator()(const StructuralBox* box) const
	{
		return this->box == box;
	}

	friend std::ostream& operator<<(std::ostream& os, const IsolateBoxF& f)
	{
		return os << *static_cast<const AbstractBox*>(f.box);
	}
};


struct IsolateSetF
{
	std::set<size_t> s;

	IsolateSetF(
		const std::vector<size_t>&        v,
		size_t                            offset = 0) :
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
		for (const size_t& x : f.s)
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
	fae_.getRoot(target)->accBegin()->label()->iterate(RootEnumF(target, selectors));
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

	// sanitize the automaton
	fae_.unreachableFree(fae_.getRoot(root));

	// vector for pairs of transitions and boxes
	std::vector<std::pair<const Transition*, const Box*>> v;

	// create a new blank automaton
	TreeAut ta(*fae_.backend);

	for (const Transition& trans : *fae_.getRoot(root))
	{	// traverse accepting transitions
		if (!trans.label()->isNode())
		{	// copy non-nodes
			ta.addTransition(trans);
			continue;
		}

		const Box* matched = nullptr;
		trans.label()->iterate(LeafScanF(fae_, trans, selector, target, matched));
		if (matched)
		{	// in case the selector was found in the label
			v.push_back(std::make_pair(&trans, matched));
		}
		else
		{	// in case the label does not contain the selector
			ta.addTransition(trans);
		}
	}

	assert(!v.empty());

	for (std::pair<const Transition*, const Box*>& transBox : v)
	{
		FAE fae(fae_);
		Splitting splitting(fae);

		// create an empty automaton
		TreeAut ta2(*fae.backend);
		if (fae_.getRoot(root)->isFinalState(transBox.first->rhs()))
		{	// in case the parent state is a root
			ta.copyTransitions(ta2);
			size_t state = fae.freshState();
			ta2.addFinalState(state);
			const Transition& t = ta2.addTransition(transBox.first->lhs(), transBox.first->label(), state)->first;
			fae.setRoot(root, std::shared_ptr<TreeAut>(&ta2.uselessAndUnreachableFree(*fae.allocTA())));
			fae.connectionGraph.invalidate(root);
			std::set<const Box*> boxes;
			splitting.isolateAtRoot(root, t, IsolateBoxF(transBox.second), boxes);
			assert(boxes.count(transBox.second));
			Unfolding(fae).unfoldBox(root, transBox.second);
			splitting.isolateOne(dst, target, selector);

			continue;
		}

		// if the parent state is not a root

		ta2.addFinalStates(fae_.getRoot(root)->getFinalStates());
		for (const Transition& trans : ta)
		{	// copy the transitions
			ta2.addTransition(trans);
			std::vector<size_t> lhs = trans.lhs();
			for (std::vector<size_t>::iterator k = lhs.begin(); k != lhs.end(); ++k)
			{	// alter all transitions to the parent state of the found transition to
				// become transitions to references of a new FA
				if (*k == transBox.first->rhs())
				{
					// TODO: several transitions may be added, is it correct??
					*k = fae.addData(ta2, Data::createRef(fae.getRootCount()));
					ta2.addTransition(lhs, trans.label(), trans.rhs());
					*k = transBox.first->rhs();
				}
			}
		}

		// create another empty automaton
		TreeAut ta3(*fae.backend);

		// ha! we can get inconsistent signatures here
		size_t offset = fae.nextState();

		// TODO: what the hell is this function doing??? Couldn't there be at least
		// *some* documentation???!?!?!? grrrrrrrrr
		// do sometning and copy 'ta2' to 'ta3', possibly modify 'offset'
		ConnectionGraph::fixSignatures(ta3, ta2, offset);

		assert(ta3.getFinalStates().size());

		fae.setStateOffset(offset);

		ta2.clear();
		size_t state = fae.freshState();    // create a new root state
		ta2.addFinalState(state);

		// insert a root transition
		ta2.addTransition(transBox.first->lhs(), transBox.first->label(), state)->first;

		// copy to 'ta2' transitions from 'ta'
		ta.copyTransitions(ta2);

		// push 'ta2' into the FA and shake it all up
		fae.appendRoot(&ta2.uselessAndUnreachableFree(*fae.allocTA()));
		fae.connectionGraph.newRoot();
		fae.makeDisjoint(fae.getRootCount() - 1);
		fae.connectionGraph.invalidate(root);

		// reload the transition after making the automata disjoint
		const std::shared_ptr<TreeAut> pTa = fae.getRoot(fae.getRootCount() - 1);
		const Transition& t = pTa->getAcceptingTransition();


		for (const size_t& f : ta3.getFinalStates())
		{	// for each final state of 'ta3' construct a new FA
			FAE fae2(fae);                // create a new FA
			Splitting splitting2(fae2);

			ta2.clear();                  // start with blank 'ta2'
			ta3.copyTransitions(ta2);     // copy transitions from 'ta3' to 'ta2'
			ta2.addFinalState(f);         // set current final state

			// set 'ta2' as the new TA at given index
			fae2.setRoot(root,
				std::shared_ptr<TreeAut>(&ta2.uselessAndUnreachableFree(*fae2.allocTA())));

			std::set<const Box*> boxes;

			splitting2.isolateAtRoot(fae2.getRootCount() - 1, t,
				IsolateBoxF(transBox.second), boxes);

			assert(boxes.count(transBox.second));

			Unfolding(fae2).unfoldBox(fae2.getRootCount() - 1, transBox.second);

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
			fae_.appendRoot(tmp2);
			fae_.makeDisjoint(fae_.getRootCount() - 1);
			// update 'o'
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
