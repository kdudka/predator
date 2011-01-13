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

#include "treeaut.hh"
#include "forestaut.hh"
#include "forestautext.hh"
#include "ufae.hh"
#include "operandinfo.hh"
#include "symctx.hh"
#include "symstate.hh"
#include "loopanalyser.hh"
#include "builtintable.hh"

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

std::ostream& operator<<(std::ostream& os, const cl_location& loc) {
	if (loc.file)
		return os << loc.file << ':' << loc.line << ':';
	else
		return os << "<unknown location>:";
}
/*
struct SymOp {

	SymState* src;
	SymState* dst;

};
*/
#define STATE_FROM_FAE(fae) ((SymState*)(assert((fae).varGet(IP_INDEX).isNativePtr()), (fae).varGet(IP_INDEX).d_native_ptr))

struct TraceRecorder {

	struct Item {

		Item* parent;
		const FAE* fae;
		FAE normalized;
		FAE::NormInfo normInfo;
		vector<Item*> children;

		Item(Item* parent, const FAE* fae, const FAE& normalized, const FAE::NormInfo& normInfo)
			: parent(parent), fae(fae), normalized(normalized), normInfo(normInfo) {
			if (parent)
				parent->children.push_back(this);
		}

		void removeChild(Item* item) {
			this->children.erase(std::find(this->children.begin(), this->children.end(), item));
		}

	};

	unordered_map<const FAE*, Item*> confMap;

	TraceRecorder() {
	}

	~TraceRecorder() { this->clear(); }

	void clear() {
		utils::eraseMap(this->confMap);
	}

	void init(const FAE* fae) {
		this->clear();
		Item* item = new Item(NULL, fae, FAE(*fae), FAE::NormInfo());
		this->confMap.insert(make_pair(fae, item));
	}

	Item* find(const FAE* fae) {
		unordered_map<const FAE*, Item*>::iterator i = this->confMap.find(fae);
		assert(i != this->confMap.end());
		return i->second;
	}

	void add(const FAE* parent, const FAE* fae, const FAE& normalized, const FAE::NormInfo& normInfo) {
		this->confMap.insert(
			make_pair(fae, new Item(this->find(parent), fae, normalized, normInfo))
		);
	}

	void remove(const FAE* fae) {
		unordered_map<const FAE*, Item*>::iterator i = this->confMap.find(fae);
		assert(i != this->confMap.end());
		delete i->second;
		this->confMap.erase(i);
	}

	template <class F>
	void invalidate(TraceRecorder::Item* node, F f) {

		for (vector<Item*>::iterator i = node->children.begin(); i != node->children.end(); ++i)
			this->invalidate(*i, f);

		const FAE* fae = node->fae;

		this->remove(fae);

		f(fae);

	}

	template <class F>
	void invalidateChildren(TraceRecorder::Item* node, F f) {

		for (vector<Item*>::iterator i = node->children.begin(); i != node->children.end(); ++i)
			this->invalidate(*i, f);

		node->children.clear();
	
	}

};

class SymExec::Engine {

	const CodeStorage::Storage& stor;

	LoopAnalyser loopAnalyser;

	TA<label_type>::Backend taBackend;
	TA<label_type>::Manager taMan;
	LabMan labMan;
	BoxMan boxMan;

	typedef unordered_map<const CodeStorage::Fnc*, SymCtx*> ctx_store_type;
	ctx_store_type ctxStore;

	typedef unordered_map<const CodeStorage::Insn*, SymState*> state_store_type;
	state_store_type stateStore;

	std::list<const FAE*> queue;

	const FAE* currentConf;
	const CodeStorage::Insn* currentInsn;
	
	TraceRecorder traceRecorder;

	std::vector<const Box*> boxes;

protected:

	SymCtx* getCtx(const CodeStorage::Fnc* fnc) {

		ctx_store_type::iterator i = this->ctxStore.find(fnc);
		if (i != this->ctxStore.end())
			return i->second;

		return this->ctxStore.insert(make_pair(fnc, new SymCtx(*fnc))).first->second;

	}

	SymState* findState(const CodeStorage::Insn* insn) {

		state_store_type::iterator i = this->stateStore.find(insn);
		assert(i != this->stateStore.end());
		return i->second;

	}

	SymState* getState(const CodeStorage::Block::const_iterator& insn, const SymCtx* ctx) {

		state_store_type::iterator i = this->stateStore.find(*insn);
		if (i != this->stateStore.end())
			return i->second;

		SymState* s = new SymState(this->taBackend, this->labMan);
		s->insn = insn;
		s->ctx = ctx;
		s->entryPoint = this->loopAnalyser.isEntryPoint(*insn);
		
		return this->stateStore.insert(make_pair(*insn, s)).first->second;

	}

