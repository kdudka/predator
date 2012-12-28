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

// Standard library headers
#include <vector>
#include <unordered_map>

// Forester headers
#include "abstractinstruction.hh"
#include "sequentialinstruction.hh"
#include "box.hh"


/**
 * @brief  Conditional jump
 *
 * Performs a conditional jump according to the content of a register
 *
 * @note  The value is always concrete, if a case split occurs, it occurs before
 * this instruction is executed.
 *
 */
class FI_cond : public VoidInstruction
{
	/// Index of the register with the Boolean value
	size_t src_;

	/// pointers to @p if and @p else blocks (in this order)
	AbstractInstruction* next_[2];

private:  // methods

	FI_cond(const FI_cond&);
	FI_cond& operator=(const FI_cond&);

public:

	FI_cond(const CodeStorage::Insn* insn, size_t src,
		AbstractInstruction* next[2]) :
		VoidInstruction(insn, fi_type_e::fiBranch),
		src_{src},
		next_{next[0], next[1]}
	{ }

	FI_cond(const CodeStorage::Insn* insn, size_t src,
		const std::vector<AbstractInstruction*>& next) :
		VoidInstruction(insn, fi_type_e::fiBranch),
		src_{src},
		next_{next[0], next[1]}
	{ }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual void finalize(const std::unordered_map<const CodeStorage::Block*,
			AbstractInstruction*>& codeIndex,
		std::vector<AbstractInstruction*>::const_iterator);

	virtual std::ostream& toStream(std::ostream& os) const
	{
		return os << "cjmp  \tr" << src_ << ", " << next_[0] << ", " << next_[1];
	}
};


/**
 * @brief  Isolates a single selector
 *
 * Isolates a single root selector of a tree automaton.
 */
class FI_acc_sel : public SequentialInstruction
{
	/// register holding the reference to the tree automaton
	size_t dst_;

	/// offset of the selector
	size_t offset_;

public:

	FI_acc_sel(const CodeStorage::Insn* insn, size_t dst, size_t offset)
		: SequentialInstruction(insn, fi_type_e::fiUnspec),
		dst_(dst), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual SymState* reverseAndIsect(
		ExecutionManager&                      execMan,
		const SymState&                        fwdPred,
		const SymState&                        bwdSucc) const;

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "acc   \t[r" << this->dst_ << " + " << this->offset_ << "]";
	}

};

/**
 * @brief  Isolates a set of selectors
 *
 * Isolates a set of root selectors of a tree automaton.
 */
class FI_acc_set : public SequentialInstruction
{
	/// register holding the reference to the tree automaton
	size_t dst_;

	/// base value for the @p offsets_
	int base_;

	/// offsets of the selectors
	std::vector<size_t> offsets_;

public:

	FI_acc_set(const CodeStorage::Insn* insn, size_t dst, int base,
		const std::vector<size_t>& offsets)
		: SequentialInstruction(insn), dst_(dst), base_(base), offsets_(offsets) {}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual SymState* reverseAndIsect(
		ExecutionManager&                      execMan,
		const SymState&                        fwdPred,
		const SymState&                        bwdSucc) const;

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "acc   \t[r" << this->dst_ << " + " << this->base_
			<< " + " << utils::wrap(this->offsets_) << ']';
	}

};

/**
 * @brief  Isolates all selectors
 *
 * Isolates all root selectors of a tree automaton.
 */
class FI_acc_all : public SequentialInstruction
{
	/// register holding the reference to the tree automaton
	size_t dst_;

public:

	FI_acc_all(const CodeStorage::Insn* insn, size_t dst)
		: SequentialInstruction(insn), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual SymState* reverseAndIsect(
		ExecutionManager&                      execMan,
		const SymState&                        fwdPred,
		const SymState&                        bwdSucc) const;

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "acca  \t[r" << this->dst_ << ']';
	}

};

/**
 * @brief  Loads a constant into a register
 */
class FI_load_cst : public RegisterAssignment
{
	/// The data value to be loaded into the register denoted by @p dst_
	Data data_;

public:

	FI_load_cst(const CodeStorage::Insn* insn, size_t dst, const Data& data)
		: RegisterAssignment(insn, dst), data_(data) {}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dstReg_ << ", " << this->data_;
	}

};

