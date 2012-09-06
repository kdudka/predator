/*
 * Copyright (C) 2010-2011 Kamil Dudka <kdudka@redhat.com>
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
#include "symplot.hh"

#include <cl/cl_msg.hh>
#include <cl/clutil.hh>
#include <cl/storage.hh>

#include "plotenum.hh"
#include "symheap.hh"
#include "sympred.hh"
#include "symseg.hh"
#include "util.hh"
#include "worklist.hh"

#include <cctype>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <string>

#include <boost/foreach.hpp>

// /////////////////////////////////////////////////////////////////////////////
// implementation of plotHeap()
struct PlotData {
    typedef std::map<TValId, bool /* isRoot */>             TValues;
    typedef std::map<TValId, ObjList>                       TLiveObjs;
    typedef std::pair<int /* ID */, TValId>                 TDangVal;
    typedef std::vector<TDangVal>                           TDangValues;

    SymHeap                             &sh;
    std::ostream                        &out;
    int                                 last;
    TValues                             values;
    TLiveObjs                           liveObjs;
    TDangValues                         dangVals;

    PlotData(const SymHeap &sh_, std::ostream &out_):
        sh(const_cast<SymHeap &>(sh_)),
        out(out_),
        last(0)
    {
    }
};

void dlSegJumpToBegIfNeeded(const SymHeap &sh, TValId *pRoot)
{
    const TValId root = *pRoot;
    if (isDlSegPeer(sh, root))
        *pRoot = dlSegPeer(sh, root);
}

#define SL_QUOTE(what) "\"" << what << "\""

void digValues(PlotData &plot, const TValList &startingPoints, bool digForward)
{
    SymHeap &sh = plot.sh;

    WorkList<TValId> todo;
    BOOST_FOREACH(const TValId val, startingPoints)
        if (0 < val)
            todo.schedule(val);

    TValId val;
    while (todo.next(val)) {
        // insert the value itself
        plot.values[val] = /* isRoot */ false;
        const EValueTarget code = sh.valTarget(val);
        if (!isPossibleToDeref(code))
            // target is not an object
            continue;

        // check the root
        const TValId root = sh.valRoot(val);
        plot.values[root] = /* isRoot */ true;
        if (root != val && todo.seen(root))
            // this root was already traversed
            continue;

        if (!digForward)
            continue;

        // traverse the root
        ObjList liveObjs;
        sh.gatherLiveObjects(liveObjs, root);
        BOOST_FOREACH(const ObjHandle &obj, liveObjs) {
            const TValId valInside = obj.value();
            if (0 < valInside)
                // schedule the value inside for processing
                todo.schedule(valInside);
        }
    }
}

inline const char* offPrefix(const TOffset off)
{
    return (off < 0)
        ? ""
        : "+";
}

#define SIGNED_OFF(off) offPrefix(off) << (off)

inline void appendLabelIf(std::ostream &str, const char *label)
{
    if (!label)
        return;

    str << ", label=\"" << label << "\"";
}

void plotOffset(PlotData &plot, const TOffset off, const int from, const int to)
{
    const char *color = (off < 0)
        ? "red"
        : "black";

    plot.out << "\t"
#if SYMPLOT_FLAT_MODE
        << SL_QUOTE(to) << " -> " << SL_QUOTE("obj" << from)
#else
        << SL_QUOTE(from) << " -> " << SL_QUOTE(to)
#endif
        << " [color=" << color
        << ", fontcolor=" << color
        << ", label=\"[" << SIGNED_OFF(off)
        << "]\"];\n";
}

class CltFinder {
    private:
        const TObjType          cltRoot_;
        const TObjType          cltToSeek_;
        const TOffset           offToSeek_;
        TFieldIdxChain          icFound_;

    public:
        CltFinder(TObjType cltRoot, TObjType cltToSeek, TOffset offToSeek):
            cltRoot_(cltRoot),
            cltToSeek_(cltToSeek),
            offToSeek_(offToSeek)
        {
        }

        const TFieldIdxChain& icFound() const { return icFound_; }

        bool operator()(const TFieldIdxChain &ic, const struct cl_type_item *it)
        {
            const TObjType clt = it->type;
            if (clt != cltToSeek_)
                return /* continue */ true;

            const TOffset off = offsetByIdxChain(cltRoot_, ic);
            if (offToSeek_ != off)
                return /* continue */ true;

            // matched!
            icFound_ = ic;
            return false;
        }
};

bool digIcByOffset(
        TFieldIdxChain                  *pDst,
        const TObjType                  cltRoot,
        const TObjType                  cltField,
        const TOffset                   offRoot)
{
    CL_BREAK_IF(!cltRoot || !cltField);
    if (!offRoot && (*cltRoot == *cltField))
        // the root matches --> no fields on the way
        return false;

    CltFinder visitor(cltRoot, cltField, offRoot);
    if (traverseTypeIc(cltRoot, visitor, /* digOnlyComposite */ true))
        // not found
        return false;

    *pDst = visitor.icFound();
    return true;
}

