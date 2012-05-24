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
#include <cl/cl_msg.hh>

// Forester headers
#include "types.hh"
#include "nodebuilder.hh"
#include "notimpl_except.hh"

#define ABP_OFFSET		0
#define ABP_SIZE		SymCtx::getSizeOfDataPtr()
#define RET_OFFSET		(ABP_OFFSET + ABP_SIZE)
#define RET_SIZE		SymCtx::getSizeOfCodePtr()

struct SymCtx {

public:   // data types


	/// Stack frame layout
	typedef std::vector<SelData> StackFrameLayout;


	/**
	 * @brief  Structure with run-time information about variable's location
	 *
	 * This structure contains information about the location of a variable, i.e.
	 * whether it is on a stack or in some register, and also the offset from the
	 * base pointer of the corresponding stack frame (in case it is on a stack),
	 * or the index of the register (in case it is therein).
	 */
	class VarInfo
	{
	private:  // data members 

		bool isStack_;
		union
		{
			size_t stackOffset_;
			size_t regIndex_;
		};

	private:  // methods

		VarInfo(bool isStack, size_t offsetIndex)
			: isStack_(isStack)
		{
			if (isOnStack())
			{
				stackOffset_ = offsetIndex;
			}
			else
			{
				regIndex_ = offsetIndex;
			}
		}

	public:   // methods

		/**
		 * @brief  Checks whether the variable is on the stack
		 *
		 * This method returns a Boolean value meaning whether the variable is on
		 * the stack.
		 *
		 * @returns  @p true in case the variable is on the stack, @p false
		 *           otherwise
		 */
		bool isOnStack() const { return isStack_;}

		/**
		 * @brief  Returns the stack offset of the variable
		 *
		 * This method returns the offset of the variable from the stack frame base
		 * pointer (for variables which are on the stack).
		 *
		 * @returns  The offset of the variable from the stack frame base pointer
		 */
		size_t getStackOffset() const
		{
			// Assertions
			assert(isOnStack());

			return stackOffset_;
		}

		/**
		 * @brief  Returns the index of the register of the variable
		 *
		 * This method returns the index of the register in which there is the
		 * variable (for variables which are in registers).
		 *
		 * @returns  The index of the register in which the variable is
		 */
		size_t getRegIndex() const
		{
			// Assertions
			assert(!isOnStack());

			return regIndex_;
		}

		/**
		 * @brief  Static method creating a variable on a stack
		 *
		 * This static method creates a new variable on a stack at given @p offset
		 * from the base pointer of the stack frame.
		 *
		 * @param[in]  offset  The offset of the variable in the given stack frame
		 *
		 * @returns  New @p VarInfo structure for the variable
		 */
		static VarInfo createOnStack(size_t offset)
		{
			return VarInfo(true, offset);
		}

		/**
		 * @brief  Static method creating a variable in a register
		 *
		 * This static method creates a new variable in the register with given @p
		 * index.
		 *
		 * @param[in]  index  Index of the register in which the variable is stored
		 *
		 * @returns  New @p VarInfo structure for the variable
		 */
		static VarInfo createInReg(size_t index)
		{
			return VarInfo(false, index);
		}
	};


	/**
	 * @brief  The type that maps identifiers of variables to @p VarInfo
	 *
	 * This type serves as a map between identifiers of variables and @p VarInfo
	 * structures.
	 */
	typedef std::unordered_map<int, VarInfo> var_map_type;


private:  // static data

	// must be initialised externally!

	/// The size of a data pointer in the analysed program
	static int size_of_data_ptr;

	/// The size of a code pointer in the analysed program
	static int size_of_code_ptr;


public:   // static methods

	/**
	 * @brief  Initialise the symbolic context
	 *
	 * This static method needs to be called before the @p SymCtx structure is
	 * used for the first time. It properly initialises static members of the
	 * structure from the passed @p CodeStorage.
	 *
	 * @param[in]  stor  The @p CodeStorage from which the context is to be
	 *                   initialised
	 */
	static void initCtx(const CodeStorage::Storage& stor)
	{
		if ((size_of_code_ptr = stor.types.codePtrSizeof()) == -1)
		{	// set the size of a code pointer
			size_of_code_ptr = sizeof(void(*)());
		}

		if ((size_of_data_ptr = stor.types.dataPtrSizeof()) == -1)
		{	// set the size of a data pointer
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

	const CodeStorage::Fnc& fnc_;


	/**
	 * @brief  The layout of stack frames
	 *
	 * The layout of stack frames (one stack frame corresponds to one structure
	 * with selectors.
	 */
	StackFrameLayout sfLayout;

	/// The map of identifiers of variables to @p VarInfo
	var_map_type varMap;

	size_t regCount_;
	size_t argCount_;

	bool isReg(const cl_operand* op, size_t& id) const {
		if (op->code != cl_operand_e::CL_OPERAND_VAR)
			return false;
		var_map_type::const_iterator i = this->varMap.find(varIdFromOperand(op));
		assert(i != this->varMap.end());
		if (i->second.isOnStack())
			return false;
		id = i->second.isOnStack();
		return true;
	}

	const VarInfo& getVarInfo(size_t id) const {
		var_map_type::const_iterator i = this->varMap.find(id);
		assert(i != this->varMap.end());
		return i->second;
	}


public:   // methods


	/**
	 * @brief  A constructor of a symbolic context for given function
	 *
	 * This is a constructor that creates a new symbolic context for given
	 * function.
	 *
	 * @param[in]  fnc  The function for which the symbolic context is to be
	 *                  created
	 */
	SymCtx(const CodeStorage::Fnc& fnc) :
		fnc_(fnc), regCount_(2), argCount_(0)
	{
		// pointer to previous stack frame
		this->sfLayout.push_back(SelData(ABP_OFFSET, ABP_SIZE, 0));

		// pointer to context info
		this->sfLayout.push_back(SelData(RET_OFFSET, RET_SIZE, 0));

		size_t offset = ABP_SIZE + RET_SIZE;

		for (auto& funcVar : fnc_.vars)
		{	// for each variable in the function
			const CodeStorage::Var& var = fnc_.stor->vars[funcVar];

			switch (var.code) {
				case CodeStorage::EVar::VAR_LC:
					if (!SymCtx::isStacked(var)) {
						this->varMap.insert(
							std::make_pair(var.uid, VarInfo::createInReg(regCount_++))
						);
						break;
					}
					// no break
				case CodeStorage::EVar::VAR_FNC_ARG:
					NodeBuilder::buildNode(this->sfLayout, var.type, offset);
					this->varMap.insert(std::make_pair(var.uid, VarInfo::createOnStack(offset)));
					offset += var.type->size;
					if (var.code == CodeStorage::EVar::VAR_FNC_ARG)
						++argCount_;
					break;
				case CodeStorage::EVar::VAR_GL:
					throw NotImplementedException("global variables", &(var.loc));
					break;
				default:
					assert(false);
			}
		}
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
