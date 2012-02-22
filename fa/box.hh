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

#include <string>
#include <stdexcept>
#include <cassert>
#include <ostream>
#include <memory>

#include <unordered_map>

#include "tatimint.hh"
#include "types.hh"
//#include "forestaut.hh"
#include "treeaut.hh"
#include "abstractbox.hh"
#include "connection_graph.hh"

class NodeHead : public AbstractBox {

	size_t size;

public:

	NodeHead(size_t size)
		: AbstractBox(box_type_e::bHead, 0), size(size) {}

	size_t getSize() const {
		return this->size;
	}

	virtual void toStream(std::ostream& os) const {
		os << "Node[" << this->getSize() << ']';
	}

};

class TypeBox : public AbstractBox {

	std::string name;
	std::vector<size_t> selectors;

public:

	TypeBox(const std::string& name, const std::vector<size_t>& selectors)
		: AbstractBox(box_type_e::bTypeInfo, 0), name(name), selectors(selectors) {}

	const std::string& getName() const {
		return this->name;
	}

	const std::vector<size_t>& getSelectors() const {
		return this->selectors;
	}

	virtual void toStream(std::ostream& os) const {
		os << this->name;
		if (this->selectors.empty())
			return;
		os << '{';
		for (std::vector<size_t>::const_iterator i = this->selectors.begin(); i != this->selectors.end(); ++i)
			os << *i << ';';
		os << '}';
	}

};

class SelBox : public StructuralBox {

	const SelData* data;

	std::set<size_t> s[2];

public:

	SelBox(const SelData* data) : StructuralBox(box_type_e::bSel, 1), data(data) {

		assert(data);

		s[0].insert(data->offset);

		this->order = data->offset;

	}

	const SelData& getData() const {
		return *this->data;
	}

	virtual void toStream(std::ostream& os) const {
		os << *this->data;
	}

	virtual bool outputCovers(size_t offset) const {
		return this->data->offset == offset;
	}

	virtual const std::set<size_t>& outputCoverage() const {
		return s[0];
	}

	virtual const std::set<size_t>& inputCoverage(size_t index) const {

		if (!(index == 0))
			assert(false);

		return s[1];

	}

	virtual size_t selectorToInput(size_t index) const {

		if (!(index == 0))
			assert(false);

		return this->data->offset;

	}

	virtual size_t outputReachable(size_t) const {

		return (size_t)(-1);

	}

};

class Box : public StructuralBox {

	friend class BoxMan;

	std::string name;
	size_t hint;
	std::shared_ptr<TA<label_type>> output;
	ConnectionGraph::CutpointSignature outputSignature;
	std::vector<label_type> outputLabels;
	std::vector<size_t> inputMap;
	std::shared_ptr<TA<label_type>> input;
	size_t inputIndex;
	ConnectionGraph::CutpointSignature inputSignature;
	std::vector<label_type> inputLabels;
	std::vector<std::pair<size_t,size_t>> selectors;

	std::vector<std::set<size_t>> selCoverage;

	bool selfReference;


protected:

	static void getDownwardCoverage(std::vector<size_t>& v, const std::vector<const AbstractBox*>& label) {

		for (auto& absBox : label) {

			switch (absBox->getType()) {

				case box_type_e::bSel:
					v.push_back(((const SelBox*)absBox)->getData().offset);
					break;

				case box_type_e::bBox: {

					const Box* box = (const Box*)absBox;
					v.insert(v.end(), box->selCoverage[0].begin(), box->selCoverage[0].end());
					break;

				}

				default: continue;

			}

		}

	}

	static bool checkDownwardCoverage(const std::vector<size_t>& v, const TA<label_type>& ta) {

		for (TA<label_type>::iterator i = ta.accBegin(); i != ta.accEnd(i); ++i) {

			std::vector<size_t> v2;

			Box::getDownwardCoverage(v2, i->label()->getNode());

			if (v2 != v)
				return false;

		}

		return true;

	}

	static void getDownwardCoverage(std::set<size_t>& s, const TA<label_type>& ta) {

		std::vector<size_t> v;

		assert(ta.accBegin() != ta.accEnd());

		Box::getDownwardCoverage(v, ta.accBegin()->label()->getNode());

		assert(Box::checkDownwardCoverage(v, ta));

		std::sort(v.begin(), v.end());

		assert(std::unique(v.begin(), v.end()) == v.end());

		s.insert(v.begin(), v.end());

	}

