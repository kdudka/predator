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

#ifndef BOX_H
#define BOX_H

// Standard library headers
#include <cassert>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

// Forester headers
#include "tatimint.hh"
#include "types.hh"
#include "treeaut_label.hh"
#include "abstractbox.hh"
#include "connection_graph.hh"


class NodeHead : public AbstractBox {

	size_t size;

public:

	NodeHead(size_t size) :
		AbstractBox(box_type_e::bHead, 0),
		size(size)
	{ }

	size_t getSize() const
	{
		return this->size;
	}

	virtual void toStream(std::ostream& os) const
	{
		os << "Node[" << this->getSize() << ']';
	}
};

class TypeBox : public AbstractBox
{
	std::string name;
	std::vector<size_t> selectors;

public:

	TypeBox(const std::string& name, const std::vector<size_t>& selectors) :
		AbstractBox(box_type_e::bTypeInfo, 0),
		name(name),
		selectors(selectors)
	{ }

	const std::string& getName() const
	{
		return this->name;
	}

	const std::vector<size_t>& getSelectors() const
	{
		return this->selectors;
	}

	virtual void toStream(std::ostream& os) const;

	virtual ~TypeBox()
	{ }
};


/**
 * @brief  TODO
 *
 * @todo
 */
class SelBox : public StructuralBox
{
	const SelData* data_;

	std::set<size_t> s[2];

private:  // methods

	SelBox(const SelBox&);
	SelBox& operator=(const SelBox&);

public:

	SelBox(const SelData* data) :
		StructuralBox(box_type_e::bSel, 1),
		data_(data)
	{
		// Assertions
		assert(data_ != nullptr);

		s[0].insert(data_->offset);

		this->order = data_->offset;
	}

	const SelData& getData() const
	{
		// Assertions
		assert(data_ != nullptr);

		return *data_;
	}

	virtual void toStream(std::ostream& os) const
	{
		// Assertions
		assert(data_ != nullptr);

		os << *data_;
	}

	virtual bool outputCovers(size_t offset) const
	{
		// Assertions
		assert(data_ != nullptr);

		return data_->offset == offset;
	}

	virtual const std::set<size_t>& outputCoverage() const
	{
		return s[0];
	}

	virtual const std::set<size_t>& inputCoverage(size_t index) const
	{
		if (index != 0)
			assert(false);

		return s[1];
	}

	virtual size_t selectorToInput(size_t index) const
	{
		if (index != 0)
			assert(false);

		return data_->offset;
	}

	virtual size_t outputReachable(size_t) const
	{
		return static_cast<size_t>(-1);
	}

	virtual size_t getRealRefCount(size_t) const
	{
		return 1;
	}

	virtual ~SelBox()
	{
		/// TODO @todo do something?
	}
};


class Box : public StructuralBox {

	friend class BoxMan;

	std::string name;
	size_t hint;
	std::shared_ptr<TreeAut> output;
	ConnectionGraph::CutpointSignature outputSignature;
	std::vector<label_type> outputLabels;
	std::vector<size_t> inputMap;
	std::shared_ptr<TreeAut> input;
	size_t inputIndex;
	ConnectionGraph::CutpointSignature inputSignature;
	std::vector<label_type> inputLabels;
	std::vector<std::pair<size_t,size_t>> selectors;

	std::vector<std::set<size_t>> selCoverage;

	bool selfReference;

public:

	struct Signature {

		ConnectionGraph::CutpointSignature outputSignature;
		size_t inputIndex;
		ConnectionGraph::CutpointSignature inputSignature;
		std::vector<std::pair<size_t,size_t>> selectors;

		Signature(
			const ConnectionGraph::CutpointSignature& outputSignature,
			size_t inputIndex,
			const ConnectionGraph::CutpointSignature& inputSignature,
			const std::vector<std::pair<size_t,size_t>>& selectors
		) :
			outputSignature(outputSignature),
			inputIndex(inputIndex),
			inputSignature(inputSignature),
			selectors(selectors)
		{ }

		bool operator==(const Signature& rhs) const
		{
			return this->outputSignature == rhs.outputSignature &&
				this->inputIndex == rhs.inputIndex &&
				this->inputSignature == rhs.inputSignature &&
				this->selectors == rhs.selectors;
		}

