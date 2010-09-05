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

#include "config.h"

#include <cl/easy.hh>
#include <cl/cl_msg.hh>
#include <cl/location.hh>
#include <cl/storage.hh>

#include <boost/foreach.hpp>

// required by the gcc plug-in API
extern "C" { int plugin_is_GPL_compatible; }

enum EVarStatus {
    VS_UNKNOWN,
    VS_NULL,
    VS_NOT_NULL,
    VS_MIGHT_BE_NULL
};

void handleFnc(const CodeStorage::Fnc &fnc) {
}

// /////////////////////////////////////////////////////////////////////////////
// see easy.hh for details
void clEasyRun(CodeStorage::Storage &stor, const char *) {
    BOOST_FOREACH(const CodeStorage::Fnc *pFnc, stor.fncs) {
        const CodeStorage::Fnc &fnc = *pFnc;
        if (!isDefined(fnc))
            continue;

        const LocationWriter lw(&fnc.def.loc);
        CL_DEBUG_MSG(lw, "analyzing function " << nameOf(fnc) << "()...");
        handleFnc(fnc);
    }
}
