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

#ifndef NOTIMPL_EXCEPT_H
#define NOTIMPL_EXCEPT_H

#include <string>
#include <stdexcept>

#include <cl/code_listener.h>

/**
 * @file notimpl_except.hh
 * NotImplementedException class declaration (and definition)
 */


/**
 * @brief  An exception class for unimplemented features
 *
 * This exception class is used for reporting unimplemented source code
 * features. It contains an error message and a Code Listener provided location
 * of the feature in the source code.
 */
class NotImplementedException : public std::exception {

	/// Error message
	std::string msg;

	/// Code Listener location to provide further information about the feature
	const cl_loc* loc;

public:

	/**
	 * @brief  Constructor
	 *
	 * Constructs and assigns value to a new object.
	 *
	 * @param[in]  msg  The error message
	 * @param[in]  loc  The location in the program with the feature
	 */
	NotImplementedException(const std::string& msg = "", const cl_loc* loc = nullptr) :
		msg(msg), loc(loc) {}

	/**
	 * @brief  Copy constructor
	 */
	NotImplementedException(const NotImplementedException& ex) :
		msg{ex.msg}, loc{ex.loc}
	{ }

	/**
	 * @brief  Assignment operator
	 */
	NotImplementedException& operator=(const NotImplementedException& ex)
	{
		if (this != &ex)
		{
			msg = ex.msg;
			loc = ex.loc;
		}

		return *this;
	}

	/**
	 * @brief  Destructor
	 *
	 * Virtual destructor.
	 */
	virtual ~NotImplementedException() throw() {}

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
	 * Retrieves the location of the unimplemented feature in the analyzed
	 * program.
	 *
	 * @returns  The location of the error
	 */
	const cl_loc* location() const throw() { return this->loc; }
};

#endif
