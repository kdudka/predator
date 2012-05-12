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
using std::vector;
using std::list;
using std::set;

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

class SymExec::Engine {

	TA<label_type>::Backend taBackend;
	TA<label_type>::Backend fixpointBackend;
	BoxMan boxMan;

#if 0
	std::vector<const Box*> boxes;
	std::vector<const Box*> basicBoxes;
	boost::unordered_map<const Box*, std::vector<const Box*> > hierarchy;
#endif

	Compiler compiler_;
	Compiler::Assembly assembly_;

	ExecutionManager execMan;

	bool dbgFlag;

protected:

#if 0
	bool foldBox(SymState* target, FAE& fae, size_t root, const Box* box) {
		CL_CDEBUG("trying " << *(const AbstractBox*)box << " at " << root);
		if (!fae.foldBox(root, box))
			return false;
		CL_CDEBUG("match");
		std::set<size_t> tmp;
		fae.getNearbyReferences(fae.varGet(ABP_INDEX).d_ref.root, tmp);
		FAE::NormInfo normInfo;
		fae.normalize(normInfo, tmp);
		boost::unordered_map<const Box*, std::vector<const Box*> >::iterator i =
			this->hierarchy.find(box);
		if (i == this->hierarchy.end())
			return true;
		this->recAbstractAndFold(target, fae, i->second);
		return true;
	}

	void recAbstractAndFold(SymState* target, FAE& fae, const std::vector<const Box*>& boxes) {

		CL_CDEBUG("abstracting and folding ... " << target->absHeight);
//		fae.heightAbstraction(target->absHeight, ExactLabelMatchF());
		CL_CDEBUG(std::endl << fae);

		// do not fold at 0
		for (size_t i = 1; i < fae.getRootCount(); ++i) {
			fae.heightAbstraction(i, target->absHeight, ExactLabelMatchF());
			for (std::vector<const Box*>::const_iterator j = boxes.begin(); j != boxes.end(); ++j) {
				if (this->foldBox(target, fae, i, *j))
					i = 1;
			}
		}

	}
	struct InvalidateF {

		list<SymState*>& queue;
		set<CfgStateExt*>& s;

		InvalidateF(list<SymState*>& queue, set<CfgStateExt*>& s) : queue(queue), s(s) {}

		void operator()(SymState* state) {
			CfgState* cfgState = CFG_FROM_FAE(*state->fae);
			if (state->queueTag != this->queue.end())
				this->queue.erase(state->queueTag);
//			state->invalidate(this->queue, node->fae);
			if (cfgState->hasExt)
				s.insert((CfgStateExt*)cfgState);
		}

	};

	void processState(SymState* state) {

		assert(state);

		CfgState* cfgState = CFG_FROM_FAE(*state->fae);

		if (cfgState->hasExt && ((CfgStateExt*)cfgState)->testInclusion(*state->fae)) {

			++this->tracesEvaluated;

			CL_CDEBUG("hit");

			this->traceRecorder.destroyBranch(state, DestroySimpleF());

			return;

		}

		state->queueTag = this->queue.end();

		try {

			const cl_loc& loc = (*cfgState->insn)->loc;

			CL_CDEBUG(loc << ' ' << **cfgState->insn);
			CL_CDEBUG("preprocessing " << state);
			CL_CDEBUG(std::endl << SymCtx::Dump(*cfgState->ctx, *state->fae));
			CL_CDEBUG(std::endl << *state->fae);

			std::vector<FAE*> tmp;

			ContainerGuard<std::vector<FAE*> > g(tmp);

			cfgState->prepareOperands(tmp, *state->fae);

			for (FAE*& fae : tmp) {

				ExecInfo info(state, cfgState, fae);

				this->processState(info);

				fae = NULL;

			}

			g.release();

		} catch (const ProgramError& e) {

			CL_CDEBUG(e.what());

			this->printTrace(state);

			throw;

			TraceRecorder::Item* item = this->revRun(*fae);

			if (!item)

				throw ProgramError(e.what(), &(*state->insn)->loc);

			CL_DEBUG("spurious counter example ...");

			this->printTrace(*fae);

			throw;

			state = STATE_FROM_FAE(*item->fae);

			assert(state->entryPoint);

			set<SymState*> s;

			this->traceRecorder.invalidateChildren(item, InvalidateF(this->queue, s));

			const FAE* tmp2 = item->fae;

			TraceRecorder::Item* parent = item->parent;

			InvalidateF(this->queue, s)(item);

			this->traceRecorder.remove(tmp2);

			assert(parent);

			parent->removeChild(item);

			for (set<SymState*>::iterator i = s.begin(); i != s.end(); ++i) {
				(*i)->recompute();
				CL_CDEBUG("new fixpoint:" << std::endl << (*i)->fwdConf);
			}

			const cl_loc& loc = (*state->insn)->loc;

			CL_CDEBUG("adjusting abstraction ... " << ++state->absHeight);
			CL_CDEBUG("resuming execution ... ");
			CL_CDEBUG(loc << ' ' << **state->insn);

			parent->queueTag = this->queue.insert(this->queue.end(), parent->fae);

		}

	}

