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

#ifndef NODE_BUILDER_H
#define NODE_BUILDER_H

#include <vector>
#include <cl/code_listener.h>

#include "types.hh"

struct NodeBuilder {

	static void buildNode(std::vector<SelData>& nodeInfo, const cl_type* type, int offset = 0) {
		
		assert(type->size > 0);

		switch (type->code) {
			case cl_type_e::CL_TYPE_STRUCT:
				for (int i = 0; i < type->item_cnt; ++i)
					NodeBuilder::buildNode(nodeInfo, type->items[i].type, offset + type->items[i].offset);
				break;
			case cl_type_e::CL_TYPE_PTR:
			case cl_type_e::CL_TYPE_INT:
			case cl_type_e::CL_TYPE_BOOL:
			default:
				nodeInfo.push_back(SelData(offset, type->size, 0));
				break;
//			case cl_type_e::CL_TYPE_UNION:
		}

	}

	static void buildNode(std::vector<size_t>& nodeInfo, const cl_type* type, int offset = 0) {
		
		assert(type->size > 0);

		switch (type->code) {
			case cl_type_e::CL_TYPE_STRUCT:
				for (int i = 0; i < type->item_cnt; ++i)
					NodeBuilder::buildNode(nodeInfo, type->items[i].type, offset + type->items[i].offset);
				break;
			case cl_type_e::CL_TYPE_PTR:
			case cl_type_e::CL_TYPE_INT:
			case cl_type_e::CL_TYPE_BOOL:
			default:
				nodeInfo.push_back(offset);
				break;
//			case cl_type_e::CL_TYPE_UNION:
		}

	}

};

#endif
