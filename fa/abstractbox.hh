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

#include <ostream>
#include <set>

typedef enum { bBox, bHead, bTypeInfo, bSel } box_type_e;

class AbstractBox {

	box_type_e type;

protected:

	size_t arity;
	size_t order;

	AbstractBox(box_type_e type, size_t arity) : type(type), arity(arity), order(0) {
	}

public:

	box_type_e getType() const {
		return this->type;
	}

	bool isType(box_type_e type) const {
		return this->type == type;
	}

	bool isStructural() const {
		return (this->type == box_type_e::bBox) || (this->type == box_type_e::bSel);
	}

	bool isBox() const {
		return this->type == box_type_e::bBox;
	}

	size_t getArity() const {
		return this->arity;
	}

	size_t getOrder() const {
		return this->order;
	}

	virtual void toStream(std::ostream& os) const = 0;

	friend std::ostream& operator<<(std::ostream& os, const AbstractBox& rhs) {
		rhs.toStream(os);
		return os;
	}

};

class StructuralBox : public AbstractBox {

protected:

	StructuralBox(box_type_e type, size_t arity) : AbstractBox(type, arity) {
	}

public:

	virtual bool outputCovers(size_t offset) const = 0;

	virtual const std::set<size_t>& outputCoverage() const = 0;

	virtual const std::set<size_t>& inputCoverage(size_t input) const = 0;

	virtual size_t selectorToInput(size_t input) const = 0;

	virtual size_t getRealRefCount(size_t input) const = 0;

	// returns (size_t)(-1) if not
	virtual size_t outputReachable(size_t input) const = 0;

};

#endif
