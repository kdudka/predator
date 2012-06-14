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

#include <string>
#include <stdexcept>

#include <cl/code_listener.h>

/**
 * @file programerror.hh
 * ProgramError class declaration (and definition)
 */


/**
 * @brief  An exception class for program analysis
 *
 * This exception class is used for program analysis using the Code Listener
 * interface. It contains an error message and a Code Listener provided
 * location.
 */
class ProgramError : public std::exception {

	/// Error message
	std::string msg;

	/// Code Listener location to provide further information about the error
	const cl_loc* loc;

public:

	/**
	 * @brief  Constructor
	 *
	 * Constructs and assigns value to a new object.
	 *
	 * @param[in]  msg  The error message
	 * @param[in]  loc  The location in the program that caused the error
	 */
	ProgramError(const std::string& msg = "", const cl_loc* loc = nullptr) :
		msg(msg), loc(loc) {}

	/**
	 * @brief  Copy constructor
	 */
	ProgramError(const ProgramError& err) :
		msg{err.msg}, loc{err.loc}
	{ }

	/**
	 * @brief  Assignment operator
	 */
	ProgramError& operator=(const ProgramError& err)
	{
		if (&err != this)
		{
			msg = err.msg;
			loc = err.loc;
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
	virtual const char* what() const throw() { return this->msg.c_str(); }

	/**
	 * @brief  Location of the error
	 *
	 * Retrieves the location of the error in the analyzed program.
	 *
	 * @returns  The location of the error
	 */
	const cl_loc* location() const throw() { return this->loc; }
};

#endif
