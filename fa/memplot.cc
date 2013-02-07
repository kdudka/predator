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
#include "forestautext.hh"
#include "memplot.hh"
#include "plotenum.hh"
#include "streams.hh"
#include "symstate.hh"

#define FA_QUOTE(what) "\"" << what << "\""

// anonymous namespace
namespace
{
/// counter for unique values
size_t uniqCnt = 0;

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

struct Pointer
{
	std::string src;      ///< Source of the pointer
	std::string dst;      ///< Destination of the pointer
	int offset;           ///< Offset into the destination

	Pointer(
		const std::string&    pSrc,
		const std::string&    pDst,
		int                   pOffset) :
		src(pSrc),
		dst(pDst),
		offset(pOffset)
	{ }
};

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

		friend std::ostream& operator<<(std::ostream& os, const SelectorData& sel)
		{
			os << sel.name;

			return os;
		}
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
	struct BlockFields
	{
		std::string name;        ///< the name of the memory node
		SelectorVec selVec;      ///< vector of selectors inside a memory node

		BlockFields(const std::string& pName, const SelectorVec& pSelVec) :
			name(pName),
			selVec(pSelVec)
		{ }
	} block_;

	/// data fields for a tree reference
	struct
	{
		size_t root;             ///< the index of the root
		int offset;              ///< the offset from the base of the root
	} treeref_;

	/// the string of the data field
	std::string dataField_;

//} // union


private:  // methods

	MemNode(size_t id, mem_type type) :
		id_{id},
		type_{type},
		block_{std::string(), SelectorVec()},
		treeref_{0, 0},
		dataField_{}
	{ }

public:   // methods

	MemNode(const MemNode& node) :
		id_{node.id_},
		type_{node.type_},
		block_(node.block_),
		treeref_(node.treeref_),
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

	static MemNode createTreeRef(size_t nodeId, size_t root, int offset)
	{
		MemNode node(nodeId, mem_type::t_treeref);
		node.treeref_.root   = root;
		node.treeref_.offset = offset;

		return node;
	}

	friend std::ostream& operator<<(std::ostream& os, const MemNode& node)
	{
		os << "Node " << node.id_ << "\n";
		os << " type: ";

		switch (node.type_)
		{
			case mem_type::t_block:
				os << "block " << node.block_.name << "(";
				for (auto it = node.block_.selVec.cbegin(); it != node.block_.selVec.cend(); ++it)
				{	// for all selectors
					if (node.block_.selVec.cbegin() != it)
					{
						os << ", ";
					}

					os << it->second;
				}

				os << ")";
				break;

			case mem_type::t_datafield:
				os << "data";
				break;

			case mem_type::t_treeref:
				os << "treeref";
				break;

			default:
				assert(false);         // fail gracefully
		}

		os << "\n";


		return os;
	}
};

class TreeAutHeap
{
public:   // data types

	/// maps states to all memory nodes into which they can point
	typedef std::multimap<size_t /* state */, MemNode> StateToMemNodeMap;

	/// container for root node IDs
	typedef std::vector<size_t> RootNodeIDList;

private:  // data members

	/**
	 * @brief  The IDs of the root memory node of the tree automaton
	 *
	 * Contains all possible root nodes' IDs (a single for normalised forest
	 * automata). Empty list denotes an invalid tree automaton.
	 */
	RootNodeIDList rootNodeIDs_;

	/// for states in the given tree automaton
	StateToMemNodeMap stateMap_;

public:   // methods

	/// The constructor
	TreeAutHeap() :
		rootNodeIDs_{},
		stateMap_{}
	{ }

	/// adds a mapping of state to a memory node
	void addStateToNodeMapping(size_t state, const MemNode& node)
	{
		stateMap_.insert(std::make_pair(state, node));
	}

	/// adds another root node
	void addRootNodeID(size_t rootNodeID)
	{
		rootNodeIDs_.push_back(rootNodeID);
	}

	/// @p true if the structure represents a valid heap component
	bool valid() const
	{
		return !rootNodeIDs_.empty();
	}

	const RootNodeIDList& getRootNodeIDs() const
	{
		return rootNodeIDs_;
	}

	const StateToMemNodeMap& getStateMap() const
	{
		return stateMap_;
	}
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
	std::vector<Pointer> pointers_;

private:  // methods

	DotPlotVisitor(const DotPlotVisitor&);
	DotPlotVisitor& operator=(const DotPlotVisitor&);

	void addPointer(const std::string& src, const std::string& dst, int offset)
	{
		pointers_.push_back(Pointer(src, dst, offset));
	}

	void addStateToMemNodeLink(size_t state, const MemNode& node)
	{
		assert(!vecTreeAut_.empty());

		vecTreeAut_.rbegin()->addStateToNodeMapping(state, node);
	}