	void printInfo(SymState* state) {
		if (this->dbgFlag) {
			CfgState* cfgState = CFG_FROM_FAE(*state->fae);
			assert(cfgState);
			if (!cfgState->hasExt)
				return;
			CL_DEBUG(std::endl << SymCtx::Dump(*((CfgStateExt*)cfgState)->ctx, *state->fae));
			CL_DEBUG(std::endl << *state->fae);
			CL_DEBUG("evaluated states: " << this->statesEvaluated << ", evaluated traces: " << this->tracesEvaluated);
			this->dbgFlag = false;
		}
	}
#endif
#if 0
	TraceRecorder::Item* revRun(const FAE& fae) {

		CL_CDEBUG("reconstructing abstract trace ...");

		vector<pair<const FAE*, const CodeStorage::Insn*> > trace;

		TraceRecorder::Item* item = this->traceRecorder.find(&fae);

		FAE tmp(fae);

		SymState* state = NULL;

		while (item->parent) {

			CL_CDEBUG(std::endl << SymCtx::Dump(*STATE_FROM_FAE(*item->fae)->ctx, *item->fae));
			CL_CDEBUG(std::endl << tmp);

			state = STATE_FROM_FAE(*item->parent->fae);

			CL_CDEBUG("rewinding " << (*state->insn)->loc << ' ' << **state->insn);

			switch (item->itemType) {

				case tr_item_type::itDenormalize: {

					CL_CDEBUG("denormalizing " << std::endl << tmp << "with" << std::endl << *item->fae);
					CL_CDEBUG(item->normInfo);

					if (!Normalization(tmp).denormalize(*item->fae, item->normInfo)) {
						CL_CDEBUG("spurious counter example (denormalization)!" << std::endl << *item->fae);
						return item;
					}

					break;

				}

				case tr_item_type::itReverse: {

					CL_CDEBUG("reversing " << std::endl << tmp << "with" << std::endl << *item->parent->fae);

					if (!ReverseRun(tmp).reverse(*item->parent->fae)) {
						CL_CDEBUG("spurious counter example (reversal)!" << std::endl << *item->parent->fae);
						return item;
					}

					NormInfo normInfo;

					VirtualMachine vm(tmp);

					std::set<size_t> s;
					vm.getNearbyReferences(vm.varGet(ABP_INDEX).d_ref.root, s);
					Normalization(tmp).normalize(normInfo, s);

					break;

				}

			}

			if (item->itemType == tr_item_type::itDenormalize)
				trace.push_back(make_pair(item->fae, *state->insn));

			item = item->parent;

		}

		assert(state);

//		trace.push_back(make_pair(item->fae, *state->insn));

		CL_CDEBUG("trace:");

		for (vector<pair<const FAE*, const CodeStorage::Insn*> >::reverse_iterator i = trace.rbegin(); i != trace.rend(); ++i) {
			if (i->second)
				CL_NOTE_MSG(&i->second->loc, *(i->second));
//			STATE_FROM_FAE(*i->first)->ctx->dumpContext(*i->first);
//			CL_CDEBUG(std::endl << *(i->first));
		}

		CL_NOTE_MSG(&this->currentInsn->loc, *this->currentInsn);

		return NULL;

	}
#endif
#if 0
	void printQueue() const {
		for (SymState* state : this->queue)
			std::cerr << *state->fae;
	}
#endif

	/**
	 * @brief  Prints a trace of preceding symbolic states
	 *
	 * This static method prints the backtrace from the given symbolic state to
	 * the initial state.
	 *
	 * @param[in]  state  The state for which the backtrace is desired
	 */
	static void printTrace(const AbstractInstruction::StateType& state)
	{
		SymState* s = state.second;

		std::vector<SymState*> trace;

		for ( ; s; s = s->parent)
		{	// until we reach the initial state of the execution tree
			trace.push_back(s);
		}

		// invert the trace so that it is in the natural order
		std::reverse(trace.begin(), trace.end());

		CL_NOTE("trace:");

		for (auto s : trace)
		{	// print out the trace
			if (s->instr->insn()) {
				CL_NOTE_MSG(&s->instr->insn()->loc,
					SSD_INLINE_COLOR(C_LIGHT_RED, *s->instr->insn()));
				CL_DEBUG_AT(2, std::endl << *s->fae);
			}

			CL_DEBUG_AT(2, *s->instr);
		}
	}

