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

#ifndef FOREST_AUT_EXT_H
#define FOREST_AUT_EXT_H

// Standard library headers
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <algorithm>

// Forester headers
#include "forestaut.hh"
#include "boxman.hh"
#include "tatimint.hh"
#include "utils.hh"

class FAE : public FA
{
	friend class Normalization;
	friend class Folding;
	friend class Unfolding;
	friend class VirtualMachine;

private:  // data members

	BoxMan* boxMan;

	size_t stateOffset;
	size_t savedStateOffset;

public:

	class RenameNonleafF
	{
	private:  // data members

		Index<size_t>& index_;

		size_t offset_;

	public:   // methods

		RenameNonleafF(Index<size_t>& index, size_t offset = 0) :
			index_(index),
			offset_(offset)
		{ }

		size_t operator()(size_t s)
		{
			if (FA::isData(s))
			{	// for data states
				return s;
			}
			else
			{	// for internal states
				return index_.translateOTF(s) + offset_;
			}
		}
	};

public:

	TreeAut& unique(
		TreeAut&          dst,
		const TreeAut&    src,
		bool              addFinalStates = true)
	{
		Index<size_t> stateIndex;

		TreeAut::rename(
			dst,
			src,
			RenameNonleafF(stateIndex, this->nextState()), addFinalStates
		);

		this->incrementStateOffset(stateIndex.size());
		return dst;
	}

	TreeAut& unique(
		TreeAut&          dst,
		const TreeAut&    src,
		Index<size_t>&    stateIndex,
		bool              addFinalStates = true)
	{
		TreeAut::rename(
			dst,
			src,
			RenameNonleafF(stateIndex, this->nextState()), addFinalStates
		);

		this->incrementStateOffset(stateIndex.size());
		return dst;
	}

	void makeDisjoint(size_t root)
	{
		// Preconditions
		assert(root < roots_.size());

		roots_[root] = std::shared_ptr<TreeAut>(&this->unique(*this->allocTA(), *roots_[root]));
	}

public:

	static bool subseteq(const FAE& lhs, const FAE& rhs);


	/**
	 * @brief  Loads compatible FA from a wrapping TA
	 *
	 * This method takes a wrapping TA @p src and breaks it into FA. Then it
	 * takes those FA which are compatible with @p fae and returns them in @p
	 * dst.
	 *
	 * @param[out]  dst          The result vector where the FA will be filled
	 * @param[in]   src          The wrapping TA which contains the FA
	 * @param[in]   backend      The TA backend
	 * @param[in]   boxMan       The used box manager
	 * @param[in]   fae          The FA with which the loaded FA are supposed to
	 *                           be compatible
	 * @param[in]   stateOffset  The offset for renaming states
	 * @param[in]   funcCompat   The functor that checks additional compatibility
	 *                           restraints
	 */
	template <class F>
	static void loadCompatibleFAs(
		std::vector<FAE*>&              dst,
		const TreeAut&                  src,
		TreeAut::Backend&               backend,
		BoxMan&                         boxMan,
		const FAE&                      fae,
		size_t                          stateOffset,
		F                               funcCompat)
	{
		// build TD cache and insert empty set of root transitions (if not present)
		TreeAut::td_cache_type cache = src.buildTDCache();
		std::vector<const Transition*>& v = cache.insert(
			std::make_pair(0, std::vector<const Transition*>())).first->second;

		for (const Transition* trans : v)
		{ // iterate over all "synthetic" transitions and constuct new FAE for each
			assert(nullptr != trans);

			const size_t& numRoots = trans->lhs().size();
			if ((fae.getRootCount() != numRoots) ||
				(fae.GetVariables() != trans->label()->getVData()))
			{	// in case the number of components or global variables does not match
				continue;
			}

			std::vector<std::shared_ptr<TreeAut>> roots;
			size_t j;
			for (j = 0; j != numRoots; ++j)
			{	// for all TA in the FA
				assert(roots.size() == j);

				TreeAut* ta = new TreeAut(backend);
				roots.push_back(std::shared_ptr<TreeAut>(ta));

				const size_t& rootState = trans->lhs()[j];

				for (TreeAut::td_iterator k = src.tdStart(cache, {rootState});
					k.isValid();
					k.next())
				{ // copy reachable transitions
					ta->addTransition(*k);
				}

				ta->addFinalState(rootState);

				// compute signatures
				ConnectionGraph::StateToCutpointSignatureMap stateMap;
				ConnectionGraph::computeSignatures(stateMap, *ta);

				auto k = stateMap.find(rootState);
				if (k == stateMap.end())
				{	// in case the 'j'-th root has no cutpoint info
					if (!fae.connectionGraph.data[j].signature.empty())
					{	// in case the 'j'-th root in FA has some cutpoint info
						break;      // the FA are not compatible
					}
				}
				else
				{	// in case the 'j'-th root has some cutpoint info
					if (k->second != fae.connectionGraph.data[j].signature)
					{	// in case the info is not compatible to the one in FA
						break;      // the FA are not compatible
					}
				}

				if (!funcCompat(fae, j, *fae.getRoot(j), *ta))
				{	// in case additional compatibility chack failed
					break;        // the FA are not compatible
				}
			}

			if (numRoots!= j)
			{	// in case the previous loop was interrupted at some point
				continue;   // try another FA
			}

			// build the FA
			FAE* tmp = new FAE(backend, boxMan);
			dst.push_back(tmp);
			tmp->loadVars(fae.GetVariables());
			tmp->roots_ = roots;
			tmp->connectionGraph = fae.connectionGraph;
			tmp->stateOffset = stateOffset;
		}
	}


