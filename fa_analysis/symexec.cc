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

#include <vector>
#include <boost/unordered_map.hpp>

#include <cl/code_listener.h>
#include <cl/cl_msg.hh>
#include <cl/clutil.hh>
#include <cl/storage.hh>

#include "treeaut.hh"
#include "forestaut.hh"
#include "forestautext.hh"
#include "ufae.hh"
#include "symexec.hh"

#define ABP_INDEX		0
#define ABP_OFFSET		0
#define CTX_INDEX		1
#define CTX_OFFSET		sizeof(void*)
#define FIXED_REG_COUNT	2

using boost::unordered_map;
using std::vector;

struct NodeBuilder {

	static void buildNode(vector<SelData>& nodeInfo, const cl_type* type, int offset = 0) {
		
		assert(type->size > 0);

		switch (type->code) {
			case cl_type_e::CL_TYPE_STRUCT:
				for (int i = 0; i < type->item_cnt; ++i)
					NodeBuilder::buildNode(nodeInfo, type->items[i].type, offset + type->items[i].offset);
				break;
			case cl_type_e::CL_TYPE_PTR:
			case cl_type_e::CL_TYPE_INT:
			case cl_type_e::CL_TYPE_BOOL:
			default:
				nodeInfo.push_back(SelData(offset, type->size, 0));
				break;
//			case cl_type_e::CL_TYPE_UNION:
		}

	}

	static void buildNode(vector<size_t>& nodeInfo, const cl_type* type, int offset = 0) {
		
		assert(type->size > 0);

		switch (type->code) {
			case cl_type_e::CL_TYPE_STRUCT:
				for (int i = 0; i < type->item_cnt; ++i)
					NodeBuilder::buildNode(nodeInfo, type->items[i].type, offset + type->items[i].offset);
				break;
			case cl_type_e::CL_TYPE_PTR:
			case cl_type_e::CL_TYPE_INT:
			case cl_type_e::CL_TYPE_BOOL:
			default:
				nodeInfo.push_back(offset);
				break;
//			case cl_type_e::CL_TYPE_UNION:
		}

	}

};

struct OperandInfo {

	bool deref;
	Data data;
	const cl_type* type;

	const cl_accessor* parseItems(const cl_accessor* acc) {

		while (acc && (acc->code == CL_ACCESSOR_ITEM)) {
			this->data.d_ref.displ += acc->type->items[acc->data.item.id].offset;
			this->type = acc->type;
			acc = acc->next;
		}

		return acc;

	}

	const cl_accessor* parseRef(const cl_accessor* acc) {

		if (acc && (acc->code == CL_ACCESSOR_REF)) {
			this->deref = false;
			this->type = acc->type;
			acc = acc->next;
		} else {
			this->deref = true;
		}

		return acc;
		
	}

	void parseVar(const FAE& fae, const cl_operand* op, size_t offset) {

		this->data = Data::createRef(fae.varGet(ABP_INDEX).d_ref.root, (int)offset);
		this->type = op->type;

		const cl_accessor* acc = op->accessor;

		if (acc && (acc->code == CL_ACCESSOR_DEREF)) {
			
			fae.nodeLookup(this->data.d_ref.root, this->data.d_ref.displ, this->data);
			if (!this->data.isRef())
				throw std::runtime_error("OperandInfo::parseVar(): dereferenced value is not a valid reference!");

			this->type = acc->type;
			acc = acc->next;

		}

		acc = this->parseItems(acc);
		acc = this->parseRef(acc);
		assert(acc == NULL);

	}

	void parseReg(const FAE& fae, const cl_operand* op, size_t index) {

		// HACK: this is a bit ugly
		this->data = Data::createRef(index);
		this->type = op->type;

		const cl_accessor* acc = op->accessor;

		if (acc && (acc->code == CL_ACCESSOR_DEREF)) {

			this->data = fae.varGet(this->data.d_ref.root);
			if (!this->data.isRef())
				throw std::runtime_error("OperandInfo::parseReg(): dereferenced value is not a valid reference!");

			this->type = acc->type;
			acc = this->parseItems(acc->next);
			acc = this->parseRef(acc);

		} else {

			acc = this->parseItems(acc);
			this->deref = false;

		}

		assert(acc == NULL);

	}

};

struct SymCtx {

	const CodeStorage::Fnc& fnc;

	vector<SelData> sfLayout;

	typedef unordered_map<int, std::pair<bool, size_t> > var_map_type;

	var_map_type varMap;

	size_t regCount;