	bool foldStructures(FAE& fae, const std::set<size_t>& forbidden) {

		// do not fold at 0
		for (size_t i = 1; i < fae.getRootCount(); ++i) {
			if (forbidden.count(i))
				continue;
			for (std::vector<const Box*>::const_iterator j = this->boxes.begin(); j != this->boxes.end(); ++j) {
				CL_CDEBUG("trying " << *(const AbstractBox*)(*j) << " at " << i);
				if (fae.foldBox(i, *j))
					return true;
			}
		}

		return false;

	}

	void stateUnion(SymState* target, FAE& fae) {

		fae.varSet(IP_INDEX, Data::createNativePtr((void*)target));

		std::set<size_t> tmp;
		FAE::NormInfo normInfo;
		fae.getNearbyReferences(fae.varGet(ABP_INDEX).d_ref.root, tmp);

		fae.normalize(normInfo, tmp);

		if (target->entryPoint) {
			CL_CDEBUG(std::endl << fae);
			std::set<size_t> tmp2;
			tmp2.insert(0);
			while (Engine::foldStructures(fae, tmp2)) {
//				CL_CDEBUG(std::endl << fae);
				normInfo.clear();
				fae.normalize(normInfo, tmp);
				tmp.clear();
				fae.getNearbyReferences(fae.varGet(ABP_INDEX).d_ref.root, tmp);
			}
		}

		FAE normalized(fae);

		if (target->entryPoint) {
			CL_CDEBUG("abstracting ... " << target->absHeight);
			fae.heightAbstraction(target->absHeight);
		}

//		CL_CDEBUG("after abstraction: " << std::endl << fae);

		size_t l = this->queue.size();

		if (target->enqueue(this->queue, fae)) {
			int i = this->queue.size() - l;
			for (std::list<const FAE*>::reverse_iterator j = this->queue.rbegin(); i > 0; --i, ++j)
				this->traceRecorder.add(this->currentConf, *j, normalized, normInfo);
		}
		else { CL_CDEBUG("hit"); }

	}

	void enqueueNextInsn(SymState* state, FAE& src) {

		state->finalizeOperands(src);

		this->stateUnion(this->getState(state->insn + 1, state->ctx), src);
		
	}

	void execAssignment(SymState* state, FAE& fae, const CodeStorage::Insn* insn) {

		OperandInfo dst, src;
		state->ctx->parseOperand(dst, fae, &insn->operands[0]);
		state->ctx->parseOperand(src, fae, &insn->operands[1]);

		assert(src.type->code == dst.type->code);

		RevInfo rev;

		if (
			src.type->code == cl_type_e::CL_TYPE_PTR &&
			src.type->items[0].type->code == cl_type_e::CL_TYPE_VOID &&
			dst.type->items[0].type->code != cl_type_e::CL_TYPE_VOID
		) {
			Data data = src.readData(fae, itov((size_t)0));
			assert(data.isVoidPtr());
			if (dst.type->items[0].type->size != (int)data.d_void_ptr)
				throw ProgramError("allocated block's size mismatch");
			vector<SelData> sels;
			NodeBuilder::buildNode(sels, dst.type->items[0].type);
			dst.writeData(
				fae,
				Data::createRef(
					fae.nodeCreate(
						sels,
						this->boxMan.getInfo(
							dst.type->items[0].type->name,
							(void*)dst.type->items[0].type
						)
					)
				),
				rev
			);
		} else {
			assert(*(src.type) == *(dst.type));
			vector<size_t> offs;
			NodeBuilder::buildNode(offs, src.type);
			dst.writeData(fae, src.readData(fae, offs), rev);
		}

		this->enqueueNextInsn(state, fae);
		
	}

	static void dataEq(const Data& x, const Data& y, bool neg, vector<Data>& res) {
		if ((x.isUnknw() || x.isUndef()) || (y.isUnknw() || y.isUndef())) {
			res.push_back(Data::createBool(false));
			res.push_back(Data::createBool(true));
		} else
			res.push_back(Data::createBool((x == y) != neg));
	}

