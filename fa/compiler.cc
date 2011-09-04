/*
 * Copyright (C) 2011 Jiri Simacek
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

#include <sstream>
#include <cstdlib>

#include <list>
#include <unordered_set>

#include <cl/cldebug.hh>

#include "programerror.hh"
//#include "loopanalyser.hh"
#include "symctx.hh"
#include "nodebuilder.hh"
#include "microcode.hh"

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

typedef enum { biNone, biMalloc, biFree, biNondet } builtin_e;

struct BuiltinTable {

	boost::unordered_map<std::string, builtin_e> _table;

public:

	BuiltinTable() {
		this->_table["malloc"] = builtin_e::biMalloc;
		this->_table["free"] = builtin_e::biFree;
		this->_table["__nondet"] = builtin_e::biNondet;
	}

	builtin_e operator[](const std::string& key) {
		boost::unordered_map<std::string, builtin_e>::iterator i = this->_table.find(key);
		return (i == this->_table.end())?(builtin_e::biNone):(i->second);
	}

};

class Compiler::Core {

	Compiler::Assembly* assembly;
	std::unordered_map<const CodeStorage::Block*, AbstractInstruction*> codeIndex;
	const SymCtx* curCtx;

	TA<label_type>::Backend& fixpointBackend;
	TA<label_type>::Backend& taBackend;
	BoxMan& boxMan;
	const std::vector<const Box*>& boxes;

	BuiltinTable builtinTable;
	LoopAnalyser loopAnalyser;

protected:

	void reset(Compiler::Assembly& assembly) {

		this->assembly = &assembly;
		this->assembly->clear();
		this->codeIndex.clear();

	}

	void append(AbstractInstruction* instr) {

		this->assembly->code_.push_back(instr);

	}

	void cLoadCst(size_t dst, const cl_operand& op) {
		
		switch (op.data.cst.code) {

			case cl_type_e::CL_TYPE_INT:
				this->append(
					new FI_load_cst(dst, Data::createInt(op.data.cst.data.cst_int.value))
				);
				break;

			default:
				assert(false);

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

			assert(op.type->code == cl_type_e::CL_TYPE_PTR);

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
				this->append(new FI_loads(dst, offset, offs));

			} else {

				this->append(new FI_acc_sel(dst, offset));
				this->append(new FI_load(dst, offset));

			}

		} else {

			acc = Core::computeOffset(offset, acc);

			assert(acc == NULL);

			this->cMoveReg(dst, src, offset);

		}

	}

	void cStoreReg(const cl_operand& op, size_t src, size_t tmp) {

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

		} else {

			acc = Core::computeOffset(offset, acc);

			assert(acc == NULL);
			assert(offset == 0);

			if (src != tmp)
				this->append(new FI_move_reg(tmp, src));

		}

	}

	size_t cLoadOperand(size_t dst, const cl_operand& op) {

		switch (op.code) {

			case cl_operand_e::CL_OPERAND_VAR: {

				auto varInfo = this->curCtx->getVarInfo(varIdFromOperand(&op));

				if (varInfo.first) {

					// stack variable

					const cl_accessor* acc = op.accessor;
			
					int offset = 0;
			
					if (acc && (acc->code == CL_ACCESSOR_DEREF)) {
			
						this->append(new FI_load_ABP(dst, (int)varInfo.second));

						acc = Core::computeOffset(offset, acc->next);

						if (acc && (acc->code == CL_ACCESSOR_REF)) {
			
							assert(op.type->code == cl_type_e::CL_TYPE_PTR);

							assert(acc->next == NULL);
							this->append(new FI_move_reg_offs(dst, dst, offset));
							break;
			
						}
			
						assert(acc == NULL);

						if (op.type->code == cl_type_e::CL_TYPE_STRUCT) {
			
							std::vector<size_t> offs;
							NodeBuilder::buildNode(offs, op.type);
			
							this->append(new FI_acc_set(dst, offset, offs));
							this->append(new FI_loads(dst, offset, offs));
			
						} else {
			
							this->append(new FI_acc_sel(dst, offset));
							this->append(new FI_load(dst, offset));
			
						}
			
					} else {
			
						acc = Core::computeOffset(offset, acc);
			
						if (acc && (acc->code == CL_ACCESSOR_REF)) {
							
							assert(acc->next == NULL);
							this->append(new FI_move_ABP(dst, offset));
							break;

						}
					
						assert(acc == NULL);
						assert(offset == 0);
			
						this->append(new FI_load_ABP(dst, (int)varInfo.second));
//						this->cMoveReg(dst, src, offset);
			
					}


				} else {
					
					// register
					dst = varInfo.second;
					this->cLoadReg(dst, dst, op);

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

		}

	}

	void cStoreOperand(const cl_operand& op, size_t src, size_t tmp) {

		switch (op.code) {

			case cl_operand_e::CL_OPERAND_VAR: {

				auto varInfo = this->curCtx->getVarInfo(varIdFromOperand(&op));

				if (varInfo.first) {

					// stack variable
					this->append(new FI_move_ABP(tmp, varInfo.second));

					const cl_accessor* acc = op.accessor;
			
					int offset = 0;

					bool needsAcc = false;

					if (acc) {

						if (acc->code == CL_ACCESSOR_DEREF) {

							assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

							this->append(new FI_load(tmp, 0));

							needsAcc = true;

						}

						acc = Core::computeOffset(offset, acc->next);

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
			
				} else {
					
					// register
					this->cStoreReg(op, src, varInfo.second);

				}

				break;
				
			}

			default:
				assert(false);

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

//		AbstractInstruction* result = new FI_load_cst(0, Data::createUndef());
		AbstractInstruction* result = new FI_move_ABP(0, 0);

		this->append(result);

		std::vector<Data::item_info> tmp;
		
		for (auto offset : offs)
			tmp.push_back(Data::item_info(offset, Data::createUndef()));

		this->append(new FI_load_cst(1, Data::createStruct(tmp)));
		this->append(new FI_stores(0, 1, 0));
//		for (auto offset : offs)
//			this->append(new FI_assert(offset, Data::createUndef()));
/*
		for (auto offset : offs)
			this->append(new FI_store_ABP(0, offset));
*/
		this->append(new FI_check());

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

