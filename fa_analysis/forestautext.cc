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

#include "forestautext.hh"

std::ostream& operator<<(std::ostream& os, const FAE& fae) {
	TA<label_type>* ta = fae.taMan->alloc();
	os << "variables:";
	for (std::vector<Data>::const_iterator i = fae.variables.begin(); i != fae.variables.end(); ++i)
		os << ' ' << *i;
	os << std::endl << "roots:" << std::endl;
	for (size_t i = 0; i < fae.roots.size(); ++i) {
		std::ostringstream ss;
		ss << "root" << i << '_';
		for (size_t j = 0; j < fae.rootMap[i].size(); ++j)
			ss << fae.rootMap[i][j];
		ta->clear();
		fae.roots[i]->minimized(*ta);
		TAWriter<label_type>(os).writeOne(*ta, ss.str());
//		TAWriter<label_type>(os).writeOne(*fae.roots[i], ss.str());
	}
	fae.taMan->release(ta);
	return os;
}
