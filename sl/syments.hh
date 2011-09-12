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

#include "symid.hh"

#include <vector>

#include <boost/foreach.hpp>

#if SH_REUSE_FREE_IDS
#   include <queue>
#endif

#ifdef NDEBUG
    // aggressive optimization
#   define DCAST static_cast
#else
#   define DCAST dynamic_cast
#endif

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
#endif // SH_COPY_ON_WRITE

template <class TBaseEnt>
class EntStore {
    public:
        EntStore() { }
        inline EntStore(const EntStore &);
        inline ~EntStore();

        template <typename TId> inline TId assignId(TBaseEnt *);
        template <typename TId> inline void releaseEnt(const TId id);

        template <typename TId> TId lastId() const {
            return static_cast<TId>(this->ents_.size() - 1);
        }

        template <typename TId> bool outOfRange(const TId id) const {
            return (this->lastId<TId>() < id) || (id < 0);
        }

        template <typename TId> inline const TBaseEnt* getEntRO(const TId id);
        template <typename TId> inline TBaseEnt* getEntRW(const TId id);

        template <class TEnt, typename TId>
        inline void getEntRO(const TEnt **, const TId id);

        template <class TEnt, typename TId>
        inline void getEntRW(TEnt **, const TId id);

    private:
        // intentionally not implemented
        EntStore& operator=(const EntStore &);

        inline void releaseEntCore(TBaseEnt *ent);

        std::vector<TBaseEnt *>                 ents_;

#if SH_REUSE_FREE_IDS
        std::queue<unsigned>                    freeIds_;
#endif
};


// /////////////////////////////////////////////////////////////////////////////
// implementation of EntStore
template <class TBaseEnt>
template <typename TId>
TId EntStore<TBaseEnt>::assignId(TBaseEnt *ptr) {
    CL_BREAK_IF(ptr->refCnt.isShared());

#if SH_REUSE_FREE_IDS
    if (!this->freeIds_.empty()) {
        const T id = static_cast<T>(this->freeIds_.front());
        this->freeIds_.pop();
        this->ents_[id] = ptr;
        CL_DEBUG("reusing heap ID #" << id 
                << " (heap size is " << this->ents_.size() << ")");
        return id;
    }
#endif
    this->ents_.push_back(ptr);
    return this->lastId<TId>();
}

template <class TBaseEnt>
inline void EntStore<TBaseEnt>::releaseEntCore(TBaseEnt *ent) {
    if (/* wasLast */ ent->refCnt.leave())
        delete ent;
}

template <class TBaseEnt>
template <typename TId>
void EntStore<TBaseEnt>::releaseEnt(const TId id) {
#if SH_REUSE_FREE_IDS
    freeIds_.push(id);
#endif
    TBaseEnt *&e = ents_[id];
    CL_BREAK_IF(!e);
    this->releaseEntCore(e);
    e = 0;
}

template <class TBaseEnt>
EntStore<TBaseEnt>::EntStore(const EntStore &ref):
    ents_(ref.ents_)
{
    // go through all heap entities
    BOOST_FOREACH(TBaseEnt *&ent, ents_) {
        if (!ent)
            continue;

        if (/* needCloning */ ent->refCnt.enter())
            ent = ent->clone();
    }
}

template <class TBaseEnt>
EntStore<TBaseEnt>::~EntStore() {
    BOOST_FOREACH(TBaseEnt *ent, ents_)
        if (ent)
            this->releaseEntCore(ent);
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
    const TBaseEnt *entRO = this->getEntRO(id);

    TBaseEnt *entRW = const_cast<TBaseEnt *>(entRO);
    if (/* needCloning */ entRW->refCnt.requireExclusivity()) {
        entRW = entRO->clone();
        ents_[id] = entRW;
    }

    return entRW;
}

template <class TBaseEnt>
template <class TEnt, typename TId>
inline void EntStore<TBaseEnt>::getEntRO(const TEnt **pEnt, const TId id) {
    const TBaseEnt *ptr = this->getEntRO(id);
    const TEnt *ent = DCAST<const TEnt *>(ptr);

    // if this fails, the entity has type that is incompatible with your request
    CL_BREAK_IF(!ent);

    // all OK!
    *pEnt = ent;
}

template <class TBaseEnt>
template <class TEnt, typename TId>
inline void EntStore<TBaseEnt>::getEntRW(TEnt **pEnt, const TId id) {
    TBaseEnt *ptr = this->getEntRW(id);
    TEnt *ent = DCAST<TEnt *>(ptr);

    // if this fails, the entity has type that is incompatible with your request
    CL_BREAK_IF(!ent);

    // all OK!
    *pEnt = ent;
}

#endif /* H_GUARD_SYM_ENTS_H */
