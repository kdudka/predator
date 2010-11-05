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

#include "symexec.hh"

using boost::unordered_set;
using boost::unordered_map;
using std::vector;
using std::list;
/*
struct FAEContainer {

	std::vector<FAE*> conf;
	
	FAEContainer() {}
	FAEContainer(const FAE& fae) {
		this->conf.push_back(new FAE(fae));
	}

	~FAEContainer() {
		utils::erase(this->conf);
	}

	template <class F>
	void apply(F f) {
		for (std::vector<FAE*>::iterator i = this->conf.begin(); i != this->conf.end(); ++i)
			f(**i);
	}

	template <class F>
	void forAll(F f) {
		std::vector<FAE*> tmp;
		ContainerGuard<FAE*> g(tmp);
		for (std::vector<FAE*>::iterator i = this->conf.begin(); i != this->conf.end(); ++i)
			f(tmp, *((const FAE*)*i));
		std::swap(this->conf, tmp);
	}
	
	template <class F>
	void forAll(FAEContainer& dst, F f) {
		for (std::vector<FAE*>::iterator i = this->conf.begin(); i != this->conf.end(); ++i)
			f(dst.conf, *((const FAE*)*i));
	}

protected:

	FAEContainer(const FAEContainer&) {}

};
*/
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

void dumpLocation(const cl_location* loc) {
	if (loc->file)
		CL_DEBUG(loc->file << ':' << loc->line << ": ");
	else
		CL_DEBUG("<unknown location>: ");
}

struct SymOp {

	SymState* src;
	SymState* dst;

};


struct TraceRecorder {

	struct Item {

		Item* parent;
		const FAE* fae;
		const CodeStorage::Insn* insn;
		vector<Item*> children;

		Item(Item* parent, const FAE* fae, const CodeStorage::Insn* insn)
			: parent(parent), fae(fae), insn(insn) {
			if (parent)
				parent->children.push_back(this);
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
		Item* item = new Item(NULL, fae, NULL);
		this->confMap.insert(make_pair(fae, item));
	}

	Item* find(const FAE* fae) {
		unordered_map<const FAE*, Item*>::iterator i = this->confMap.find(fae);
		assert(i != this->confMap.end());
		return i->second;
	}

	void add(const FAE* parent, const FAE* fae, const CodeStorage::Insn* insn) {
		this->confMap.insert(
			make_pair(fae, new Item(this->find(parent), fae, insn))
		);
	}
	
};

typedef enum { biNone, biMalloc, biFree, biNondet } builtin_e;

struct BuiltinTable {

	unordered_map<string, builtin_e> _table;

public:

	BuiltinTable() {
		this->_table["malloc"] = builtin_e::biMalloc;
		this->_table["free"] = builtin_e::biFree;
		this->_table["__nondet"] = builtin_e::biNondet;
	}

	builtin_e operator[](const string& key) {
		unordered_map<string, builtin_e>::iterator i = this->_table.find(key);
		return (i == this->_table.end())?(builtin_e::biNone):(i->second);
	}

};

#define STATE_FROM_FAE(fae) ((SymState*)(assert((fae).varGet(IP_INDEX).isNativePtr()), (fae).varGet(IP_INDEX).d_native_ptr))

class SymExec::Engine {

	static BuiltinTable builtins;

	const CodeStorage::Storage& stor;

	LoopAnalyser loopAnalyser;

	TA<label_type>::Backend taBackend;
	TA<label_type>::Manager taMan;
	LabMan labMan;
	BoxManager boxMan;

	typedef unordered_map<const CodeStorage::Fnc*, SymCtx*> ctx_store_type;
	ctx_store_type ctxStore;

	typedef unordered_map<const CodeStorage::Insn*, SymState*> state_store_type;
	state_store_type stateStore;

	std::list<const FAE*> queue;

	const FAE* currentConf;
	const CodeStorage::Insn* currentInsn;
	
	TraceRecorder traceRecorder;

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

