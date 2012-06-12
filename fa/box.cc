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
#include "box.hh"


void TypeBox::toStream(std::ostream& os) const
{
	os << this->name;
	if (this->selectors.empty())
		return;

	os << '{';
	for (auto it = this->selectors.cbegin(); it != this->selectors.cend(); ++it)
		os << *it << ';';

	os << '}';
}


size_t hash_value(const Box::Signature& signature)
{
	size_t h = 0;
	boost::hash_combine(h, signature.outputSignature);
	boost::hash_combine(h, signature.inputIndex);
	boost::hash_combine(h, signature.inputSignature);
	boost::hash_combine(h, signature.selectors);
	return h;
}


void Box::getDownwardCoverage(
	std::vector<size_t>& v,
	const std::vector<const AbstractBox*>& label)
{
	for (auto& absBox : label)
	{
		switch (absBox->getType())
		{
			case box_type_e::bSel:
				v.push_back((static_cast<const SelBox*>(absBox))->getData().offset);
				break;

			case box_type_e::bBox:
			{
				const Box* box = static_cast<const Box*>(absBox);
				v.insert(v.end(), box->selCoverage[0].begin(), box->selCoverage[0].end());
				break;
			}

			default: continue;
		}
	}
}


bool Box::checkDownwardCoverage(
	const std::vector<size_t>& v,
	const TreeAut& ta)
{
	for (TreeAut::iterator i = ta.accBegin(); i != ta.accEnd(i); ++i)
	{
		std::vector<size_t> v2;

		Box::getDownwardCoverage(v2, i->label()->getNode());

		if (v2 != v)
			return false;
	}

	return true;
}


void Box::getDownwardCoverage(
	std::set<size_t>& s,
	const TreeAut& ta)
{
	std::vector<size_t> v;

	assert(ta.accBegin() != ta.accEnd());

	Box::getDownwardCoverage(v, ta.accBegin()->label()->getNode());

	assert(Box::checkDownwardCoverage(v, ta));

	std::sort(v.begin(), v.end());

	assert(std::unique(v.begin(), v.end()) == v.end());

	s.insert(v.begin(), v.end());
}


void Box::getAcceptingLabels(
	std::vector<label_type>& labels,
	const TreeAut& ta)
{
	for (auto& state : ta.getFinalStates())
	{
		for (auto i = ta.begin(state); i != ta.end(state, i); ++i)
			labels.push_back(i->label());
	}

	std::sort(labels.begin(), labels.end());

	labels.resize(std::unique(labels.begin(), labels.end()) - labels.begin());
}


Box::Box(
	const std::string& name,
	const std::shared_ptr<TreeAut>& output,
	ConnectionGraph::CutpointSignature outputSignature,
	const std::vector<size_t>& inputMap,
	const std::shared_ptr<TreeAut>& input,
	size_t inputIndex,
	ConnectionGraph::CutpointSignature inputSignature,
	const std::vector<std::pair<size_t,size_t>>& selectors
) :
	StructuralBox(box_type_e::bBox, selectors.size()),
	name(name),
	hint(),
	output(output),
	outputSignature(outputSignature),
	outputLabels(),
	inputMap(inputMap),
	input(input),
	inputIndex(inputIndex),
	inputSignature(inputSignature),
	inputLabels(),
	selectors(selectors),
	selCoverage{},
	selfReference{}
{
	Box::getAcceptingLabels(this->outputLabels, *output);

	boost::hash_combine(this->hint, selectors);
	boost::hash_combine(this->hint, this->outputLabels);
	boost::hash_combine(this->hint, outputSignature);

	if (input)
	{
		Box::getAcceptingLabels(this->inputLabels, *input);

		boost::hash_combine(this->hint, this->inputLabels);
		boost::hash_combine(this->hint, inputSignature);
	}
}


bool Box::LeafEnumF::getRef(size_t state, size_t& ref) const
{
	TreeAut::Iterator i = this->ta.begin(state);

	assert(i != this->ta.end(state));

	if (!i->label()->isData())
		return false;

	const Data& data = i->label()->getData();

	if (!data.isRef())
		return false;

	ref = data.d_ref.root;

	return true;
}


bool Box::LeafEnumF::operator()(
	const AbstractBox* abox,
	size_t,
	size_t offset)
{
	if (!abox->isType(box_type_e::bBox))
		return true;

	const Box* box = static_cast<const Box*>(abox);

	for (size_t k = 0; k < box->getArity(); ++k, ++offset)
	{
		size_t ref;

		if (this->getRef(t.lhs()[offset], ref))
		{
			this->selectors[ref].insert(
				box->inputCoverage(k).begin(), box->inputCoverage(k).end()
			);
		}
	}

	return true;
}


void Box::enumerateSelectorsAtLeaves(
	std::vector<std::set<size_t>>& selectors,
	const TreeAut& ta) const
{
	for (auto i = ta.begin(); i != ta.end(); ++i)
	{
		if (i->label()->isNode())
			i->label()->iterate(LeafEnumF(selectors, ta, *i));
	}
}


bool Box::operator==(const Box& rhs) const
{
	if (static_cast<bool>(this->input) != static_cast<bool>(rhs.input))
		return false;

	if (this->input)
	{
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


bool Box::operator<=(const Box& rhs) const
{
	if (static_cast<bool>(this->input) != static_cast<bool>(rhs.input))
		return false;

	if (this->input)
	{
		if (this->inputIndex != rhs.inputIndex)
			return false;

		if (this->inputSignature != rhs.inputSignature)
			return false;
	}

	if (this->outputSignature != rhs.outputSignature)
		return false;

	if (this->selectors != rhs.selectors)
		return false;

	if (!Box::lessOrEqual(*this->output, *rhs.output))
		return false;

	if (!this->input)
		return true;

	return Box::lessOrEqual(*this->input, *rhs.input);
}


bool Box::simplifiedLessThan(const Box& rhs) const
{
	if (static_cast<bool>(this->input) != static_cast<bool>(rhs.input))
		return false;

	if (this->input)
	{
		if (this->inputIndex != rhs.inputIndex)
			return false;
	}

	if (!Box::lessOrEqual(*this->output, *rhs.output))
		return false;

	if (!this->input)
		return true;

	return Box::lessOrEqual(*this->input, *rhs.input);
}


void Box::initialize()
{

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


std::ostream& operator<<(std::ostream& os, const Box& box)
{
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
