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

#include <cl/cl_msg.hh>
#include <cl/storage.hh>

#include "treeaut.hh"
#include "forestaut.hh"
#include "forestautext.hh"
#include "ufae.hh"
#include "symexec.hh"

using boost::unordered_map;

struct NodeBuilder {

	static void buildNode(vector<SelData>& nodeInfo, CodeStorage::cl_type* type, int offset = 0) {
		
		assert(type->size > 0);

		switch (type->code) {
			case cl_type_e::CL_TYPE_PTR:
			case cl_type_e::CL_TYPE_INT:
			case cl_type_e::CL_TYPE_BOOL:
				nodeInfo.push_back(SelData::create(offset, type->size, 0));
				break;
			case cl_type_e::CL_TYPE_STRUCT:
				for (int i = 0; i < type->item_cnt; ++i)
					NodeBuilder::buildNode(nodeInfo, type->items[i]->type, offset + type->items[i]->offset);
				break;
//			case cl_type_e::CL_TYPE_UNION:
		}

	}

};

struct OperandInfo {
	size_t root;
	int offset;
	bool ref;
	const CodeStorage::cl_type* type;
};

#define ABP_INDEX		0
#define ABP_OFFSET		0
#define CTX_INDEX		1
#define CTX_OFFSET		sizeof(void*)
#define FIXED_REG_COUNT	2

struct SymCtx {

	const CodeStorage::Fnc& fnc;

	vector<SelData> sfLayout;

	unordered_map<int, size_t> varMap;

	size_t regCount;

