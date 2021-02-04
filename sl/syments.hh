/*
 * Copyright (C) 2011 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_SYM_ENTS_H
#define H_GUARD_SYM_ENTS_H

#include "config.h"

#include "util.hh"

#include <vector>

#include <boost/foreach.hpp>

#if SH_COPY_ON_WRITE
class RefCounter {
    private:
        typedef int TCnt;
        TCnt cnt_;

    public:
        /// initialize to 1
        RefCounter():
            cnt_(1)
        {
        }

        /// initialize to 1, even if the source has another value
        RefCounter(const RefCounter &):
            cnt_(1)
        {
        }

        /// initialize to 1, even if the source has another value
        RefCounter& operator=(const RefCounter &) {
            cnt_ = 1;
            return *this;
        }

        /// the destruction is only allowed with reference count equal to zero
        ~RefCounter() {
            CL_BREAK_IF(cnt_);
        }

        bool isShared() const {
            CL_BREAK_IF(cnt_ < 1);
            return (1 < cnt_);
        }

        bool /* needCloning */ enter() {
            CL_BREAK_IF(cnt_ < 1);
            ++cnt_;
            return false;
        }

        bool /* needCloning */ requireExclusivity() {
            if (!this->isShared())
                return false;

            --cnt_;
            return true;
        }

        bool /* wasLast */ leave() {
            return !(--cnt_);
        }

}; // class RefCounter

#else // SH_COPY_ON_WRITE

// dummy implementation, no data inside
class RefCounter {
    public:
        bool isShared() const {
            return false;
        }

        bool /* needCloning */ enter() {
            return true;
        }

        bool /* needCloning */ requireExclusivity() {
            return false;
        }

        bool /* wasLast */ leave() {
            return true;
        }
};

#if SH_PREVENT_AMBIGUOUS_ENT_ID
#error SH_PREVENT_AMBIGUOUS_ENT_ID requires SH_COPY_ON_WRITE to be enabled
#endif

#endif // SH_COPY_ON_WRITE

enum ERefCntObjKind {
    RCO_VIRTUAL,
    RCO_NON_VIRT
};

template <enum ERefCntObjKind TKind> struct RefCntUtil;

template <>
struct RefCntUtil<RCO_VIRTUAL> {
    template <class T> static void clone(T *&ptr) {
        ptr = ptr->clone();
    }
};

template <>
struct RefCntUtil<RCO_NON_VIRT> {
    template <class T> static void clone(T *&ptr) {
        ptr = new T(*ptr);
    }
};

template <enum ERefCntObjKind TKind> struct RefCntLib {
    template <class T> static void enter(T *&ptr) {
        if (/* needCloning */ ptr->refCnt.enter())
            RefCntUtil<TKind>::clone(ptr);
    }

    template <class T> static void leave(T *&ptr) {
        if (/* wasLast */ ptr->refCnt.leave())
            delete ptr;

        // avoid possible use after free at caller's side
        ptr = 0;
    }

    template <class T> static void requireExclusivity(T *&ptr) {
        if (/* needCloning */ ptr->refCnt.requireExclusivity())
            RefCntUtil<TKind>::clone(ptr);
    }
};

struct EntCounter {
#if SH_PREVENT_AMBIGUOUS_ENT_ID
    long                entCnt;
    RefCounter          refCnt;

    EntCounter():
        entCnt(0L)
    {
    }
#endif
};

template <class TBaseEnt>
class EntStore {
    public:
        EntStore();
        inline EntStore(const EntStore &);
        inline ~EntStore();

        template <typename TId> inline TId assignId(TBaseEnt *);
        template <typename TId> inline void assignId(TId id, TBaseEnt *);
        template <typename TId> inline void releaseEnt(TId id);
        template <typename TId> inline bool isValidEnt(TId id) const;

        template <typename TId> TId lastId() const {
            // we need to be careful with integral arithmetic on enums
            const long last = -1L + ents_.size();
            return static_cast<TId>(last);
        }

        template <typename TId> bool outOfRange(const TId id) const {
            return (this->lastId<TId>() < id) || (id < 0);
        }

        template <typename TId> inline const TBaseEnt* getEntRO(TId id);
        template <typename TId> inline TBaseEnt* getEntRW(TId id);

        template <class TEnt, typename TId>
        inline void getEntRO(const TEnt **, TId id);

        template <class TEnt, typename TId>
        inline void getEntRW(TEnt **, TId id);

    private:
        // intentionally not implemented
        EntStore& operator=(const EntStore &);

        std::vector<TBaseEnt *>                 ents_;
        EntCounter                             *entCnt_;
};


