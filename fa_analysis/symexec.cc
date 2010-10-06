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
#include <cl/cl_debug.hh>
#include <cl/storage.hh>

#include "treeaut.hh"
#include "forestaut.hh"
#include "forestautext.hh"
#include "ufae.hh"
#include "symexec.hh"

// abstract base pointer
#define ABP_INDEX		0
#define ABP_OFFSET		0
// 'return address'
#define RET_INDEX		1
#define RET_OFFSET		sizeof(void*)
// index of register with return value
#define IAX_INDEX		2
#define IAX_OFFSET		2*sizeof(void*)

#define FIXED_REG_COUNT	3

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

typedef enum { safe_ref, ref, reg, value } o_flag_e;

struct OperandInfo {

	o_flag_e flag;

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

		assert((this->flag == o_flag_e::ref) || (this->flag == o_flag_e::safe_ref));

		if (acc && (acc->code == CL_ACCESSOR_REF)) {
			this->flag = o_flag_e::value;
			this->type = acc->type;
			acc = acc->next;
		} 

		return acc;
		
	}

	void parseCst(const cl_operand* op) {
		
		this->flag = o_flag_e::value;
		this->type = op->type;

		switch (op->data.cst.code) {
			case cl_type_e::CL_TYPE_INT:
				this->data = Data::createInt(op->data.cst.data.cst_int.value);
				break;
			default:
				assert(false);
		}

	}

	void parseVar(const FAE& fae, const cl_operand* op, size_t offset) {

		this->data = Data::createRef(fae.varGet(ABP_INDEX).d_ref.root, (int)offset);
		this->flag = o_flag_e::safe_ref;
		this->type = op->type;

		const cl_accessor* acc = op->accessor;

		if (acc && (acc->code == CL_ACCESSOR_DEREF)) {
			
			fae.nodeLookup(this->data.d_ref.root, this->data.d_ref.displ, this->data);
			if (!this->data.isRef())
				throw std::runtime_error("OperandInfo::parseVar(): dereferenced value is not a valid reference!");

			this->flag = o_flag_e::ref;
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

			this->flag = o_flag_e::ref;
			this->type = acc->type;
			acc = this->parseItems(acc->next);
			acc = this->parseRef(acc);

		} else {

			this->flag = o_flag_e::reg;
			acc = this->parseItems(acc);

		}

		assert(acc == NULL);

	}

	static bool isRef(o_flag_e flag) {

		return flag == o_flag_e::ref || flag == o_flag_e::safe_ref;

	}

	static bool isLValue(o_flag_e flag) {

		return flag == o_flag_e::ref || flag == o_flag_e::safe_ref || flag == o_flag_e::reg;

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
				throw std::runtime_error("OperandInfo::extractNestedStruct(): selectors mismatch!");
			tmp.d_struct->push_back(make_pair(*i, *j->second));
		}
		return tmp;
	}

	static void modifyNestedStruct(Data& dst, size_t base, const Data& src) {
		assert(dst.isStruct());
		assert(src.isStruct());
		std::map<size_t, const Data*> m;
		for (std::vector<Data::item_info>::const_iterator i = src.d_struct->begin(); i != src.d_struct->end(); ++i)
			m.insert(make_pair(base + i->first, &i->second));
		Data tmp = Data::createStruct();
		size_t matched = 0;
		for (std::vector<Data::item_info>::iterator i = dst.d_struct->begin(); i != dst.d_struct->end(); ++i) {
			std::map<size_t, const Data*>::iterator j = m.find(i->first);
			if (j != m.end()) {
				i->second = *j->second;
				++matched;
			}
		}
		if (matched != src.d_struct->size())
			throw std::runtime_error("OperandInfo::modifyNestedStruct(): selectors mismatch!");
	}

};

struct SymCtx {

	const CodeStorage::Fnc& fnc;

	vector<SelData> sfLayout;

	// uid -> stack x offset/index
	typedef unordered_map<int, std::pair<bool, size_t> > var_map_type;

	var_map_type varMap;

	size_t regCount;