void describeVar(PlotData &plot, const TValId rootAt)
{
    if (VAL_ADDR_OF_RET == rootAt) {
        plot.out << "OBJ_RETURN";
        return;
    }

    SymHeap &sh = plot.sh;
    TStorRef stor = sh.stor();

    // var lookup
    CVar cv = sh.cVarByRoot(rootAt);

    // write identity of the var
    plot.out << "CL" << varToString(stor, cv.uid) << " [root = #" << rootAt;
    if (1 < cv.inst)
        plot.out << ", inst = " << cv.inst;
    plot.out << "]";
}

void describeFieldPlacement(PlotData &plot, const ObjHandle &obj, TObjType clt)
{
    SymHeap &sh = plot.sh;

    const TObjType cltField = obj.objType();
    if (!cltField || *cltField == *clt)
        // nothing interesting here
        return;

    // read field offset
    const TValId at = obj.placedAt();
    const TOffset off = sh.valOffset(at);

    TFieldIdxChain ic;
    if (!digIcByOffset(&ic, clt, cltField, off))
        // type of the field not found in clt
        return;

    // chain of indexes found!
    BOOST_FOREACH(const int idx, ic) {
        CL_BREAK_IF(clt->item_cnt <= idx);
        const cl_type_item *item = clt->items + idx;
        const cl_type_e code = clt->code;

        if (CL_TYPE_ARRAY == code) {
            // TODO: support non-zero indexes? (not supported by CltFinder yet)
            CL_BREAK_IF(item->offset);
            plot.out << "[0]";
        }
        else {
            // read field name
            const char *name = item->name;
            if (!name)
                name = "<anon>";

            plot.out << "." << name;
        }

        // jump to the next item
        clt = item->type;
    }
}

void describeObject(PlotData &plot, const ObjHandle &obj, const bool lonely)
{
    SymHeap &sh = plot.sh;

    // check root
    const TValId at = obj.placedAt();
    const TValId root = sh.valRoot(at);
    const EValueTarget code = sh.valTarget(at);

    const char *tag = "";
    if (lonely && isProgramVar(code)) {
        describeVar(plot, root);
        tag = "field";
    }

    const TObjType cltRoot = sh.valLastKnownTypeOfTarget(root);
    if (cltRoot)
        describeFieldPlacement(plot, obj, cltRoot);

    plot.out << " " << tag << "#" << obj.objId();
}

void printRawRange(
        std::ostream                &str,
        const IR::Range             &rng,
        const char                  *suffix = "")
{
    if (isSingular(rng)) {
        str << rng.lo << suffix;
        return;
    }

    str << rng.lo << suffix << " .. " << rng.hi << suffix;

    if (isAligned(rng))
        str << ", alignment = " << rng.alignment << suffix;
}

void plotRootValue(PlotData &plot, const TValId val, const char *color)
{
    SymHeap &sh = plot.sh;
    const TSizeRange size = sh.valSizeOfTarget(val);
    const unsigned refCnt = sh.usedByCount(val);

#if SYMPLOT_FLAT_MODE
    if (refCnt)
#endif
    {
        // visualize the count of references as pen width
        const float pw = static_cast<float>(1U + refCnt);
        plot.out << "\t" << SL_QUOTE(val)
            << " [shape=ellipse, penwidth=" << pw
            << ", color=" << color
            << ", fontcolor=" << color
            << ", label=\"";

        const EValueTarget code = sh.valTarget(val);
        if (isProgramVar(code))
            describeVar(plot, val);
        else
            plot.out << "#" << val;
    }

#if SYMPLOT_FLAT_MODE
    if (refCnt)
        plot.out << "\"];";

    TValId obj = val;
    if (isDlSegPeer(sh, val))
        obj = dlSegPeer(sh, val);
    else
        plot.out << "\n\t" << SL_QUOTE("obj" << val)
            << " [shape=box"
            << ", color=" << color
            << ", fontcolor=" << color
            << ", label=\"O #" << val;

    if (val == obj)
#endif
    {
        plot.out << " [size = ";
        printRawRange(plot.out, size, " B");
        plot.out << "]\"];\n";
    }

#if SYMPLOT_FLAT_MODE
    if (!refCnt)
        return;

    plot.out << "\t" << SL_QUOTE(val) << " -> " << SL_QUOTE("obj" << obj)
        << " [color=" << color
        << ", fontcolor=" << color
        << ", label=\"[" << SIGNED_OFF(0)
        << "]\"];\n";
#endif
}

enum EObjectClass {
    OC_VOID = 0,
    OC_PTR,
    OC_NEXT,
    OC_PREV,
    OC_DATA
};

struct AtomicObject {
    ObjHandle       obj;
    EObjectClass    code;

    AtomicObject():
        code(OC_VOID)
    {
    }

    AtomicObject(const ObjHandle &obj_, EObjectClass code_):
        obj(obj_),
        code(code_)
    {
    }

    AtomicObject(const ObjHandle &obj_):
        obj(obj_),
        code(isDataPtr(obj.objType())
            ? OC_PTR
            : OC_DATA)
    {
    }
};

