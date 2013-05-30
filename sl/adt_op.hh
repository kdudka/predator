/*
 * Copyright (C) 2013 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_ADT_OP_H
#define H_GUARD_ADT_OP_H

#include "clean_list.hh"
#include "symheap.hh"

namespace AdtOp {

/// (preferred) search direction
enum ESearchDirection {
    SD_INVALID = 0,
    SD_FORWARD,                             /// search forward along the CFG
    SD_BACKWARD                             /// search backward along the CFG
};

/// operation footprint given by an input/output template pair
class OpFootprint {
    public:
        OpFootprint(const SymHeap &input, const SymHeap &output);

        const SymHeap& input() const {
            return input_;
        }

        const SymHeap& output() const {
            return output_;
        }

        ESearchDirection searchDirection() const {
            return searchDirection_;
        }

    private:
        SymHeap                 input_;
        SymHeap                 output_;
        ESearchDirection        searchDirection_;
};

/// an operation (push_back, erase, ...) template we can match by footprints
class OpTemplate {
    public:
        /// @name a human-readable name of the operation
        OpTemplate(const std::string &name):
            name_(name)
        {
        }

        /// takes ownership of *footprint and will release it on destruction
        void addFootprint(OpFootprint *footprint) {
            fList_.append(footprint);
        }

        void plot() const;

    private:
        const std::string       name_;
        CleanList<OpFootprint>  fList_;
};

/// collection of operation templates
class OpCollection {
    public:
        /// takes ownership of *tpl and will release it on destruction
        void addTemplate(OpTemplate *tpl) {
            tList_.append(tpl);
        }

        void plot() const;

    private:
        CleanList<OpTemplate>   tList_;
};

} // namespace AdtOp

#endif /* H_GUARD_ADT_OP_H */
