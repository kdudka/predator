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

#include "forestaut.hh"
#include "streams.hh"
#include "tatimint.hh"

// anonymous namespace
namespace
{
size_t getLabelArity(std::vector<const AbstractBox*>& label)
{
	size_t arity = 0;

	for (auto& box : label)
		arity += box->getArity();

	return arity;
}
} // namespace


void FA::reorderBoxes(
	std::vector<const AbstractBox*>&     label,
	std::vector<size_t>&                 lhs)
{
	// local structure
	struct BoxCmpF
	{
		bool operator()(
			const std::pair<const AbstractBox*, std::vector<size_t>>& v1,
			const std::pair<const AbstractBox*, std::vector<size_t>>& v2)
		{
			if (v1.first->isType(box_type_e::bTypeInfo))
				return !v2.first->isType(box_type_e::bTypeInfo);

			return v1.first->getOrder() < v2.first->getOrder();
		}
	};

	// Assertions
	assert(getLabelArity(label) == lhs.size());

	// vector for holding pairs <box, vector of corresponding states>
	std::vector<std::pair<const AbstractBox*, std::vector<size_t>>> tmp;

	std::vector<size_t>::iterator lhsBegin;
	std::vector<size_t>::iterator lhsEnd = lhs.begin();

	for (size_t i = 0; i < label.size(); ++i)
	{	// fill the 'tmp' vector
		lhsBegin = lhsEnd;  // beginning is the end from the last iteration

		lhsEnd += label[i]->getArity();

		tmp.push_back(std::make_pair(label[i], std::vector<size_t>(lhsBegin, lhsEnd)));
	}

	// sort the boxes into a normal form
	std::sort(tmp.begin(), tmp.end(), BoxCmpF());

	lhs.clear();

	for (size_t i = 0; i < tmp.size(); ++i)
	{	// reorder the vectors according to the normal form
		label[i] = tmp[i].first;

		lhs.insert(lhs.end(), tmp[i].second.begin(), tmp[i].second.end());
	}
}


std::string FA::writeTransition(const Transition& trans)
{
	std::ostringstream oss;
	TimbukWriter writer(oss);

	std::ostringstream tmp;
	tmp << trans.label();

	writer.writeTransition(trans.lhs(), tmp.str(), trans.rhs(), FA::writeState);

	return oss.str();
}


std::ostream& operator<<(std::ostream& os, const TreeAut& ta)
{
	TAWriter<label_type> writer(os);
	os << '[';

	for (const size_t& state : ta.getFinalStates())
		writer.writeState(state);

	os << " ]" << std::endl;;
	writer.writeTransitions(ta, FA::writeState);
	return os;
}


std::ostream& operator<<(std::ostream& os, const FA& fa)
{
	os << '[';
	for (const Data& var : fa.variables_)
	{
		os << ' ' << var;
	}
	os << " ]" << std::endl;

	for (size_t i = 0; i < fa.getRootCount(); ++i)
	{
		if (!fa.getRoot(i))
			continue;

		os << "===" << std::endl << "root " << i; // no cutpoint info
		//os << "===" << std::endl << "root " << i << " [" << fa.connectionGraph.data[i] << ']';

		TAWriter<label_type> writer(os);

		for (size_t state : fa.getRoot(i)->getFinalStates())
		{
			writer.writeState(state);
		}

		writer.endl();
		writer.writeTransitions(*fa.getRoot(i), FA::writeState);
	}

	return os;
}