//			this->append(new FI_is_type(srcReg, data_type_e::t_void_ptr));
			this->append(new FI_node_create(this->boxMan, srcReg, srcReg, dst.type->items[0].type));
			
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

		if (dst.type->items[0].type->code == cl_type_e::CL_TYPE_VOID) {

			this->append(new FI_alloc(srcReg, srcReg));

		} else {

			this->append(new FI_node_alloc(this->boxMan, srcReg, srcReg, dst.type->items[0].type));

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

		assert(dst.type->code == cl_type_e::CL_TYPE_INT);
		assert(src1.type->code == cl_type_e::CL_TYPE_INT);
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

	void compileRet(const CodeStorage::Insn& insn) {

		this->append(new FI_ret());

	}

	void compileCond(const CodeStorage::Insn& insn) {

		const cl_operand& src = insn.operands[0];

		size_t srcReg = this->cLoadOperand(0, src);

		this->cKillDeadVariables(insn.varsToKill);

		AbstractInstruction* tmp[2] = { NULL, NULL };

		size_t blockHead = this->assembly->code_.size();

		this->append(NULL);

		for (auto i : { 0, 1 }) {

			tmp[i] = this->cKillDeadVariables(insn.killPerTarget[i]);

			this->append(new FI_jmp(insn.targets[i]));

			if (!tmp[i])
				tmp[i] = this->assembly->code_.back();

		}

		this->assembly->code_[blockHead] = new FI_cond(srcReg, tmp); 

	}

	void compileNondet(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];

		size_t dstReg = this->lookupStoreReg(dst, 0);

		this->append(new FI_load_cst(dstReg, Data::createUnknw()));
		this->cStoreOperand(dst, dstReg, 1);

	}

	void compileInstruction(const CodeStorage::Insn& insn) {

		CL_CDEBUG(insn.loc << ' ' << insn);
		
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
				assert(insn.operands[1].code == cl_operand_e::CL_OPERAND_CST);
				assert(insn.operands[1].data.cst.code == cl_type_e::CL_TYPE_FNC);
				switch (this->builtinTable[insn.operands[1].data.cst.data.cst_fnc.name]) {
					case builtin_e::biMalloc:
						this->compileMalloc(insn);
						break;
					case builtin_e::biFree:
						this->compileFree(insn);
						break;
					case builtin_e::biNondet:
						this->compileNondet(insn);
						break;
					default:
						throw std::runtime_error("feature not implemented");
				}
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

	void compileBlock(const CodeStorage::Block* block) {

		if (this->loopAnalyser.isEntryPoint(*block->begin())) {
			this->append(
				new FI_fix(this->fixpointBackend, this->taBackend, this->boxMan, this->boxes)
			);
		}

		for (auto insn : *block) {

			size_t head = this->assembly->code_.size();

			this->compileInstruction(*insn);

			for (size_t i = head; i < this->assembly->code_.size(); ++i)
				CL_CDEBUG(this->assembly->code_[i] << ":\t" << *this->assembly->code_[i]);

		}
		
	}

	void compileFunction(const CodeStorage::Fnc& fnc) {

		// build context
		SymCtx ctx(fnc);
		this->curCtx = &ctx;

		if (this->assembly->regFileSize_ < ctx.regCount)
			this->assembly->regFileSize_ = ctx.regCount;
		
		// compute loop entry points
		this->loopAnalyser.init(fnc.cfg.entry());

		auto functionHead = this->assembly->code_.size();

		// compile underlying CFG
		std::list<const CodeStorage::Block*> queue;

		queue.push_back(fnc.cfg.entry());

		while (!queue.empty()) {

			const CodeStorage::Block* block = queue.front();
			queue.pop_front();

			auto p = this->codeIndex.insert(std::make_pair(block, (AbstractInstruction*)NULL));
			if (!p.second)
				continue;

			size_t blockHead = this->assembly->code_.size();

			this->compileBlock(block);

			assert(blockHead < this->assembly->code_.size());
			p.first->second = this->assembly->code_[blockHead];

			for (auto target : block->targets())
				queue.push_back(target);

		}

		auto iter = this->codeIndex.find(fnc.cfg.entry());
		assert(iter != this->codeIndex.end());
		this->assembly->functionIndex_.insert(std::make_pair(&fnc, iter->second));

		for (auto i = functionHead; i < this->assembly->code_.size(); ++i)
			this->assembly->code_[i]->finalize(this->codeIndex, this->assembly->code_.begin() + i);
		
	}

public:

	Core(TA<label_type>::Backend& fixpointBackend, TA<label_type>::Backend& taBackend,
		BoxMan& boxMan, const std::vector<const Box*>& boxes)
		: fixpointBackend(fixpointBackend), taBackend(taBackend), boxMan(boxMan), boxes(boxes) {}

	void compile(Compiler::Assembly& assembly, const CodeStorage::Storage& stor) {

		this->reset(assembly);

		for (auto fnc : stor.fncs) {

			if (isDefined(*fnc))
				this->compileFunction(*fnc);

		}

	}

};

Compiler::Compiler(TA<label_type>::Backend& fixpointBackend, TA<label_type>::Backend& taBackend,
BoxMan& boxMan, const std::vector<const Box*>& boxes)
	: core_(new Core(fixpointBackend, taBackend, boxMan, boxes)) {}

Compiler::~Compiler() {
	delete this->core_;
}

void Compiler::compile(Compiler::Assembly& assembly, const CodeStorage::Storage& stor) {
	this->core_->compile(assembly, stor);
}
