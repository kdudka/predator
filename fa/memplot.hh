/*
 * Copyright (C) 2012 Ondrej Lengal
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

#ifndef _MEMPLOT_HH_
#define _MEMPLOT_HH_

// Code Listener headers
#include <cl/storage.hh>

class MemPlotter
{
private:

	/**
	 * @brief  Generates plot filename from location info
	 *
	 * Generates plot filename from location info.
	 *
	 * @param[in]  loc  Location info
	 *
	 * @returns  Filename of the plot
	 */
	static std::string generatePlotName(
		const struct cl_loc*              loc);

	/**
	 * @brief  Generates the name of a plot
	 *
	 * Shamelessly copied from Predator.
	 *
	 * @todo
	 */
	static bool readPlotName(
		std::string*                      dst,
		const CodeStorage::TOperandList&  opList,
		const struct cl_loc*              loc);

public:   // methods

	static bool handlePlot(
		const SymState&           state,
		const CodeStorage::Insn&  insn);

	static bool plotHeap(
		const SymState&        state,
		const std::string&     name,
		const struct cl_loc*   loc = nullptr);

	static bool plotHeap(
		const SymState&        state);
};

#endif /* _MEMPLOT_HH_ */
