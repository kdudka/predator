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
#include "forestautext.hh"
#include "memplot.hh"
#include "plotenum.hh"
#include "symstate.hh"

#define FA_QUOTE(what) "\"" << what << "\""

// anonymous namespace
namespace
{
/// visitor that outputs the visited object into an output stream in the Dot
/// format
class DotPlotVisitor
{
private:  // data members

	/// the output stream
	std::ostream& os_;

	/// counter for transition numbers
	size_t transCnt_;

private:  // methods

	DotPlotVisitor(const DotPlotVisitor&);
	DotPlotVisitor& operator=(const DotPlotVisitor&);

	static std::string dataToDot(const Data& data)
	{
		std::ostringstream os;
		switch (data.type)
		{
			case data_type_e::t_undef:
			{
			  os << FA_QUOTE(data);
			  break;
			}

			case data_type_e::t_unknw:
			{
			  os << FA_QUOTE(data);
			  break;
			}

			case data_type_e::t_native_ptr:
			{
			  os << FA_QUOTE(data);
			  break;
			}

			case data_type_e::t_void_ptr:
			{
			  os << FA_QUOTE(data);
			  break;
			}

			case data_type_e::t_ref:
			{
			  os << FA_QUOTE("treeaut" << data.d_ref.root);
			  break;
			}

			case data_type_e::t_int:
			{
			  os << FA_QUOTE(data);
			  break;
			}

			case data_type_e::t_bool:
			{
			  os << FA_QUOTE((data.d_bool? "TRUE" : "FALSE"));
			  break;
			}

			case data_type_e::t_struct:
			{
			  os << FA_QUOTE(data);
			  break;
			}

			case data_type_e::t_other:
			{
			  os << FA_QUOTE(data);
			  break;
			}

			default:
			{
				assert(false);        // fail gracefully
			}
		}

		return os.str();
	}

	static std::string stateToString(const size_t state)
	{
		std::ostringstream os;

		if (_MSB_TEST(state))
			os << 'r' << _MSB_GET(state);
		else
			os << 'q' << state;

		return os.str();
	}

public:   // methods

	DotPlotVisitor(std::ostream& os) :
		os_(os),
		transCnt_{0}
	{ }

	void operator()(const ExecState& state)
	{
		const DataArray& regs = state.GetRegs();
		for (size_t i = 0; i < regs.size(); ++i)
		{
			os_ << "  " << FA_QUOTE("reg" << i) << " -> " << dataToDot(regs[i]) << ";\n";
		}

		os_ << "\n";

		state.GetMem()->GetFAE()->accept(*this);
	}

	void operator()(const FA& fa)
	{
		const DataArray& vars = fa.GetVariables();
		for (size_t i = 0; i < vars.size(); ++i)
		{
			os_ << "  " << FA_QUOTE("greg" << i) << " -> " << dataToDot(vars[i]) << ";\n";
		}

		os_ << "\n";

		for (size_t i = 0; i < fa.getRootCount(); ++i)
		{
			os_ << "  subgraph " << FA_QUOTE("cluster_treeaut" << i) <<  " {\n"
				<< "    rank=same;\n"
				<< "    label=\"\";\n"
				<< "    color=black;\n"
				<< "    fontcolor=black;\n"
				<< "    bgcolor=gray98;\n"
				<< "    style=dashed;\n"
				<< "    penwidth=1.0;\n"
				<< "\n"
				<< "    " << FA_QUOTE("treeaut" << i) << ";\n";

				fa.getRoot(i)->accept(*this);

			os_ << "\n  }\n";
		}
	}

	void operator()(const TreeAut& ta)
	{
		for (const TreeAut::Transition& trans : ta)
		{
			trans.accept(*this);
		}
	}

	void operator()(const TTBase<label_type>& trans)
	{
		os_ << "    subgraph " << FA_QUOTE("cluster_trans" << transCnt_++) << " {\n"
			<< "      rank=same;\n"
			<< "      label=\"\";\n"
			<< "      color=black;\n"
			<< "      fontcolor=black;\n"
			<< "      bgcolor=orange;\n"
			<< "      style=dashed;\n"
			<< "      penwidth=1.0;\n\n";

		const NodeLabel& label = *trans.label();

//		os_ << "      " << FA_QUOTE(stateToString(state)) << " -> "
//			<< FA_QUOTE(&label) << ";\n";

		switch (label.type)
		{
			case NodeLabel::node_type::n_unknown:
			{
				os_ << "      " << FA_QUOTE(&label) << " [shape=box, label="
					<< FA_QUOTE("<unknown>") << "];\n";
				break;
			}

			case NodeLabel::node_type::n_node:
			{
				// Assertions
				assert(nullptr != label.node.v);
				assert(nullptr != label.node.m);
				assert(nullptr != label.node.sels);

				const std::vector<const AbstractBox*>& boxes = *label.node.v;
				assert(!boxes.empty());

				os_ << "      " << FA_QUOTE(&label) << " [shape=ellipse, label="
					<< FA_QUOTE(*boxes[0]) << "];\n";

				for (size_t i = 1; i < boxes.size(); ++i)
				{
					assert(trans.lhs.size() <= i);

					os_ << "      " << FA_QUOTE(boxes[i]) << " [shape=box, label="
						<< FA_QUOTE(*(boxes[i])) << "];\n";

					os_ << "      " << FA_QUOTE(boxes[i]) << " -> "
						<< FA_QUOTE(stateToString(trans.lhs()[i-1])) << ";\n";
				}

				break;
			}

			case NodeLabel::node_type::n_data:
			{
				// Assertions
				assert(nullptr != label.data.data);
				assert(0 == trans.lhs.size());

				os_ << "      " << FA_QUOTE(stateToString(trans.rhs()))
					<< " [shape=box, label=" << dataToDot(*label.data.data) << "]\n";

				break;
			}

			case NodeLabel::node_type::n_vData: break;
			{
				// Assertions
				assert(nullptr != label.vData);
				assert(0 == trans.lhs.size());

				os_ << "      " << FA_QUOTE(&label) << " [shape=box, label="
					<< FA_QUOTE(label) << "];\n";

				for (const Data& data : *label.vData)
				{
					os_ << "      " << FA_QUOTE(&data) << " [shape=box, label="
						<< dataToDot(data) << "]\n";
					os_ << "      " << FA_QUOTE(stateToString(trans.rhs())) << " -> "
						<< FA_QUOTE(&data) << ";\n";
				}

				break;
			}

			default:
			{
				assert(false);    // fail gracefully
			}
		}

		os_ << "\n    }\n";
	}
};
} /* namespace */


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
	out << "digraph " << FA_QUOTE(plotName) << " {\n"
		<< "  label=<<FONT POINT-SIZE=\"18\">" << plotName << "</FONT>>;\n"
		<< "  clusterrank=local;\n"
		<< "  labelloc=t;\n\n";

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
