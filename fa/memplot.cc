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
#include <cstring>
#include <fstream>
#include <libgen.h>

// Forester headers
#include "exec_state.hh"
#include "forestautext.hh"
#include "memplot.hh"
#include "plotenum.hh"
#include "streams.hh"
#include "symstate.hh"

#define FA_QUOTE(what) "\"" << what << "\""

// anonymous namespace
namespace
{

typedef TTBase<label_type> BaseTransition;
typedef TreeAut::Transition Transition;

std::string stateToString(const size_t state)
{
	std::ostringstream os;

	if (_MSB_TEST(state))
		os << 'r' << _MSB_GET(state);
	else
		os << 'q' << state;

	return os.str();
}

/// class for a memory node
class MemNode
{
public:   // data types

	enum class mem_type : unsigned char
	{
		t_block,
		t_treeref,
		t_datafield
	};

	struct SelectorData
	{
		std::string   name;
		size_t        targetState;

		SelectorData(const std::string& name, size_t targetState) :
			name(name), targetState(targetState)
		{ }
	};

	typedef std::vector<std::pair<SelData, SelectorData>> SelectorVec;


public:   // data members

	/// the ID of the node
	size_t id_;

	/// the type of the memory node
	mem_type type_;


// the following fields are mutually exclusive (impossible to make as a union in C++)
//union
//{

	/// data fields for a block
	struct
	{
		/// the name of the memory node
		std::string name;

		/// vector of selectors inside a memory node
		SelectorVec selVec;
	} block_;

	/// data fields for a tree reference
	size_t treeref_;

	/// the string of the data field
	std::string dataField_;

//} // union


private:  // methods

	MemNode(size_t id, mem_type type) :
		id_{id},
		type_{type},
		block_{std::string(), SelectorVec()},
		treeref_{},
		dataField_{}
	{ }

public:   // methods

	MemNode(const MemNode& node) :
		id_{node.id_},
		type_{node.type_},
		block_(node.block_),
		treeref_{node.treeref_},
		dataField_{node.dataField_}
	{ }

	static MemNode createBlock(size_t nodeId, const std::string& name)
	{
		MemNode node(nodeId, mem_type::t_block);
		node.block_.name = name;

		return node;
	}

	static MemNode createDataField(size_t nodeId, const std::string& dataStr)
	{
		MemNode node(nodeId, mem_type::t_datafield);
		node.dataField_ = dataStr;

		return node;
	}

	static MemNode createTreeRef(size_t nodeId, size_t treeref)
	{
		MemNode node(nodeId, mem_type::t_treeref);
		node.treeref_ = treeref;

		return node;
	}
};

struct TreeAutHeap
{
	/// maps states to all memory nodes into which they can point
	typedef std::multimap<size_t /* state */, MemNode> StateToMemNodeMap;

	/**
	 * @brief  The ID of the root memory node of the tree automaton
	 *
	 * Zero indicates an invalid objact.
	 */
	size_t rootNodeID;

	/// for states in the given tree automaton
	StateToMemNodeMap stateMap;

	explicit TreeAutHeap(size_t rootNode = 0) :
		rootNodeID{rootNode},
		stateMap{}
	{ }
};

inline size_t getRandomID()
{
	return static_cast<size_t>(rand() - std::numeric_limits<int>::min());
}

inline size_t getTransID(const BaseTransition& trans)
{
	return reinterpret_cast<size_t>(&trans);
}

/// visitor that outputs the visited object into an output stream in the Dot
/// format
class DotPlotVisitor
{
private:  // data members

	/// the output stream
	std::ostream& os_;

	/// the location to be plotted
	const cl_loc* loc_;

	/// maps root numbers to tree automata representation of heap
	std::vector<TreeAutHeap> vecTreeAut_;

	/// vector of pointers
	std::vector<std::pair<std::string /* src */, std::string /* dst */>> pointers_;

private:  // methods

	DotPlotVisitor(const DotPlotVisitor&);
	DotPlotVisitor& operator=(const DotPlotVisitor&);

	void addPointer(const std::string& src, const std::string& dst)
	{
		pointers_.push_back(std::make_pair(src, dst));
	}

	void addStateToMemNodeLink(size_t state, const MemNode& node)
	{
		assert(!vecTreeAut_.empty());

		vecTreeAut_.rbegin()->stateMap.insert(std::make_pair(state, node));
	}

