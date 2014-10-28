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
#include <boost/lexical_cast.hpp>

namespace GlConf {

Options data;

Options::Options():
    trackUninit(false),
    oomSimulation(false),
    memLeakIsError(false),
    skipUserPlots(false),
    errorRecoveryMode(SE_ERROR_RECOVERY_MODE),
    allowCyclicTraceGraph(SE_ALLOW_CYCLIC_TRACE_GRAPH),
    allowThreeWayJoin(SE_ALLOW_THREE_WAY_JOIN),
    forbidHeapReplace(SE_FORBID_HEAP_REPLACE),
    intArithmeticLimit(SE_INT_ARITHMETIC_LIMIT),
    joinOnLoopEdgesOnly(SE_JOIN_ON_LOOP_EDGES_ONLY),
    stateLiveOrdering(SE_STATE_ON_THE_FLY_ORDERING),
    detectContainers(false),
    fixedPoint(0)
{
}

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

void handleAllowThreeWayJoin(const string &name, const string &value)
{
    if (value.empty()) {
        data.allowThreeWayJoin = /* allow everything */ 3;
        return;
    }

    try {
        data.allowThreeWayJoin = boost::lexical_cast<int>(value);
        if (data.allowThreeWayJoin < 0)
            data.allowThreeWayJoin = 0;
        if (data.allowThreeWayJoin > 3)
            data.allowThreeWayJoin = 3;
    }
    catch (...) {
        CL_WARN("ignoring option \"" << name << "\" with invalid value");
        return;
    }
}

void handleJoinOnLoopEdgesOnly(const string &name, const string &value)
{
    if (value.empty()) {
        data.joinOnLoopEdgesOnly = /* skip isomorphism check if possible */ 3;
        return;
    }

    try {
        data.joinOnLoopEdgesOnly = boost::lexical_cast<int>(value);
        if (data.joinOnLoopEdgesOnly < 0)
            data.joinOnLoopEdgesOnly = 0;
        if (data.joinOnLoopEdgesOnly > 3)
            data.joinOnLoopEdgesOnly = 3;
    }
    catch (...) {
        CL_WARN("ignoring option \"" << name << "\" with invalid value");
        return;
    }
}

void handleStateLiveOrdering(const string &name, const string &value)
{
    if (value.empty()) {
        data.stateLiveOrdering = /* reorder heaps also in SymHeapUnion */ 2;
        return;
    }

    try {
        data.stateLiveOrdering = boost::lexical_cast<int>(value);
        if (data.stateLiveOrdering < 0)
            data.stateLiveOrdering = 0;
        if (data.stateLiveOrdering > 2)
            data.stateLiveOrdering = 2;
    }
    catch (...) {
        CL_WARN("ignoring option \"" << name << "\" with invalid value");
        return;
    }
}

void handleIntArithmeticLimit(const string &name, const string &value)
{
    try {
        data.intArithmeticLimit = boost::lexical_cast<int>(value);
    }
    catch (...) {
        CL_WARN("ignoring option \"" << name << "\" with invalid value");
    }
}

void handleAllowCyclicTraceGraph(const string &name, const string &value)
{
    assumeNoValue(name, value);
    data.allowCyclicTraceGraph = true;
}

void handleForbidHeapReplace(const string &name, const string &value)
{
    assumeNoValue(name, value);
    data.forbidHeapReplace = true;
}

void handleMemLeakIsError(const string &name, const string &value)
{
    assumeNoValue(name, value);
    data.memLeakIsError = true;
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

void handleDetectContainers(const string &name, const string &value)
{
#if !SH_PREVENT_AMBIGUOUS_ENT_ID
    CL_ERROR("option \"" << name << "\" requires SH_PREVENT_AMBIGUOUS_ENT_ID");
    return;
#endif
    data.allowCyclicTraceGraph      = true;
    data.forbidHeapReplace          = true;
    data.detectContainers           = true;
    data.joinOnLoopEdgesOnly        = 1;
    data.intArithmeticLimit         = /* disabled */ 0;
    data.stateLiveOrdering          = /* disabled */ 0;
    data.allowThreeWayJoin          = /* for prototypes only */ 1;
    handleDumpFixedPoint(name, value);
}

ConfigStringParser::ConfigStringParser()
{
    tbl_["allow_cyclic_trace_graph"]= handleAllowCyclicTraceGraph;
    tbl_["allow_three_way_join"]    = handleAllowThreeWayJoin;
    tbl_["dump_fixed_point"]        = handleDumpFixedPoint;
    tbl_["detect_containers"]       = handleDetectContainers;
    tbl_["error_label"]             = handleErrorLabel;
    tbl_["forbid_heap_replace"]     = handleForbidHeapReplace;
    tbl_["int_arithmetic_limit"]    = handleIntArithmeticLimit;
    tbl_["join_on_loop_edges_only"] = handleJoinOnLoopEdgesOnly;
    tbl_["memleak_is_error"]        = handleMemLeakIsError;
    tbl_["no_error_recovery"]       = handleNoErrorRecovery;
    tbl_["no_plot"]                 = handleNoPlot;
    tbl_["oom"]                     = handleOOM;
    tbl_["state_live_ordering"]     = handleStateLiveOrdering;
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