	SymCtx(const CodeStorage::Fnc& fnc) : fnc(fnc), regCount(0) {

		// pointer to previous stack frame
		this->sfLayout.push_back(SelData(ABP_OFFSET, sizeof(void*), 0));

		// pointer to context info
		this->sfLayout.push_back(SelData(RET_OFFSET, sizeof(void*), 0));

		// pointer to context info
		this->sfLayout.push_back(SelData(IAX_OFFSET, sizeof(size_t), 0));

		size_t offset = 2*sizeof(void*) + sizeof(size_t);

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
		fae.varPopulate((size_t)3);
		fae.varSet(ABP_INDEX, Data::createVoidPtr(0));
		fae.varSet(RET_INDEX, Data::createUndef());
		fae.varSet(IAX_INDEX, Data::createUndef());
	}

	void createStackFrame(vector<FAE*>& dst, const FAE& fae) const {

	    CL_DEBUG("entering stack frame: " << CodeStorage::nameOf(this->fnc));

		vector<pair<SelData, Data> > stackInfo;

		for (vector<SelData>::const_iterator i = this->sfLayout.begin(); i != this->sfLayout.end(); ++i)
			stackInfo.push_back(make_pair(*i, Data::createUndef()));

		FAE* tmp = new FAE(fae);

		stackInfo[0].second = tmp->varGet(ABP_INDEX);
		stackInfo[1].second = Data::createNativePtr((void*)this);

		tmp->varSet(ABP_INDEX, Data::createRef(tmp->nodeCreate(stackInfo)));
		// TODO: ...
		tmp->varSet(RET_INDEX, Data::createNativePtr(NULL));
		tmp->varSet(IAX_INDEX, Data::createInt(0));
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
		tmp.varSet(ABP_INDEX, data);

		if (data.isRef()) {
			tmp.nodeLookup(abp.d_ref.root, RET_OFFSET, data);
			assert(data.isNativePtr());
			tmp.varSet(RET_INDEX, data);
		} else {
			tmp.varSet(RET_INDEX, Data::createUndef());
		}

		tmp.isolateAtRoot(dst, abp.d_ref.root, FAE::IsolateAllF());
		
	}

	void parseOperand(OperandInfo& operandInfo, const FAE& fae, const cl_operand* op) const {

		switch (op->code) {
			case cl_operand_e::CL_OPERAND_VAR: {
				var_map_type::const_iterator i = this->varMap.find(op->data.var.id);
				assert(i != this->varMap.end());
				switch (i->second.first) {
					case true: operandInfo.parseVar(fae, op, i->second.second); break;
					case false: operandInfo.parseReg(fae, op, i->second.second); break;
				}
				break;
			}
			case cl_operand_e::CL_OPERAND_CST:
				operandInfo.parseCst(op);
				break;
			default:
				assert(false);
		}

	}
/*
	static SymCtx* extractCtx(const FAE& fae) {

		const Data& abp = fae.varGet(CTX_INDEX);

		assert(abp.isNativePtr());

		return (SymCtx*)abp.d_native_ptr;
		
	}
*/
};

struct SymState {

	// configuration obtained in forward run
	TA<label_type> fwdConf;

	// outstanding configurations
	vector<FAE*> outConf;

	UFAE fwdConfWrapper;

	const SymCtx* ctx;

	CodeStorage::Block::const_iterator insn;

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

typedef enum { biNone, biMalloc, biFree } builtin_e;

struct BuiltinTable {

	unordered_map<string, builtin_e> _table;

public:

	BuiltinTable() {
		this->_table["malloc"] = builtin_e::biMalloc;
		this->_table["free"] = builtin_e::biFree;
	}

	builtin_e operator[](const string& key) {
		unordered_map<string, builtin_e>::iterator i = this->_table.find(key);
		return (i == this->_table.end())?(builtin_e::biNone):(i->second);
	}

};

class SymExec::Engine {

	static BuiltinTable builtins;

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

	typedef unordered_map<const CodeStorage::Insn*, SymState*> state_store_type;
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

	SymState* getState(const CodeStorage::Block::const_iterator& insn, const SymCtx* ctx) {

		state_store_type::iterator i = this->stateStore.find(*insn);
		if (i != this->stateStore.end())
			return i->second;

		SymState* s = new SymState(this->taBackend, this->labMan);
		s->insn = insn;
		s->ctx = ctx;
		
		return this->stateStore.insert(make_pair(*insn, s)).first->second;

	}

