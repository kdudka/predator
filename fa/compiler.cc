/*
 * Copyright (C) 2011 Jiri Simacek
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


/**
 * @file compiler.cc
 *
 * File with the implementation of the compiler of microinstructions.
 */


// Standard library headers
#include <sstream>
#include <cstdlib>
#include <list>
#include <unordered_set>

// Code Listener headers
#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>

// Forester headers
#include "notimpl_except.hh"
#include "symctx.hh"
#include "nodebuilder.hh"
#include "call.hh"
#include "jump.hh"
#include "comparison.hh"
#include "fixpoint.hh"
#include "microcode.hh"
#include "regdef.hh"
#include "compiler.hh"


// anonymous namespace
namespace {

/**
 * @brief  Translates operation code of unary operations to string
 */
std::string translUnOpCode(const unsigned code)
{
	switch (code)
	{
		case CL_UNOP_ASSIGN:    return "CL_UNOP_ASSIGN";
		case CL_UNOP_TRUTH_NOT: return "CL_UNOP_TRUTH_NOT";
		case CL_UNOP_BIT_NOT:   return "CL_UNOP_BIT_NOT";
		case CL_UNOP_MINUS:     return "CL_UNOP_MINUS";
		case CL_UNOP_ABS:       return "CL_UNOP_ABS";
		case CL_UNOP_FLOAT:     return "CL_UNOP_FLOAT";
		default: throw std::runtime_error("Invalid unary operation code");
	}
}

/**
 * @brief  Translates operation code of binary operations to string
 */
std::string translBinOpCode(const unsigned code)
{
	switch (code)
	{
		case CL_BINOP_EQ:            return "CL_BINOP_EQ";
		case CL_BINOP_TRUTH_XOR:     return "CL_BINOP_TRUTH_XOR";
		case CL_BINOP_NE:            return "CL_BINOP_NE";
		case CL_BINOP_LT:            return "CL_BINOP_LT";
		case CL_BINOP_GT:            return "CL_BINOP_GT";
		case CL_BINOP_LE:            return "CL_BINOP_LE";
		case CL_BINOP_GE:            return "CL_BINOP_GE";
		case CL_BINOP_PLUS:          return "CL_BINOP_PLUS";
		case CL_BINOP_MINUS:         return "CL_BINOP_MINUS";
		case CL_BINOP_MULT:          return "CL_BINOP_MULT";
		case CL_BINOP_RDIV:          return "CL_BINOP_RDIV";
		case CL_BINOP_EXACT_DIV:     return "CL_BINOP_EXACT_DIV";
		case CL_BINOP_TRUNC_DIV:     return "CL_BINOP_TRUNC_DIV";
		case CL_BINOP_TRUNC_MOD:     return "CL_BINOP_TRUNC_MOD";
		case CL_BINOP_POINTER_PLUS:  return "CL_BINOP_POINTER_PLUS";
		case CL_BINOP_BIT_IOR:       return "CL_BINOP_BIT_IOR";
		case CL_BINOP_BIT_AND:       return "CL_BINOP_BIT_AND";
		case CL_BINOP_BIT_XOR:       return "CL_BINOP_BIT_XOR";
		case CL_BINOP_TRUTH_AND:     return "CL_BINOP_TRUTH_AND";
		case CL_BINOP_TRUTH_OR:      return "CL_BINOP_TRUTH_OR";
		case CL_BINOP_MIN:           return "CL_BINOP_MIN";
		case CL_BINOP_MAX:           return "CL_BINOP_MAX";
		case CL_BINOP_LSHIFT:        return "CL_BINOP_LSHIFT";
		case CL_BINOP_RSHIFT:        return "CL_BINOP_RSHIFT";
		case CL_BINOP_LROTATE:       return "CL_BINOP_LROTATE";
		case CL_BINOP_RROTATE:       return "CL_BINOP_RROTATE";
		default: throw std::runtime_error("Invalid binary operation code");
	}
}

/**
 * @brief  Translates operation code of instructions to string
 */
std::string translInsnOpCode(const unsigned code)
{
	switch (code)
	{
		case CL_INSN_NOP:     return "CL_INSN_NOP";
		case CL_INSN_JMP:     return "CL_INSN_JMP";
		case CL_INSN_COND:    return "CL_INSN_COND";
		case CL_INSN_RET:     return "CL_INSN_RET";
		case CL_INSN_ABORT:   return "CL_INSN_ABORT";
		case CL_INSN_UNOP:    return "CL_INSN_UNOP";
		case CL_INSN_BINOP:   return "CL_INSN_BINOP";
		case CL_INSN_CALL:    return "CL_INSN_CALL";
		case CL_INSN_SWITCH:  return "CL_INSN_SWITCH";
		case CL_INSN_LABEL:   return "CL_INSN_LABEL";
		default: throw std::runtime_error("Invalid instruction code");
	}
}

/**
 * @brief  Translates code of data types to string
 */
std::string translTypeCode(const unsigned code)
{
	switch (code)
	{

		case CL_TYPE_VOID:     return "CL_TYPE_VOID";
		case CL_TYPE_UNKNOWN:  return "CL_TYPE_UNKNOWN";
		case CL_TYPE_PTR:      return "CL_TYPE_PTR";
		case CL_TYPE_STRUCT:   return "CL_TYPE_STRUCT";
		case CL_TYPE_UNION:    return "CL_TYPE_UNION";
		case CL_TYPE_ARRAY:    return "CL_TYPE_ARRAY";
		case CL_TYPE_FNC:      return "CL_TYPE_FNC";
		case CL_TYPE_INT:      return "CL_TYPE_INT";
		case CL_TYPE_CHAR:     return "CL_TYPE_CHAR";
		case CL_TYPE_BOOL:     return "CL_TYPE_BOOL";
		case CL_TYPE_ENUM:     return "CL_TYPE_ENUM";
		case CL_TYPE_REAL:     return "CL_TYPE_REAL";
		case CL_TYPE_STRING:   return "CL_TYPE_STRING";
		default: throw std::runtime_error("Invalid type code");
	}
}

/**
 * @brief  Translates code of operand types to string
 */
std::string translOperandCode(const unsigned code)
{
	switch (code)
	{
		case CL_OPERAND_VOID:  return "CL_OPERAND_VOID";
		case CL_OPERAND_CST:   return "CL_OPERAND_CST";
		case CL_OPERAND_VAR:   return "CL_OPERAND_VAR";
		default: throw std::runtime_error("Invalid operand code");
	}
}

} // namespace


/**
 * @brief  A wrapper of CL's operands
 *
 * A structure that wraps over CL's operands and provides the output stream <<
 * operator.
 */
struct OpWrapper {

	const cl_operand* op_;

	/**
	 * @brief  Implicit conversion operator from CL's operand
	 */
	OpWrapper(const cl_operand& op) : op_(&op) {}

	friend std::ostream& operator<<(std::ostream& os, const OpWrapper& op)
	{
		os << "operand: ";
		cltToStream(os, op.op_->type, false);
		os << ",";

		const cl_accessor* acc = op.op_->accessor;

		while (acc) {
			os << ' ' << acc->code << ": ";
			cltToStream(os, acc->type, false);
			acc = acc->next;
		}

		return os;
	}
}; // struct OpWrapper


/**
 * @brief  Analyser of loops
 *
 * This structure contains methods for analysing programs for finding and
 * collecting entry points of loops in there.
 *
 * @todo rewrite to something more robust (struct is not the very best idea
 *       here...)
 */
struct LoopAnalyser {

	/**
	 * @brief  A list of CL's code storage blocks
	 *
	 * A list of Code Listener's code storage blocks with the lookup function.
	 */
	struct BlockListItem {
		BlockListItem* prev;
		const CodeStorage::Block* block;

		BlockListItem(BlockListItem* prev, const CodeStorage::Block* block)
			: prev(prev), block(block)
		{ }

		/**
		 * @brief  Looks up a value (block) in the list
		 *
		 * @param[in]  item   The list to be searched through
		 * @param[in]  block  The searched value
		 *
		 * @returns  @p true if @p block is in list @p item, @p false otherwise
		 */
		static bool lookup(const BlockListItem* item, const CodeStorage::Block* block)
		{
			if (!item)
				return false;
			if (item->block == block)
				return true;
			return BlockListItem::lookup(item->prev, block);
		}
	}; // struct BlockListItem


	/// The set of entry points of the analysed program
	std::unordered_set<const CodeStorage::Insn*> entryPoints;


	/**
	 * @brief  Default constructor
	 */
	LoopAnalyser() :
		entryPoints{}
	{ }