	static void getAcceptingLabels(std::vector<label_type>& labels, const TA<label_type>& ta) {

		for (auto& state : ta.getFinalStates()) {

			for (auto i = ta.begin(state); i != ta.end(state, i); ++i)
				labels.push_back(i->label());

		}

		std::sort(labels.begin(), labels.end());

		labels.resize(std::unique(labels.begin(), labels.end()) - labels.begin());

	}

	Box(
		const std::string& name,
		const std::shared_ptr<TA<label_type>>& output,
		ConnectionGraph::CutpointSignature outputSignature,
		const std::vector<size_t>& inputMap,
		const std::shared_ptr<TA<label_type>>& input,
		size_t inputIndex,
		ConnectionGraph::CutpointSignature inputSignature,
		const std::vector<std::pair<size_t,size_t>>& selectors
	) : StructuralBox(box_type_e::bBox, selectors.size()), name(name), hint(), output(output),
		outputSignature(outputSignature), outputLabels(), inputMap(inputMap), input(input),
		inputIndex(inputIndex), inputSignature(inputSignature), inputLabels(), selectors(selectors) {

		Box::getAcceptingLabels(this->outputLabels, *output);

		boost::hash_combine(this->hint, selectors);
		boost::hash_combine(this->hint, this->outputLabels);
		boost::hash_combine(this->hint, outputSignature);

		if (input) {

			Box::getAcceptingLabels(this->inputLabels, *input);

			boost::hash_combine(this->hint, this->inputLabels);
			boost::hash_combine(this->hint, inputSignature);

		}

	}

	struct LeafEnumF {

		std::vector<std::set<size_t>>& selectors;
		const TA<label_type>& ta;
		const TT<label_type>& t;

		bool getRef(size_t state, size_t& ref) const {

			TA<label_type>::Iterator i = this->ta.begin(state);

			assert(i != this->ta.end(state));

			if (!i->label()->isData())
				return false;

			const Data& data = i->label()->getData();

			if (!data.isRef())
				return false;

			ref = data.d_ref.root;

			return true;

		}

		LeafEnumF(std::vector<std::set<size_t>>& selectors, const TA<label_type>& ta,
			const TT<label_type>& t) : selectors(selectors), ta(ta), t(t)  {}

		bool operator()(const AbstractBox* abox, size_t, size_t offset) {

			if (!abox->isType(box_type_e::bBox))
				return true;

			const Box* box = (const Box*)abox;

			for (size_t k = 0; k < box->getArity(); ++k, ++offset) {

				size_t ref;

				if (this->getRef(t.lhs()[offset], ref)) {

					this->selectors[ref].insert(
						box->inputCoverage(k).begin(), box->inputCoverage(k).end()
					);

				}

			}

			return true;

		}

	};

	// enumerates upward selectors
	void enumerateSelectorsAtLeaves(std::vector<std::set<size_t>>& selectors,
		const TA<label_type>& ta) const {

		for (auto i = ta.begin(); i != ta.end(); ++i) {

			if (i->label()->isNode())
				i->label()->iterate(LeafEnumF(selectors, ta, *i));

		}

	}

public:

	virtual bool outputCovers(size_t offset) const {

		assert(this->selCoverage.size());

		return this->selCoverage[0].count(offset) > 0;

	}

	virtual const std::set<size_t>& outputCoverage() const {

		assert(this->selCoverage.size());

		return this->selCoverage[0];

	}

	bool inputCovers(size_t index, size_t offset) const {

		assert((index + 1) < this->selCoverage.size());

		return this->selCoverage[index + 1].count(offset) > 0;

	}

	virtual const std::set<size_t>& inputCoverage(size_t index) const {

		assert((index + 1) < this->selCoverage.size());

		return this->selCoverage[index + 1];

	}

	virtual size_t selectorToInput(size_t input) const {

		assert(input < this->selectors.size());

		return this->selectors[input].first;

	}

	virtual size_t outputReachable(size_t input) const {

		assert(input < this->selectors.size());

		return this->selectors[input].second;

	}

	const TA<label_type>* getOutput() const {

		return this->output.get();

	}