	static MemNode dataToMemNode(size_t transID, const Data& data)
	{
		switch (data.type)
		{
			case data_type_e::t_undef:
			case data_type_e::t_native_ptr:
			case data_type_e::t_bool:
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
				std::ostringstream os;
				os << "(void*) [size=" << data.d_void_ptr_size << " B]";
				return MemNode::createDataField(transID, os.str());
			}

			case data_type_e::t_ref:
			{
				return MemNode::createTreeRef(transID, data.d_ref.root, data.d_ref.displ);
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

	void operator()(const SymState& state)
	{
		state.GetFAE()->accept(*this);
	}

	void operator()(const FA& fa)
	{
		for (size_t i = 0; i < fa.getRootCount(); ++i)
		{
			// Assertions
			assert(vecTreeAut_.size() == i);

			if (nullptr != fa.getRoot(i))
			{
				const TreeAut& ta = *fa.getRoot(i);
				assert(ta.accBegin() != ta.accEnd());

				TreeAutHeap taHeap;
				for (auto it = ta.accBegin(); it != ta.accEnd(); ++it)
				{
					const Transition& trans = *it;
					taHeap.addRootNodeID(getTransID(trans));
				}

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

		switch (label.GetType())
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

				const std::vector<const AbstractBox*>& boxes = *label.node.v;
				const std::vector<SelData>* sels             = label.node.sels;

				// Assertions
				assert(!boxes.empty());
				assert(nullptr != boxes[0]);
				assert(boxes[0]->isType(box_type_e::bTypeInfo));

				const TypeBox& type = *static_cast<const TypeBox*>(boxes[0]);
				MemNode node = MemNode::createBlock(getTransID(trans), type.getName());

				for (size_t i = 1; i < boxes.size(); ++i)
				{	// go over all selectors (and boxes)
					// Assertions
					assert(i <= trans.lhs().size());
					assert((nullptr == sels) || (i-1 < sels->size()));
					assert(nullptr != boxes[i]);
					assert(boxes[i]->isStructural());

					if (boxes[i]->isSelector())
					{	// for selectors
						if (nullptr != sels)
						{	// if the selectors are nicely named
							// FIXME: this is not correct
							MemNode::SelectorData sel((*sels)[i-1].name, trans.lhs()[i-1]);
							node.block_.selVec.push_back(std::make_pair((*sels)[i-1], sel));
						}
						else
						{	// otherwise we need to manage somehow else
							const SelBox* selBox = static_cast<const SelBox*>(boxes[i]);

							MemNode::SelectorData sel(selBox->getData().name, trans.lhs()[i-1]);
							node.block_.selVec.push_back(std::make_pair(selBox->getData(), sel));
						}
					}
					else if (boxes[i]->isBox())
					{	// for hierarchical boxes
						const Box& box = *static_cast<const Box*>(boxes[i]);

						std::ostringstream osBox;
						box.toStream(osBox);
						// FIXME: this is also not correct
						MemNode::SelectorData sel(osBox.str(), trans.lhs()[i-1]);

						// get the lowest output selector offset in the box
						assert(!box.outputCoverage().empty());
						size_t selOffset = *box.outputCoverage().cbegin();

						node.block_.selVec.push_back(
							std::make_pair(SelData(selOffset, 0, 0, osBox.str()), sel)
							);
					}
					else
					{	// undefined
						assert(false);       // fail gracefully
					}
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
		const MemNode&                          node,
		const TreeAutHeap::StateToMemNodeMap&   stateMap)
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

				if (MemNode::mem_type::t_treeref == tmpNode.type_)
				{	// if the node is a tree automaton reference
					const TreeAutHeap& taHeap = vecTreeAut_[tmpNode.treeref_.root];

					if (!taHeap.valid())
					{
						FA_DEBUG("Plotting invalid TA!");
					}

					const TreeAutHeap::RootNodeIDList& rootNodeIDs = taHeap.getRootNodeIDs();
					for (auto it = rootNodeIDs.cbegin(); it != rootNodeIDs.cend(); ++it)
					{
						std::ostringstream tmpOs;
						tmpOs << *it;
						this->addPointer(selId, tmpOs.str(), tmpNode.treeref_.offset);
					}
				}
				else
				{	// in case the node is anything but the tree automaton reference
					std::ostringstream tmpOs;
					tmpOs << tmpNode.id_;
					this->addPointer(selId, tmpOs.str(), 0);
				}
			}
		}
	}

	void plotTreeAutHeapNum(size_t num)
	{
		// Assertions
		assert(num < vecTreeAut_.size());

		const TreeAutHeap& taHeap = vecTreeAut_[num];

		for (const auto& stateMemNodePair : taHeap.getStateMap())
		{
			const MemNode& node = stateMemNodePair.second;

			os_ << "    subgraph "
				<< FA_QUOTE("cluster_treeaut" << num << "_"
				<< stateToString(stateMemNodePair.first) << "_" << uniqCnt++) << " {\n"
				<< "      rank=same;\n"
				<< "      label=" << stateToString(stateMemNodePair.first) << ";\n"
				<< "      labeljust=left;\n"
				<< "      color=black;\n"
				<< "      fontcolor=black;\n"
				<< "      bgcolor=orange;\n"
				<< "      style=dashed;\n"
				<< "      penwidth=1.0;\n\n";

			this->plotMemNode(node, taHeap.getStateMap());

			os_ << "    }\n\n";
		}
	}

	void plotPointers() const
	{
		for (const auto& ptr : pointers_)
		{
			os_ << "  " << FA_QUOTE(ptr.src)
				<< " -> " << FA_QUOTE(ptr.dst);

			if (0 != ptr.offset)
			{
				os_ << " [label="
				<< FA_QUOTE("[" << ((ptr.offset > 0)? "+" : "") << ptr.offset << "]")
				<< "]";
			}

			os_ << ";\n";
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
