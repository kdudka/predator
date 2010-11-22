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

#include "forestaut.hh"
#include "tatimint.hh"

struct WriteStateF {

	std::string operator()(size_t state) const {
		std::ostringstream ss;
		if (_MSB_TEST(state))
			ss << 'r' << _MSB_GET(state);
		else
			ss << 'q' << state;
		return ss.str();
	}

};

std::ostream& operator<<(std::ostream& os, const TA<label_type>& ta) {
	TAWriter<label_type>(os).writeOne(ta, WriteStateF());
	return os;
}
