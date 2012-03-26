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

// Standard library headers
#include <sstream>
#include <cstdlib>
#include <list>
#include <unordered_set>

// Code Listener headers
#include <cl/cldebug.hh>

// Forester headers
#include "programerror.hh"
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


typedef enum { biNone, biMalloc, biFree, biNondet, biFix, biAbort, biPrintHeap } builtin_e;


struct BuiltinTable {

	boost::unordered_map<std::string, builtin_e> _table;

public:

	BuiltinTable() {
		this->_table["malloc"] = builtin_e::biMalloc;
		this->_table["free"] = builtin_e::biFree;
		this->_table["__nondet"] = builtin_e::biNondet;
		this->_table["__fix"] = builtin_e::biFix;
		this->_table["__print_heap"] = builtin_e::biPrintHeap;
		this->_table["abort"] = builtin_e::biAbort;
	}

	builtin_e operator[](const std::string& key) {
		boost::unordered_map<std::string, builtin_e>::iterator i = this->_table.find(key);
		return (i == this->_table.end())?(builtin_e::biNone):(i->second);
	}

};

/// @todo: document!!!
class Compiler::Core {

	Compiler::Assembly* assembly;
	std::unordered_map<const CodeStorage::Block*, AbstractInstruction*> codeIndex;
	std::unordered_map<const CodeStorage::Fnc*, std::pair<SymCtx, CodeStorage::Block>> fncIndex;
	const SymCtx* curCtx;

	TA<label_type>::Backend& fixpointBackend;
	TA<label_type>::Backend& taBackend;
	BoxMan& boxMan;

	BuiltinTable builtinTable;
	LoopAnalyser loopAnalyser;

protected:

	std::pair<SymCtx, CodeStorage::Block>& getFncInfo(const CodeStorage::Fnc* fnc) {

		auto info = this->fncIndex.find(fnc);
		assert(info != this->fncIndex.end());
		return info->second;

	}

	void reset(Compiler::Assembly& assembly) {

		this->assembly = &assembly;
		this->assembly->clear();
		this->codeIndex.clear();
		this->fncIndex.clear();

	}

	AbstractInstruction* append(AbstractInstruction* instr) {

		this->assembly->code_.push_back(instr);

		return instr;

	}

	AbstractInstruction* override(AbstractInstruction* instr) {

		assert(this->assembly->code_.size());

		delete this->assembly->code_.back();

		this->assembly->code_.back() = instr;

		return instr;

	}

	void cAbstraction(const CodeStorage::Insn* insn = nullptr) {
		this->append(
			new FI_abs(insn, this->fixpointBackend, this->taBackend, this->boxMan)
		);
	}

	void cFixpoint(const CodeStorage::Insn& insn) {
		this->append(
			new FI_fix(&insn, this->fixpointBackend, this->taBackend, this->boxMan)
		);
	}

	void cPrintHeap(const CodeStorage::Insn& insn) {
		this->append(new FI_print_heap(&insn, this->curCtx));
	}

	void cLoadCst(size_t dst, const cl_operand& op,
		const CodeStorage::Insn& insn) {

		// Assertions
		assert(op.code == cl_operand_e::CL_OPERAND_CST);
		assert(op.type != nullptr);

		switch (op.type->code) {
			// according to the type of the operand
			case cl_type_e::CL_TYPE_INT:
			case cl_type_e::CL_TYPE_ENUM:
				this->append(
					new FI_load_cst(&insn, dst, Data::createInt(intCstFromOperand(&op)))
				);
				break;

			case cl_type_e::CL_TYPE_PTR:
				switch (op.data.cst.code) {
					// according to the type of the constant
					case cl_type_e::CL_TYPE_INT:
						this->append(
							new FI_load_cst(&insn, dst, Data::createInt(intCstFromOperand(&op)))
						);
						break;

					case cl_type_e::CL_TYPE_STRING:
						this->append(new FI_load_cst(&insn, dst, Data::createUnknw()));
						break;

					default:
						throw NotImplementedException(translTypeCode(op.data.cst.code) +
							": pointer constant type", &insn.loc);
				}
				break;

			case cl_type_e::CL_TYPE_BOOL:
				this->append(
					new FI_load_cst(&insn, dst, Data::createBool(intCstFromOperand(&op)))
				);
				break;

			default:
				throw NotImplementedException(translTypeCode(op.data.cst.code) +
					": constant type", &insn.loc);
		}
	}
/*
	void cLoadVar(size_t dst, size_t offset) {

		this->append(new FI_load_ABP(dst, (int)offset));

	}
*/
	void cMoveReg(size_t dst, size_t src, int offset,
		const CodeStorage::Insn& insn) {

		if (offset > 0) {

			this->append(new FI_move_reg_offs(&insn, dst, src, offset));

		} else {

			if (src != dst)
				this->append(new FI_move_reg(&insn, dst, src));

		}

	}

