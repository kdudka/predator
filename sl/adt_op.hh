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
#include "shape.hh"

#include <utility>                          // for std::pair

namespace AdtOp {

/// (preferred) search direction
enum ESearchDirection {
    SD_INVALID = 0,
    SD_FORWARD,                             /// search forward along the CFG
    SD_BACKWARD                             /// search backward along the CFG
};

typedef int                                         TTemplateIdx;
typedef int                                         TFootprintIdx;
typedef std::pair<TTemplateIdx, TFootprintIdx>      TFootprintIdent;

/// operation footprint given by an input/output template pair
struct OpFootprint {
    SymHeap                     input;
    SymHeap                     output;

    OpFootprint(const SymHeap &input_, const SymHeap &output_):
        input(input_),
        output(output_)
    {
    }
};

/// an operation (push_back, erase, ...) template we can match by footprints
class OpTemplate {
    public:
        /// @name a human-readable name of the operation
        OpTemplate(const std::string &name):
            name_(name),
            dirty_(false)
        {
        }

        const std::string& name() const {
            return name_;
        }

        TFootprintIdx size() const {
            return fList_.size();
        }

        const OpFootprint& operator[](TFootprintIdx idx) const {
            return *fList_[idx];
        }

        /// takes ownership of *footprint and will release it on destruction
        void addFootprint(OpFootprint *footprint) {
            fList_.append(footprint);
            dirty_ = true;
        }

        const TShapeListByHeapIdx& inShapes() const {
            this->updateMetaIfNeeded();
            return inShapes_;
        }

        const TShapeListByHeapIdx& outShapes() const {
            this->updateMetaIfNeeded();
            return outShapes_;
        }

        ESearchDirection searchDirection() const {
            this->updateMetaIfNeeded();
            return searchDirection_;
        }

        void plot() const;

    private:
        const std::string               name_;
        CleanList<OpFootprint>          fList_;
        mutable bool                    dirty_;
        mutable TShapeListByHeapIdx     inShapes_;
        mutable TShapeListByHeapIdx     outShapes_;
        mutable ESearchDirection        searchDirection_;

        void updateMetaIfNeeded() const;
};

/// collection of operation templates
class OpCollection {
    public:
        TTemplateIdx size() const {
            return tList_.size();
        }

        const OpTemplate& operator[](TTemplateIdx idx) const {
            return *tList_[idx];
        }

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
