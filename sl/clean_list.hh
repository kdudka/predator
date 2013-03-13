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

#ifndef H_GUARD_CLEAN_LIST_H
#define H_GUARD_CLEAN_LIST_H

#include "config.h"

#include <vector>

// TODO: replace this by boost::something once we find it
template <class T>
class CleanList {
    private:
        typedef std::vector<T *> TList;
        TList cl_;

    public:
        CleanList() { }

        ~CleanList() {
            typename TList::const_reverse_iterator i;
            for (i = cl_.rbegin(); i != cl_.rend(); ++i)
                delete *i;
        }

        void append(T *ptr) {
            cl_.push_back(ptr);
        }

        unsigned size() const {
            return cl_.size();
        }

        template <typename TIdx>
        T* operator[](const TIdx idx) {
            CL_BREAK_IF(static_cast<TIdx>(cl_.size()) <= idx);
            return cl_[idx];
        }

        template <typename TIdx>
        const T* operator[](const TIdx idx) const {
            return const_cast<CleanList *>(this)->operator[](idx);
        }

    private:
        // copying NOT allowed
        CleanList(const CleanList &);
        CleanList& operator=(const CleanList &);
};

#endif /* H_GUARD_CLEAN_LIST_H */
