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

#include <sstream>
#include <cstdlib>

#include <list>
#include <unordered_set>

#include <cl/cldebug.hh>

#include "programerror.hh"
#include "symctx.hh"
#include "nodebuilder.hh"
#include "call.hh"
#include "jump.hh"
#include "comparison.hh"
#include "fixpoint.hh"
#include "microcode.hh"
#include "regdef.hh"

#include "compiler.hh"

struct OpWrapper {

	const cl_operand* op_;

	OpWrapper(const cl_operand& op) : op_(&op) {}

	friend std::ostream& operator<<(std::ostream& os, const OpWrapper& op) {

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

};

struct LoopAnalyser {

	struct BlockListItem {

		BlockListItem* prev;
		const CodeStorage::Block* block;

		BlockListItem(BlockListItem* prev, const CodeStorage::Block* block)
			: prev(prev), block(block) {}

		static bool lookup(const BlockListItem* item, const CodeStorage::Block* block) {
			if (!item)
				return false;
			if (item->block == block)
				return true;
			return BlockListItem::lookup(item->prev, block);
		}

	};

	std::unordered_set<const CodeStorage::Insn*> entryPoints;

	void visit(const CodeStorage::Block* block, std::unordered_set<const CodeStorage::Block*>& visited, BlockListItem* prev) {

		BlockListItem item(prev, block);

		if (!visited.insert(block).second) {
			if (BlockListItem::lookup(prev, block))
				this->entryPoints.insert(*block->begin());
			return;
		}

		for (auto target : block->targets())
			this->visit(target, visited, &item);

	}

	void init(const CodeStorage::Block* block) {

		std::unordered_set<const CodeStorage::Block*> visited;
		this->entryPoints.clear();
		this->visit(block, visited, NULL);

	}

	bool isEntryPoint(const CodeStorage::Insn* insn) const {
		return this->entryPoints.find(insn) != this->entryPoints.end();
	}

};

typedef enum { biNone, biMalloc, biFree, biNondet, biFix, biPrintHeap } builtin_e;

struct BuiltinTable {

	boost::unordered_map<std::string, builtin_e> _table;

public:

	BuiltinTable() {
		this->_table["malloc"] = builtin_e::biMalloc;
		this->_table["free"] = builtin_e::biFree;
		this->_table["__nondet"] = builtin_e::biNondet;
		this->_table["__fix"] = builtin_e::biFix;
		this->_table["__print_heap"] = builtin_e::biPrintHeap;
	}

	builtin_e operator[](const std::string& key) {
		boost::unordered_map<std::string, builtin_e>::iterator i = this->_table.find(key);
		return (i == this->_table.end())?(builtin_e::biNone):(i->second);
	}

};

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

	void cAbstraction() {
		this->append(
			new FI_abs(this->fixpointBackend, this->taBackend, this->boxMan)
		);
	}

	void cFixpoint() {
		this->append(
			new FI_fix(this->fixpointBackend, this->taBackend, this->boxMan)
		);
	}

	void cPrintHeap() {
		this->append(new FI_print_heap(this->curCtx));
	}

	void cLoadCst(size_t dst, const cl_operand& op) {

		switch (op.type->code) {

			case cl_type_e::CL_TYPE_INT:
			case cl_type_e::CL_TYPE_ENUM:
			case cl_type_e::CL_TYPE_PTR:
				this->append(
					new FI_load_cst(dst, Data::createInt(intCstFromOperand(&op)))
				);
				break;

			case cl_type_e::CL_TYPE_BOOL:
				this->append(
					new FI_load_cst(dst, Data::createBool(intCstFromOperand(&op)))
				);
				break;

			default:
				assert(false);
				break;

		}

	}
/*
	void cLoadVar(size_t dst, size_t offset) {

		this->append(new FI_load_ABP(dst, (int)offset));

	}
*/
	void cMoveReg(size_t dst, size_t src, int offset) {

		if (offset > 0) {

			this->append(new FI_move_reg_offs(dst, src, offset));

		} else {

			if (src != dst)
				this->append(new FI_move_reg(dst, src));

		}

	}

	static const cl_accessor* computeOffset(int& offset, const cl_accessor* acc) {

		while (acc && (acc->code == CL_ACCESSOR_ITEM)) {

			offset += acc->type->items[acc->data.item.id].offset;
			acc = acc->next;

		}

		return acc;

	}

