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

#ifndef COMPARISON_H
#define COMPARISON_H

// Forester headers
#include "sequentialinstruction.hh"

/**
 * @brief  The base class for comparison instructions
 *
 * According to the result, it enqueues for processing the correct symbolic
 * state. Note that two states may be enqueued in case the precise result is
 * not known.
 */
class FI_cmp_base : public RegisterAssignment
{
	template <class F>
	friend void executeGeneric(
		const FI_cmp_base&        cmp,
		ExecutionManager&         execMan,
		SymState&                 state,
		F                         f);

protected:

	/// Index of the register with the left-hand side operand
	size_t src1_;

	/// Index of the register with the right-hand side operand
	size_t src2_;

public:

	FI_cmp_base(
		const CodeStorage::Insn*          insn,
		size_t                            dst,
		size_t                            src1,
		size_t                            src2) :
		RegisterAssignment(insn, dst),
		src1_(src1),
		src2_(src2)
	{ }
};

/**
 * @brief  Comparison for equality
 *
 * This instruction compares its operands for equality.
 */
class FI_eq : public FI_cmp_base
{
public:

	FI_eq(
		const CodeStorage::Insn*          insn,
		size_t                            dst,
		size_t                            src1,
		size_t                            src2) :
		FI_cmp_base(insn, dst, src1, src2)
	{ }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "eq    \tr" << this->dstReg_ << ", r" << this->src1_ << ", r"
			<< this->src2_;
	}
};

/**
 * @brief  Comparison for inequality
 *
 * This instruction compares its operands for inequality.
 */
class FI_neq : public FI_cmp_base
{
public:

	FI_neq(
		const CodeStorage::Insn*          insn,
		size_t                            dst,
		size_t                            src1,
		size_t                            src2) :
		FI_cmp_base(insn, dst, src1, src2)
	{ }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "neq   \tr" << this->dstReg_ << ", r" << this->src1_ << ", r"
			<< this->src2_;
	}

};

/**
 * @brief  Comparison for less-than
 *
 * This instruction compares its operands for the less-than relation.
 */
class FI_lt : public FI_cmp_base
{
public:

	FI_lt(
		const CodeStorage::Insn*          insn,
		size_t                            dst,
		size_t                            src1,
		size_t                            src2) :
		FI_cmp_base(insn, dst, src1, src2)
	{ }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "lt    \tr" << this->dstReg_ << ", r" << this->src1_ << ", r"
			<< this->src2_;
	}
};

/**
 * @brief  Comparison for greater-than
 *
 * This instruction compares its operands for the greater-than relation.
 */
class FI_gt : public FI_cmp_base
{
public:

	FI_gt(
		const CodeStorage::Insn*          insn,
		size_t                            dst,
		size_t                            src1,
		size_t                            src2) :
		FI_cmp_base(insn, dst, src1, src2)
	{ }

	virtual void execute(ExecutionManager& execMan, SymState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "gt    \tr" << this->dstReg_ << ", r" << this->src1_ << ", r"
			<< this->src2_;
	}
};

#endif
