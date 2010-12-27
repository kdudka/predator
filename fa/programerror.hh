/*
 * Copyright (C) 2010 Jiri Simacek
 *
 * This file is part of predator.
 *
 * predator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * predator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with predator.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROGRAM_ERROR_H
#define PROGRAM_ERROR_H

#include <string>
#include <stdexcept>

#include <cl/location.hh>

class ProgramError : public std::exception {
	std::string msg;
	const cl_location* loc;
public:
	ProgramError(const std::string& msg = "", const cl_location* loc = NULL) : msg(msg), loc(loc) {}
	virtual ~ProgramError() throw() {}
	virtual const char* what() const throw() { return this->msg.c_str(); }
	const cl_location* location() const throw() { return this->loc; }
	
};

#endif
