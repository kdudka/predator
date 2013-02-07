/*
 * Copyright (C) 2013 Jiri Simacek
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
#include "folding.hh"

bool Folding::isSignaturesCompatible(
	const ConnectionGraph::CutpointSignature&    s1,
	const ConnectionGraph::CutpointSignature&    s2)
{
	if (s1.size() != s2.size())
		return false;

	for (size_t i = 0; i < s1.size(); ++i)
	{
		if (s1[i].root != s2[i].root)
			return false;
/*
		if (*s1[i].fwdSelectors.begin() != *s2[i].fwdSelectors.begin())
			return false;
*/
		if (s1[i].bwdSelector != s2[i].bwdSelector)
			return false;

		if (s1[i].defines != s2[i].defines)
			return false;
	}

	return true;
}


std::pair<std::shared_ptr<TreeAut>, std::shared_ptr<TreeAut>> Folding::separateCutpoint(
	ConnectionGraph::CutpointSignature&            boxSignature,
	size_t                                         root,
	size_t                                         state,
	size_t                                         cutpoint)
{
	auto ta = std::shared_ptr<TreeAut>(fae_.allocTA());
	auto tmp = std::shared_ptr<TreeAut>(fae_.allocTA());

	this->componentCut(*ta, *tmp, boxSignature, root, state, cutpoint);

	auto tmp2 = std::shared_ptr<TreeAut>(fae_.allocTA());

	tmp->unreachableFree(*tmp2);

	return std::make_pair(ta, tmp2);
}


const ConnectionGraph::StateToCutpointSignatureMap& Folding::getSignatures(
	size_t      root)
{
	// Preconditions
	assert(root < signatureMap_.size());

	if (!signatureMap_[root].first)
	{
		ConnectionGraph::computeSignatures(
			signatureMap_[root].second, *fae_.getRoot(root)
		);

		signatureMap_[root].first = true;
	}

	assert(signatureMap_[root].first);

	return signatureMap_[root].second;
}


bool Folding::discover1(
	size_t                       root,
	const std::set<size_t>&      forbidden,
	bool                         conditional)
{
	// Preconditions
	assert(fae_.getRootCount() == fae_.connectionGraph.data.size());
	assert(root < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(root));

	if (forbidden.count(root))
	{
		return nullptr;
	}

	bool found = false;
dis1_start:
	// save state offset
	fae_.pushStateOffset();

	fae_.updateConnectionGraph();

	for (auto& cutpoint : fae_.connectionGraph.data[root].signature)
	{
		if (cutpoint.root != root)
		{
			continue;
		}

		FA_DEBUG_AT(3, "type 1 cutpoint detected at root " << root);

		auto boxPtr = this->makeType1Box(
			root, fae_.getRoot(root)->getFinalState(), root, forbidden, conditional
		);

		if (boxPtr)
		{
			found = true;

			goto dis1_start;
		}

		fae_.popStateOffset();
	}

	return found;
}

bool Folding::discover2(
	size_t                       root,
	const std::set<size_t>&      forbidden,
	bool                         conditional)
{
	// Preconditions
	assert(fae_.getRootCount() == fae_.connectionGraph.data.size());
	assert(root < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(root));

	if (forbidden.count(root))
	{
		return nullptr;
	}

	bool found = false;
dis2_start:
	// save state offset
	fae_.pushStateOffset();

	fae_.updateConnectionGraph();

	for (auto& cutpoint : fae_.connectionGraph.data[root].signature)
	{
		if (cutpoint.refCount < 2)
			continue;

		auto& signatures = this->getSignatures(root);

		for (auto& stateSignaturePair : signatures)
		{
			for (auto& tmp : stateSignaturePair.second)
			{
				if ((tmp.refCount < 2) || tmp.refInherited || (tmp.root != cutpoint.root))
				{
					continue;
				}

				FA_DEBUG_AT(3, "type 2 cutpoint detected inside component " << root
					<< " at state q" << stateSignaturePair.first);

				auto boxPtr = this->makeType1Box(
					root, stateSignaturePair.first, cutpoint.root, forbidden, conditional
				);

				if (boxPtr)
				{
					found = true;

					goto dis2_start;
				}

				fae_.popStateOffset();
			}
		}
	}

	return found;
}

