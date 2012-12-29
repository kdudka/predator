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

// Forester headers
#include "forestautext.hh"
#include "streams.hh"

bool FAE::subseteq(const FAE& lhs, const FAE& rhs)
{
	if (lhs.getRootCount() != rhs.getRootCount())
		return false;

	if (lhs.connectionGraph.data != rhs.connectionGraph.data)
		return false;

	for (size_t i = 0; i < lhs.getRootCount(); ++i)
	{
		if (!TreeAut::subseteq(*lhs.getRoot(i), *rhs.getRoot(i)))
			return false;
	}

	return true;
}


TreeAut& FAE::relabelReferences(
	TreeAut&                      dst,
	const TreeAut&                src,
	const std::vector<size_t>&    index)
{
	dst.addFinalStates(src.getFinalStates());
	for (const Transition& tr : src)
	{
		if (tr.label()->isData())
			continue;

		std::vector<size_t> lhs;
		for (size_t state : tr.lhs())
		{
			const Data* data;
			if (this->isData(state, data))
			{
				if (data->isRef())
				{
					if (index[data->d_ref.root] != static_cast<size_t>(-1))
					{
						lhs.push_back(this->addData(dst, Data::createRef(index[data->d_ref.root], data->d_ref.displ)));
					}
					else
					{
						lhs.push_back(this->addData(dst, Data::createUndef()));
					}
				} else {
					lhs.push_back(this->addData(dst, *data));
				}
			} else
			{
				lhs.push_back(state);
			}
		}

		dst.addTransition(lhs, tr.label(), tr.rhs());
	}

	return dst;
}


TreeAut& FAE::invalidateReference(
	TreeAut&                       dst,
	const TreeAut&                 src,
	size_t                         root)
{
	dst.addFinalStates(src.getFinalStates());
	for (const Transition& tr : src)
	{
		std::vector<size_t> lhs;
		for (size_t state : tr.lhs())
		{
			const Data* data;
			if (FAE::isData(state, data) && data->isRef(root))
			{
				lhs.push_back(this->addData(dst, Data::createUndef()));
			} else {
				lhs.push_back(state);
			}
		}
		if (!FAE::isRef(tr.label(), root))
			dst.addTransition(lhs, tr.label(), tr.rhs());
	}
	return dst;
}


void FAE::buildLTCacheExt(
	const TreeAut&               ta,
	TreeAut::lt_cache_type&      cache)
{
	label_type lUndef = this->boxMan->lookupLabel(Data::createUndef());
	for (TreeAut::iterator i = ta.begin(); i != ta.end(); ++i)
	{
		if (i->label()->isData())
		{
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


void FAE::freePosition(size_t root)
{
	// Preconditions
	assert(root < roots_.size());
	assert(nullptr != roots_[root]);

	size_t pos;
	for (pos = 0; pos < roots_.size(); ++pos)
	{	// try to find a gap in the forest
		if (nullptr == roots_[pos])
		{
			break;
		}
	}

	if (roots_.size() == pos)
	{ // in case no gap was found
		roots_.push_back(std::shared_ptr<TreeAut>());
	}

	// 'pos' is now the target index
	assert(nullptr == roots_[pos]);

	std::vector<size_t> index(roots_.size());
	for (size_t i = 0; i < index.size(); ++i)
	{	// create the index
		index[i] = i;
	}

	index[root] = pos;
	index[pos] = static_cast<size_t>(-1);

	std::ostringstream os;
	utils::printCont(os, index);
	FA_NOTE("Index: " << os.str());

	// relabel references to the TA that was at 'pos'
	for (size_t i = 0; i < roots_.size(); ++i)
	{
		if (nullptr != roots_[i])
		{	// if there is some TA
			roots_[i] = std::shared_ptr<TreeAut>(
				this->relabelReferences(roots_[i].get(), index));
		}
	}

	// swap the TA
	roots_[pos].swap(roots_[root]);

	FA_NOTE("TA " << root << " moved to " << pos << ".");
	FA_NOTE("FAE now: " << *this);
}

