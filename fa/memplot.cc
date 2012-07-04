/*
 * Copyright (C) 2012 Ondrej Lengal
 *
 * This file is part of forester.
 *
 * forester is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * forester is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with forester.  If not, see <http://www.gnu.org/licenses/>.
 */

// Standard library headers
#include <fstream>

// Code Listener headers
#include <cl/cl_msg.hh>

// Forester headers
#include "exec_state.hh"
#include "memplot.hh"
#include "plotenum.hh"

#define FA_QUOTE(what) "\"" << what << "\""

/// visitor that outputs the visited object into an output stream in the Dot
/// format
class DotPlotVisitor
{
private:  // data members

	/// the output stream
	std::ostream& os_;

public:   // methods

	DotPlotVisitor(std::ostream& os) :
		os_(os)
	{ }

	void operator()(const ExecState& state)
	{
		const DataArray& regs = state.GetRegs();
		for (size_t i = 0; i < regs.size(); ++i)
		{
			os_ << FA_QUOTE("r" << i) << " -> " << FA_QUOTE(regs[i]) << "\n";
		}
	}

};


bool MemPlotter::plotHeap(
	const ExecState&     state,
	const std::string&   name,
	const struct cl_loc  *loc)
{
	std::string plotName = PlotEnumerator::instance()->decorate(name);
	std::string fileName = plotName + ".dot";

	// create a dot file
	std::fstream out(fileName.c_str(), std::ios::out);
	if (!out)
	{
		CL_ERROR("unable to create file '" << fileName << "'");
		return false;
	}

	// open graph
	out << "digraph " << FA_QUOTE(plotName)
		<< " {\n\tlabel=<<FONT POINT-SIZE=\"18\">" << plotName
		<< "</FONT>>;\n\tclusterrank=local;\n\tlabelloc=t;\n";

	// check whether we can write to stream
	if (!out.flush())
	{
		CL_ERROR("unable to write file '" << fileName << "'");
		out.close();
		return false;
	}

	if (loc)
		CL_NOTE_MSG(loc, "writing memory graph to '" << fileName << "'...");
	else
		CL_DEBUG("writing memory graph to '" << fileName << "'...");

	DotPlotVisitor visitor(out);

	state.accept(visitor);

	// close graph
	out << "}\n";
	const bool ok = !!out;
	out.close();
	return ok;
}