	/**
	 * @brief  Visits recursively all reachable blocks
	 *
	 * This method visits recursively all reachable blocks of @p block and checks
	 * whether there is a loop, i.e. whether @p block can reach @p block. In case
	 * there is a loop, the method stores the first instruction of @p block into
	 * the set of entry points @p entryPoints.
	 *
	 * @param[in]  block    The block to be visited
	 * @param[in]  visited  Set of already visited blocks
	 * @param[in]  prev     Pointer to a BlockListItem instance in the stack frame
	 *                      of the callee function (these items are linked into
	 *                      a list during recursive calls to visit())
	 */
	void visit(const CodeStorage::Block* block,
		std::unordered_set<const CodeStorage::Block*>& visited, BlockListItem* prev)
	{
		// note that nodes of the list are on the stack, linked during recursive
		// calls to visit()
		BlockListItem item(prev, block);

		if (!visited.insert(block).second)
		{	// in case 'block' was already in the container
			if (BlockListItem::lookup(prev, block))
			{	// if there is a loop from 'block' back to 'block', set the first
				// instruction of 'block' as the loop's entry point
				this->entryPoints.insert(*block->begin());
			}

			return;
		}

		for (auto target : block->targets())
		{	// visit recursively all successors of 'block'
			this->visit(target, visited, &item);
		}
	}


	/**
	 * @brief  Initialises the structure and analyses the program
	 *
	 * This method initialises the structure and analyses the program for loops.
	 *
	 * @param[in]  block  The first block of the analysed program
	 */
	void init(const CodeStorage::Block* block)
	{
		std::unordered_set<const CodeStorage::Block*> visited;
		this->entryPoints.clear();
		this->visit(block, visited, nullptr);
	}

	/**
	 * @brief  Is given instruction a loop's entry point?
	 *
	 * Checks whether the passed instruction is an entry point of some loop in the
	 * program.
	 *
	 * @param[in]  insn  The checked instruction
	 *
	 * @returns  @p true if @p insn is an entry point of some loop, @p false
	 *           otherwise
	 */
	bool isEntryPoint(const CodeStorage::Insn* insn) const
	{
		return this->entryPoints.find(insn) != this->entryPoints.end();
	}
}; // struct LoopAnalyser


/**
 * @brief  Enumeration of built-in functions
 */
enum class builtin_e
{
	biNone,
	biMalloc,
	biFree,
	biNondet,
	biFix,
	biAbort,
	biPrintHeap,
	biPlotHeap,
	biError
};


/**
 * @brief  The table with built-in functions
 *
 * This table serves as a translation table of function names to the @p
 * builtin_e enumeration.
 */
class BuiltinTable
{
private:

	/// the hash table that maps function name to the value of enumeration
	std::unordered_map<std::string, builtin_e> _table;

public:

	/**
	 * @brief  The default constructor
	 *
	 * The default constructor. It properly initialises the translation table with
	 * proper values.
	 */
	BuiltinTable() :
		_table{}
	{
		this->_table["malloc"]                  = builtin_e::biMalloc;
		this->_table["free"]                    = builtin_e::biFree;
		this->_table["abort"]                   = builtin_e::biAbort;
		this->_table["___fa_get_nondet_int"]    = builtin_e::biNondet;
		this->_table["___fa_error"]             = builtin_e::biError;
		this->_table["__VERIFIER_plot"]         = builtin_e::biPlotHeap;
		this->_table["___fa_fix"]               = builtin_e::biFix;
		this->_table["___fa_print_heap"]        = builtin_e::biPrintHeap;
	}

	/**
	 * @brief  The index operator
	 *
	 * Translates the function name @p key to the enumeration @p builtin_e.
	 * Returns @p builtin_e::biNone if @p key is not a name of a built-in
	 * function.
	 *
	 * @param[in]  key  The name of the function to check
	 * 
	 * @returns  The value of the @p builtin_e enumeration corresponding to @p key
	 */
	builtin_e operator[](const std::string& key)
	{
		std::unordered_map<std::string, builtin_e>::iterator i = this->_table.find(key);
		return (i == this->_table.end())?(builtin_e::biNone):(i->second);
	}
}; // class BuiltinTable


/**
 * @brief  The compiler core
 *
 * The core of the compiler that performs the compilation itself. It is given
 * a code storage and generates an (linear) assembly of microinstructions,
 * together with pointers of functions to the assembly.
 */
class Compiler::Core
{
private:

	/// The assembly code of the program
	Compiler::Assembly* assembly_;

	/// @todo: 
	std::unordered_map<const CodeStorage::Block*, AbstractInstruction*> codeIndex_;
	/// The index of functions, maps function names to their code
	std::unordered_map<const CodeStorage::Fnc*, std::pair<SymCtx,
		CodeStorage::Block>> fncIndex_;
	/// @todo:
	const SymCtx* curCtx_;

	/// The backend for fixpoints
	TreeAut::Backend& fixpointBackend_;
	/// The backend for tree automata
	TreeAut::Backend& taBackend_;
	/// The box manager
	BoxMan& boxMan_;

	/// The table with built-in functions
	BuiltinTable builtinTable_;
	/// The loop analyser
	LoopAnalyser loopAnalyser_;

private:  // methods

	Core(const Core&);
	Core& operator=(const Core&);

protected:

	std::pair<SymCtx, CodeStorage::Block>& getFncInfo(const CodeStorage::Fnc* fnc)
	{
		auto info = fncIndex_.find(fnc);
		// Assertions
		assert(info != fncIndex_.end());
		return info->second;
	}


	/**
	 * @brief  Resets the compiler
	 *
	 * Resets the compiler.
	 *
	 * @param[out]  assembly  The new output for the assembly
	 */
	void reset(Compiler::Assembly& assembly)
	{
		assembly_ = &assembly;
		assembly_->clear();
		codeIndex_.clear();
		fncIndex_.clear();
	}


	/**
	 * @brief  Append instruction to the assembly
	 *
	 * Appends the instruction @p instr to the assembly that is set as the output
	 * of the compiler.
	 *
	 * @param[in]  instr  The instruction to be appended
	 *
	 * @returns  The inserted instruction (should be equal to @p instr)
	 */
	AbstractInstruction* append(AbstractInstruction* instr)
	{
		assembly_->code_.push_back(instr);

		return instr;
	}


	/**
	 * @brief  Overrides the previous instruction
	 *
	 * This method overrides the previous instruction in the assembly with the
	 * provided one.
	 *
	 * @param[in]  instr  The new instruction
	 *
	 * @returns  The inserted instruction
	 */
	AbstractInstruction* override(AbstractInstruction* instr)
	{
		// Assertions
		assert(assembly_->code_.size() > 0);

		delete assembly_->code_.back();
		assembly_->code_.back() = instr;
		return instr;
	}


	/**
	 * @brief  Compile abstraction
	 *
	 * Compiles @b Abstraction as the next microinstruction in the assembly.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void cAbstraction(const CodeStorage::Insn* insn = nullptr)
	{
		append(new FI_abs(insn, fixpointBackend_, taBackend_, boxMan_));
	}


	/**
	 * @brief  Compile fixpoint
	 *
	 * Compiles @b Fixpoint as the next microinstruction in the assembly.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void cFixpoint(const CodeStorage::Insn& insn)
	{
		append(new FI_fix(&insn, fixpointBackend_, taBackend_, boxMan_));
	}


	/**
	 * @brief  Compile heap print
	 *
	 * Compiles @b PrintHeap  as the next microinstruction in the assembly.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void cPrintHeap(const CodeStorage::Insn& insn)
	{
		append(new FI_print_heap(&insn, curCtx_));
	}


	/**
	 * @brief  Compile heap plot
	 *
	 * Compiles @b PlotHeap  as the next microinstruction in the assembly.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void cPlotHeap(const CodeStorage::Insn& insn)
	{
		append(new FI_plot_heap(&insn));
	}


	/**
	 * @brief  Compile loading of a constant
	 *
	 * Compiles @b LoadConstant as the next microinstruction in the assembly.
	 *
	 * @param[in]  dst   Index of the destination register (where the constant is
	 *                   to be loaded)
	 * @param[in]  op    The operand to be loaded (contains the value of the
	 *                   constant)
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void cLoadCst(size_t dst, const cl_operand& op,
		const CodeStorage::Insn& insn)
	{
		// Assertions
		assert(op.code == cl_operand_e::CL_OPERAND_CST);
		assert(op.type != nullptr);

		switch (op.type->code)
		{
			// according to the type of the operand
			case cl_type_e::CL_TYPE_INT:
			case cl_type_e::CL_TYPE_ENUM:
				append(
					new FI_load_cst(&insn, dst, Data::createInt(intCstFromOperand(&op)))
				);
				break;

			case cl_type_e::CL_TYPE_PTR:
				switch (op.data.cst.code) {
					// according to the type of the constant
					case cl_type_e::CL_TYPE_INT:
						append(
							new FI_load_cst(&insn, dst, Data::createInt(intCstFromOperand(&op)))
						);
						break;

					case cl_type_e::CL_TYPE_STRING:
						append(new FI_load_cst(&insn, dst, Data::createUnknw()));
						break;

					default:
						throw NotImplementedException(translTypeCode(op.data.cst.code) +
							": pointer constant type", &insn.loc);
				}
				break;

			case cl_type_e::CL_TYPE_BOOL:
				append(
					new FI_load_cst(&insn, dst, Data::createBool(intCstFromOperand(&op)))
				);
				break;

			default:
				throw NotImplementedException(translTypeCode(op.data.cst.code) +
					": constant type", &insn.loc);
		}
	}


	/**
	 * @brief  Compile a move between registers
	 *
	 * Compiles @b MoveRegister as the next microinstruction in the assembly.
	 *
	 * @param[in]  dst     Index of the destination register
	 * @param[in]  src     Index of the source register
	 * @param[in]  offset  Offset of the destination
	 * @param[in]  insn    The corresponding instruction in the code storage
	 */
	void cMoveReg(size_t dst, size_t src, int offset,
		const CodeStorage::Insn& insn)
	{
		if (offset > 0)
		{
			append(new FI_move_reg_offs(&insn, dst, src, offset));
		} else
		{
			if (src != dst)
				append(new FI_move_reg(&insn, dst, src));
		}
	}


