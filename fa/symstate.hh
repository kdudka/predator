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
	
	boost::unordered_map<const FAE*, std::list<const FAE*>::iterator> confMap;

	// outstanding configurations
//	std::vector<FAE*> outConf;

	const SymCtx* ctx;

	CodeStorage::Block::const_iterator insn;

	bool entryPoint;

	size_t absHeight;

	SymState(TA<label_type>::Backend& taBackend, LabMan& labMan)
		: fwdConf(taBackend), fwdConfWrapper(this->fwdConf, labMan), absHeight(1) {}

	~SymState() {
		utils::eraseMapFirst(this->confMap);
//		utils::erase(this->outConf);
//		for (std::vector<FAE*>::iterator i = this->outConf.begin(); i != this->outConf.end(); ++i)
//			delete *i;
	}

	void invalidate(std::list<const FAE*>& queue, const FAE* fae) {
		boost::unordered_map<const FAE*, std::list<const FAE*>::iterator>::iterator i = this->confMap.find(fae);
		assert(i != this->confMap.end());
		std::list<const FAE*>::iterator j = i->second;
		delete i->first;
		this->confMap.erase(i);
		if (j != queue.end())
			queue.erase(j);
	}

	void recompute(std::list<const FAE*>& queue) {
		this->fwdConfWrapper.clear();
		this->fwdConf.clear();
		TA<label_type> ta(*this->fwdConf.backend);
		Index<size_t> index;
		for (boost::unordered_map<const FAE*, std::list<const FAE*>::iterator>::iterator i = this->confMap.begin(); i != this->confMap.end(); ++i)
			this->fwdConfWrapper.fae2ta(ta, index, *i->first);
		if (!ta.getTransitions().empty()) {
			this->fwdConfWrapper.adjust(index);
			ta.minimized(this->fwdConf);
		}		
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
		OperandInfo oi;
		for (std::vector<FAE*>::const_iterator i = src.begin(); i != src.end(); ++i) {
			this->ctx->parseOperand(oi, **i, &op);
			if (oi.flag == o_flag_e::ref)
				(*i)->isolateAtRoot(dst, oi.data.d_ref.root, FAE::IsolateSetF(offs, oi.data.d_ref.displ));
			else
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

	void enqueue(std::list<const FAE*>& queue, const std::vector<FAE*>& src) {
		for (std::vector<FAE*>::const_iterator i = src.begin(); i != src.end(); ++i) {
			this->confMap.insert(std::make_pair(*i, queue.insert(queue.end(), *i)));
			CL_DEBUG("enqueued " << *i);
			this->ctx->dumpContext(**i);
			CL_DEBUG(std::endl << **i);
		}
	}

	void prepareFree(std::list<const FAE*>& queue, const FAE& fae) {
		std::vector<FAE*> tmp;
		ContainerGuard<vector<FAE*> > g(tmp);
		this->prepareOperandsGeneric(tmp, fae);
		for (std::vector<FAE*>::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			OperandInfo src;
			std::vector<FAE*> tmp2;
			this->ctx->parseOperand(src, **i, &(*this->insn)->operands[2]);
			Data data = src.readData(**i, itov((size_t)0));
			if (!data.isRef() || (data.d_ref.displ != 0)) {
				tmp2.push_back(new FAE(**i));
				this->enqueue(queue, tmp2);
				continue;
			}
			fae.isolateAtRoot(tmp2, data.d_ref.root, FAE::IsolateAllF());
			this->enqueue(queue, tmp2);
		}
	}

	// TODO: correctly unroll stack frame
	void prepareRet(std::list<const FAE*>& queue, const FAE& fae) {
		std::vector<FAE*> tmp;
		ContainerGuard<vector<FAE*> > g(tmp);
		this->prepareOperandsGeneric(tmp, fae);
		g.release();
		this->enqueue(queue, tmp);
	}

	void prepareOperands(std::list<const FAE*>& queue, const FAE& fae) {
		switch ((*this->insn)->code) {
			case cl_insn_e::CL_INSN_RET:
				this->prepareRet(queue, fae);
				return;

			case cl_insn_e::CL_INSN_CALL:
				assert((*this->insn)->operands[1].code == cl_operand_e::CL_OPERAND_CST);
				assert((*this->insn)->operands[1].data.cst.code == cl_type_e::CL_TYPE_FNC);
				switch (BuiltinTableStatic::data[(*this->insn)->operands[1].data.cst.data.cst_fnc.name]) {
					case builtin_e::biFree:
						this->prepareFree(queue, fae);
						break;
					default: {
						std::vector<FAE*> tmp;
						ContainerGuard<vector<FAE*> > g(tmp);
						this->prepareOperandsGeneric(tmp, fae);
						g.release();
						this->enqueue(queue, tmp);
						break;
					}
				}
				break;

			default: {
				std::vector<FAE*> tmp;
				ContainerGuard<vector<FAE*> > g(tmp);
				this->prepareOperandsGeneric(tmp, fae);
				g.release();
				this->enqueue(queue, tmp);
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

	bool enqueue(std::list<const FAE*>& queue, const FAE& fae) {

		if (this->entryPoint) {

			TA<label_type> ta(*this->fwdConf.backend);
			Index<size_t> index;

			this->fwdConfWrapper.fae2ta(ta, index, fae);

//			CL_DEBUG("challenge" << std::endl << ta);
//			CL_DEBUG("response" << std::endl << this->fwdConf);

			if (TA<label_type>::subseteq(ta, this->fwdConf))
				return false;

			this->fwdConfWrapper.join(ta, index);

			ta = this->fwdConf;
			this->fwdConf.clear();
			ta.minimized(this->fwdConf);			

		}
		
		this->prepareOperands(queue, fae);

		return true;

	}
/*
	void selfCheck() {
		for (std::vector<FAE*>::iterator i = this->outConf.begin(); i != this->outConf.end(); ++i)
			(*i)->selfCheck();
	}
*/
};

#endif
