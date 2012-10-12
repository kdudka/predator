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

#ifndef _BACKWARD_RUN_HH_
#define _BACKWARD_RUN_HH_

// Forester headers
#include "executionmanager.hh"
#include "symstate.hh"

class BackwardRun
{
private:  // data members

	/// The execution manager
	ExecutionManager& execMan_;

private:  // methods

	BackwardRun();

public:   // methods

	explicit BackwardRun(ExecutionManager& execMan) :
		execMan_(execMan)
	{ }

	/**
	 * @brief  Test for spuriousness of a counterexample trace
	 *
	 * @param[in]  fwdTrace  The forward trace
	 *
	 * @returns  @p true if the counterexample is spurious, @p false if otherwise
	 */
	bool isSpuriousCE(SymState::Trace& fwdTrace);
};


#endif
