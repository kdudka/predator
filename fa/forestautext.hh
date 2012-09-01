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
#include "programerror.hh"

class FAE : public FA {

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
			: index(index), offset(offset) {}

		size_t operator()(size_t s)
		{
			if (_MSB_TEST(s))
				return s;

			return this->index.translateOTF(s) + this->offset;
		}
	};

public:

	TreeAut& unique(
		TreeAut& dst,
		const TreeAut& src,
		bool addFinalStates = true)
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
		TreeAut& dst,
		const TreeAut& src,
		Index<size_t>& stateIndex,
		bool addFinalStates = true)
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

	static bool subseteq(const FAE& lhs, const FAE& rhs)
	{
		if (lhs.roots.size() != rhs.roots.size())
			return false;

		if (lhs.connectionGraph.data != rhs.connectionGraph.data)
			return false;

		for (size_t i = 0; i < lhs.roots.size(); ++i)
		{
			if (!TreeAut::subseteq(*lhs.roots[i], *rhs.roots[i]))
				return false;
		}

		return true;
	}

	template <class F>
	static void loadCompatibleFAs(
		std::vector<FAE*>& dst,
		const TreeAut& src,
		TreeAut::Backend& backend,
		BoxMan& boxMan,
		const FAE* fae,
		size_t stateOffset,
		F f)
	{
		TreeAut::td_cache_type cache;
		src.buildTDCache(cache);
		std::vector<const TT<label_type>*>& v = cache.insert(
			std::make_pair(0, std::vector<const TT<label_type>*>())).first->second;

		// iterate over all "synthetic" transitions and constuct new FAE for each
		for (const TT<label_type>* trans : v)
		{
			if (trans->lhs().size() != fae->roots.size())
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
				for (TreeAut::td_iterator k = src.tdStart(cache, itov(trans->lhs()[j]));
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

				if (!f(*fae, j, *fae->roots[j], *ta))
					break;
			}

			if (j < trans->lhs().size())
				continue;

			FAE* tmp = new FAE(backend, boxMan);
			dst.push_back(tmp);
			tmp->loadVars(fae->GetVariables());
			tmp->roots = roots;
			tmp->connectionGraph = fae->connectionGraph;
			tmp->stateOffset = stateOffset;
		}
	}

	template <class F>
	void fuse(const std::vector<FAE*>& src, F f) {
		if (src.empty())
			return;
		Index<size_t> index;
		for (std::vector<FAE*>::const_iterator i = src.begin(); i != src.end(); ++i) {
			assert(this->roots.size() == (*i)->roots.size());
			for (size_t j = 0; j < this->roots.size(); ++j) {
				if (!f(j, *i))
					continue;
				index.clear();
				TreeAut::rename(*this->roots[j], *(*i)->roots[j], RenameNonleafF(index, this->nextState()), false);
				this->incrementStateOffset(index.size());
			}
		}
	}

	template <class F, class G>
	void fuse(const TreeAut& src, F f, G g) {
		Index<size_t> index;
		TreeAut tmp2(src, g);
		TreeAut tmp(*this->backend);
		TreeAut::rename(tmp, tmp2, RenameNonleafF(index, this->nextState()), false);
		this->incrementStateOffset(index.size());
		for (size_t i = 0; i < this->roots.size(); ++i) {
			if (!f(i, nullptr))
				continue;
			tmp.copyTransitions(*this->roots[i]);
		}
	}

	void minimizeRoots() {
		for (size_t i = 0; i < this->roots.size(); ++i) {
			if (!this->roots[i])
				continue;
			this->roots[i] = std::shared_ptr<TreeAut>(&this->roots[i]->minimized(*this->allocTA()));
		}
	}

	void minimizeRootsCombo() {
		for (size_t i = 0; i < this->roots.size(); ++i) {
			if (!this->roots[i])
				continue;
			this->roots[i] = std::shared_ptr<TreeAut>(&this->roots[i]->minimizedCombo(*this->allocTA()));
		}
	}

	void unreachableFree(std::shared_ptr<TreeAut>& ta) {
		auto tmp = ta;
		ta = std::shared_ptr<TreeAut>(this->allocTA());
		tmp->unreachableFree(*ta);
	}

	void unreachableFree() {

		for (auto& ta : this->roots) {

			if (ta)
				this->unreachableFree(ta);

		}

	}

