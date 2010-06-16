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

#ifndef VAR_INFO_H
#define VAR_INFO_H

#include <boost/unordered_map.hpp>

using boost::hash_value;

struct var_info {
		
	size_t index;
	size_t offset;

	var_info(size_t index, size_t offset) : index(index), offset(offset) {}
//	var_info(const var_info& x) : index(x.index), offset(x.offset) {}

	friend size_t hash_value(const var_info& v) {
		return hash_value(v.index + v.offset);
	}

	bool operator==(const var_info& rhs) const {
		return (this->index == rhs.index) && (this->offset == rhs.offset);
	}

};

#endif
