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

#ifndef LAB_MAN_H
#define LAB_MAN_H

#include <vector>
#include <utility>

#include "cache.hh"
#include "types.hh"

class LabMan {
	
	Cache<std::pair<size_t, std::vector<Data> > > varStore;
	Cache<std::vector<const class Box*> > boxStore;

public:

	LabMan() {}
	
	const std::vector<Data>& lookup(const std::vector<Data>& x, size_t arity) {
		return this->varStore.lookup(std::make_pair(arity, x))->first.second;
	}

	const std::vector<const class Box*>& lookup(const std::vector<const class Box*>& x) {
		return this->boxStore.lookup(x)->first;
	}

};

#endif
