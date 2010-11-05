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

	SymState(TA<label_type>::Backend& taBackend, LabMan& labMan)
		: fwdConf(taBackend), fwdConfWrapper(this->fwdConf, labMan) {}

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
		this->confMap.erase(i);
		if (j != queue.end())
			queue.erase(j);
	}

	void recompute(std::list<const FAE*>& queue) {
		this->fwdConfWrapper.clear();
		this->fwdConf.clear();
		TA<label_type> ta(*this->fwdConf.backend);
		Index<size_t> index;
		for (boost::unordered_map<const FAE*, std::list<const FAE*>::iterator>::iterator i = this->confMap.begin(); i != this->confMap.end(); ++i) {
			if (i->second != queue.end())
				continue;
			this->fwdConfWrapper.fae2ta(ta, index, *i->first);
		}
		this->fwdConfWrapper.adjust(index);
		ta.minimized(this->fwdConf);			
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

	void prepareOperands(std::list<const FAE*>& queue, const FAE& fae) {
		std::vector<FAE*> tmp1, tmp2;
		ContainerGuard<vector<FAE*> > g1(tmp1), g2(tmp2);
		tmp1.push_back(new FAE(fae));
		for (std::vector<cl_operand>::const_iterator i = (*this->insn)->operands.begin(); i != (*this->insn)->operands.end(); ++i) {
			if (i->code != cl_operand_e::CL_OPERAND_VAR)
				continue;
			this->prepareOperand(tmp2, tmp1, *i);
			utils::erase(tmp1);
			std::swap(tmp1, tmp2);
		}
//		dst.insert(dst.end(), tmp1.begin(), tmp1.end());
		for (vector<FAE*>::iterator i = tmp1.begin(); i != tmp1.end(); ++i) {
//			this->outConf.push_back(*i);
			this->confMap.insert(std::make_pair(*i, queue.insert(queue.end(), *i)));
			CL_DEBUG("enqueued " << *i);
			this->ctx->dumpContext(**i);
			CL_DEBUG(std::endl << **i);
		}
		g1.release();
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
