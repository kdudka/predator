/*
 * Copyright (C) 2012 Jiri Simacek
 *
 * This file is part of forester.
 *
 * forester is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * forester is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with forester.  If not, see <http://www.gnu.org/licenses/>.
 */

// Forester headers
#include "integrity.hh"


bool Integrity::check() const
{
	std::vector<bool> bitmap(this->fae.roots.size(), false);
	std::map<std::pair<const TreeAut*, size_t>, std::set<size_t>> states;

	for (size_t i = 0; i < this->fae.roots.size(); ++i)
	{
		if (!this->fae.roots[i])
			continue;

		if (!this->checkRoot(i, bitmap, states))
		{
			CL_CDEBUG(1, "inconsistent heap: " << std::endl << this->fae);

			return false;
		}
	}

	return true;
}