	void cLoadReg(size_t dst, size_t src, const cl_operand& op) {

		const cl_accessor* acc = op.accessor;

		int offset = 0;

		if (acc && (acc->code == CL_ACCESSOR_DEREF)) {

			assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

			acc = Core::computeOffset(offset, acc->next);

			if (acc && (acc->code == CL_ACCESSOR_REF)) {

				assert(acc->next == NULL);
				this->cMoveReg(dst, src, offset);
				return;

			}

			assert(acc == NULL);

			if (op.type->code == cl_type_e::CL_TYPE_STRUCT) {

				std::vector<size_t> offs;
				NodeBuilder::buildNode(offs, op.type);

				this->append(new FI_acc_set(dst, offset, offs));
				this->append(new FI_loads(dst, dst, offset, offs));

			} else {

				this->append(new FI_acc_sel(dst, offset));
				this->append(new FI_load(dst, dst, offset));

			}

			this->append(new FI_check());

		} else {

			acc = Core::computeOffset(offset, acc);

			assert(acc == NULL);

			this->cMoveReg(dst, src, offset);

		}

	}

	bool cStoreReg(const cl_operand& op, size_t src, size_t tmp) {

		const cl_accessor* acc = op.accessor;

		int offset = 0;

		if (acc && (acc->code == CL_ACCESSOR_DEREF)) {

			assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

			acc = Core::computeOffset(offset, acc->next);

			assert(acc == NULL);

			if (op.type->code == cl_type_e::CL_TYPE_STRUCT) {

				std::vector<size_t> offs;
				NodeBuilder::buildNode(offs, op.type);

				this->append(new FI_acc_set(tmp, offset, offs));
				this->append(new FI_stores(tmp, src, offset));

			} else {

				this->append(new FI_acc_sel(tmp, offset));
				this->append(new FI_store(tmp, src, offset));

			}

			this->append(new FI_check());

			return true;

		} else {

			acc = Core::computeOffset(offset, acc);

			assert(acc == NULL);
			assert(offset == 0);

			if (src != tmp)
				this->append(new FI_move_reg(tmp, src));

			return false;

		}

	}

	size_t cLoadOperand(size_t dst, const cl_operand& op, bool canOverride = true) {

		switch (op.code) {

			case cl_operand_e::CL_OPERAND_VAR: {

				auto varInfo = this->curCtx->getVarInfo(varIdFromOperand(&op));

				if (varInfo.first) {

					// stack variable
					const cl_accessor* acc = op.accessor;

					int offset = 0;

					if (acc && (acc->code == CL_ACCESSOR_DEREF)) {

						assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

						this->append(new FI_load_ABP(dst, (int)varInfo.second));

						acc = Core::computeOffset(offset, acc->next);

						if (acc && (acc->code == CL_ACCESSOR_REF)) {

							assert(op.type->code == cl_type_e::CL_TYPE_PTR);

							assert(acc->next == NULL);

							if (offset)
								this->append(new FI_move_reg_offs(dst, dst, offset));

							break;

						}

						assert(acc == NULL);

						if (op.type->code == cl_type_e::CL_TYPE_STRUCT) {

							std::vector<size_t> offs;
							NodeBuilder::buildNode(offs, op.type);

							this->append(new FI_acc_set(dst, offset, offs));
							this->append(new FI_loads(dst, dst, offset, offs));

						} else {

							this->append(new FI_acc_sel(dst, offset));
							this->append(new FI_load(dst, dst, offset));

						}

					} else {

						offset = (int)varInfo.second;

						acc = Core::computeOffset(offset, acc);

						if (acc && (acc->code == CL_ACCESSOR_REF)) {

							assert(acc->next == NULL);
							this->append(new FI_get_ABP(dst, offset));
							break;

						}

						assert(acc == NULL);

						this->append(new FI_load_ABP(dst, offset));
//						this->cMoveReg(dst, src, offset);

					}


				} else {

					// register
					if (canOverride) {
						dst = varInfo.second;
						this->cLoadReg(dst, dst, op);
					} else {
						this->cLoadReg(dst, varInfo.second, op);
					}

				}

				break;

			}

			case cl_operand_e::CL_OPERAND_CST:
				this->cLoadCst(dst, op);
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

	bool cStoreOperand(const cl_operand& op, size_t src, size_t tmp) {

		switch (op.code) {

			case cl_operand_e::CL_OPERAND_VAR: {

				auto varInfo = this->curCtx->getVarInfo(varIdFromOperand(&op));

				if (varInfo.first) {

					// stack variable
					this->append(new FI_get_ABP(tmp, 0));

					const cl_accessor* acc = op.accessor;

					int offset = (int)varInfo.second;

					bool needsAcc = false;

					if (acc) {

						if (acc->code == CL_ACCESSOR_DEREF) {

							assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

							// override previous instruction
							this->override(new FI_load_ABP(tmp, varInfo.second));

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

						if (needsAcc)
							this->append(new FI_acc_set(tmp, offset, offs));
						this->append(new FI_stores(tmp, src, offset));

					} else {

						if (needsAcc)
							this->append(new FI_acc_sel(tmp, offset));
						this->append(new FI_store(tmp, src, offset));

					}

					this->append(new FI_check());

					return true;

				} else {

					// register
					return this->cStoreReg(op, src, varInfo.second);

				}

				break;

			}

			default:
				assert(false);
				return false;

		}

	}

	AbstractInstruction* cKillDeadVariables(const CodeStorage::TKillVarList& vars) {

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
			new FI_load_cst(0, (offs.size() > 1)?(Data::createStruct(tmp)):(Data::createUndef()))
		);

		this->append(new FI_get_ABP(1, 0));
		this->append(
			(offs.size() > 1)
				?((AbstractInstruction*)new FI_stores(1, 0, 0))
				:((AbstractInstruction*)new FI_store(1, 0, *offs.begin()))
		);

		return result;

	}

	void compileAssignment(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src = insn.operands[1];

		assert(src.type->code == dst.type->code);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t srcReg = this->cLoadOperand(dstReg, src);

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
					srcReg,
					srcReg,
					dst.type->items[0].type->size,
					this->boxMan.getTypeInfo(typeName),
					sels
				)
			);

		}

