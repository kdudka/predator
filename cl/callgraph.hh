/*
 * Copyright (C) 2012 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_CALLGRAPH_H
#define H_GUARD_CALLGRAPH_H

/**
 * @file callgraph.hh
 * @todo some dox
 */

namespace CodeStorage {

struct Storage;

namespace CallGraph {

void buildCallGraph(const Storage &);

} // namespace CallGraph

} // namespace CodeStorage

#endif /* H_GUARD_CALLGRAPH_H */
