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

#ifndef RELATION_H
#define RELATION_H

#include <vector>
#include <algorithm>

class Relation {

	std::vector<std::vector<bool> > _data;
	size_t _index;

public:

	Relation(size_t initialSize = 16)
		: _data(initialSize, std::vector<bool>(initialSize, true)), _index(0) {}

	void reset() {
		for (std::vector<std::vector<bool> >::iterator i = this->_data.begin(); i != this->_data.end(); ++i)
			std::fill(i->begin(), i->end(), true);
		this->_index = 0;
	}

	size_t newEntry() {
		if (this->_index == this->_data.size()) {
			for (std::vector<std::vector<bool> >::iterator i = this->_data.begin(); i != this->_data.end(); ++i)
				i->resize(2*this->_data.size(), true);
	  	    this->_data.resize(2*this->_data.size(), std::vector<bool>(2*this->_data.size(), true));
	  	}
		return this->_index++;
	}

	std::vector<std::vector<bool> >& data() {
		return this->_data;
	}
	
	const std::vector<std::vector<bool> >& data() const {
		return this->_data;
	}

	void load(const std::vector<std::vector<bool> >& src) {
		this->_data = src;
		this->_index = this->_data.size();
	}
	
	void store(std::vector<std::vector<bool> >& dst, size_t size) const {
		dst.resize(size);
		for (size_t i = 0; i < size; ++i) {
			dst[i].resize(size);
			for (size_t j = 0; j < size; ++j) {
				dst[i][j] = this->_data[i][j];
			}
		}
	}	

	void dump() const {
		for (size_t i = 0; i < this->_index; ++i) {
			for (size_t j = 0; j < this->_index; ++j) 
				std::cout << (this->_data[i][j]?1:0);
			std::cout << std::endl;
		}
	}

};

#endif
