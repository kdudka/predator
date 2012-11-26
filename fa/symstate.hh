/*
 * Copyright (C) 2011 Jiri Simacek
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

#ifndef SYM_STATE_H
#define SYM_STATE_H

// Standard library headers
#include <list>
#include <vector>
#include <set>
#include <memory>
#include <cassert>

// Forester headers
#include "abstractinstruction.hh"
#include "forestautext.hh"
#include "link_tree.hh"
#include "recycler.hh"
#include "types.hh"

/**
 * @file symstate.hh
 * SymState - structure that represents symbolic state of the execution engine
 */

// forward declaration
class AbstractInstruction;

/**
 * @brief  Symbolic state of the execution
 */
class SymState : public LinkTree
{
private:  // data types

	typedef TreeAut::Transition Transition;

public:   // data types

	/// Trace of symbolic states
	typedef std::vector<const SymState*> Trace;

private:  // data members

	/// Instruction that the symbolic state corresponds to
	AbstractInstruction* instr_;

	/// Forest automaton for the symbolic state
	std::shared_ptr<const FAE> fae_;

	/// the registers
	std::shared_ptr<DataArray> regs_;

private:  // methods

	SymState(const SymState&);
	SymState& operator=(const SymState&);

public:   // methods

	/**
	 * @brief  Constructor
	 *
	 * Default constructor
	 */
	SymState() :
		instr_{},
		fae_{},
		regs_(nullptr)
	{ }

	/**
	 * @brief  Destructor
	 *
	 * Destructor
	 */
	~SymState()
	{
		// Assertions
		assert(nullptr == fae_);
	}

	std::shared_ptr<const FAE> GetFAE() const
	{
		return fae_;
	}

	const DataArray& GetRegs() const
	{
		// Assertions
		assert(nullptr != regs_);

		return *regs_;
	}

	const Data& GetReg(size_t index) const
	{
		// Assertions
		assert(index < this->GetRegs().size());

		return this->GetRegs()[index];
	}

	size_t GetRegCount() const
	{
		return this->GetRegs().size();
	}

	void SetReg(size_t index, const Data& data)
	{
		// Assertions
		assert(nullptr != regs_);
		assert(index < this->GetRegs().size());

		(*regs_)[index] = data;
	}

	const std::shared_ptr<DataArray>& GetRegsShPtr() const
	{
		return regs_;
	}

	const AbstractInstruction* GetInstr() const
	{
		return instr_;
	}

	AbstractInstruction* GetInstr()
	{
		return instr_;
	}

	void SetFAE(const std::shared_ptr<FAE> fae)
	{
		fae_ = fae;
	}


	/**
	 * @brief  Initializes the symbolic state
	 *
	 * Method that initializes the symbolic state.
	 *
	 * @param[in]  parent    The parent symbolic state
	 * @param[in]  instr     The instruction the state corresponds to
	 * @param[in]  fae       The forest automaton for the symbolic state
	 * @param[in]  regs      Values of registers
	 */
	void init(
		SymState*                                      parent,
		AbstractInstruction*                           instr,
		const std::shared_ptr<const FAE>&              fae,
		const std::shared_ptr<DataArray>&              regs);


	/**
	 * @brief  Initializes the symbolic state
	 *
	 * Method that initializes the symbolic state from an old state
	 *
	 * @param[in]  oldState  The old symbolic state
	 */
	void init(
		const SymState&                                oldState);


	/**
	 * @brief  Initializes the symbolic state with new registers
	 *
	 * Method that initializes the symbolic state from an old state using new
	 * register array.
	 *
	 * @param[in]  oldState  The old symbolic state
	 * @param[in]  regs      New register array
	 */
	void init(
		const SymState&                                oldState,
		const std::shared_ptr<DataArray>               regs);


	/**
	 * @brief  Initializes the symbolic state with new registers
	 *
	 * Method that initializes the symbolic state from an old state using new
	 * register array.
	 *
	 * @param[in]  oldState  The old symbolic state
	 * @param[in]  regs      New register array
	 */
	void init(
		const SymState&                                oldState,
		const std::shared_ptr<DataArray>               regs,
		AbstractInstruction*                           insn);


	/**
	 * @brief  Initialises the symbolic state from its parent
	 *
	 * Method that initialises the symbolic state from its @p parent (which is indeed
	 * set as the parent state) for the given instruction @p instr.
	 *
	 * @param[in,out]  parent  The parent state
	 * @param[in]      instr   The instruction to be associated with the state
	 */
	void initChildFrom(
		SymState*                                      parent,
		AbstractInstruction*                           instr);


	/**
	 * @brief  Initialises the symbolic state from its parent with new registers
	 *
	 * Method that initialises the symbolic state from its @p parent (which is indeed
	 * set as the parent state) for the given instruction @p instr, with new
	 * register array
	 *
	 * @param[in,out]  parent  The parent state
	 * @param[in]      instr   The instruction to be associated with the state
	 * @param[in]      regs    The new register array
	 */
	void initChildFrom(
		SymState*                                      parent,
		AbstractInstruction*                           instr,
		const std::shared_ptr<DataArray>               regs);

	/**
	 * @brief  Recycles the symbolic state for further use
	 *
	 * This method recycles the symbolic state for further use. It also recycles
	 * recursively all children of the state.
	 *
	 * @param[in,out]  recycler  The Recycler object
	 */
	void recycle(Recycler<SymState>& recycler);


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
	 * @brief  Retrieves the trace to this state
	 *
	 * This method retrieves the trace to this state in the reverse order
	 * (starting with the current state).
	 *
	 * @returns  The trace
	 */
	Trace getTrace() const;


	/**
	 * @brief  Substitutes references to one FA root with another root
	 *
	 * Traverses @p *this and @p src in parallel and in places where @p src
	 * references @p oldValue adds (in @p *this) a reference to @p newValue.
	 *
	 * @param[in]  src       The reference symbolic state
	 * @param[in]  oldValue  The value to be substituted
	 * @param[in]  newValue  The new value
	 */
	void SubstituteRefs(
		const SymState&      src,
		const Data&          oldValue,
		const Data&          newValue);


	/**
	 * @brief  Performs intersection of the symbolic state with other state
	 *
	 * This method performs intersection with another symbolic state, i.e. it
	 * creates a forest automaton such that the language of the automaton is the
	 * intersection of languages of automata of the states and alters local
	 * registers accordingly. 
	 *
	 * @param[in]  fwd  The other symbolic state (from the forward run)
	 *
	 * @note  This may not compute the most precise intersection!
	 */
	void Intersect(
		const SymState&      fwd);


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
	friend std::ostream& operator<<(std::ostream& os, const SymState& state);
};

#endif
