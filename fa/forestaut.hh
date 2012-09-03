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

public:

	std::vector<std::shared_ptr<TreeAut>> roots;

	mutable ConnectionGraph connectionGraph;

	static void reorderBoxes(
		std::vector<const AbstractBox*>& label,
		std::vector<size_t>& lhs);

	TreeAut* allocTA()
	{
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

	FA(TreeAut::Backend& backend) :
		backend(&backend),
		variables_{},
		roots{},
		connectionGraph{}
	{ }

	FA(const FA& src) :
		backend(src.backend),
		variables_(src.variables_),
		roots(src.roots),
		connectionGraph(src.connectionGraph)
	{ }

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

	void clear()
	{
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

	/**
	 * @brief  Run a visitor on the instance
	 *
	 * This method is the @p accept method of the Visitor design pattern.
	 *
	 * @param[in]  visitor  The visitor of the type @p TVisitor
	 *
	 * @tparam  TVisitor  The type of the visitor
	 */
	template <class TVisitor>
	void accept(TVisitor& visitor) const
	{
		visitor(*this);
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