	const ConnectionGraph::CutpointSignature& getOutputSignature() const {

		return this->outputSignature;

	}

	const TA<label_type>* getInput() const {

		return this->input.get();

	}

	size_t getInputIndex() const {

		return this->inputIndex;

	}

	static bool equal(const TA<label_type>& a, const TA<label_type>& b) {

		return TA<label_type>::subseteq(a, b) && TA<label_type>::subseteq(b, a);

	}

	static bool lessOrEqual(const TA<label_type>& a, const TA<label_type>& b) {

		return TA<label_type>::subseteq(a, b);

	}

	size_t getSelector(size_t input) const {

		assert(input < this->inputMap.size());

		return this->inputMap[input];

	}

	bool hasSelfReference() const {

		return this->selfReference;

	}

public:

	virtual void toStream(std::ostream& os) const {
		os << this->name << '(' << this->arity << ')';
	}

	friend size_t hash_value(const Box& box) {

		return box.hint;

	}

	bool operator==(const Box& rhs) const {

		if ((bool)this->input != (bool)rhs.input)
			return false;

		if (this->input) {

			if (this->inputIndex != rhs.inputIndex)
				return false;

			if (this->inputSignature != rhs.inputSignature)
				return false;

			if (this->inputLabels != rhs.inputLabels)
				return false;

		}

		if (this->outputSignature != rhs.outputSignature)
			return false;

		if (this->outputLabels != rhs.outputLabels)
			return false;

		if (this->selectors != rhs.selectors)
			return false;

		if (!Box::equal(*this->output, *rhs.output))
			return false;

		if (!this->input)
			return true;

		return Box::equal(*this->input, *rhs.input);

	}

	bool operator<=(const Box& rhs) const {

		if ((bool)this->input != (bool)rhs.input)
			return false;

		if (this->input) {

			if (this->inputIndex != rhs.inputIndex)
				return false;

			if (this->inputSignature != rhs.inputSignature)
				return false;

			if (this->inputLabels != rhs.inputLabels)
				return false;

		}

		if (this->outputSignature != rhs.outputSignature)
			return false;

		if (this->outputLabels != rhs.outputLabels)
			return false;

		if (this->selectors != rhs.selectors)
			return false;

		if (!Box::lessOrEqual(*this->output, *rhs.output))
			return false;

		if (!this->input)
			return true;

		return Box::lessOrEqual(*this->input, *rhs.input);

	}

	void initialize() {

		this->selCoverage.resize(this->arity + 1);

		Box::getDownwardCoverage(this->selCoverage[0], *this->output);

		assert(this->selCoverage[0].size());

		this->order = *this->selCoverage[0].begin();

		this->enumerateSelectorsAtLeaves(this->selCoverage, *this->output);

		if (!this->input)
			return;

		assert(this->inputIndex < this->selCoverage.size());

		Box::getDownwardCoverage(this->selCoverage[this->inputIndex + 1], *this->input);

		this->enumerateSelectorsAtLeaves(this->selCoverage, *this->input);

		this->selfReference = ConnectionGraph::containsCutpoint(this->outputSignature, 0);

	}

	friend std::ostream& operator<<(std::ostream& os, const Box& box) {

		auto writeStateF = [](size_t state) -> std::string {

			std::ostringstream ss;

			if (_MSB_TEST(state))
				ss << 'r' << _MSB_GET(state);
			else
				ss << 'q' << state;

			return ss.str();

		};

		os << "===" << std::endl << "output (";

		for(auto& s : box.outputCoverage())
			os << " +" << s;

		os << " ) [" << box.outputSignature << "] ";

		TAWriter<label_type> writer(os);

		for (auto state : box.output->getFinalStates())
			writer.writeState(state);

		writer.endl();
		writer.writeTransitions(*box.output, writeStateF);

		if (!box.input)
			return os;

		os << "===" << std::endl << "input " << box.inputIndex << " (";

		for(auto& s : box.inputCoverage(box.inputIndex))
			os << " +" << s;

		os << " ) [" << box.inputSignature << "] ";

		for (auto state : box.input->getFinalStates())
			writer.writeState(state);

		writer.endl();
		writer.writeTransitions(*box.input, writeStateF);

		return os;

	}

};

#endif