	/**
	 * @brief  Computes the final offset of a chain of accessor in a record
	 *
	 * Given a starting offset @p offset, this function modifies the value
	 * according to the chain of item accessors (in C: "rec.acc1.acc2.acc3...",
	 * where "rec", "rec.acc1", "rec.acc2", ... are records), thus obtaining the
	 * offset from the beginning of the record when we look at the flattened image
	 * of the record.
	 *
	 * @param[in,out]  offset  The initial offset of the record (to be modified)
	 * @param[in]      acc     The head of the list of accessors
	 *
	 * @returns  Next accessor that is not a record accessor (i.e. not "." but
	 *           rather "*", "[]", ...)
	 */
	static const cl_accessor* computeOffset(int& offset, const cl_accessor* acc)
	{
		while (acc && (CL_ACCESSOR_ITEM == acc->code))
		{	// while there are more record accessors (in C: "rec.acc")
			offset += acc->type->items[acc->data.item.id].offset;
			acc = acc->next;
		}

		return acc;
	}


	/**
	 * @brief  Compile a load of an operand into a register
	 *
	 * Compiles a load of an operand in the source register @p src into the
	 * destination register @p dst. The operand is modifed according to the
	 * accessors desribed in @p op. For instance, it may load the value given by
	 * the accessors "reg1->acc1.acc2".
	 *
	 * @param[in]  dst   Index of the destination register
	 * @param[in]  src   Index of the source register
	 * @param[in]  op    The operand to be loaded (contains the accessors)
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void cLoadReg(size_t dst, size_t src, const cl_operand& op,
		const CodeStorage::Insn& insn)
	{
		const cl_accessor* acc = op.accessor;   // the initial accessor
		int offset = 0;                         // the initial offset

		if (acc && (acc->code == CL_ACCESSOR_DEREF))
		{	// in case there is a dereference (in C: "*op") at the start

			// Assertions
			assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

			// compute offset of the accessors after the dereference
			acc = Core::computeOffset(offset, acc->next);
			if (acc && (acc->code == CL_ACCESSOR_REF))
			{	// in case the next accessor is a reference (in C: "&op")

				// assert that there are no more accessors
				assert(acc->next == nullptr);

				// move the register from given offset
				cMoveReg(dst, src, offset, insn);
				return;
			}

			// assert that there are no more accessors
			assert(acc == nullptr);

			if (op.type->code == cl_type_e::CL_TYPE_STRUCT)
			{	// in case the operand is a structure
				std::vector<size_t> offs;
				NodeBuilder::buildNode(offs, op.type);

				// add an instruction to access the set of all selectors of the structure
				append(new FI_acc_set(&insn, dst, offset, offs));
				// add an instruction to load all selectors of the structure
				append(new FI_loads(&insn, dst, dst, offset, offs));
			} else
			{	// in case the operand is not a structure

				// add an instruction to access a single selector
				append(new FI_acc_sel(&insn, dst, offset));
				// add an instruction to load a single selector
				append(new FI_load(&insn, dst, dst, offset));
			}

			// add an instruction to check invariants of the virtual machine
			append(new FI_check(&insn));
		} else
		{	// in case the next accessor is not a dereference

			// compute offset of the accessors
			acc = Core::computeOffset(offset, acc);

			// assert that there are no more accessors
			assert(acc == nullptr);

			// move the register from given offset
			cMoveReg(dst, src, offset, insn);
		}
	}


	/**
	 * @brief  Compile a store of a register value into an operand
	 *
	 * @param[in]  op    The target operand
	 * @param[in]  src   The index of the source register
	 * @param[in]  tmp   The index of the register used as destination
	 * @param[in]  insn  The corresponding instruction in the code storage
	 *
	 * @returns  @p true if the target is accessed using dereference, @p false
	 *           otherwise
	 */
	bool cStoreReg(
		const cl_operand&           op,
		size_t                      src,
		size_t                      tmp,
		const CodeStorage::Insn&    insn)
	{
		const cl_accessor* acc = op.accessor;    // the initial accessor
		int offset = 0;                          // the initial offset

		if (acc && (acc->code == CL_ACCESSOR_DEREF))
		{	// in case there is a dereference (in C: "*op") at the start

			// Assertions
			assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

			// compute offset of the accessors after the dereference
			acc = Core::computeOffset(offset, acc->next);

			// assert that there are no more accessors
			assert(acc == nullptr);

			if (op.type->code == cl_type_e::CL_TYPE_STRUCT)
			{	// in case the operand is a structure
				std::vector<size_t> offs;
				NodeBuilder::buildNode(offs, op.type);

				// add an instruction to access the set of all selectors of the structure
				append(new FI_acc_set(&insn, tmp, offset, offs));
				// add an instruction to store all selectors of the structure
				append(new FI_stores(&insn, tmp, src, offset));
			} else
			{	// in case the operand is not a structure

				// add an instruction to access a single selector
				append(new FI_acc_sel(&insn, tmp, offset));
				// add an instruction to store a single selector
				append(new FI_store(&insn, tmp, src, offset));
			}

			// add an instruction to check invariants of the virtual machine
			append(new FI_check(&insn));

			return true;
		} else
		{	// in case the next accessor is not a dereference

			// compute offset of the accessors
			acc = Core::computeOffset(offset, acc);

			// Assertions
			assert(acc == nullptr);   // there are no more accessors
			assert(offset == 0);

			if (src != tmp)
				append(new FI_move_reg(&insn, tmp, src));

			return false;
		}
	}


	/**
	 * @brief  Compile a load of an operand into a register
	 *
	 * Compiles the load instruction of the operand @p op into the destination
	 * register @p dst. In case the @p canOverride parameter is set to @p true,
	 * the destination register may change (in case the operand is already in
	 * a register).
	 *
	 * @param[in]  dst          The index of the destination register
	 * @param[in]  op           The source operand
	 * @param[in]  insn         The corresponding instruction in the code storage
	 * @param[in]  canOverride  @p true if the destination register may change
	 *
	 * @returns  Index of the register into which will the value be loaded (may
	 *           differ from @p dst)
	 */
	size_t cLoadOperand(size_t dst, const cl_operand& op,
		const CodeStorage::Insn& insn, bool canOverride = true)
	{
		switch (op.code)
		{	// according to the type of the operand
			case cl_operand_e::CL_OPERAND_VAR:
			{	// in case the operand is a variable
				const VarInfo& varInfo = curCtx_->getVarInfo(varIdFromOperand(&op));

				if (varInfo.isOnStack() || varInfo.isGlobal())
				{ // in the case of a variable on the stack or global
					const cl_accessor* acc = op.accessor;   // get the first accessor
					int offset = 0;                         // initialize the offset

					if (acc && (acc->code == CL_ACCESSOR_DEREF))
					{	// in case there is the dereference accessor ('*' in C)
						assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

						if (varInfo.isOnStack())
						{
							// append an instruction to load value at the address relative to
							// the abstract base pointer in the symbolic stack
							append(new FI_load_ABP(&insn, dst, static_cast<int>(varInfo.getStackOffset())));
						}
						else if (varInfo.isGlobal())
						{
							throw NotImplementedException("cLoadOperand(): global variables 1");
						}
						else
						{
							assert(false);           // fail gracefully
						}

						// jump to the next accessor
						acc = Core::computeOffset(offset, acc->next);

						if (acc && (acc->code == CL_ACCESSOR_REF))
						{	// in case the next accessor is a reference ('&' in C)
							// assert the operand is a pointer
							assert(op.type->code == cl_type_e::CL_TYPE_PTR);
							// assert '&' is the last accessor
							assert(acc->next == nullptr);

							if (offset)
							{	// in case offset is non-zero

								// append instruction to move the value on the offset to the
								// beginning of the register
								append(new FI_move_reg_offs(&insn, dst, dst, offset));
							}

							break;
						}

						// assert there are no more accessors
						assert(acc == nullptr);

						if (op.type->code == cl_type_e::CL_TYPE_STRUCT)
						{	// in case the operand is a structure
							std::vector<size_t> offs;
							NodeBuilder::buildNode(offs, op.type);

							// append an instruction to isolate the root
							append(new FI_acc_set(&insn, dst, offset, offs));
							// append an instruction to load the root
							append(new FI_loads(&insn, dst, dst, offset, offs));
						} else
						{
							// append an instruction to isolate the element
							append(new FI_acc_sel(&insn, dst, offset));
							// append an instruction to load the element
							append(new FI_load(&insn, dst, dst, offset));
						}
					} else
					{	// in case there is not a dereference
						if (varInfo.isOnStack())
						{
							offset = static_cast<int>(varInfo.getStackOffset());
						}
						else if (varInfo.isGlobal())
						{
							offset = static_cast<int>(varInfo.getGlobalBlockOffset());
						}

						// compute the real offset from record accessors
						acc = Core::computeOffset(offset, acc);

						if (acc && (acc->code == CL_ACCESSOR_REF))
						{	// in case the next accessor is a reference ('&' in C)
							// assert there are no more accessors
							assert(acc->next == nullptr);

							if (varInfo.isOnStack())
							{
								// append the instruction to get the value at given offset
								// TODO @todo  should this really be there? The value is loaded
								// later...
								append(new FI_get_ABP(&insn, dst, offset));
							}
							else if (varInfo.isGlobal())
							{
								throw NotImplementedException("cLoadOperand(): global variables 2");
							}
							else
							{
								assert(false);          // fail gracefully
							}
							break;
						}

						// assert there are no more accessors
						assert(acc == nullptr);

						if (varInfo.isOnStack())
						{
							// append the instruction to load the value at given offset
							append(new FI_load_ABP(&insn, dst, offset));
						}
						else if (varInfo.isGlobal())
						{
							// append the instruction to load the global value at given offset
							append(new FI_load_GLOB(&insn, dst, offset));
						}
						else
						{
							assert(false);          // fail gracefully
						}
					}
				} else if (varInfo.isInReg())
				{	// in case the variable is in a register
					if (canOverride)
					{	// in case the destination register can be overridden
						dst = varInfo.getRegIndex();          // change the value
						cLoadReg(dst, dst, op, insn);
					} else
					{	// in case the destination register cannot be overridden
						cLoadReg(dst, varInfo.getRegIndex(), op, insn);
					}
				} else
				{	// otherwise
					assert(false);      // fail gracefully
				}

				break;
			}

			case cl_operand_e::CL_OPERAND_CST: // if the operand is a constant
				cLoadCst(dst, op, insn);         // load the constant
				break;

			default:
				assert(false);                   // fail gracefully
		}

		return dst;
	}