	static const cl_accessor* computeOffset(int& offset, const cl_accessor* acc) {

		while (acc && (acc->code == CL_ACCESSOR_ITEM)) {

			offset += acc->type->items[acc->data.item.id].offset;
			acc = acc->next;

		}

		return acc;

	}

	void cLoadReg(size_t dst, size_t src, const cl_operand& op,
		const CodeStorage::Insn& insn) {

		const cl_accessor* acc = op.accessor;

		int offset = 0;

		if (acc && (acc->code == CL_ACCESSOR_DEREF)) {

			assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

			acc = Core::computeOffset(offset, acc->next);

			if (acc && (acc->code == CL_ACCESSOR_REF)) {

				assert(acc->next == NULL);
				this->cMoveReg(dst, src, offset, insn);
				return;

			}

			assert(acc == NULL);

			if (op.type->code == cl_type_e::CL_TYPE_STRUCT) {

				std::vector<size_t> offs;
				NodeBuilder::buildNode(offs, op.type);

				this->append(new FI_acc_set(&insn, dst, offset, offs));
				this->append(new FI_loads(&insn, dst, dst, offset, offs));

			} else {

				this->append(new FI_acc_sel(&insn, dst, offset));
				this->append(new FI_load(&insn, dst, dst, offset));

			}

			this->append(new FI_check(&insn));

		} else {

			acc = Core::computeOffset(offset, acc);

			assert(acc == NULL);

			this->cMoveReg(dst, src, offset, insn);

		}

	}

	bool cStoreReg(const cl_operand& op, size_t src, size_t tmp,
		const CodeStorage::Insn& insn) {

		const cl_accessor* acc = op.accessor;

		int offset = 0;

		if (acc && (acc->code == CL_ACCESSOR_DEREF)) {

			assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

			acc = Core::computeOffset(offset, acc->next);

			assert(acc == NULL);

			if (op.type->code == cl_type_e::CL_TYPE_STRUCT) {

				std::vector<size_t> offs;
				NodeBuilder::buildNode(offs, op.type);

				this->append(new FI_acc_set(&insn, tmp, offset, offs));
				this->append(new FI_stores(&insn, tmp, src, offset));

			} else {

				this->append(new FI_acc_sel(&insn, tmp, offset));
				this->append(new FI_store(&insn, tmp, src, offset));

			}

			this->append(new FI_check(&insn));

			return true;

		} else {

			acc = Core::computeOffset(offset, acc);

			assert(acc == NULL);
			assert(offset == 0);

			if (src != tmp)
				this->append(new FI_move_reg(&insn, tmp, src));

			return false;

		}

	}

