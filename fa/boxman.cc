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
#include "boxman.hh"
#include "streams.hh"
#include "restart_request.hh"

namespace
{	// anonymous namespace

/**
 * @brief  Translates the cutpoint signature according to an index
 *
 * Given the mapping of original cutpoints to the ones in the box @p index, this
 * function translates the cutpoint signature @p signature. 
 *
 * @param[out] result     The translated cutpoint signature
 * @param[out] selectors  Vector of pairs of first selectors reaching the
 *                        cutpoints and lowest selectors reaching through the
 *                        opposite direction
 * @param[in]  root       The index of the input automaton
 * @param[in]  signature  The input signature of cutpoints
 * @param[in]  aux        Index of the second automaton
 * @param[in]  index      The mapping of each tree automaton index to the
 *                        index of a component in the box
 *
 * @returns  Offset of the first selector reaching the second component
 */
size_t translateSignature(
	ConnectionGraph::CutpointSignature&         result,
	std::vector<std::pair<size_t, size_t>>&     selectors,
	size_t                                      root,
	const ConnectionGraph::CutpointSignature&   signature,
	size_t                                      aux,
	const std::vector<size_t>&                  index)
{
	// Preconditions
	assert(result.empty());
	assert(selectors.empty());

	size_t auxSelector = static_cast<size_t>(-1);

	for (const ConnectionGraph::CutpointInfo& cutpoint : signature)
	{	// process the sequence of cutpoints in the signature in the order

		// make sure the cutpoint is OK
		assert(cutpoint.root < index.size());
		assert(!cutpoint.fwdSelectors.empty());

		// append the cutpoint and change its root according to the translation in
		// 'index'
		result.push_back(cutpoint);
		result.back().root = index[cutpoint.root];

		if (cutpoint.root == aux)
		{	// in case the current cutpoint is of the second component
			assert(static_cast<size_t>(-1) == auxSelector);

			// set the first selector that reaches the second component
			auxSelector = *cutpoint.fwdSelectors.begin();
		}

		if (cutpoint.root != root)
		{	// in case the cutpoint is other than the input component

			// append into 'selectors' the pair of the first selector reaching the
			// cutpoint and the lowest selector of root from which it can be reached
			// throught the opposite direction
			selectors.push_back(
				std::make_pair(*cutpoint.fwdSelectors.begin(), cutpoint.bwdSelector)
			);
		}
	}

	return auxSelector;
}

} // namespace


const Box* BoxAntichain::get(const Box& box)
{
	modified_ = false;

	auto p = boxes_.insert(std::make_pair(box.getSignature(), std::list<Box>()));

	if (!p.second)
	{
		for (auto iter = p.first->second.begin(); iter != p.first->second.end(); )
		{
			// Assertions
			assert(!modified_ || !box.simplifiedLessThan(*iter));

			if (!modified_ && box.simplifiedLessThan(*iter))
				return &*iter;

			if (iter->simplifiedLessThan(box))
			{
				auto tmp = iter++;

				obsolete_.splice(obsolete_.end(), p.first->second, tmp);

				modified_ = true;
			}
			else
			{
				++iter;
			}
		}
	}

	p.first->second.push_back(box);

	modified_ = true;

	++size_;

	return &p.first->second.back();
}


const Box* BoxAntichain::lookup(const Box& box) const
{
	// find the box according to the signature
	auto iter = boxes_.find(box.getSignature());

	if (iter != boxes_.end())
	{
		for (auto& box2 : iter->second)
		{
			if (box.simplifiedLessThan(box2))
			{
				return &box2;
			}
		}
	}

	return nullptr;
}


void BoxAntichain::asVector(std::vector<const Box*>& boxes) const
{
	for (auto& signatureListPair : boxes_)
	{
		for (auto& box : signatureListPair.second)
			boxes.push_back(&box);
	}
}


