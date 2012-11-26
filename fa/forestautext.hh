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
	friend class Abstraction;
	friend class Splitting;
	friend class VirtualMachine;

	BoxMan* boxMan;

	size_t stateOffset;
	size_t savedStateOffset;

public:

	struct RenameNonleafF
	{
		Index<size_t>& index;

		size_t offset;

		RenameNonleafF(Index<size_t>& index, size_t offset = 0)
			: index(index), offset(offset)
		{ }

		size_t operator()(size_t s)
		{
			if (_MSB_TEST(s))
				return s;

			return this->index.translateOTF(s) + this->offset;
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

public:

	static bool subseteq(const FAE& lhs, const FAE& rhs);

	template <class F>
	static void loadCompatibleFAs(
		std::vector<FAE*>&              dst,
		const TreeAut&                  src,
		TreeAut::Backend&               backend,
		BoxMan&                         boxMan,
		const FAE*                      fae,
		size_t                          stateOffset,
		F                               f)
	{
		TreeAut::td_cache_type cache;
		src.buildTDCache(cache);
		std::vector<const TT<label_type>*>& v = cache.insert(
			std::make_pair(0, std::vector<const TT<label_type>*>())).first->second;

		// iterate over all "synthetic" transitions and constuct new FAE for each
		for (const TT<label_type>* trans : v)
		{
			if (trans->lhs().size() != fae->getRootCount())
				continue;
			if (trans->label()->getVData() != fae->GetVariables())
				continue;

			std::vector<std::shared_ptr<TreeAut>> roots;
			size_t j;
			for (j = 0; j != trans->lhs().size(); ++j)
			{
				TreeAut* ta = new TreeAut(backend);
				roots.push_back(std::shared_ptr<TreeAut>(ta));

				// add reachable transitions
				for (TreeAut::td_iterator k = src.tdStart(cache, {trans->lhs()[j]});
					k.isValid();
					k.next())
				{
					ta->addTransition(*k);
				}

				ta->addFinalState(trans->lhs()[j]);

				// compute signatures
				ConnectionGraph::StateToCutpointSignatureMap stateMap;

				ConnectionGraph::computeSignatures(stateMap, *ta);

				auto k = stateMap.find(trans->lhs()[j]);

				if (k == stateMap.end())
				{
					if (!fae->connectionGraph.data[roots.size() - 1].signature.empty())
						break;
				}
				else
				{
					if (k->second != fae->connectionGraph.data[roots.size() - 1].signature)
						break;
				}

				if (!f(*fae, j, *fae->getRoot(j), *ta))
					break;
			}

			if (j < trans->lhs().size())
				continue;

			FAE* tmp = new FAE(backend, boxMan);
			dst.push_back(tmp);
			tmp->loadVars(fae->GetVariables());
			tmp->roots_ = roots;
			tmp->connectionGraph = fae->connectionGraph;
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
		for (FAE* fae : src)
		{
			assert(this->getRootCount() == fae->getRootCount());
			for (size_t j = 0; j < this->getRootCount(); ++j)
			{
				if (!f(j, fae))
					continue;
				index.clear();
				TreeAut::rename(*roots_[j], *fae->getRoot(j),
					RenameNonleafF(index, this->nextState()), false);
				this->incrementStateOffset(index.size());
			}
		}
	}

	template <class F, class G>
	void fuse(
		const TreeAut&       src,
		F                    f,
		G                    g)
	{
		Index<size_t> index;
		TreeAut tmp2(src, g);
		TreeAut tmp(*this->backend);
		TreeAut::rename(tmp, tmp2, RenameNonleafF(index, this->nextState()), false);
		this->incrementStateOffset(index.size());
		for (size_t i = 0; i < this->getRootCount(); ++i)
		{
			if (!f(i, nullptr))
				continue;
			tmp.copyTransitions(*roots_[i]);
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

	size_t nextState()
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
		return &this->relabelReferences(*this->allocTA(), *src, index);
	}


	TreeAut& invalidateReference(
		TreeAut&                       dst,
		const TreeAut&                 src,
		size_t                         root);


	TreeAut* invalidateReference(TreeAut* src, size_t root)
	{
		return &this->invalidateReference(*this->allocTA(), *src, root);
	}

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