	size_t cLoadOperand(size_t dst, const cl_operand& op,
		const CodeStorage::Insn& insn, bool canOverride = true) {

		switch (op.code) {

			case cl_operand_e::CL_OPERAND_VAR: {

				auto varInfo = this->curCtx->getVarInfo(varIdFromOperand(&op));

				if (varInfo.first) {

					// stack variable
					const cl_accessor* acc = op.accessor;

					int offset = 0;

					if (acc && (acc->code == CL_ACCESSOR_DEREF)) {

						assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

						this->append(new FI_load_ABP(&insn, dst, (int)varInfo.second));

						acc = Core::computeOffset(offset, acc->next);

						if (acc && (acc->code == CL_ACCESSOR_REF)) {

							assert(op.type->code == cl_type_e::CL_TYPE_PTR);

							assert(acc->next == NULL);

							if (offset)
								this->append(new FI_move_reg_offs(&insn, dst, dst, offset));

							break;

						}

						assert(acc == NULL);

						if (op.type->code == cl_type_e::CL_TYPE_STRUCT) {

							std::vector<size_t> offs;
							NodeBuilder::buildNode(offs, op.type);

							this->append(new FI_acc_set(&insn, dst, offset, offs));
							this->append(new FI_loads(&insn, dst, dst, offset, offs));

						} else {

							this->append(new FI_acc_sel(&insn, dst, offset));
							this->append(new FI_load(&insn, dst, dst, offset));

						}

					} else {

						offset = (int)varInfo.second;

						acc = Core::computeOffset(offset, acc);

						if (acc && (acc->code == CL_ACCESSOR_REF)) {

							assert(acc->next == NULL);
							this->append(new FI_get_ABP(&insn, dst, offset));
							break;

						}

						assert(acc == NULL);

						this->append(new FI_load_ABP(&insn, dst, offset));
//						this->cMoveReg(dst, src, offset);

					}


				} else {

					// register
					if (canOverride) {
						dst = varInfo.second;
						this->cLoadReg(dst, dst, op, insn);
					} else {
						this->cLoadReg(dst, varInfo.second, op, insn);
					}

				}

				break;

			}

			case cl_operand_e::CL_OPERAND_CST:
				this->cLoadCst(dst, op, insn);
				break;

			default:
				assert(false);

		}

		return dst;

	}

	size_t lookupStoreReg(const cl_operand& op, size_t src) {

		size_t tmp = src;

		switch (op.code) {

			case cl_operand_e::CL_OPERAND_VAR: {

				auto varInfo = this->curCtx->getVarInfo(varIdFromOperand(&op));

				if (varInfo.first) {

					// stack variable
					return src;

				} else {

					// register
					tmp = varInfo.second;

				}

				const cl_accessor* acc = op.accessor;

				return (acc && (acc->code == CL_ACCESSOR_DEREF))?(src):(tmp);

			}

			default:
				assert(false);
				return 0;

		}

	}

	bool cStoreOperand(const cl_operand& op, size_t src, size_t tmp,
		const CodeStorage::Insn& insn) {

		switch (op.code) {

			case cl_operand_e::CL_OPERAND_VAR: {

				auto varInfo = this->curCtx->getVarInfo(varIdFromOperand(&op));

				if (varInfo.first) {

					// stack variable
					this->append(new FI_get_ABP(&insn, tmp, 0));

					const cl_accessor* acc = op.accessor;

					int offset = (int)varInfo.second;

					bool needsAcc = false;

					if (acc) {

						if (acc->code == CL_ACCESSOR_DEREF) {

							assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

							// override previous instruction
							this->override(new FI_load_ABP(&insn, tmp, varInfo.second));

							needsAcc = true;

							acc = acc->next;

							offset = 0;

						}

						acc = Core::computeOffset(offset, acc);

					}

					assert(acc == NULL);

					if (op.type->code == cl_type_e::CL_TYPE_STRUCT) {

						std::vector<size_t> offs;
						NodeBuilder::buildNode(offs, op.type);

						if (needsAcc) {
							this->append(new FI_acc_set(&insn, tmp, offset, offs));
						}
						this->append(new FI_stores(&insn, tmp, src, offset));

					} else {

						if (needsAcc) {
							this->append(new FI_acc_sel(&insn, tmp, offset));
						}
						this->append(new FI_store(&insn, tmp, src, offset));

					}

					this->append(new FI_check(&insn));

					return true;

				} else {

					// register
					return this->cStoreReg(op, src, varInfo.second, insn);

				}

				break;

			}

			default:
				assert(false);
				return false;

		}

	}