/**
 * @brief  Moves a value between two registers
 */
class FI_move_reg : public RegisterAssignment
{
	/// Index of the source register
	size_t src_;

public:

	FI_move_reg(const CodeStorage::Insn* insn, size_t dst, size_t src)
		: RegisterAssignment(insn, dst), src_(src)
	{
		// Check that we don't make a useless move
		assert(src_ != dstReg_);
	}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dstReg_ << ", r" << this->src_;
	}

};

/**
 * @brief  Negates a Boolean value in a register
 */
class FI_bnot : public RegisterAssignment
{
public:

	FI_bnot(const CodeStorage::Insn* insn, size_t dst)
		: RegisterAssignment(insn, dst) { }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "not   \tr" << this->dstReg_;
	}

};

/**
 * @brief  Negates an integer value in a register
 *
 * Negates an integer value in a register: the result is a Boolean.
 */
class FI_inot : public RegisterAssignment
{
public:

	FI_inot(const CodeStorage::Insn* insn, size_t dst) :
		RegisterAssignment(insn, dst) { }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "not   \tr" << this->dstReg_;
	}

};

/**
 * @brief  Moves a reference between register with an immediate offset
 *
 * Moves a reference to a tree automaton from one register into another
 * register. Before storing the reference into the target register, the
 * displacement is incremented by a specified offset.
 */
class FI_move_reg_offs : public RegisterAssignment
{
	/// Index of the source register
	size_t src_;

	/// The offset by which the displacement will be modified
	int offset_;

public:

	FI_move_reg_offs(const CodeStorage::Insn* insn,
		size_t dst, size_t src, int offset)
		: RegisterAssignment(insn, dst), src_(src), offset_(offset)
	{ }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dstReg_ << ", r" << this->src_
			<< " + " << this->offset_;
	}

};

/**
 * @brief  Moves a reference between register with an offset in a register
 *
 * Moves a reference to a tree automaton from one register into another
 * register. Before storing the reference into the target register, the
 * displacement is incremented by the value in the specified register.
 */
class FI_move_reg_inc : public RegisterAssignment
{
	/// Index of the source register
	size_t src1_;

	/// Index of the register with the offset value
	size_t src2_;

public:

	FI_move_reg_inc(const CodeStorage::Insn* insn,
		size_t dst, size_t src1, size_t src2)
		: RegisterAssignment(insn, dst), src1_(src1), src2_(src2)
	{ }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dstReg_ << ", r" << this->src1_
			<< " + r" << this->src2_;
	}

};

/**
 * @brief  Loads a global register into a local register
 *
 * Loads the value from a global register into a local register.
 */
class FI_get_greg : public RegisterAssignment
{
	/// Index of the source global register
	size_t src_;

public:

	FI_get_greg(const CodeStorage::Insn* insn, size_t dst, size_t src)
		: RegisterAssignment(insn, dst), src_(src) { }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dstReg_ << ", gr" << this->src_;
	}

};

/**
 * @brief  Loads a local register into a global register
 *
 * Loads the value from a local register into a global register
 */
class FI_set_greg : public SequentialInstruction
{
	/// Index of the target global register
	size_t dst_;

	/// Index of the source local register
	size_t src_;

public:

	FI_set_greg(const CodeStorage::Insn* insn, size_t dst, size_t src)
		: SequentialInstruction(insn), dst_(dst), src_(src) {}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual SymState* reverseAndIsect(
		ExecutionManager&                      execMan,
		const SymState&                        fwdPred,
		const SymState&                        bwdSucc) const;

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tgr" << this->dst_ << ", r" << this->src_;
	}

};

/**
 * @brief  Loads the ABP pointer into a register
 *
 * Loads the ABP pointer incremented by the specified offset into a register.
 */
class FI_get_ABP : public RegisterAssignment
{
	/// Offset to be added to the loaded pointer
	int offset_;

public:

	FI_get_ABP(const CodeStorage::Insn* insn, size_t dst, int offset)
		: RegisterAssignment(insn, dst), offset_(offset) { }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dstReg_ << ", ABP + " << this->offset_;
	}

};

/**
 * @brief  Loads the GLOB pointer into a register
 *
 * Loads the GLOB pointer incremented by the specified offset into a register.
 */
