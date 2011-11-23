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

#ifndef H_GUARD_INTRANGE_H
#define H_GUARD_INTRANGE_H

#include "config.h"

#include <climits>

typedef signed long TInt;

#define IntMin  (LONG_MIN)
#define Int0    (0L)
#define IntMax  (LONG_MAX)

/// a closed interval over integral domain
struct IntRange {
    TInt        lo;         ///< lower bound of the interval (included)
    TInt        hi;         ///< upper bound of the interval (included)

    // NOTE: there is no constructor, becase we put IntRange to unions
};

inline IntRange rngFromNum(TInt num) {
    IntRange rng;
    rng.lo = num;
    rng.hi = num;
    return rng;
}

/// FIXME: this way we are asking for overflow (build vs. host arch mismatch)
extern const struct IntRange IntRangeDomain;

inline bool operator==(const IntRange &a, const IntRange &b) {
    return (a.lo == b.lo)
        && (a.hi == b.hi);
}

inline bool operator!=(const IntRange &a, const IntRange &b) {
    return !operator==(a, b);
}

/// invert polarity of the number
TInt invertInt(const TInt);

/// invert polarity of the range
inline IntRange operator-(IntRange rng) {
    const TInt hi = invertInt(rng.lo);
    rng.lo = invertInt(rng.hi);
    rng.hi = hi;

    return rng;
}

/// add another range, but preserve boundary values if already reached
IntRange& operator+=(IntRange &rng, const IntRange &other);

/// multiply by another range, but preserve boundary values if already reached
IntRange& operator*=(IntRange &rng, const IntRange &other);

/// subtract another range, but preserve boundary values if already reached
inline IntRange& operator-=(IntRange &rng, const IntRange &other) {
    rng += (-other);
    return rng;
}

inline IntRange operator+(IntRange rng, const IntRange &other) {
    rng += other;
    return rng;
}

inline IntRange operator*(IntRange rng, const IntRange &other) {
    rng *= other;
    return rng;
}

inline IntRange operator-(IntRange rng, const IntRange &other) {
    rng -= other;
    return rng;
}

bool isCovered(const IntRange &small, const IntRange &big);

/// return true if the range contain exactly one number; break if no one at all
bool isSingular(const IntRange &);

/// return the count of integral numbers that beTInt the given range
TInt widthOf(const IntRange &);


#endif /* H_GUARD_INTRANGE_H */