	/**
	 * @brief  Gets the index of the register where there is given operand
	 *
	 * Gets the index of the register where the value of the operand @p op which
	 * is in register @p src is stored. So, if there is a variable reference in @p
	 * src and the operand is dereferenced, the target of the reference is
	 * obtained.
	 *
	 * @param[in]  op   The source operand
	 * @param[in]  src  Index of the register with the source
	 *
	 * @returns  Index of the register with the requested operand
	 */
	size_t lookupStoreReg(const cl_operand& op, size_t src)
	{
		switch (op.code)
		{	// depending on the type of the operand
			case cl_operand_e::CL_OPERAND_VAR:
			{	// in case it is a variable

				// get info about the variable
				const VarInfo& varInfo = curCtx_->getVarInfo(varIdFromOperand(&op));

				if (varInfo.isOnStack() || varInfo.isGlobal())
				{	// in case it is on the stack or global
					return src;
				} else if (varInfo.isInReg())
				{	// in case it is in a register
					size_t tmp = varInfo.getRegIndex();

					const cl_accessor* acc = op.accessor;

					// in case there is dereference at the operand, store to the new
					// register, otherwise use the register in which the variable already
					// is
					return (acc && (acc->code == CL_ACCESSOR_DEREF))? (src) : (tmp);
				} else
				{	// othewise
					assert(false);      // fail gracefully
				}
			}

			default:
				assert(false);      // fail gracefully
				return src;					// for the sake of compiler (should never get here)
		}
	}


	/**
	 * @brief  Compile a store of a value into an operand
	 *
	 * Compiles the store instruction that stores the value given in @p src into
	 * the operand which is pointed to by the @p tmp parameter.
	 *
	 * @param[in]  op    The target operand
	 * @param[in]  src   Index of the register with the value to be stored
	 * @param[in]  tmp   Index of the register the points to the symbolic memory
	 *                   location where the value is to be stored
	 * @param[in]  insn  The corresponding instruction in the code storage
	 *
	 * @returns  @p true if the value is stored into a variable, @p false if it is
	 *           stored into a register
	 */
	bool cStoreOperand(
		const cl_operand&           op,
		size_t                      src,
		size_t                      tmp,
		const CodeStorage::Insn&    insn)
	{
		switch (op.code)
		{	// according to the type of the operand
			case cl_operand_e::CL_OPERAND_VAR:
			{	// in case it is a variable

				// get variable info
				const VarInfo& varInfo = curCtx_->getVarInfo(varIdFromOperand(&op));

				if (varInfo.isOnStack() || varInfo.isGlobal())
				{	// in case it is on the stack or is global
					if (varInfo.isOnStack())
					{ // append the instruction to get the value at given offset
						append(new FI_get_ABP(&insn, tmp, 0));
					}
					else if (varInfo.isGlobal())
					{ // append the instruction to get the value at given offset
						append(new FI_get_GLOB(&insn, tmp, 0));
					}
					else
					{
						assert(false);          // fail gracefully
					}

					int offset;

					if (varInfo.isOnStack())
					{
						offset = static_cast<int>(varInfo.getStackOffset());
					}
					else if (varInfo.isGlobal())
					{
						offset = static_cast<int>(varInfo.getGlobalBlockOffset());
					}
					else
					{
						assert(false);         // fail gracefully
						offset = 0;            // avoid using an uninitialized value in case assert(false) is noop
					}

					bool needsAcc = false;

					const cl_accessor* acc = op.accessor;

					if (acc)
					{	// in case there are some accessors
						if (acc->code == CL_ACCESSOR_DEREF)
						{	// in case the accessor is a dereference ('*' in C)
							assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

							if (varInfo.isOnStack())
							{
								// override previous instruction
								override(new FI_load_ABP(&insn, tmp, varInfo.getStackOffset()));
							}
							else if (varInfo.isGlobal())
							{
								throw NotImplementedException("cStoreOperand(): global variables 3");
							}
							else
							{
								assert(false);         // fail gracefully
							}

							// separation is needed
							needsAcc = true;

							// skip to the next accessor
							acc = acc->next;

							// reinitialize the offset
							offset = 0;
						}

						// compute the real offset (while ``flattening'' the structure)
						acc = Core::computeOffset(offset, acc);
					}

					// assert there are no more accessors
					assert(acc == nullptr);

					if (op.type->code == cl_type_e::CL_TYPE_STRUCT)
					{	// in case the operand is a structure

						// build a symbolic node
						std::vector<size_t> offs;
						NodeBuilder::buildNode(offs, op.type);

						if (needsAcc)
						{	// in case separation is needed
							// append separation of a set of nodes
							append(new FI_acc_set(&insn, tmp, offset, offs));
						}

						// append store of the value into register
						append(new FI_stores(&insn, tmp, src, offset));
					} else
					{	// in case the operand is anything but a structure

						if (needsAcc)
						{	// in case separation is needed
							// append separation of a node
							append(new FI_acc_sel(&insn, tmp, offset));
						}

						// append store of the value into register
						append(new FI_store(&insn, tmp, src, offset));
					}

					// add an instruction to check invariants of the virtual machine
					append(new FI_check(&insn));

					return true;
				} else if (varInfo.isInReg())
				{	// in case it is in a register
					return cStoreReg(op, src, varInfo.getRegIndex(), insn);
				} else
				{ // otherwise
					assert(false);        // fail gracefully
				}
			}

			default:          // the default case
				assert(false);
				return false;
		}
	}