class FI_get_GLOB : public RegisterAssignment
{
	/// Offset to be added to the loaded pointer
	int offset_;

public:

	FI_get_GLOB(const CodeStorage::Insn* insn, size_t dst, int offset)
		: RegisterAssignment(insn, dst),  offset_(offset) { }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dstReg_ << ", GLOB + " << this->offset_;
	}

};

/**
 * @brief  Loads a value pointed by a register into another register
 *
 * Loads a value at a given @p offset_ from the location pointed by the @p src_
 * register into the @p dst_ register.
 */
class FI_load : public RegisterAssignment
{
	/// Index of the source register
	size_t src_;

	/// Offset of the requested value
	int offset_;

public:

	FI_load(const CodeStorage::Insn* insn, size_t dst, size_t src, int offset)
		: RegisterAssignment(insn, dst), src_(src), offset_(offset)
	{ }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dstReg_ << ", [r" << this->src_
			<< " + " << this->offset_ << ']';
	}

};

/**
 * @brief  Loads a value pointed by the ABP pointer
 *
 * Loads a value which is at the specified offset from the location pointed by
 * the ABP pointer into a register.
 */
class FI_load_ABP : public RegisterAssignment
{
	/// Offset from the ABP 
	int offset_;

public:

	FI_load_ABP(const CodeStorage::Insn* insn, size_t dst, int offset)
		: RegisterAssignment(insn, dst), offset_(offset) { }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dstReg_ << ", [ABP + " << this->offset_ << ']';
	}

};

/**
 * @brief  Loads a value pointed by the GLOB pointer
 *
 * Loads a value which is at the specified offset from the location pointed by
 * the GLOB pointer into a register.
 */
class FI_load_GLOB : public RegisterAssignment
{
	/// Offset from the GLOB
	int offset_;

public:

	FI_load_GLOB(const CodeStorage::Insn* insn, size_t dst, int offset)
		: RegisterAssignment(insn, dst), offset_(offset) { }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dstReg_ << ", [GLOB + " << this->offset_ << ']';
	}

};

/**
 * @brief  Stores a value from a register into forest automaton
 *
 * Stores a value located in the @p src_ register into a memory location at
 * a location displaced by @p offset_ from the location pointed by register @p
 * dst_.
 */
class FI_store : public SequentialInstruction
{
	/// Index of the target register
	size_t dst_;

	/// Index of the source register
	size_t src_;

	/// Offset from the location pointed by @p src_
	int offset_;

public:

	FI_store(const CodeStorage::Insn* insn, size_t dst, size_t src, int offset)
		: SequentialInstruction(insn), dst_(dst), src_(src), offset_(offset)
	{
		// Assertions
		assert(src_ != dst_);
	}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual SymState* reverseAndIsect(
		ExecutionManager&                      execMan,
		const SymState&                        fwdPred,
		const SymState&                        bwdSucc) const;

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \t[r" << this->dst_ << " + " << this->offset_
			<< "], r" << this->src_;
	}

};

/**
 * @brief  Loads a structure pointed by a register into another register
 *
 * Loads a structure with multiple offsets pointed by the @p src_ register into
 * the @p dst_ register.
 */
class FI_loads : public RegisterAssignment
{
	/// Index of the source register
	size_t src_;

	/// Base for the offsets
	int base_;

	/// Offsets for selectors in the structure
	std::vector<size_t> offsets_;

public:

	FI_loads(const CodeStorage::Insn* insn, size_t dst, size_t src, int base,
		const std::vector<size_t>& offsets) :
		RegisterAssignment(insn, dst), src_(src), base_(base),
		offsets_(offsets)
	{ }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \tr" << this->dstReg_ << ", [r" << this->src_ << " + "
			<< this->base_ << " + " << utils::wrap(this->offsets_) << ']';
	}

};

/**
 * @brief  Stores a structure in a register into the forest automaton
 *
 * Stores a structure with multiple offsets (from the @p base_) located in the
 * @p src_ register into the memory location pointed to by the @p dst_ register.
 */
class FI_stores : public SequentialInstruction
{
	/// Index of the target register
	size_t dst_;

	/// Index of the source register
	size_t src_;

	/// Base for the offsets
	int base_;

public:

