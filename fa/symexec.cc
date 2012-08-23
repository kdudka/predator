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

// Standard library headers
#include <sstream>
#include <vector>
#include <list>
#include <set>
#include <algorithm>

// Boost headers
//#include <boost/unordered_set.hpp>
//#include <boost/unordered_map.hpp>

// Code Listener headers
#include <cl/code_listener.h>
#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>
#include <cl/clutil.hh>
#include <cl/storage.hh>
#include "../cl/ssd.h"

// Forester headers
#include "forestautext.hh"
#include "symctx.hh"
#include "executionmanager.hh"
#include "fixpointinstruction.hh"
#include "restart_request.hh"
#include "symexec.hh"

using namespace ssd;

#if 0
void dumpOperandTypes(std::ostream& os, const cl_operand* op) {
	os << "operand:" << std::endl;
	cltToStream(os, op->type, false);
	os << "accessors:" << std::endl;
	const cl_accessor* acc = op->accessor;
	while (acc) {
		cltToStream(os, acc->type, false);
		acc = acc->next;
	}
}
#endif

class SymExec::Engine
{
private:  // data members

	TreeAut::Backend taBackend_;
	TreeAut::Backend fixpointBackend_;
	BoxMan boxMan_;

	Compiler compiler_;
	Compiler::Assembly assembly_;

	ExecutionManager execMan_;

	volatile bool dbgFlag_;
	volatile bool userRequestFlag_;

protected:

// TODO: remove (obsolete? we have SymState::printTrace)
//	/**
//	 * @brief  Prints a trace of preceding symbolic states
//	 *
//	 * This static method prints the backtrace from the given symbolic state to
//	 * the initial state.
//	 *
//	 * @param[in]  state  The state for which the backtrace is desired
//	 */
//	static void printTrace(const ExecState& state)
//	{
//		SymState* s = state.GetMem();
//
//		std::vector<SymState*> trace;
//
//		for ( ; s; s = s->GetParent())
//		{	// until we reach the initial state of the execution tree
//			trace.push_back(s);
//		}
//
//		// invert the trace so that it is in the natural order
//		std::reverse(trace.begin(), trace.end());
//
//		CL_NOTE("trace:");
//
//		for (auto s : trace)
//		{	// print out the trace
//			if (s->GetInstr()->insn()) {
//				CL_NOTE_MSG(&s->GetInstr()->insn()->loc,
//					SSD_INLINE_COLOR(C_LIGHT_RED, *s->GetInstr()->insn()));
//				CL_DEBUG_AT(2, std::endl << *s->GetFAE());
//			}
//
//			CL_DEBUG_AT(2, *s->GetInstr());
//		}
//	}

	/**
	 * @brief  Prints boxes
	 *
	 * Method that prints all boxes from the box manager.
	 */
	void printBoxes() const
	{
		std::vector<const Box*> boxes;

		boxMan_.boxDatabase().asVector(boxes);

		std::map<std::string, const Box*> orderedBoxes;

		// reorder according to the name
		for (auto& box : boxes)
		{
			std::stringstream ss;

			ss << *static_cast<const AbstractBox*>(box);

			orderedBoxes.insert(std::make_pair(ss.str(), box));
		}

		for (auto& nameBoxPair : orderedBoxes)
		{
			CL_DEBUG_AT(1, nameBoxPair.first << ':' << std::endl << *nameBoxPair.second);
		}
	}

