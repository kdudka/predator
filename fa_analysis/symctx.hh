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

#ifndef SYM_CTX_H
#define SYM_CTX_H

#include <boost/unordered_map.hpp>

#include <cl/code_listener.h>
#include <cl/cldebug.hh>
#include <cl/storage.hh>

#include "regdef.hh"
#include "types.hh"
#include "operandinfo.hh"

struct SymCtx {

	const CodeStorage::Fnc& fnc;

	vector<SelData> sfLayout;

	// uid -> stack x offset/index
	typedef boost::unordered_map<int, std::pair<bool, size_t> > var_map_type;

	var_map_type varMap;

	size_t regCount;

	SymCtx(const CodeStorage::Fnc& fnc) : fnc(fnc), regCount(0) {

		// pointer to previous stack frame
		this->sfLayout.push_back(SelData(ABP_OFFSET, sizeof(void*), 0));

		// pointer to context info
		this->sfLayout.push_back(SelData(RET_OFFSET, sizeof(void*), 0));

		// pointer to context info
		this->sfLayout.push_back(SelData(AAX_OFFSET, sizeof(size_t), 0));

		size_t offset = 2*sizeof(void*) + sizeof(size_t);

		for (CodeStorage::TVarList::const_iterator i = fnc.vars.begin(); i != fnc.vars.end(); ++i) {

			const CodeStorage::Var& var = fnc.stor->vars[*i];

			switch (var.code) {
				case CodeStorage::EVar::VAR_LC:
					if (var.name.empty()) {
						this->varMap.insert(
							std::make_pair(
								var.uid, std::make_pair(false, FIXED_REG_COUNT + this->regCount++)
							)
						);
					} else {
						NodeBuilder::buildNode(this->sfLayout, var.clt, offset);
						this->varMap.insert(std::make_pair(var.uid, make_pair(true, offset)));
						offset += var.clt->size;
					}
					break;
				default:
					break;
			}
			
		}

	}
/*
	struct InitF {
		void operator()(FAE& fae) {
			assert(fae.varCount() == 0);
			// create ABP, RET, IAX registers
			fae.varPopulate(FIXED_REG_COUNT);
			fae.varSet(ABP_INDEX, Data::createInt(0));
			fae.varSet(RET_INDEX, Data::createUndef());
			fae.varSet(AAX_INDEX, Data::createUndef());
		}
	};
*/
	static void init(FAE& fae) {
		assert(fae.varCount() == 0);
		// create ABP and CTX registers
		fae.varPopulate(FIXED_REG_COUNT);
		fae.varSet(ABP_INDEX, Data::createInt(0));
		fae.varSet(RET_INDEX, Data::createUndef());
		fae.varSet(AAX_INDEX, Data::createUndef());
	}
/*
	struct StackFrameCreateF {
		
		const SymCtx* ctx;

		StackFrameCreateF(const SymCtx* ctx) : ctx(ctx) {}

		void operator()(vector<FAE*>& dst, const FAE& fae) {

			std::vector<std::pair<SelData, Data> > stackInfo;

			for (vector<SelData>::const_iterator i = this->ctx->sfLayout.begin(); i != this->ctx->sfLayout.end(); ++i)
				stackInfo.push_back(std::make_pair(*i, Data::createUndef()));

			FAE* tmp = new FAE(fae);

			stackInfo[0].second = tmp->varGet(ABP_INDEX);
			stackInfo[1].second = Data::createNativePtr(NULL);

			tmp->varSet(ABP_INDEX, Data::createRef(tmp->nodeCreate(stackInfo)));
			// TODO: ...

			tmp->varSet(RET_INDEX, Data::createNativePtr(NULL));
			tmp->varSet(AAX_INDEX, Data::createInt(0));
			tmp->varPopulate(this->regCount);

			dst.push_back(tmp);

		}

	};
*/
	void createStackFrame(vector<FAE*>& dst, const FAE& fae) const {

		std::vector<pair<SelData, Data> > stackInfo;

		for (vector<SelData>::const_iterator i = this->sfLayout.begin(); i != this->sfLayout.end(); ++i)
			stackInfo.push_back(make_pair(*i, Data::createUndef()));

		FAE* tmp = new FAE(fae);

		stackInfo[0].second = tmp->varGet(ABP_INDEX);
		stackInfo[1].second = Data::createNativePtr(NULL);

		tmp->varSet(ABP_INDEX, Data::createRef(tmp->nodeCreate(stackInfo)));
		// TODO: ...

		tmp->varSet(RET_INDEX, Data::createNativePtr(NULL));
		tmp->varSet(AAX_INDEX, Data::createInt(0));
		tmp->varPopulate(this->regCount);

		dst.push_back(tmp);
		
	}

	void destroyStackFrame(vector<FAE*>& dst, const FAE& fae) const {

		FAE tmp(fae);

		const Data& abp = tmp.varGet(ABP_INDEX);

		assert(abp.isRef());
		assert(abp.d_ref.displ == 0);
		
		Data data;

		tmp.varRemove(this->regCount);
		tmp.nodeLookup(abp.d_ref.root, ABP_OFFSET, data);
		tmp.unsafeNodeDelete(abp.d_ref.root);
		tmp.varSet(ABP_INDEX, data);

		if (abp.isRef()) {
			tmp.nodeLookup(abp.d_ref.root, RET_OFFSET, data);
			assert(data.isNativePtr());
			tmp.varSet(RET_INDEX, data);
			tmp.isolateAtRoot(dst, abp.d_ref.root, FAE::IsolateAllF());
		} else {
			tmp.varSet(RET_INDEX, Data::createUndef());
			dst.push_back(new FAE(tmp));
		}
		
	}

	void parseOperand(OperandInfo& operandInfo, const FAE& fae, const cl_operand* op) const {

		switch (op->code) {
			case cl_operand_e::CL_OPERAND_VAR: {
				var_map_type::const_iterator i = this->varMap.find(varIdFromOperand(op));
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

	void dumpContext(const FAE& fae) const {

		vector<size_t> offs;

		for (vector<SelData>::const_iterator i = this->sfLayout.begin(); i != this->sfLayout.end(); ++i)
			offs.push_back((*i).offset);

		Data data;

		fae.nodeLookupMultiple(fae.varGet(ABP_INDEX).d_ref.root, 0, offs, data);

		unordered_map<size_t, Data> tmp;
		for (vector<Data::item_info>::const_iterator i = data.d_struct->begin(); i != data.d_struct->end(); ++i)
			tmp.insert(make_pair(i->first, i->second));

		for (CodeStorage::TVarList::const_iterator i = this->fnc.vars.begin(); i != this->fnc.vars.end(); ++i) {

			const CodeStorage::Var& var = this->fnc.stor->vars[*i];

			var_map_type::const_iterator j = this->varMap.find(var.uid);
			assert(j != this->varMap.end());

			switch (var.code) {
				case CodeStorage::EVar::VAR_LC:
					if (var.name.empty()) {
//						CL_DEBUG('#' << var.uid << " = " << fae.varGet(j->second.second));
					} else {
						unordered_map<size_t, Data>::iterator k = tmp.find(j->second.second);
						assert(k != tmp.end());
						CL_DEBUG('#' << var.uid << ':' << var.name << " = " << k->second);
					}
					break;
				default:
					break;
			}
			
		}

	}
/*
	static SymCtx* extractCtx(const FAE& fae) {

		var_map_type::iterator i = this->varMap.find(varIdFromOperand(op));

		const Data& abp = fae.varGet(CTX_INDEX);

		assert(abp.isNativePtr());

		return (SymCtx*)abp.d_native_ptr;
		
	}
*/
};

#endif