const std::pair<const Data, NodeLabel*>& BoxMan::insertData(const Data& data)
{
	std::pair<TDataStore::iterator, bool> p = dataStore_.insert(
		std::make_pair(data, static_cast<NodeLabel*>(nullptr)));

	if (p.second)
	{
		p.first->second = new NodeLabel(&p.first->first, dataIndex_.size());
		dataIndex_.push_back(&p.first->first);
	}
	return *p.first;
}


std::string BoxMan::getBoxName() const
{
	// Assertions
	assert(boxes_.size());

	std::stringstream sstr;

	sstr << "box" << boxes_.size() - 1;

	return sstr.str();
}


label_type BoxMan::lookupLabel(
	size_t                        arity,
	const DataArray&              x)
{
	std::pair<TVarDataStore::iterator, bool> p = vDataStore_.insert(
		std::make_pair(std::make_pair(arity, x), static_cast<NodeLabel*>(nullptr)));
	if (p.second)
		p.first->second = new NodeLabel(&p.first->first.second);

	return p.first->second;
}


bool BoxMan::EvaluateBoxF::operator()(
	const AbstractBox*       aBox,
	size_t                   index,
	size_t                   offset)
{
	switch (aBox->getType())
	{
		case box_type_e::bSel:
		{
			const SelBox* sBox = static_cast<const SelBox*>(aBox);
			this->label.addMapItem(sBox->getData().offset, aBox, index, offset);
			this->tag.push_back(sBox->getData().offset);
			break;
		}
		case box_type_e::bBox:
		{
			const Box* bBox = static_cast<const Box*>(aBox);
			for (auto it= bBox->outputCoverage().cbegin();
				it != bBox->outputCoverage().cend(); ++it)
			{
				this->label.addMapItem(*it, aBox, index, offset);
				this->tag.push_back(*it);
			}
			break;
		}
		case box_type_e::bTypeInfo:
			this->label.addMapItem(static_cast<size_t>(-1), aBox, index,
				static_cast<size_t>(-1));
			break;
		default:
			assert(false);      // fail gracefully
			break;
	}

	return true;
}


label_type BoxMan::lookupLabel(
	const std::vector<const AbstractBox*>&     x,
	const std::vector<SelData>*                nodeInfo)
{
	std::pair<TNodeStore::iterator, bool> p = nodeStore_.insert(
		std::make_pair(x, static_cast<NodeLabel*>(nullptr)));

	if (p.second)
	{
		NodeLabel* label = new NodeLabel(&p.first->first, nodeInfo);

		std::vector<size_t> tag;

		label->iterate(EvaluateBoxF(*label, tag));

		std::sort(tag.begin(), tag.end());

		label->setTag(
			const_cast<void*>(
				reinterpret_cast<const void*>(
					&*tagStore_.insert(
						std::make_pair(
							static_cast<const TypeBox*>(label->boxLookup(static_cast<size_t>(-1),
							nullptr)), tag)
					).first
				)
			)
		);

		p.first->second = label;
	}

	return p.first->second;
}


const SelBox* BoxMan::getSelector(const SelData& sel)
{
	std::pair<const SelData, const SelBox*>& p = *selIndex_.insert(
		std::make_pair(sel, static_cast<const SelBox*>(nullptr))
	).first;

	if (!p.second)
	{
		p.second = new SelBox(&p.first);
	}

	return p.second;
}


const TypeBox* BoxMan::getTypeInfo(const std::string& name)
{
	TTypeIndex::const_iterator i = typeIndex_.find(name);
	if (i == typeIndex_.end())
		throw std::runtime_error("BoxMan::getTypeInfo(): type for "
			+ name + " not found!");
	return i->second;
}


const TypeBox* BoxMan::createTypeInfo(
	const std::string&            name,
	const std::vector<size_t>&    selectors)
{
	std::pair<const std::string, const TypeBox*>& p = *typeIndex_.insert(
		std::make_pair(name, static_cast<const TypeBox*>(nullptr))).first;
	if (p.second && (selectors != p.second->getSelectors()))
	{
		// FIXME: this should be done in a better way, more C99-like
		throw std::runtime_error("BoxMan::createTypeInfo(): type \'" + name
			+ "\' already exists with incompatible declaration!");
	}

	p.second = new TypeBox(name, selectors);
	return p.second;
}


