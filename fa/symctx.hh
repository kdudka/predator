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

#include <vector>

#include <boost/unordered_map.hpp>

#include <cl/code_listener.h>
#include <cl/cldebug.hh>
#include <cl/storage.hh>
#include <cl/clutil.hh>
#include <cl/cl_msg.hh>

#include "regdef.hh"
#include "types.hh"
#include "operandinfo.hh"

#define ABP_OFFSET		0
#define ABP_SIZE		SymCtx::size_of_data
#define IP_OFFSET		(ABP_OFFSET + ABP_SIZE)
#define IP_SIZE			SymCtx::size_of_data

struct SymCtx {

	// must be initialized externally!
	static int size_of_data;
	static int size_of_code;

	// initialize size_of_void
	static void initCtx(const CodeStorage::Storage& stor) {
		size_of_code = stor.types.codePtrSizeof();
		if (size_of_code == -1)
			size_of_code = sizeof(void(*)());
		size_of_data = stor.types.dataPtrSizeof();
		if (size_of_data == -1)
			size_of_data = sizeof(void*);
	}

	const CodeStorage::Fnc& fnc;

	std::vector<SelData> sfLayout;

	// uid -> stack x offset/index
	typedef boost::unordered_map<int, std::pair<bool, size_t> > var_map_type;

	var_map_type varMap;

	size_t regCount;

	SymCtx(const CodeStorage::Fnc& fnc) : fnc(fnc), regCount(0) {

		// pointer to previous stack frame
		this->sfLayout.push_back(SelData(ABP_OFFSET, ABP_SIZE, 0));

		// pointer to context info
		this->sfLayout.push_back(SelData(IP_OFFSET, IP_SIZE, 0));

		size_t offset = ABP_SIZE + IP_SIZE;

		for (CodeStorage::TVarSet::const_iterator i = fnc.vars.begin(); i != fnc.vars.end(); ++i) {

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
						NodeBuilder::buildNode(this->sfLayout, var.type, offset);
						this->varMap.insert(std::make_pair(var.uid, make_pair(true, offset)));
						offset += var.type->size;
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

		VirtualMachine vm(fae);

		assert(vm.varCount() == 0);
		// create ABP and RET registers
		vm.varPopulate(FIXED_REG_COUNT);
		vm.varSet(ABP_INDEX, Data::createInt(0));
		vm.varSet(IP_INDEX, Data::createUndef());
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
	void createStackFrame(FAE& fae, struct SymState* target) const {

		VirtualMachine vm(fae);

		std::vector<pair<SelData, Data> > stackInfo;

		for (std::vector<SelData>::const_iterator i = this->sfLayout.begin(); i != this->sfLayout.end(); ++i)
			stackInfo.push_back(make_pair(*i, Data::createUndef()));

		stackInfo[0].second = vm.varGet(ABP_INDEX);
		stackInfo[1].second = vm.varGet(IP_INDEX);

		vm.varSet(ABP_INDEX, Data::createRef(vm.nodeCreate(stackInfo)));
		vm.varSet(IP_INDEX, Data::createNativePtr((void*)target));
		vm.varPopulate(this->regCount);

	}
	
	// if true then do fae.isolateAtRoot(dst, <ABP>.d_ref.root, FAE::IsolateAllF()) in the next step
	bool destroyStackFrame(FAE& fae) const {

		VirtualMachine vm(fae);

		const Data& abp = vm.varGet(ABP_INDEX);

		assert(abp.isRef());
		assert(abp.d_ref.displ == 0);
		
		Data data;

		vm.varRemove(this->regCount);
		vm.nodeLookup(abp.d_ref.root, ABP_OFFSET, data);
		vm.unsafeNodeDelete(abp.d_ref.root);
		vm.varSet(ABP_INDEX, data);

		if (!abp.isRef()) {
			vm.varSet(IP_INDEX, Data::createUndef());
			return false;
		}

		vm.nodeLookup(abp.d_ref.root, IP_OFFSET, data);
		assert(data.isNativePtr());
		vm.varSet(IP_INDEX, data);

		return true;
		
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

	bool isReg(const cl_operand* op, size_t& id) const {
		if (op->code != cl_operand_e::CL_OPERAND_VAR)
			return false;
		var_map_type::const_iterator i = this->varMap.find(varIdFromOperand(op));
		assert(i != this->varMap.end());
		if (i->second.first)
			return false;
		id = i->second.second;
		return true;
	}

	struct Dump {

		const SymCtx& ctx;
		const FAE& fae;

		Dump(const SymCtx& ctx, const FAE& fae) : ctx(ctx), fae(fae) {}
		
		friend std::ostream& operator<<(std::ostream& os, const Dump& cd) {

			VirtualMachine vm(cd.fae);

			std::vector<size_t> offs;

			for (std::vector<SelData>::const_iterator i = cd.ctx.sfLayout.begin(); i != cd.ctx.sfLayout.end(); ++i)
				offs.push_back((*i).offset);

			Data data;

			vm.nodeLookupMultiple(vm.varGet(ABP_INDEX).d_ref.root, 0, offs, data);

			boost::unordered_map<size_t, Data> tmp;
			for (std::vector<Data::item_info>::const_iterator i = data.d_struct->begin(); i != data.d_struct->end(); ++i)
				tmp.insert(make_pair(i->first, i->second));

			for (CodeStorage::TVarSet::const_iterator i = cd.ctx.fnc.vars.begin(); i != cd.ctx.fnc.vars.end(); ++i) {

				const CodeStorage::Var& var = cd.ctx.fnc.stor->vars[*i];

				var_map_type::const_iterator j = cd.ctx.varMap.find(var.uid);
				assert(j != cd.ctx.varMap.end());

				switch (var.code) {
					case CodeStorage::EVar::VAR_LC:
						if (var.name.empty()) {
//							os << '#' << var.uid << " = " << fae.varGet(j->second.second) << std::endl;
						} else {
							boost::unordered_map<size_t, Data>::iterator k = tmp.find(j->second.second);
							assert(k != tmp.end());
							os << '#' << var.uid << ':' << var.name << " = " << k->second << std::endl;
						}
						break;
					default:
						break;
				}
			
			}

			return os;

		}

	};
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