		friend size_t hash_value(const Signature& signature);
	};

	Signature getSignature() const
	{
		return Signature(
			this->outputSignature, this->inputIndex, this->inputSignature, this->selectors
		);
	}

protected:

	static void getDownwardCoverage(
		std::vector<size_t>& v,
		const std::vector<const AbstractBox*>& label);

	static bool checkDownwardCoverage(
		const std::vector<size_t>& v,
		const TreeAut& ta);

	static void getDownwardCoverage(
		std::set<size_t>& s,
		const TreeAut& ta);

	static void getAcceptingLabels(
		std::vector<label_type>& labels,
		const TreeAut& ta);

	Box(
		const std::string& name,
		const std::shared_ptr<TreeAut>& output,
		ConnectionGraph::CutpointSignature outputSignature,
		const std::vector<size_t>& inputMap,
		const std::shared_ptr<TreeAut>& input,
		size_t inputIndex,
		ConnectionGraph::CutpointSignature inputSignature,
		const std::vector<std::pair<size_t,size_t>>& selectors
	);

	struct LeafEnumF {

		std::vector<std::set<size_t>>& selectors;
		const TreeAut& ta;
		const TT<label_type>& t;

		bool getRef(size_t state, size_t& ref) const;

		LeafEnumF(
			std::vector<std::set<size_t>>& selectors,
			const TreeAut& ta,
			const TT<label_type>& t
		) :
			selectors(selectors),
			ta(ta),
			t(t)
		{ }

		bool operator()(const AbstractBox* abox, size_t, size_t offset);
	};

	// enumerates upward selectors
	void enumerateSelectorsAtLeaves(
		std::vector<std::set<size_t>>& selectors,
		const TreeAut& ta) const;

public:

	virtual bool outputCovers(size_t offset) const
	{
		assert(this->selCoverage.size());
		return this->selCoverage[0].count(offset) > 0;
	}

	virtual const std::set<size_t>& outputCoverage() const
	{
		assert(this->selCoverage.size());
		return this->selCoverage[0];
	}

	bool inputCovers(size_t index, size_t offset) const
	{
		assert((index + 1) < this->selCoverage.size());
		return this->selCoverage[index + 1].count(offset) > 0;
	}

	virtual const std::set<size_t>& inputCoverage(size_t index) const
	{
		assert((index + 1) < this->selCoverage.size());
		return this->selCoverage[index + 1];
	}

	virtual size_t selectorToInput(size_t input) const
	{
		assert(input < this->selectors.size());
		return this->selectors[input].first;
	}

	virtual size_t outputReachable(size_t input) const
	{
		assert(input < this->selectors.size());
		return this->selectors[input].second;
	}

	virtual size_t getRealRefCount(size_t input) const
	{
		assert(input < this->outputSignature.size());
		return this->outputSignature[input].realRefCount;
	}

	const TreeAut* getOutput() const
	{
		return this->output.get();
	}

	const ConnectionGraph::CutpointSignature& getOutputSignature() const
	{
		return this->outputSignature;
	}

	const TreeAut* getInput() const
	{
		return this->input.get();
	}

	size_t getInputIndex() const
	{
		return this->inputIndex;
	}

	static bool equal(const TreeAut& a, const TreeAut& b)
	{
		return TreeAut::subseteq(a, b) && TreeAut::subseteq(b, a);
	}

	static bool lessOrEqual(const TreeAut& a, const TreeAut& b)
	{
		return TreeAut::subseteq(a, b);
	}

	size_t getSelector(size_t input) const
	{
		assert(input < this->inputMap.size());
		return this->inputMap[input];
	}

	bool hasSelfReference() const
	{
		return this->selfReference;
	}

public:

	virtual void toStream(std::ostream& os) const
	{
		os << this->name << '(' << this->arity << ')';
	}

	friend size_t hash_value(const Box& box)
	{
		return box.hint;
	}

	bool operator==(const Box& rhs) const;

	bool operator<=(const Box& rhs) const;

	bool simplifiedLessThan(const Box& rhs) const;

	void initialize();

	friend std::ostream& operator<<(std::ostream& os, const Box& box);

	virtual ~Box()
	{ }
};

#endif
