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

        TObjId createObj(SymHeap *pSh, EObjKind kind) const;

        void nullFieldsOfObj(SymHeap *pSh, TObjId obj) const;

        void dropFieldsOfObj(SymHeap *pSh, TObjId obj) const;

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

void TplFactory::nullFieldsOfObj(SymHeap *pSh, const TObjId obj) const
{
    // obtain handles for next/prev fields
    const PtrHandle nextPtr(*pSh, obj, this->nextAt());
    const PtrHandle prevPtr(*pSh, obj, this->prevAt());

    // nullify the next/prev fields
    nextPtr.setValue(VAL_NULL);
    prevPtr.setValue(VAL_NULL);
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

enum EListSide {
    LS_INVALID,
    LS_FRONT,
    LS_BACK
};

void connectPush(
        SymHeap                    &sh,
        const TplFactory           &fact,
        const TObjId                dls,
        const TObjId                reg,
        const EListSide             side)
{
    TOffset offNext;
    TOffset offPrev;
    ETargetSpecifier ts;
    switch (side) {
        case LS_FRONT:
            offNext = fact.prevAt();
            offPrev = fact.nextAt();
            ts = TS_FIRST;
            break;

        case LS_BACK:
            offNext = fact.nextAt();
            offPrev = fact.prevAt();
            ts = TS_LAST;
            break;

        default:
            CL_BREAK_IF("invalid call of connectPush()");
            return;
    }

    // obtain handles for next/prev fields
    const PtrHandle nextPtr(sh, reg, offNext);
    const PtrHandle prevPtr(sh, reg, offPrev);
    const PtrHandle endPtr (sh, dls, offNext);

    // chain both objects together such that they represent a linked list
    const TValId regAt = sh.addrOfTarget(reg, TS_REGION, fact.headAt());
    const TValId endAt = sh.addrOfTarget(dls, ts,        fact.headAt());
    endPtr.setValue(regAt);
    prevPtr.setValue(endAt);
    nextPtr.setValue(VAL_NULL);
}

OpTemplate* createPushByRef(TplFactory &fact, const EListSide side)
{
    OpTemplate *tpl = new OpTemplate((side == LS_FRONT)
            ? "push_front_by_ref"
            : "push_back_by_ref");

    SymHeap sh(fact.createHeap());

    // allocate an uninitialized region
    const TObjId reg = fact.createObj(&sh, OK_REGION);
    SymHeap input(sh);
    Trace::waiveCloneOperation(input);

    // nullify the next/prev fields
    fact.nullFieldsOfObj(&sh, reg);

    // register pre/post pair for push_back() to an empty list
    SymHeap output(sh);
    Trace::waiveCloneOperation(output);
    OpFootprint *fp = new OpFootprint(input, output);
    fp->inArgs.push_back(reg);
    tpl->addFootprint(fp);

    // drop the nullified fields of 'reg'
    fact.dropFieldsOfObj(&sh, reg);

    // allocate a DLS that will represent a container shape in our template
    const TObjId dls = fact.createObj(&sh, OK_DLS);
    fact.nullFieldsOfObj(&sh, dls);
    input = sh;

    // chain both objects together such that they represent a linked list
    connectPush(sh, fact, dls, reg, side);
    output = sh;

    // register pre/post pair for push_back() to a non-empty list
    Trace::waiveCloneOperation(input);
    Trace::waiveCloneOperation(output);
    fp = new OpFootprint(input, output);
    fp->inArgs.push_back(reg);
    tpl->addFootprint(fp);

    return tpl;
}

OpTemplate* createPushByVal(TplFactory &fact, const EListSide side)
{
    OpTemplate *tpl = new OpTemplate((side == LS_FRONT)
            ? "push_front_by_val"
            : "push_back_by_val");

    SymHeap sh(fact.createHeap());
    SymHeap input(sh);
    Trace::waiveCloneOperation(input);

    // allocate an uninitialized region
    TObjId reg = fact.createObj(&sh, OK_REGION);

    // nullify the next/prev fields
    fact.nullFieldsOfObj(&sh, reg);

    // register pre/post pair for push_back() to an empty list
    SymHeap output(sh);
    Trace::waiveCloneOperation(output);
    OpFootprint *fp = new OpFootprint(input, output);
    fp->outArgs.push_back(reg);
    tpl->addFootprint(fp);

    // start with a fresh heap
    sh = fact.createHeap();
    Trace::waiveCloneOperation(sh);

    // allocate a DLS that will represent a container shape in our template
    const TObjId dls = fact.createObj(&sh, OK_DLS);
    fact.nullFieldsOfObj(&sh, dls);
    input = sh;

    // allocate an uninitialized region
    reg = fact.createObj(&sh, OK_REGION);

    // chain both objects together such that they represent a linked list
    connectPush(sh, fact, dls, reg, side);
    output = sh;

    // register pre/post pair for push_back() to a non-empty list
    Trace::waiveCloneOperation(input);
    Trace::waiveCloneOperation(output);
    fp = new OpFootprint(input, output);
    fp->outArgs.push_back(reg);
    tpl->addFootprint(fp);

    return tpl;
}

OpTemplate* createPop(TplFactory &fact, const EListSide side)
{
    TOffset offNext;
    TOffset offPrev;
    ETargetSpecifier ts;
    OpTemplate *tpl;

    switch (side) {
        case LS_FRONT:
            offNext = fact.prevAt();
            offPrev = fact.nextAt();
            ts = TS_FIRST;
            tpl = new OpTemplate("pop_front");
            break;

        case LS_BACK:
            offNext = fact.nextAt();
            offPrev = fact.prevAt();
            ts = TS_LAST;
            tpl = new OpTemplate("pop_back");
            break;

        default:
            CL_BREAK_IF("invalid call of createPop()");
            return new OpTemplate("pop_invalid");
    }

    // allocate a region
    SymHeap sh(fact.createHeap());
    const TObjId reg = fact.createObj(&sh, OK_REGION);

    // nullify the next/prev fields
    const PtrHandle regNext(sh, reg, offNext);
    const PtrHandle regPrev(sh, reg, offPrev);
    regNext.setValue(VAL_NULL);
    regPrev.setValue(VAL_NULL);

    // store the input heap
    SymHeap input(sh);
    Trace::waiveCloneOperation(input);

    // free the region
    sh.objInvalidate(reg);

    // register pre/post pair for pop_back() on a singleton list
    tpl->addFootprint(new OpFootprint(input, /* output */ sh));

    // roll-back the "free" operation
    sh.swap(input);

    // allocate a DLS that will represent a container shape in our template
    const TObjId dls = fact.createObj(&sh, OK_DLS);
    const PtrHandle dlsNext(sh, dls, fact.nextAt());
    const PtrHandle dlsPrev(sh, dls, fact.prevAt());

    // chain both objects together such that they represent a linked list
    const TValId regAt = sh.addrOfTarget(reg, TS_REGION, fact.headAt());
    const TValId endAt = sh.addrOfTarget(dls, ts,        fact.headAt());
    dlsPrev.setValue(VAL_NULL);
    regPrev.setValue(endAt);
    dlsNext.setValue(regAt);

    // store the input heap
    input = sh;
    Trace::waiveCloneOperation(input);

    // free the region and re-terminate the remainder of the list
    sh.objInvalidate(reg);
    dlsNext.setValue(VAL_NULL);

    // register pre/post pair for pop_back() on a 2+ list
    tpl->addFootprint(new OpFootprint(input, /* output */ sh));

    return tpl;
}

OpTemplate* createClear2(TplFactory &fact)
{
    OpTemplate *tpl = new OpTemplate("clear2");

    // create an empty heap as the output
    SymHeap sh(fact.createHeap());
    SymHeap output(sh);
    Trace::waiveCloneOperation(output);

    // allocate a pair of regions
    const TObjId reg1 = fact.createObj(&sh, OK_REGION);
    const TObjId reg2 = fact.createObj(&sh, OK_REGION);

    // acquire handles to their prev/next fields
    const PtrHandle reg1Prev(sh, reg1, fact.prevAt());
    const PtrHandle reg1Next(sh, reg1, fact.nextAt());
    const PtrHandle reg2Prev(sh, reg2, fact.prevAt());
    const PtrHandle reg2Next(sh, reg2, fact.nextAt());

    // connect them as a list
    reg1Prev.setValue(VAL_NULL);
    reg1Next.setValue(sh.addrOfTarget(reg2, TS_REGION, fact.headAt()));
    reg2Prev.setValue(sh.addrOfTarget(reg1, TS_REGION, fact.headAt()));
    reg2Next.setValue(VAL_NULL);

    // register pre/post pair for clear2()
    tpl->addFootprint(new OpFootprint(/* input */ sh, output));
    return tpl;
}

bool loadDefaultOperations(OpCollection *pDst, const CodeStorage::Storage &stor)
{
    if (stor.types.dataPtrSizeof() <= 0) {
        CL_BREAK_IF("loadDefaultOperations() failed to resolve sizeof(void *)");
        return false;
    }

    TplFactory fact(stor);
    for (int i = LS_FRONT; i <= LS_BACK; ++i) {
        const EListSide side = static_cast<EListSide>(i);
        pDst->addTemplate(createPushByRef(fact, side));
        pDst->addTemplate(createPushByVal(fact, side));
        pDst->addTemplate(createPop(fact, side));
    }
    pDst->addTemplate(createClear2(fact));
    return true;
}

} // namespace AdtOp