bool plotAtomicObj(PlotData &plot, const AtomicObject &ao, const bool lonely)
{
    SymHeap &sh = plot.sh;

    const ObjHandle &obj = ao.obj;
    CL_BREAK_IF(!obj.isValid());

    const char *color = "black";
    const char *props = ", penwidth=3.0, style=dashed";

    const EObjectClass code = ao.code;
    switch (code) {
        case OC_VOID:
            CL_BREAK_IF("plotAtomicObj() got an object of class OC_VOID");
            return false;

        case OC_PTR:
            props = "";
            break;

        case OC_NEXT:
            color = "red";
            break;

        case OC_PREV:
#if !SYMPLOT_DEBUG_DLS
            if (OK_DLS == sh.valTargetKind(obj.placedAt()))
                return false;
#endif
            // cppcheck-suppress unreachableCode
            color = "gold";
            break;

        case OC_DATA:
            color = "gray";
            props = ", style=dotted";
    }

    // store address mapping for the live object
    const TValId at = obj.placedAt();
    plot.liveObjs[at].push_back(obj);

#if SYMPLOT_FLAT_MODE
    return false;
#endif

    // cppcheck-suppress unreachableCode
    if (lonely) {
        const EValueTarget code = sh.valTarget(at);
        switch (code) {
            case VT_STATIC:
            case VT_ON_STACK:
                color = "blue";
                break;

            default:
                break;
        }
    }

    plot.out << "\t" << SL_QUOTE(obj.objId())
        << " [shape=box, color=" << color
        << ", fontcolor=" << color << props
        << ", label=\"";

    describeObject(plot, obj, lonely);

    if (OC_DATA == code)
        plot.out << " [size = " << obj.objType()->size << "B]";

    plot.out << "\"];\n";
    return true;
}

void plotUniformBlocks(PlotData &plot, const TValId root)
{
    SymHeap &sh = plot.sh;

    // get all uniform blocks inside the given root
    TUniBlockMap bMap;
    sh.gatherUniformBlocks(bMap, root);

    // plot all uniform blocks
    BOOST_FOREACH(TUniBlockMap::const_reference item, bMap) {
        const UniformBlock &bl = item.second;

        // plot block node
        const int id = ++plot.last;
#if !SYMPLOT_FLAT_MODE
        plot.out << "\t" << SL_QUOTE("lonely" << id)
            << " [shape=box, color=blue, fontcolor=blue, label=\"UNIFORM_BLOCK "
            << bl.size << "B\"];\n";

        // plot offset edge
        const TOffset off = bl.off;
        CL_BREAK_IF(off < 0);
        plot.out << "\t" << SL_QUOTE(root)
            << " -> " << SL_QUOTE("lonely" << id)
            << " [color=black, fontcolor=black, label=\"[+"
            << off << "]\"];\n";
#endif

        // schedule hasValue edge
        const PlotData::TDangVal dv(id, bl.tplValue);
        plot.dangVals.push_back(dv);
    }
}

template <class TCont>
void plotInnerObjects(PlotData &plot, const TValId at, const TCont &liveObjs)
{
    SymHeap &sh = plot.sh;

    ObjHandle next;
    ObjHandle prev;
    const EObjKind kind = sh.valTargetKind(at);
    switch (kind) {
        case OK_CONCRETE:
        case OK_OBJ_OR_NULL:
            break;

        case OK_DLS:
        case OK_SEE_THROUGH_2N:
            prev = prevPtrFromSeg(sh, at);
            // fall through!

        case OK_SEE_THROUGH:
        case OK_SLS:
            next = nextPtrFromSeg(sh, at);
    }

    // sort objects by offset
    typedef std::vector<AtomicObject>           TAtomList;
    typedef std::map<TOffset, TAtomList>        TAtomByOff;
    TAtomByOff objByOff;
    BOOST_FOREACH(const ObjHandle &obj, liveObjs) {
        EObjectClass code;
        if (obj == next)
            code = OC_NEXT;
        else if (obj == prev)
            code = OC_PREV;
        else if (isDataPtr(obj.objType()))
            code = OC_PTR;
        else
            code = OC_DATA;

        const TOffset off = sh.valOffset(obj.placedAt());
        AtomicObject ao(obj, code);
        objByOff[off].push_back(ao);
    }

    // plot all atomic objects inside
    BOOST_FOREACH(TAtomByOff::const_reference item, objByOff) {
        const TOffset off = item.first;
        BOOST_FOREACH(const AtomicObject &ao, /* TAtomList */ item.second) {
            // plot a single object
            if (!plotAtomicObj(plot, ao, /* lonely */ false))
                continue;

            // connect the inner object with the root by an offset edge
            plotOffset(plot, off, at, ao.obj.objId());
        }
    }
}