	/**
	 * @brief  Method that kills given variables
	 *
	 * This method is to be called after an instruction that may introduce
	 * temporary variables is compiled. It identifies which variables can be
	 * killed and kills them.
	 *
	 * @param[in]  vars  A list of variables to be killed
	 * @param[in]  insn  The corresponding instruction in the code storage
	 *
	 * @returns  Either @p nullptr if there are no feasible variables to be killed
	 *           or a pointer to the instruction that loads the undefined data
	 *           block that is used to kill given variables
	 *
	 * @note  Note that We are killing @e dead variables. This sounds very brutal.
	 */
	AbstractInstruction* cKillDeadVariables(
		const CodeStorage::TKillVarList&        vars,
		const CodeStorage::Insn&                insn)
	{
		std::set<size_t> offs;

		for (auto var : vars)
		{	// for every variable to be killed
			if (var.onlyIfNotPointed)
			{	// if killing is safe only if nobody points at the variable
				continue;
			}

			// obtain information about the variable
			const VarInfo& varInfo = curCtx_->getVarInfo(var.uid);

			if (varInfo.isOnStack())
			{	// on case the variable is not on the stack
				// retrieve the offset of the variable at the current stack frame
				offs.insert(varInfo.getStackOffset());
			}
		}

		if (offs.empty())
		{	// in case there are no feasible variables to be killed
			return nullptr;
		}

		std::vector<Data::item_info> tmp;

		if (offs.size() > 1)
		{	// in case there is more than one offset, prepare for creating a temporary
			// structure
			for (auto offset : offs)
				tmp.push_back(Data::item_info(offset, Data::createUndef()));
		}

		// append an instruction to load an undefined constant to r0
		AbstractInstruction* result = append(
			new FI_load_cst(&insn, /* dst reg */ 0,
				(offs.size() > 1)?(Data::createStruct(tmp)):(Data::createUndef()))
		);

		// append an instruction to load the abstract base pointer into r1
		append(new FI_get_ABP(&insn, /* dst reg */ 1, /* offset */ 0));

		// append an instruction to store the undefined value in r0 to the address
		// stored in r1
		append(
			(offs.size() > 1)
				?(static_cast<AbstractInstruction*>(new FI_stores(
						&insn,
						/* reg with addr of dst */ 1,
						/* src reg */ 0,
						/* offset of the dst */ 0
					)))
				:(static_cast<AbstractInstruction*>(new FI_store(
						&insn,
						/* reg with addr of dst */ 1,
						/* src reg */ 0,
						/* offset of the dst */ *offs.begin()
					)))
		);

		return result;
	}


