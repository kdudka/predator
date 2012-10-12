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

// Forester headers
#include "comparison.hh"
#include "executionmanager.hh"
#include "streams.hh"

// anonymous namespace
namespace
{
struct Lt
{
	bool operator()(const Data& x, const Data& y) const
	{
		if (x.isInt() && y.isInt())
			return x.d_int < y.d_int;

		if (x.isBool() && y.isBool())
			return x.d_bool < y.d_bool;

		throw std::runtime_error(
			"Lt()(): comparison of the corresponding types not supported");
	}
};

struct Gt
{
	bool operator()(const Data& x, const Data& y) const
	{
		if (x.isInt() && y.isInt())
			return x.d_int < y.d_int;

		if (x.isBool() && y.isBool())
			return x.d_bool < y.d_bool;

		throw std::runtime_error(
			"Gt()(): comparison of the corresponding types not supported");
	}
};
}

template <class F>
inline void dataCmp(
	std::vector<bool>&              res,
	const Data&                     x,
	const Data&                     y,
	F                               f)
{
	if ((x.isUnknw() || x.isUndef()) || (y.isUnknw() || y.isUndef()))
	{
		if (static_cast<float>(random())/RAND_MAX < 0.5)
		{
			res.push_back(false);
			res.push_back(true);
		} else
		{
			res.push_back(true);
			res.push_back(false);
		}
	} else
	{
		res.push_back(f(x, y));
	}
}

template <class F>
inline void executeGeneric(
	const FI_cmp_base&        cmp,
	ExecutionManager&         execMan,
	SymState&                 state,
	F                         f)
{
	std::vector<bool> res;

	dataCmp(res, state.GetReg(cmp.src1_), state.GetReg(cmp.src2_), f);

	for (auto v : res)
	{
		SymState* tmpState = execMan.createChildStateWithNewRegs(state, cmp.next_);
		tmpState->SetReg(cmp.dstReg_, Data::createBool(v));

		execMan.enqueue(tmpState);
	}
}

void FI_eq::execute(ExecutionManager& execMan, SymState& state)
{
	executeGeneric(*this, execMan, state,
		[](const Data& x, const Data& y){return x == y;});
}

void FI_neq::execute(ExecutionManager& execMan, SymState& state)
{
	executeGeneric(*this, execMan, state,
		[](const Data& x, const Data& y){return x != y;});
}

void FI_lt::execute(ExecutionManager& execMan, SymState& state)
{
	executeGeneric(*this, execMan, state, Lt());
}

void FI_gt::execute(ExecutionManager& execMan, SymState& state)
{
	executeGeneric(*this, execMan, state, Gt());
}
