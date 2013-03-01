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

#include "fixed_point_proxy.hh"

#include <cl/cl_msg.hh>

#include <algorithm>
#include <map>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/foreach.hpp>

namespace GlConf {

Options data;

class ConfigStringParser {
    public:
        ConfigStringParser();
        void handleRawOption(const std::string &) const;

    private:
        typedef void (*THandler)(const std::string &, const std::string &);
        typedef std::map<std::string, THandler> TLookup;
        TLookup tbl_;
};

using std::string;

void assumeNoValue(const string &name, const string &value)
{
    if (value.empty())
        return;

    CL_WARN("option \"" << name << "\" takes no value");
}

void handleDumpFixedPoint(const string &name, const string &value)
{
    assumeNoValue(name, value);
    if (data.fixedPoint)
        CL_BREAK_IF("we are leaking an instance of FixedPoint::StateByInsn");

    data.fixedPoint = new FixedPoint::StateByInsn;
}

void handleErrorLabel(const string &name, const string &value)
{
    if (value.empty()) {
        CL_WARN("ignoring option \"" << name << "\" without a valid value");
        return;
    }

    data.errLabel = value;
}

void handleNoErrorRecovery(const string &name, const string &value)
{
    assumeNoValue(name, value);
    data.errorRecoveryMode = /* no_error_recovery */ 0;
}

void handleNoPlot(const string &name, const string &value)
{
    assumeNoValue(name, value);
    data.skipUserPlots = true;
}

void handleOOM(const string &name, const string &value)
{
    assumeNoValue(name, value);
    data.oomSimulation = true;
}

void handleTrackUninit(const string &name, const string &value)
{
    assumeNoValue(name, value);
    data.trackUninit = true;
}

ConfigStringParser::ConfigStringParser()
{
    tbl_["dump_fixed_point"]        = handleDumpFixedPoint;
    tbl_["error_label"]             = handleErrorLabel;
    tbl_["no_error_recovery"]       = handleNoErrorRecovery;
    tbl_["no_plot"]                 = handleNoPlot;
    tbl_["oom"]                     = handleOOM;
    tbl_["track_uninit"]            = handleTrackUninit;
}

void ConfigStringParser::handleRawOption(const string &raw) const
{
    // split the string to name/value, separated by ':'
    string::const_iterator it = std::find(raw.begin(), raw.end(), ':');
    const string name(raw.begin(), it);

    // lookup by name
    const TLookup::const_iterator hit = tbl_.find(name);
    if (tbl_.end() == hit) {
        CL_WARN("unknown option: " << name);
        return;
    }

    if (raw.end() != it)
        // skip ':' in the value part
        ++it;

    const string value(it, raw.end());
    CL_DEBUG("processing an option: name=\""
            << name << "\", value=\""
            << value << "\"");

    // call the appropriate handler
    const THandler hdl = hit->second;
    hdl(name, value);
}

void loadConfigString(const string &cnf)
{
    if (cnf.empty())
        return;

    // split the given config string into comma-separated options
    std::vector<string> opts;
    boost::split(opts, cnf, boost::algorithm::is_any_of(","));

    // go through all options and parse them one by one
    ConfigStringParser parser;
    BOOST_FOREACH(const string &str, opts)
        parser.handleRawOption(str);
}

} // namespace GlConf
