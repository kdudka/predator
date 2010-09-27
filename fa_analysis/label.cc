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

#include "label.hh"
#include "box.hh"

std::ostream& operator<<(std::ostream& os, const label_type& label) {
	os << '<';
	switch (label.type) {
		case 0: {
			assert(!label.data->empty());
			std::vector<Data>::const_iterator i = label.data->begin();
			os << *i;
			for (++i; i != label.data->end(); ++i)
				os << ',' << *i;
			break;
		}
		case 1: {
			assert(!label.dataB->empty());
			std::vector<const class Box*>::const_iterator i = label.dataB->begin();
			os << **i;
			for (++i; i != label.dataB->end(); ++i)
				os << ',' << **i;
			break;
		}
	}
	return os << '>';
}
