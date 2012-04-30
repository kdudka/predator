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

#ifndef FOREST_AUT_H
#define FOREST_AUT_H

#include <vector>
#include <stdexcept>
#include <cassert>
#include <ostream>
#include <sstream>
#include <algorithm>
#include <memory>

#include <unordered_map>

#include "types.hh"
#include "treeaut.hh"
#include "tatimint.hh"
#include "label.hh"
#include "abstractbox.hh"
#include "connection_graph.hh"

class FA {

	friend class UFAE;

public:

	friend std::ostream& operator<<(std::ostream& os, const FA& fa) {

		os << '[';
		for (auto var : fa.variables)
			os << ' ' << var;
		os << " ]" << std::endl;

		for (size_t i = 0; i < fa.roots.size(); ++i) {

			if (!fa.roots[i])
				continue;

			os << "===" << std::endl << "root " << i << " [" << fa.connectionGraph.data[i] << ']';

			TAWriter<label_type> writer(os);

			for (auto state : fa.roots[i]->getFinalStates())
				writer.writeState(state);

			writer.endl();
			writer.writeTransitions(*fa.roots[i], FA::WriteStateF());

		}

		return os;
	}

public:

	TA<label_type>::Backend* backend;

	std::vector<Data> variables;
	std::vector<std::shared_ptr<TA<label_type> > > roots;

	mutable ConnectionGraph connectionGraph;

	struct BoxCmpF {

		bool operator()(const std::pair<const AbstractBox*, std::vector<size_t>>& v1,
			const std::pair<const AbstractBox*, std::vector<size_t>>& v2) {

			if (v1.first->isType(box_type_e::bTypeInfo))
				return !v2.first->isType(box_type_e::bTypeInfo);

			return v1.first->getOrder() < v2.first->getOrder();

		}

	};

	static size_t getLabelArity(std::vector<const AbstractBox*>& label) {

		size_t arity = 0;

		for (auto& box : label)
			arity += box->getArity();

		return arity;

	}

	static void reorderBoxes(std::vector<const AbstractBox*>& label, std::vector<size_t>& lhs) {

		assert(FA::getLabelArity(label) == lhs.size());

		std::vector<std::pair<const AbstractBox*, std::vector<size_t> > > tmp;
		std::vector<size_t>::iterator lhsBegin = lhs.end(), lhsEnd = lhs.begin();

		for (size_t i = 0; i < label.size(); ++i) {

			lhsBegin = lhsEnd;

			lhsEnd += label[i]->getArity();

			tmp.push_back(std::make_pair(label[i], std::vector<size_t>(lhsBegin, lhsEnd)));

		}

		std::sort(tmp.begin(), tmp.end(), BoxCmpF());

		lhs.clear();

		for (size_t i = 0; i < tmp.size(); ++i) {

			label[i] = tmp[i].first;

			lhs.insert(lhs.end(), tmp[i].second.begin(), tmp[i].second.end());

		}

	}

	TA<label_type>* allocTA() {
		return new TA<label_type>(*this->backend);
	}

public:

	static bool isData(size_t state) {
		return _MSB_TEST(state);
	}

	struct WriteStateF {

		std::string operator()(size_t state) const {
			std::ostringstream ss;
			if (_MSB_TEST(state))
				ss << 'r' << _MSB_GET(state);
			else
				ss << 'q' << state;
			return ss.str();
		}

	};

	FA(TA<label_type>::Backend& backend) : backend(&backend) {}

	FA(const FA& src) : backend(src.backend), variables(src.variables), roots(src.roots),
		connectionGraph(src.connectionGraph) { }

	void clear() {
		this->roots.clear();
		this->connectionGraph.clear();
		this->variables.clear();
	}

	size_t getRootCount() const {
		return this->roots.size();
	}

	const TA<label_type>* getRoot(size_t i) const {
		assert(i < this->roots.size());
		return this->roots[i].get();
	}

	void appendRoot(TA<label_type>* ta) {
		this->roots.push_back(std::shared_ptr<TA<label_type>>(ta));
	}

	void appendRoot(std::shared_ptr<TA<label_type>> ta) {
		this->roots.push_back(ta);
	}

	void updateConnectionGraph() const {

		this->connectionGraph.updateIfNeeded(this->roots);

	}

};

std::ostream& operator<<(std::ostream& os, const TA<label_type>& ta);

#endif