	/**
	 * @brief  The main execution loop
	 *
	 * This method is the main execution loop for the symbolic execution. It
	 * assumes that the microcode is already compiled, etc.
	 */
	bool main()
	{
		CL_CDEBUG(2, "creating empty heap ...");

		// create an empty heap
		std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(
			new FAE(taBackend_, boxMan_));

		CL_CDEBUG(2, "scheduling initial state ...");

		// schedule the initial state for processing
		execMan_.init(
			DataArray(assembly_.regFileSize_, Data::createUndef()),
			fae,
			assembly_.code_.front()
		);

		ExecState state;

		try
		{	// expecting problems...
			size_t cntStates = 0;

			while (execMan_.dequeueDFS(state))
			{	// process all states in the DFS order
				const CodeStorage::Insn* insn = state.GetMem()->GetInstr()->insn();
				if (nullptr != insn)
				{	// in case current instruction IS an instruction
					CL_CDEBUG(2, SSD_INLINE_COLOR(C_LIGHT_RED, insn->loc << *insn));
					CL_CDEBUG(2, state);
				}
				else
				{
					CL_CDEBUG(3, state);
				}

				if (testAndClearUserRequestFlag())
				{
					CL_NOTE("Executed " << std::setw(7) << cntStates << " states so far.");
				}

				// run the state
				execMan_.execute(state);
				++cntStates;
			}

			return true;
		}
		catch (ProgramError& e)
		{
			//Engine::printTrace(state);
			const CodeStorage::Insn* insn = state.GetMem()->GetInstr()->insn();
			if (nullptr != insn) {
				CL_NOTE_MSG(&insn->loc, SSD_INLINE_COLOR(C_LIGHT_RED, *insn));
				CL_DEBUG_AT(2, std::endl << *state.GetMem()->GetFAE());
			}
			throw;
		}
		catch (RestartRequest& e)
		{	// in case a restart is requested, clear all fixpoint computation points
			for (auto instr : assembly_.code_)
			{
				if (instr->getType() != fi_type_e::fiFix)
				{
					continue;
				}

				// clear the fixpoint
				static_cast<FixpointInstruction*>(instr)->clear();
			}

			CL_CDEBUG(2, e.what());

			return false;
		}
	}

public:

	/**
	 * @brief  The default constructor
	 *
	 * The default constructor.
	 */
	Engine() :
		taBackend_{},
		fixpointBackend_{},
		boxMan_{},
		compiler_(fixpointBackend_, taBackend_, boxMan_),
		assembly_{},
		execMan_{},
		dbgFlag_{false},
		userRequestFlag_{false}
	{ }

	/**
	 * @brief  Loads types from a storage
	 *
	 * This method loads data types and function stackframes from the provided
	 * storage.
	 *
	 * @param[in]  stor  The code storage containing types
	 */
	void loadTypes(const CodeStorage::Storage& stor)
	{
		CL_DEBUG_AT(3, "loading types ...");

		// clear the box manager
		boxMan_.clear();

		// ************ infer data types' layouts ************
		for (const cl_type* type : stor.types)
		{	// for each data type in the storage
			std::vector<size_t> v;
			std::string name;

			switch (type->code)
			{
				case cl_type_e::CL_TYPE_STRUCT: // for a structure

					NodeBuilder::buildNode(v, type);

					if (type->name)
					{	// in case the structure has a name
						name = std::string(type->name);
					}
					else
					{	// in case the structure is nameless
						std::ostringstream ss;
						ss << type->uid;
						name = ss.str();
					}

					CL_DEBUG_AT(3, name);

					boxMan_.createTypeInfo(name, v);
					break;

				default: // for other types
					break;
			}
		}

		// ************ infer functions' stackframes ************
		for (auto fnc : stor.fncs)
		{	// for each function in the storage, create a data structure representing
			// its stackframe
			std::vector<size_t> v;

			const SymCtx ctx(*fnc);
			for (auto sel : ctx.GetStackFrameLayout())
			{	// create the stackframe
				v.push_back(sel.offset);
			}

			std::ostringstream ss;
			ss << nameOf(*fnc) << ':' << uidOf(*fnc);

			CL_DEBUG_AT(3, ss.str());

			boxMan_.createTypeInfo(ss.str(), v);
		}

		// ************ compile layout of the block of global vars ************
		std::vector<const cl_type*> components;
		for (const CodeStorage::Var& var : stor.vars)
		{
			if (CodeStorage::EVar::VAR_GL == var.code)
			{
				components.push_back(var.type);
			}
		}

		std::vector<size_t> v;
		if (!components.empty())
		{ // in case the are some global variables
			NodeBuilder::buildNode(v, components, 0);
		}
		else
		{	// in case there are no global variables, make one fake
			v.push_back(0);
		}

		boxMan_.createTypeInfo(GLOBAL_VARS_BLOCK_STR, v);
		CL_DEBUG_AT(1, "created box for global variables: "
			<< *boxMan_.getTypeInfo(GLOBAL_VARS_BLOCK_STR));
	}

#if 0
	void loadBoxes(const std::unordered_map<std::string, std::string>& db) {

		CL_DEBUG_AT(2, "loading boxes ...");

		for (auto p : db) {

			this->boxes.push_back((const Box*)boxMan_.loadBox(p.first, db));

			CL_DEBUG(p.first << ':' << std::endl << *(const FA*)this->boxes.back());

		}

		boxMan_.buildBoxHierarchy(this->hierarchy, this->basicBoxes);

	}
#endif

