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

// Standard library headers
#include <vector>
#include <stdexcept>
#include <cassert>
#include <ostream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <unordered_map>

// Forester headers
#include "types.hh"
#include "treeaut_label.hh"
#include "abstractbox.hh"
#include "connection_graph.hh"




/**
 * @brief  @todo
 */
class FA
{
public:   // data types


public:

	TreeAut::Backend* backend;

private:  // data members

	DataArray variables_;

private:  // static methods

	/**
	 * @brief  @todo
	 *
	 * @todo
	 */
	static size_t getLabelArity(std::vector<const AbstractBox*>& label)
	{
		size_t arity = 0;

		for (auto& box : label)
			arity += box->getArity();

		return arity;
	}


public:

	std::vector<std::shared_ptr<TreeAut>> roots;

	mutable ConnectionGraph connectionGraph;

	struct BoxCmpF {

		bool operator()(const std::pair<const AbstractBox*, std::vector<size_t>>& v1,
			const std::pair<const AbstractBox*, std::vector<size_t>>& v2) {

			if (v1.first->isType(box_type_e::bTypeInfo))
				return !v2.first->isType(box_type_e::bTypeInfo);

			return v1.first->getOrder() < v2.first->getOrder();

		}

	};

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

	TreeAut* allocTA() {
		return new TreeAut(*this->backend);
	}

protected:// methods

	void loadVars(const DataArray& data)
	{
		variables_ = data;
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

	FA(TreeAut::Backend& backend) : backend(&backend), variables_{}, roots{}, connectionGraph{} {}

	FA(const FA& src) : backend(src.backend), variables_(src.variables_), roots(src.roots),
		connectionGraph(src.connectionGraph) { }

	/**
	 * @brief  Assignment operator
	 */
	FA& operator=(const FA& src)
	{
		if (this != &src)
		{
			backend = src.backend;
			variables_ = src.variables_;
			roots = src.roots;
			connectionGraph = src.connectionGraph;
		}

		return *this;
	}

	void clear() {
		this->roots.clear();
		this->connectionGraph.clear();
		variables_.clear();
	}

	size_t getRootCount() const {
		return this->roots.size();
	}

	const TreeAut* getRoot(size_t i) const {
		assert(i < this->roots.size());
		return this->roots[i].get();
	}

	void appendRoot(TreeAut* ta) {
		this->roots.push_back(std::shared_ptr<TreeAut>(ta));
	}

	void appendRoot(std::shared_ptr<TreeAut> ta) {
		this->roots.push_back(ta);
	}

	void updateConnectionGraph() const {

		this->connectionGraph.updateIfNeeded(this->roots);

	}

	void PushVar(const Data& var)
	{
		return variables_.push_back(var);
	}

	void PopVar()
	{
		variables_.pop_back();
	}

	void AddNewVars(size_t count)
	{
		variables_.resize(variables_.size() + count, Data::createUndef());
	}

	size_t GetVarCount() const
	{
		return variables_.size();
	}

	const Data& GetVar(size_t varId) const
	{
		// Assertions
		assert(varId < variables_.size());

		return variables_[varId];
	}

	const DataArray& GetVariables() const
	{
		return variables_;
	}

	void SetVar(size_t varId, const Data& data)
	{
		// Assertions
		assert(varId < variables_.size());

		variables_[varId] = data;
	}

	void SetVarsToUndefForRoot(size_t root)
	{
		for (auto& var : variables_)
		{	// set every variable referencing 'root' to undef
			if (var.isRef(root))
				var = Data::createUndef();
		}
	}

	void UpdateVarsRootRefs(const std::vector<size_t>& index)
	{
		for (Data& var : variables_)
		{
			if (!var.isRef())
				continue;

			// Assertions
			assert(index.size() > var.d_ref.root);
			assert(index[var.d_ref.root] != static_cast<size_t>(-1));

			var.d_ref.root = index[var.d_ref.root];
		}
	}


	const Data& GetTopVar() const
	{
		// Assertions
		assert(!variables_.empty());

		return variables_.back();
	}

public:   // static methods

	friend std::ostream& operator<<(std::ostream& os, const FA& fa);

	/**
	 * @brief  Virtual destructor
	 */
	virtual ~FA()
	{ }
};

std::ostream& operator<<(std::ostream& os, const TreeAut& ta);

#endif