std::string labelOfCompObj(const SymHeap &sh, const TValId root, bool showProps)
{
    std::ostringstream label;
    const TProtoLevel protoLevel= sh.valTargetProtoLevel(root);
    if (protoLevel)
        label << "[L" << protoLevel << " prototype] ";

    const EObjKind kind = sh.valTargetKind(root);
    switch (kind) {
        case OK_CONCRETE:
            return label.str();

        case OK_OBJ_OR_NULL:
        case OK_SEE_THROUGH:
        case OK_SEE_THROUGH_2N:
            label << "0..1";
            break;

        case OK_SLS:
            label << "SLS";
            break;

        case OK_DLS:
            label << "DLS";
            break;
    }

    switch (kind) {
        case OK_SLS:
        case OK_DLS:
            // append minimal segment length
            label << " " << sh.segMinLength(root) << "+";

        default:
            break;
    }

    if (showProps && OK_OBJ_OR_NULL != kind) {
        const BindingOff &bf = sh.segBinding(root);
        switch (kind) {
            case OK_SLS:
            case OK_DLS:
                label << ", head [" << SIGNED_OFF(bf.head) << "]";

            default:
                break;
        }

        switch (kind) {
            case OK_SEE_THROUGH:
            case OK_SLS:
            case OK_DLS:
                label << ", next [" << SIGNED_OFF(bf.next) << "]";

            default:
                break;
        }

        if (OK_DLS == kind)
            label << ", prev [" << SIGNED_OFF(bf.prev) << "]";
    }

    return label.str();
}

template <class TCont>
void plotCompositeObj(PlotData &plot, const TValId at, const TCont &liveObjs)
{
    SymHeap &sh = plot.sh;

    const char *color = "black";
    const char *pw = "1.0";

    const EValueTarget code = sh.valTarget(at);
    switch (code) {
        case VT_STATIC:
        case VT_ON_STACK:
            color = "blue";
            break;

        case VT_ON_HEAP:
        case VT_ABSTRACT:
            break;

        default:
            CL_BREAK_IF("plotCompositeObj() got invalid root object");
            return;
    }

    const EObjKind kind = sh.valTargetKind(at);
    switch (kind) {
        case OK_CONCRETE:
            break;

        case OK_OBJ_OR_NULL:
        case OK_SEE_THROUGH:
        case OK_SEE_THROUGH_2N:
            color = "green";
            pw = "3.0";
            break;

        case OK_SLS:
            color = "red";
            pw = "3.0";
            break;

        case OK_DLS:
            color = "gold";
            pw = "3.0";
            break;
    }

    const std::string label = labelOfCompObj(sh, at, /* showProps */ true);

    // open cluster
    plot.out
        << "subgraph \"cluster" << (++plot.last)
        << "\" {\n\trank=same;\n\tlabel=" << SL_QUOTE(label)
        << ";\n\tcolor=" << color
        << ";\n\tfontcolor=" << color
#if SYMPLOT_FLAT_MODE
        << ";\n\tcolor=transparent;"
#else
        << ";\n\tbgcolor=gray98;\n\tstyle=dashed;"
#endif
        << "\n\tpenwidth=" << pw
        << ";\n";

    // plot the root value
    plotRootValue(plot, at, color);

    // plot all uniform blocks
    plotUniformBlocks(plot, at);

    // plot all atomic objects inside
    plotInnerObjects(plot, at, liveObjs);

    // in case of DLS, plot the corresponding peer
    TValId peer;
    if (OK_DLS == sh.valTargetKind(at)
            && OK_DLS == sh.valTargetKind((peer = dlSegPeer(sh, at))))
    {
        // plot peer's root value
        plotRootValue(plot, peer, color);
#if !SYMPLOT_DEBUG_DLS
        // plot peer's uniform blocks
        plotUniformBlocks(plot, at);
#endif

#if SYMPLOT_DEBUG_DLS
        ObjList peerObjs;
        sh.gatherLiveObjects(peerObjs, peer);
#else
        TObjSet peerObjs;
        buildIgnoreList(peerObjs, sh, peer);
#endif
        // plot all atomic objects inside
        plotInnerObjects(plot, peer, peerObjs);
    }

    // close cluster
    plot.out << "}\n";
}

bool plotSimpleRoot(PlotData &plot, const ObjHandle &obj)
{
    SymHeap &sh = plot.sh;

    const TValId at = obj.placedAt();
    if (sh.valOffset(at))
        // offset detected
        return false;

    const TValId root = sh.valRoot(at);
    if (sh.usedByCount(root))
        // root pointed
        return false;

    // TODO: support for objects with variable size?
    const TSizeRange size = sh.valSizeOfTarget(root);
    CL_BREAK_IF(!isSingular(size));

    const TObjType clt = obj.objType();
    CL_BREAK_IF(!clt);
    if (clt->size != size.lo)
        // size mismatch detected
        return false;

    const AtomicObject ao(obj);
    plotAtomicObj(plot, ao, /* lonely */ true);
    return true;
}

