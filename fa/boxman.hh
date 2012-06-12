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

#ifndef BOX_MANAGER_H
#define BOX_MANAGER_H

// Standard library headers
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>

// Forester headers
#include "config.h"
#include "treeaut_label.hh"
#include "tatimint.hh"
#include "label.hh"
#include "types.hh"
#include "box.hh"
#include "utils.hh"
#include "restart_request.hh"

class BoxAntichain
{
	using TBoxStore = std::unordered_map<Box::Signature, std::list<Box>,
		boost::hash<Box::Signature>>;

	TBoxStore boxes_;

	std::list<Box> obsolete_;

	bool modified_;

	size_t size_;

public:

	BoxAntichain() :
		boxes_(),
		obsolete_(),
		modified_(false),
		size_(0)
	{ }

	const Box* get(const Box& box)
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
				} else
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

	const Box* lookup(const Box& box) const
	{
		auto iter = boxes_.find(box.getSignature());

		if (iter != boxes_.end())
		{
			for (auto& box2 : iter->second)
			{
				if (box.simplifiedLessThan(box2))
					return &box2;
			}
		}

		return nullptr;
	}

	bool modified() const
	{
		return modified_;
	}

	size_t size() const
	{
		return size_;
	}

	void clear()
	{
		boxes_.clear();
	}

	void asVector(std::vector<const Box*>& boxes) const
	{
		for (auto& signatureListPair : boxes_)
		{
			for (auto& box : signatureListPair.second)
				boxes.push_back(&box);
		}
	}
};


class BoxSet
{
	using TBoxSet = std::unordered_set<Box, boost::hash<Box>>;

	TBoxSet boxes_;

	bool modified_;

public:

	BoxSet() :
		boxes_(),
		modified_(false)
	{ }

	const Box* get(const Box& box)
	{
		auto p = boxes_.insert(box);

		modified_ = p.second;

		return &*p.first;
	}

	const Box* lookup(const Box& box) const
	{
		auto iter = boxes_.find(box);

		return (iter == boxes_.end())?(nullptr):(&*iter);
	}

	bool modified() const
	{
		return modified_;
	}

	void clear()
	{
		boxes_.clear();
	}

	size_t size() const
	{
		return boxes_.size();
	}

	void asVector(std::vector<const Box*>& boxes) const
	{
		for (auto& box : boxes_)
			boxes.push_back(&box);
	}
};

#if FA_BOX_APPROXIMATION
	typedef BoxAntichain BoxDatabase;
#else
	typedef BoxSet BoxDatabase;
#endif

class BoxMan
{
private:  // data types

	using TDataStore = std::unordered_map<Data, NodeLabel*, boost::hash<Data>>;
	using TNodeStore = std::unordered_map<std::vector<const AbstractBox*>,
		NodeLabel*, boost::hash<std::vector<const AbstractBox*>>>;
	using TVarDataStore = std::unordered_map<std::pair<size_t, DataArray>,
		NodeLabel*, boost::hash<std::pair<size_t, DataArray>>>;
	using TTagStore = std::unordered_set<
		std::pair<const TypeBox*, std::vector<size_t>>,
		boost::hash<std::pair<const TypeBox*, std::vector<size_t>>>>;
	using TSelIndex = std::unordered_map<SelData, const SelBox*,
		boost::hash<SelData>>;
	using TTypeIndex = std::unordered_map<std::string, const TypeBox*>;

private:  // data members

	TDataStore dataStore_;
	std::vector<const Data*> dataIndex_;
	TNodeStore nodeStore_;
	TTagStore tagStore_;
	TVarDataStore vDataStore_;

	TSelIndex selIndex_;
	TTypeIndex typeIndex_;

	BoxDatabase boxes_;

	const std::pair<const Data, NodeLabel*>& insertData(const Data& data)
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

	std::string getBoxName() const
	{
		// Assertions
		assert(boxes_.size());

		std::stringstream sstr;

		sstr << "box" << boxes_.size() - 1;

		return sstr.str();
	}

public:

	label_type lookupLabel(const Data& data)
	{
		return this->insertData(data).second;
	}

	label_type lookupLabel(size_t arity, const DataArray& x)
	{
		std::pair<TVarDataStore::iterator, bool> p = vDataStore_.insert(
			std::make_pair(std::make_pair(arity, x), static_cast<NodeLabel*>(nullptr)));
		if (p.second)
			p.first->second = new NodeLabel(&p.first->first.second);

		return p.first->second;
	}

	struct EvaluateBoxF
	{
		NodeLabel& label;
		std::vector<size_t>& tag;

		EvaluateBoxF(NodeLabel& label, std::vector<size_t>& tag) : label(label), tag(tag) {}

		bool operator()(const AbstractBox* aBox, size_t index, size_t offset)
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
	};

