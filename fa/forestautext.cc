/*
 * Copyright (C) 2010 Jiri Simacek
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
#include "forestautext.hh"
#include "streams.hh"

bool FAE::subseteq(const FAE& lhs, const FAE& rhs)
{
	if (lhs.getRootCount() != rhs.getRootCount())
		return false;

	if (lhs.connectionGraph.data != rhs.connectionGraph.data)
		return false;

	for (size_t i = 0; i < lhs.getRootCount(); ++i)
	{
		if (!TreeAut::subseteq(*lhs.getRoot(i), *rhs.getRoot(i)))
			return false;
	}

	return true;
}