	void compile(const CodeStorage::Storage& stor, const CodeStorage::Fnc& entry)
	{
		compiler_.compile(assembly_, stor, entry);
	}

	const Compiler::Assembly& GetAssembly() const
	{
		return assembly_;
	}

	void run()
	{
		// Assertions
		assert(assembly_.code_.size());

		try
		{	// expect problems...
			while (!this->main())
			{	// while the analysis hasn't terminated
			}

			// print out boxes
			this->printBoxes();

			for (auto instr : assembly_.code_)
			{	// print out all fixpoints
				if (instr->getType() != fi_type_e::fiFix)
				{
					continue;
				}

				if (instr->insn()) {
					CL_DEBUG_AT(1, "fixpoint at " << instr->insn()->loc << std::endl
						<< (static_cast<FixpointInstruction*>(instr))->getFixPoint());
				} else {
					CL_DEBUG_AT(1, "fixpoint at unknown location" << std::endl
						<< (static_cast<FixpointInstruction*>(instr))->getFixPoint());
				}
			}

			// print out stats
			CL_DEBUG_AT(1, "forester has generated " << execMan_.statesEvaluated()
				<< " symbolic configuration(s) in " << execMan_.tracesEvaluated()
				<< " trace(s) using " << boxMan_.boxDatabase().size() << " box(es)");
		}
		catch (std::exception& e)
		{
			CL_DEBUG(e.what());

			this->printBoxes();

			throw;
		}
	}

	void run(const Compiler::Assembly& assembly)
	{
		assembly_ = assembly;

		try
		{
			this->run();
			assembly_.code_.clear();
		}
		catch (...)
		{
			assembly_.code_.clear();

			throw;
		}
	}

	void setDbgFlag()
	{
		dbgFlag_ = true;
	}

	void setUserRequestFlag()
	{
		userRequestFlag_ = true;
	}

	bool testAndClearUserRequestFlag()
	{
		bool oldValue = userRequestFlag_;
		userRequestFlag_ = false;
		return oldValue;
	}
};

SymExec::SymExec() :
	engine(new Engine())
{ }

SymExec::~SymExec()
{
	// Assertions
	assert(engine != nullptr);

	delete this->engine;
}

void SymExec::loadTypes(const CodeStorage::Storage& stor)
{
	// Assertions
	assert(engine != nullptr);

	this->engine->loadTypes(stor);
}

#if 0
void SymExec::loadBoxes(const std::unordered_map<std::string, std::string>& db) {
	this->engine->loadBoxes(db);
}
#endif

const Compiler::Assembly& SymExec::GetAssembly() const
{
	// Assertions
	assert(nullptr != engine);

	return this->engine->GetAssembly();
}

void SymExec::compile(const CodeStorage::Storage& stor,
	const CodeStorage::Fnc& main)
{
	// Assertions
	assert(engine != nullptr);

	this->engine->compile(stor, main);
}

void SymExec::run()
{
	// Assertions
	assert(engine != nullptr);

	this->engine->run();
}

void SymExec::run(const Compiler::Assembly& assembly)
{
	// Assertions
	assert(engine != nullptr);

	this->engine->run(assembly);
}

void SymExec::setDbgFlag()
{
	// Assertions
	assert(engine != nullptr);

	this->engine->setDbgFlag();
}

void SymExec::setUserRequestFlag()
{
	// Assertions
	assert(engine != nullptr);

	this->engine->setUserRequestFlag();
}
