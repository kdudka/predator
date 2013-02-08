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

#ifndef ABSTRACT_BOX_H
#define ABSTRACT_BOX_H

// Standard library headers
#include <ostream>
#include <set>


enum class box_type_e
{
	bBox,       ///< type of box encapsulating nested forest automaton
	bTypeInfo,  ///< type of box encapsulating type info
	bSel        ///< type of box used for selectors
};


class AbstractBox
{
private:  // data members

	box_type_e type_;

protected:// data members

	size_t arity_;
	size_t order_;

protected:// methods

	AbstractBox(box_type_e type, size_t arity) :
		type_(type),
		arity_(arity),
		order_(0)
	{ }

public:

	box_type_e getType() const
	{
		return type_;
	}

	bool isType(box_type_e type) const
	{
		return type_ == type;
	}

	bool isStructural() const
	{
		return (type_ == box_type_e::bBox) || (type_ == box_type_e::bSel);
	}

	bool isBox() const
	{
		return type_ == box_type_e::bBox;
	}

	bool isSelector() const
	{
		return box_type_e::bSel == type_;
	}

	size_t getArity() const
	{
		return arity_;
	}

	size_t getOrder() const
	{
		return order_;
	}

	virtual void toStream(std::ostream& os) const = 0;

	friend std::ostream& operator<<(std::ostream& os, const AbstractBox& rhs)
	{
		rhs.toStream(os);
		return os;
	}

	/**
	 * @brief  Virtual destructor
	 */
	virtual ~AbstractBox()
	{ }
};

class StructuralBox : public AbstractBox
{
protected:// methods

	StructuralBox(box_type_e type, size_t arity) :
		AbstractBox(type, arity)
	{ }

public:

	virtual bool outputCovers(size_t offset) const = 0;

	virtual const std::set<size_t>& outputCoverage() const = 0;

	virtual const std::set<size_t>& inputCoverage(size_t input) const = 0;

	virtual size_t selectorToInput(size_t input) const = 0;

	virtual size_t getSelCount(size_t input) const = 0;

	// returns (size_t)(-1) if not
	virtual size_t outputReachable(size_t input) const = 0;

	/**
	 * @brief  Virtual destructor
	 */
	virtual ~StructuralBox()
	{ }
};

#endif
