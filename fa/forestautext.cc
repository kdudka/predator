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

#include <string>
#include <sstream>

#include "forestautext.hh"

std::ostream& operator<<(std::ostream& os, const FAE& fae) {
//	TA<label_type>::Backend backend;
//	TA<label_type> ta(backend);
/*	os << "vars";
	for (std::vector<Data>::const_iterator i = fae.variables.begin(); i != fae.variables.end(); ++i)
		os << ' ' << *i;
	os << std::endl;*/
	for (size_t i = 0; i < fae.roots.size(); ++i) {
		if (!fae.roots[i])
			continue;
		os << "===" << std::endl << "root " << i << " o=[";
		for (size_t j = 0; j < fae.rootMap[i].size(); ++j)
			os << fae.rootMap[i][j];
		os << ']';
		TA<label_type>& ta = *fae.roots[i];
//		ta.clear();
//		fae.roots[i]->minimized(ta);
		TAWriter<label_type> writer(os);
		for (std::set<size_t>::iterator j = ta.getFinalStates().begin(); j != ta.getFinalStates().end(); ++j)
			writer.writeState(*j);
		writer.endl();
		writer.writeTransitions(ta, WriteStateF());
//		TAWriter<label_type>(os).writeOne(*ta, ss.str());
//		TAWriter<label_type>(os).writeOne(*fae.roots[i], ss.str());
	}
	return os;
}