	void execEq(SymState* state, const FAE& fae, const CodeStorage::Insn* insn, bool neg) {

		OperandInfo dst, src1, src2;
		state->ctx->parseOperand(dst, fae, &insn->operands[0]);
		state->ctx->parseOperand(src1, fae, &insn->operands[1]);
		state->ctx->parseOperand(src2, fae, &insn->operands[2]);

		assert(*src1.type == *src2.type);
		assert(OperandInfo::isLValue(dst.flag));
		assert(dst.type->code == cl_type_e::CL_TYPE_BOOL);

		vector<size_t> offs1;
		NodeBuilder::buildNode(offs1, src1.type);

		vector<size_t> offs2;
		NodeBuilder::buildNode(offs2, src2.type);

		Data data1 = src1.readData(fae, offs1);
		Data data2 = src2.readData(fae, offs2);
		vector<Data> res;
		Engine::dataEq(data1, data2, neg, res);
		RevInfo rev;
		for (vector<Data>::iterator j = res.begin(); j != res.end(); ++j) {
			FAE tmp(fae);
			dst.writeData(tmp, *j, rev);
			this->enqueueNextInsn(state, tmp);
		}

	}

	void execPlus(SymState* state, FAE& fae, const CodeStorage::Insn* insn) {

		OperandInfo dst, src1, src2;
		state->ctx->parseOperand(dst, fae, &insn->operands[0]);
		state->ctx->parseOperand(src1, fae, &insn->operands[1]);
		state->ctx->parseOperand(src2, fae, &insn->operands[2]);

		assert(dst.type->code == cl_type_e::CL_TYPE_INT);
		assert(src1.type->code == cl_type_e::CL_TYPE_INT);
		assert(src2.type->code == cl_type_e::CL_TYPE_INT);

		vector<size_t> offs1, offs2;
		NodeBuilder::buildNode(offs1, src1.type);
		NodeBuilder::buildNode(offs2, src2.type);

		Data data1 = src1.readData(fae, offs1);
		Data data2 = src2.readData(fae, offs2);
		assert(data1.isInt() && data2.isInt());
		Data res = Data::createInt((data1.d_int + data2.d_int > 0)?(1):(0));
		RevInfo rev;
		dst.writeData(fae, res, rev);

		this->enqueueNextInsn(state, fae);
		
	}

	void execMalloc(SymState* state, FAE& fae, const CodeStorage::Insn* insn) {

		OperandInfo dst, src;
		state->ctx->parseOperand(dst, fae, &insn->operands[0]);
		state->ctx->parseOperand(src, fae, &insn->operands[2]);
		assert(src.type->code == cl_type_e::CL_TYPE_INT);

		Data data = src.readData(fae, itov((size_t)0));
		assert(data.isInt());
		RevInfo rev;
		dst.writeData(fae, Data::createVoidPtr(data.d_int), rev);

		this->enqueueNextInsn(state, fae);
	
	}

	void execFree(SymState* state, FAE& fae, const CodeStorage::Insn* insn) {

		OperandInfo src;
		state->ctx->parseOperand(src, fae, &insn->operands[2]);
		Data data = src.readData(fae, itov((size_t)0));
		if (!data.isRef())
			throw ProgramError("releasing non-pointer value");
		if (data.d_ref.displ != 0)
			throw ProgramError("releasing a pointer which points inside the block");
		fae.nodeDelete(data.d_ref.root);
		this->enqueueNextInsn(state, fae);
		
	}

	void execNondet(SymState* state, FAE& fae, const CodeStorage::Insn* insn) {

		OperandInfo dst;
		state->ctx->parseOperand(dst, fae, &insn->operands[0]);

		RevInfo rev;
		dst.writeData(fae, Data::createUnknw(), rev);
		this->enqueueNextInsn(state, fae);

	}

	void execJmp(SymState* state, FAE& fae, const CodeStorage::Insn* insn) {

		state->finalizeOperands(fae);

		this->stateUnion(this->getState(insn->targets[0]->begin(), state->ctx), fae);

	}

	void execCond(SymState* state, FAE& fae, const CodeStorage::Insn* insn) {

		OperandInfo src;
		state->ctx->parseOperand(src, fae, &insn->operands[0]);

		assert(src.type->code == cl_type_e::CL_TYPE_BOOL);

		Data data = src.readData(fae, itov((size_t)0));

		if (!data.isBool())
			throw runtime_error("Engine::execCond(): non boolean condition argument!");

		state->finalizeOperands(fae);

		this->stateUnion(this->getState(insn->targets[((data.d_bool))?(0):(1)]->begin(), state->ctx), fae);

	}

	// TODO: implement proper return
	void execRet(SymState* state, FAE& fae, const CodeStorage::Insn* insn) {

		bool b = state->ctx->destroyStackFrame(fae);
		assert(!b);

		fae.check();
		
	}