	static MemNode dataToMemNode(size_t transID, const Data& data)
	{
		switch (data.type)
		{
			case data_type_e::t_undef:
			case data_type_e::t_native_ptr:
			case data_type_e::t_int:
			{
				std::ostringstream os;
				os << data;
				return MemNode::createDataField(transID, os.str());
			}

			case data_type_e::t_unknw:
			{
				assert(false);     // not supported
			  break;
			}

			case data_type_e::t_void_ptr:
			{
				assert(false);     // not supported
			  break;
			}

			case data_type_e::t_ref:
			{
				return MemNode::createTreeRef(transID, data.d_ref.root);
			}

			case data_type_e::t_bool:
			{
				assert(false);     // not supported
			  break;
			}

			case data_type_e::t_struct:
			{
				assert(false);     // not supported
			  break;
			}

			case data_type_e::t_other:
			{
				assert(false);     // not supported
			  break;
			}

			default:
			{
				assert(false);        // fail gracefully
			}
		}

		// in case we fall through in the non-DEBUG build
		throw std::runtime_error("Unsupported for now");
	}

public:   // methods

	DotPlotVisitor(std::ostream& os, const cl_loc* loc) :
		os_(os),
		loc_(loc),
		vecTreeAut_{},
		pointers_{}
	{ }

	void operator()(const ExecState& state)
	{
		assert(nullptr != state.GetMem());
		state.GetMem()->accept(*this);

//		const DataArray& regs = state.GetRegs();
//		for (size_t i = 0; i < regs.size(); ++i)
//		{
//			os_ << "  " << FA_QUOTE("reg" << i) << " -> " << dataToDot(regs[i]) << ";\n";
//		}
//
//		os_ << "\n";
	}

	void operator()(const SymState& state)
	{
		state.GetFAE()->accept(*this);
	}

	void operator()(const FA& fa)
	{
//		const DataArray& vars = fa.GetVariables();
//		for (size_t i = 0; i < vars.size(); ++i)
//		{
//			os_ << "  " << FA_QUOTE("greg" << i) << " -> " << dataToDot(vars[i]) << ";\n";
//		}
//
//		os_ << "\n";
//
//		// in the first traversal, create tree automata heap representation and set
//		// the root memory node
//		for (size_t i = 0; i < fa.getRootCount(); ++i)
//		{
//			assert(vecTreeAut_.size() == i);
//
//			TreeAutHeap taHeap;
//
//			if (nullptr != fa.getRoot(i))
//			{
//				const TreeAut& ta = *fa.getRoot(i);
//				assert(ta.accBegin() != ta.accEnd());
//
//				if (++(ta.accBegin()) != ta.accEnd())
//				{
//					FA_NOTE("More accepting transitions! Considering only the first...");
//				}
//
//				const Transition& trans = *ta.accBegin();
//				taHeap.rootNodeID = getTransID(trans);
//			}
//
//			vecTreeAut_.push_back(taHeap);
//		}

		for (size_t i = 0; i < fa.getRootCount(); ++i)
		{
			// Assertions
			assert(vecTreeAut_.size() == i);

			if (nullptr != fa.getRoot(i))
			{
				const TreeAut& ta = *fa.getRoot(i);
				assert(ta.accBegin() != ta.accEnd());

				if (++(ta.accBegin()) != ta.accEnd())
				{
					FA_WARN_MSG(loc_,
						"More accepting transitions! Considering only the first...");
				}

				const Transition& trans = *ta.accBegin();

				TreeAutHeap taHeap(getTransID(trans));
				vecTreeAut_.push_back(taHeap);

				// process the tree automaton
				ta.accept(*this);
			}
			else
			{
				vecTreeAut_.push_back(TreeAutHeap());
			}
		}
	}

	void operator()(const TreeAut& ta)
	{
		for (const Transition& trans : ta)
		{
			trans.accept(*this);
		}
	}

	void operator()(const BaseTransition& trans)
	{
		const NodeLabel& label = *trans.label();

		switch (label.type)
		{
			case NodeLabel::node_type::n_unknown:
			{
				assert(false);      // not supported
				break;
			}

			case NodeLabel::node_type::n_node:
			{
				// Assertions
				assert(nullptr != label.node.v);
				assert(nullptr != label.node.m);
				assert(nullptr != label.node.sels);

				const std::vector<const AbstractBox*>& boxes = *label.node.v;
				const std::vector<SelData>& sels             = *label.node.sels;

				// Assertions
				assert(!boxes.empty());
				assert(nullptr != boxes[0]);
				assert(boxes[0]->isType(box_type_e::bTypeInfo));

				const TypeBox& type = *static_cast<const TypeBox*>(boxes[0]);
				MemNode node = MemNode::createBlock(getTransID(trans), type.getName());

				for (size_t i = 1; i < boxes.size(); ++i)
				{	// go over all selectors (and boxes)
					// Assertions
					assert(i   <= trans.lhs().size());
					assert(i-1 <  sels.size());
					assert(nullptr != boxes[i]);
					assert(boxes[i]->isType(box_type_e::bSel));

					// FIXME: this is not correct
					MemNode::SelectorData sel(sels[i-1].name, trans.lhs()[i-1]);
					node.block_.selVec.push_back(std::make_pair(sels[i-1], sel));
				}

				this->addStateToMemNodeLink(trans.rhs(), node);
				break;
			}

			case NodeLabel::node_type::n_data:
			{
				// Assertions
				assert(nullptr != label.data.data);
				assert(trans.lhs().empty());

				const Data& data = *label.data.data;

				this->addStateToMemNodeLink(trans.rhs(),
					dataToMemNode(getRandomID(), data));
				break;
			}

			case NodeLabel::node_type::n_vData:
			{
				assert(false);     // not supported
				break;
			}

			default:
			{
				assert(false);    // fail gracefully
			}
		}
	}

