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

#ifndef H_GUARD_FIXED_POINT_PROXY_H
#define H_GUARD_FIXED_POINT_PROXY_H

#include "symstate.hh"

#include <map>

namespace CodeStorage {
    struct Insn;
}

namespace FixedPoint {

    typedef const CodeStorage::Insn        *TInsn;

    class StateByInsn {
        public:
            typedef std::map<TInsn, SymStateWithJoin> TStateMap;

            StateByInsn();
            ~StateByInsn();

            bool /* any change */ insert(const TInsn insn, const SymHeap &sh);

            const TStateMap& stateMap() const;

            void plotAll();

        private:
            struct Private;
            Private *d;
    };

} // namespace FixedPoint

#endif /* H_GUARD_FIXED_POINT_PROXY_H */