	void execInsn(SymState* state, FAE& fae) {

		const CodeStorage::Insn* insn = *state->insn;

		switch (insn->code) {

			case cl_insn_e::CL_INSN_UNOP:
				switch (insn->subCode) {
					case cl_unop_e::CL_UNOP_ASSIGN:
						this->execAssignment(state, fae, insn);
						break;
					default:
						throw std::runtime_error("feature not implemented");
				}
				break;

			case cl_insn_e::CL_INSN_BINOP:
				switch (insn->subCode) {
					case cl_binop_e::CL_BINOP_EQ:
						this->execEq(state, fae, insn, false);
						break;
					case cl_binop_e::CL_BINOP_NE:
						this->execEq(state, fae, insn, true);
						break;
					case cl_binop_e::CL_BINOP_PLUS:
						this->execPlus(state, fae, insn);
						break;
					default:
						throw std::runtime_error("feature not implemented");
				}
				break;

			case cl_insn_e::CL_INSN_CALL:
				assert(insn->operands[1].code == cl_operand_e::CL_OPERAND_CST);
				assert(insn->operands[1].data.cst.code == cl_type_e::CL_TYPE_FNC);
				switch (BuiltinTableStatic::data[insn->operands[1].data.cst.data.cst_fnc.name]) {
					case builtin_e::biMalloc:
						this->execMalloc(state, fae, insn);
						break;
					case builtin_e::biFree:
						this->execFree(state, fae, insn);
						break;
					case builtin_e::biNondet:
						this->execNondet(state, fae, insn);
						break;
					default:
						throw std::runtime_error("feature not implemented");
				}
				break;

			case cl_insn_e::CL_INSN_RET:
				this->execRet(state, fae, insn);
				break;

			case cl_insn_e::CL_INSN_JMP:
				this->execJmp(state, fae, insn);
				break;

			case cl_insn_e::CL_INSN_COND:
				this->execCond(state, fae, insn);
				break;

			default:
				throw std::runtime_error("feature not implemented");

		}
				
	}

	struct InvalidateF {

		list<const FAE*>& queue;
		set<SymState*>& s;
		
		InvalidateF(list<const FAE*>& queue, set<SymState*>& s) : queue(queue), s(s) {}

		void operator()(const FAE* fae) {
			SymState* state = STATE_FROM_FAE(*fae);
			state->invalidate(this->queue, fae);
			if (state->entryPoint)
				s.insert(state);
		}

	};

	void processState(const FAE* fae) {

		assert(fae);

		SymState* state = STATE_FROM_FAE(*fae);

		this->currentInsn = *state->insn;

		state->confMap[fae] = this->queue.end();

		const cl_location& loc = (*state->insn)->loc;

		CL_CDEBUG("processing " << fae);
		CL_CDEBUG(std::endl << SymCtx::Dump(*state->ctx, *fae));
		CL_CDEBUG(std::endl << *fae);
		CL_CDEBUG(loc << ' ' << **state->insn);

		try {

			FAE tmp(*fae);

			this->execInsn(state, tmp);

		} catch (const ProgramError& e) {

//			throw;

			CL_CDEBUG(e.what());

			TraceRecorder::Item* item = this->revRun(*fae);

			if (!item)

				throw ProgramError(e.what(), &(*state->insn)->loc);

			state = STATE_FROM_FAE(*item->fae);

			assert(state->entryPoint);

			set<SymState*> s;

			this->traceRecorder.invalidateChildren(item, InvalidateF(this->queue, s));

			const FAE* tmp2 = item->fae;
			
			TraceRecorder::Item* parent = item->parent;

			this->traceRecorder.remove(tmp2);

			InvalidateF(this->queue, s)(tmp2);

			assert(parent);

			parent->removeChild(item);

			for (set<SymState*>::iterator i = s.begin(); i != s.end(); ++i) {
				(*i)->recompute(this->queue);
				CL_CDEBUG("new fixpoint:" << std::endl << (*i)->fwdConf);
			}

			CL_CDEBUG("adjusting abstraction ... " << ++state->absHeight);
			CL_CDEBUG("resuming execution ... ");
			CL_CDEBUG(loc << ' ' << **state->insn);

			STATE_FROM_FAE(*parent->fae)->enqueue(this->queue, itov((FAE*)parent->fae));

		}

	}