	void plotMemNode(
		const MemNode& node,
		const TreeAutHeap::StateToMemNodeMap& stateMap)
	{
		switch (node.type_)
		{
			case MemNode::mem_type::t_block:
			{
				os_ << "      " << FA_QUOTE(node.id_)
					<< " [shape=ellipse, style=filled, fillcolor=lightblue, label="
					<< FA_QUOTE(node.block_.name) << "];\n";

				break;
			}

			case MemNode::mem_type::t_treeref:
			{	// do not print anything for tree references
				break;
			}

			case MemNode::mem_type::t_datafield:
			{
				os_ << "      " << FA_QUOTE(node.id_)
					<< " [shape=box, style=filled, fillcolor=red, label="
					<< FA_QUOTE(node.dataField_) << "];\n";

				break;
			}

			default:
			{
				assert(false);          // fail gracefully
				break;
			}
		}

		for (const auto& selSelectorPair : node.block_.selVec)
		{
			const SelData& selData = selSelectorPair.first;
			const MemNode::SelectorData& sel = selSelectorPair.second;

			// get the ID of a selector
			std::ostringstream oss;
			oss << node.id_ << "." << selData.offset;
			std::string selId = oss.str();

			os_ << "      " << FA_QUOTE(selId)
				<< " [shape=box, style=filled, fillcolor=pink, label="
				<< FA_QUOTE(sel.name) << "];\n";

			os_ << "      " << FA_QUOTE(node.id_) << " -> " << FA_QUOTE(selId)
				<< "[label=" << FA_QUOTE("["
				<< selData.offset << ":"
				<< selData.size << ":"
				<< ((selData.displ >= 0)? "+":"") << selData.displ
				<< "]") << "];\n";

			for (auto beginEndItPair = stateMap.equal_range(sel.targetState);
				beginEndItPair.first != beginEndItPair.second;
				++(beginEndItPair.first))
			{
				const MemNode& tmpNode = (*beginEndItPair.first).second;

				std::ostringstream tmpOs;
				if (MemNode::mem_type::t_treeref == tmpNode.type_)
				{	// if the node is a tree automaton reference
					const TreeAutHeap& taHeap = vecTreeAut_[tmpNode.treeref_];

					tmpOs << taHeap.rootNodeID;
				}
				else
				{	// in case the node is anything but the tree automaton reference
					tmpOs << tmpNode.id_;
				}

				this->addPointer(selId, tmpOs.str());
			}
		}
	}

	void plotTreeAutHeapNum(size_t num)
	{
		// Assertions
		assert(num < vecTreeAut_.size());

		const TreeAutHeap& taHeap = vecTreeAut_[num];

		for (const auto& stateMemNodePair : taHeap.stateMap)
		{
			const MemNode& node = stateMemNodePair.second;

			os_ << "    subgraph "
				<< FA_QUOTE("cluster_treeaut" << num << "_"
				<< stateToString(stateMemNodePair.first)) << " {\n"
				<< "      rank=same;\n"
				<< "      label=" << stateToString(stateMemNodePair.first) << ";\n"
				<< "      labeljust=left;\n"
				<< "      color=black;\n"
				<< "      fontcolor=black;\n"
				<< "      bgcolor=orange;\n"
				<< "      style=dashed;\n"
				<< "      penwidth=1.0;\n\n";

			this->plotMemNode(node, taHeap.stateMap);

			os_ << "    }\n\n";
		}
	}

	void plotPointers() const
	{
		for (const auto& srcDstPair : pointers_)
		{
			os_ << "  " << FA_QUOTE(srcDstPair.first)
				<< " -> " << FA_QUOTE(srcDstPair.second)
				<< ";\n";
		}
	}

