/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of sl.
 *
 * sl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sl.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef H_GUARD_UTIL_H
#define H_GUARD_UTIL_H

#include <cstring>
#include <tuple>

#ifndef STREQ
#   define STREQ(s1, s2) (0 == strcmp(s1, s2))
#endif

// ensure (a <= b)
template <typename T>
void sortValues(T &a, T &b) {
    if (a <= b)
        return;

    const T tmp = a;
    a = b;
    b = tmp;
}

template <typename TCont>
bool hasKey(const TCont &cont, const typename TCont::key_type &key) {
    return cont.end() != cont.find(key);
}

template <typename TCont>
bool hasKey(const TCont *cont, const typename TCont::key_type &key) {
    return hasKey(*cont, key);
}

template <class TStack, class TFirst, class TSecond>
void push(TStack &dst, const TFirst &first, const TSecond &second)
{
    dst.push(typename TStack::value_type(first, second));
}

template <class TStack, class TFirst, class TSecond>
void push(TStack *dst, const TFirst &first, const TSecond &second)
{
    push(*dst, first, second);
}

#endif /* H_GUARD_UTIL_H */
