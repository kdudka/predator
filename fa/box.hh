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


class TypeBox : public AbstractBox
{
private:  // data members

	std::string name_;
	std::vector<size_t> selectors_;

public:   // methods

	TypeBox(
		const std::string&             name,
		const std::vector<size_t>&     selectors) :
		AbstractBox(box_type_e::bTypeInfo, 0),
		name_(name),
		selectors_(selectors)
	{ }

	const std::string& getName() const
	{
		return name_;
	}

	const std::vector<size_t>& getSelectors() const
	{
		return selectors_;
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
private:  // data members

	const SelData* data_;

	std::set<size_t> s_[2];

private:  // methods

	SelBox(const SelBox&);
	SelBox& operator=(const SelBox&);

public:   // methods

	SelBox(const SelData* data) :
		StructuralBox(box_type_e::bSel, 1),
		data_(data)
	{
		// Assertions
		assert(data_ != nullptr);

		s_[0].insert(data_->offset);

		order_ = data_->offset;
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
		return s_[0];
	}

	virtual const std::set<size_t>& inputCoverage(size_t index) const
	{
		if (index != 0)
			assert(false);

		return s_[1];
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

	virtual size_t getSelCount(size_t) const
	{
		return 1;
	}

	virtual ~SelBox()
	{
		/// TODO @todo do something?
	}
};


/**
 * @brief  The box containing a nested forest automaton
 *
 * This box contains a nested forest automaton with 1 or 2 components.
 *
 * @note  This is a simplification of the theoretic box which may have an
 * arbitrary number of components!
 */
class Box : public StructuralBox
{
private:  // data types

	friend class BoxMan;

private:  // data members

	std::string name_;
	size_t hint_;
	std::shared_ptr<TreeAut> output_;
	ConnectionGraph::CutpointSignature outputSignature_;
	std::vector<label_type> outputLabels_;
	std::vector<size_t> inputMap_;
	std::shared_ptr<TreeAut> input_;
	size_t inputIndex_;
	ConnectionGraph::CutpointSignature inputSignature_;
	std::vector<label_type> inputLabels_;
	std::vector<std::pair<size_t,size_t>> selectors_;

	std::vector<std::set<size_t>> selCoverage_;

	bool selfReference_;

public:

	struct Signature
	{
		ConnectionGraph::CutpointSignature outputSignature;
		size_t inputIndex;
		ConnectionGraph::CutpointSignature inputSignature;
		std::vector<std::pair<size_t,size_t>> selectors;

		Signature(
			const ConnectionGraph::CutpointSignature&       outputSignature,
			size_t                                          inputIndex,
			const ConnectionGraph::CutpointSignature&       inputSignature,
			const std::vector<std::pair<size_t,size_t>>&    selectors) :
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
		return Signature( outputSignature_, inputIndex_, inputSignature_, selectors_);
	}

protected:

	static void getDownwardCoverage(
		std::vector<size_t>&                       v,
		const std::vector<const AbstractBox*>&     label);

	static bool checkDownwardCoverage(
		const std::vector<size_t>&                 v,
		const TreeAut&                             ta);

	static void getDownwardCoverage(
		std::set<size_t>&                          s,
		const TreeAut&                             ta);

	static void getAcceptingLabels(
		std::vector<label_type>&                   labels,
		const TreeAut&                             ta);



	struct LeafEnumF
	{
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
		std::vector<std::set<size_t>>&        selectors,
		const TreeAut&                        ta) const;

public:

	virtual bool outputCovers(size_t offset) const
	{
		assert(selCoverage_.size());
		return selCoverage_[0].count(offset) > 0;
	}

	virtual const std::set<size_t>& outputCoverage() const
	{
		assert(selCoverage_.size());
		return selCoverage_[0];
	}

	bool inputCovers(size_t index, size_t offset) const
	{
		assert((index + 1) < selCoverage_.size());
		return selCoverage_[index + 1].count(offset) > 0;
	}

	virtual const std::set<size_t>& inputCoverage(size_t index) const
	{
		assert((index + 1) < selCoverage_.size());
		return selCoverage_[index + 1];
	}

	size_t outPortsNum() const
	{
		assert(!selCoverage_.empty());
		return selCoverage_.size() - 1;
	}

	virtual size_t selectorToInput(size_t input) const
	{
		assert(input < selectors_.size());
		return selectors_[input].first;
	}

	virtual size_t outputReachable(size_t input) const
	{
		assert(input < selectors_.size());
		return selectors_[input].second;
	}

	virtual size_t getSelCount(size_t input) const
	{
		assert(input < outputSignature_.size());
		return outputSignature_[input].selCount;
	}

	const TreeAut* getOutput() const
	{
		return output_.get();
	}

	const ConnectionGraph::CutpointSignature& getOutputSignature() const
	{
		return outputSignature_;
	}

	const std::string& getName() const
	{
		return name_;
	}

	const TreeAut* getInput() const
	{
		return input_.get();
	}

	size_t getInputIndex() const
	{
		return inputIndex_;
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
		assert(input < inputMap_.size());
		return inputMap_[input];
	}

	bool hasSelfReference() const
	{
		return selfReference_;
	}

public:


	/**
	 * @brief  Constructor
	 *
	 * Construct a box of the name @p name with the output direction tree
	 * automaton @p output with the signature @p outputSignature and the map @p
	 * inputMap of components to selectors and with the input direction tree
	 * automaton @p input with the index of the input automaton @p inputIndex.
	 *
	 * @param[in]  name             The name of the box
	 * @param[in]  output           The tree automaton for the output
	 * @param[in]  outputSignature  The signature of the output component
	 * @param[in]  inputMap         The map of components to selectors
	 * @param[in]  input            The tree automaton for the input
	 * @param[in]  inputIndex       Index of the input tree automaton
	 * @param[in]  inputSignature   The signature of the input component
	 * @param[in]  selectors        The vector of pairs of forward and backward
	 *                              selectors
	 */
	Box(
		const std::string&                               name,
		const std::shared_ptr<TreeAut>&                  output,
		ConnectionGraph::CutpointSignature               outputSignature,
		const std::vector<size_t>&                       inputMap,
		const std::shared_ptr<TreeAut>&                  input,
		size_t                                           inputIndex,
		ConnectionGraph::CutpointSignature               inputSignature,
		const std::vector<std::pair<size_t,size_t>>&     selectors);


	virtual void toStream(std::ostream& os) const;

	friend size_t hash_value(const Box& box)
	{
		return box.hint_;
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