void plotRootObjects(PlotData &plot)
{
    SymHeap &sh = plot.sh;

    // go through roots
    BOOST_FOREACH(PlotData::TValues::const_reference item, plot.values) {
        if (! /* isRoot */ item.second)
            continue;

        const TValId root = item.first;
        if (isDlSegPeer(plot.sh, root))
            // DLS peers are never plotted directly at this level
            continue;

        // gather live objects
        ObjList liveObjs;
        sh.gatherLiveObjects(liveObjs, root);

#if !SYMPLOT_FLAT_MODE
        if (OK_CONCRETE == sh.valTargetKind(root)
                && (1 == liveObjs.size())
                && plotSimpleRoot(plot, liveObjs.front()))
            // this one went out in a simplified form
            continue;
#endif

        plotCompositeObj(plot, root, liveObjs);
    }
}

#define GEN_labelByCode(cst) case cst: return #cst

const char* labelByOrigin(const EValueOrigin code)
{
    switch (code) {
        GEN_labelByCode(VO_INVALID);
        GEN_labelByCode(VO_ASSIGNED);
        GEN_labelByCode(VO_UNKNOWN);
        GEN_labelByCode(VO_REINTERPRET);
        GEN_labelByCode(VO_DEREF_FAILED);
        GEN_labelByCode(VO_STACK);
        GEN_labelByCode(VO_HEAP);
    }

    CL_BREAK_IF("invalid call of labelByOrigin()");
    return "";
}

const char* labelByTarget(const EValueTarget code)
{
    switch (code) {
        GEN_labelByCode(VT_INVALID);
        GEN_labelByCode(VT_UNKNOWN);
        GEN_labelByCode(VT_COMPOSITE);
        GEN_labelByCode(VT_CUSTOM);
        GEN_labelByCode(VT_STATIC);
        GEN_labelByCode(VT_ON_STACK);
        GEN_labelByCode(VT_ON_HEAP);
        GEN_labelByCode(VT_LOST);
        GEN_labelByCode(VT_DELETED);
        GEN_labelByCode(VT_RANGE);
        GEN_labelByCode(VT_ABSTRACT);
    }

    CL_BREAK_IF("invalid call of labelByTarget()");
    return "";
}

void describeInt(PlotData &plot, const IR::TInt num, const TValId val)
{
    plot.out << ", fontcolor=red, label=\"[int] " << num;
    if (IR::Int0 < num && num < UCHAR_MAX && isprint(num))
        plot.out << " = '" << static_cast<char>(num) << "'";

    plot.out << " (#" << val << ")\"";
}

void describeIntRange(PlotData &plot, const IR::Range &rng, const TValId val)
{
    plot.out << ", fontcolor=blue, label=\"[int range] ";

    printRawRange(plot.out, rng);
    
    plot.out << " (#" << val << ")\"";
}

void describeReal(PlotData &plot, const float fpn, const TValId val)
{
    plot.out << ", fontcolor=red, label=\"[real] "
        << fpn << " (#"
        << val << ")\"";
}

void describeFnc(PlotData &plot, const int uid, const TValId val)
{
    TStorRef stor = plot.sh.stor();
    const CodeStorage::Fnc *fnc = stor.fncs[uid];
    CL_BREAK_IF(!fnc);

    const std::string name = nameOf(*fnc);
    plot.out << ", fontcolor=green, label=\""
        << name << "() (#"
        << val << ")\"";
}

void describeStr(PlotData &plot, const std::string &str, const TValId val)
{
    // we need to escape twice, once for the C compiler and once for graphviz
    plot.out << ", fontcolor=blue, label=\"\\\""
        << str << "\\\" (#"
        << val << ")\"";
}

void describeCustomValue(PlotData &plot, const TValId val)
{
    SymHeap &sh = plot.sh;
    const CustomValue cVal = sh.valUnwrapCustom(val);

    const ECustomValue code = cVal.code();
    switch (code) {
        case CV_INVALID:
            plot.out << ", fontcolor=red, label=CV_INVALID";
            break;

        case CV_INT_RANGE: {
            const IR::Range &rng = cVal.rng();
            if (isSingular(rng))
                describeInt(plot, rng.lo, val);
            else
                describeIntRange(plot, rng, val);
            break;
        }

        case CV_REAL:
            describeReal(plot, cVal.fpn(), val);
            break;

        case CV_FNC:
            describeFnc(plot, cVal.uid(), val);
            break;

        case CV_STRING:
            describeStr(plot, cVal.str(), val);
            break;
    }
}

void plotCustomValue(
        PlotData                       &plot,
        const int                       idFrom,
        const TValId                    val,
        const char                     *edgeLabel)
{
    const int id = ++plot.last;
    plot.out << "\t" << SL_QUOTE("lonely" << id) << " [shape=plaintext";

    describeCustomValue(plot, val);

    plot.out << "];\n\t"
#if SYMPLOT_FLAT_MODE
        << SL_QUOTE("obj" << idFrom)
#else
        << SL_QUOTE(idFrom)
#endif
        << " -> " << SL_QUOTE("lonely" << id)
        << " [color=blue, fontcolor=blue";
    appendLabelIf(plot.out, edgeLabel);
    plot.out << "];\n";
}