	/**
	 * @brief  Compiles an assignment of a value to some memory location
	 *
	 * This method compiles an assignment of a value to a memory location, given
	 * by the operands in @p insn.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void compileAssignment(const CodeStorage::Insn& insn)
	{
		const cl_operand& dst = insn.operands[0];
		const cl_operand& src = insn.operands[1];

		// Assertions
		assert(src.type != nullptr);
		assert(dst.type != nullptr);
		assert(src.type->code == dst.type->code);

		// get registers for the source and the target
		size_t dstReg = lookupStoreReg(dst, 0);
		size_t srcReg = cLoadOperand(dstReg, src, insn);

		if (src.type->code == cl_type_e::CL_TYPE_PTR &&
			src.type->items[0].type->code == cl_type_e::CL_TYPE_VOID &&
			dst.type->items[0].type->code != cl_type_e::CL_TYPE_VOID)
		{	// in case the source is a void pointer and the destination is not;
			// this happens for example at the call of malloc, when conversion from
			// a void pointer to a typed pointer is done

			// build a node according to the destination type
			std::vector<SelData> sels;
			NodeBuilder::buildNode(sels, dst.type->items[0].type);

			// create a string with the name of the type
			std::string typeName;
			if (dst.type->items[0].type->name)
			{	// in case the type is named
				typeName = std::string(dst.type->items[0].type->name);
			} else
			{	// in case the type is unnamed
				std::ostringstream ss;
				ss << dst.type->items[0].type->uid;
				typeName = ss.str();
			}

			// append an instruction to create a new node
			append(
				new FI_node_create(
					&insn,
					/* dst reg where the node will be stored */ srcReg,
					/* reg with the value from which the node is to be created */ srcReg,
					/* size of the created node */ dst.type->items[0].type->size,
					/* type information */ boxMan_.getTypeInfo(typeName),
					/* selectors of the node */ sels
				)
			);
		}

		cStoreOperand(
			/* target operand */ dst,
			/* reg with src value */ srcReg,
			/* reg pointing to memory location with the value to be stored */ 1,
			insn
		);
		// kill dead variables
		cKillDeadVariables(insn.varsToKill, insn);
	}


	/**
	 * @brief  Compiles a Boolean negation
	 *
	 * This method compiles a negation of a Boolean (or, in general, integer)
	 * expression.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void compileTruthNot(const CodeStorage::Insn& insn)
	{
		const cl_operand& dst = insn.operands[0];
		const cl_operand& src = insn.operands[1];

		// assert that the destination is a Boolean
		assert(dst.type->code == cl_type_e::CL_TYPE_BOOL);

		// get registers for the source and the target
		size_t dstReg = lookupStoreReg(dst, 0);
		size_t srcReg = cLoadOperand(dstReg, src, insn);

		switch (src.type->code)
		{	// append a corresponding indstruction according to the type of the source
			case cl_type_e::CL_TYPE_BOOL:
				append(new FI_bnot(&insn, srcReg));
				break;

			case cl_type_e::CL_TYPE_INT:
				append(new FI_inot(&insn, srcReg));
				break;

			default:
				assert(false);
		}

		cStoreOperand(
			/* target operand */ dst,
			/* reg with src value */ srcReg,
			/* reg pointing to memory location with the value to be stored */ 1,
			insn
		);
		// kill dead variables
		cKillDeadVariables(insn.varsToKill, insn);
	}


	/**
	 * @brief  Compiles memory allocation
	 *
	 * This method compiles allocation of a symbolic memory block of given size
	 * and type.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void compileMalloc(const CodeStorage::Insn& insn)
	{
		const cl_operand& dst = insn.operands[0];
		const cl_operand& src = insn.operands[2];

		// assert that 
		assert(src.type->code == cl_type_e::CL_TYPE_INT);
		assert(dst.type->code == cl_type_e::CL_TYPE_PTR);

		// get registers for the source and the target
		size_t dstReg = lookupStoreReg(dst, 0);
		size_t srcReg = cLoadOperand(dstReg, src, insn);

		// append the instruction to set information for the pointer
		append(new FI_alloc(
			&insn,
			/* reg where the result shall be stored*/ srcReg,
			/* reg with the number of allocated bytes */ srcReg
		));

		if (dst.type->items[0].type->code != cl_type_e::CL_TYPE_VOID)
		{	// in case the destination pointer is not a void pointer

			// build a node with proper selectors
			std::vector<SelData> sels;
			NodeBuilder::buildNode(sels, dst.type->items[0].type);

			// get the name of the target type
			std::string typeName;
			if (dst.type->items[0].type->name)
				typeName = std::string(dst.type->items[0].type->name);
			else {
				std::ostringstream ss;
				ss << dst.type->items[0].type->uid;
				typeName = ss.str();
			}

			// append an instruction to create the node in the symbolic memory
			append(new FI_node_create(
				&insn,
				/* dst reg for the pointer to the node */ srcReg,
				/* reg with the value from which the node is to be created */ srcReg,
				/* size of the created node*/ dst.type->items[0].type->size,
				/* type information */ boxMan_.getTypeInfo(typeName),
				/* selectors of the node */ sels
			));
		}

		cStoreOperand(
			/* target operand */ dst,
			/* reg with src value */ srcReg,
			/* reg pointing to memory location with the value to be stored */ 1,
			insn
		);
		// kill dead variables
		cKillDeadVariables(insn.varsToKill, insn);
	}


	/**
	 * @brief  Compile freeing of memory
	 *
	 * Compiles instructions that release a symbolic memory block.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void compileFree(const CodeStorage::Insn& insn)
	{
		const cl_operand& src = insn.operands[2];

		// get registers for the source
		size_t srcReg = cLoadOperand(0, src, insn);

		// append an instruction to isolate all selectors
		append(new FI_acc_all(
			&insn,
			/* reg with ref to the tree to have selectors isolated */ srcReg
		));

		// append an instruction to free a symbolic memory node
		append(new FI_node_free(&insn,
			/* reg with ref to the freed node */ srcReg
		));
		// add an instruction to check invariants of the virtual machine
		append(new FI_check(&insn));

		// kill dead variables
		cKillDeadVariables(insn.varsToKill, insn);
	}


	/**
	 * @brief  Compiles a comparison of two operands
	 *
	 * This instruction compiles a comparison (of the type @p F) of two operands.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 *
	 * @tparam  F  The type of instruction to be compiled
	 */
	template <class F>
	void compileCmp(const CodeStorage::Insn& insn)
	{
		const cl_operand& dst = insn.operands[0];
		const cl_operand& src1 = insn.operands[1];
		const cl_operand& src2 = insn.operands[2];

		// assert the destination is a Boolean
		assert(dst.type->code == cl_type_e::CL_TYPE_BOOL);

		// get registers for the sources and the target
		size_t dstReg = lookupStoreReg(dst, 0);
		size_t src1Reg = cLoadOperand(0, src1, insn);
		size_t src2Reg = cLoadOperand(1, src2, insn);

		// append the desired instruction
		append(new F(&insn, dstReg, src1Reg, src2Reg));

		cStoreOperand(
			/* target operand */ dst,
			/* reg with src value */ dstReg,
			/* reg pointing to memory location with the value to be stored */ 1,
			insn
		);
		// kill dead variables
		cKillDeadVariables(insn.varsToKill, insn);
	}


	/**
	 * @brief  Compiles integer addition
	 *
	 * This method compiles addition of two integer operands.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void compilePlus(const CodeStorage::Insn& insn)
	{
		const cl_operand& dst = insn.operands[0];
		const cl_operand& src1 = insn.operands[1];
		const cl_operand& src2 = insn.operands[2];

		// assert that all operands are integer
		assert(dst.type->code == cl_type_e::CL_TYPE_INT);
		assert(src1.type->code == cl_type_e::CL_TYPE_INT);
		assert(src2.type->code == cl_type_e::CL_TYPE_INT);

		// get registers for the sources and the target
		size_t dstReg = lookupStoreReg(dst, 0);
		size_t src1Reg = cLoadOperand(0, src1, insn);
		size_t src2Reg = cLoadOperand(1, src2, insn);

		// append the instruction for integer addition
		append(new FI_iadd(&insn, dstReg, src1Reg, src2Reg));

		cStoreOperand(
			/* target operand */ dst,
			/* reg with src value */ dstReg,
			/* reg pointing to memory location with the value to be stored */ 1,
			insn
		);
		// kill dead variables
		cKillDeadVariables(insn.varsToKill, insn);
	}

	/**
	 * @brief  Compile pointer arithmetics
	 *
	 * This method compiles instructions for pointer arithmetics, i.e. addition of
	 * an integer to a pointer.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void compilePointerPlus(const CodeStorage::Insn& insn)
	{
		const cl_operand& dst = insn.operands[0];
		const cl_operand& src1 = insn.operands[1];
		const cl_operand& src2 = insn.operands[2];

		// TODO: why this order? Cannot it be the other one?
		// assert that @todo
		assert(dst.type->code == cl_type_e::CL_TYPE_PTR);
		assert(src1.type->code == cl_type_e::CL_TYPE_PTR);
		assert(src2.type->code == cl_type_e::CL_TYPE_INT);

		// get registers for the sources and the target
		size_t dstReg = lookupStoreReg(dst, 0);
		size_t src1Reg = cLoadOperand(0, src1, insn);
		size_t src2Reg = cLoadOperand(1, src2, insn);

		// append an instruction to increment displacement of a pointer
		append(new FI_move_reg_inc(&insn, dstReg, src1Reg, src2Reg));

		cStoreOperand(
			/* target operand */ dst,
			/* reg with src value */ dstReg,
			/* reg pointing to memory location with the value to be stored */ 1,
			insn
		);
		// kill dead variables
		cKillDeadVariables(insn.varsToKill, insn);
	}


	/**
	 * @brief  Compiles a jump
	 *
	 * This method compiles a jump instruction.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void compileJmp(const CodeStorage::Insn& insn)
	{
		// append an instruction to perform a jump
		append(new FI_jmp(&insn, insn.targets[0]));
	}


	/**
	 * @brief  Compiles a call of an internal function
	 *
	 * Compiles a call of an internal function, i.e. function the implementation
	 * of which is known.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 * @param[in]  fnc   The function which is to be called
	 */
	void compileCallInternal(const CodeStorage::Insn& insn,
		const CodeStorage::Fnc& fnc)
	{
		// assertion on the number of arguments
		assert(fnc.args.size() + 2 == insn.operands.size());

		for (size_t i = fnc.args.size() + 1; i > 1; --i)
		{ // feed registers with arguments (r2 ... )
			cLoadOperand(
				/* destination reg */ i,
				/* operand */ insn.operands[i],
				/* instruction */ insn,
				/* can the destination reg be overriden? */ false
			);
		}

		CodeStorage::TKillVarList varsToKill = insn.varsToKill;

		// kill also the destination variable if possible
		if (insn.operands[0].code == cl_operand_e::CL_OPERAND_VAR)
		{	// in case the result of the function should be stored into a variable
			auto varId = varIdFromOperand(&insn.operands[0]);

			if (curCtx_->getVarInfo(varId).isOnStack())
			{	// if the variable is on the stack
				auto acc = insn.operands[0].accessor;

				if (!acc || (acc->code != CL_ACCESSOR_DEREF))
				{	// in case the variable is not accessed by dereference
					varsToKill.insert(CodeStorage::KillVar(varId, false));
				}
			}
		}

		// kill dead variables
		cKillDeadVariables(varsToKill, insn);

		// current offset in the code segment
		size_t head = assembly_->code_.size();

		// put placeholder for loading return address into r1
		append(nullptr);

		// call
		append(new FI_jmp(&insn, &getFncInfo(&fnc).second));

		// load ABP into r1
		append(new FI_get_ABP(
			&insn,
			/* dst reg */ 1,
			/* offset */ 0
		));

		// isolate adjacent nodes (current ABP)
		append(new FI_acc_all(&insn, 1));

		// the new offset in the code segment
		size_t head2 = assembly_->code_.size();

		// fixpoint
		cFixpoint(insn);

		// TODO: I don't know why this is here
		assembly_->code_[head2]->insn(&insn);

		if (insn.operands[0].code != CL_OPERAND_VOID)
		{	// in case the called function returns some value

			// pop return value into r0
			append(new FI_pop_greg(&insn, 0));
			// collect result from r0
			cStoreOperand(
				/* target operand */ insn.operands[0],
				/* reg with src value */ 0,
				/* reg pointing to memory location with the value to be stored */ 1,
				insn);
		}

		// construct instruction for loading return address
		assembly_->code_[head] =
			new FI_load_cst(&insn, 1,
				Data::createNativePtr(assembly_->code_[head + 2]));

		// set target flag
		assembly_->code_[head + 2]->setTarget();
	}


	/**
	 * @brief  Compiles a return from a function
	 *
	 * This method compiles a return from a function, either with or without
	 * a return value.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void compileRet(const CodeStorage::Insn& insn)
	{
		if (insn.operands[0].code != CL_OPERAND_VOID)
		{	// in case the function returns a value

			// move the return value into r0
			cLoadOperand(
				/* destination reg */	0,
				/* operand */ insn.operands[0],
				/* instruction */ insn,
				/* can the destination reg be overriden? */ false);

			// push r0 to gr1
			append(new FI_push_greg(&insn, 0));
		}

		// load previous ABP into r0
		append(new FI_load_ABP(&insn, 0, ABP_OFFSET));

		// store current ABP into r1
		append(new FI_get_ABP(&insn, 1, 0));

		// restore previous ABP (r0)
		append(new FI_set_greg(&insn, ABP_INDEX, 0));

		// move return address into r0
		append(new FI_load(&insn, 0, 1, RET_OFFSET));

		// delete stack frame (r1)
		append(new FI_node_free(&insn, 1));

		// return to r0
		append(new FI_ret(&insn, 0));
	}

	/**
	 * @brief  Compiles the error instruction
	 *
	 * Compiles an instruction that represents an error location in the code.
	 */
	void compileError(const CodeStorage::Insn& insn)
	{
		const CodeStorage::TOperandList &opList = insn.operands;
		assert(3 == opList.size());

		const cl_operand& opMsg = opList[2];
		// TODO: also allow CL_OPERAND_VAR
		assert(CL_OPERAND_CST == opMsg.code);

		const struct cl_cst &cstMsg = opMsg.data.cst;
		assert(CL_TYPE_STRING == cstMsg.code);
		assert(nullptr != cstMsg.data.cst_string.value);

		std::string msg(cstMsg.data.cst_string.value);

		append(new FI_error(&insn, msg));
	}


	/**
	 * @brief  Compiles a conditional jump
	 *
	 * This method compiles instructions for a conditional jump.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void compileCond(const CodeStorage::Insn& insn)
	{
		const cl_operand& src = insn.operands[0];

		// get register for the source
		size_t srcReg = cLoadOperand(0, src, insn);

		cKillDeadVariables(insn.varsToKill, insn);

		AbstractInstruction* tmp[2] = { nullptr, nullptr };

		// assign mark and allocate space in the code assembly for the condition
		// test instruction
		size_t sentinel = assembly_->code_.size();
		append(nullptr);

		for (auto i : { 0, 1 })
		{	// for each branch

			// try to kill dead variables
			tmp[i] = cKillDeadVariables(insn.killPerTarget[i], insn);

			// append an instruction to jump to the corresponding branch
			append(new FI_jmp(&insn, insn.targets[i]));

			if (!tmp[i])
			{	// if no dead variables were killed
				tmp[i] = assembly_->code_.back();
			}
		}

		// append the condition test instruction
		assembly_->code_[sentinel] = new FI_cond(
			&insn,
			/* register with the result of the condition */ srcReg,
			/* array of a pair of successor states (true and false branch) */ tmp
		);
	}


	/**
	 * @brief  Compiles a load of a nondeterministic value
	 *
	 * This method compiles a load of a nondeterministic value into a register.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void compileNondet(const CodeStorage::Insn& insn)
	{
		const cl_operand& dst = insn.operands[0];

		// get register for the destination
		size_t dstReg = lookupStoreReg(dst, 0);

		// append an instruction to load an unknown constant
		append(new FI_load_cst(&insn, dstReg, Data::createUnknw()));

		cStoreOperand(
			/* target operand */ dst,
			/* reg with src value */ dstReg,
			/* reg pointing to memory location with the value to be stored */ 1,
			insn
		);
		// kill dead variables
		cKillDeadVariables(insn.varsToKill, insn);
	}


	/**
	 * @brief  Compiles a function call
	 *
	 * This method compiles a generic function call. It handles all types of
	 * functions, i.e. special functions (such as malloc or free in C),
	 * Forester-specific functions (fixpoint computation, printing of heap, ...)
	 * and also ``normal'' functions.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void compileCall(const CodeStorage::Insn& insn)
	{
		// assert that the operand is a constant function
		assert(insn.operands[1].code == cl_operand_e::CL_OPERAND_CST);
		assert(insn.operands[1].data.cst.code == cl_type_e::CL_TYPE_FNC);

		switch (builtinTable_[insn.operands[1].data.cst.data.cst_fnc.name])
		{	// switch according to the name of the function
			case builtin_e::biMalloc:
				compileMalloc(insn);
				return;
			case builtin_e::biFree:
				compileFree(insn);
				return;
			case builtin_e::biNondet:
				compileNondet(insn);
				return;
			case builtin_e::biFix:
				cFixpoint(insn);
				return;
			case builtin_e::biPrintHeap:
				cPrintHeap(insn);
				return;
			case builtin_e::biPlotHeap:
				cPlotHeap(insn);
				return;
			case builtin_e::biAbort:
				// abort() is not supported... anyway, if abort() is used, Code Listener
				// gives a special instruction as the last instruction (as there is
				// no return): CL_INSN_ABORT
				throw NotImplementedException(
					insn.operands[1].data.cst.data.cst_fnc.name, &insn.loc);
				// TODO: this does not look very nice either... where is checking for
				// garbage?
				this->append(new FI_abort(&insn));
				return;
			case builtin_e::biError:
				compileError(insn);
				return;
			default:
				break;
		}

		// in case the function has no special meaning

		const CodeStorage::Fnc* fnc =
			insn.stor->fncs[insn.operands[1].data.cst.data.cst_fnc.uid];

		if (!isDefined(*fnc))
		{	// in case the implementation of the function is not provided
			FA_NOTE_MSG(&insn.loc, "ignoring call to undefined function '" <<
				insn.operands[1].data.cst.data.cst_fnc.name << '\'');

			if (insn.operands[0].code != CL_OPERAND_VOID)
			{	// in case the function returns a value

				// append an instruction to load an unknown value
				append(new FI_load_cst(&insn, 0, Data::createUnknw()));
				cStoreOperand(
					/* target operand */ insn.operands[0],
					/* reg with src value */ 0,
					/* reg pointing to memory location with the value to be stored */ 1,
					insn
				);
			}
		} else
		{	// in case the function has an implementation
			compileCallInternal(insn, *fnc);
		}
	}


	/**
	 * @brief  Compiles an instruction
	 *
	 * This method compiles an arbitrary instruction, by calling a proper method
	 * corresponding to the instruction code and subcode.
	 *
	 * @param[in]  insn  The corresponding instruction in the code storage
	 */
	void compileInstruction(const CodeStorage::Insn& insn)
	{
		FA_DEBUG_AT(3, insn.loc << ' ' << insn);

		switch (insn.code)
		{	// according to the main instruction code
			case cl_insn_e::CL_INSN_UNOP: // unary operator
				switch (insn.subCode)
				{	// according to the instruction subcode
					case cl_unop_e::CL_UNOP_ASSIGN:
						compileAssignment(insn);
						break;
					case cl_unop_e::CL_UNOP_BIT_NOT:
						// TODO
					case cl_unop_e::CL_UNOP_TRUTH_NOT:
						compileTruthNot(insn);
						break;
					default:
						throw NotImplementedException(translUnOpCode(insn.subCode),
							&insn.loc);
				}
				break;

			case cl_insn_e::CL_INSN_BINOP: // binary operator
				switch (insn.subCode)
				{	// according to the instruction subcode
					case cl_binop_e::CL_BINOP_EQ:
						compileCmp<FI_eq>(insn);
						break;
					case cl_binop_e::CL_BINOP_NE:
						compileCmp<FI_neq>(insn);
						break;
					case cl_binop_e::CL_BINOP_LT:
						compileCmp<FI_lt>(insn);
						break;
					case cl_binop_e::CL_BINOP_GT:
						compileCmp<FI_gt>(insn);
						break;
					case cl_binop_e::CL_BINOP_PLUS:
						compilePlus(insn);
						break;
					case cl_binop_e::CL_BINOP_POINTER_PLUS:
						compilePointerPlus(insn);
						break;
					default:
						throw NotImplementedException(translBinOpCode(insn.subCode),
							&insn.loc);
				}
				break;

			case cl_insn_e::CL_INSN_CALL: // function call
				compileCall(insn);
				break;

			case cl_insn_e::CL_INSN_RET: // return from a function
				compileRet(insn);
				break;

			case cl_insn_e::CL_INSN_JMP: // jump
				compileJmp(insn);
				break;

			case cl_insn_e::CL_INSN_COND: // condition
				compileCond(insn);
				break;

			case cl_insn_e::CL_INSN_LABEL:
				// safe to ignore unless we support checking for error label reachability
				break;

			default:
				throw NotImplementedException(translInsnOpCode(insn.code),
					&insn.loc);
		}
	}


	/**
	 * @brief  Compile a basic block of a control-flow graph
	 *
	 * This method compiles a basic block of a control-flow graph.
	 *
	 * @param[in]  block     The block of the control-flow graph in CodeStorage
	 * @param[in]  abstract  @p true if there should be abstraction at the
	 *                       beginning of the block, @p false otherwise
	 */
	void compileBlock(const CodeStorage::Block* block, bool abstract)
	{
		size_t head = assembly_->code_.size();

		if (abstract || loopAnalyser_.isEntryPoint(*block->begin()))
		{	// in case there should be abstraction at the start of the block
			cAbstraction();
		}

		for (auto insn : *block)
		{	// for every instruction of the block
			compileInstruction(*insn);

			if (head != assembly_->code_.size())
			{	// in case some instruction(s) was compiled
				assembly_->code_[head]->insn(insn);
				head = assembly_->code_.size();
			}
		}
	}


	/**
	 * @brief  Compiles a function
	 *
	 * This method compiles a function into the assembly.
	 *
	 * @param[in]  fnc  The function to be compiled
	 */
	void compileFunction(const CodeStorage::Fnc& fnc)
	{
		std::pair<SymCtx, CodeStorage::Block>& fncInfo = getFncInfo(&fnc);

		//         ************  create function context  *************

		// get context
		curCtx_ = &fncInfo.first;

		if (assembly_->regFileSize_ < curCtx_->GetRegCount())
		{	// allocate the necessary number of registers
			assembly_->regFileSize_ = curCtx_->GetRegCount();
		}

		if (assembly_->regFileSize_ < (curCtx_->GetArgCount() + 2))
		{	// we need 2 more registers in order to facilitate call
			assembly_->regFileSize_ = curCtx_->GetArgCount() + 2;
		}

		// move ABP into r0
		append(new FI_get_ABP(nullptr, 0, 0))->setTarget();

		// store entry point
		codeIndex_.insert(std::make_pair(&fncInfo.second, assembly_->code_.back()));

		// gather arguments
		std::vector<size_t> offsets = { ABP_OFFSET, RET_OFFSET };

		for (auto arg : fnc.args)
			offsets.push_back(curCtx_->getVarInfo(arg).getStackOffset());

		// build the stack frame in r0
		append(new FI_build_struct(nullptr, 0, 0, offsets));

		// move void ptr of size 1 into r1
		append(new FI_load_cst(nullptr, 1, Data::createVoidPtr(1)));

		// get function name
		std::ostringstream ss;
		ss << nameOf(fnc) << ':' << uidOf(fnc);

		// allocate stack frame to r1
		append(
			new FI_node_create(
				/* instruction */ nullptr,
				/* dst reg where the node will be stored */ 1,
				/* reg with the value from which the node is to be created */ 1,
				/* size of the created node */ 1,
				/* type information */ boxMan_.getTypeInfo(ss.str()),
				/* selectors of the node */ curCtx_->GetStackFrameLayout()
			)
		);

		// store arguments to the new frame (r1)
		append(
			new FI_stores(
				/* instruction */ nullptr,
				/* reg with addr of dst */ 1,
				/* src reg */ 0,
				/* offset of the dst */ 0
			)
		);

		// set new ABP (r1)
		append(new FI_set_greg(nullptr, ABP_INDEX, 1));

		//         ************  execute the function  *************

		// jump to the beginning of the first block of the function control-flow
		// graph
		append(new FI_jmp(nullptr, fnc.cfg.entry()));

		// compute loop entry points
		loopAnalyser_.init(fnc.cfg.entry());

		// compile underlying CFG
		std::list<const CodeStorage::Block*> queue;

		queue.push_back(fnc.cfg.entry());

		bool first = true;

		while (!queue.empty())
		{	// until all used basic blocks are compiled
			const CodeStorage::Block* block = queue.front();
			queue.pop_front();

			auto p = codeIndex_.insert(std::make_pair(block,
				static_cast<AbstractInstruction*>(nullptr)));

			if (!p.second)
				continue;

			size_t blockHead = assembly_->code_.size();

			// compile the block, abstract when the block is the first
			compileBlock(block, first);

			assert(blockHead < assembly_->code_.size());

			p.first->second = assembly_->code_[blockHead];

			for (auto target : block->targets())
				queue.push_back(target);

			first = false;
		}

		auto iter = codeIndex_.find(fnc.cfg.entry());
		assert(iter != codeIndex_.end());
		assembly_->functionIndex_.insert(std::make_pair(&fnc, iter->second));
	}

	/**
	 * @brief  Compiles initialisation
	 *
	 * Compiles the initialisation of global registers, global variables, etc.
	 *
	 * @param[in]   stor        Code storage with the code
	 */
	void compileInitialisation(const CodeStorage::Storage& stor)
	{
		//     ********  prepare the structure with global variables ********
		std::vector<SelData> globalVarsLayout;
		SymCtx::var_map_type globalVarMap;
		size_t globalVarsOffset = 0;

		for (const CodeStorage::Var& var : stor.vars)
		{
			if (CodeStorage::EVar::VAR_GL == var.code)
			{
				NodeBuilder::buildNode(globalVarsLayout, var.type, globalVarsOffset,
					var.name);
				globalVarMap.insert(
					std::make_pair(var.uid, VarInfo::createGlobal(globalVarsOffset)));
				globalVarsOffset += var.type->size;
			}
		}

		//     ********  prepare functions' symbolic contexts ********
		for (auto fnc : stor.fncs)
		{
			if (isDefined(*fnc))
			{	// in case the function is defined and not only declared
				fncIndex_.insert(std::make_pair(
					fnc,
					std::make_pair(
						SymCtx(*fnc, &globalVarMap),
						CodeStorage::Block()
					)
				));
			}
		}

		//     ******* compile creation of global variables *******
		// move void ptr of size 1 into r0
		append(new FI_load_cst(nullptr, 0, Data::createVoidPtr(1)));

		if (globalVarsLayout.empty())
		{	// if there are no global variables, fake one
			globalVarsLayout.push_back(SelData(0, 1, 0, "__fake_global__"));
		}

		// allocate the block with global variables to r0
		append(
			new FI_node_create(
				/* instruction */ nullptr,
				/* dst reg where the node will be stored */ 0,
				/* reg with the value from which the node is to be created */ 0,
				/* size of the created node */ 1,
				/* type information */ boxMan_.getTypeInfo(GLOBAL_VARS_BLOCK_STR),
				/* selectors of the node */ globalVarsLayout
			)
		);

		//     ******* load global registers *******
		// push r0 as GLOB
		append(new FI_push_greg(nullptr, 0));

		// load NULL into r0
		append(new FI_load_cst(nullptr, 0, Data::createInt(0)));

		// push r0 as ABP
		append(new FI_push_greg(nullptr, 0));

		//     ******* compile initialization of global variables *******
		curCtx_ = new SymCtx(&globalVarMap);

		for (const CodeStorage::Var& var : stor.vars)
		{
			if (CodeStorage::EVar::VAR_GL == var.code)
			{	// for each global variable
				if (!var.isExtern)
				{	// if the variable has internal linkage
					// assert it is initialised (explicitly or implicitely)
					assert(var.initialized);

					if (!var.initials.empty())
					{	// in case the variable is initialised explicitly
						for (const CodeStorage::Insn* insn : var.initials)
						{
							// Assertions
							assert(nullptr != insn);
							assert((cl_insn_e::CL_INSN_UNOP == insn->code)
								|| (cl_insn_e::CL_INSN_BINOP == insn->code));

							/// @todo: perform implicit zeroing (e.g. for structures)?
							compileInstruction(*insn);
						}
					}
					else
					{	// in case the variable is initialised implicitly

						// NOTE: this is a very basic implementation with only very basic
						// support. Advanced features are not supported

						// load 0 into r0
						append(new FI_load_cst(nullptr, 0, Data::createInt(0)));

						auto itVarMap = globalVarMap.find(var.uid);
						assert(globalVarMap.end() != itVarMap);

						int offset = itVarMap->second.getGlobalBlockOffset();

						// load the variable into r1
						append(new FI_get_GLOB(nullptr, 1, offset));

						if (cl_type_e::CL_TYPE_STRUCT == var.type->code)
						{	// in case the operand is a structure
#if 0
							// build a symbolic node
							std::vector<size_t> offs;
							NodeBuilder::buildNode(offs, op.type);

							if (needsAcc)
							{	// in case separation is needed
								// append separation of a set of nodes
								append(new FI_acc_set(&insn, tmp, offset, offs));
							}

							// append store of the value into register
							append(new FI_stores(&insn, tmp, src, offset));
#endif
							throw NotImplementedException("Implicit initialization of a structure");
						} else
						{	// in case the operand is anything but a structure

							// store the value in r0 to the memory location pointed by r1
							append(new FI_store(nullptr, 1, 0, offset));
						}


						// TODO: is the check instruction really necessary here?

						// add an instruction to check invariants of the virtual machine
						append(new FI_check(nullptr));
						//throw NotImplementedException("Implicitly initialised global variable");
					}
				}
				else
				{	// if the variable has external linkage
					assert(!var.initialized);
					assert(var.initials.empty());

					/// @todo: create with undef value
					throw NotImplementedException("Global variable with external linkage");
				}
			}
		}

		delete curCtx_;
		curCtx_ = nullptr;
	}