	void stateUnion(SymState* target, FAE& fae) {

		fae.varSet(IP_INDEX, Data::createNativePtr((void*)target));

		FAE::NormInfo normInfo;

		std::vector<size_t> tmp;

		fae.getNearbyReferences(fae.varGet(ABP_INDEX).d_ref.root, tmp);
		fae.normalize(normInfo, tmp);

		if (target->entryPoint)
			fae.heightAbstraction(1);

//		CL_DEBUG("after abstraction: " << std::endl << fae);

		size_t l = this->queue.size();

		if (target->enqueue(this->queue, fae)) {
			int i = this->queue.size() - l;
			for (std::list<const FAE*>::reverse_iterator j = this->queue.rbegin(); i > 0; --i, ++j)
				this->traceRecorder.add(this->currentConf, *j, this->currentInsn);
		}
		else CL_DEBUG("hit");

	}

	void enqueueNextInsn(SymState* state, FAE& src) {

		state->finalizeOperands(src);

		this->stateUnion(this->getState(state->insn + 1, state->ctx), src);
		
	}

	Data readData(const FAE& fae, const OperandInfo& oi, const vector<size_t>& offs) {
		Data data;
		switch (oi.flag) {
			case o_flag_e::ref:
			case o_flag_e::safe_ref:
				if (offs.size() > 1)
					fae.nodeLookupMultiple(oi.data.d_ref.root, oi.data.d_ref.displ, offs, data);
				else
					fae.nodeLookup(oi.data.d_ref.root, oi.data.d_ref.displ, data);
				break;
			case o_flag_e::reg:
				if (offs.size() > 1)
					data = OperandInfo::extractNestedStruct(fae.varGet(oi.data.d_ref.root), oi.data.d_ref.displ, offs);
				else
					data = fae.varGet(oi.data.d_ref.root);
				break;
			case o_flag_e::val:
				data = oi.data;
				break;
			default:
				assert(false);
		}
		CL_DEBUG("read: " << oi << " -> " << data);
		return data;
	}

	void writeData(FAE& fae, const OperandInfo& oi, const Data& in, Data& out) {
		CL_DEBUG("write: " << in << " -> " << oi);
		switch (oi.flag) {
			case o_flag_e::ref:
			case o_flag_e::safe_ref:
				if (in.isStruct())
					fae.nodeModifyMultiple(oi.data.d_ref.root, oi.data.d_ref.displ, in, out);
				else
					fae.nodeModify(oi.data.d_ref.root, oi.data.d_ref.displ, in, out);
				break;
			case o_flag_e::reg:
				out = fae.varGet(oi.data.d_ref.root);
				if (in.isStruct()) {
					Data tmp = out;
					OperandInfo::modifyNestedStruct(tmp, oi.data.d_ref.displ, in);
					fae.varSet(oi.data.d_ref.root, tmp);
				} else {
					fae.varSet(oi.data.d_ref.root, in);
				}
				break;
			default:
				assert(false);
		}
	}

