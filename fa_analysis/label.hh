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

#ifndef LABEL_H
#define LABEL_H

#include "types.hh"

struct label_type {

	size_t type;
		
	union {
		const std::vector<Data>* data;
		const std::vector<const class Box*>* dataB;
	};

	label_type(const std::vector<Data>* data) : type(0), data(data) {}
	label_type(const std::vector<const class Box*>* dataB) : type(1), dataB(dataB) {}

	friend size_t hash_value(const label_type& l) {
		using namespace boost;
		return hash_value(hash_value(l.type) + hash_value(l.data));
	}

	const class Box& head() const {
		assert(this->dataB);
		assert(this->dataB->size() > 0);
		return *(*this->dataB)[0];
	}
		
	bool operator<(const label_type& rhs) const { return this->data < rhs.data; }

	bool operator==(const label_type& rhs) const { return this->data == rhs.data; }

	friend std::ostream& operator<<(std::ostream& os, const label_type& label);

};

#endif
