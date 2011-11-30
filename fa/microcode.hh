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

#ifndef MICROCODE_H
#define MICROCODE_H

#include <vector>
#include <unordered_map>

#include "abstractinstruction.hh"
#include "sequentialinstruction.hh"

class FI_cond : public AbstractInstruction {

	size_t src_;

	AbstractInstruction* next_[2];

public:

	FI_cond(size_t src, AbstractInstruction* next[2])
		: AbstractInstruction(fi_type_e::fiBranch), src_(src), next_({ next[0], next[1] }) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual void finalize(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex,
		std::vector<AbstractInstruction*>::const_iterator
	);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "cjmp  \tr" << this->src_ << ", " << this->next_[0] << ", " << this->next_[1];
	}

};

class FI_acc_sel : public SequentialInstruction {

	size_t dst_;
	size_t offset_;

public:

	FI_acc_sel(size_t dst, size_t offset)
		: SequentialInstruction(), dst_(dst), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "acc   \t[r" << this->dst_ << " + " << this->offset_ << "]";
	}

};

class FI_acc_set : public SequentialInstruction {

	size_t dst_;
	int base_;
	std::vector<size_t> offsets_;

public:

	FI_acc_set(size_t dst, int base, const std::vector<size_t>& offsets)
		: SequentialInstruction(), dst_(dst), base_(base), offsets_(offsets) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "acc   \t[r" << this->dst_ << " + " << this->base_ << " + " << utils::wrap(this->offsets_) << ']';
	}

};

class FI_acc_all : public SequentialInstruction {

	size_t dst_;

public:

	FI_acc_all(size_t dst)
		: SequentialInstruction(), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "acca  \t[r" << this->dst_ << ']';
	}

};

class FI_load_cst : public SequentialInstruction {

	size_t dst_;
	Data data_;

public:

	FI_load_cst(size_t dst, const Data& data)
		: SequentialInstruction(), dst_(dst), data_(data) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dst_ << ", " << this->data_;
	}

};

class FI_move_reg : public SequentialInstruction {

	size_t dst_;
	size_t src_;

public:

	FI_move_reg(size_t dst, size_t src)
		: SequentialInstruction(), dst_(dst), src_(src) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dst_ << ", r" << this->src_;
	}

};

class FI_bnot : public SequentialInstruction {

	size_t dst_;

public:

	FI_bnot(size_t dst) : SequentialInstruction(), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "not   \tr" << this->dst_;
	}

};

class FI_inot : public SequentialInstruction {

	size_t dst_;

public:

	FI_inot(size_t dst) : SequentialInstruction(), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "not   \tr" << this->dst_;
	}

};

class FI_move_reg_offs : public SequentialInstruction {

	size_t dst_;
	size_t src_;
	int offset_;

public:

	FI_move_reg_offs(size_t dst, size_t src, int offset)
		: SequentialInstruction(), dst_(dst), src_(src), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dst_ << ", r" << this->src_ << " + " << this->offset_;
	}

};

class FI_move_reg_inc : public SequentialInstruction {

	size_t dst_;
	size_t src1_;
	size_t src2_;

public:

	FI_move_reg_inc(size_t dst, size_t src1, size_t src2)
		: SequentialInstruction(), dst_(dst), src1_(src1), src2_(src2) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dst_ << ", r" << this->src1_ << " + r" << this->src2_;
	}

};

class FI_get_greg : public SequentialInstruction {

	size_t dst_;
	size_t src_;

public:

	FI_get_greg(size_t dst, size_t src)
		: SequentialInstruction(), dst_(dst), src_(src) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dst_ << ", gr" << this->src_;
	}

};

class FI_set_greg : public SequentialInstruction {

	size_t dst_;
	size_t src_;

public:

	FI_set_greg(size_t dst, size_t src)
		: SequentialInstruction(), dst_(dst), src_(src) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tgr" << this->dst_ << ", r" << this->src_;
	}

};

class FI_get_ABP : public SequentialInstruction {

	size_t dst_;
	int offset_;

public:

	FI_get_ABP(size_t dst, int offset)
		: SequentialInstruction(), dst_(dst), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dst_ << ", ABP + " << this->offset_;
	}

};
/*
class FI_is_type : public SequentialInstruction {

	size_t dst_;
	data_type_e type_;

public:

	FI_is_type(size_t dst, data_type_e type)
		: SequentialInstruction(NULL), dst_(dst), type_(type) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		if (!(*state.first)[this->dst_].type != this->type_) {

			std::stringstream ss;
			ss << "unexpected data type: " << (*state.first)[this->dst_];
			throw ProgramError(ss.str());

		}

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "istype\tr" << this->dst_ << ", " << this->type_;
	}

};

class FI_is_ref : public SequentialInstruction {

	size_t dst_;

public:

	FI_is_ref(size_t dst)
		: SequentialInstruction(NULL), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		if (!(*state.first)[this->dst_].isRef()) {

			std::stringstream ss;
			ss << "dereferenced value is not a valid reference [" << (*state.first)[this->dst_] << ']';
			throw ProgramError(ss.str());

		}

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "isref\tr" << this->dst_;
	}

};
*/
/*
class FI_inc_off : public SequentialInstruction {

	size_t dst_;
	size_t src_;
	int offset_;

public:

	FI_inc_off(size_t dst, size_t src, int offset)
		: SequentialInstruction(NULL), dst_(dst), src_(src), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		assert((*state.first)[this->src_].isRef());

		(*state.first)[this->dst_] = Data::createRef(
			(*state.first)[this->src_].d_ref.root,
			(*state.first)[this->src_].d_ref.offset + this->offset_
		);

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) {
		return os << "inc\tr" << this->dst_ << ", " << this->offset_;
	}

};
*/
class FI_load : public SequentialInstruction {

