/*
 * Copyright (C) 2012 Jiri Simacek
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
#include "integrity.hh"
#include "streams.hh"


struct LeafEnumF
{
	const FAE& fae;
	const TreeAut::Transition& t;
	size_t target;
	std::set<size_t>& selectors;


	LeafEnumF(
		const FAE& fae,
		const TreeAut::Transition& t,
		size_t target,
		std::set<size_t>& selectors
	) :
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
		for (size_t k = 0; k < box->getArity(); ++k, ++offset) {
			size_t ref;
			if (fae.getRef(this->t.lhs()[offset], ref) && ref == this->target)
				this->selectors.insert(box->inputCoverage(k).begin(), box->inputCoverage(k).end());
		}
		return true;
	}
};


/**
 * @brief  Functor for checking integrity of a label
 */
struct Integrity::CheckIntegrityF
{
	/// the integrity object
	const Integrity& integrity;

	/// the tree automaton
	const TreeAut& ta;

	/// the transition that is checked
	const TT<label_type>& t;

	/// the set of states which can only appear as cover in a structural box
	std::set<size_t>* required;

	/// the set of already processed tree automata
	std::vector<bool>& bitmap;

	/// the map of processed states
	std::map<std::pair<const TreeAut*, size_t>, std::set<size_t>>& states;


	CheckIntegrityF(
		const Integrity& integrity,
		const TreeAut& ta,
		const TT<label_type>& t,
		std::set<size_t>* required,
		std::vector<bool>& bitmap,
		std::map<std::pair<const TreeAut*, size_t>, std::set<size_t>>& states
	) :
		integrity(integrity),
		ta(ta),
		t(t),
		required(required),
		bitmap(bitmap),
		states(states)
	{ }


	bool operator()(const AbstractBox* aBox, size_t, size_t offset)
	{
		switch (aBox->getType())
		{
			case box_type_e::bBox:
			{
				const Box* tmp = static_cast<const Box*>(aBox);

				for (size_t i = 0; i < tmp->getArity(); ++i)
				{
					assert(offset + i < this->t.lhs().size());

					const Data* data;

					if (this->integrity.fae_.isData(this->t.lhs()[offset + i], data) && !data->isRef() && !data->isUndef())
						return false;

					if (!this->integrity.checkState(this->ta, this->t.lhs()[offset + i], tmp->inputCoverage(i), this->bitmap, states))
						return false;
				}

				break;
			}

			case  box_type_e::bSel:
			{
				assert(offset < this->t.lhs().size());

				if (!this->integrity.checkState(this->ta, this->t.lhs()[offset], std::set<size_t>(), this->bitmap, states))
					return false;

				break;
			}

			default:
				break;
		}

		if (this->required && aBox->isStructural())
		{
			for (auto s : (static_cast<const StructuralBox*>(aBox))->outputCoverage())
			{
				if (!this->required->erase(s))
					return false;
			}
		}

		return true;
	}
};


void Integrity::enumerateSelectorsAtLeaf(
	std::set<size_t>& selectors,
	size_t target) const
{
	for (auto p_ta : fae_.getRoots())
	{
		if (!p_ta)
			continue;

		for (auto rule : *p_ta)
		{
			if (rule.label()->isNode())
				rule.label()->iterate(LeafEnumF(fae_, rule, target, selectors));
		}
	}
}


bool Integrity::checkState(
	const TreeAut& ta,
	size_t state,
	const std::set<size_t>& defined,
	std::vector<bool>& bitmap,
	std::map<std::pair<const TreeAut*, size_t>, std::set<size_t>>& states) const
{
	const Data* data;

	if (fae_.isData(state, data))
	{
		if (!data->isRef())
			return true;

		return this->checkRoot(data->d_ref.root, bitmap, states);
	}

	auto p = states.insert(std::make_pair(std::make_pair(&ta, state), defined));

	if (!p.second)
		return (defined == p.first->second);

	for (TreeAut::iterator i = ta.begin(state); i != ta.end(state); ++i)
	{
		const TypeBox* typeBox = static_cast<const TypeBox*>(i->label()->boxLookup(static_cast<size_t>(-1), nullptr));

		assert(typeBox);

		const std::vector<size_t>& sels = typeBox->getSelectors();

		std::set<size_t> tmp(sels.begin(), sels.end());

		for (auto s : defined)
		{
			if (tmp.erase(s) != 1)
				return false;
		}

		if (!i->label()->iterate(CheckIntegrityF(*this, ta, *i, &tmp, bitmap, states)))
			return false;

		if (!tmp.empty())
			return false;
	}

	return true;
}


bool Integrity::checkRoot(
	size_t root,
	std::vector<bool>& bitmap,
	std::map<std::pair<const TreeAut*, size_t>, std::set<size_t>>& states) const
{
	// Assertions
	assert(root < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(root));
	assert(root < bitmap.size());

	if (bitmap[root])
		return true;

	bitmap[root] = true;

	std::set<size_t> tmp;

	this->enumerateSelectorsAtLeaf(tmp, root);

	for (auto s : fae_.getRoot(root)->getFinalStates())
	{
		if (!this->checkState(*fae_.getRoot(root), s, tmp, bitmap, states))
			return false;
	}

	return true;
}


bool Integrity::check() const
{
	std::vector<bool> bitmap(fae_.getRootCount(), false);
	std::map<std::pair<const TreeAut*, size_t>, std::set<size_t>> states;

	for (size_t i = 0; i < fae_.getRootCount(); ++i)
	{
		if (!fae_.getRoot(i))
			continue;

		if (!this->checkRoot(i, bitmap, states))
		{
			FA_DEBUG_AT(1, "inconsistent heap: " << std::endl << fae_);

			return false;
		}
	}

	return true;
}
