/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_SYMPATH_H
#define H_GUARD_SYMPATH_H

/**
 * @file sympath.hh
 * PathTracer - SymStateMap based implementation of the IPathTracer interface
 */

#include "symbt.hh"

class SymStateMap;

namespace CodeStorage {
    class Block;
}

/// SymStateMap based implementation of the IPathTracer interface
class PathTracer: public IPathTracer {
    public:
        /**
         * @param smap instance of SymStateMap to read the paths from
         * @attention the SymStateMap object has to stay valid (and allocated at
         * the same address) until the destruction of PathTracer
         */
        PathTracer(SymStateMap &smap):
            smap_(smap),
            block_(0)
        {
        }

        /// set the basic block that is currently being executed by symexec
        void setBlock(const CodeStorage::Block *block) {
            block_ = block;
        }

        virtual void printPaths() const;

    private:
        SymStateMap                 &smap_;
        const CodeStorage::Block    *block_;
};


#endif /* H_GUARD_SYMPATH_H */
