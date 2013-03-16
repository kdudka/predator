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
#include <unordered_map>

// Forester headers
#include "box.hh"

class BoxAntichain
{
private:  // data types

	typedef std::unordered_map<Box::Signature, std::list<Box>,
		boost::hash<Box::Signature>> TBoxStore;

	typedef TBoxStore::const_iterator const_iterator;

private:  // data members

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


	/**
	 * @brief  Retrieves a box from the antichain (may insert it)
	 *
	 * This method retrieves the unique pointer to @p box from the antichain (in
	 * the case it is not present it inserts it).
	 *
	 * @param[in]  box  The box to be retrieved
	 *
	 * @returns  The unique pointer to @p box
	 */
	const Box* get(const Box& box);


	/**
	 * @brief  Retrievse a box from the antichain
	 *
	 * This method retrieves the unique pointer to @p box (or @p nullptr if not
	 * present) from the antichain.
	 *
	 * @param[in]  box  The box to be retrieved
	 *
	 * @returns  The unique pointer to @p box or @p nullptr
	 */
	const Box* lookup(const Box& box) const;


	/**
	 * @brief  Has the antichain been modified?
	 *
	 * This method returns @p true in the case the antichain has been modified
	 * during the last BoxAntichain::get() operation (the operation resets it at
	 * the beginning).
	 *
	 * @returns  Has the antichain been modified?
	 */
	bool modified() const
	{
		return modified_;
	}


	/**
	 * @brief  The count of active elements
	 *
	 * Returns the count of active elements (obsolete are not counted).
	 *
	 * @returns  The count of active elements
	 */
	size_t size() const
	{
		return size_ - obsolete_.size();
	}


	/**
	 * @brief  Is the antichain empty?
	 *
	 * Returns @p true in the case the antichain is empty, not taking into account
	 * obsolete elements, @p false otherwise.
	 *
	 * @returns @p true in the case the antichain is empty, @p false otherwise
	 */
	bool empty() const
	{
		return this->size() == 0;
	}


	void clear()
	{
		boxes_.clear();
	}

	const_iterator begin() const
	{
		return boxes_.begin();
	}

	const_iterator end() const
	{
		return boxes_.end();
	}

	void asVector(std::vector<const Box*>& boxes) const;
};


class BoxSet
{
private:  // data types

	typedef std::unordered_set<Box, boost::hash<Box>> TBoxSet;

	typedef TBoxSet::const_iterator const_iterator;

private:  // data members

	TBoxSet boxes_;

	bool modified_;

public:

	BoxSet() :
		boxes_(),
		modified_(false)
	{ }

	/**
	 * @brief  Retrieves a box from the set (may insert it)
	 *
	 * This method retrieves the unique pointer to @p box from the set (in
	 * the case it is not present it inserts it).
	 *
	 * @param[in]  box  The box to be retrieved
	 *
	 * @returns  The unique pointer to @p box
	 */
	const Box* get(const Box& box)
	{
		auto iterBoolPair = boxes_.insert(box);
		modified_ = iterBoolPair.second;
		return &*iterBoolPair.first;
	}


	/**
	 * @brief  Retrievse a box from the set
	 *
	 * This method retrieves the unique pointer to @p box (or @p nullptr if not
	 * present) from the set.
	 *
	 * @param[in]  box  The box to be retrieved
	 *
	 * @returns  The unique pointer to @p box or @p nullptr
	 */
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


	/**
	 * @brief  The count of active elements
	 *
	 * Returns the count of active elements.
	 *
	 * @returns  The count of active elements
	 */
	size_t size() const
	{
		return boxes_.size();
	}


	/**
	 * @brief  Is the set empty?
	 *
	 * Returns @p true in the case the set is empty, @p false otherwise.
	 *
	 * @returns @p true in the case the set is empty, @p false otherwise
	 */
	bool empty() const
	{
		return boxes_.empty();
	}


	const_iterator begin() const
	{
		return boxes_.begin();
	}

