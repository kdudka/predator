/*
 * Copyright (C) 2010 Jiri Simacek
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

#include <cl/easy.hh>
#include <cl/cl_msg.hh>
#include <cl/location.hh>
#include <cl/storage.hh>

#include "symexec.hh"

// required by the gcc plug-in API
extern "C" { int plugin_is_GPL_compatible; }

void clEasyRun(const CodeStorage::Storage& stor, const char* configString) {
//    CL_ERROR("fa_analysis is not implemented yet");
    using namespace CodeStorage;

    // look for main() by name
    CL_DEBUG("looking for 'main()' at gl scope...");
    const NameDb::TNameMap &glNames = stor.fncNames.glNames;
    const NameDb::TNameMap::const_iterator iter = glNames.find("main");
    if (glNames.end() == iter) {
        CL_WARN("main() not found at global scope");
        return;
    }

    // look for definition of main()
    const FncDb &fncs = stor.fncs;
    const Fnc *main = fncs[iter->second];
    if (!main || !isDefined(*main)) {
        CL_WARN("main() not defined");
        return;
    }

    CL_DEBUG("starting verification stuff ...");
	SymExec(stor).run(*main);

}
