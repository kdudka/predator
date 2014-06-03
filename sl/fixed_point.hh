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

#ifndef H_GUARD_FIXED_POINT_H
#define H_GUARD_FIXED_POINT_H

#include "clean_list.hh"
#include "fixed_point_proxy.hh"
#include "id_mapper.hh"
#include "shape.hh"
#include "symstate.hh"

#include <climits>                  // for INT_MIN/INT_MAX
#include <utility>

namespace CodeStorage {
    struct Fnc;
}

namespace FixedPoint {

typedef const CodeStorage::Fnc                     *TFnc;

typedef int                                         TLocIdx;
typedef int                                         THeapIdx;
typedef int                                         TShapeIdx;

/// heap (in our case represented by SMG) identity
typedef std::pair<TLocIdx, THeapIdx>                THeapIdent;

/// container shape (in our case represented by Shape) identity
typedef std::pair<THeapIdent, TShapeIdx>            TShapeIdent;

/// fixed heap ID for signalling failures where ID is expected
extern const THeapIdent InvalidHeap;

/// NOTE: we can omit specifying INT_MIN/INT_MAX when compiling as C++11
typedef IdMapper<TShapeIdx, INT_MIN, INT_MAX>       TShapeMapper;
typedef IdMapper<TObjId, OBJ_INVALID, OBJ_MAX_ID>   TObjectMapper;

class GenericInsn {
    public:
        virtual ~GenericInsn() { }

        // NVI to catch missing/incorrect overrides of doClone()
        GenericInsn* clone() const;

        /// write human-readable representation of the insn to an output stream
        virtual void writeToStream(std::ostream &) const = 0;

        /// return CL representation of the insn if exists; 0 otherwise
        virtual TInsn clInsn() const = 0;

    private:
        /// see Herb Sutter: C++ Coding Standards (rules #39 and #54) for details
        virtual GenericInsn* doClone() const = 0;
};

inline std::ostream& operator<<(std::ostream &str, const GenericInsn &insn)
{
    insn.writeToStream(str);
    return str;
}

/// single heap-level trace edge holding inner ID mappings inside
struct TraceEdge {
    THeapIdent              src;            /// source heap
    THeapIdent              dst;            /// destination heap
    TShapeMapper            csMap;          /// container shapes mapping
    TObjectMapper           objMap;         /// heap object IDs mapping

    TraceEdge(const THeapIdent &src_, const THeapIdent &dst_):
        src(src_),
        dst(dst_)
    {
    }
};

typedef std::vector<TraceEdge *>                    TTraceEdgeList;
typedef std::vector<TTraceEdgeList>                 TEdgeListByHeapIdx;

/// single control-flow edge connecting instructions (no basic blocks here)
struct CfgEdge {
    TLocIdx                 targetLoc;      /// the opposite adjacent location
    bool                    closesLoop;     /// true for loop-closing CFG edge

    CfgEdge(const TLocIdx targetLoc_, const bool closesLoop_ = false):
        targetLoc(targetLoc_),
        closesLoop(closesLoop_)
    {
    }
};

typedef std::vector<CfgEdge>                        TCfgEdgeList;

/// state summary for a single location (preceding a single instruction)
struct LocalState {
    GenericInsn            *insn;           /// insn using the state as input
    SymHeapList             heapList;       /// union of heaps giving the state
    TShapeListByHeapIdx     shapeListByHeapIdx; /// container shapes per heap
    TCfgEdgeList            cfgInEdges;     /// ingoing control-flow edges
    TCfgEdgeList            cfgOutEdges;    /// outgoing control-flow edges
    TEdgeListByHeapIdx      traceInEdges;   /// ingoing heap-level trace edges
    TEdgeListByHeapIdx      traceOutEdges;  /// outgoing heap-level trace edges

    LocalState(): insn(0) { }
    ~LocalState() { delete insn; }
    LocalState(const LocalState &);
    LocalState& operator=(const LocalState &);
};

/// annotated fixed-point of a program (or its part, e.g. a function)
class GlobalState {
    public:
        GlobalState() { }

        /// return state summary local to the given location
        LocalState& operator[](const TLocIdx loc) {
            return *stateList_[loc];
        }

        /// return state summary local to the given location
        const LocalState& operator[](const TLocIdx loc) const {
            return *stateList_[loc];
        }

        /// return total count of locations maintained by the container
        TLocIdx size() const {
            return stateList_.size();
        }

    private:
        CleanList<LocalState>       stateList_;
        CleanList<TraceEdge>        traceList_;

    private:
        // intentionally not implemented
        GlobalState(const GlobalState &);
        GlobalState& operator=(const GlobalState &);

        friend GlobalState* computeStateOf(const TFnc,
                const StateByInsn::TStateMap &);

        friend void exportControlFlow(GlobalState *pDst,
                const GlobalState &glState);

        friend class StateRewriter;
};

/// return heap of the given state by its identity
const SymHeap *heapByIdent(const GlobalState &, THeapIdent);

/// return heap of the given state by its identity
SymHeap *heapByIdent(GlobalState &, THeapIdent);

/// return shape of the given state by its identity
const Shape *shapeByIdent(const GlobalState &, const TShapeIdent &);

/// caller is responsible to destroy the returned instance
GlobalState* computeStateOf(TFnc, const StateByInsn::TStateMap &);

/// write the CFG-only skeleton of glState into *pDst
void exportControlFlow(GlobalState *pDst, const GlobalState &glState);

/// remove instructions assigning values to dead variables
void removeDeadCode(GlobalState *pState);

/// pretty print the given ID mapping
void sl_dump(const TShapeMapper &);

/// pretty print the given ID mapping
void sl_dump(const TObjectMapper &);

} // namespace FixedPoint

#endif /* H_GUARD_FIXED_POINT_H */
