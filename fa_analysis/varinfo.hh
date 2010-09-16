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

#include <ostream>

#include <boost/unordered_map.hpp>

using boost::hash_value;

struct var_info {
		
	size_t data;
	int aux;

	var_info(size_t data, int aux) : data(data), aux(aux) {}

	friend size_t hash_value(const var_info& v) {
		return hash_value(v.data + v.aux);
	}

	bool operator==(const var_info& rhs) const {
		return (this->data == rhs.data) && (this->aux == rhs.aux);
	}
/*	
	friend std::ostream& operator<<(std::ostream& os, const var_info& x) {
		os << "v:";
		switch (x.index) {
			case (size_t)(-1): return os << "null";
			case (size_t)(-2): return os << "undef";
			default: return os << x.index << '+' << x.offset;
		}
	}
*/
};

#endif
