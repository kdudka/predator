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

#ifndef FOREST_AUT_H
#define FOREST_AUT_H

#include <vector>
#include <stdexcept>
#include <cassert>
#include <ostream>

#include "types.hh"
#include "treeaut.hh"
#include "label.hh"
#include "labman.hh"

class FA {

	friend class UFAE;

protected:

	std::vector<Data> variables;
	std::vector<TA<label_type>*> roots;
	mutable TA<label_type>::Manager* taMan;

public:

	FA(TA<label_type>::Manager& taMan) : taMan(&taMan) {}
	
	FA(const FA& src) : variables(src.variables), roots(src.roots), taMan(src.taMan) {
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			this->taMan->addRef(*i);
	}

	~FA() { this->clear(); }
	
	FA& operator=(const FA& x) {
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			this->taMan->release(*i);
		this->variables = x.variables;
		this->roots = x.roots;
		this->taMan = x.taMan;
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			this->taMan->addRef(*i);
		return *this;		
	}
	
	void clear() {
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			this->taMan->release(*i);
		this->roots.clear();
		this->variables.clear();
	}
	
};

std::ostream& operator<<(std::ostream& os, const TA<label_type>& ta);

#endif
