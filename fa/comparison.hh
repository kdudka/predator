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

#include "sequentialinstruction.hh"

class FI_cmp_base : public SequentialInstruction {

	template <class F>
	friend void executeGeneric(const FI_cmp_base& cmp, ExecutionManager& execMan,
		const ExecState& state, F f);

protected:

	size_t dst_;
	size_t src1_;
	size_t src2_;

public:

	FI_cmp_base(size_t dst, size_t src1, size_t src2)
		: SequentialInstruction(), dst_(dst), src1_(src1), src2_(src2) {}

};

class FI_eq : public FI_cmp_base {

public:

	FI_eq(size_t dst, size_t src1, size_t src2) : FI_cmp_base(dst, src1, src2) {}

	virtual void execute(ExecutionManager& execMan, const ExecState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "eq    \tr" << this->dst_ << ", r" << this->src1_ << ", r"
			<< this->src2_;
	}

};

class FI_neq : public FI_cmp_base {

public:

	FI_neq(size_t dst, size_t src1, size_t src2) : FI_cmp_base(dst, src1, src2) {}

	virtual void execute(ExecutionManager& execMan, const ExecState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "neq   \tr" << this->dst_ << ", r" << this->src1_ << ", r"
			<< this->src2_;
	}

};

class FI_lt : public FI_cmp_base {

public:

	FI_lt(size_t dst, size_t src1, size_t src2) : FI_cmp_base(dst, src1, src2) {}

	virtual void execute(ExecutionManager& execMan, const ExecState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "lt    \tr" << this->dst_ << ", r" << this->src1_ << ", r"
			<< this->src2_;
	}

};

class FI_gt : public FI_cmp_base {

public:

	FI_gt(size_t dst, size_t src1, size_t src2) : FI_cmp_base(dst, src1, src2) {}

	virtual void execute(ExecutionManager& execMan, const ExecState& state);

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "gt    \tr" << this->dst_ << ", r" << this->src1_ << ", r"
			<< this->src2_;
	}

};
#endif
