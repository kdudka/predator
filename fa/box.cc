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
	os << name_;
	if (selectors_.empty())
		return;

	os << '{';
	for (auto it = selectors_.cbegin(); it != selectors_.cend(); ++it)
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
	std::vector<size_t>&                          v,
	const std::vector<const AbstractBox*>&        label)
{
	for (const AbstractBox* absBox : label)
	{	// for all boxes in the label
		switch (absBox->getType())
		{	// according to the type of the box
			case box_type_e::bSel: // selector
				v.push_back((static_cast<const SelBox*>(absBox))->getData().offset);
				break;

			case box_type_e::bBox: // hierarchical box
			{
				const Box* box = static_cast<const Box*>(absBox);
				v.insert(v.end(), box->selCoverage_[0].begin(), box->selCoverage_[0].end());
				break;
			}

			default: continue;
		}
	}
}


bool Box::checkDownwardCoverage(
	const std::vector<size_t>&                    v,
	const TreeAut&                                ta)
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
	std::set<size_t>&                             s,
	const TreeAut&                                ta)
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
	std::vector<label_type>&                       labels,
	const TreeAut&                                 ta)
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
	const std::string&                                 name,
	const std::shared_ptr<TreeAut>&                    output,
	ConnectionGraph::CutpointSignature                 outputSignature,
	const std::vector<size_t>&                         inputMap,
	const std::shared_ptr<TreeAut>&                    input,
	size_t                                             inputIndex,
	ConnectionGraph::CutpointSignature                 inputSignature,
	const std::vector<std::pair<size_t,size_t>>&       selectors
) :
	StructuralBox(box_type_e::bBox, selectors.size()),
	name_(name),
	hint_(),
	output_(output),
	outputSignature_(outputSignature),
	outputLabels_(),
	inputMap_(inputMap),
	input_(input),
	inputIndex_(inputIndex),
	inputSignature_(inputSignature),
	inputLabels_(),
	selectors_(selectors),
	selCoverage_{},
	selfReference_{}
{
	Box::getAcceptingLabels(outputLabels_, *output_);

	boost::hash_combine(hint_, selectors_);
	boost::hash_combine(hint_, outputLabels_);
	boost::hash_combine(hint_, outputSignature_);

	if (input_)
	{
		Box::getAcceptingLabels(inputLabels_, *input_);

		boost::hash_combine(hint_, inputLabels_);
		boost::hash_combine(hint_, inputSignature_);
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
	if (static_cast<bool>(input_) != static_cast<bool>(rhs.input_))
		return false;

	if (input_)
	{
		if (inputIndex_ != rhs.inputIndex_)
			return false;

		if (inputSignature_ != rhs.inputSignature_)
			return false;

		if (inputLabels_ != rhs.inputLabels_)
			return false;
	}

	if (outputSignature_ != rhs.outputSignature_)
		return false;

	if (outputLabels_ != rhs.outputLabels_)
		return false;

	if (selectors_ != rhs.selectors_)
		return false;

	if (!Box::equal(*output_, *rhs.output_))
		return false;

	if (!input_)
		return true;

	return Box::equal(*input_, *rhs.input_);
}


bool Box::operator<=(const Box& rhs) const
{
	if (static_cast<bool>(input_) != static_cast<bool>(rhs.input_))
		return false;

	if (input_)
	{
		if (inputIndex_ != rhs.inputIndex_)
			return false;

		if (inputSignature_ != rhs.inputSignature_)
			return false;
	}

	if (outputSignature_ != rhs.outputSignature_)
		return false;

	if (selectors_ != rhs.selectors_)
		return false;

	if (!Box::lessOrEqual(*output_, *rhs.output_))
		return false;

	if (!input_)
		return true;

	return Box::lessOrEqual(*input_, *rhs.input_);
}


bool Box::simplifiedLessThan(const Box& rhs) const
{
	if (static_cast<bool>(input_) != static_cast<bool>(rhs.input_))
		return false;

	if (input_)
	{
		if (inputIndex_ != rhs.inputIndex_)
			return false;
	}

	if (!Box::lessOrEqual(*output_, *rhs.output_))
		return false;

	if (!input_)
		return true;

	return Box::lessOrEqual(*input_, *rhs.input_);
}


void Box::initialize()
{
	selCoverage_.resize(arity_ + 1);

	Box::getDownwardCoverage(selCoverage_[0], *output_);

	assert(selCoverage_[0].size());

	order_ = *selCoverage_[0].begin();

	this->enumerateSelectorsAtLeaves(selCoverage_, *output_);

	selfReference_ = ConnectionGraph::containsCutpoint(outputSignature_, 0);

	if (!input_)
		return;

	assert(inputIndex_ < selCoverage_.size());

	Box::getDownwardCoverage(selCoverage_[inputIndex_ + 1], *input_);

	this->enumerateSelectorsAtLeaves(selCoverage_, *input_);
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

	os << " ) [" << box.outputSignature_ << "] ";

	TAWriter<label_type> writer(os);

	for (auto state : box.output_->getFinalStates())
		writer.writeState(state);

	writer.endl();
	writer.writeTransitions(*box.output_, writeStateF);

	if (!box.input_)
		return os;

	os << "===" << std::endl << "input " << box.inputIndex_ << " (";

	for(auto& s : box.inputCoverage(box.inputIndex_))
		os << " +" << s;

	os << " ) [" << box.inputSignature_ << "] ";

	for (auto state : box.input_->getFinalStates())
		writer.writeState(state);

	writer.endl();
	writer.writeTransitions(*box.input_, writeStateF);

	return os;
}

void Box::toStream(std::ostream& os) const
{
	os << name_ << '(' << arity_ << ")[in=";

	// coverage of the input port
	for (auto it = this->outputCoverage().cbegin(); it != this->outputCoverage().cend(); ++it)
	{
		if (this->outputCoverage().cbegin() != it)
		{
			os << ",";
		}

		os << *it;
	}

	// coverage of output ports
	for (size_t i = 0; i < this->outPortsNum(); ++i)
	{
		os << "; out" << i << "=";

		for (auto it = this->inputCoverage(i).cbegin(); it != this->inputCoverage(i).cend(); ++it)
		{
			if (this->inputCoverage(i).cbegin() != it)
			{
				os << ",";
			}

			os << *it;
		}
	}

	os << "]";
}
