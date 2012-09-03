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

#ifndef SYM_EXEC_H
#define SYM_EXEC_H

// Standard library headers
#include <unordered_map>

// Forester headers
#include "compiler.hh"
#include "programconfig.hh"

/**
 * @file symexec.hh
 * SymExec - top level algorithm of the @b symbolic @b execution
 */

namespace CodeStorage {
	struct Fnc;
	struct Storage;
}


/**
 * @brief  Top level algorithm of the @b symbolic @b execution
 *
 * This class provides the top level algorithm of the @b symbolic @b execution.
 */
class SymExec
{
private:  // methods

	SymExec(const SymExec&);
	SymExec& operator=(const SymExec&);

public:   // methods

	/**
	 * @brief  Constructor
	 *
	 * Constructor.
	 *
	 * @param[in]  conf  Program configuration
	 */
	explicit SymExec(const ProgramConfig& conf);

	/**
	 * @brief  Destructor
	 *
	 * Destructor.
	 */
	~SymExec();

	/**
	 * @brief  Loads types from given code storage
	 *
	 * Provided a code storage, this method loads types from the storage.
	 *
	 * @param[in]  stor  The code storage with the types
	 */
	void loadTypes(const CodeStorage::Storage& stor);

//	void loadBoxes(const std::unordered_map<std::string, std::string>& db);


	/**
	 * @brief  Returns the compiled code
	 *
	 * Returns the compiled code in the form of microinstructions.
	 *
	 * @returns  The compiled code
	 */
	const Compiler::Assembly& GetAssembly() const;

	/**
	 * @brief  Compiles the code from code storage
	 *
	 * Compiles the code from the code storage into assembly code, starting with
	 * the given entry point. Before being compiled, the data types of the program
	 * need to be loaded first by the method @p loadTypes.
	 *
	 * @param[in]  stor   Code storage with the code
	 * @param[in]  entry  The entry point of the symbolic execution
	 */
	void compile(const CodeStorage::Storage& stor, const CodeStorage::Fnc& entry);

	/**
	 * @brief  Runs the symbolic execution
	 *
	 * This method runs the symbolic execution of the analysed program. Before
	 * being run, the program needs to be compiled into microcode by the method @p
	 * compile.
	 */
	void run();

	/**
	 * @brief  Runs the symbolic execution
	 *
	 * This method runs the symbolic execution of the analysed program. Before
	 * being run, the program needs to be compiled into microcode.
	 */
	void run(const Compiler::Assembly& assembly);

	/**
	 * @brief  Sets the flag for debugging
	 *
	 * The method that sets the debugging flag.
	 */
	void setDbgFlag();

	/**
	 * @brief  Sets the flag for user request
	 *
	 * The method that sets the flag denoting user request.
	 */
	void setUserRequestFlag();

private:

	class Engine;
	Engine *engine;

};

#endif
