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

#include <climits>

namespace IR {

// /////////////////////////////////////////////////////////////////////////////
// implementation of Range

const TInt Int0   = 0L;
const TInt Int1   = 1L;
const TInt IntMin = LONG_MIN;
const TInt IntMax = LONG_MAX;

const Range FullRange = {
    /* lo        */ IntMin,
    /* hi        */ IntMax,
    /* alignment */ Int1
};

// we use the bit next to MSB to detect improper handling of IntMin/IntMax
#define RZ_MIN (IntMin >> 1)
#define RZ_MAX (IntMax >> 1)

// return true if the given number is located somewhere in the Red Zone (RZ)
#define RZ_CORRUPTION(n) (\
        (IntMin != (n) && (n) < RZ_MIN) || \
        (IntMax != (n) && RZ_MAX < (n)))

void chkRange(const Range &rng) {
    // check red zone
    CL_BREAK_IF(RZ_CORRUPTION(rng.lo));
    CL_BREAK_IF(RZ_CORRUPTION(rng.hi));
    CL_BREAK_IF(IntMax == rng.lo);
    CL_BREAK_IF(IntMin == rng.hi);

    // we do not allow empty ranges
    CL_BREAK_IF(rng.hi < rng.lo);

    // check alignment
    CL_BREAK_IF(rng.alignment < Int1);
    CL_BREAK_IF(rng.lo % rng.alignment);
    CL_BREAK_IF(rng.hi % rng.alignment);
    if (IntMin != rng.lo && IntMax != rng.hi)
        CL_BREAK_IF(1 + rng.hi - rng.lo < rng.alignment);
}

// TODO: replace this implementation by something useful (it can loop badly)
TInt greatestCommonDivisor(TInt a, TInt b) {
    CL_BREAK_IF(a < RZ_MIN || RZ_MAX < a);
    CL_BREAK_IF(b < RZ_MIN || RZ_MAX < b);

#if SE_DISABLE_ALIGNMENT_TRACKING
    return Int1;
#endif

    if (a < Int0)
        a = -a;
    if (b < Int0)
        b = -b;

    while (a != b) {
        if (a < b)
            b -= a;
        else
            a -= b;
    }

    CL_BREAK_IF(a < Int1);
    return a;
}

bool isCovered(const Range &small, const Range &big) {
    chkRange(small);
    chkRange(big);

    return (big.lo <= small.lo)
        && (small.hi <= big.hi)
        && (Int1 == big.alignment || big.alignment ==
                greatestCommonDivisor(small.alignment, big.alignment));
}

bool isSingular(const Range &range) {
    chkRange(range);
    return (range.lo == range.hi);
}

bool isAligned(const Range &range) {
    chkRange(range);
    return (Int1 < range.alignment);
}

TInt widthOf(const Range &range) {
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

inline void intBinOp(TInt *pDst, const TInt other, const EIntBinOp code) {
    switch (code) {
        case IBO_ADD:
            (*pDst) += other;
            break;

        case IBO_MUL:
            (*pDst) *= other;
            break;
    }
}

// the real arithmetic actually works only for "small" numbers this way
inline void rngBinOp(Range &rng, const Range &other, const EIntBinOp code) {
    chkRange(rng);
    chkRange(other);

    if (IntMin != rng.lo) {
        if (IntMin == other.lo)
            rng.lo = IntMin;
        else
            intBinOp(&rng.lo, other.lo, code);
    }

    if (IntMax != rng.hi) {
        if (IntMax == other.hi)
            rng.hi = IntMax;
        else
            intBinOp(&rng.hi, other.hi, code);
    }

    chkRange(rng);
}

Range& operator+=(Range &rng, const Range &other) {
    rngBinOp(rng, other, IBO_ADD);
    rng.alignment = greatestCommonDivisor(rng.alignment, other.alignment);
    return rng;
}

Range& operator*=(Range &rng, const Range &other) {
    rngBinOp(rng, other, IBO_MUL);
    rng.alignment *= other.alignment;
    return rng;
}

} // namespace IR
