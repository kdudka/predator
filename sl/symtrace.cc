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

#include "symtrace.hh"

#include <cl/cl_msg.hh>

#include "plotenum.hh"

#include <algorithm>
#include <fstream>

#include <boost/foreach.hpp>

namespace Trace {

// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::Node

Node::~Node() {
    BOOST_FOREACH(Node *parent, parents_)
        parent->notifyDeath(this);
}

void Node::notifyBirth(Node *child) {
    children_.push_back(child);
}

void Node::notifyDeath(Node *child) {
    // remove the dead child from the list
    children_.erase(
            std::remove(children_.begin(), children_.end(), child),
            children_.end());

    if (children_.empty())
        // FIXME: this may cause stack overflow on complex trace graphs
        delete this;
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::NodeHandle

void NodeHandle::notifyBirth(Node *) {
    CL_BREAK_IF("NodeHandle::notifyBirth() is not supposed to be called");
}

void NodeHandle::notifyDeath(Node *) {
    CL_BREAK_IF("NodeHandle::notifyDeath() is not supposed to be called");
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::plotTrace()

struct TracePlotter {
    std::ostream                        &out;

    TracePlotter(std::ostream &out_):
        out(out_)
    {
    }
};

// FIXME: copy-pasted from symplot.cc
#define SL_QUOTE(what) "\"" << what << "\""

void plotTraceCore(TracePlotter &tplot, Node *endPoint) {
    CL_BREAK_IF("please implement");
    (void) tplot;
    (void) endPoint;
}

// FIXME: copy-pasted from symplot.cc
bool plotTrace(Node *endPoint) {
    PlotEnumerator *pe = PlotEnumerator::instance();
    std::string plotName(pe->decorate("symtrace"));
    std::string fileName(plotName + ".dot");

    // create a dot file
    std::fstream out(fileName.c_str(), std::ios::out);
    if (!out) {
        CL_ERROR("unable to create file '" << fileName << "'");
        return false;
    }

    // open graph
    out << "digraph " << SL_QUOTE(plotName)
        << " {\n\tlabel=<<FONT POINT-SIZE=\"18\">" << plotName
        << "</FONT>>;\n\tclusterrank=local;\n\tlabelloc=t;\n";

    // check whether we can write to stream
    if (!out.flush()) {
        CL_ERROR("unable to write file '" << fileName << "'");
        out.close();
        return false;
    }

    // initialize an instance of PlotData
    CL_NOTE("symtrace: trace graph dumped to '" << fileName << "'");
    TracePlotter tplot(out);

    // do our stuff
    plotTraceCore(tplot, endPoint);

    // close graph
    out << "}\n";
    out.close();
    return !!out;
}


} // namespace Trace
