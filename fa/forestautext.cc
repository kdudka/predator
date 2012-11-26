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


void FAE::SubstituteRefs(
	const FAE&           src,
	const Data&          oldValue,
	const Data&          newValue)
{
	// Preconditions
	assert(oldValue.isRef());
	assert(newValue.isRef());
	assert(this->GetVarCount() == src.GetVarCount());

	struct RootState
	{
		size_t root;
		size_t state;

		RootState(size_t pRoot, size_t pState) :
			root(pRoot),
			state(pState)
		{ }

		bool operator<(const RootState& rhs) const
		{
			if (root < rhs.root)
			{
				return true;
			}
			else if (root == rhs.root)
			{
				return state < rhs.state;
			}
			else
			{
				return false;
			}
		}
	};

	typedef std::pair<RootState, RootState> ProdState;

	FA_LOG("old: " << oldValue << ", new: " << newValue);

	std::vector<std::shared_ptr<TreeAut>> newRoots(src.roots_.size(), nullptr);
	for (size_t i = 0; i < this->getRootCount(); ++i)
	{	// allocate existing TA
		if (nullptr != this->getRoot(i))
		{
			newRoots[i] = std::shared_ptr<TreeAut>(this->allocTA());
			newRoots[i]->addFinalStates(this->getRoot(i)->getFinalStates());
		}
	}

	for (size_t i = 0; i < this->GetVarCount(); ++i)
	{
		FA_LOG("Processing gvar " << i);
		const Data& thisVar = this->GetVar(i);
		const Data& srcVar  = src.GetVar(i);

		if (srcVar.isRef() && thisVar.isUndef())
		{	// in case we need to substitute at global variable
			assert(oldValue == srcVar);
			this->SetVar(i, newValue);

			FA_LOG("Changed global variable: " << *this);
		}
		else if (!srcVar.isRef() || !thisVar.isRef())
		{	// in case some of them is not a reference
			assert(srcVar == thisVar);
			continue;
		}

		assert(thisVar.isRef() && srcVar.isRef());
		assert(thisVar.d_ref.displ == srcVar.d_ref.displ);

		const TreeAut* thisRoot = this->getRoot(thisVar.d_ref.root).get();
		const TreeAut* srcRoot  = src.getRoot(srcVar.d_ref.root).get();
		assert((nullptr != thisRoot) && (nullptr != srcRoot));

		ProdState initialState(
			RootState(thisVar.d_ref.root, thisRoot->getFinalState()),
			RootState(srcVar.d_ref.root, srcRoot->getFinalState())
		);

		std::set<ProdState> processed;
		std::vector<ProdState> workset;

		workset.push_back(initialState);
		processed.insert(initialState);

		while (!workset.empty())
		{
			const ProdState curState = *workset.crbegin();
			workset.pop_back();

			const size_t& thisRoot = curState.first.root;
			const size_t& srcRoot = curState.second.root;

			const std::shared_ptr<TreeAut> thisTA = this->getRoot(thisRoot);
			const std::shared_ptr<TreeAut> srcTA = src.getRoot(srcRoot);
			assert((nullptr != thisTA) && (nullptr != srcTA));

			const size_t& thisState = curState.first.state;
			const size_t& srcState = curState.second.state;

			TreeAut::iterator thisIt = thisTA->begin(thisState);
			TreeAut::iterator thisEnd = thisTA->end(thisState, thisIt);
			TreeAut::iterator srcIt = srcTA->begin(srcState);
			TreeAut::iterator srcEnd = srcTA->end(srcState, srcIt);

			for (; thisIt != thisEnd; ++thisIt)
			{
				for (; srcIt != srcEnd; ++srcIt)
				{
					const Transition& thisTrans = *thisIt;
					const Transition& srcTrans = *srcIt;

					if (thisTrans.label() == srcTrans.label())
					{
						assert(thisTrans.lhs().size() == srcTrans.lhs().size());

						if (thisTrans.label()->isData())
						{	// data are processed one level up
							assert(srcTrans.label()->isData());
							continue;
						}

						std::vector<size_t> lhs;

						FA_LOG("matching: " << thisTrans << ", " << srcTrans);
						for (size_t i = 0; i < thisTrans.lhs().size(); ++i)
						{	// for each pair of states that map to each other
							FA_LOG("matching pairs: " << thisTrans.lhs()[i] << ", "
								<< srcTrans.lhs()[i]);

							const Data* srcData;
							if (src.isData(srcTrans.lhs()[i], srcData))
							{	// for data states
								assert(nullptr != srcData);

								const Data* thisData;
								if (!this->isData(thisTrans.lhs()[i], thisData))
								{
									assert(false);       // fail gracefully
								}

								assert(nullptr != thisData);

								FA_LOG("DATA: " << *thisData << ", " << *srcData);

								if (oldValue == *srcData)
								{	// in case we are at the right value
									FA_LOG("Found old value!");
									assert(thisData->isUndef());
									// TODO: or may it reference itself?

									assert(false);
									// TODO: we should now change the undef to newValue

									// add sth to 'lhs'
								}
								else if (srcData->isRef())
								{	// for the case of other reference
									assert(thisData->isRef());
									assert(thisData->d_ref.displ == srcData->d_ref.displ);

									const size_t& thisNewRoot = thisData->d_ref.root;
									const size_t& srcNewRoot  = srcData->d_ref.root;

									const std::shared_ptr<TreeAut> thisNewTA = this->getRoot(thisData->d_ref.root);
									const std::shared_ptr<TreeAut> srcNewTA  = src.getRoot(srcData->d_ref.root);
									assert((nullptr != thisNewTA) && (nullptr != srcNewTA));

									ProdState jumpState(
										RootState(thisNewRoot, thisNewTA->getFinalState()),
										RootState(srcNewRoot, srcNewTA->getFinalState())
									);

									if (processed.insert(jumpState).second)
									{	// in case the state has not been processed before
										FA_LOG("new jump state!");
										workset.push_back(jumpState);
									}

									lhs.push_back(this->addData(*newRoots[thisRoot].get(), *thisData));
								}
								else
								{	// for other data
									lhs.push_back(this->addData(*newRoots[thisRoot].get(), *thisData));
								}
							}
							else
							{
								const ProdState newState(std::make_pair(
									RootState(thisRoot, thisTrans.lhs()[i]),
									RootState(srcRoot, srcTrans.lhs()[i])
								));

								if (processed.insert(newState).second)
								{	// in case the state has not been processed before
									FA_LOG("new state!");
									workset.push_back(newState);
								}
							}
						}

						newRoots[thisRoot]->addTransition(lhs, thisTrans.label(), thisTrans.rhs());
					}
					else
					{
						FA_LOG("not-matching: " << *thisIt << ", " << *srcIt);
					}
				}
			}
		}
	}

	roots_.swap(newRoots);
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
