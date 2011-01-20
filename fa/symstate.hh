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

#ifndef SYM_STATE_H
#define SYM_STATE_H

#include <vector>
#include <list>
#include <cassert>
#include <algorithm>

#include <boost/unordered_map.hpp>

#include <cl/storage.hh>

#include "treeaut.hh"
#include "forestautext.hh"
#include "ufae.hh"
#include "nodebuilder.hh"
#include "symctx.hh"
#include "builtintable.hh"
#include "utils.hh"

struct SymState {

	// configuration obtained in forward run
	TA<label_type> fwdConf;
	UFAE fwdConfWrapper;

//	TA<label_type> fixpoint;
//	UFAE fixpointWrapper;
	
//	boost::unordered_map<const FAE*, std::list<const FAE*>::iterator> confMap;

	// outstanding configurations
//	std::vector<FAE*> outConf;

	std::vector<const FAE*> fixpoint;

	const SymCtx* ctx;

	CodeStorage::Block::const_iterator insn;

	bool entryPoint;

	size_t absHeight;

	SymState(TA<label_type>::Backend& fwdBackend, TA<label_type>::Backend& fixpointBackend, BoxMan& boxMan)
		: fwdConf(fixpointBackend), fwdConfWrapper(this->fwdConf, boxMan)
		/*, fixpoint(fixpointBackend), fixpointWrapper(this->fixpoint, labMan)*/, absHeight(1) {}

	~SymState() {
//		utils::eraseMapFirst(this->confMap);
		utils::erase(this->fixpoint);
//		for (std::vector<FAE*>::iterator i = this->outConf.begin(); i != this->outConf.end(); ++i)
//			delete *i;
	}

	void extendFixpoint(const FAE* fae) {
		this->fixpoint.push_back(new FAE(*fae));
/*		TA<label_type> ta(*this->fixpoint.backend);
		Index<size_t> index;
		this->fixpointWrapper.fae2ta(ta, index, *fae);
		this->fixpointWrapper.join(ta, index);
		this->fixpoint.minimized(ta);
		this->fixpoint = ta;*/
		
/*		boost::unordered_map<const FAE*, std::list<const FAE*>::iterator>::iterator i = this->confMap.find(fae);
		assert(i != this->confMap.end());
		delete i->first;
		this->confMap.erase(i);*/
	}
/*
	void invalidate(const FAE* fae) {
		boost::unordered_map<const FAE*, std::list<const FAE*>::iterator>::iterator i = this->confMap.find(fae);
		assert(i != this->confMap.end());
		delete i->first;
		this->confMap.erase(i);
	}
*/
/*	void invalidate(std::list<const FAE*>& queue, const FAE* fae) {
		boost::unordered_map<const FAE*, std::list<const FAE*>::iterator>::iterator i = this->confMap.find(fae);
		assert(i != this->confMap.end());
		std::list<const FAE*>::iterator j = i->second;
		delete i->first;
		this->confMap.erase(i);
		if (j != queue.end())
			queue.erase(j);
	}
*/
	void recompute() {
		this->fwdConfWrapper.clear();
		this->fwdConf.clear();
		TA<label_type> ta(*this->fwdConf.backend);
		Index<size_t> index;
		for (std::vector<const FAE*>::iterator i = this->fixpoint.begin(); i != this->fixpoint.end(); ++i)
			this->fwdConfWrapper.fae2ta(ta, index, **i);
		if (!ta.getTransitions().empty()) {
			this->fwdConfWrapper.adjust(index);
			ta.minimized(this->fwdConf);
		}		
//		this->fwdConfWrapper.setStateOffset(this->fixpointWrapper.getStateOffset());
//		this->fwdConf = this->fixpoint;
	}
/*
	FAE* next() {
		if (this->outConf.empty())
			return NULL;
		FAE* fae = this->outConf.back();
		this->outConf.pop_back();
		return fae;
	}
*/
	void prepareOperand(std::vector<FAE*>& dst, const vector<FAE*>& src, const cl_operand& op) {
		std::vector<size_t> offs;
		NodeBuilder::buildNode(offs, op.type);
		std::vector<size_t> offs2(offs.size());
		OperandInfo oi;
		for (std::vector<FAE*>::const_iterator i = src.begin(); i != src.end(); ++i) {
			this->ctx->parseOperand(oi, **i, &op);
			if (oi.flag == o_flag_e::ref) {
				for (size_t j = 0; j < offs.size(); ++j)
					offs2[j] = offs[j] + oi.data.d_ref.displ;
				(*i)->isolateSet(dst, oi.data.d_ref.root, offs2);
				assert(dst.size());
			} else
				dst.push_back(new FAE(**i));
		}
	}

