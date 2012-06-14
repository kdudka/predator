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

#ifndef SMART_SET_H
#define SMART_SET_H

#include <iostream>
#include <vector>
#include <list>
#include <stdexcept>

class SmartSet {

	class Iterator {
		std::list<std::pair<size_t, size_t> >::const_iterator _i;
	public:
		Iterator(std::list<std::pair<size_t, size_t> >::const_iterator i) : _i(i) {}

		Iterator& operator++() {
			return ++this->_i, *this;
		}

		Iterator operator++(int) {
			return Iterator(this->_i++);
		}
		
		int operator*() {
			return this->_i->first;
		}
		
		bool operator==(const Iterator& rhs) {
			return this->_i == rhs._i;
		}

		bool operator!=(const Iterator& rhs) {
			return this->_i != rhs._i;
		}

	};

public:

	typedef Iterator iterator;

private:

	std::list<std::pair<size_t, size_t> > _elements;
	std::vector<std::list<std::pair<size_t, size_t> >::iterator> _index;

public:

	SmartSet(size_t size = 0) : _elements{}, _index(size, _elements.end()) {}
	
	SmartSet(const SmartSet& x)
		: _elements(x._elements), _index(x._index.size(), _elements.end()) {
		for (std::list<std::pair<size_t, size_t> >::iterator i = this->_elements.begin(); i != this->_elements.end(); ++i)
			this->_index[i->first] = i;
	}
	
	SmartSet& operator=(const SmartSet& x) {
		this->_elements = x._elements;
		this->_index.resize(x._index.size(), this->_elements.end());
		for (std::list<std::pair<size_t, size_t> >::iterator i = this->_elements.begin(); i != this->_elements.end(); ++i)
			this->_index[i->first] = i;
		return *this;
	}

	SmartSet::iterator begin() const { return SmartSet::Iterator(this->_elements.begin()); }
	SmartSet::iterator end() const { return SmartSet::Iterator(this->_elements.end()); }
	
	void add(size_t x) {
		std::list<std::pair<size_t, size_t> >::iterator i = this->_index[x];
		if (i == this->_elements.end())
			this->_index[x] = this->_elements.insert(this->_elements.begin(), std::pair<size_t, size_t>(x, 1));
		else
			++i->second;
	}

	void remove(size_t x) {
		std::list<std::pair<size_t, size_t> >::iterator i = this->_index[x];
		if (i->second == 1) {
			this->_elements.erase(i);
			this->_index[x] = this->_elements.end();
		} else
			--i->second;
	}

	bool contains(size_t x) const {
		return (this->_index[x] != this->_elements.end());
	}
	
	size_t count(size_t x) const {
		std::list<std::pair<size_t, size_t> >::iterator i = this->_index[x];
		return (i != this->_elements.end())?(0):(i->second);
	}

	size_t size() const {
		return this->_elements.size();
	}

	void clear() {
		for (std::list<std::pair<size_t, size_t> >::iterator i = this->_elements.begin(); i != this->_elements.end(); ++i)
			this->_index[i->first] = this->_elements.end();
		this->_elements.clear();
	}
	
	void buildVector(std::vector<size_t>& v) const {
		v.clear();
		for (std::list<std::pair<size_t, size_t> >::const_iterator i = this->_elements.begin(); i != this->_elements.end(); ++i)
			v.push_back(i->first);
	}

	void dump() const {
		std::cout << "size: " << this->_index.size() << ", ";
		for (std::list<std::pair<size_t, size_t> >::const_iterator i = this->_elements.begin(); i != this->_elements.end(); ++i)
			std::cout << i->first << " ";
		std::cout << std::endl;
	}

};

#endif