	SymCtx(const CodeStorage::Fnc& fnc) : fnc(fnc) {

		// pointer to previous stack frame
		this->sfLayout.push_back(SelData::create(ABP_OFFSET, sizeof(void*), 0));

		// pointer to context info
		this->sfLayout.push_back(SelData::create(CTX_OFFSET, sizeof(void*), 0));

		size_t offset = 2*sizeof(void*);

		for (CodeStorage::TVarList::const_iterator i = fnc.vars.begin(); i != fnc.vars.end(); ++i) {

			const CodeStorage::Var& var = fnc.stor.vars[*i];

			switch (var.code) {

				case EVAR::VAR_LC:
					NodeBuilder::buildNode(this->sfLayout, var->type, offset);
					this->varMap.insert(make_pair(var.uid, offset));
					offset += var.clt->size;
					break;

				case EVAR::VAR_REG:
					this->varMap.insert(make_pair(var.uid, this->regCount++));
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

		for (vector<SelData>::iterator i = this->sfLayout.begin(); i != this->sfLayout.end(); ++i)
			stackInfo.push_back(make_pair(*i, Data::createUndef()));

		FAE* tmp = new FAE(fae);

		const Data& abp = tmp->varGet(ABP_INDEX);

		switch (abp.type) {
			// NULL
			case Data::type_enum::t_void_ptr: assert(abp.d_void_ptr == 0); break;
			// ordinary pointer
			case Data::type_enum::t_ref: assert(abp.d_ref.offset == 0); break;
			// :-(
			default: assert(false);
		}

		stackInfo[0].second = abp;
		stackInfo[1].second = Data::createNativePtr(this);

		tmp->varSet(ABP_INDEX, Data::createRef(tmp->nodeCreate(stackInfo)));
		tmp->varSet(CTX_INDEX, Data::createNativePtr(this));
		tmp->varPopulate(this->regCount);

		dst.push_back(tmp);
		
	}

	void destroyStackFrame(vector<FAE*> dst, const FAE& fae) const {

		FAE tmp(fae);

		const Data& abp = tmp.varGet(ABP_INDEX);

		assert(abp.isRef());
		assert(abp.d_ref.offset == 0);
		
		const SelData* selInfo;
		const Data* data;

		tmp.varDrop(this->regCount);
		tmp.nodeLookup(abp.d_ref.root, ABP_OFFSET, selInfo, data);
		tmp.unsafeNodeDelete(abp.d_ref.root);

		switch (data.type) {
			// NULL
			case Data::type_enum::t_void_ptr: assert(data->d_void_ptr == 0); break;
			// ordinary pointer
			case Data::type_enum::t_ref: assert(selInfo->offset == 0); break;
			// :-(
			default: assert(false);
		}

		tmp.varSet(ABP_INDEX, *data);

		if (data->isRef()) {
			tmp.nodeLookup(abp.d_ref.root, CTX_OFFSET, selInfo, data);
			assert(data->isNativePtr());
			tmp.varSet(CTX_INDEX, data);
		} else {
			tmp.varSet(CTX_INDEX, Data::createUndef());
		}

		tmp.isolateAtRoot(dst, abp.d_ref.root, FAE::IsolateAllF());
		
	}

	void parseAccessorChain(OperandInfo& operandInfo, const FAE& fae, const CodeStorage::cl_operand* op) {

		unordered_map<int, size_t>::iterator i = this->varMap.find(op->var.id);

		assert(i != this->varMap.end());

		op->code == CodeStorage::CL_OPERAND_REG

		if (op->code == CodeStorage::CL_OPERAND_VAR || op->code == CodeStorage::CL_OPERAND_ARG);

		operandInfo.root = ABP_INDEX;
		operandInfo.offset = (int)i->second;
		operandInfo.type = op.type;
		operandInfo.ref = false;

		const SelData* selAux;
		const Data* aux;

		CodeStorage::cl_accessor* accessor = op->accessor;

		if (accessor && (accessor->code == CodeStorage::CL_ACCESSOR_DEREF)) {
			fae.nodeLookup(root, (size_t)offset, selAux, aux);
			if (!aux->isRef())
				throw std::runtime_error("Dereferenced variable does not contain a valid reference!");
			operandInfo.root = aux->d_ref.root;
			operandInfo.offset = selAux->aux;
			operandInfo.type = accessor->type;
			accessor = accessor->next;
		}

		while (accessor && (accessor->code == CodeStorage::CL_ACCESSOR_ITEM)) {
			operandInfo.offset += accessor->type->items[accessor->item.id]->offset;
			operandInfo.type = accessor->type;
			accessor = accessor->next;
		}

		if (accessor && (accessor->code == CodeStorage::CL_ACCESSOR_REF)) {
			operandInfo.ref = true;
			operandInfo.type = accessor->type;
			accessor = accessor->next;
		}

		assert(accessor == NULL);			

	}

};

struct SymState {

	// configuration obtained in forward run
	TA<FA::label_type> fwdConf;

	// outstanding configurations
	vector<FAE*> outConf;

	UFAE fwdConfWrapper;

	SymCtx* ctx;

	CodeStorage::Block* block;

	SymState(TA<FA::label_type>::Backend& taBackend, LabMan& labMan)
		: fwdConf(taBackend), fwdConfWrapper(this->fwdConf, labMan) {}

	~SymState() {
		for (std::vector<FAE*>::iterator i = this->outConf.begin(); this->outConf.end(); ++i)
			delete *i;
	}

};

struct SymOp {
};

class SymExec::Engine {

	const CodeStorage::Storage& stor;

	TA<FA::label_type>::Backend taBackend;

	TA<FA::label_type> taMan;
	LabMan labMan;
	BoxManager boxMan;

//	std::list<SymCtx*> ctxStore;

	// call ctx * call isns -> nested call ctx ! ... >:)
//	unordered_map<SymCtx*, CodeStorage::Isns*> ctxCache;

	typedef unordered_map<CodeStorage::Fnc*, SymCtx*> ctx_store_type;
	ctx_store_type ctxStore;

	typedef unordered_map<CodeStorage::Block*, SymState*> state_store_type;
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
		
		return this->stateStore.insert(block, s).first->second;

	}

	void stateUnion(SymState* target, const vector<FAE*>& src) {

		bool changed = false;

		for (vector<FAE*>::const_iterator i = src.begin(); i != src.end(); ++i) {

			TA<FA::label_type> ta(this->taBackend);
			Index<size_t> index;

			target->fwdConfWrapper.fae2ta(ta, index, **i);

			if (TA<label_type>::subseteq(target->fwdConf, ta)) {
				target->outConf.push_back(*i);
				changed = true;
			} else {
				this->fwdConfWrapper.join(ta, index);
				delete *i;
			}

		}

		if (changed)
			this->todo.push_back(target);
		
	}

	void processIsns(vector<FAE*> dst, SymState* state, const vector<FAE*> src, const CodeStorage::Isns* isns) {
		
		switch (isns->code) {

			case cl_isns_e::CL_ISNS_UNOP:
				break;

			case cl_isns_e::CL_ISNS_BINOP:
				break;

			default: assert(false);

		}
				
	}

	void processTermIsns(SymState* state, const vector<FAE*> src, const CodeStorage::Isns* isns) {

		switch (isns->code) {

			case cl_isns_e::CL_ISNS_JMP:
				this->stateUnion(this->getState(isns->targets[0], state->ctx), src);				
				break;

			case cl_isns_e::CL_ISNS_COND:
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

		for(; (!cl_is_term_isns(*i); ++i) {

			this->processIsns(dst, state, src, *i);

			utils::erase(src);
			
			std::swap(src, dst);

		}

		this->processTermIsns(state, src, *i);

		utils::erase(src);

	}

public:

	Engine(const CodeStorage::Storage& stor)
		: stor(stor), boxMan(this->taMan, this->labMan) {}

	~Engine() {
		erase(this->statesStore);
		erase(this->ctxStore);
	}

	void run(const CodeStorage::Fnc& main) {

		// create main context
		SymCtx* mainCtx = this->getCtx(main);
		
		// create an initial state
		SymState* init = this->getState(main.entry(), mainCtx);

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
