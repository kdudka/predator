/*
 * Copyright (C) 2012  Ondrej Lengal
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

#ifndef _LINK_TREE_HH_
#define _LINK_TREE_HH_

// Standard library headers
#include <cassert>
#include <set>

/**
 * @brief  Base class for linking data structures in a tree hierarchy
 *
 * This class serves as the base class for linking another class in a tree
 * hierarchy (but can be also used separately).
 */
class LinkTree
{
public:   // data types

	typedef std::set<LinkTree*> TChildren;

private:  // data members

	/// The parent if there is no parent
	LinkTree* parent_;

	/// Children nodes
	/// @todo: try other container
	TChildren children_;

private:  // methods

	LinkTree(const LinkTree&);
	LinkTree& operator=(const LinkTree&);

protected:// methods 

	/**
	 * @brief  Sets the parent of a node
	 *
	 * Sets the parent of a node and also set the node as a child of the parent.
	 *
	 * @param[in]  parent  The parent of a node
	 */
	void setParent(LinkTree* parent)
	{
		parent_ = parent;

		if (nullptr != parent_)
		{
			parent_->addChild(this);
		}
	}

	/**
	 * @brief  Clears all children
	 */
	void clearChildren()
	{
		children_.clear();
	}

public:   // methods

	/**
	 * @brief  Default constructor
	 */
	LinkTree() :
		parent_(nullptr),
		children_()
	{ }

	/**
	 * @brief  Returns children of a node
	 *
	 * @returns  Children of a node
	 */
	const TChildren& GetChildren() const
	{
		return children_;
	}

	/**
	 * @brief  Returns the parent of a node
	 *
	 * @returns  The parent of a node
	 */
	LinkTree* GetParent()
	{
		return parent_;
	}

	/**
	 * @copydoc LinkTree::GetParent() const
	 */
	const LinkTree* GetParent() const
	{
		return parent_;
	}

	/**
	 * @brief  Add a child
	 *
	 * Adds a new child.
	 *
	 * @param[in]  child  The child to be added
	 */
	void addChild(LinkTree* child)
	{
		if (!children_.insert(child).second)
		{	// in case already present child was added
			assert(false);        // fail gracefully
		}
	}

	/**
	 * @brief  Remove a child
	 *
	 * Removes a child.
	 *
	 * @param[in]  child  The child to be removed
	 */
	void removeChild(LinkTree* child)
	{
		if (children_.erase(child) != 1)
		{	// in case the child to be removed is not present
			assert(false);        // fail gracefully
		}
	}

	/**
	 * @brief  Clears the tree links
	 */
	void clearTree()
	{
		parent_ = nullptr;
		children_.clear();
	}

	/**
	 * @brief  Destructor
	 */
	virtual ~LinkTree()
	{
		assert(children_.empty());
	}
};

#endif