	/**
	 * @brief  Prints boxes
	 *
	 * Method that prints all boxes from the box manager.
	 */
	void printBoxes() const
	{
		std::vector<const Box*> boxes;

		this->boxMan.boxDatabase().asVector(boxes);

		std::map<std::string, const Box*> orderedBoxes;

		// reorder according to the name
		for (auto& box : boxes)
		{
			std::stringstream ss;

			ss << *(const AbstractBox*)box;

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
			new FAE(this->taBackend, this->boxMan));

		CL_CDEBUG(2, "sheduling initial state ...");

		// schedule the initial state for processing
		this->execMan.init(
			std::vector<Data>(this->assembly_.regFileSize_, Data::createUndef()),
			fae,
			this->assembly_.code_.front()
		);

		AbstractInstruction::StateType state;

		try
		{	// expecting problems...
			while (this->execMan.dequeueDFS(state))
			{	// process all states in the DFS order
				if (state.second->instr->insn())
				{	// in case current instruction IS an instruction
					CL_CDEBUG(2, SSD_INLINE_COLOR(C_LIGHT_RED,
						state.second->instr->insn()->loc << *(state.second->instr->insn())));
					CL_CDEBUG(2, state);
				}
				else
				{
					CL_CDEBUG(3, state);
				}

				// run the state
				this->execMan.execute(state);
			}

			return true;
		}
		catch (ProgramError& e)
		{
//			Engine::printTrace(state);
			if (state.second->instr->insn()) {
				CL_NOTE_MSG(&state.second->instr->insn()->loc,
					SSD_INLINE_COLOR(C_LIGHT_RED, *state.second->instr->insn()));
				CL_DEBUG_AT(2, std::endl << *state.second->fae);
			}
			throw;
		}
		catch (RestartRequest& e)
		{	// in case a restart is requested, clear all fixpoint computation points
			for (auto instr : this->assembly_.code_)
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
		boxMan(), compiler_(this->fixpointBackend, this->taBackend, this->boxMan),
		dbgFlag(false)
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
		this->boxMan.clear();

		for (auto type : stor.types)
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

					this->boxMan.createTypeInfo(name, v);
					break;

				default: // for other types
					break;
			}
		}

		for (auto fnc : stor.fncs)
		{	// for each function in the storage, create a data structure representing
			// its stackframe
			std::vector<size_t> v;

			for (auto sel : SymCtx(*fnc).sfLayout)
			{	// create the stackframe
				v.push_back(sel.offset);
			}

			std::ostringstream ss;
			ss << nameOf(*fnc) << ':' << uidOf(*fnc);

			CL_DEBUG_AT(3, ss.str());

			this->boxMan.createTypeInfo(ss.str(), v);
		}
	}

#if 0
	void loadBoxes(const std::unordered_map<std::string, std::string>& db) {

		CL_DEBUG_AT(2, "loading boxes ...");

		for (auto p : db) {

			this->boxes.push_back((const Box*)this->boxMan.loadBox(p.first, db));

			CL_DEBUG(p.first << ':' << std::endl << *(const FA*)this->boxes.back());

		}

		this->boxMan.buildBoxHierarchy(this->hierarchy, this->basicBoxes);

	}
#endif

	void compile(const CodeStorage::Storage& stor, const CodeStorage::Fnc& entry)
	{
		CL_DEBUG_AT(2, "compiling ...");
		this->compiler_.compile(this->assembly_, stor, entry);
		CL_DEBUG_AT(2, "assembly:" << std::endl << this->assembly_);
	}

	void run()
	{
		// Assertions
		assert(this->assembly_.code_.size());

		try
		{	// expect problems...
			while (!this->main())
			{	// while the analysis hasn't terminated
			}

			// print out boxes
			this->printBoxes();

			for (auto instr : this->assembly_.code_)
			{	// print out all fixpoints
				if (instr->getType() != fi_type_e::fiFix)
				{
					continue;
				}

				if (instr->insn()) {
					CL_DEBUG_AT(1, "fixpoint at " << instr->insn()->loc << std::endl
						<< ((FixpointInstruction*)instr)->getFixPoint());
				} else {
					CL_DEBUG_AT(1, "fixpoint at unknown location" << std::endl
						<< ((FixpointInstruction*)instr)->getFixPoint());
				}
			}

			// print out stats
			CL_DEBUG_AT(1, "forester has generated " << this->execMan.statesEvaluated()
				<< " symbolic configuration(s) in " << this->execMan.tracesEvaluated() 
                                << " trace(s) using " << this->boxMan.boxDatabase().size() << " box(es)");

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
		this->assembly_ = assembly;

		try {

			this->run();
			this->assembly_.code_.clear();

		} catch (...) {

			this->assembly_.code_.clear();

			throw;

		}

	}

	void setDbgFlag()
	{
		this->dbgFlag = 1;
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
