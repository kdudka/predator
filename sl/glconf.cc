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

#include "config.h"
#include "glconf.hh"

#include <cl/cl_msg.hh>

#include <cstring>                  // for strlen(), strncmp()

namespace GlConf {

Options data;

void loadConfigString(const std::string &cnf)
{
    using std::string;
    if (cnf.empty())
        return;

    if (string("no_error_recovery") == cnf) {
        CL_DEBUG("parseConfigString: \"no_error_recovery\" mode requested");
        data.errorRecoveryMode = /* no_error_recovery */ 0;
        return;
    }

    if (string("oom") == cnf) {
        CL_DEBUG("parseConfigString: \"OOM simulation\" mode requested");
        data.oomSimulation = true;
        return;
    }

    if (string("track_uninit") == cnf) {
        CL_DEBUG("parseConfigString: \"track_uninit\" mode requested");
        data.trackUninit = true;
        return;
    }

    // TODO: make it possible to handle both parameters together (comma
    // separated list or whatever)
    // TODO: document all the parameters somewhere
    if (string("noplot") == cnf) {
        CL_DEBUG("parseConfigString: \"noplot\" mode requested");
        data.skipUserPlots = true;
        return;
    }

    const char *cstr = cnf.c_str();
    const char *elPrefix = "error_label:";
    const size_t elPrefixLen = strlen(elPrefix);
    if (!strncmp(cstr, elPrefix, elPrefixLen)) {
        cstr += elPrefixLen;
        CL_DEBUG("parseConfigString: error label is \"" << cstr << "\"");
        data.errLabel = cstr;
        return;
    }

    CL_WARN("unhandled config string: \"" << cnf << "\"");
}

} // namespace GlConf