public:

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

	void setStateOffset(size_t offset) {
		this->stateOffset = offset;
	}

	void pushStateOffset() {
		this->savedStateOffset = this->stateOffset;
	}

	void popStateOffset() {
		this->stateOffset = this->savedStateOffset;
	}

	size_t addData(TreeAut& dst, const Data& data) {
		label_type label = this->boxMan->lookupLabel(data);
		size_t state = _MSB_ADD(label->getDataId());
		dst.addTransition(std::vector<size_t>(), label, state);
		return state;
	}

	bool isData(size_t state, const Data*& data) const {
		if (!FA::isData(state))
			return false;
		data = &this->boxMan->getData(_MSB_GET(state));
		return true;
	}

	const Data& getData(size_t state) const {

		assert(FA::isData(state));

		return this->boxMan->getData(_MSB_GET(state));

	}

	bool getRef(size_t state, size_t& ref) const {
		if (!FA::isData(state))
			return false;
		const Data& data = this->boxMan->getData(_MSB_GET(state));
		if (!data.isRef())
			return false;
		ref = data.d_ref.root;
		return true;
	}

	size_t getRef(size_t state) const {

		assert(FA::isData(state));

		const Data& data = this->boxMan->getData(_MSB_GET(state));

		assert(data.isRef());

		return data.d_ref.root;

	}

	static bool isRef(label_type label) {
		if (!label->isData())
			return false;
		return label->getData().isRef();
	}


	static bool isRef(label_type label, size_t ref) {
		if (!label->isData())
			return false;
		return label->getData().isRef(ref);
	}

	static bool getRef(label_type label, size_t& ref) {
		if (!label->isData())
			return false;
		if (!label->getData().isRef())
			return false;
		ref = label->getData().d_ref.root;
		return true;
	}

	TreeAut& relabelReferences(TreeAut& dst, const TreeAut& src, const std::vector<size_t>& index) {
		dst.addFinalStates(src.getFinalStates());
		for (TreeAut::iterator i = src.begin(); i != src.end(); ++i) {
			if (i->label()->isData())
				continue;
			std::vector<size_t> lhs;
			for (std::vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j) {
				const Data* data;
				if (this->isData(*j, data)) {
					if (data->isRef()) {
						if (index[data->d_ref.root] != static_cast<size_t>(-1))
							lhs.push_back(this->addData(dst, Data::createRef(index[data->d_ref.root], data->d_ref.displ)));
						else
							lhs.push_back(this->addData(dst, Data::createUndef()));
					} else {
						lhs.push_back(this->addData(dst, *data));
					}

				} else
					lhs.push_back(*j);
			}
			dst.addTransition(lhs, i->label(), i->rhs());
		}
		return dst;
	}

	TreeAut* relabelReferences(TreeAut* src, const std::vector<size_t>& index) {
		return &this->relabelReferences(*this->allocTA(), *src, index);
	}

	TreeAut& invalidateReference(TreeAut& dst, const TreeAut& src, size_t root) {
		dst.addFinalStates(src.getFinalStates());
		for (TreeAut::iterator i = src.begin(); i != src.end(); ++i) {
			std::vector<size_t> lhs;
			for (std::vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j) {
				const Data* data;
				if (FAE::isData(*j, data) && data->isRef(root)) {
					lhs.push_back(this->addData(dst, Data::createUndef()));
				} else {
					lhs.push_back(*j);
				}
			}
			if (!FAE::isRef(i->label(), root))
				dst.addTransition(lhs, i->label(), i->rhs());
		}
		return dst;
	}

	TreeAut* invalidateReference(TreeAut* src, size_t root)
	{
		return &this->invalidateReference(*this->allocTA(), *src, root);
	}

public:

	void buildLTCacheExt(
		const TreeAut& ta,
		TreeAut::lt_cache_type& cache)
	{
		label_type lUndef = this->boxMan->lookupLabel(Data::createUndef());
		for (TreeAut::iterator i = ta.begin(); i != ta.end(); ++i) {
			if (i->label()->isData()) {
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

	const TypeBox* getType(size_t target) const
	{
		// Assertions
		assert(target < this->roots.size());
		assert(this->roots[target]);
		assert(this->roots[target]->getFinalStates().size());

		return static_cast<const TypeBox*>(this->roots[target]->begin(
			*this->roots[target]->getFinalStates().begin()
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