	void stateUnion(SymState* target, vector<FAE*>& src) {

		bool changed = false;

		while (!src.empty()) {

			FAE::NormInfo normInfo;

			src.back()->normalize(normInfo);
			src.back()->heightAbstraction();

			TA<label_type> ta(this->taBackend);
			Index<size_t> index;

			target->fwdConfWrapper.fae2ta(ta, index, *src.back());

			if (TA<label_type>::subseteq(target->fwdConf, ta)) {
				target->outConf.push_back(src.back());
				changed = true;
			} else {
				target->fwdConfWrapper.join(ta, index);
				delete src.back();
			}

			src.pop_back();

		}

		if (changed)
			this->todo.push_back(target);
		
	}

	void enqueueNextInsn(SymState* state, vector<FAE*>& src) {

		this->stateUnion(this->getState(state->insn + 1, state->ctx), src);
		
	}

	void isolateIfNeeded(vector<FAE*>& dst, const vector<FAE*>& src, const OperandInfo& oi, const vector<size_t>& offs) {

		for (vector<FAE*>::const_iterator i = src.begin(); i != src.end(); ++i) {
			if (oi.flag == o_flag_e::ref)
				(*i)->isolateAtRoot(dst, oi.data.d_ref.root, FAE::IsolateSetF(offs, oi.data.d_ref.displ));
			else
				dst.push_back(new FAE(**i));
		}

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
			case o_flag_e::value:
				data = oi.data;
				break;
			default:
				assert(false);
		}
		return data;
	}