	FI_stores(const CodeStorage::Insn* insn, size_t dst, size_t src, int base)
		: SequentialInstruction(insn), dst_(dst), src_(src), base_(base)
	{
		// Assertions
		assert(src_ != dst_);
	}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual SymState* reverseAndIsect(
		ExecutionManager&                      execMan,
		const SymState&                        fwdPred,
		const SymState&                        bwdSucc) const;

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov   \t[r" << this->dst_ << " + " << this->base_
			<< "], r" << this->src_;
	}

};

/**
 * @brief  Allocates a memory block of given size
 *
 * Allocates a memory block of given size and stores a reference to it in
 * a register.
 */
class FI_alloc : public RegisterAssignment
{
	/// Index of the register with the size of the allocated block
	size_t src_;

public:

	FI_alloc(const CodeStorage::Insn* insn, size_t dst, size_t src)
		: RegisterAssignment(insn, dst), src_(src)
	{ }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "alloc \tr" << this->dstReg_ << ", r" << this->src_;
	}

};

/**
 * @brief  Builds a new memory node
 *
 * Creates a new tree automaton in the forest automaton. This new single-level
 * tree automaton is represents the created memory node.
 */
class FI_node_create : public SequentialInstruction
{
	/// Index of the register holding a reference to the created tree automaton
	size_t dst_;

	/// Index of the register pointing to a memory block
	size_t src_;

	/// Size of the desired data structure
	size_t size_;

	/// Type of the desired data structure
	const TypeBox* typeInfo_;

	/// Selectors of the desired data structure
	std::vector<SelData> sels_;

private:  // methods

	FI_node_create(const FI_node_create&);
	FI_node_create& operator=(const FI_node_create&);

public:

	FI_node_create(
		const CodeStorage::Insn*        insn,
		size_t                          dst,
		size_t                          src,
		size_t                          size,
		const TypeBox*                  typeInfo,
		const std::vector<SelData>&     sels
	) :
		SequentialInstruction(insn),
		dst_(dst),
		src_(src),
		size_(size),
		typeInfo_(typeInfo),
		sels_(sels)
	{ }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual SymState* reverseAndIsect(
		ExecutionManager&                      execMan,
		const SymState&                        fwdPred,
		const SymState&                        bwdSucc) const;

	virtual std::ostream& toStream(std::ostream& os) const
	{
		// Assertions
		assert(nullptr != typeInfo_);

		os << "node  \tr" << this->dst_ << ", r" << this->src_ << ", ("
			<< typeInfo_->getName() << "), <";

		for (auto it = sels_.cbegin(); it != sels_.cend(); ++it)
		{
			if (it != sels_.cbegin())
				os << ", ";

			os << *it;
		}

		return os << ">";
	}
};

/**
 * @brief  Frees a memory node
 *
 * Frees a memory node and invalidates all references pointing to the node.
 */
class FI_node_free : public SequentialInstruction
{
	/// Index of the register holding the reference to the freed node
	size_t dst_;

public:

	FI_node_free(const CodeStorage::Insn* insn, size_t dst)
		: SequentialInstruction(insn), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual SymState* reverseAndIsect(
		ExecutionManager&                      execMan,
		const SymState&                        fwdPred,
		const SymState&                        bwdSucc) const;

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "free  \tr" << this->dst_;
	}

};

/**
 * @brief  Computes integer addition
 */
class FI_iadd : public RegisterAssignment
{
	/// Index of the register with the first operand
	size_t src1_;

	/// Index of the register with the other operand
	size_t src2_;

public:

	FI_iadd(const CodeStorage::Insn* insn, size_t dst, size_t src1, size_t src2)
		: RegisterAssignment(insn, dst), src1_(src1), src2_(src2)
	{ }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "iadd  \tr" << this->dstReg_ << ", r" << this->src1_
			<< ", r" << this->src2_;
	}

};

/**
 * @brief  Checks for the absence of garbage
 */
class FI_check : public VoidInstruction
{
public:

	FI_check(const CodeStorage::Insn* insn)
		: VoidInstruction(insn, fi_type_e::fiCheck) {}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "check ";
	}

};

/**
 * @brief  Assertion on the value of a register
 *
 * Checks whether the value of a register is equal to the desired value.
 */
class FI_assert : public VoidInstruction
{
	/// Index of the source register
	size_t dst_;