	SymCtx(const CodeStorage::Fnc& fnc) : fnc(fnc) {

		// pointer to previous stack frame
		this->sfLayout.push_back(SelData(ABP_OFFSET, sizeof(void*), 0));

		// pointer to context info
		this->sfLayout.push_back(SelData(CTX_OFFSET, sizeof(void*), 0));

		size_t offset = 2*sizeof(void*);

		for (CodeStorage::TVarList::const_iterator i = fnc.vars.begin(); i != fnc.vars.end(); ++i) {

			const CodeStorage::Var& var = fnc.stor->vars[*i];

			switch (var.code) {
				case CodeStorage::EVar::VAR_LC:
					if (var.name.empty()) {
						this->varMap.insert(make_pair(var.uid, make_pair(false, this->regCount++)));
					} else {
						NodeBuilder::buildNode(this->sfLayout, var.clt, offset);
						this->varMap.insert(make_pair(var.uid, make_pair(true, offset)));
						offset += var.clt->size;
					}
					break;
				default:
					break;
			}
			
		}

	}

	static void init(FAE& fae) {
		assert(fae.varCount() == 0);
		// create ABP and CTX registers
		fae.varPopulate(2);
		fae.varSet(ABP_INDEX, Data::createVoidPtr(0));
		fae.varSet(CTX_INDEX, Data::createUndef());
	}

	void createStackFrame(vector<FAE*>& dst, const FAE& fae) const {

		vector<pair<SelData, Data> > stackInfo;

		for (vector<SelData>::const_iterator i = this->sfLayout.begin(); i != this->sfLayout.end(); ++i)
			stackInfo.push_back(make_pair(*i, Data::createUndef()));

		FAE* tmp = new FAE(fae);

		const Data& abp = tmp->varGet(ABP_INDEX);
/*
		switch (abp.type) {
			// NULL
			case Data::type_enum::t_void_ptr: assert(abp.d_void_ptr == 0); break;
			// ordinary pointer
			case Data::type_enum::t_ref: assert(abp.d_ref.offset == 0); break;
			// :-(
			default: assert(false);
		}
*/
		stackInfo[0].second = abp;
		stackInfo[1].second = Data::createNativePtr((void*)this);

		tmp->varSet(ABP_INDEX, Data::createRef(tmp->nodeCreate(stackInfo)));
		tmp->varSet(CTX_INDEX, Data::createNativePtr((void*)this));
		tmp->varPopulate(this->regCount);

		dst.push_back(tmp);
		
	}

	void destroyStackFrame(vector<FAE*> dst, const FAE& fae) const {

		FAE tmp(fae);

		const Data& abp = tmp.varGet(ABP_INDEX);

		assert(abp.isRef());
		assert(abp.d_ref.displ == 0);
		
		Data data;

		tmp.varRemove(this->regCount);
		tmp.nodeLookup(abp.d_ref.root, ABP_OFFSET, data);
		tmp.unsafeNodeDelete(abp.d_ref.root);
/*
		switch (data.type) {
			// NULL
			case Data::type_enum::t_void_ptr: assert(data->d_void_ptr == 0); break;
			// ordinary pointer
			case Data::type_enum::t_ref: assert(selInfo->offset == 0); break;
			// :-(
			default: assert(false);
		}
*/
		tmp.varSet(ABP_INDEX, data);

		if (data.isRef()) {
			tmp.nodeLookup(abp.d_ref.root, CTX_OFFSET, data);
			assert(data.isNativePtr());
			tmp.varSet(CTX_INDEX, data);
		} else {
			tmp.varSet(CTX_INDEX, Data::createUndef());
		}

		tmp.isolateAtRoot(dst, abp.d_ref.root, FAE::IsolateAllF());
		
	}

	static Data extractNestedStruct(const Data& data, size_t base, const std::vector<size_t>& offsets) {
		assert(data.isStruct());
		std::map<size_t, const Data*> m;
		for (std::vector<Data::item_info>::const_iterator i = data.d_struct->begin(); i != data.d_struct->end(); ++i)
			m.insert(make_pair(i->first, &i->second));
		Data tmp = Data::createStruct();
		for (std::vector<size_t>::const_iterator i = offsets.begin(); i != offsets.end(); ++i) {
			std::map<size_t, const Data*>::iterator j = m.find(*i + base);
			if (j == m.end())
				throw std::runtime_error("SymCtx::extractNestedStruct(): selectors mismatch!");
			tmp.d_struct->push_back(make_pair(*i, *j->second));
		}
		return tmp;
	}

	void parseOperand(OperandInfo& operandInfo, const FAE& fae, const cl_operand* op) {

		var_map_type::iterator i = this->varMap.find(varIdFromOperand(op));

		assert(i != this->varMap.end());

		if (i->first)
			operandInfo.parseVar(fae, op, i->second.second);
		else
			operandInfo.parseReg(fae, op, i->second.second);

	}

};

struct SymState {

	// configuration obtained in forward run
	TA<label_type> fwdConf;

	// outstanding configurations
	vector<FAE*> outConf;

	UFAE fwdConfWrapper;

	const SymCtx* ctx;