// /////////////////////////////////////////////////////////////////////////////
// implementation of EntStore
template <class TBaseEnt>
template <typename TId>
TId EntStore<TBaseEnt>::assignId(TBaseEnt *ptr)
{
    CL_BREAK_IF(ptr->refCnt.isShared());
#if SH_PREVENT_AMBIGUOUS_ENT_ID
    const TId id = static_cast<TId>(entCnt_->entCnt);
    this->assignId(id, ptr);
    return id;
#else
    this->ents_.push_back(ptr);
    return this->lastId<TId>();
#endif
}

template <class TBaseEnt>
template <typename TId>
void EntStore<TBaseEnt>::assignId(const TId id, TBaseEnt *ptr)
{
    CL_BREAK_IF(ptr->refCnt.isShared());

    // make sure we have enough space allocated
    if (this->lastId<TId>() < id)
        ents_.resize(id + 1, 0);

    TBaseEnt *&ref = ents_[id];

    // if this fails, you wanted to overwrite pointer to a valid entity
    CL_BREAK_IF(ref);

    ref = ptr;
#if SH_PREVENT_AMBIGUOUS_ENT_ID
    const long cntNow = 1L + id;
    if (entCnt_->entCnt < cntNow)
        entCnt_->entCnt = cntNow;
#endif
}

template <class TBaseEnt>
template <typename TId>
void EntStore<TBaseEnt>::releaseEnt(const TId id)
{
    RefCntLib<RCO_VIRTUAL>::leave(ents_[id]);
}

template <class TBaseEnt>
template <typename TId>
bool EntStore<TBaseEnt>::isValidEnt(const TId id) const
{
    if (this->outOfRange(id))
        return false;

    return !!ents_[id];
}

template <class TBaseEnt>
EntStore<TBaseEnt>::EntStore()
#if SH_PREVENT_AMBIGUOUS_ENT_ID
    : entCnt_(new EntCounter)
#endif
{
}

template <class TBaseEnt>
EntStore<TBaseEnt>::EntStore(const EntStore &ref):
    ents_(ref.ents_)
#if SH_PREVENT_AMBIGUOUS_ENT_ID
    , entCnt_(ref.entCnt_)
#endif
{
#if SH_PREVENT_AMBIGUOUS_ENT_ID
    RefCntLib<RCO_NON_VIRT>::enter(entCnt_);
#endif
    BOOST_FOREACH(TBaseEnt *&ent, ents_)
        if (ent)
            RefCntLib<RCO_VIRTUAL>::enter(ent);
}

template <class TBaseEnt>
EntStore<TBaseEnt>::~EntStore()
{
#if SH_PREVENT_AMBIGUOUS_ENT_ID
    RefCntLib<RCO_NON_VIRT>::leave(entCnt_);
#endif
    BOOST_FOREACH(TBaseEnt *ent, ents_)
        if (ent)
            RefCntLib<RCO_VIRTUAL>::leave(ent);
}

template <class TBaseEnt>
template <typename TId>
inline const TBaseEnt* EntStore<TBaseEnt>::getEntRO(const TId id)
{
    // if this fails, the ID has never been valid
    CL_BREAK_IF(this->outOfRange(id));

    // if this fails, the ID is no longer valid
    const TBaseEnt *ptr = ents_[id];
    CL_BREAK_IF(!ptr);
    return ptr;
}

template <class TBaseEnt>
template <typename TId>
inline TBaseEnt* EntStore<TBaseEnt>::getEntRW(const TId id)
{
#ifndef NDEBUG
    this->getEntRO(id);
#endif
    TBaseEnt *&entRW = ents_[id];
    RefCntLib<RCO_VIRTUAL>::requireExclusivity(entRW);
    return entRW;
}

template <class TBaseEnt>
template <class TEnt, typename TId>
inline void EntStore<TBaseEnt>::getEntRO(const TEnt **pEnt, const TId id)
{
    const TBaseEnt *ptr = this->getEntRO(id);
    const TEnt *ent = DCAST<const TEnt *>(ptr);

    // if this fails, the entity has type that is incompatible with your request
    CL_BREAK_IF(!ent);

    // all OK!
    *pEnt = ent;
}

template <class TBaseEnt>
template <class TEnt, typename TId>
inline void EntStore<TBaseEnt>::getEntRW(TEnt **pEnt, const TId id)
{
    TBaseEnt *ptr = this->getEntRW(id);
    TEnt *ent = DCAST<TEnt *>(ptr);

    // if this fails, the entity has type that is incompatible with your request
    CL_BREAK_IF(!ent);

    // all OK!
    *pEnt = ent;
}

#endif /* H_GUARD_SYM_ENTS_H */