void plotValue(PlotData &plot, const TValId val)
{
    SymHeap &sh = plot.sh;

    const char *color = "black";
    const char *suffix = 0;

    const EValueTarget code = sh.valTarget(val);
    switch (code) {
        case VT_CUSTOM:
            // skipt it, custom values are now handled in plotHasValue()
            return;

        case VT_INVALID:
        case VT_COMPOSITE:
        case VT_LOST:
        case VT_DELETED:
        case VT_RANGE:
            color = "red";
            break;

        case VT_STATIC:
        case VT_ON_STACK:
            color = "blue";
            break;

        case VT_UNKNOWN:
            suffix = labelByOrigin(sh.valOrigin(val));
            // fall through!

        case VT_ABSTRACT:
            color = "green";
            // fall through!

        case VT_ON_HEAP:
            goto preserve_suffix;
    }

    suffix = labelByTarget(code);
preserve_suffix:

    const float pw = static_cast<float>(1U + sh.usedByCount(val));
    plot.out << "\t" << SL_QUOTE(val)
        << " [shape=ellipse, penwidth=" << pw
        << ", fontcolor=" << color
        << ", label=\"#" << val;

    if (suffix)
        plot.out << " " << suffix;

    const TValId root = sh.valRoot(val);

    if (VT_RANGE == code) {
        const IR::Range &offRange = sh.valOffsetRange(val);
        plot.out << " [root = #" << root
            << ", off = " << offRange.lo << ".." << offRange.hi;

        if (isAligned(offRange))
            plot.out << ", alignment = " << offRange.alignment;
    
        plot.out << "]";
    }
    else {
        const TOffset off = sh.valOffset(val);
        if (off)
            plot.out << " [root = #" << root << ", off = " << off << "]";
    }

    plot.out << "\"];\n";
}

void plotPointsTo(PlotData &plot, const TValId val, const TObjId target)
{
    plot.out << "\t" << SL_QUOTE(val)
        << " -> " << SL_QUOTE(target)
        << " [color=green, fontcolor=green];\n";
}

void plotRangePtr(PlotData &plot, TValId val, TValId root, const IR::Range &rng)
{
    plot.out << "\t" << SL_QUOTE(val) << " -> "
#if SYMPLOT_FLAT_MODE
        << SL_QUOTE("obj" << root)
#else
        << SL_QUOTE(root)
#endif
        << " [color=red, fontcolor=red, label=\"[";

    printRawRange(plot.out, rng);
    
    plot.out << "]\"];\n";
}

void plotNonRootValues(PlotData &plot)
{
    SymHeap &sh = plot.sh;

    // go through non-roots
    BOOST_FOREACH(PlotData::TValues::const_reference item, plot.values) {
        if (/* isRoot */ item.second)
            continue;

        // plot a value node
        const TValId val = item.first;
        plotValue(plot, val);

        const TValId root = sh.valRoot(val);
        const EValueTarget code = sh.valTarget(val);
        if (VT_RANGE == code) {
            const IR::Range &rng = sh.valOffsetRange(val);
            plotRangePtr(plot, val, root, rng);
            continue;
        }
        else if (!isPossibleToDeref(code))
            // no valid target
            continue;

        TValId offEdgeRoot = root;
#if SYMPLOT_FLAT_MODE
        dlSegJumpToBegIfNeeded(sh, &offEdgeRoot);
#else
        // assume an off-value
        PlotData::TLiveObjs::const_iterator it = plot.liveObjs.find(val);
        if ((plot.liveObjs.end() != it) && (1 == it->second.size())) {
            // exactly one target
            const ObjHandle &target = it->second.front();
            plotPointsTo(plot, val, target.objId());
            continue;
        }
#endif

        // an off-value with either no target, or too many targets
        const TOffset off = sh.valOffset(val);
        CL_BREAK_IF(!off);
        plotOffset(plot, off, offEdgeRoot, val);
    }

    // go through value prototypes used in uniform blocks
    BOOST_FOREACH(PlotData::TDangValues::const_reference item, plot.dangVals) {
        const TValId val = item.second;
        if (val <= 0)
            continue;

        // plot a value node
        CL_BREAK_IF(isPossibleToDeref(sh.valTarget(val)));
        plotValue(plot, val);
    }
}

const char* valNullLabel(const SymHeapCore &sh, const TObjId obj)
{
    const ObjHandle hdl(const_cast<SymHeapCore &>(sh), obj);
    const TObjType clt = hdl.objType();
    if (!clt)
        return "[type-free] 0";

    const enum cl_type_e code = clt->code;
    switch (code) {
        case CL_TYPE_INT:
            return "[int] 0";

        case CL_TYPE_PTR:
            return "NULL";

        case CL_TYPE_BOOL:
            return "FALSE";

        default:
            return "[?] 0";
    }
}

