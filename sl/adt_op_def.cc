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

#include "config.h"
#include "adt_op_def.hh"

#include "adt_op.hh"
#include "symtrace.hh"

#include <cl/storage.hh>

namespace AdtOp {

class TplFactory {
    public:
        TplFactory(const CodeStorage::Storage &stor):
            stor_(stor),
            node_(new Trace::TransientNode("TplFactory")),
            ptrSize_(stor.types.dataPtrSizeof()),
            objSize_(IR::rngFromNum(2 * ptrSize_))
        {
            bOff_.head = 0;
            bOff_.next = 0;
            bOff_.prev = ptrSize_;
        }

        TOffset headAt() const {
            return bOff_.head;
        }

        TOffset nextAt() const {
            return bOff_.next;
        }

        TOffset prevAt() const {
            return bOff_.prev;
        }

        SymHeap createHeap() const {
            return SymHeap(stor_, node_.node());
        }

        TObjId createObj(SymHeap *pSh, const EObjKind kind) const;

        void dropFieldsOfObj(SymHeap *pSh, const TObjId obj) const;

    private:
        const CodeStorage::Storage         &stor_;
        const Trace::NodeHandle             node_;
        const TSizeOf                       ptrSize_;
        const TSizeRange                    objSize_;
        BindingOff                          bOff_;
};

TObjId TplFactory::createObj(SymHeap *pSh, const EObjKind kind) const
{
    const TObjId obj = pSh->heapAlloc(objSize_);
    if (OK_REGION != kind)
        pSh->objSetAbstract(obj, kind, bOff_);

    return obj;
}

void TplFactory::dropFieldsOfObj(SymHeap *pSh, const TObjId obj) const
{
    const TValId valUnknown = pSh->valCreate(VT_UNKNOWN, VO_ASSIGNED);

    const UniformBlock ubAll = {
        /* off      */  0,
        /* size     */  objSize_.lo,
        /* tplValue */  valUnknown
    };

    pSh->writeUniformBlock(obj, ubAll);
}

OpTemplate* createPushBack(TplFactory &fact)
{
    OpTemplate *tpl = new OpTemplate("push_back");

    SymHeap sh(fact.createHeap());

    // allocate an uninitialized region
    const TObjId reg = fact.createObj(&sh, OK_REGION);
    SymHeap input(sh);
    Trace::waiveCloneOperation(input);

    // obtain handles for next/prev fields
    const PtrHandle nextPtr(sh, reg, fact.nextAt());
    const PtrHandle prevPtr(sh, reg, fact.prevAt());

    // nullify the next/prev fields
    nextPtr.setValue(VAL_NULL);
    prevPtr.setValue(VAL_NULL);

    // register pre/post pair for push_back() to an empty list
    SymHeap output(sh);
    Trace::waiveCloneOperation(output);
    tpl->addFootprint(new OpFootprint(input, output));

    // drop the nullified fields of 'reg'
    fact.dropFieldsOfObj(&sh, reg);

    // allocate a DLS that will represent a container shape in our template
    const TObjId dls = fact.createObj(&sh, OK_DLS);
    const PtrHandle begPtr(sh, dls, fact.prevAt());
    const PtrHandle endPtr(sh, dls, fact.nextAt());
    begPtr.setValue(VAL_NULL);
    endPtr.setValue(VAL_NULL);
    input = sh;

    // chain both objects together such that they represent a linked list
    const TValId regAt = sh.addrOfTarget(reg, TS_REGION, fact.headAt());
    const TValId endAt = sh.addrOfTarget(dls, TS_LAST,   fact.headAt());
    endPtr.setValue(regAt);
    prevPtr.setValue(endAt);
    nextPtr.setValue(VAL_NULL);
    output = sh;

    // register pre/post pair for push_back() to a non-empty list
    tpl->addFootprint(new OpFootprint(input, output));

    return tpl;
}

void loadDefaultOperations(OpCollection *pDst, const CodeStorage::Storage &stor)
{
    TplFactory fact(stor);
    pDst->addTemplate(createPushBack(fact));
}

} // namespace AdtOp
