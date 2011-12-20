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

#ifndef RESTART_REQUEST_H
#define RESTART_REQUEST_H

#include <string>
#include <stdexcept>

/**
 * @file restart_request.hh
 * RestartRequest class declaration (and definition)
 */


/**
 * @brief  An exception class for verification restart
 *
 * This exception class is used to indicate that verification restart is required.
 */
class RestartRequest : public std::exception {

	/// Error message
	std::string msg;

public:

	/**
	 * @brief  Constructor
	 *
	 * Constructs and assigns value to a new object.
	 *
	 * @param[in]  reason  The reason for restart
	 */
	RestartRequest(const std::string& reason = "") :
		msg("a restart is requested" + ((reason == "")?("."):(" (" + reason + ")."))) {}

	/**
	 * @brief  Destructor
	 *
	 * Virtual destructor.
	 */
	virtual ~RestartRequest() throw() {}

	/**
	 * @brief  Description of
	 *
	 * Retrieves the description of the request (overrides the
	 * std::exception::what() method)
	 *
	 * @returns  The description of the request
	 */
	virtual const char* what() const throw() { return this->msg.c_str(); }

};

#endif
