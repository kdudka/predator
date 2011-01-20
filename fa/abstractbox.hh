/*
 * Copyright (C) 2010 Jiri Simacek
 *
 * This file is part of predator.
 *
 * predator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * predator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with predator.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ABSTRACT_BOX_H
#define ABSTRACT_BOX_H

#include <ostream>

typedef enum { bBox, bTypeInfo, bSel/*, bData*/ } box_type_e;

class AbstractBox {

	box_type_e type;

protected:

	AbstractBox(box_type_e type) : type(type) {
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
/*
	bool isData() const {
		return this->type == box_type_e::bData;
	}
*/
	virtual size_t getArity() const {
		return 0;
	}

	virtual void toStream(std::ostream& os) const = 0;

	friend std::ostream& operator<<(std::ostream& os, const AbstractBox& rhs) {
		rhs.toStream(os);
		return os;
	}

};

#endif