	/// The data value that the @p dst_ register should hold
	Data cst_;

public:

	FI_assert(const CodeStorage::Insn* insn, size_t dst, const Data& cst)
		: VoidInstruction(insn), dst_(dst), cst_(cst) {}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "assert\tr" << this->dst_ << ", " << this->cst_;
	}

};

/**
 * @brief  Aborts the program execution (exit)
 */
class FI_abort : public SequentialInstruction
{
public:

	FI_abort(const CodeStorage::Insn* insn)
		: SequentialInstruction(insn, fi_type_e::fiAbort) {}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual SymState* reverseAndIsect(
		ExecutionManager&                      execMan,
		const SymState&                        fwdPred,
		const SymState&                        bwdSucc) const;

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "abort ";
	}

	/**
	 * @copydoc AbstractInstruction::finalize
	 */
	virtual void finalize(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>&,
		std::vector<AbstractInstruction*>::const_iterator
	) {}

};

/**
 * @brief  Builds a structure from registers
 *
 * Builds a memory structure (e.g. a stack frame) from registers' content
 * (starting from the @p start_ register.
 */
class FI_build_struct : public RegisterAssignment
{
	/// Index of the starting register
	size_t start_;

	/// List of offsets
	std::vector<size_t> offsets_;

public:

	FI_build_struct(const CodeStorage::Insn* insn, size_t dst, size_t start,
		const std::vector<size_t>& offsets) :
		RegisterAssignment(insn, dst), start_(start), offsets_(offsets) { }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		os << "mov   \tr" << this->dstReg_ << ", {";
		for (size_t i = 0; i < this->offsets_.size(); ++i) {
			os << " +" << this->offsets_[i] << ":r" << this->start_ + i;
		}

		return os << " }";
	}

};

/**
 * @brief  Allocate and load a new global register
 *
 * Allocates new global register and fills it with the content of the specified
 * register.
 */
class FI_push_greg : public SequentialInstruction
{
	/// Index of the source register
	size_t src_;

public:

	FI_push_greg(const CodeStorage::Insn* insn, size_t src)
		: SequentialInstruction(insn), src_(src) {}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual SymState* reverseAndIsect(
		ExecutionManager&                      execMan,
		const SymState&                        fwdPred,
		const SymState&                        bwdSucc) const;

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "gpush \tr" << this->src_;
	}

};

/**
 * @brief  Loads the value of the latest global register
 *
 * Loads the value of the latest global register into a local register and
 * deletes the global register.
 */
class FI_pop_greg : public SequentialInstruction
{
	/// Index of the target local register
	size_t dst_;

public:

	FI_pop_greg(const CodeStorage::Insn* insn, size_t dst)
		: SequentialInstruction(insn), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual SymState* reverseAndIsect(
		ExecutionManager&                      execMan,
		const SymState&                        fwdPred,
		const SymState&                        bwdSucc) const;

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "gpop  \tr" << this->dst_;
	}

};

/**
 * @brief  Prints the heap
 *
 * Prints the heap in a human readable format (ehm?)
 */
class FI_print_heap : public VoidInstruction
{
	/// The context to be printed
	const struct SymCtx* ctx_;

private:  // methods

	FI_print_heap(const FI_print_heap&);
	FI_print_heap& operator=(const FI_print_heap&);

public:

	FI_print_heap(const CodeStorage::Insn* insn, const struct SymCtx* ctx)
		: VoidInstruction(insn), ctx_(ctx) {}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "prh ";
	}

};

/**
 * @brief  Plots the heap
 *
 * Plots the heap.
 */
class FI_plot_heap : public VoidInstruction
{
private:  // methods

	FI_plot_heap(const FI_plot_heap&);
	FI_plot_heap& operator=(const FI_plot_heap&);

public:

	FI_plot_heap(const CodeStorage::Insn* insn)
		: VoidInstruction(insn) {}

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "plot ";
	}
};

/**
 * @brief  Error instruction
 *
 * Represents an error location.
 */
class FI_error : public VoidInstruction
{
private:  // data members

	/// the error message
	std::string msg_;

public:

	FI_error(const CodeStorage::Insn* insn, const std::string msg) :
		VoidInstruction(insn),
		msg_(msg)
	{ }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const
	{
		return os << "error";
	}

};
#endif
