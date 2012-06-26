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

bool Integrity::checkState(
	const TreeAut& ta,
	size_t state,
	const std::set<size_t>& defined,
	std::vector<bool>& bitmap,
	std::map<std::pair<const TreeAut*, size_t>, std::set<size_t>>& states) const
{
	const Data* data;

	if (this->fae.isData(state, data))
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
	assert(root < this->fae.roots.size());
	assert(this->fae.roots[root]);
	assert(root < bitmap.size());

	if (bitmap[root])
		return true;

	bitmap[root] = true;

	std::set<size_t> tmp;

	this->enumerateSelectorsAtLeaf(tmp, root);

	for (auto s : this->fae.roots[root]->getFinalStates())
	{
		if (!this->checkState(*this->fae.roots[root], s, tmp, bitmap, states))
			return false;
	}

	return true;
}

bool Integrity::check() const
{
	std::vector<bool> bitmap(this->fae.roots.size(), false);
	std::map<std::pair<const TreeAut*, size_t>, std::set<size_t>> states;

	for (size_t i = 0; i < this->fae.roots.size(); ++i)
	{
		if (!this->fae.roots[i])
			continue;

		if (!this->checkRoot(i, bitmap, states))
		{
			CL_CDEBUG(1, "inconsistent heap: " << std::endl << this->fae);

			return false;
		}
	}

	return true;
}