		this->cStoreOperand(dst, srcReg, 1);
		this->cKillDeadVariables(insn.varsToKill);

	}

	void compileTruthNot(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src = insn.operands[1];

		assert(dst.type->code == cl_type_e::CL_TYPE_BOOL);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t srcReg = this->cLoadOperand(dstReg, src);

		switch (src.type->code) {

			case cl_type_e::CL_TYPE_BOOL:
				this->append(new FI_bnot(srcReg));
				break;

			case cl_type_e::CL_TYPE_INT:
				this->append(new FI_inot(srcReg));
				break;

			default:
				assert(false);

		}

		this->cStoreOperand(dst, srcReg, 1);
		this->cKillDeadVariables(insn.varsToKill);

	}

	void compileMalloc(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src = insn.operands[2];

		assert(src.type->code == cl_type_e::CL_TYPE_INT);
		assert(dst.type->code == cl_type_e::CL_TYPE_PTR);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t srcReg = this->cLoadOperand(dstReg, src);

		this->append(new FI_alloc(srcReg, srcReg));

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
					srcReg,
					srcReg,
					dst.type->items[0].type->size,
					this->boxMan.getTypeInfo(typeName),
					sels
				)
			);

		}

		this->cStoreOperand(dst, srcReg, 1);
		this->cKillDeadVariables(insn.varsToKill);

	}

	void compileFree(const CodeStorage::Insn& insn) {

		const cl_operand& src = insn.operands[2];

		size_t srcReg = this->cLoadOperand(0, src);

		this->append(new FI_acc_all(srcReg));
		this->append(new FI_node_free(srcReg));
		this->append(new FI_check());
		this->cKillDeadVariables(insn.varsToKill);

	}

	template <class F>
	void compileCmp(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src1 = insn.operands[1];
		const cl_operand& src2 = insn.operands[2];

		assert(dst.type->code == cl_type_e::CL_TYPE_BOOL);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t src1Reg = this->cLoadOperand(0, src1);
		size_t src2Reg = this->cLoadOperand(1, src2);

		this->append(new F(dstReg, src1Reg, src2Reg));
		this->cStoreOperand(dst, dstReg, 1);
		this->cKillDeadVariables(insn.varsToKill);

	}

	void compilePlus(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src1 = insn.operands[1];
		const cl_operand& src2 = insn.operands[2];

		assert(dst.type->code == cl_type_e::CL_TYPE_INT);
		assert(src1.type->code == cl_type_e::CL_TYPE_INT);
		assert(src2.type->code == cl_type_e::CL_TYPE_INT);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t src1Reg = this->cLoadOperand(0, src1);
		size_t src2Reg = this->cLoadOperand(1, src2);

		this->append(new FI_iadd(dstReg, src1Reg, src2Reg));
		this->cStoreOperand(dst, dstReg, 1);
		this->cKillDeadVariables(insn.varsToKill);

	}

	void compilePointerPlus(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src1 = insn.operands[1];
		const cl_operand& src2 = insn.operands[2];

		assert(dst.type->code == cl_type_e::CL_TYPE_PTR);
		assert(src1.type->code == cl_type_e::CL_TYPE_PTR);
		assert(src2.type->code == cl_type_e::CL_TYPE_INT);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t src1Reg = this->cLoadOperand(0, src1);
		size_t src2Reg = this->cLoadOperand(1, src2);

		this->append(new FI_move_reg_inc(dstReg, src1Reg, src2Reg));
		this->cStoreOperand(dst, dstReg, 1);
		this->cKillDeadVariables(insn.varsToKill);

	}

	void compileJmp(const CodeStorage::Insn& insn) {

		this->append(new FI_jmp(insn.targets[0]));

	}

	void compileCallInternal(const CodeStorage::Insn& insn, const CodeStorage::Fnc& fnc) {

		assert(fnc.args.size() + 2 == insn.operands.size());

		// feed registers with arguments (r2 ... )
		for (size_t i = fnc.args.size() + 1; i > 1; --i)
			this->cLoadOperand(i, insn.operands[i], false);

		CodeStorage::TKillVarList varsToKill = insn.varsToKill;

		// kill also the destination variable if possible

		if (insn.operands[0].code == cl_operand_e::CL_OPERAND_VAR) {

			auto varId = varIdFromOperand(&insn.operands[0]);

			if (this->curCtx->getVarInfo(varId).first) {

				auto acc = insn.operands[0].accessor;

				if (!acc || (acc->code != CL_ACCESSOR_DEREF))
					varsToKill.push_back(CodeStorage::KillVar(varId, false));

			}

		}

		// kill dead variables
		this->cKillDeadVariables(varsToKill);

		size_t head = this->assembly->code_.size();

		// put placeholder for loading return address into r1
		this->append(NULL);

		// call
		this->append(new FI_jmp(&this->getFncInfo(&fnc).second));

		// load ABP into r1
		this->append(new FI_get_ABP(1, 0));

		// isolate adjacent nodes (current ABP)
		this->append(new FI_acc_all(1));

		size_t head2 = this->assembly->code_.size();

		// fixpoint
		this->cFixpoint();

		this->assembly->code_[head2]->insn(&insn);

		// pop return value into r0
		this->append(new FI_pop_greg(0));

		// collect result from r0
		if (insn.operands[0].code != CL_OPERAND_VOID)
			this->cStoreOperand(insn.operands[0], 0, 1);

		// construct instruction for loading return address
		this->assembly->code_[head] =
			new FI_load_cst(1, Data::createNativePtr(this->assembly->code_[head + 2]));

		// set target flag
		this->assembly->code_[head + 2]->setTarget();

	}

	void compileRet(const CodeStorage::Insn& insn) {

		// move return value into r0
		if (insn.operands[0].code != CL_OPERAND_VOID)
			this->cLoadOperand(0, insn.operands[0], false);

		// push r0 to gr1
		this->append(new FI_push_greg(0));

		// load previous ABP into r0
		this->append(new FI_load_ABP(0, ABP_OFFSET));

		// store current ABP into r1
		this->append(new FI_get_ABP(1, 0));

		// restore previous ABP (r0)
		this->append(new FI_set_greg(ABP_INDEX, 0));

		// move return address into r0
		this->append(new FI_load(0, 1, RET_OFFSET));

		// delete stack frame (r1)
		this->append(new FI_node_free(1));

		// return to r0
		this->append(new FI_ret(0));

	}

	void compileCond(const CodeStorage::Insn& insn) {

		const cl_operand& src = insn.operands[0];

		size_t srcReg = this->cLoadOperand(0, src);

		this->cKillDeadVariables(insn.varsToKill);

		AbstractInstruction* tmp[2] = { NULL, NULL };

		size_t sentinel = this->assembly->code_.size();

		this->append(NULL);

		for (auto i : { 0, 1 }) {

			tmp[i] = this->cKillDeadVariables(insn.killPerTarget[i]);

			this->append(new FI_jmp(insn.targets[i]));

			if (!tmp[i])
				tmp[i] = this->assembly->code_.back();

		}

		this->assembly->code_[sentinel] = new FI_cond(srcReg, tmp);

	}

	void compileNondet(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];

		size_t dstReg = this->lookupStoreReg(dst, 0);

		this->append(new FI_load_cst(dstReg, Data::createUnknw()));
		this->cStoreOperand(dst, dstReg, 1);
		this->cKillDeadVariables(insn.varsToKill);

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
				this->cFixpoint();
				return;
			case builtin_e::biPrintHeap:
				this->cPrintHeap();
				return;
			default:
				break;
		}

		const CodeStorage::Fnc* fnc = insn.stor->fncs[insn.operands[1].data.cst.data.cst_fnc.uid];

		if (!isDefined(*fnc)) {
			CL_NOTE_MSG(&insn.loc, "ignoring call to undefined function '" << insn.operands[1].data.cst.data.cst_fnc.name << '\'');
			if (insn.operands[0].code != CL_OPERAND_VOID) {
				this->append(new FI_load_cst(0, Data::createUnknw()));
				this->cStoreOperand(insn.operands[0], 0, 1);
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
					case cl_unop_e::CL_UNOP_TRUTH_NOT:
						this->compileTruthNot(insn);
						break;
					default:
						throw std::runtime_error("feature not implemented");
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
						throw std::runtime_error("feature not implemented");
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

			default:
				throw std::runtime_error("feature not implemented");

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
		this->append(new FI_get_ABP(0, 0))->setTarget();

		// store entry point
		this->codeIndex.insert(std::make_pair(&fncInfo.second, this->assembly->code_.back()));

		// gather arguments
		std::vector<size_t> offsets = { ABP_OFFSET, RET_OFFSET };

		for (auto arg : fnc.args)
			offsets.push_back(this->curCtx->getVarInfo(arg).second);

		// build structure in r0
		this->append(new FI_build_struct(0, 0, offsets));

		// build stack frame

		// move void ptr of size 1 into r1
		this->append(new FI_load_cst(1, Data::createVoidPtr(1)));

		// get function name
		std::ostringstream ss;
		ss << nameOf(fnc) << ':' << uidOf(fnc);

		// allocate stack frame to r1
		this->append(
			new FI_node_create(1, 1, 1, this->boxMan.getTypeInfo(ss.str()), this->curCtx->sfLayout)
		);

		// store arguments to the new frame (r1)
		this->append(new FI_stores(1, 0, 0));

		// set new ABP (r1)
		this->append(new FI_set_greg(ABP_INDEX, 1));

		// jump to the beginning of the first block
		this->append(new FI_jmp(fnc.cfg.entry()));

		// compute loop entry points
		this->loopAnalyser.init(fnc.cfg.entry());

		// compile underlying CFG
		std::list<const CodeStorage::Block*> queue;

		queue.push_back(fnc.cfg.entry());

		bool first = true;

		while (!queue.empty()) {

			const CodeStorage::Block* block = queue.front();
			queue.pop_front();

			auto p = this->codeIndex.insert(std::make_pair(block, (AbstractInstruction*)NULL));
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

	void compile(Compiler::Assembly& assembly, const CodeStorage::Storage& stor, const CodeStorage::Fnc& entry) {

		this->reset(assembly);

		for (auto fnc : stor.fncs) {

			if (isDefined(*fnc))
				this->fncIndex.insert(std::make_pair(fnc, std::make_pair(SymCtx(*fnc), CodeStorage::Block())));

		}

		// compile entry call

		// load NULL into r0
		this->append(new FI_load_cst(0, Data::createInt(0)));

		// push r0 as ABP
		this->append(new FI_push_greg(0));

		// feed registers with arguments (unknown values)
		for (size_t i = entry.args.size() + 1; i > 1; --i)
			this->append(new FI_load_cst(i, Data::createUnknw()));

		AbstractInstruction* instr = new FI_check();

		// set target flag
		instr->setTarget();

		// store return address into r1
		this->append(new FI_load_cst(1, Data::createNativePtr(instr)));

		// call
		this->append(new FI_jmp(&this->getFncInfo(&entry).second));

		this->append(instr);

		// pop return value into r0
		this->append(new FI_pop_greg(0));

		// pop ABP into r1
		this->append(new FI_pop_greg(1));

		// check stack frame
		this->append(new FI_assert(1, Data::createInt(0)));

		// abort
		this->append(new FI_abort());

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