	void execAssignment(SymState* state, FAE& fae, const CodeStorage::Insn* insn) {

		OperandInfo dst, src;
		state->ctx->parseOperand(dst, fae, &insn->operands[0]);
		state->ctx->parseOperand(src, fae, &insn->operands[1]);

		assert(src.type->code == dst.type->code);

		Data dataOut;

		if (
			src.type->code == cl_type_e::CL_TYPE_PTR &&
			src.type->items[0].type->code == cl_type_e::CL_TYPE_VOID &&
			dst.type->items[0].type->code != cl_type_e::CL_TYPE_VOID
		) {
			Data data = this->readData(fae, src, itov((size_t)0));
			assert(data.isVoidPtr());
			if (dst.type->items[0].type->size != (int)data.d_void_ptr)
				throw runtime_error("Engine::execAssignment(): size of allocated block doesn't correspond to the size of the destination!");
			vector<SelData> sels;
			NodeBuilder::buildNode(sels, dst.type->items[0].type);
			this->writeData(fae, dst, Data::createRef(fae.nodeCreate(sels)), dataOut);
		} else {
			assert(*(src.type) == *(dst.type));
			vector<size_t> offs;
			NodeBuilder::buildNode(offs, src.type);
			this->writeData(fae, dst, this->readData(fae, src, offs), dataOut);
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

		Data data1 = this->readData(fae, src1, offs1);
		Data data2 = this->readData(fae, src2, offs2);
		vector<Data> res;
		Engine::dataEq(data1, data2, neg, res);
		Data dataOut;
		for (vector<Data>::iterator j = res.begin(); j != res.end(); ++j) {
			FAE tmp(fae);
			this->writeData(tmp, dst, *j, dataOut);
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

		Data data1 = this->readData(fae, src1, offs1);
		Data data2 = this->readData(fae, src2, offs2);
		assert(data1.isInt() && data2.isInt());
		Data res = Data::createInt((data1.d_int + data2.d_int > 0)?(1):(0)), dataOut;
		this->writeData(fae, dst, res, dataOut);

		this->enqueueNextInsn(state, fae);
		
	}

	void execMalloc(SymState* state, FAE& fae, const CodeStorage::Insn* insn) {

		OperandInfo dst, src;
		state->ctx->parseOperand(dst, fae, &insn->operands[0]);
		state->ctx->parseOperand(src, fae, &insn->operands[2]);
		assert(src.type->code == cl_type_e::CL_TYPE_INT);

		Data data = this->readData(fae, src, itov((size_t)0)), out;
		assert(data.isInt());
		this->writeData(fae, dst, Data::createVoidPtr(data.d_int), out);

		this->enqueueNextInsn(state, fae);
	
	}

	void execFree(SymState* state, FAE& fae, const CodeStorage::Insn* insn) {

		OperandInfo src;
		state->ctx->parseOperand(src, fae, &insn->operands[2]);
		Data data = this->readData(fae, src, itov((size_t)0));
		if (!data.isRef()) {
			std::stringstream ss;
			ss << "Engine::execFree(): attempt to release an unsiutable value - " << data << '!';
			throw runtime_error(ss.str());
		}
		if (data.d_ref.displ != 0) {
			std::stringstream ss;
			ss << "Engine::execFree(): attempt to release a reference not pointing at the beginning of an allocated block - " << data << '!';
			throw runtime_error(ss.str());
		}
		vector<FAE*> tmp;
		ContainerGuard<vector<FAE*> > g(tmp);
		fae.isolateAtRoot(tmp, data.d_ref.root, FAE::IsolateAllF());
		for (vector<FAE*>::iterator j = tmp.begin(); j != tmp.end(); ++j) {
			(*j)->nodeDelete(data.d_ref.root);
			this->enqueueNextInsn(state, **j);
		}
		
	}

	void execNondet(SymState* state, FAE& fae, const CodeStorage::Insn* insn) {

		OperandInfo dst;
		state->ctx->parseOperand(dst, fae, &insn->operands[0]);

		Data out;
		this->writeData(fae, dst, Data::createUnknw(), out);
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

		Data data = this->readData(fae, src, itov((size_t)0));

		if (!data.isBool())
			throw runtime_error("Engine::execCond(): non boolean condition argument!");

		state->finalizeOperands(fae);

		this->stateUnion(this->getState(insn->targets[((data.d_bool))?(0):(1)]->begin(), state->ctx), fae);

	}


	void execRet(SymState* state, FAE& fae, const CodeStorage::Insn* insn) {

		vector<FAE*> tmp;
		ContainerGuard<vector<FAE*> > g(tmp);

		assert(!state->ctx->destroyStackFrame(fae));

		for (vector<FAE*>::iterator i = tmp.begin(); i != tmp.end(); ++i)
			(*i)->check();
		
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
						assert(false);
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
						assert(false);
				}
				break;

			case cl_insn_e::CL_INSN_CALL:
				assert(insn->operands[1].code == cl_operand_e::CL_OPERAND_CST);
				assert(insn->operands[1].data.cst.code == cl_type_e::CL_TYPE_FNC);
				switch (Engine::builtins[insn->operands[1].data.cst.data.cst_fnc.name]) {
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
						assert(false);
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
				assert(false);

		}
				
	}

	void processState(const FAE* fae) {

		assert(fae);

		SymState* state = STATE_FROM_FAE(*fae);

		this->currentInsn = *state->insn;

		state->confMap[fae] = this->queue.end();

		const cl_location* loc = &(*state->insn)->loc;

		CL_DEBUG("processing " << fae);

		state->ctx->dumpContext(*fae);
		
		CL_DEBUG(std::endl << *fae);

		dumpLocation(loc);

		try {

			FAE tmp(*fae);

			this->execInsn(state, tmp);

		} catch (const std::exception& e) {

			CL_DEBUG("reconstructing abstract trace ...");

			vector<pair<const FAE*, const CodeStorage::Insn*> > trace;

			const TraceRecorder::Item* item = this->traceRecorder.find(this->currentConf);
			
			while (item) {
				trace.push_back(make_pair(item->fae, item->insn));
				item = item->parent;
			}

			CL_DEBUG("trace:");

			for (vector<pair<const FAE*, const CodeStorage::Insn*> >::reverse_iterator i = trace.rbegin(); i != trace.rend(); ++i) {
				STATE_FROM_FAE(*i->first)->ctx->dumpContext(*i->first);
				if (i->second)
					CL_DEBUG(*(i->second));
				CL_DEBUG(std::endl << *(i->first));
			}

			CL_DEBUG(*this->currentInsn);

			throw;

		}

	}

public:

	Engine(const CodeStorage::Storage& stor)
		: stor(stor), taMan(this->taBackend), boxMan(this->taMan, this->labMan) {}

	~Engine() {
		utils::eraseMap(this->stateStore);
		utils::eraseMap(this->ctxStore);
	}

	void run(const CodeStorage::Fnc& main) {

	    CL_DEBUG("calculating loop entry points ...");
		// compute loop entry points
		this->loopAnalyser.init(main.cfg.entry());
		
	    CL_DEBUG("creating main context ...");
		// create main context
		SymCtx* mainCtx = this->getCtx(&main);

	    CL_DEBUG("creating initial state ...");
		// create an initial state
		SymState* init = this->getState(main.cfg.entry()->begin(), mainCtx);

	    CL_DEBUG("creating empty heap ...");
		// create empty heap with no local variables
		FAE fae(this->taMan, this->labMan, this->boxMan);

	    CL_DEBUG("allocating global registers ...");
		// add global registers
		SymCtx::init(fae);

	    CL_DEBUG("entering main stack frame ...");
		// enter main stack frame
		mainCtx->createStackFrame(fae, init);
/*
	    CL_DEBUG("initial state dump ...");
		for (std::vector<FAE*>::iterator i = init->outConf.begin(); i != init->outConf.end(); ++i) {
			CL_DEBUG(std::endl << **i);
			this->confBuffer.push_back(new FAE(**i));
			this->confDict[*i] = this->confBuffer.back();
		}
*/
	    CL_DEBUG("sheduling initial state ...");
		// schedule initial state for processing
//		this->todo.push_back(init);
		init->enqueue(this->queue, fae);

		this->traceRecorder.init(this->queue.front());

		try {

			while (!this->queue.empty()) {
				this->currentConf = this->queue.front();
				this->queue.pop_front();
				this->processState(this->currentConf);
			}

		} catch (std::exception& e) {
			CL_DEBUG(e.what());
			throw;
		}
		
	}

};

BuiltinTable SymExec::Engine::builtins;

SymExec::SymExec(const CodeStorage::Storage &stor)
	: engine(new Engine(stor)) {}

SymExec::~SymExec() {
	delete this->engine;
}

void SymExec::run(const CodeStorage::Fnc& main) {
	this->engine->run(main);
}
