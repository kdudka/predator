/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_UTIL_H
#define H_GUARD_UTIL_H

#include <algorithm>                // for std::find()
#include <cstring>

#ifndef STREQ
#   define STREQ(s1, s2) (0 == strcmp(s1, s2))
#endif

#define FIXW(w) std::fixed << std::setfill('0') << std::setw(w)

#define RETURN_IF_COMPARED(a, b, member) do {   \
    if (a.member < b.member)                    \
        return true;                            \
    if (b.member < a.member)                    \
        return false;                           \
} while (0)

template <typename T>
void swapValues(T &a, T &b)
{
    const T tmp = a;
    a = b;
    b = tmp;
}

// ensure (a <= b)
template <typename T>
void sortValues(T &a, T &b)
{
    if (b < a)
        swapValues(a, b);
}

template <typename TCont>
bool hasItem(const TCont &cont, const typename TCont::value_type &key)
{
    return cont.end() != std::find(cont.begin(), cont.end(), key);
}

template <typename TCont>
bool hasKey(const TCont &cont, const typename TCont::key_type &key)
{
    return cont.end() != cont.find(key);
}

template <typename TCont>
bool hasKey(const TCont *cont, const typename TCont::key_type &key)
{
    return hasKey(*cont, key);
}

template <typename TCont>
bool insertOnce(TCont &cont, const typename TCont::key_type &key)
{
    return cont.insert(key)./* inserted */second;
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