	AbstractInstruction* cKillDeadVariables(const CodeStorage::TKillVarList& vars,
		const CodeStorage::Insn& insn) {

		std::set<size_t> offs;

		for (auto var : vars) {

			if (var.onlyIfNotPointed)
				continue;

			const std::pair<bool, size_t>& varInfo = this->curCtx->getVarInfo(var.uid);

			if (!varInfo.first)
				continue;

			offs.insert(varInfo.second);

		}

		if (offs.empty())
			return NULL;

		std::vector<Data::item_info> tmp;

		if (offs.size() > 1) {

			for (auto offset : offs)
				tmp.push_back(Data::item_info(offset, Data::createUndef()));

		}

		AbstractInstruction* result = this->append(
			new FI_load_cst(&insn, 0,
				(offs.size() > 1)?(Data::createStruct(tmp)):(Data::createUndef()))
		);

		this->append(new FI_get_ABP(&insn, 1, 0));
		this->append(
			(offs.size() > 1)
				?((AbstractInstruction*)new FI_stores(&insn, 1, 0, 0))
				:((AbstractInstruction*)new FI_store(&insn, 1, 0, *offs.begin()))
		);

		return result;

	}

	void compileAssignment(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src = insn.operands[1];

		// Assertions
		assert(src.type != nullptr);
		assert(dst.type != nullptr);
		assert(src.type->code == dst.type->code);


		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t srcReg = this->cLoadOperand(dstReg, src, insn);

		if (
			src.type->code == cl_type_e::CL_TYPE_PTR &&
			src.type->items[0].type->code == cl_type_e::CL_TYPE_VOID &&
			dst.type->items[0].type->code != cl_type_e::CL_TYPE_VOID
		) {
			std::vector<SelData> sels;
			NodeBuilder::buildNode(sels, dst.type->items[0].type);

			std::string typeName;
			if (dst.type->items[0].type->name)
				typeName = std::string(dst.type->items[0].type->name);
			else {
				std::ostringstream ss;
				ss << dst.type->items[0].type->uid;
				typeName = ss.str();
			}

			this->append(
				new FI_node_create(
					&insn,
					srcReg,
					srcReg,
					dst.type->items[0].type->size,
					this->boxMan.getTypeInfo(typeName),
					sels
				)
			);

		}

		this->cStoreOperand(dst, srcReg, 1, insn);
		this->cKillDeadVariables(insn.varsToKill, insn);

	}

	void compileTruthNot(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src = insn.operands[1];

		assert(dst.type->code == cl_type_e::CL_TYPE_BOOL);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t srcReg = this->cLoadOperand(dstReg, src, insn);

		switch (src.type->code) {

			case cl_type_e::CL_TYPE_BOOL:
				this->append(new FI_bnot(&insn, srcReg));
				break;

			case cl_type_e::CL_TYPE_INT:
				this->append(new FI_inot(&insn, srcReg));
				break;

			default:
				assert(false);

		}

		this->cStoreOperand(dst, srcReg, 1, insn);
		this->cKillDeadVariables(insn.varsToKill, insn);

	}

	void compileMalloc(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src = insn.operands[2];

		assert(src.type->code == cl_type_e::CL_TYPE_INT);
		assert(dst.type->code == cl_type_e::CL_TYPE_PTR);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t srcReg = this->cLoadOperand(dstReg, src, insn);

		this->append(new FI_alloc(&insn, srcReg, srcReg));

		if (dst.type->items[0].type->code != cl_type_e::CL_TYPE_VOID) {

			std::vector<SelData> sels;
			NodeBuilder::buildNode(sels, dst.type->items[0].type);

			std::string typeName;
			if (dst.type->items[0].type->name)
				typeName = std::string(dst.type->items[0].type->name);
			else {
				std::ostringstream ss;
				ss << dst.type->items[0].type->uid;
				typeName = ss.str();
			}

			this->append(
				new FI_node_create(
					&insn,
					srcReg,
					srcReg,
					dst.type->items[0].type->size,
					this->boxMan.getTypeInfo(typeName),
					sels
				)
			);

		}

		this->cStoreOperand(dst, srcReg, 1, insn);
		this->cKillDeadVariables(insn.varsToKill, insn);

	}