	void plot()
	{
		for (size_t i = 0; i < vecTreeAut_.size(); ++i)
		{
			os_ << "  subgraph "
				<< FA_QUOTE("cluster_treeaut" << i) << " {\n"
				<< "    rank=same;\n"
				<< "    label=" << FA_QUOTE("TA " << i) << ";\n"
				<< "    labeljust=right;\n"
				<< "    color=black;\n"
				<< "    fontcolor=black;\n"
				<< "    bgcolor=green;\n"
				<< "    style=dashed;\n"
				<< "    penwidth=1.0;\n\n";

			this->plotTreeAutHeapNum(i);

			os_ << "  }\n\n";
		}

		this->plotPointers();
	}
};

void emitPrototypeError(const struct cl_loc *lw, const char *name)
{
	FA_WARN_MSG(lw, "incorrectly called " << name
			<< "() not recognized as built-in");
}

} /* namespace */


/**
 * @brief  Generates plot filename from location info
 *
 * Generates plot filename from location info.
 *
 * @param[in]  loc  Location info
 *
 * @returns  Filename of the plot
 */
std::string MemPlotter::generatePlotName(
	const struct cl_loc*              loc)
{
	if (!loc || !loc->file) {
		// sorry, no location info here
		return "anonplot";
	}

	char *dup = strdup(loc->file);
	const char *fname = basename(dup);

	std::ostringstream os;
	os << fname << "-" << loc->line;

	free(dup);

	return os.str();
}

// Shamelessly copied from Predator
bool MemPlotter::readPlotName(
	std::string*                      dst,
	const CodeStorage::TOperandList&  opList,
	const struct cl_loc*              loc)
{
	const cl_operand &op = opList[/* dst + fnc */ 2];
	if (CL_OPERAND_CST != op.code)
		return false;

	const cl_cst &cst = op.data.cst;
	if (CL_TYPE_STRING == cst.code) {
		// plot name given as a string literal
		*dst = cst.data.cst_string.value;
		return true;
	}

	if (CL_TYPE_INT != cst.code || cst.data.cst_int.value)
		// no match
		return false;

	// NULL given as plot name, we're asked to generate the name automagically
	*dst = generatePlotName(loc);
	return true;
}


std::string MemPlotter::handlePlot(
	const SymState&          state,
	const CodeStorage::Insn  &insn)
{
	const CodeStorage::TOperandList &opList = insn.operands;
	const cl_loc& loc = insn.loc;

	const char* name = "___fa_plot";

	const int cntArgs = opList.size() - /* dst + fnc */ 2;
	if (cntArgs != 1) {
		emitPrototypeError(&loc, name);
		// wrong count of arguments
		return std::string();
	}

	if (CL_OPERAND_VOID != opList[/* dst */ 0].code) {
		// not a function returning void
		emitPrototypeError(&loc, name);
		return std::string();
	}

	std::string plotName;
	if (!readPlotName(&plotName, opList, &loc)) {
		emitPrototypeError(&loc, name);
		return std::string();
	}

	return plotHeap(state, plotName, &loc);
}


std::string MemPlotter::plotHeap(
	const SymState&      state,
	const std::string&   name,
	const struct cl_loc  *loc)
{
	std::string plotName = PlotEnumerator::instance()->decorate(name);
	std::string fileName = plotName + ".dot";

	// create a dot file
	std::fstream out(fileName.c_str(), std::ios::out);
	if (!out)
	{
		FA_WARN("unable to create file '" << fileName << "'");
		return fileName;
	}

	// open graph
	out << "digraph " << FA_QUOTE(plotName) << " {\n"
		<< "  label=<<FONT POINT-SIZE=\"18\">" << plotName << "</FONT>>;\n"
		<< "  clusterrank=local;\n"
		<< "  labelloc=t;\n\n";

	// check whether we can write to stream
	if (!out.flush())
	{
		FA_WARN("unable to write file '" << fileName << "'");
		out.close();
		return fileName;
	}

	if (loc)
		FA_NOTE_MSG(loc, "writing memory graph to '" << fileName << "'...");
	else
		FA_DEBUG("writing memory graph to '" << fileName << "'...");

	DotPlotVisitor visitor(out, loc);

	state.accept(visitor);

	visitor.plot();

	// close graph
	out << "}\n";
	if (!out)
	{
		FA_WARN("Error writing to file " << fileName);
	}
	out.close();

	return fileName;
}

std::string MemPlotter::plotHeap(
	const SymState&        state)
{
	const struct cl_loc* loc = nullptr;
	if (state.GetInstr() && state.GetInstr()->insn())
	{
		loc = &state.GetInstr()->insn()->loc;
	}

	return plotHeap(state, generatePlotName(loc), loc);
}
