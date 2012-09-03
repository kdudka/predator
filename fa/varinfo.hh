/*
 * Copyright (C) 2012 Jiri Simacek
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

#ifndef _VARINFO_HH_
#define _VARINFO_HH_


/**
 * @brief  Class with run-time information about variable's location
 *
 * This class contains information about the location of a variable, i.e.
 * whether it is on a stack or in some register, and also the offset from the
 * base pointer of the corresponding stack frame (in case it is on a stack),
 * or the index of the register (in case it is therein).
 */
class VarInfo
{
private:  // data types

	/// Possible location of a variable in the memory
	enum class TVarLocation
	{
		ON_STACK,         ///< the variable is on the stack
		IN_REGISTER,      ///< the variable is in a register
		GLOBAL            ///< the variable is global
	};

private:  // data members 

	/// The location of the variable
	TVarLocation varLocation_;

	union
	{
		size_t stackOffset_;         ///< offset on the stack from the base pointer
		size_t regIndex_;            ///< index of the register
		size_t globalBlockOffset_;   ///< offset in the block for global variables
	};

private:  // methods

	VarInfo(TVarLocation location, size_t offsetIndex)
		: varLocation_(location)
	{
		if (isOnStack()) {
			stackOffset_ = offsetIndex;
		}
		else if (isInReg()) {
			regIndex_ = offsetIndex;
		}
		else if (isGlobal()) {
			globalBlockOffset_ = offsetIndex;
		}
		else {
			assert(false);    // fail gracefully
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
	bool isOnStack() const { return TVarLocation::ON_STACK == varLocation_;}

	/**
	 * @brief  Checks whether the variable is in a register
	 *
	 * This method returns a Boolean value meaning whether the variable is in
	 * a register.
	 *
	 * @returns  @p true in case the variable is in a register, @p false
	 *           otherwise
	 */
	bool isInReg() const { return TVarLocation::IN_REGISTER == varLocation_;}

	/**
	 * @brief  Checks whether the variable is global
	 *
	 * This method returns a Boolean value meaning whether the variable is global.
	 *
	 * @returns  @p true in case the variable is global, @p false otherwise
	 */
	bool isGlobal() const { return TVarLocation::GLOBAL == varLocation_;}

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
		assert(isInReg());

		return regIndex_;
	}

	/**
	 * @brief  Returns the offset of the variable in the block of global variables
	 *
	 * This method returns the offset of the variable in the block of global
	 * variables (for variables which are global).
	 *
	 * @returns  The offset of the variable in the block of global variables
	 */
	size_t getGlobalBlockOffset() const
	{
		// Assertions
		assert(isGlobal());

		return globalBlockOffset_;
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
		return VarInfo(TVarLocation::ON_STACK, offset);
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
		return VarInfo(TVarLocation::IN_REGISTER, index);
	}

	/**
	 * @brief  Static method creating a global variable
	 *
	 * This static method creates a new global variable at given @p offset
	 * in the block of global variables.
	 *
	 * @param[in]  offset  The offset of the variable in the block of global
	 *                     variables
	 *
	 * @returns  New @p VarInfo structure for the variable
	 */
	static VarInfo createGlobal(size_t offset)
	{
		return VarInfo(TVarLocation::GLOBAL, offset);
	}

};

#endif /* _VARINFO_HH_ */