	void compileFree(const CodeStorage::Insn& insn) {

		const cl_operand& src = insn.operands[2];

		size_t srcReg = this->cLoadOperand(0, src, insn);

		this->append(new FI_acc_all(&insn, srcReg));
		this->append(new FI_node_free(&insn, srcReg));
		this->append(new FI_check(&insn));
		this->cKillDeadVariables(insn.varsToKill, insn);

	}

	template <class F>
	void compileCmp(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src1 = insn.operands[1];
		const cl_operand& src2 = insn.operands[2];

		assert(dst.type->code == cl_type_e::CL_TYPE_BOOL);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t src1Reg = this->cLoadOperand(0, src1, insn);
		size_t src2Reg = this->cLoadOperand(1, src2, insn);

		this->append(new F(dstReg, src1Reg, src2Reg));
		this->cStoreOperand(dst, dstReg, 1, insn);
		this->cKillDeadVariables(insn.varsToKill, insn);

	}

	void compilePlus(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src1 = insn.operands[1];
		const cl_operand& src2 = insn.operands[2];

		assert(dst.type->code == cl_type_e::CL_TYPE_INT);
		assert(src1.type->code == cl_type_e::CL_TYPE_INT);
		assert(src2.type->code == cl_type_e::CL_TYPE_INT);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t src1Reg = this->cLoadOperand(0, src1, insn);
		size_t src2Reg = this->cLoadOperand(1, src2, insn);

		this->append(new FI_iadd(&insn, dstReg, src1Reg, src2Reg));
		this->cStoreOperand(dst, dstReg, 1, insn);
		this->cKillDeadVariables(insn.varsToKill, insn);

	}

	void compilePointerPlus(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src1 = insn.operands[1];
		const cl_operand& src2 = insn.operands[2];

		assert(dst.type->code == cl_type_e::CL_TYPE_PTR);
		assert(src1.type->code == cl_type_e::CL_TYPE_PTR);
		assert(src2.type->code == cl_type_e::CL_TYPE_INT);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t src1Reg = this->cLoadOperand(0, src1, insn);
		size_t src2Reg = this->cLoadOperand(1, src2, insn);

		this->append(new FI_move_reg_inc(&insn, dstReg, src1Reg, src2Reg));
		this->cStoreOperand(dst, dstReg, 1, insn);
		this->cKillDeadVariables(insn.varsToKill, insn);

	}

	void compileJmp(const CodeStorage::Insn& insn) {

		this->append(new FI_jmp(&insn, insn.targets[0]));

	}

	void compileCallInternal(const CodeStorage::Insn& insn, const CodeStorage::Fnc& fnc) {

		assert(fnc.args.size() + 2 == insn.operands.size());

		// feed registers with arguments (r2 ... )
		for (size_t i = fnc.args.size() + 1; i > 1; --i)
			this->cLoadOperand(i, insn.operands[i], insn, false);

		CodeStorage::TKillVarList varsToKill = insn.varsToKill;

		// kill also the destination variable if possible

		if (insn.operands[0].code == cl_operand_e::CL_OPERAND_VAR) {

			auto varId = varIdFromOperand(&insn.operands[0]);

			if (this->curCtx->getVarInfo(varId).first) {

				auto acc = insn.operands[0].accessor;

				if (!acc || (acc->code != CL_ACCESSOR_DEREF))
					varsToKill.insert(CodeStorage::KillVar(varId, false));

			}

		}

		// kill dead variables
		this->cKillDeadVariables(varsToKill, insn);

		size_t head = this->assembly->code_.size();

		// put placeholder for loading return address into r1
		this->append(NULL);

		// call
		this->append(new FI_jmp(&insn, &this->getFncInfo(&fnc).second));

		// load ABP into r1
		this->append(new FI_get_ABP(&insn, 1, 0));

		// isolate adjacent nodes (current ABP)
		this->append(new FI_acc_all(&insn, 1));

		size_t head2 = this->assembly->code_.size();

		// fixpoint
		this->cFixpoint(insn);

		this->assembly->code_[head2]->insn(&insn);

		if (insn.operands[0].code != CL_OPERAND_VOID) {
			// pop return value into r0
			this->append(new FI_pop_greg(&insn, 0));
			// collect result from r0
			this->cStoreOperand(insn.operands[0], 0, 1, insn);

		}

		// construct instruction for loading return address
		this->assembly->code_[head] =
			new FI_load_cst(&insn, 1,
				Data::createNativePtr(this->assembly->code_[head + 2]));

		// set target flag
		this->assembly->code_[head + 2]->setTarget();

	}

