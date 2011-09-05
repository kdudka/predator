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

#include <sstream>
#include <vector>
#include <list>
#include <set>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include <cl/code_listener.h>
#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>
#include <cl/clutil.hh>
#include <cl/storage.hh>

#include "forestautext.hh"
#include "symctx.hh"
#include "executionmanager.hh"
#include "fixpointinstruction.hh"

#include "symexec.hh"

using std::vector;
using std::list;
using std::set;
using boost::unordered_set;
using boost::unordered_map;

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

class SymExec::Engine {

	TA<label_type>::Backend taBackend;
	TA<label_type>::Backend fixpointBackend;
	BoxMan boxMan;

	std::vector<const Box*> boxes;
	std::vector<const Box*> basicBoxes;
	boost::unordered_map<const Box*, std::vector<const Box*> > hierarchy;

	Compiler compiler_;
	Compiler::Assembly assembly_;

	ExecutionManager execMan;

	bool dbgFlag;

protected:
/*
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
*/
/*
	void printTrace(SymState* state) {

		vector<pair<SymState*, const CodeStorage::Insn*> > trace;

		while (state) {

			trace.push_back(make_pair(state, *CFG_FROM_FAE(*state->fae)->insn));
			state = state->parent;

		}

//		trace.push_back(make_pair(item->fae, *state->insn));

		CL_DEBUG("trace:");

		for (auto i = trace.rbegin(); i != trace.rend(); ++i) {
			if (i->first->payload) {
				CfgState* state = CFG_FROM_FAE(*i->first->fae);
				CL_DEBUG(std::endl << SymCtx::Dump(*state->ctx, *i->first->fae));
				CL_DEBUG(std::endl << *i->first->fae);
				CL_NOTE_MSG(&i->second->loc, *(i->second));
			}
//			STATE_FROM_FAE(*i->first)->ctx->dumpContext(*i->first);
//			CL_CDEBUG(std::endl << *(i->first));
		}

//		state = STATE_FROM_FAE(fae);
//		CL_CDEBUG(std::endl << SymCtx::Dump(*state->ctx, fae));
//		CL_CDEBUG(std::endl << fae);
//		CL_NOTE_MSG(this->currentInsn->loc, *this->currentInsn);

	}
*/
/*
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
*/
	void loadTypes(const CodeStorage::Storage& stor) {

	    CL_CDEBUG("loading types ...");

		for (auto type : stor.types) {
			if (type->code != cl_type_e::CL_TYPE_STRUCT)
				continue;
			std::string name;
			if (type->name)
				name = std::string(type->name);
			else {
				std::ostringstream ss;
				ss << type->uid;
				name = ss.str();
			}
				
			std::vector<size_t> v;
			NodeBuilder::buildNode(v, type);
			this->boxMan.createTypeInfo(type->name, v);
		}

	}
/*
	void printQueue() const {
		for (SymState* state : this->queue)
			std::cerr << *state->fae;
	}
*/
public:

	Engine() : boxMan(this->taBackend),
		compiler_(this->fixpointBackend, this->taBackend, this->boxMan, this->boxes),
		dbgFlag(false) {}

	void loadBoxes(const boost::unordered_map<std::string, std::string>& db) {

	    CL_DEBUG("loading boxes ...");

		for (boost::unordered_map<std::string, std::string>::const_iterator i = db.begin(); i != db.end(); ++i) {
			this->boxes.push_back((const Box*)this->boxMan.loadBox(i->first, db));
			CL_DEBUG(i->first << ':' << std::endl << *(const FA*)this->boxes.back());
		}

		this->boxMan.buildBoxHierarchy(this->hierarchy, this->basicBoxes);
		
	}

	void compile(const CodeStorage::Storage& stor) {

		CL_DEBUG("compiling ...");

		this->compiler_.compile(this->assembly_, stor);

		CL_DEBUG("assembly:" << std::endl << this->assembly_);		

		this->loadTypes(stor);

	}

	void run(const CodeStorage::Fnc& main) {

		assert(this->assembly_.code_.size());

		this->execMan.clear();

	    CL_CDEBUG("creating main context ...");
		// create main context
		SymCtx mainCtx(main);

	    CL_CDEBUG("creating empty heap ...");
		// create empty heap with no local variables
		std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(this->taBackend, this->boxMan));

	    CL_CDEBUG("allocating global registers ...");
		// add global registers
		SymCtx::init(*fae);

	    CL_CDEBUG("entering main stack frame ...");
		// enter main stack frame
		mainCtx.createStackFrame(*fae);

	    CL_CDEBUG("sheduling initial state ...");
		// schedule initial state for processing
		this->execMan.init(
			std::vector<Data>(this->assembly_.regFileSize_, Data::createUndef()), fae, this->assembly_.getEntry(&main)
		);

		try {

			AbstractInstruction::StateType state;

			while (this->execMan.dequeueDFS(state)) {

				CL_CDEBUG(state);

				this->execMan.execute(state);

			}

			for (auto instr : this->assembly_.code_) {

				if (instr->getType() != e_fi_type::fiFix)
					continue;

				CL_DEBUG("fixpoint at " << instr->insn()->loc << ":" << std::endl << ((FixpointInstruction*)instr)->getFixPoint());

			}

			CL_DEBUG("states: " << this->execMan.statesEvaluated() << ", traces: " << this->execMan.tracesEvaluated());

		} catch (std::exception& e) {
			CL_CDEBUG(e.what());
			throw;
		}
		
	}

	void setDbgFlag() {
		this->dbgFlag = 1;
	}	

};

SymExec::SymExec() : engine(new Engine()) {}

SymExec::~SymExec() {
	delete this->engine;
}

void SymExec::loadBoxes(const boost::unordered_map<std::string, std::string>& db) {
	this->engine->loadBoxes(db);
}

void SymExec::compile(const CodeStorage::Storage& stor) {
	this->engine->compile(stor);
}

void SymExec::run(const CodeStorage::Fnc& main) {
	this->engine->run(main);
}

void SymExec::setDbgFlag() {
	this->engine->setDbgFlag();
}
