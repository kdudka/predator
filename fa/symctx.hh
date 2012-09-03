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

#ifndef SYM_CTX_H
#define SYM_CTX_H

// Standard library headers
#include <vector>
#include <unordered_map>

// Code Listener headers
#include <cl/storage.hh>
#include <cl/clutil.hh>

// Forester headers
#include "nodebuilder.hh"
#include "notimpl_except.hh"
#include "streams.hh"
#include "types.hh"
#include "varinfo.hh"

#define ABP_OFFSET		0
#define ABP_SIZE		SymCtx::getSizeOfDataPtr()
#define RET_OFFSET		(ABP_OFFSET + ABP_SIZE)
#define RET_SIZE		SymCtx::getSizeOfCodePtr()

#define GLOBAL_VARS_BLOCK_STR   "__global_vars_block"

/**
 * @brief  Symbolic context of a function
 *
 * This class represents a symbolic context of a function.
 *
 * @todo
 */
class SymCtx
{
public:   // data types

	/// Stack frame layout
	typedef std::vector<SelData> StackFrameLayout;


	/**
	 * @brief  The type that maps identifiers of variables to @p VarInfo
	 *
	 * This type serves as a map between identifiers of variables and @p VarInfo
	 * structures.
	 */
	typedef std::unordered_map<int, VarInfo> var_map_type;


private:  // static data

	/// @todo is @p static really the best option?

	// must be initialised externally!

	/// The size of a data pointer in the analysed program
	static size_t size_of_data_ptr;

	/// The size of a code pointer in the analysed program
	static size_t size_of_code_ptr;


public:   // static methods

	/**
	 * @brief  Initialise the symbolic context
	 *
	 * This static method needs to be called before the @p SymCtx class is
	 * used for the first time. It properly initialises static members of the
	 * class from the passed @p CodeStorage.
	 *
	 * @param[in]  stor  The @p CodeStorage from which the context is to be
	 *                   initialised
	 */
	static void initCtx(const CodeStorage::Storage& stor)
	{
		if (stor.types.codePtrSizeof() >= 0)
		{
			size_of_code_ptr = static_cast<size_t>(stor.types.codePtrSizeof());
		}
		else
		{
			size_of_code_ptr = sizeof(void(*)());
		}

		if (stor.types.dataPtrSizeof() >= 0)
		{
			size_of_data_ptr = static_cast<size_t>(stor.types.dataPtrSizeof());
		}
		else
		{
			size_of_data_ptr = sizeof(void*);
		}

		// Post-condition
		assert(size_of_data_ptr > 0);
		assert(size_of_code_ptr > 0);
	}

	static size_t getSizeOfCodePtr()
	{
		// Assertions
		assert(size_of_code_ptr > 0);

		return size_of_code_ptr;
	}

	static size_t getSizeOfDataPtr()
	{
		// Assertions
		assert(size_of_data_ptr > 0);

		return size_of_data_ptr;
	}

	static bool isStacked(const CodeStorage::Var& var) {
		switch (var.code) {
			case CodeStorage::EVar::VAR_FNC_ARG: return true;
			case CodeStorage::EVar::VAR_LC: return !var.name.empty();
			case CodeStorage::EVar::VAR_GL: return false;
			default: return false;
		}
	}

private:  // data members

	/// Reference to the function in the @p CodeStorage
	const CodeStorage::Fnc& fnc_;

	/**
	 * @brief  The layout of stack frames
	 *
	 * The layout of stack frames (one stack frame corresponds to one structure
	 * with selectors.
	 */
	StackFrameLayout sfLayout_;

	/// The map of identifiers of variables to @p VarInfo
	var_map_type varMap_;

	/// The number of variables in registers
	size_t regCount_;

	/// The number of arguments of the function
	size_t argCount_;


public:   // methods


	/**
	 * @brief  A constructor of a symbolic context for given function
	 *
	 * This is a constructor that creates a new symbolic context for given
	 * function.
	 *
	 * @param[in]  fnc           The function for which the symbolic context is to
	 *                           be created
	 * @param[in]  globalVarMap  Map of global variables (in case the function is
	 *                           to be compiled, otherwise @p nullptr)
	 */
	SymCtx(
		const CodeStorage::Fnc& fnc,
		const var_map_type* globalVarMap = nullptr
	) :
		fnc_(fnc),
		sfLayout_{},
		varMap_{},
		regCount_(2),
		argCount_(0)
	{
		// pointer to previous stack frame
		sfLayout_.push_back(SelData(ABP_OFFSET, ABP_SIZE, 0, "_pABP"));

		// pointer to context info
		sfLayout_.push_back(SelData(RET_OFFSET, RET_SIZE, 0, "_retaddr"));

		size_t offset = ABP_SIZE + RET_SIZE;

		for (auto& funcVar : fnc_.vars)
		{	// for each variable in the function
			const CodeStorage::Var& var = fnc_.stor->vars[funcVar];

			switch (var.code) {
				case CodeStorage::EVar::VAR_LC:
					if (!SymCtx::isStacked(var)) {
						varMap_.insert(
							std::make_pair(var.uid, VarInfo::createInReg(regCount_++))
						);
						break;
					}
					// no break
				case CodeStorage::EVar::VAR_FNC_ARG:
					NodeBuilder::buildNode(sfLayout_, var.type, offset, var.name);
					varMap_.insert(std::make_pair(var.uid, VarInfo::createOnStack(offset)));
					offset += var.type->size;
					if (var.code == CodeStorage::EVar::VAR_FNC_ARG)
						++argCount_;
					break;
				case CodeStorage::EVar::VAR_GL:
				{ // global variables do not occur at the stack, but we need to track
					// them as they can be used
					if (nullptr != globalVarMap)
					{	// in case we are compiling the function
						FA_NOTE("Compiling global variable " << var.name << " in function "
							<< nameOf(fnc_));

						auto itGlobalVar = globalVarMap->find(var.uid);
						if (globalVarMap->end() == itGlobalVar)
						{ // the variable must be in the global map
							assert(false);
						}

						/// @todo: instead of inserting, why not just initialise varMap_ to
						///        globalVarMap
						varMap_.insert(std::make_pair(var.uid, itGlobalVar->second));
					}

					break;
				}
				default:
					assert(false);
			}
		}
	}

	/**
	 * @brief  Constructor for @e global context
	 *
	 * The constructor that creates the context for global variables. The function
	 * reference is set to @p NULL reference (which is not very nice).
	 *
	 * @todo: revise the whole * concept
	 */
	SymCtx(const var_map_type* globalVarMap) :
		fnc_(*static_cast<CodeStorage::Fnc*>(nullptr)),
		sfLayout_{},
		varMap_{*globalVarMap},
		regCount_(0),
		argCount_(0)
	{
		// Assertions
		assert(nullptr != globalVarMap);
	}

	const VarInfo& getVarInfo(size_t id) const {
		var_map_type::const_iterator i = varMap_.find(id);
		assert(i != varMap_.end());
		return i->second;
	}


	const StackFrameLayout& GetStackFrameLayout() const
	{
		return sfLayout_;
	}

	const var_map_type& GetVarMap() const
	{
		return varMap_;
	}

	size_t GetRegCount() const
	{
		return regCount_;
	}

	size_t GetArgCount() const
	{
		return argCount_;
	}

	const CodeStorage::Fnc& GetFnc() const
	{
		return fnc_;
	}

};

#endif