	void compileRet(const CodeStorage::Insn& insn) {

		if (insn.operands[0].code != CL_OPERAND_VOID) {
			// move return value into r0
			this->cLoadOperand(0, insn.operands[0], insn, false);
			// push r0 to gr1
			this->append(new FI_push_greg(&insn, 0));

		}

		// load previous ABP into r0
		this->append(new FI_load_ABP(&insn, 0, ABP_OFFSET));

		// store current ABP into r1
		this->append(new FI_get_ABP(&insn, 1, 0));

		// restore previous ABP (r0)
		this->append(new FI_set_greg(&insn, ABP_INDEX, 0));

		// move return address into r0
		this->append(new FI_load(&insn, 0, 1, RET_OFFSET));

		// delete stack frame (r1)
		this->append(new FI_node_free(&insn, 1));

		// return to r0
		this->append(new FI_ret(&insn, 0));

	}

	void compileCond(const CodeStorage::Insn& insn) {

		const cl_operand& src = insn.operands[0];

		size_t srcReg = this->cLoadOperand(0, src, insn);

		this->cKillDeadVariables(insn.varsToKill, insn);

		AbstractInstruction* tmp[2] = { NULL, NULL };

		size_t sentinel = this->assembly->code_.size();

		this->append(NULL);

		for (auto i : { 0, 1 }) {

			tmp[i] = this->cKillDeadVariables(insn.killPerTarget[i], insn);

			this->append(new FI_jmp(&insn, insn.targets[i]));

			if (!tmp[i])
				tmp[i] = this->assembly->code_.back();

		}

		this->assembly->code_[sentinel] = new FI_cond(&insn, srcReg, tmp);

	}

	void compileNondet(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];

		size_t dstReg = this->lookupStoreReg(dst, 0);