bool Folding::discover3(
	size_t                      root,
	const std::set<size_t>&     forbidden,
	bool                        conditional)
{
	// Preconditions
	assert(fae_.getRootCount() == fae_.connectionGraph.data.size());
	assert(root < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(root));

	if (forbidden.count(root))
	{
		return nullptr;
	}

	bool found = false;
dis3_start:
	// save state offset
	fae_.pushStateOffset();

	fae_.updateConnectionGraph();

	for (auto& cutpoint : fae_.connectionGraph.data[root].signature)
	{
		if (forbidden.count(cutpoint.root)/* || cutpoint.joint*/)
		{
			continue;
		}

		size_t selectorToRoot = ConnectionGraph::getSelectorToTarget(
			fae_.connectionGraph.data[cutpoint.root].signature, root
		);

		if (selectorToRoot == static_cast<size_t>(-1))
		{
			continue;
		}
/*
		if (selectorToRoot == cutpoint.forwardSelector)
			continue;
*/
		assert(!cutpoint.fwdSelectors.empty());

		if (/*(selectorToRoot < *cutpoint.fwdSelectors.begin()) ||*/
			this->makeType2Box(cutpoint.root, root, forbidden, true, true))
		{
			continue;
		}

		FA_DEBUG_AT(3, "type 3 cutpoint detected at roots " << root << " and "
			<< cutpoint.root);

		auto boxPtr = this->makeType2Box(root, cutpoint.root, forbidden, conditional);

		if (boxPtr)
		{
			found = true;

			goto dis3_start;
		}

		fae_.popStateOffset();
	}

	return found;
}

void Folding::componentCut(
	TreeAut&                                 res,
	TreeAut&                                 complement,
	ConnectionGraph::CutpointSignature&      complementSignature,
	size_t                                   root,
	size_t                                   state,
	size_t                                   target)
{
	// Preconditions
	assert(root < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(root));

	const TreeAut& src = *fae_.getRoot(root);

	res.addFinalStates(src.getFinalStates());

	complement.addFinalState(state);

	auto& signatures = this->getSignatures(root);

	std::unordered_set<const AbstractBox*> boxes;

	// enumerate which boxes to fold
	for (auto i = src.begin(state); i != src.end(state, i); ++i)
	{
		size_t lhsOffset = 0;

		const Transition& trans = *i;

		for (auto& box : trans.label()->getNode())
		{
			// look for target cutpoint
			for (size_t j = 0; j < box->getArity(); ++j)
			{
				assert(lhsOffset + j < trans.lhs().size());

				if (ConnectionGraph::containsCutpoint(
					Folding::getSignature(trans.lhs()[lhsOffset + j], signatures), target))
				{
					boxes.insert(box);
					break;
				}
			}

			lhsOffset += box->getArity();
		}
	}

	ConnectionGraph::CutpointSignature tmp;

	for (const Transition& trans : src)
	{
		if (trans.rhs() != state)
		{
			res.addTransition(trans);
			complement.addTransition(trans);

			continue;
		}

		std::vector<size_t> lhs, cLhs;
		std::vector<const AbstractBox*> label, cLabel;

		size_t lhsOffset = 0;

		tmp.clear();

		// split transition
		for (auto& box : trans.label()->getNode())
		{
			if (boxes.count(box) == 0)
			{
				Folding::copyBox(lhs, label, box, trans.lhs(), lhsOffset);
			} else
			{
				for (size_t j = 0; j < box->getArity(); ++j)
				{
					assert(lhsOffset + j < trans.lhs().size());

					ConnectionGraph::processStateSignature(
						tmp,
						static_cast<const StructuralBox*>(box),
						j,
						trans.lhs()[lhsOffset + j],
						Folding::getSignature(trans.lhs()[lhsOffset + j], signatures)
					);
				}

				Folding::copyBox(cLhs, cLabel, box, trans.lhs(), lhsOffset);
			}

			lhsOffset += box->getArity();
		}

		ConnectionGraph::normalizeSignature(tmp);

		assert(tmp.size());

		// did we arrive here for the first time?
		if (complementSignature.empty())
			complementSignature = tmp;

		// a bit hacky but who cares
		assert(Folding::isSignaturesCompatible(complementSignature, tmp));

		for (size_t i = 0; i < tmp.size(); ++i)
		{
			complementSignature[i].refCount = std::max(complementSignature[i].refCount, tmp[i].refCount);

			complementSignature[i].fwdSelectors.insert(
				tmp[i].fwdSelectors.begin(), tmp[i].fwdSelectors.end()
			);
		}

		assert(label.size());
		FAE::reorderBoxes(label, lhs);
		res.addTransition(lhs, fae_.boxMan->lookupLabel(label), state);

		assert(cLabel.size());
		FAE::reorderBoxes(cLabel, cLhs);
		complement.addTransition(cLhs, fae_.boxMan->lookupLabel(cLabel), state);
	}
}


