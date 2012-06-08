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

#include "executionmanager.hh"

#include "comparison.hh"

struct Eq {
	bool operator()(const Data& x, const Data& y) const {
		return x == y;
	}
};

struct Neq {
	bool operator()(const Data& x, const Data& y) const {
		return x != y;
	}
};

struct Lt {
	bool operator()(const Data& x, const Data& y) const {
		if (x.isInt() && y.isInt())
			return x.d_int < y.d_int;
		if (x.isBool() && y.isBool())
			return x.d_bool < y.d_bool;
		throw std::runtime_error("Lt()(): comparison of the corresponding types not supported");
	}
};

struct Gt {
	bool operator()(const Data& x, const Data& y) const {
		if (x.isInt() && y.isInt())
			return x.d_int < y.d_int;
		if (x.isBool() && y.isBool())
			return x.d_bool < y.d_bool;
		throw std::runtime_error("Gt()(): comparison of the corresponding types not supported");
	}
};

template <class F>
inline void dataCmp(std::vector<bool>& res, const Data& x, const Data& y, F f) {

	if ((x.isUnknw() || x.isUndef()) || (y.isUnknw() || y.isUndef())) {

		if (static_cast<float>(random())/RAND_MAX < 0.5) {
			res.push_back(false);
			res.push_back(true);
		} else {
			res.push_back(true);
			res.push_back(false);
		}

	} else {

		res.push_back(f(x, y));

	}

}

template <class F>
inline void executeGeneric(const FI_cmp_base& cmp, ExecutionManager& execMan, const ExecState& state, F f) {

	std::vector<bool> res;

	dataCmp(res, state.GetReg(cmp.src1_), state.GetReg(cmp.src2_), f);

	for (auto v : res) {

		std::shared_ptr<DataArray> regs = execMan.allocRegisters(state.GetRegs());

		(*regs)[cmp.dst_] = Data::createBool(v);

		execMan.enqueue(state.GetMem(), regs, state.GetMem()->GetFAE(), cmp.next_);

	}

}

void FI_eq::execute(ExecutionManager& execMan, const ExecState& state) {

	executeGeneric(*this, execMan, state, Eq());

}

void FI_neq::execute(ExecutionManager& execMan, const ExecState& state) {

	executeGeneric(*this, execMan, state, Neq());

}

void FI_lt::execute(ExecutionManager& execMan, const ExecState& state) {

	executeGeneric(*this, execMan, state, Lt());

}

void FI_gt::execute(ExecutionManager& execMan, const ExecState& state) {

	executeGeneric(*this, execMan, state, Gt());

}
