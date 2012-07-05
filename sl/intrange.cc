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

#include <algorithm>                // for std::min and std::max
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
    CL_BREAK_IF(IntMin != rng.lo && rng.lo % rng.alignment);
    CL_BREAK_IF(IntMax != rng.hi && rng.hi % rng.alignment);
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

Range join(const Range &rng1, const Range &rng2) {
    Range result;
    result.lo = std::min(rng1.lo, rng2.lo);
    result.hi = std::max(rng1.hi, rng2.hi);

    // TODO
    result.alignment = Int1;

    chkRange(result);
    return result;
}

bool isRangeByNum(bool *pIsRange1, const Range &rng1, const Range &rng2) {
    const bool isRange1 = !isSingular(rng1);
    const bool isRange2 = !isSingular(rng2);
    if (isRange1 == isRange2)
        return false;

    *pIsRange1 = isRange1;
    return true;
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

TUInt widthOf(const Range &range) {
    chkRange(range);

    if (range == FullRange)
        // FIXME: this would overflow
        CL_BREAK_IF("widthOf() does not work for IR::FullRange");

    return /* closed interval */ 1UL + range.hi - range.lo;
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
    IBO_MUL,
    IBO_LSHIFT,
    IBO_RSHIFT
};

inline void intBinOp(TInt *pDst, const TInt other, const EIntBinOp code) {
    switch (code) {
        case IBO_ADD:
            (*pDst) += other;
            break;

        case IBO_MUL:
            (*pDst) *= other;
            break;

        case IBO_LSHIFT:
            (*pDst) <<= other;
            break;

        case IBO_RSHIFT:
            (*pDst) >>= other;
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
}

TInt alignmentOf(const Range &rng) {
    chkRange(rng);

    if (!isSingular(rng))
        return rng.alignment;

    CL_BREAK_IF(Int1 != rng.alignment);
    const TInt num = rng.lo;
    if (!num)
        return Int1;
    else if (num < Int0)
        return -num;
    else
        return num;
}

Range& operator+=(Range &rng, const Range &other) {
    // this needs to be done before rng is modified
    const TInt al1 = alignmentOf(rng);
    const TInt al2 = alignmentOf(other);

    // perform the actual range operation
    rngBinOp(rng, other, IBO_ADD);

    // compute the resulting alignment
    rng.alignment = Int1;
    if (!isSingular(rng))
        rng.alignment = greatestCommonDivisor(al1, al2);

    chkRange(rng);
    return rng;
}

Range& operator<<=(Range &rng, const TUInt n) {
    rngBinOp(rng, rngFromNum(n), IBO_LSHIFT);
    rng.alignment = Int1;
    chkRange(rng);
    return rng;
}

Range& operator>>=(Range &rng, const TUInt n) {
    rngBinOp(rng, rngFromNum(n), IBO_RSHIFT);
    rng.alignment = Int1;
    chkRange(rng);
    return rng;
}

Range& operator*=(Range &rng, const Range &other) {
    // this needs to be done before rng is modified
    TInt coef = Int1;
    bool isRange1;
    if (isRangeByNum(&isRange1, rng, other))
        coef = (isRange1) ? other.lo : rng.lo;

    if (Int0 == coef)
        // the result is going to be zero anyway
        coef = Int1;
    else if (coef < Int0)
        // TODO: double check if we are still on the safe side here
        coef = -coef;

    // perform the actual range operation
    rngBinOp(rng, other, IBO_MUL);

    if (rng.lo == rng.hi)
        rng.alignment = Int1;
    else {
        // compute the resulting alignment
        rng.alignment *= other.alignment;
        rng.alignment *= coef;
    }

    chkRange(rng);
    return rng;
}

bool isZeroIntersection(TInt alignment, TInt mask) {
    CL_BREAK_IF(alignment < Int1);

    if (mask)
        --alignment;
    else
        return true;

    while (alignment & 1) {
        alignment >>= 1;
        mask >>= 1;
        if (!mask)
            return true;
    }

    return false;
}

TInt maskToAlignment(TInt mask) {
    if (!mask) {
        CL_BREAK_IF("invalid call of maskToAlignment()");
        return Int1;
    }

    TInt alignment = Int1;
    while (!(mask & 1)) {
        alignment <<= 1;
        mask >>= 1;
    }

    return alignment;
}

Range& operator&=(Range &rng, TInt mask) {
    if (isZeroIntersection(rng.alignment, mask))
        // the whole range was masked, we are back to zero
        return (rng = rngFromNum(Int0));

    if (Int0 < mask)
        // TODO
        return (rng = FullRange);

    // include all possible scenarios into consideration
    rng.lo       &= mask;
    rng.hi       &= mask;
    rng.alignment = maskToAlignment(mask);

    chkRange(rng);
    return rng;
}

} // namespace IR
