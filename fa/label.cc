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
#include "box.hh"
#include "label.hh"


std::ostream& operator<<(std::ostream& os, const NodeLabel& label)
{
	os << '<';
	switch (label.type_) {
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

			const std::vector<SelData>* sels = label.node.sels;

			size_t selIndex = 0;

			for (auto itLb = label.node.v->cbegin(); itLb != label.node.v->cend(); ++itLb)
			{
				if (label.node.v->cbegin() != itLb)
					os << ',';

				const AbstractBox* box = *itLb;
				assert(nullptr != box);

				if ((box_type_e::bSel == box->getType()) && (nullptr != sels))
				{	// in case there is a selector box with selectors
					// assert the range is correct
					assert(selIndex < sels->size());

					SelBox tmpBox(&(*sels)[selIndex]);
					++selIndex;

					os << tmpBox;

					continue;
				}

				os << **itLb;
			}

			break;
		}
	}
	return os << '>';
}