const Box* Folding::makeType1Box(
	size_t                        root,
	size_t                        state,
	size_t                        aux,
	const std::set<size_t>&       forbidden,
	bool                          conditional,
	bool                          test)
{
	// Preconditions
	assert(root < fae_.getRootCount());
	assert(fae_.getRoot(root));

	std::vector<size_t> index(fae_.getRootCount(), static_cast<size_t>(-1)), inputMap;
	std::unordered_map<size_t, size_t> selectorMap;
	ConnectionGraph::CutpointSignature outputSignature;

	size_t start = 0;

	auto p = this->separateCutpoint(outputSignature, root, state, aux);

	index[root] = start++;

	for (auto& cutpoint : outputSignature)
	{
		if (forbidden.count(cutpoint.root))
			return nullptr;

		assert(cutpoint.root < index.size());

		if (cutpoint.root != root)
			index[cutpoint.root] = start++;
	}

	if (!Folding::computeSelectorMap(selectorMap, root, state))
	{
		return nullptr;
	}

	Folding::extractInputMap(inputMap, selectorMap, root, index);

	auto box = std::unique_ptr<Box>(
		boxMan_.createType1Box(
			root,
			this->relabelReferences(*p.second, index),
			outputSignature,
			inputMap,
			index
		)
	);

	auto boxPtr = this->getBox(*box, conditional);

	if (test)
	{
		return boxPtr;
	}

	if (!boxPtr)
	{
		return nullptr;
	}

	FA_DEBUG_AT(2, *static_cast<const AbstractBox*>(boxPtr) << " found");

	fae_.setRoot(root, this->joinBox(*p.first, state, root, boxPtr, outputSignature));
	fae_.connectionGraph.invalidate(root);

	this->invalidateSignatures(root);

	return boxPtr;
}


const Box* Folding::makeType2Box(
	size_t                      root,
	size_t                      aux,
	const std::set<size_t>&     forbidden,
	bool                        conditional,
	bool                        test)
{
	// Preconditions
	assert(root < fae_.getRootCount());
	assert(aux < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(root));
	assert(nullptr != fae_.getRoot(aux));

	size_t finalState = fae_.getRoot(root)->getFinalState();

	std::vector<size_t> index(fae_.getRootCount(), static_cast<size_t>(-1)), index2, inputMap;
	std::vector<bool> rootMask(fae_.getRootCount(), false);
	std::unordered_map<size_t, size_t> selectorMap;
	ConnectionGraph::CutpointSignature outputSignature, inputSignature, tmpSignature;

	size_t start = 0;

	auto p = this->separateCutpoint(outputSignature, root, finalState, aux);

	index[root] = start++;

	for (auto& cutpoint : outputSignature)
	{
/*
		// we assume type 1 box is not present
		assert(cutpoint.root != root);
*/
		if (cutpoint.root == root)
		{
			return nullptr;
		}

		if (forbidden.count(cutpoint.root))
		{
			return nullptr;
		}

		assert(cutpoint.root < index.size());

		if (cutpoint.root != root)
		{
			index[cutpoint.root] = start++;
		}
	}

	if (!Folding::computeSelectorMap(selectorMap, root, finalState))
	{
		return nullptr;
	}

	Folding::extractInputMap(inputMap, selectorMap, root, index);

	auto auxP = this->separateCutpoint(
		inputSignature, aux, fae_.getRoot(aux)->getFinalState(), root
	);
/*
	if (Folding::isSingular(*auxP.first))
		return false;
*/
	index2 = index;

	for (auto& cutpoint : inputSignature)
	{
		if (cutpoint.refCount > 1)
		{
			return nullptr;
		}

		if (forbidden.count(cutpoint.root))
		{
			return nullptr;
		}

		assert(cutpoint.root < index.size());

		if (index[cutpoint.root] == static_cast<size_t>(-1))
		{
			assert(index2[cutpoint.root] == static_cast<size_t>(-1));

			index2[cutpoint.root] = start++;

			tmpSignature.push_back(cutpoint);

			inputMap.push_back(static_cast<size_t>(-1));
		}
	}

	selectorMap.clear();

	if (!Folding::computeSelectorMap(selectorMap, aux,
		fae_.getRoot(aux)->getFinalState()))
	{
		assert(false);           // fail gracefully
	}

	size_t selector = Folding::extractSelector(selectorMap, root);

	auto box = std::unique_ptr<Box>(
		boxMan_.createType2Box(
			root,
			this->relabelReferences(*p.second, index),
			outputSignature,
			inputMap,
			aux,
			this->relabelReferences(*auxP.second, index2),
			inputSignature,
			selector,
			index
		)
	);

	auto boxPtr = this->getBox(*box, conditional);

	if (test)
	{
		return boxPtr;
	}

	if (!boxPtr)
	{
		return nullptr;
	}

	FA_DEBUG_AT(2, *static_cast<const AbstractBox*>(boxPtr) << " found");

	for (auto& cutpoint : tmpSignature)
	{
		outputSignature.push_back(cutpoint);
	}

	fae_.setRoot(root, this->joinBox(*p.first, finalState, root, boxPtr, outputSignature));
	fae_.connectionGraph.invalidate(root);

	this->invalidateSignatures(root);

	fae_.setRoot(aux, auxP.first);
	fae_.connectionGraph.invalidate(aux);

	this->invalidateSignatures(aux);

	return boxPtr;
}