	const CodeStorage::Block* block;

	SymState(TA<label_type>::Backend& taBackend, LabMan& labMan)
		: fwdConf(taBackend), fwdConfWrapper(this->fwdConf, labMan) {}

	~SymState() {
		utils::erase(this->outConf);
//		for (std::vector<FAE*>::iterator i = this->outConf.begin(); i != this->outConf.end(); ++i)
//			delete *i;
	}

};

struct SymOp {
};

class SymExec::Engine {

	const CodeStorage::Storage& stor;

	TA<label_type>::Backend taBackend;
	TA<label_type>::Manager taMan;
	LabMan labMan;
	BoxManager boxMan;

//	std::list<SymCtx*> ctxStore;

	// call ctx * call isns -> nested call ctx ! ... >:)
//	unordered_map<SymCtx*, CodeStorage::Isns*> ctxCache;

	typedef unordered_map<const CodeStorage::Fnc*, SymCtx*> ctx_store_type;
	ctx_store_type ctxStore;

	typedef unordered_map<const CodeStorage::Block*, SymState*> state_store_type;
	state_store_type stateStore;

	std::vector<SymState*> todo;

	std::vector<SymOp*> trace;

protected:

	SymCtx* getCtx(const CodeStorage::Fnc* fnc) {

		ctx_store_type::iterator i = this->ctxStore.find(fnc);
		if (i != this->ctxStore.end())
			return i->second;

		return this->ctxStore.insert(make_pair(fnc, new SymCtx(*fnc))).first->second;

	}

	SymState* getState(const CodeStorage::Block* block, const SymCtx* ctx) {

		state_store_type::iterator i = this->stateStore.find(block);
		if (i != this->stateStore.end())
			return i->second;

		SymState* s = new SymState(this->taBackend, this->labMan);
		s->block = block;
		s->ctx = ctx;
		
		return this->stateStore.insert(make_pair(block, s)).first->second;

	}

	void stateUnion(SymState* target, const vector<FAE*>& src) {

		bool changed = false;

		for (vector<FAE*>::const_iterator i = src.begin(); i != src.end(); ++i) {

			TA<label_type> ta(this->taBackend);
			Index<size_t> index;

			target->fwdConfWrapper.fae2ta(ta, index, **i);

			if (TA<label_type>::subseteq(target->fwdConf, ta)) {
				target->outConf.push_back(*i);
				changed = true;
			} else {
				target->fwdConfWrapper.join(ta, index);
				delete *i;
			}

		}

		if (changed)
			this->todo.push_back(target);
		
	}

	void processIsns(vector<FAE*>& dst, SymState* state, const vector<FAE*> src, const CodeStorage::Insn* insn) {
		
		switch (insn->code) {

			case cl_insn_e::CL_INSN_UNOP:
				break;

			case cl_insn_e::CL_INSN_BINOP:
				break;

			default: assert(false);

		}
				
	}

	void processTermIsns(SymState* state, const vector<FAE*> src, const CodeStorage::Insn* insn) {

		switch (insn->code) {

			case cl_insn_e::CL_INSN_JMP:
				this->stateUnion(this->getState(insn->targets[0], state->ctx), src);				
				break;

			case cl_insn_e::CL_INSN_COND:
				break;

			default: assert(false);

		}
		
	}

	void processState(SymState* state) {

		vector<FAE*> src, dst;

		if (state->outConf.empty())
			return;

		std::swap(src, state->outConf);

		CodeStorage::Block::const_iterator i = state->block->begin();

		for(; !cl_is_term_insn((*i)->code); ++i) {

			this->processIsns(dst, state, src, *i);

			utils::erase(src);
			
			std::swap(src, dst);

		}

		this->processTermIsns(state, src, *i);

		utils::erase(src);

	}

public:

	Engine(const CodeStorage::Storage& stor)
		: stor(stor), taMan(this->taBackend), boxMan(this->taMan, this->labMan) {}

	~Engine() {
		utils::eraseMap(this->stateStore);
		utils::eraseMap(this->ctxStore);
	}

	void run(const CodeStorage::Fnc& main) {

		// create main context
		SymCtx* mainCtx = this->getCtx(&main);
		
		// create an initial state
		SymState* init = this->getState(main.cfg.entry(), mainCtx);

		// create empty heap with no local variables
		FAE fae(this->taMan, this->labMan, this->boxMan);

		// add global registers
		SymCtx::init(fae);

		// enter main stack frame
		mainCtx->createStackFrame(init->outConf, fae);

		// schedule initial state for processing
		this->todo.push_back(init);

		while (!todo.empty()) {

			SymState* state = this->todo.back();
			this->todo.pop_back();
			this->processState(state);
			
		}
		
	}

};

SymExec::SymExec(const CodeStorage::Storage &stor)
	: engine(new Engine(stor)) {}