	void writeData(FAE& fae, const OperandInfo& oi, const Data& in, Data& out) {
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

	void execAssignment(SymState* state, const FAE& fae, const CodeStorage::Insn* insn) {
		OperandInfo dst, src;
		state->ctx->parseOperand(dst, fae, &insn->operands[0]);
		state->ctx->parseOperand(src, fae, &insn->operands[1]);

		assert(*(src.type) == *(dst.type));
		assert(OperandInfo::isLValue(dst.flag));

		if (src.type->code == cl_type_e::CL_TYPE_PTR && src.data.isVoidPtr()) {
			assert(dst.type->items[0].type->size == (int)src.data.d_void_ptr);
			vector<FAE*> tmp;
			ContainerGuard<vector<FAE*> > g(tmp);
			if (dst.flag == o_flag_e::ref) {
				fae.isolateAtRoot(tmp, dst.data.d_ref.root, FAE::IsolateOneF(dst.data.d_ref.displ));
			} else {
				tmp.push_back(new FAE(fae));
			}
			vector<SelData> sels;
			NodeBuilder::buildNode(sels, dst.type->items[0].type);
			for (vector<FAE*>::iterator i = tmp.begin(); i != tmp.end(); ++i) {
				Data data = Data::createRef((*i)->nodeCreate(sels)), out;
				switch (dst.flag) {
					case o_flag_e::ref:
					case o_flag_e::safe_ref:
						(*i)->nodeModify(dst.data.d_ref.root, dst.data.d_ref.displ, data, out);
						break;
					case o_flag_e::reg:
						(*i)->varSet(dst.data.d_ref.root, data);
						break;
					default:
						assert(false);					
				}
			}
			this->enqueueNextInsn(state, tmp);
			return;
		}

		vector<size_t> offs;
		NodeBuilder::buildNode(offs, src.type);

		vector<FAE*> tmp, tmp2;
		ContainerGuard<vector<FAE*> > f(tmp), g(tmp2);

		this->isolateIfNeeded(tmp, itov((FAE*)&fae), src, offs);
		this->isolateIfNeeded(tmp2, tmp, dst, offs);

		for (vector<FAE*>::iterator i = tmp2.begin(); i != tmp2.end(); ++i) {
			Data data = this->readData(**i, src, offs), dataOut;
			this->writeData(**i, dst, data, dataOut);
		}

		this->enqueueNextInsn(state, tmp2);
		
	}

	void execCmp(SymState* state, const FAE& fae, const CodeStorage::Insn* insn, bool neg) {
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

		vector<FAE*> tmp, tmp2, tmp3;
		ContainerGuard<vector<FAE*> > f(tmp), g(tmp2), h(tmp3);

		this->isolateIfNeeded(tmp, itov((FAE*)&fae), src1, offs1);
		this->isolateIfNeeded(tmp2, tmp, src2, offs2); 
		this->isolateIfNeeded(tmp3, tmp2, dst, itov((size_t)0));

		for (vector<FAE*>::iterator i = tmp3.begin(); i != tmp3.end(); ++i) {
			Data data1 = this->readData(**i, src1, offs1);
			Data data2 = this->readData(**i, src2, offs2);
			Data res = Data::createBool((data1 == data2) != neg), dataOut;
			this->writeData(**i, dst, res, dataOut);
		}

		this->enqueueNextInsn(state, tmp2);
		
	}

	void execMalloc(SymState* state, const FAE& fae, const CodeStorage::Insn* insn) {
		OperandInfo dst, src;
		state->ctx->parseOperand(dst, fae, &insn->operands[0]);
		assert(dst.flag == o_flag_e::reg);
		state->ctx->parseOperand(src, fae, &insn->operands[2]);
		assert(src.type->code == cl_type_e::CL_TYPE_INT);

		vector<FAE*> tmp;
		ContainerGuard<vector<FAE*> > g(tmp);

		if (src.flag == o_flag_e::ref) {
			fae.isolateAtRoot(tmp, src.data.d_ref.root, FAE::IsolateOneF(src.data.d_ref.displ));
		} else {
			tmp.push_back(new FAE(fae));
		}

		for (vector<FAE*>::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			if (OperandInfo::isRef(src.flag))
				(*i)->nodeLookup(src.data.d_ref.root, src.data.d_ref.displ, src.data);
			assert(src.data.isInt());
			(*i)->varSet(dst.data.d_ref.root, Data::createVoidPtr(src.data.d_int));
		}

		this->enqueueNextInsn(state, tmp);
	
	}

	void execFree(SymState* state, const FAE& fae, const CodeStorage::Insn* insn) {

		OperandInfo dst;
		state->ctx->parseOperand(dst, fae, &insn->operands[2]);
		assert(dst.flag == o_flag_e::ref);
		assert(dst.data.d_ref.displ == 0);

		vector<FAE*> tmp;
		ContainerGuard<vector<FAE*> > g(tmp);

		fae.isolateAtRoot(tmp, dst.data.d_ref.root, FAE::IsolateAllF());

		for (vector<FAE*>::iterator i = tmp.begin(); i != tmp.end(); ++i)
			(*i)->nodeDelete(dst.data.d_ref.root);

		this->enqueueNextInsn(state, tmp);
		
	}

	void execJmp(SymState* state, const FAE& fae, const CodeStorage::Insn* insn) {

		vector<FAE*> tmp = itov(new FAE(fae));

		this->stateUnion(this->getState(insn->targets[0]->begin(), state->ctx), tmp);

	}

	void execCond(SymState* state, const FAE& fae, const CodeStorage::Insn* insn) {

		OperandInfo src;
		state->ctx->parseOperand(src, fae, &insn->operands[0]);

		assert(src.type->code == cl_type_e::CL_TYPE_BOOL);

		vector<FAE*> tmp, tmp2, tmp3;
		ContainerGuard<vector<FAE*> > g(tmp);

		this->isolateIfNeeded(tmp, itov((FAE*)&fae), src, itov((size_t)0));

		for (vector<FAE*>::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			Data data = this->readData(**i, src, itov((size_t)0));
			if (!data.isBool())
				throw runtime_error("Engine::execCond(): non boolean condition argument!");
			if (data.d_bool)
				tmp2.push_back(*i);
			else
				tmp3.push_back(*i);
		}

		tmp.clear();

		this->stateUnion(this->getState(insn->targets[0]->begin(), state->ctx), tmp2);
		this->stateUnion(this->getState(insn->targets[1]->begin(), state->ctx), tmp3);

	}

	void execInsn(SymState* state, const FAE& fae, const CodeStorage::Insn* insn) {
		
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
						this->execCmp(state, fae, insn, false);
						break;
					case cl_binop_e::CL_BINOP_NE:
						this->execCmp(state, fae, insn, true);
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
					default:
						assert(false);
				}
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

	void processState(SymState* state) {

		if (state->outConf.empty())
			return;

		vector<FAE*> src;

		std::swap(src, state->outConf);

		for (vector<FAE*>::iterator i = src.begin(); i != src.end(); ++i) {

			assert(*i);

			CL_DEBUG(std::endl << **i << std::endl);

			this->execInsn(state, **i, *state->insn);

		}

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
		mainCtx->createStackFrame(init->outConf, fae);

	    CL_DEBUG("sheduling initial state ...");
		// schedule initial state for processing
		this->todo.push_back(init);

		try {

			while (!todo.empty()) {

				SymState* state = this->todo.back();

				CL_DEBUG("executing location : " << **state->insn);

				this->todo.pop_back();
				this->processState(state);

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