	label_type lookupLabel(const std::vector<const AbstractBox*>& x)
	{
		std::pair<TNodeStore::iterator, bool> p
			= nodeStore_.insert(std::make_pair(x, static_cast<NodeLabel*>(nullptr)));

		if (p.second)
		{
			NodeLabel* label = new NodeLabel(&p.first->first);

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

	const Data& getData(const Data& data)
	{
		return this->insertData(data).first;
	}

	size_t getDataId(const Data& data)
	{
		return this->insertData(data).second->getDataId();
	}

	const Data& getData(size_t index) const
	{
		// Assertions
		assert(index < dataIndex_.size());

		return *dataIndex_[index];
	}

	const SelBox* getSelector(const SelData& sel)
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

	const TypeBox* getTypeInfo(const std::string& name)
	{
		TTypeIndex::const_iterator i = typeIndex_.find(name);
		if (i == typeIndex_.end())
			throw std::runtime_error("BoxMan::getTypeInfo(): type for "
				+ name + " not found!");
		return i->second;
	}

	const TypeBox* createTypeInfo(const std::string& name,
		const std::vector<size_t>& selectors)
	{
		std::pair<const std::string, const TypeBox*>& p = *typeIndex_.insert(
			std::make_pair(name, static_cast<const TypeBox*>(nullptr))
		).first;
		if (p.second)
			throw std::runtime_error("BoxMan::createTypeInfo(): type already exists!");
		p.second = new TypeBox(name, selectors);
		return p.second;
	}

	static size_t translateSignature(
		ConnectionGraph::CutpointSignature& result,
		std::vector<std::pair<size_t, size_t>>& selectors,
		size_t root,
		const ConnectionGraph::CutpointSignature& signature,
		size_t aux,
		const std::vector<size_t>& index)
	{
		size_t auxSelector = static_cast<size_t>(-1);

		for (auto& cutpoint : signature)
		{
			// Assertions
			assert(cutpoint.root < index.size());
			assert(cutpoint.fwdSelectors.size());

			result.push_back(cutpoint);
			result.back().root = index[cutpoint.root];

			if (cutpoint.root == aux)
				auxSelector = *cutpoint.fwdSelectors.begin();

			if (cutpoint.root != root)
			{
				selectors.push_back(
					std::make_pair(*cutpoint.fwdSelectors.begin(), cutpoint.bwdSelector)
				);
			}
		}

		return auxSelector;
	}

	Box* createType1Box(
		size_t root,
		const std::shared_ptr<TreeAut>& output,
		const ConnectionGraph::CutpointSignature& signature,
		std::vector<size_t>& inputMap,
		const std::vector<size_t>& index)
	{
		ConnectionGraph::CutpointSignature outputSignature;
		std::vector<std::pair<size_t, size_t>> selectors;

		BoxMan::translateSignature(
			outputSignature, selectors, root, signature, static_cast<size_t>(-1), index
		);

		return new Box(
			"",
			output,
			outputSignature,
			inputMap,
			std::shared_ptr<TreeAut>(nullptr),
			0,
			ConnectionGraph::CutpointSignature(),
			selectors
		);
	}

	Box* createType2Box(
		size_t root,
		const std::shared_ptr<TreeAut>& output,
		const ConnectionGraph::CutpointSignature& signature,
		std::vector<size_t>& inputMap,
		size_t aux,
		const std::shared_ptr<TreeAut>& input,
		const ConnectionGraph::CutpointSignature& signature2,
		size_t inputSelector,
		std::vector<size_t>& index)
	{
		// Assertions
		assert(aux < index.size());
		assert(index[aux] >= 1);

		ConnectionGraph::CutpointSignature outputSignature, inputSignature;
		std::vector<std::pair<size_t, size_t>> selectors;

		size_t auxSelector = BoxMan::translateSignature(
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

	const Box* getBox(const Box& box)
	{
		auto cpBox = boxes_.get(box);

		if (boxes_.modified()) {

			Box* pBox = const_cast<Box*>(cpBox);

			pBox->name = this->getBoxName();
			pBox->initialize();

			CL_CDEBUG(1, "learning " << *static_cast<const AbstractBox*>(cpBox)
				<< ':' << std::endl << *cpBox);

#if FA_RESTART_AFTER_BOX_DISCOVERY
			throw RestartRequest("a new box encountered");
#endif
		}

		return cpBox;
	}

	const Box* lookupBox(const Box& box) const
	{
		return boxes_.lookup(box);
	}

public:

	BoxMan() :
		dataStore_{},
		dataIndex_{},
		nodeStore_{},
		tagStore_{},
		vDataStore_{},
		selIndex_{},
		typeIndex_{},
		boxes_{}
	{ }

	~BoxMan()
	{
		this->clear();
	}

	void clear()
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

	const BoxDatabase& boxDatabase() const
	{
		return boxes_;
	}
};

#endif
