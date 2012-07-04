/*
 * Copyright (C) 2012  Ondrej Lengal
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

#ifndef _EXEC_STATE_HH_
#define _EXEC_STATE_HH_

// Standard library headers
#include <memory>

// Forester headers
#include "types.hh"


// Declarations of used classes
struct SymState;

/**
 * @brief  Type of state of execution
 *
 * The data type for the state of execution. It contains a vector of values
 * in registers and a state of the symbolic heap.
 */
class ExecState
{
private:  // data members

	/// the registers
	std::shared_ptr<DataArray> regs_;

	/// the memory
	SymState* mem_;

private:  // methods

	DataArray& GetRegs()
	{
		// Assertions
		assert(nullptr != regs_);
		assert(nullptr != mem_);

		return *regs_;
	}

public:   // methods

	/**
	 * @brief  Default constructor
	 *
	 * Constructs an empty object of the class.
	 */
	ExecState() :
		regs_(nullptr),
		mem_(nullptr)
	{ }

	/**
	 * @brief  Constructor
	 *
	 * Constructs an object of the class.
	 *
	 * @param[in]  regs  Registers of the state of execution
	 * @param[in]  mem   Representation of the memory
	 */
	ExecState(const std::shared_ptr<DataArray>& regs, SymState* mem) :
		regs_(regs),
		mem_(mem)
	{
		// Assertions
		assert(nullptr != regs_);
		assert(nullptr != mem_);
	}

	/**
	 * @brief  Copy constructor
	 *
	 * @param[in]  state  The execution state to be copied
	 */
	ExecState(const ExecState& state) :
		regs_(state.regs_),
		mem_(state.mem_)
	{
		// Assertions
		assert(nullptr != regs_);
		assert(nullptr != mem_);
	}

	/**
	 * @brief  Assignment operator
	 *
	 * @param[in]  state  The state to be assigned
	 *
	 * @returns  The new object
	 */
	ExecState& operator=(const ExecState& state)
	{
		if (&state != this)
		{
			regs_ = state.regs_;
			mem_ = state.mem_;
		}

		// Assertions
		assert(nullptr != regs_);
		assert(nullptr != mem_);

		return *this;
	}

	const DataArray& GetRegs() const
	{
		// Assertions
		assert(nullptr != regs_);
		assert(nullptr != mem_);

		return *regs_;
	}

	const std::shared_ptr<DataArray>& GetRegsShPtr() const
	{
		return regs_;
	}

	const Data& GetReg(size_t index) const
	{
		// Assertions
		assert(index < this->GetRegs().size());

		return this->GetRegs()[index];
	}

	void SetReg(size_t index, const Data& data)
	{
		// Assertions
		assert(index < this->GetRegs().size());

		this->GetRegs()[index] = data;
	}

	SymState* GetMem() const
	{
		// Assertions
		assert(nullptr != mem_);

		return mem_;
	}


	/**
	 * @brief  Run a visitor on the instance
	 *
	 * This method is the @p accept method of the Visitor design pattern.
	 *
	 * @param[in]  visitor  The visitor of the type @p TVisitor
	 *
	 * @tparam  TVisitor  The type of the visitor
	 */
	template <class TVisitor>
	void accept(TVisitor& visitor) const
	{
		visitor(*this);
	}

	/**
	 * @brief  The output stream operator
	 *
	 * The std::ostream << operator for conversion to a string.
	 *
	 * @param[in,out]  os     The output stream
	 * @param[in]      state  The value to be appended to the stream
	 *
	 * @returns  The modified output stream
	 */
	friend std::ostream& operator<<(std::ostream& os, const ExecState& state);
};

#endif /* _EXEC_STATE_HH_ */