		this->append(new FI_load_cst(&insn, dstReg, Data::createUnknw()));
		this->cStoreOperand(dst, dstReg, 1, insn);
		this->cKillDeadVariables(insn.varsToKill, insn);

	}

	void compileCall(const CodeStorage::Insn& insn) {

		assert(insn.operands[1].code == cl_operand_e::CL_OPERAND_CST);
		assert(insn.operands[1].data.cst.code == cl_type_e::CL_TYPE_FNC);
		switch (this->builtinTable[insn.operands[1].data.cst.data.cst_fnc.name]) {
			case builtin_e::biMalloc:
				this->compileMalloc(insn);
				return;
			case builtin_e::biFree:
				this->compileFree(insn);
				return;
			case builtin_e::biNondet:
				this->compileNondet(insn);
				return;
			case builtin_e::biFix:
				this->cFixpoint(insn);
				return;
			case builtin_e::biPrintHeap:
				this->cPrintHeap(insn);
				return;
			case builtin_e::biAbort:
				this->append(new FI_abort(&insn));
				return;
			default:
				break;
		}

		const CodeStorage::Fnc* fnc = insn.stor->fncs[insn.operands[1].data.cst.data.cst_fnc.uid];

		if (!isDefined(*fnc)) {
			CL_NOTE_MSG(&insn.loc, "ignoring call to undefined function '" << insn.operands[1].data.cst.data.cst_fnc.name << '\'');
			if (insn.operands[0].code != CL_OPERAND_VOID) {
				this->append(new FI_load_cst(&insn, 0, Data::createUnknw()));
				this->cStoreOperand(insn.operands[0], 0, 1, insn);
			}
		} else {
			this->compileCallInternal(insn, *fnc);
		}

	}

	void compileInstruction(const CodeStorage::Insn& insn) {

		CL_DEBUG_AT(3, insn.loc << ' ' << insn);

		switch (insn.code) {

			case cl_insn_e::CL_INSN_UNOP:
				switch (insn.subCode) {
					case cl_unop_e::CL_UNOP_ASSIGN:
						this->compileAssignment(insn);
						break;
					case cl_unop_e::CL_UNOP_BIT_NOT:
						// TODO
					case cl_unop_e::CL_UNOP_TRUTH_NOT:
						this->compileTruthNot(insn);
						break;
					default:
						throw NotImplementedException(translUnOpCode(insn.subCode),
							&insn.loc);
				}
				break;

			case cl_insn_e::CL_INSN_BINOP:
				switch (insn.subCode) {
					case cl_binop_e::CL_BINOP_EQ:
						this->compileCmp<FI_eq>(insn);
						break;
					case cl_binop_e::CL_BINOP_NE:
						this->compileCmp<FI_neq>(insn);
						break;
					case cl_binop_e::CL_BINOP_LT:
						this->compileCmp<FI_lt>(insn);
						break;
					case cl_binop_e::CL_BINOP_GT:
						this->compileCmp<FI_gt>(insn);
						break;
					case cl_binop_e::CL_BINOP_PLUS:
						this->compilePlus(insn);
						break;
/*					case cl_binop_e::CL_BINOP_MINUS:
						this->execMinus(state, parent, insn);
						break;*/
					case cl_binop_e::CL_BINOP_POINTER_PLUS:
						this->compilePointerPlus(insn);
						break;
					default:
						throw NotImplementedException(translBinOpCode(insn.subCode),
							&insn.loc);
				}
				break;

			case cl_insn_e::CL_INSN_CALL:
				this->compileCall(insn);
				break;

			case cl_insn_e::CL_INSN_RET:
				this->compileRet(insn);
				break;

			case cl_insn_e::CL_INSN_JMP:
				this->compileJmp(insn);
				break;

			case cl_insn_e::CL_INSN_COND:
				this->compileCond(insn);
				break;

			case cl_insn_e::CL_INSN_LABEL:
				// safe to ignore unless we support checking for error label reachability
				break;

			default:
				throw NotImplementedException(translInsnOpCode(insn.code),
					&insn.loc);
		}
	}

	void compileBlock(const CodeStorage::Block* block, bool abstract) {

		size_t head = this->assembly->code_.size();

		if (abstract || this->loopAnalyser.isEntryPoint(*block->begin()))
			this->cAbstraction();
/*		else
			this->cFixpoint();*/

		for (auto insn : *block) {

			this->compileInstruction(*insn);

			if (head == this->assembly->code_.size())
				continue;

			this->assembly->code_[head]->insn(insn);
/*
			for (size_t i = head; i < this->assembly->code_.size(); ++i)
				CL_CDEBUG(this->assembly->code_[i] << ":\t" << *this->assembly->code_[i]);
*/
			head = this->assembly->code_.size();

		}

	}

	void compileFunction(const CodeStorage::Fnc& fnc) {

		std::pair<SymCtx, CodeStorage::Block>& fncInfo = this->getFncInfo(&fnc);

		// get context
		this->curCtx = &fncInfo.first;

		if (this->assembly->regFileSize_ < this->curCtx->regCount)
			this->assembly->regFileSize_ = this->curCtx->regCount;

		// we need 2 more registers in order to facilitate call
		if (this->assembly->regFileSize_ < (this->curCtx->argCount + 2))
			this->assembly->regFileSize_ = this->curCtx->argCount + 2;

		// move ABP into r0
		this->append(new FI_get_ABP(nullptr, 0, 0))->setTarget();

		// store entry point
		this->codeIndex.insert(std::make_pair(&fncInfo.second, this->assembly->code_.back()));

		// gather arguments
		std::vector<size_t> offsets = { ABP_OFFSET, RET_OFFSET };

		for (auto arg : fnc.args)
			offsets.push_back(this->curCtx->getVarInfo(arg).second);

		// build structure in r0
		this->append(new FI_build_struct(nullptr, 0, 0, offsets));

		// build stack frame

		// move void ptr of size 1 into r1
		this->append(new FI_load_cst(nullptr, 1, Data::createVoidPtr(1)));

		// get function name
		std::ostringstream ss;
		ss << nameOf(fnc) << ':' << uidOf(fnc);

		// allocate stack frame to r1
		this->append(
			new FI_node_create(nullptr, 1, 1, 1, this->boxMan.getTypeInfo(ss.str()),
				this->curCtx->sfLayout)
		);

		// store arguments to the new frame (r1)
		this->append(new FI_stores(nullptr, 1, 0, 0));

		// set new ABP (r1)
		this->append(new FI_set_greg(nullptr, ABP_INDEX, 1));

		// jump to the beginning of the first block
		this->append(new FI_jmp(nullptr, fnc.cfg.entry()));

		// compute loop entry points
		this->loopAnalyser.init(fnc.cfg.entry());

		// compile underlying CFG
		std::list<const CodeStorage::Block*> queue;

		queue.push_back(fnc.cfg.entry());

		bool first = true;

		while (!queue.empty()) {

			const CodeStorage::Block* block = queue.front();
			queue.pop_front();

			auto p = this->codeIndex.insert(std::make_pair(block,
				(AbstractInstruction*)NULL));
			if (!p.second)
				continue;

			size_t blockHead = this->assembly->code_.size();

			this->compileBlock(block, first);

			assert(blockHead < this->assembly->code_.size());
			p.first->second = this->assembly->code_[blockHead];

			for (auto target : block->targets())
				queue.push_back(target);

			first = false;

		}

		auto iter = this->codeIndex.find(fnc.cfg.entry());
		assert(iter != this->codeIndex.end());
		this->assembly->functionIndex_.insert(std::make_pair(&fnc, iter->second));

	}

