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
#include "label.hh"


std::ostream& operator<<(std::ostream& os, const NodeLabel& label)
{
	os << '<';
	switch (label.type) {
		case NodeLabel::node_type::n_unknown:
			os << "unknown";
			break;
		case NodeLabel::node_type::n_data:
			assert(label.data.data);
			os << *label.data.data;
			break;
		case NodeLabel::node_type::n_vData: {
			assert(label.vData);
			assert(label.vData->size());
			DataArray::const_iterator i = label.vData->begin();
			os << *i;
			for (++i; i != label.vData->end(); ++i)
				os << ',' << *i;
			break;
		}
		case NodeLabel::node_type::n_node: {
			assert(label.node.v);
			if (label.node.v->empty())
				break;

			for (auto it = label.node.v->cbegin(); it != label.node.v->cend(); ++it)
			{
				if (it != label.node.v->cbegin())
					os << ',';

				os << **it;
			}

			break;
		}
	}
	return os << '>';
}