public:

	/**
	 * @brief  The constructor
	 *
	 * The constructor sets the fixpoint backend, the tree automata backend, and
	 * the box manager.
	 *
	 * @param[in,out]  fixpointBackend  The fixpoint backend
	 * @param[in,out]  taBackend        Tree automata backend
	 * @param[in,out]  boxMan           The box manager
	 */
	Core(TreeAut::Backend& fixpointBackend,
		TreeAut::Backend& taBackend, BoxMan& boxMan) :
		assembly_{},
		codeIndex_{},
		fncIndex_{},
		curCtx_{},
		fixpointBackend_(fixpointBackend),
		taBackend_(taBackend),
		boxMan_(boxMan),
		builtinTable_{},
		loopAnalyser_{}
	{ }


	/**
	 * @brief  The method that compiles the code from the code storage
	 *
	 * Compiles the code from the entry point @p entry from the code storage @p
	 * stor into the provided assembly @p assembly.
	 *
	 * @param[out]  assembly  Assembly that serves as the output
	 * @param[in]   stor      Code storage with the code
	 * @param[in]   entry     The entry point of the program
	 */
	void compile(Compiler::Assembly& assembly, const CodeStorage::Storage& stor,
		const CodeStorage::Fnc& entry)
	{
		// clear the code in the assembly
		reset(assembly);

		// compile the initial code
		compileInitialisation(stor);

		//              ******* compile entry call *******

		// feed registers with arguments (unknown values)
		for (size_t i = entry.args.size() + 1; i > 1; --i)
			append(new FI_load_cst(nullptr, i, Data::createUnknw()));

		// create the instruction that we will return to after performing analysis
		// of the entry function
		AbstractInstruction* instr = new FI_check(nullptr);
		instr->setTarget();

		// store return address into r1
		append(new FI_load_cst(nullptr, 1, Data::createNativePtr(instr)));

		// call the entry point
		append(new FI_jmp(nullptr, &getFncInfo(&entry).second));

		//     ******* compile return from the entry function *******

		// push the previously created instruction into the assembly; it is set as
		// the return address of the call to the entry function
		append(instr);

		// pop return value into r0
		append(new FI_pop_greg(nullptr, 0));

		// pop ABP into r1
		append(new FI_pop_greg(nullptr, 1));

		// check the stack frame
		append(new FI_assert(nullptr, 1, Data::createInt(0)));

		// store the GLOB into r1
		append(new FI_get_GLOB(nullptr, 1, 0));

		// delete the block with global variables (r1)
		append(new FI_node_free(nullptr, 1));

		// check for garbage
		append(new FI_check(nullptr));

		// abort
		append(new FI_abort(nullptr));

		for (auto fnc : stor.fncs)
		{	// compile all functions in the code storage
			if (isDefined(*fnc))
				compileFunction(*fnc);
		}

		for (auto i = assembly_->code_.begin(); i != assembly_->code_.end(); ++i)
		{	// finalize all microinstructions
			(*i)->finalize(codeIndex_, i);
		}
	}
};


Compiler::Compiler(TreeAut::Backend& fixpointBackend,
	TreeAut::Backend& taBackend, BoxMan& boxMan)
	: core_(new Core(fixpointBackend, taBackend, boxMan))
{ }


Compiler::~Compiler()
{
	delete core_;
}


void Compiler::compile(Compiler::Assembly& assembly,
	const CodeStorage::Storage& stor, const CodeStorage::Fnc& entry)
{
	core_->compile(assembly, stor, entry);
}