	size_t dst_;
	size_t src_;
	int offset_;

public:

	FI_load(size_t dst, size_t src, int offset)
		: SequentialInstruction(), dst_(dst), src_(src), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dst_ << ", [r" << this->src_ << " + " << this->offset_ << ']';
	}

};

class FI_load_ABP : public SequentialInstruction {

	size_t dst_;
	int offset_;

public:

	FI_load_ABP(size_t dst, int offset)
		: SequentialInstruction(), dst_(dst), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dst_ << ", [ABP + " << this->offset_ << ']';
	}

};

class FI_store : public SequentialInstruction {

	size_t dst_;
	size_t src_;
	int offset_;

public:

	FI_store(size_t dst, size_t src, int offset)
		: SequentialInstruction(), dst_(dst), src_(src), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \t[r" << this->dst_ << " + " << this->offset_ << "], r" << this->src_;
	}

};
/*
class FI_store_ABP : public SequentialInstruction {

	size_t src_;
	int offset_;

public:

	FI_store_ABP(size_t src, int offset)
		: SequentialInstruction(NULL), src_(src), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov\t[ABP + " << this->offset_ << "], r" << this->src_;
	}

};
*/
class FI_loads : public SequentialInstruction {

	size_t dst_;
	size_t src_;
	int base_;
	std::vector<size_t> offsets_;

public:

	FI_loads(size_t dst, size_t src, int base, const std::vector<size_t>& offsets)
		: SequentialInstruction(), dst_(dst), src_(src), base_(base), offsets_(offsets) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dst_ << ", [r" << this->src_ << " + " << this->base_ << " + " << utils::wrap(this->offsets_) << ']';
	}

};

class FI_stores : public SequentialInstruction {

	size_t dst_;
	size_t src_;
	int base_;
	std::vector<size_t> offsets_;

public:

	FI_stores(size_t dst, size_t src, int base)
		: SequentialInstruction(), dst_(dst), src_(src), base_(base) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \t[r" << this->dst_ << " + " << this->base_ << "], r" << this->src_;
	}

};

class FI_alloc : public SequentialInstruction {

	size_t dst_;
	size_t src_;

public:

	FI_alloc(size_t dst, size_t src)
		: SequentialInstruction(), dst_(dst), src_(src) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "alloc \tr" << this->dst_ << ", r" << this->src_;
	}

};

class FI_node_create : public SequentialInstruction {

	size_t dst_;
	size_t src_;
	size_t size_;
	const TypeBox* typeInfo_;
	std::vector<SelData> sels_;

public:

	FI_node_create(size_t dst, size_t src, size_t size, const TypeBox* typeInfo, const std::vector<SelData>& sels)
		: SequentialInstruction(), dst_(dst), src_(src), size_(size), typeInfo_(typeInfo), sels_(sels) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "node  \tr" << this->dst_ << ", r" << this->src_;
	}

};
/*
class FI_node_alloc : public SequentialInstruction {

	BoxMan& boxMan_;
	size_t dst_;
	size_t src_;
	const cl_type* type_;

public:

	FI_node_alloc(BoxMan& boxMan, size_t dst, size_t src, const cl_type* type)
		: SequentialInstruction(), boxMan_(boxMan), dst_(dst), src_(src), type_(type) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "alloc \tr" << this->dst_ << ", r" << this->src_;
	}

};
*/
class FI_node_free : public SequentialInstruction {

	size_t dst_;

public:

	FI_node_free(size_t dst)
		: SequentialInstruction(), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "free  \tr" << this->dst_;
	}

};

class FI_iadd : public SequentialInstruction {

	size_t dst_;
	size_t src1_;
	size_t src2_;

public:

	FI_iadd(size_t dst, size_t src1, size_t src2)
		: SequentialInstruction(), dst_(dst), src1_(src1), src2_(src2) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "iadd  \tr" << this->dst_ << ", r" << this->src1_ << ", r" << this->src2_;
	}

};

class FI_check : public SequentialInstruction {

public:

	FI_check() : SequentialInstruction(fi_type_e::fiCheck) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "check ";
	}

};

class FI_assert : public SequentialInstruction {

	size_t dst_;
	Data cst_;

public:

	FI_assert(size_t dst, const Data& cst)
		: SequentialInstruction(), dst_(dst), cst_(cst) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "assert\tr" << this->dst_ << ", " << this->cst_;
	}

};

class FI_abort : public SequentialInstruction {

public:

	FI_abort() : SequentialInstruction(fi_type_e::fiAbort) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "abort ";
	}

};

class FI_build_struct : public SequentialInstruction {

	size_t dst_;
	size_t start_;
	std::vector<size_t> offsets_;

public:

	FI_build_struct(size_t dst, size_t start, const std::vector<size_t>& offsets)
		: SequentialInstruction(), dst_(dst), start_(start), offsets_(offsets) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		os << "mov   \tr" << this->dst_ << ", {";
		for (size_t i = 0; i < this->offsets_.size(); ++i)
			os << " +" << this->offsets_[i] << ":r" << this->start_ + i;
		return os << " }";
	}

};

class FI_push_greg : public SequentialInstruction {

	size_t src_;

public:

	FI_push_greg(size_t src) : SequentialInstruction(), src_(src) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "gpush \tr" << this->src_;
	}

};

class FI_pop_greg : public SequentialInstruction {

	size_t dst_;

public:

	FI_pop_greg(size_t dst) : SequentialInstruction(), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "gpop \tr" << this->dst_;
	}

};

class FI_print_heap : public SequentialInstruction {

	const struct SymCtx* ctx_;

public:

	FI_print_heap(const struct SymCtx* ctx) : SequentialInstruction(), ctx_(ctx) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "prh ";
	}

};

#endif
