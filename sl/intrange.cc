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

#include "intrange.hh"

// /////////////////////////////////////////////////////////////////////////////
// implementation of IntRange

#define RZ_MIN (IntMin >> 1)
#define RZ_MAX (IntMax >> 1)

#define RZ_CORRUPTION(n) (\
        (IntMin != (n) && (n) < RZ_MIN) || \
        (IntMax != (n) && RZ_MAX < (n)))

inline void chkRange(const IntRange &rng) {
    CL_BREAK_IF(RZ_CORRUPTION(rng.lo));
    CL_BREAK_IF(RZ_CORRUPTION(rng.hi));
    CL_BREAK_IF(IntMax == rng.lo);
    CL_BREAK_IF(IntMin == rng.hi);

    // FIXME: should we allow an empty range?
    CL_BREAK_IF(rng.hi < rng.lo);
}

const struct IntRange IntRangeDomain = {
    LONG_MIN,
    LONG_MAX
};

bool isCovered(const IntRange &small, const IntRange &big) {
    chkRange(small);
    chkRange(big);

    return (big.lo <= small.lo)
        && (small.hi <= big.hi);
}

bool isSingular(const IntRange &range) {
    chkRange(range);
    return (range.lo == range.hi);
}

TInt widthOf(const IntRange &range) {
    chkRange(range);
    return /* closed interval */ 1 + range.hi - range.lo;
}

TInt invertInt(const TInt num) {
    CL_BREAK_IF(RZ_CORRUPTION(num));

    if (IntMin == num)
        return IntMax;
    else if (IntMax == num)
        return IntMin;
    else
        return -num;
}

enum EIntBinOp {
    IBO_ADD,
    IBO_MUL
};

inline void intBinOp(TInt &dst, const TInt other, const EIntBinOp code) {
    switch (code) {
        case IBO_ADD:
            dst += other;
            break;

        case IBO_MUL:
            dst *= other;
            break;
    }
}

// the real arithmetic actually work only for "small" numbers this way
inline void rngBinOp(IntRange &rng, const IntRange &other, const EIntBinOp code)
{
    chkRange(rng);
    chkRange(other);

    if (IntMin != rng.lo) {
        if (IntMin == other.lo)
            rng.lo = IntMin;
        else
            intBinOp(rng.lo, other.lo, code);
    }

    if (IntMax != rng.hi) {
        if (IntMax == other.hi)
            rng.hi = IntMax;
        else
            intBinOp(rng.hi, other.hi, code);
    }

    chkRange(rng);
}

IntRange& operator+=(IntRange &rng, const IntRange &other) {
    rngBinOp(rng, other, IBO_ADD);
    return rng;
}

IntRange& operator*=(IntRange &rng, const IntRange &other) {
    rngBinOp(rng, other, IBO_MUL);
    return rng;
}
