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


#ifndef H_GUARD_SYM_PLOT_H
#define H_GUARD_SYM_PLOT_H

#include <string>

namespace CodeStorage {
    struct Fnc;
    struct Storage;
}

class SymHeap;

class SymHeapPlotter {
    public:
        ~SymHeapPlotter();
        SymHeapPlotter(const CodeStorage::Storage   &stor,
                       const SymHeap                &heap);

        bool plot(const std::string                 &name);

        bool plotHeapValue(const std::string        &name,
                           int                      value);

        bool plotStackFrame(const std::string       &name,
                            const CodeStorage::Fnc  &fnc);

    private:
        // object copying is not allowed
        SymHeapPlotter(const SymHeapPlotter &);
        SymHeapPlotter& operator=(const SymHeapPlotter &);

    private:
        struct Private;
        Private *d;
};

#endif /* H_GUARD_SYM_PLOT_H */