std::shared_ptr<TreeAut> Folding::joinBox(
	const TreeAut&                               src,
	size_t                                       state,
	size_t                                       root,
	const Box*                                   box,
	const ConnectionGraph::CutpointSignature&    signature)
{
	auto ta = std::shared_ptr<TreeAut>(fae_.allocTA());

	ta->addFinalStates(src.getFinalStates());

	for (const Transition& trans : src)
	{
		if (trans.rhs() != state)
		{
			ta->addTransition(trans);

			continue;
		}

		std::vector<const AbstractBox*> label(trans.label()->getNode());
		std::vector<size_t> lhs(trans.lhs());

		label.push_back(box);

		for (auto& cutpoint : signature)
		{
			if ((cutpoint.root == root) && (src.getFinalStates().count(state)))
			{
				continue;
			}

			lhs.push_back(
				fae_.addData(*ta, Data::createRef(cutpoint.root))
			);
		}

		FA::reorderBoxes(label, lhs);

		ta->addTransition(lhs, fae_.boxMan->lookupLabel(label), state);
	}

	return ta;
}


void Folding::computeSelectorMap(
	std::unordered_map<size_t, size_t>&                    selectorMap,
	const Transition&                                      t,
	const ConnectionGraph::StateToCutpointSignatureMap&    stateMap)
{
	size_t lhsOffset = 0;

	for (auto& absBox : t.label()->getNode())
	{
		switch (absBox->getType())
		{
			case box_type_e::bSel:
			{
				auto iter = stateMap.find(t.lhs()[lhsOffset]);

				assert(iter != stateMap.end());

				Folding::updateSelectorMap(
					selectorMap,
					(static_cast<const SelBox*>(absBox))->getData().offset,
					iter->second
				);

				break;
			}

			case box_type_e::bBox:
			{
				const Box* box = static_cast<const Box*>(absBox);

				for (size_t i = 0; i < box->getArity(); ++i)
				{
					auto iter = stateMap.find(t.lhs()[lhsOffset + i]);

					assert(iter != stateMap.end());

					Folding::updateSelectorMap(
						selectorMap, box->getSelector(i), iter->second
					);
				}

				break;
			}

			default: break;
		}

		lhsOffset += absBox->getArity();
	}
}


bool Folding::checkSelectorMap(
	const std::unordered_map<size_t, size_t>&     selectorMap,
	size_t                                        root,
	size_t                                        state)
{
	// Preconditions
	assert(root < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(root));

	auto& signatures = this->getSignatures(root);

	auto& ta = *fae_.getRoot(root);

	for (TreeAut::iterator i = ta.begin(state); i != ta.end(state, i); ++i)
	{
		std::unordered_map<size_t, size_t> m;

		Folding::computeSelectorMap(m, *i, signatures);

		if (m != selectorMap)
		{
			return false;
		}
	}

	return true;
}


bool Folding::computeSelectorMap(
	std::unordered_map<size_t, size_t>&      selectorMap,
	size_t                                   root,
	size_t                                   state)
{
	// Preconditions
	assert(root < fae_.getRootCount());
	assert(nullptr != fae_.getRoot(root));

	auto& ta = *fae_.getRoot(root);

	assert(ta.begin(state) != ta.end(state));
/*
	for (TreeAut::iterator i = ta.accBegin(); i != ta.accEnd(i); ++i)
		Folding::computeSelectorMap(selectorMap, *i, stateMap);
*/
	auto& signatures = this->getSignatures(root);

	Folding::computeSelectorMap(selectorMap, *ta.begin(state), signatures);

	return this->checkSelectorMap(selectorMap, root, state);
}


void Folding::extractInputMap(
	std::vector<size_t>&                         inputMap,
	const std::unordered_map<size_t, size_t>&    selectorMap,
	size_t                                       root,
	const std::vector<size_t>&                   index)
{
	// Preconditions
	assert(index[root] == 0);

	inputMap.resize(selectorMap.size());

	size_t count = 0;

	for (auto& cutpointSelectorPair : selectorMap)
	{
		if (cutpointSelectorPair.first == root)
		{
			// reference to root does not appear in the box interface
			continue;
		}

		assert(cutpointSelectorPair.first < index.size());

		if (index[cutpointSelectorPair.first] == static_cast<size_t>(-1))
			continue;

		assert(index[cutpointSelectorPair.first] >= 1);
		assert(index[cutpointSelectorPair.first] < inputMap.size() + 1);

		inputMap[index[cutpointSelectorPair.first] - 1] = cutpointSelectorPair.second;

		++count;
	}

	inputMap.resize(count);
}
