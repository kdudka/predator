/*
 * Copyright (C) 2012  Ondrej Lengal
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
#include "exec_state.hh"
#include "symstate.hh"

std::ostream& operator<<(std::ostream& os, const ExecState& state)
{
	os << "registers:";

	for (size_t i = 0; i < state.GetRegs().size(); ++i) {

		os << " r" << i << '=' << state.GetReg(i);

	}

	os << ", heap:" << std::endl << *state.GetMem()->GetFAE();

	return os << "instruction (" << state.GetMem()->GetInstr() << "): "
		<< *state.GetMem()->GetInstr();
}
