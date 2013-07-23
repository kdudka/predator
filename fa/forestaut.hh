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
#include "streams.hh"


/**
 * @brief  @todo
 */
class FA
{
public:   // data types

	typedef TreeAut::Transition Transition;

public:   // data members

	TreeAut::Backend* backend;

protected:// data members

	DataArray variables_;

	std::vector<std::shared_ptr<TreeAut>> roots_;

public:   // data members

	mutable ConnectionGraph connectionGraph;

public:   // methods


	/**
	 * @brief  Reorder boxes into the normal form
	 *
	 * This method reorders the passed vector of boxes (as well as the vector of
	 * states that are target states of the boxes) into the @e normal form.
	 *
	 * @param[in,out]  label  Vector of boxes to be reordered
	 * @param[in,out]  lhs    Vector of states corresponding to @p label
	 */
	static void reorderBoxes(
		std::vector<const AbstractBox*>&     label,
		std::vector<size_t>&                 lhs);


	/**
	 * @brief  Allocates a new TreeAut
	 *
	 * Allocates a new TreeAut using the backend of the FA.
	 *
	 * @returns  Pointer to a new TreeAut
	 */
	TreeAut* allocTA()
	{
		return new TreeAut(*this->backend);
	}


protected:// methods

	void loadVars(const DataArray& data)
	{
		variables_ = data;
	}

public:   // methods

	static bool isData(size_t state)
	{
		return _MSB_TEST(state);
	}

	FA(TreeAut::Backend& backend) :
		backend(&backend),
		variables_{},
		roots_{},
		connectionGraph{}
	{ }

	FA(const FA& src) :
		backend(src.backend),
		variables_(src.variables_),
		roots_(src.roots_),
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
			roots_ = src.roots_;
			connectionGraph = src.connectionGraph;
		}

		return *this;
	}

	void clear()
	{
		roots_.clear();
		this->connectionGraph.clear();
		variables_.clear();
	}

	size_t getRootCount() const
	{
		return roots_.size();
	}

	/**
	 * @brief  Gets the count of valid roots
	 *
	 * Retrieve the number of valid roots (i.e. not-@p nullptr roots).
	 *
	 * @returns  The count of valid roots
	 *
	 * @todo: cache?
	 */
	size_t getValidRootCount() const
	{
		size_t sum = 0;
		for (std::shared_ptr<TreeAut> rootPtr : this->getRoots())
		{
			if (nullptr != rootPtr) ++sum;
		}

		return sum;
	}

	const std::shared_ptr<TreeAut>& getRoot(size_t i) const
	{
		assert(i < this->getRootCount());
		return roots_[i];
	}

	std::shared_ptr<TreeAut>& getRoot(size_t i)
	{
		assert(i < this->getRootCount());
		return roots_[i];
	}

	void setRoot(size_t i, std::shared_ptr<TreeAut> ta)
	{
		assert(i < this->getRootCount());
		roots_[i] = ta;
	}

	void appendRoot(TreeAut* ta)
	{
		roots_.push_back(std::shared_ptr<TreeAut>(ta));
	}

	const std::vector<std::shared_ptr<TreeAut>>& getRoots() const
	{
		return roots_;
	}

	void appendRoot(std::shared_ptr<TreeAut> ta)
	{
		roots_.push_back(ta);
	}

	void resizeRoots(size_t newSize)
	{
		roots_.resize(newSize);
	}

	void swapRoots(std::vector<std::shared_ptr<TreeAut>> otherRoots)
	{
		roots_.swap(otherRoots);
	}


	void updateConnectionGraph() const
	{
		this->connectionGraph.updateIfNeeded(this->getRoots());
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

	bool Empty() const
	{
		return roots_.empty();
	}

	void SetVar(size_t varId, const Data& data)
	{
		// Assertions
		assert(varId < variables_.size());

		variables_[varId] = data;
	}

	/**
	 * @brief  Sets all variables referencing given root to undef
	 *
	 * @param[in]  root  The desired root reference
	 */
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


	static std::string writeState(size_t state)
	{
		std::ostringstream ss;
		if (_MSB_TEST(state))
			ss << 'r' << _MSB_GET(state);
		else
			ss << 'q' << state;

		return ss.str();
	}

	static std::string writeTransition(const Transition& trans);

	friend std::ostream& operator<<(std::ostream& os, const FA& fa);

	/**
	 * @brief  Virtual destructor
	 */
	virtual ~FA()
	{ }
};

/**
 * @brief  The output stream operator
 *
 * The std::ostream << operator for conversion of a TA to a string.
 *
 * @param[in,out]  os  The output stream
 * @param[in]      ta  The TA to be appended to the stream
 *
 * @returns  The modified output stream
 */
std::ostream& operator<<(std::ostream& os, const TreeAut& ta);

#endif