void plotAuxValue(
        PlotData                       &plot,
        const int                       node,
        const TValId                    val,
        const bool                      isObj,
        const char                     *edgeLabel = 0)
{
    const char *color = "blue";
    const char *label = "NULL";

    switch (val) {
        case VAL_NULL:
            if (isObj)
                label = valNullLabel(plot.sh, static_cast<TObjId>(node));
            break;

        case VAL_TRUE:
            color = "gold";
            label = "TRUE";
            break;

        case VAL_ADDR_OF_RET:
            label = "ADDR_OF_RET";
            break;

        case VAL_INVALID:
        default:
            color = "red";
            label = "VAL_INVALID";
    }

    const int id = ++plot.last;
    plot.out << "\t" << SL_QUOTE("lonely" << id)
        << " [shape=plaintext, fontcolor=" << color
        << ", label=" << SL_QUOTE(label) << "];\n";

    const char *prefix = "";
    if (edgeLabel)
        prefix = "obj";
    else if (!isObj)
        prefix = "lonely";

    plot.out << "\t" << SL_QUOTE(prefix << node)
        << " -> " << SL_QUOTE("lonely" << id)
        << " [color=blue, fontcolor=blue";
    appendLabelIf(plot.out, edgeLabel);
    plot.out << "];\n";
}

void plotHasValue(
        PlotData                       &plot,
        const int                       idFrom,
        const TValId                    val,
        const bool                      isObj = true,
        const char                     *edgeLabel = 0)
{
    SymHeap &sh = plot.sh;

    if (val <= 0) {
        plotAuxValue(plot, idFrom, val, isObj, edgeLabel);
        return;
    }

    const EValueTarget code = sh.valTarget(val);
    if (VT_CUSTOM == code) {
        plotCustomValue(plot, idFrom, val, edgeLabel);
        return;
    }

    plot.out << "\t"
#if SYMPLOT_FLAT_MODE
        << SL_QUOTE("obj" << idFrom)
#else
        << SL_QUOTE(idFrom)
#endif
        << " -> " << SL_QUOTE(val)
        << " [color=blue, fontcolor=blue";
    appendLabelIf(plot.out, edgeLabel);
    plot.out << "];\n";
}

/// (0 == clt) means a uniform block because uniform blocks are type-free
void plotHasValueFlat(
        PlotData                       &plot,
        TValId                          root,
        const TOffset                   beg,
        const TOffset                   end,
        const TObjType                  clt,
        const TValId                    val)
{
    dlSegJumpToBegIfNeeded(plot.sh, &root);

    std::ostringstream strLabel;
    if (clt) {
        const cl_type_e code = clt->code;
        switch (code) {
            case CL_TYPE_INT:
                strLabel << "INT";
                break;

            case CL_TYPE_PTR:
                if (isDataPtr(clt)) {
                    strLabel << "PTR";
                    break;
                }
                // fall through!

            default:
                // TODO
                strLabel << "OBJ";
                break;
        }
    }
    else
        strLabel << "uni_block";

    strLabel << "@<" << beg << ", " << end << ")";

    const std::string label(strLabel.str());
    plotHasValue(plot, root, val, /* isObj */ false, label.c_str());
}

void plotNeqZero(PlotData &plot, const TValId val)
{
    const int id = ++plot.last;
    plot.out << "\t" << SL_QUOTE("lonely" << id)
        << " [shape=plaintext, fontcolor=blue, label=NULL];\n";

    plot.out << "\t" << SL_QUOTE(val)
        << " -> " << SL_QUOTE("lonely" << id)
        << " [color=red, fontcolor=gold, label=neq style=dashed"
        ", penwidth=2.0];\n";
}

void plotNeqCustom(PlotData &plot, const TValId val, const TValId valCustom)
{
    const int id = ++plot.last;
    plot.out << "\t" << SL_QUOTE("lonely" << id)
        << " [shape=plaintext";

    describeCustomValue(plot, valCustom);

    plot.out << "];\n\t" << SL_QUOTE(val)
        << " -> " << SL_QUOTE("lonely" << id)
        << " [color=red, fontcolor=gold, label=neq style=dashed"
        ", penwidth=2.0];\n";
}

void plotNeq(std::ostream &out, const TValId v1, const TValId v2)
{
    out << "\t" << SL_QUOTE(v1)
        << " -> " << SL_QUOTE(v2)
        << " [color=red, style=dashed, penwidth=2.0, arrowhead=none"
        ", label=neq, fontcolor=gold, constraint=false];\n";
}

class NeqPlotter: public SymPairSet<TValId, /* IREFLEXIVE */ true> {
    public:
        void plotNeqEdges(PlotData &plot) {
            BOOST_FOREACH(const TItem &item, cont_) {
                const TValId v1 = item.first;
                const TValId v2 = item.second;

                if (VAL_NULL == v1)
                    plotNeqZero(plot, v2);
                else if (VT_CUSTOM == plot.sh.valTarget(v2))
                    plotNeqCustom(plot, v1, v2);
                else if (VT_CUSTOM == plot.sh.valTarget(v1))
                    plotNeqCustom(plot, v2, v1);
                else
                    plotNeq(plot.out, v1, v2);
            }
        }
};