	template <class F>
	void fuse(
		const std::vector<FAE*>&       src,
		F                              f)
	{
		if (src.empty())
			return;

		Index<size_t> index;
		for (const FAE* fae : src)
		{
			assert(this->getRootCount() == fae->getRootCount());
			for (size_t j = 0; j < this->getRootCount(); ++j)
			{
				if (!f(j, fae))
					continue;
				index.clear();
				TreeAut::rename(
					/* destination */ *roots_[j],
					/* src */ *fae->getRoot(j),
					/* renaming fctor */ RenameNonleafF(index, this->nextState()),
					/* copy final states? */ false);

				this->incrementStateOffset(index.size());
			}
		}
	}

	void minimizeRoots()
	{
		for (std::shared_ptr<TreeAut> ta : roots_)
		{
			if (!ta)
				continue;

			ta = std::shared_ptr<TreeAut>(&ta->minimized(*this->allocTA()));
		}
	}

	void minimizeRootsCombo()
	{
		for (std::shared_ptr<TreeAut> ta : roots_)
		{
			if (!ta)
				continue;

			ta = std::shared_ptr<TreeAut>(&ta->minimizedCombo(*this->allocTA()));
		}
	}

	void unreachableFree(std::shared_ptr<TreeAut>& ta)
	{
		std::shared_ptr<TreeAut> tmp = ta;
		ta = std::shared_ptr<TreeAut>(this->allocTA());
		tmp->unreachableFree(*ta);
	}

	void unreachableFree()
	{
		for (std::shared_ptr<TreeAut>& ta : roots_)
		{
			if (ta)
				this->unreachableFree(ta);
		}
	}

public:

	void newState()
	{
		++this->stateOffset;
	}

	size_t nextState() const
	{
		return this->stateOffset;
	}

	size_t freshState()
	{
		return this->stateOffset++;
	}

	void incrementStateOffset(size_t amount)
	{
		this->stateOffset += amount;
	}

	void setStateOffset(size_t offset)
	{
		this->stateOffset = offset;
	}

	void pushStateOffset()
	{
		this->savedStateOffset = this->stateOffset;
	}

	void popStateOffset()
	{
		this->stateOffset = this->savedStateOffset;
	}

	size_t addData(TreeAut& dst, const Data& data)
	{
		label_type label = this->boxMan->lookupLabel(data);
		size_t state = _MSB_ADD(label->getDataId());
		dst.addTransition(std::vector<size_t>(), label, state);
		return state;
	}

	bool isData(size_t state, const Data*& data) const
	{
		if (!FA::isData(state))
			return false;
		data = &this->boxMan->getData(_MSB_GET(state));
		return true;
	}

