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

#ifndef ABSTRACT_INSTRUCTION_H
#define ABSTRACT_INSTRUCTION_H

// Standard library headers
#include <ostream>
#include <vector>
#include <unordered_map>
#include <memory>

// Forester headers
#include "symstate.hh"
#include "types.hh"

/**
 * @file abstractinstruction.hh
 * AbstractInstruction - abstract base class for instructions
 */


// forward declaration
class SymState;


namespace CodeStorage {
	struct Fnc;
	struct Storage;
	struct Block;
	struct Insn;
}

class ExecutionManager;

enum class fi_type_e { fiAbort, fiBranch, fiCheck, fiFix, fiJump, fiUnspec };

/**
 * @brief  An abstract base class that represents an instruction
 *
 * Abstract class representing an instruction. This class serves as the base
 * class for all instructions.
 */
class AbstractInstruction
{
private:

	/// pointer to the instruction in CL's code storage
	const CodeStorage::Insn* insn_;

	/// the type of the instruction
	fi_type_e fiType_;

	/// is the instruction the target of some jump?
	bool isTarget_;

private:  // methods

	AbstractInstruction(const AbstractInstruction&);
	AbstractInstruction& operator=(const AbstractInstruction&);

public:

	/**
	 * @brief  The constructor
	 *
	 * ``Default'' constructor, creates a new abstract instruction of given type
	 * for given instruction in CL's code storage.
	 *
	 * @param[in]  insn    Source instruction in the CL's code storage
	 * @param[in]  fiType  The type of the instruction (from #fi_type_e)
	 */
	AbstractInstruction(const CodeStorage::Insn* insn = nullptr,
		fi_type_e fiType = fi_type_e::fiUnspec) :
		insn_(insn), fiType_(fiType), isTarget_(false) {}


	/**
	 * @brief  Virtual destructor
	 *
	 * Virtual destructor.
	 */
	virtual ~AbstractInstruction() {}


	/**
	 * @brief  Method called for finalising the output code
	 *
	 * A virtual method that is to be called when all the instructions of the
	 * program's CL's code storage have been successfully transformed into
	 * a sequence of instructions. The overriding methods may perform some
	 * optimisations, etc.
	 *
	 * @param[in]  codeIndex  The mapping of basic blocks from the CL's code
	 *                        storage to instructions
	 * @param[in]  cur        Iterator for pointing to the instruction in
	 *                        a corresponding container
	 */
	virtual void finalize(
		const std::unordered_map<const CodeStorage::Block*,
			AbstractInstruction*>& codeIndex,
		std::vector<AbstractInstruction*>::const_iterator cur) = 0;


	/**
	 * @brief  Executes given instruction
	 *
	 * The virtual method that executes given instruction in given state of the
	 * virtual machine.
	 *
	 * @param[in,out]  execMan  The execution manager
	 * @param[in]      state    The state of the virtual machine in which the
	 *                          instruction is executed
	 */
	virtual void execute(ExecutionManager& execMan, SymState& state) = 0;


	/**
	 * @brief  Reverses the effect of the instruction
	 *
	 * This method computes the @p SymState that corresponds to reversing the
	 * effect of the instruction on @p bwdSucc and performs intersection of the
	 * result with @p fwdPred. This corresponds to checking spuriousness of
	 * a counterexample in Abstract Regular Tree Model Checking. The parameter @p
	 * fwdPred is the predecessor in the <b>forward run</b> and @p bwdSucc is the
	 * successor in the backward run (looking forward from the initial state).
	 *
	 * @param[in]  execMan  The execution manager
	 * @param[in]  fwdPred  Predecessor in the forward run
	 * @param[in]  bwdSucc  Successor in the backward run
	 *
	 * @returns  @p fwdPred after inverting the instruction and intersection with
	 *           @p bwdSucc
	 */
	virtual SymState* reverseAndIsect(
		ExecutionManager&                      execMan,
		const SymState&                        fwdPred,
		const SymState&                        bwdSucc) const = 0;


	/**
	 * @brief  Outputs instruction to std::ostream
	 *
	 * Virtual method for attaching of a representation of the instruction into
	 * the given stream.
	 *
	 * @param[in,out]  os  The output stream
	 *
	 * @returns  The modified stream
	 */
	virtual std::ostream& toStream(std::ostream& os) const = 0;


	/**
	 * @brief  Gets the instruction from CL's code storage
	 *
	 * This method retrieves the corresponding instruction from the CL's code
	 * storage.
	 *
	 * @returns  The corresponding instruction in the CL's code storage
	 */
	const CodeStorage::Insn* insn() const { return this->insn_; }


	/**
	 * @brief  Sets the corresponding instruction from the CL's code storage
	 *
	 * This method sets the instruction from CL's code storage that corresponds to
	 * the abstract instruction.
	 *
	 * @param[in]  insn  The instruction from the CL's code storage
	 */
	void insn(const CodeStorage::Insn* insn) { this->insn_ = insn; }


	/**
	 * @brief  Gets the type of the instruction
	 *
	 * This method retrieves the type of the instruction.
	 *
	 * @returns  The type of the instruction
	 */
	fi_type_e getType() const { return this->fiType_; }


	/**
	 * @brief  Is the instruction a target of a jump?
	 *
	 * This method retrieves the information whether the instruction is a target
	 * of a function call or a jump.
	 *
	 * @returns  Is the instruction a target of a jump?
	 */
	bool isTarget() const { return this->isTarget_; }


	/**
	 * @brief  Sets the instruction as a jump target
	 *
	 * Sets the flag of the instruction denoting that the instruction is the
	 * target of a function call or a jump.
	 */
	void setTarget() { this->isTarget_ = true; }


	/**
	 * @brief  The output stream operator
	 *
	 * The std::ostream << operator for conversion to a string.
	 *
	 * @param[in,out]  os     The output stream
	 * @param[in]      instr  The value to be appended to the stream
	 *
	 * @returns  The modified output stream
	 */
	friend std::ostream& operator<<(std::ostream& os,
		const AbstractInstruction& instr)
	{
		return instr.toStream(os);
	}

};

#endif