	TraceRecorder::Item* revRun(const FAE& fae) {

//		CL_CDEBUG("reconstructing abstract trace ...");

		vector<pair<const FAE*, const CodeStorage::Insn*> > trace;

		TraceRecorder::Item* item = this->traceRecorder.find(&fae);

		FAE tmp(fae);

		SymState* state = NULL;
		
		while (item->parent) {

			CL_CDEBUG(std::endl << SymCtx::Dump(*STATE_FROM_FAE(*item->fae)->ctx, *item->fae));
			CL_CDEBUG(std::endl << tmp);

			state = STATE_FROM_FAE(*item->parent->fae);

			CL_CDEBUG("rewinding " << (*state->insn)->loc << ' ' << **state->insn);
			
			FAE::NormInfo normInfo;

			std::set<size_t> s;
			tmp.getNearbyReferences(fae.varGet(ABP_INDEX).d_ref.root, s);
			tmp.normalize(normInfo, s);

//			CL_CDEBUG("denormalizing " << std::endl << tmp << "with" << std::endl << item->normalized);
//			CL_CDEBUG(item->normInfo);

			if (!tmp.denormalize(item->normalized, item->normInfo)) {
				CL_CDEBUG("spurious counter example (denormalization)!" << std::endl << item->normalized);
				return item;
			}

//			CL_CDEBUG("reversing " << std::endl << tmp << "with" << std::endl << *item->parent->fae);

			if (!tmp.reverse(*item->parent->fae)) {
				CL_CDEBUG("spurious counter example (reversal)!" << std::endl << *item->parent->fae);
				return item;
			}

			trace.push_back(make_pair(item->fae, *state->insn));

			item = item->parent;

		}

		assert(state);

//		trace.push_back(make_pair(item->fae, *state->insn));

		CL_CDEBUG("trace:");

		for (vector<pair<const FAE*, const CodeStorage::Insn*> >::reverse_iterator i = trace.rbegin(); i != trace.rend(); ++i) {
			if (i->second)
				CL_NOTE_MSG(i->second->loc, *(i->second));
//			STATE_FROM_FAE(*i->first)->ctx->dumpContext(*i->first);
//			CL_CDEBUG(std::endl << *(i->first));
		}

		CL_NOTE_MSG(this->currentInsn->loc, *this->currentInsn);

		return NULL;

	}

public:

	Engine(const CodeStorage::Storage& stor)
		: stor(stor), taMan(this->taBackend), boxMan(this->taMan, this->labMan) {
	}

	~Engine() {
		utils::eraseMap(this->stateStore);
		utils::eraseMap(this->ctxStore);
	}

	void loadBoxes(const boost::unordered_map<std::string, std::string>& db) {

	    CL_CDEBUG("loading boxes ...");

		for (boost::unordered_map<std::string, std::string>::const_iterator i = db.begin(); i != db.end(); ++i) {
			this->boxes.push_back((const Box*)this->boxMan.loadBox(i->first, db));
			CL_CDEBUG(i->first << ':' << std::endl << *(const FA*)this->boxes.back());
		}
		
	}

	void run(const CodeStorage::Fnc& main) {

	    CL_CDEBUG("calculating loop entry points ...");
		// compute loop entry points
		this->loopAnalyser.init(main.cfg.entry());
		
	    CL_CDEBUG("creating main context ...");
		// create main context
		SymCtx* mainCtx = this->getCtx(&main);

	    CL_CDEBUG("creating initial state ...");
		// create an initial state
		SymState* init = this->getState(main.cfg.entry()->begin(), mainCtx);

	    CL_CDEBUG("creating empty heap ...");
		// create empty heap with no local variables
		FAE fae(this->taMan, this->labMan, this->boxMan);

	    CL_CDEBUG("allocating global registers ...");
		// add global registers
		SymCtx::init(fae);

	    CL_CDEBUG("entering main stack frame ...");
		// enter main stack frame
		mainCtx->createStackFrame(fae, init);

	    CL_CDEBUG("sheduling initial state ...");
		// schedule initial state for processing
		init->enqueue(this->queue, fae);

		this->traceRecorder.init(this->queue.front());

		try {

			while (!this->queue.empty()) {
				this->currentConf = this->queue.front();
				this->queue.pop_front();
				this->processState(this->currentConf);
			}

			for (state_store_type::iterator i = this->stateStore.begin(); i != this->stateStore.end(); ++i) {
				if (!i->second->entryPoint)
					continue;
				CL_DEBUG("fixpoint at " << (*i->second->insn)->loc);
				CL_DEBUG(std::endl << i->second->fwdConf);
			}				

		} catch (std::exception& e) {
			CL_CDEBUG(e.what());
			throw;
		}
		
	}

};

SymExec::SymExec(const CodeStorage::Storage &stor)
	: engine(new Engine(stor)) {}

SymExec::~SymExec() {
	delete this->engine;
}

void SymExec::loadBoxes(const boost::unordered_map<std::string, std::string>& db) {
	this->engine->loadBoxes(db);
}


void SymExec::run(const CodeStorage::Fnc& main) {
	this->engine->run(main);
}