	const_iterator end() const
	{
		return boxes_.end();
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

	typedef std::unordered_map<Data, NodeLabel*, boost::hash<Data>> TDataStore;
	typedef std::unordered_map<std::vector<const AbstractBox*>, NodeLabel*,
		boost::hash<std::vector<const AbstractBox*>>> TNodeStore;
	typedef std::unordered_map<std::pair<size_t, DataArray>, NodeLabel*,
		boost::hash<std::pair<size_t, DataArray>>> TVarDataStore;
	typedef std::unordered_set< std::pair<const TypeBox*, std::vector<size_t>>,
		boost::hash<std::pair<const TypeBox*, std::vector<size_t>>>> TTagStore;
	typedef std::unordered_map<SelData, const SelBox*, boost::hash<SelData>>
		TSelIndex;
	typedef std::unordered_map<std::string, const TypeBox*> TTypeIndex;
	typedef std::unordered_map<const TypeBox*, std::vector<SelData>>
		TTypeDescDict;

private:  // data members

	TDataStore dataStore_;
	std::vector<const Data*> dataIndex_;
	TNodeStore nodeStore_;
	TTagStore tagStore_;
	TVarDataStore vDataStore_;

	TSelIndex selIndex_;
	TTypeIndex typeIndex_;

	BoxDatabase boxes_;

	TTypeDescDict typeDescDict_;

private:  // methods

	const std::pair<const Data, NodeLabel*>& insertData(const Data& data);


	/**
	 * @brief  Retrieves the name of a new box
	 *
	 * This method retrieves the name of a new box.
	 *
	 * @returns  The name of a new box
	 */
	std::string getBoxName() const;


public:

	label_type lookupLabel(const Data& data)
	{
		return this->insertData(data).second;
	}

	label_type lookupLabel(size_t arity, const DataArray& x);

	const std::vector<SelData>* LookupTypeDesc(
		const TypeBox* tb,
		const std::vector<SelData>& sels)
	{
		auto itBoolPair = typeDescDict_.insert(std::make_pair(tb, sels));
		if (!itBoolPair.second)
		{	// in case a new element was not inserted
			const std::vector<SelData>& oldSels = itBoolPair.first->second;
			if (sels != oldSels)
			{
				assert(false);       // fail gracefully
			}
		}

		return &itBoolPair.first->second;
	}

	struct EvaluateBoxF
	{
		NodeLabel& label;
		std::vector<size_t>& tag;

		EvaluateBoxF(
			NodeLabel& label,
			std::vector<size_t>& tag
		) :
			label(label),
			tag(tag)
		{ }

		bool operator()(const AbstractBox* aBox, size_t index, size_t offset);
	};

	label_type lookupLabel(
		const std::vector<const AbstractBox*>&       x,
		const std::vector<SelData>*                  nodeInfo = nullptr);

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

	const SelBox* getSelector(const SelData& sel);

	const TypeBox* getTypeInfo(const std::string& name);

	const TypeBox* createTypeInfo(
		const std::string&                           name,
		const std::vector<size_t>&                   selectors);

	/**
	 * @brief  Creates a box with a single component
	 *
	 * This static method creates a new box with a single (output) component.
	 *
	 * @param[in]  root       Index of the tree automaton which is to be put in
	 *                        the box
	 * @param[in]  output     The tree automaton to be put into the box
	 * @param[in]  signature  The signature of @p output
	 * @param[in]  inputMap   The mapping of cutpoints to selectors
	 * @param[in]  index      Index for renaming cutpoints
	 *
	 * @returns  The created box with the @p output tree automaton inside
	 */
	static Box* createType1Box(
		size_t                                      root,
		const std::shared_ptr<TreeAut>&             output,
		const ConnectionGraph::CutpointSignature&   signature,
		const std::vector<size_t>&                  inputMap,
		const std::vector<size_t>&                  index);


	/**
	 * @brief  Creates a box with a pair of components
	 *
	 * This static method creates a new box that contains a pair of components:
	 * the @p output component (the starts in the first tree), and the @p input
	 * component (that may go backwards).
	 *
	 * @param[in]      root           The index of the @p output tree automaton
	 * @param[in]      output         The output tree automaton to be put in the
	 *                                box
	 * @param[in]      signature      Signature of the @p output tree automaton
	 * @param[in]      inputMap       The mapping of cutpoints to selectors
	 * @param[in]      aux            The index of the @p input tree automaton
	 * @param[in]      input          The input tree automaton to be put in the
	 *                                box
	 * @param[in]      signature2     Signature of the @p input tree automaton
	 * @param[in]      inputSelector  Offset of the lowest selector in the box
	 * @param[in,out]  index          Index for renaming cutpoints (may change)
	 *
	 * @returns  The created box with the @p output and @p input tree automata
	 *           inside
	 */
	static Box* createType2Box(
		size_t                                       root,
		const std::shared_ptr<TreeAut>&              output,
		const ConnectionGraph::CutpointSignature&    signature,
		const std::vector<size_t>&                   inputMap,
		size_t                                       aux,
		const std::shared_ptr<TreeAut>&              input,
		const ConnectionGraph::CutpointSignature&    signature2,
		size_t                                       inputSelector,
		std::vector<size_t>&                         index);


	/**
	 * @brief  Retrieves a desired box from the database (may insert it)
	 *
	 * This method searches the database of boxes for the @p box and returns
	 * a unique pointer to it. In the case a new box is inserted, it is
	 * initialized.
	 *
	 * @param[in]  box  The box to be found (or inserted) in the database
	 *
	 * @returns  Unique pointer to the box
	 */
	const Box* getBox(const Box& box);


	const Box* lookupBox(const Box& box) const
	{
		return boxes_.lookup(box);
	}

	BoxMan() :
		dataStore_{},
		dataIndex_{},
		nodeStore_{},
		tagStore_{},
		vDataStore_{},
		selIndex_{},
		typeIndex_{},
		boxes_{},
		typeDescDict_{}
	{ }

	~BoxMan()
	{
		this->clear();
	}

	void clear();

	const BoxDatabase& boxDatabase() const
	{
		return boxes_;
	}
};

#endif