void plotNeqEdges(PlotData &plot)
{
    // cppcheck-suppress unreachableCode
    SymHeap &sh = plot.sh;

    // gather relevant "neq" edges
    NeqPlotter np;
    BOOST_FOREACH(PlotData::TValues::const_reference item, plot.values) {
        const TValId val = item.first;
        const EValueTarget code = sh.valTarget(val);
        if (isKnownObject(code))
            // even if there was a non-equivalence, it would be the implicit one
            continue;

        // go through related values
        TValList relatedVals;
        sh.gatherRelatedValues(relatedVals, val);
        BOOST_FOREACH(const TValId rel, relatedVals)
            if (VAL_NULL == rel
                    || hasKey(plot.values, rel)
                    || VT_CUSTOM == sh.valTarget(rel))
                np.add(val, rel);
    }

    // plot "neq" edges
    np.plotNeqEdges(plot);
}

void plotFlatEdges(PlotData &plot)
{
    SymHeap &sh = plot.sh;

    // plot "hasValue" edges
    BOOST_FOREACH(PlotData::TLiveObjs::const_reference item, plot.liveObjs) {
        const TValId at = item.first;
        const TValId root = sh.valRoot(at);
        const TOffset beg = sh.valOffset(at);

        BOOST_FOREACH(const ObjHandle &obj, /* ObjList */ item.second) {
            const TObjType clt = obj.objType();
            const TOffset end = beg + clt->size;
            const TValId val = obj.value();

            plotHasValueFlat(plot, root, beg, end, clt, val);
        }
    }

    // go through all roots
    BOOST_FOREACH(PlotData::TValues::const_reference item, plot.values) {
        if (! /* isRoot */ item.second)
            continue;

        const TValId root = item.first;
#if !SYMPLOT_OMIT_DEBUG_DLS
        if (isDlSegPeer(plot.sh, root))
            // plot uniform blocks only once for a DLS pair
            continue;
#endif

        // get all uniform blocks inside the given root
        TUniBlockMap bMap;
        sh.gatherUniformBlocks(bMap, root);

        // plot flat edges for all uniform blocks at the current root
        BOOST_FOREACH(TUniBlockMap::const_reference item, bMap) {
            const UniformBlock &bl = item.second;
            const TOffset beg = bl.off;
            const TOffset end = beg + bl.size;
            const TValId val = bl.tplValue;

            plotHasValueFlat(plot, root, beg, end, /* clt */ 0, val);
        }
    }
}

void plotHasValueEdges(PlotData &plot)
{
    // plot "hasValue" edges
    BOOST_FOREACH(PlotData::TLiveObjs::const_reference item, plot.liveObjs)
        BOOST_FOREACH(const ObjHandle &obj, /* ObjList */ item.second)
            plotHasValue(plot, obj.objId(), obj.value());

    // plot "hasValue" edges for uniform block prototypes
    BOOST_FOREACH(PlotData::TDangValues::const_reference item, plot.dangVals) {
        const int id = item.first;
        const TValId val = item.second;

        if (val <= 0) {
            plotAuxValue(plot, id, val, /* isObj */ false);
            continue;
        }

        plot.out << "\t" << SL_QUOTE("lonely" << id)
            << " -> " << SL_QUOTE(val)
            << " [color=blue, fontcolor=blue];\n";
    }
}

void plotEverything(PlotData &plot)
{
    plotRootObjects(plot);
    plotNonRootValues(plot);

#if SYMPLOT_FLAT_MODE
    plotFlatEdges(plot);
#else
    plotHasValueEdges(plot);
#endif

#if !SYMPLOT_OMIT_NEQ_EDGES
    plotNeqEdges(plot);
#endif
}

bool plotHeap(
        const SymHeap                   &sh,
        const std::string               &name,
        const struct cl_loc             *loc,
        const TValList                  &startingPoints,
        const bool                      digForward)
{
    PlotEnumerator *pe = PlotEnumerator::instance();
    std::string plotName(pe->decorate(name));
    std::string fileName(plotName + ".dot");

    // create a dot file
    std::fstream out(fileName.c_str(), std::ios::out);
    if (!out) {
        CL_ERROR("unable to create file '" << fileName << "'");
        return false;
    }

    // open graph
    out << "digraph " << SL_QUOTE(plotName)
        << " {\n\tlabel=<<FONT POINT-SIZE=\"18\">" << plotName
        << "</FONT>>;\n\tclusterrank=local;\n\tlabelloc=t;\n";

    // check whether we can write to stream
    if (!out.flush()) {
        CL_ERROR("unable to write file '" << fileName << "'");
        out.close();
        return false;
    }

    if (loc)
        CL_NOTE_MSG(loc, "writing heap graph to '" << fileName << "'...");
    else
        CL_DEBUG("writing heap graph to '" << fileName << "'...");

    // initialize an instance of PlotData
    PlotData plot(sh, out);

    // do our stuff
    digValues(plot, startingPoints, digForward);
    plotEverything(plot);

    // close graph
    out << "}\n";
    const bool ok = !!out;
    out.close();
    return ok;
}

bool plotHeap(
        const SymHeap                   &sh,
        const std::string               &name,
        const struct cl_loc             *loc)
{
    TValList roots;
    sh.gatherRootObjects(roots);
    return plotHeap(sh, name, loc, roots);
}
