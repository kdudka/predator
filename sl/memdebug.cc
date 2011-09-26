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

#include "config.h"
#include "memdebug.hh"

#include <cl/cl_msg.hh>

#include <iomanip>

#if DEBUG_MEM_USAGE
#   include <malloc.h>

bool rawMemUsage(size_t *pDst) {
    static bool overflowDetected;
    if (overflowDetected)
        return false;

    struct mallinfo info = mallinfo();
    const size_t raw = info.uordblks;
    const unsigned mib = raw >> /* MiB */ 20;
    if (2048U < mib) {
        // mallinfo() is broken by design <https://bugzilla.redhat.com/173813>
        overflowDetected = true;
        return false;
    }

    *pDst = raw;
    return true;
}

static size_t memDrift;

bool initMemDrif() {
    if (rawMemUsage(&::memDrift))
        return true;

    // failed to get current memory usage
    ::memDrift = 0U;
    return false;
}

bool currentMemUsage(size_t *pDst) {
    if (!rawMemUsage(pDst))
        // failed to get current memory usage
        return false;

    *pDst -= ::memDrift;
    return true;
}

bool printMemUsage(const char *fnc) {
    size_t cb;
    if (!currentMemUsage(&cb))
        // instead of printing misleading numbers, we rather print nothing
        return false;

    const float amount = /* MiB */ static_cast<float>(cb >> 10) / 1024.0;
    CL_DEBUG("current memory usage: "
             << std::fixed << std::setw(7) << std::setprecision(2)
             << amount << " MB" << " (just completed " << fnc << "())");

    return true;
}

#else // DEBUG_MEM_USAGE

bool rawMemUsage(size_t *) {
    return false;
}

bool initMemDrif() {
    return false;
}

bool currentMemUsage(size_t *) {
    return false;
}

bool printMemUsage(const char *) {
    return false;
}

#endif
