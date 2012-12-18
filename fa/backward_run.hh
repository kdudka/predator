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
	 * Performs the backward run, trying to validate the spuriousness of the
	 * forward trace @p fwdTrace, i.e. whether the trace really appears in the
	 * real program (unabstracted).
	 *
	 * @param[in]  fwdTrace   The forward trace
	 * @param[out] failPoint  The point in the forward trace where the backward
	 *                        run failed (the intersection with the corresponding
	 *                        state in @p fwdTrace was empty), for the case when
	 *                        the forward trace is spurious.
	 * @param[out] predicate  A forest automaton which was extracted as a good
	 *                        predicate to be used in predicate abstraction, for
	 *                        the case when the forward trace is spurious
	 *
	 * @returns  @p true if the counterexample is spurious, @p false if otherwise
	 */
	bool isSpuriousCE(
		const SymState::Trace&              fwdTrace,
		SymState*&                          failPoint,
		std::shared_ptr<const FAE>&         predicate);
};


#endif