Box* BoxMan::createType1Box(
	size_t                                      root,
	const std::shared_ptr<TreeAut>&             output,
	const ConnectionGraph::CutpointSignature&   signature,
	const std::vector<size_t>&                  inputMap,
	const std::vector<size_t>&                  index)
{
	ConnectionGraph::CutpointSignature outputSignature;
	std::vector<std::pair<size_t, size_t>> selectors;

	// translate the cutpoint signature according to 'index'
	translateSignature(
		outputSignature, selectors, root, signature, static_cast<size_t>(-1), index
	);

	return new Box(
		/* the name of the box */ "",
		/* the output TA */ output,
		/* signature of the output component */ outputSignature,
		/* mapping of cutpoints to selectors */ inputMap,
		/* the input TA */ std::shared_ptr<TreeAut>(nullptr),
		/* index of the input TA */ 0,
		/* cutpoint signature of the input TA*/ ConnectionGraph::CutpointSignature(),
		/* The vector of pairs of fwd and bwd selectors */ selectors
	);
}


Box* BoxMan::createType2Box(
	size_t                                        root,
	const std::shared_ptr<TreeAut>&               output,
	const ConnectionGraph::CutpointSignature&     signature,
	const std::vector<size_t>&                    inputMap,
	size_t                                        aux,
	const std::shared_ptr<TreeAut>&               input,
	const ConnectionGraph::CutpointSignature&     signature2,
	size_t                                        inputSelector,
	std::vector<size_t>&                          index)
{
	// Assertions
	assert(aux < index.size());
	assert(index[aux] >= 1);

	ConnectionGraph::CutpointSignature outputSignature, inputSignature;
	std::vector<std::pair<size_t, size_t>> selectors;

	size_t auxSelector = translateSignature(
		outputSignature, selectors, root, signature, aux, index
	);

	size_t start = selectors.size();

	for (auto& cutpoint : signature2)
	{
		// Assertions
		assert(cutpoint.root < index.size());

		if (index[cutpoint.root] == static_cast<size_t>(-1))
		{
			index[cutpoint.root] = start++;

			selectors.push_back(std::make_pair(auxSelector, static_cast<size_t>(-1)));
		}

		inputSignature.push_back(cutpoint);
		inputSignature.back().root = index[cutpoint.root];
	}

	size_t inputIndex = index[aux] - 1;

	assert(inputIndex < selectors.size());

	if (selectors[inputIndex].second > inputSelector)
		selectors[inputIndex].second = inputSelector;

	return new Box(
		"",
		output,
		outputSignature,
		inputMap,
		input,
		inputIndex,
		inputSignature,
		selectors
	);
}


const Box* BoxMan::getBox(const Box& box)
{
	// insert the box into the manager
	const Box* cpBox = boxes_.get(box);
	assert(nullptr != cpBox);

	if (boxes_.modified())
	{	// in the case a new box was inserted
		Box* pBox = const_cast<Box*>(cpBox);

		// perform initialization
		pBox->name_ = this->getBoxName();
		pBox->initialize();

		FA_DEBUG_AT(1, "learning " << *static_cast<const AbstractBox*>(cpBox)
			<< ':' << std::endl << *cpBox);

#if FA_RESTART_AFTER_BOX_DISCOVERY
		throw RestartRequest("a new box encountered");
#endif
	}

	return cpBox;
}


void BoxMan::clear()
{
	utils::eraseMap(dataStore_);
	dataIndex_.clear();
	utils::eraseMap(nodeStore_);
	tagStore_.clear();
	utils::eraseMap(vDataStore_);
	utils::eraseMap(selIndex_);
	utils::eraseMap(typeIndex_);
	boxes_.clear();
}
