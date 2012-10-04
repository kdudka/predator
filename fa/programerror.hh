/*
 * Copyright (C) 2010 Jiri Simacek
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

#ifndef PROGRAM_ERROR_H
#define PROGRAM_ERROR_H

// Standard library headers
#include <string>
#include <stdexcept>

// Code Listener headers
#include <cl/code_listener.h>

// Forester Headers
//#include "symstate.hh"


/**
 * @file programerror.hh
 * ProgramError class declaration (and definition)
 */

class SymState;

/**
 * @brief  An exception class for program analysis
 *
 * This exception class is used for program analysis using the Code Listener
 * interface. It contains an error message and a Code Listener provided
 * location.
 */
class ProgramError : public std::exception
{
	/// Error message
	std::string msg_;

	/// Code Listener location to provide further information about the error
	const cl_loc* loc_;

	/// The state in which the error appeared
	const SymState* state_;

public:

	/**
	 * @brief  Constructor
	 *
	 * Constructs and assigns value to a new object.
	 *
	 * @param[in]  msg    The error message
	 * @param[in]  state  State in which the error appeared
	 * @param[in]  loc    The location in the program that caused the error
	 */
	ProgramError(
		const std::string&  msg = "",
		const SymState*     state = nullptr,
		const cl_loc*       loc = nullptr) :
		msg_(msg),
		loc_(loc),
		state_(state)
	{ }

	/**
	 * @brief  Copy constructor
	 */
	ProgramError(const ProgramError& err) :
		msg_{err.msg_}, loc_{err.loc_}, state_{err.state_}
	{ }

	/**
	 * @brief  Assignment operator
	 */
	ProgramError& operator=(const ProgramError& err)
	{
		if (&err != this)
		{
			msg_   = err.msg_;
			loc_   = err.loc_;
			state_ = err.state_;
		}

		return *this;
	}

	/**
	 * @brief  Destructor
	 *
	 * Virtual destructor.
	 */
	virtual ~ProgramError() throw() {}

	/**
	 * @brief  Description of error
	 *
	 * Retrieves the description of the error (overrides the
	 * std::exception::what() method)
	 *
	 * @returns  The description of the error
	 */
	virtual const char* what() const throw() { return msg_.c_str(); }

	/**
	 * @brief  Location of the error
	 *
	 * Retrieves the location of the error in the analyzed program.
	 *
	 * @returns  The location of the error
	 */
	const cl_loc* location() const throw() { return loc_; }

	/**
	 * @brief  State in which the error occured
	 *
	 * Retrieves the symbolic state in which the error was detected.
	 *
	 * @returns  The state in which the error occured
	 */
	const SymState* state() const throw() { return state_; }
};

#endif