	const Data& getData(size_t state) const
	{
		assert(FA::isData(state));

		return this->boxMan->getData(_MSB_GET(state));
	}

	bool getRef(size_t state, size_t& ref) const
	{
		if (!FA::isData(state))
			return false;

		const Data& data = this->boxMan->getData(_MSB_GET(state));
		if (!data.isRef())
			return false;

		ref = data.d_ref.root;
		return true;
	}

	size_t getRef(size_t state) const
	{
		assert(FA::isData(state));

		const Data& data = this->boxMan->getData(_MSB_GET(state));

		assert(data.isRef());

		return data.d_ref.root;
	}

	static bool isRef(label_type label)
	{
		if (!label->isData())
			return false;

		return label->getData().isRef();
	}


	static bool isRef(label_type label, size_t ref)
	{
		if (!label->isData())
			return false;

		return label->getData().isRef(ref);
	}

	static bool getRef(label_type label, size_t& ref)
	{
		if (!label->isData())
			return false;

		if (!label->getData().isRef())
			return false;

		ref = label->getData().d_ref.root;
		return true;
	}

	TreeAut& relabelReferences(
		TreeAut&                      dst,
		const TreeAut&                src,
		const std::vector<size_t>&    index);


	TreeAut* relabelReferences(
		TreeAut*                       src,
		const std::vector<size_t>&     index)
	{
		// Preconditions
		assert(nullptr != src);

		return &this->relabelReferences(*this->allocTA(), *src, index);
	}


	TreeAut& invalidateReference(
		TreeAut&                       dst,
		const TreeAut&                 src,
		size_t                         root);


	TreeAut* invalidateReference(TreeAut* src, size_t root)
	{
		// Preconditions
		assert(nullptr != src);

		return &this->invalidateReference(*this->allocTA(), *src, root);
	}


	/**
	 * @brief  Frees a position in the FA by moving given TA away
	 *
	 * This method frees a position at index @p root in the forest automaton. In
	 * case there is some tree automaton at the position, it moves it to some
	 * other position and relabels references accordingly.
	 *
	 * @param[in]  root  The index in the FA to be freed
	 */
	void freePosition(size_t root);


public:

	void buildLTCacheExt(
		const TreeAut&               ta,
		TreeAut::lt_cache_type&      cache);

	const TypeBox* getType(size_t target) const
	{
		// Assertions
		assert(target < this->getRootCount());
		assert(nullptr != this->getRoot(target));
		assert(!this->getRoot(target)->getFinalStates().empty());

		return static_cast<const TypeBox*>(this->getRoot(target)->begin(
			*this->getRoot(target)->getFinalStates().begin()
		)->label()->boxLookup(static_cast<size_t>(-1)).aBox);
	}


	/**
	 * @brief  Makes a product of states of two FA
	 *
	 * This method performs a parallel traversal through a pair of forest
	 * automata @p lhs and @p rhs and generates all possible mappings of states
	 * between these two automata.
	 *
	 * @param[in]   lhs     First forest automaton
	 * @param[in]   rhs     The other forest automaton
	 * @param[out]  result  A container with achieved mappings
	 */
	static void makeProduct(
		const FAE&                             lhs,
		const FAE&                             rhs,
		std::set<std::pair<size_t, size_t>>&   result);


public:

	// state 0 should never be allocated by FAE (?)
	FAE(TreeAut::Backend& backend, BoxMan& boxMan) :
		FA(backend),
		boxMan(&boxMan),
		stateOffset(1),
		savedStateOffset()
	{ }

	FAE(const FAE& x) :
		FA(x),
		boxMan(x.boxMan),
		stateOffset(x.stateOffset),
		savedStateOffset{}
	{ }

	~FAE()
	{
		this->clear();
	}

	FAE& operator=(const FAE& x)
	{
		if (this != &x)
		{
			FA::operator=(x);
			this->boxMan = x.boxMan;
			this->stateOffset = x.stateOffset;
		}

		return *this;
	}

	void clear()
	{
		FA::clear();
		this->stateOffset = 1;
	}
};

#endif