public:

	Core(TA<label_type>::Backend& fixpointBackend, TA<label_type>::Backend& taBackend,
		BoxMan& boxMan)
		: fixpointBackend(fixpointBackend), taBackend(taBackend), boxMan(boxMan) {}

	void compile(Compiler::Assembly& assembly, const CodeStorage::Storage& stor,
		const CodeStorage::Fnc& entry) {

		this->reset(assembly);

		for (auto fnc : stor.fncs) {

			if (isDefined(*fnc))
				this->fncIndex.insert(std::make_pair(fnc, std::make_pair(SymCtx(*fnc), CodeStorage::Block())));

		}

		// compile entry call

		// load NULL into r0
		this->append(new FI_load_cst(nullptr, 0, Data::createInt(0)));

		// push r0 as ABP
		this->append(new FI_push_greg(nullptr, 0));

		// feed registers with arguments (unknown values)
		for (size_t i = entry.args.size() + 1; i > 1; --i)
			this->append(new FI_load_cst(nullptr, i, Data::createUnknw()));

		AbstractInstruction* instr = new FI_check(nullptr);

		// set target flag
		instr->setTarget();

		// store return address into r1
		this->append(new FI_load_cst(nullptr, 1, Data::createNativePtr(instr)));

		// call
		this->append(new FI_jmp(nullptr, &this->getFncInfo(&entry).second));

		this->append(instr);

		// pop return value into r0
		this->append(new FI_pop_greg(nullptr, 0));

		// pop ABP into r1
		this->append(new FI_pop_greg(nullptr, 1));

		// check stack frame
		this->append(new FI_assert(nullptr, 1, Data::createInt(0)));

		// abort
		this->append(new FI_abort(nullptr));

		for (auto fnc : stor.fncs) {

			if (isDefined(*fnc))
				this->compileFunction(*fnc);

		}

		for (auto i = this->assembly->code_.begin(); i != this->assembly->code_.end(); ++i)
			(*i)->finalize(this->codeIndex, i);

	}

};

Compiler::Compiler(TA<label_type>::Backend& fixpointBackend, TA<label_type>::Backend& taBackend,
BoxMan& boxMan)
	: core_(new Core(fixpointBackend, taBackend, boxMan)) {}

Compiler::~Compiler() {
	delete this->core_;
}

void Compiler::compile(Compiler::Assembly& assembly, const CodeStorage::Storage& stor, const CodeStorage::Fnc& entry) {
	this->core_->compile(assembly, stor, entry);
}
