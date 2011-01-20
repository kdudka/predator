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

#include "stopwatch.hh"
#include <iomanip>
#include <time.h>

struct StopWatch::Private {
    clock_t start;
};

StopWatch::StopWatch():
    d(new Private)
{
    this->reset();
}

StopWatch::~StopWatch() {
    delete d;
}

void StopWatch::reset() {
    d->start = clock();
}

long /* msec */ StopWatch::elapsed() const {
    static const long RATIO = CLOCKS_PER_SEC/1000L;
    const clock_t diff = clock() - d->start;
    return diff/RATIO;
}

std::ostream& operator<<(std::ostream &str, const StopWatch &watch) {
    float elapsed = watch.elapsed();
    elapsed /= /* msec -> sec */ 1000.0;
    str << std::fixed << std::setprecision(3) << elapsed << " s";
    return str;
}