	void prepareOperandsGeneric(std::vector<FAE*>& dst, const FAE& fae) {
		std::vector<FAE*> tmp;
		ContainerGuard<vector<FAE*> > g(tmp);
		dst.push_back(new FAE(fae));
		for (std::vector<cl_operand>::const_iterator i = (*this->insn)->operands.begin(); i != (*this->insn)->operands.end(); ++i) {
			if (i->code != cl_operand_e::CL_OPERAND_VAR)
				continue;
			this->prepareOperand(tmp, dst, *i);
			utils::erase(dst);
			std::swap(dst, tmp);
		}
	}
/*
	void enqueue(std::list<const FAE*>& queue, const std::vector<FAE*>& src) {
		for (std::vector<FAE*>::const_iterator i = src.begin(); i != src.end(); ++i) {
//			this->confMap.insert(std::make_pair(*i, queue.insert(queue.end(), *i)));
			queue.push_back(*i);
			CL_CDEBUG("enqueued " << *i);
			CL_CDEBUG(std::endl << SymCtx::Dump(*this->ctx, **i));
			CL_CDEBUG(std::endl << **i);
		}
	}
*/
	void prepareFree(std::vector<FAE*>& dst, const FAE& fae) {
		std::vector<FAE*> tmp;
		ContainerGuard<vector<FAE*> > g(tmp);
		this->prepareOperandsGeneric(tmp, fae);
		for (std::vector<FAE*>::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			OperandInfo src;
			this->ctx->parseOperand(src, **i, &(*this->insn)->operands[2]);
			Data data = src.readData(**i, itov((size_t)0));
			if (!data.isRef() || (data.d_ref.displ != 0)) {
				dst.push_back(new FAE(**i));
			} else {
				const TypeBox* typeBox = fae.getType(data.d_ref.root);
				fae.isolateSet(dst, data.d_ref.root, typeBox->getSelectors());
			}
		}
	}

	// TODO: correctly unroll stack frame
	void prepareRet(std::vector<FAE*>& dst, const FAE& fae) {
		this->prepareOperandsGeneric(dst, fae);
	}

	void prepareOperands(std::vector<FAE*>& dst, const FAE& fae) {
		switch ((*this->insn)->code) {
			case cl_insn_e::CL_INSN_RET:
				this->prepareRet(dst, fae);
				return;

			case cl_insn_e::CL_INSN_CALL:
				assert((*this->insn)->operands[1].code == cl_operand_e::CL_OPERAND_CST);
				assert((*this->insn)->operands[1].data.cst.code == cl_type_e::CL_TYPE_FNC);
				switch (BuiltinTableStatic::data[(*this->insn)->operands[1].data.cst.data.cst_fnc.name]) {
					case builtin_e::biFree:
						this->prepareFree(dst, fae);
						break;
					default: {
						this->prepareOperandsGeneric(dst, fae);
						break;
					}
				}
				break;

			default: {
				this->prepareOperandsGeneric(dst, fae);
				break;
			}
		}
	}

	// careful
	void finalizeOperands(FAE& fae) {
//		OperandInfo oi;
		for (std::vector<cl_operand>::const_iterator i = (*this->insn)->operands.begin(); i != (*this->insn)->operands.end(); ++i) {
			if (
				(i == (*this->insn)->operands.begin()) &&
				(!i->accessor || (i->accessor->code != CL_ACCESSOR_DEREF)) &&
				((*this->insn)->code != cl_insn_e::CL_INSN_COND)
			)
				continue;
//			if (i->code != cl_operand_e::CL_OPERAND_VAR)
//				continue;
			size_t id;
			if (this->ctx->isReg(&*i, id))
				fae.varSet(id, Data::createUndef());
		}
	}

	bool testInclusion(const FAE& fae) {

		if (this->entryPoint) {

			TA<label_type> ta(*this->fwdConf.backend);
			Index<size_t> index;

			this->fwdConfWrapper.fae2ta(ta, index, fae);

//			CL_CDEBUG("challenge" << std::endl << ta);
//			CL_CDEBUG("response" << std::endl << this->fwdConf);

			if (TA<label_type>::subseteq(ta, this->fwdConf))
				return true;

//			CL_DEBUG("extending fixpoint with:" << std::endl << fae);

			this->fwdConfWrapper.join(ta, index);
			this->fwdConf.minimized(ta);
			this->fwdConf = ta;

		}
		
		return false;

	}
/*
	void selfCheck() {
		for (std::vector<FAE*>::iterator i = this->outConf.begin(); i != this->outConf.end(); ++i)
			(*i)->selfCheck();
	}
*/
};

#endif
