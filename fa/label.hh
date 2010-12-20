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
#include "abstractbox.hh"

struct label_type {

	size_t type;
		
	union {
		const std::vector<Data>* data;
		const std::vector<const AbstractBox*>* dataB;
	};

	label_type() : type(0), data(NULL) {}
	label_type(const std::vector<Data>* data) : type(1), data(data) {}
	label_type(const std::vector<const AbstractBox*>* dataB) : type(2), dataB(dataB) {}

	friend size_t hash_value(const label_type& l) {
		size_t h = boost::hash_value(l.type);
		boost::hash_combine(h, l.data);
		return h;
	}

	const AbstractBox* head() const {
		assert(this->type == 2);
		assert(this->dataB);
		assert(this->dataB->size() > 0);
		return (*this->dataB)[0];
	}
		
	bool operator<(const label_type& rhs) const {
		return this->data < rhs.data;
	}

	bool operator==(const label_type& rhs) const { return this->data == rhs.data; }

	bool operator!=(const label_type& rhs) const { return this->data != rhs.data; }

	friend std::ostream& operator<<(std::ostream& os, const label_type& label) {
		os << '<';
		switch (label.type) {
			case 1: {
				assert(label.data);
				assert(!label.data->empty());
				std::vector<Data>::const_iterator i = label.data->begin();
				os << *i;
				for (++i; i != label.data->end(); ++i)
					os << ',' << *i;
				break;
			}
			case 2: {
				assert(label.dataB);
				assert(!label.dataB->empty());
				std::vector<const AbstractBox*>::const_iterator i = label.dataB->begin();
				os << **i;
				for (++i; i != label.dataB->end(); ++i)
					os << ',' << **i;
				break;
			}
		}
		return os << '>';
	}

};

#endif
