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

//#include "treeaut.hh"
//#include "forestaut.hh"
#include "forestautext.hh"
//#include "ufae.hh"
//#include "nodebuilder.hh"
//#include "operandinfo.hh"
#include "symctx.hh"
#include "symstate.hh"
//#include "tracerecorder.hh"
//#include "cfgstate.hh"
//#include "loopanalyser.hh"
//#include "builtintable.hh"

//#include "normalization.hh"
//#include "folding.hh"
//#include "virtualmachine.hh"
//#include "abstraction.hh"
//#include "reverserun.hh"
#include "executionmanager.hh"

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

/*
struct SymOp {

	SymState* src;
	SymState* dst;

};
*/
//#define CFG_FROM_FAE(fae) ((CfgState*)(assert(VirtualMachine(fae).varGet(IP_INDEX).isNativePtr()), VirtualMachine(fae).varGet(IP_INDEX).d_native_ptr))

//typedef enum { itDenormalize, itReverse }  tr_item_type;

class SymExec::Engine {

//	const CodeStorage::Storage& stor;

//	LoopAnalyser loopAnalyser;

	TA<label_type>::Backend taBackend;
	TA<label_type>::Backend fixpointBackend;
	BoxMan boxMan;

	std::vector<const Box*> boxes;
	std::vector<const Box*> basicBoxes;
	boost::unordered_map<const Box*, std::vector<const Box*> > hierarchy;

	Compiler compiler_;
	Compiler::Assembly assembly_;

	ExecutionManager execMan;

/*
	typedef unordered_map<const CodeStorage::Fnc*, SymCtx*> ctx_store_type;
	ctx_store_type ctxStore;

	typedef unordered_map<const CodeStorage::Insn*, CfgState*> state_store_type;
	state_store_type stateStore;

	std::list<SymState*> queue;

	const FAE* currentConf;
	const CodeStorage::Insn* currentInsn;
	
	TraceRecorder traceRecorder;
*/
	bool dbgFlag;

//	size_t statesEvaluated;
//	size_t tracesEvaluated;

protected:
/*
	SymCtx* getCtx(const CodeStorage::Fnc* fnc) {

		ctx_store_type::iterator i = this->ctxStore.find(fnc);
		if (i != this->ctxStore.end())
			return i->second;

		return this->ctxStore.insert(make_pair(fnc, new SymCtx(*fnc))).first->second;

	}

	CfgState* findCfgState(const CodeStorage::Insn* insn) {

		state_store_type::iterator i = this->stateStore.find(insn);
		assert(i != this->stateStore.end());
		return i->second;

	}

	CfgState* getCfgState(const CodeStorage::Block::const_iterator& insn, const SymCtx* ctx) {

		state_store_type::iterator i = this->stateStore.find(*insn);
		if (i != this->stateStore.end())
			return i->second;

		CfgState* s;
		
		if (this->loopAnalyser.isEntryPoint(*insn)) {
			s = new CfgStateExt(this->fixpointBackend, this->boxMan);
		} else {
			s = new CfgState();
		}

		s->insn = insn;
		s->ctx = ctx;
		
		return this->stateStore.insert(make_pair(*insn, s)).first->second;

	}

	struct ExactTMatchF {
		bool operator()(const TT<label_type>& t1, const TT<label_type>& t2) {
			return t1.label() == t2.label();
		}
	};

	struct SmartTMatchF {
		bool operator()(const TT<label_type>& t1, const TT<label_type>& t2) {
			if (t1.label()->isNode() && t2.label()->isNode())
				return t1.label()->getTag() == t2.label()->getTag();
			return t1.label() == t2.label();
		}
	};

	struct SmarterTMatchF {
		bool operator()(const TT<label_type>& t1, const TT<label_type>& t2) {
			if (t1.label()->isNode() && t2.label()->isNode()) {
				if (t1.label()->getTag() != t2.label()->getTag())
					return false;
				std::vector<size_t> tmp;
				for (std::vector<size_t>::const_iterator i = t1.lhs().begin(); i != t1.lhs().end(); ++i) {
					if (FA::isData(*i))
						tmp.push_back(*i);
				}
				size_t i = 0;
				for (std::vector<size_t>::const_iterator j = t2.lhs().begin(); j != t2.lhs().end(); ++j) {
					if (FA::isData(*j)) {
						if ((i >= tmp.size()) || (*j != tmp[i++]))
							return false;
					}
				}
				return (i == tmp.size());
			}
			return t1.label() == t2.label();
		}
	};

	bool foldBox(SymState* target, FAE& fae, size_t root, const Box* box) {
		CL_CDEBUG("trying " << *(const AbstractBox*)box << " at " << root);
		if (!fae.foldBox(root, box))
			return false;
		CL_CDEBUG("match");
		std::set<size_t> tmp;
		fae.getNearbyReferences(fae.varGet(ABP_INDEX).d_ref.root, tmp);
		FAE::NormInfo normInfo;
		fae.normalize(normInfo, tmp);
		boost::unordered_map<const Box*, std::vector<const Box*> >::iterator i =
			this->hierarchy.find(box);
		if (i == this->hierarchy.end())
			return true;
		this->recAbstractAndFold(target, fae, i->second);
		return true;
	}

	void recAbstractAndFold(SymState* target, FAE& fae, const std::vector<const Box*>& boxes) {

		CL_CDEBUG("abstracting and folding ... " << target->absHeight);
//		fae.heightAbstraction(target->absHeight, ExactLabelMatchF());
		CL_CDEBUG(std::endl << fae);

		// do not fold at 0
		for (size_t i = 1; i < fae.getRootCount(); ++i) {
			fae.heightAbstraction(i, target->absHeight, ExactLabelMatchF());
			for (std::vector<const Box*>::const_iterator j = boxes.begin(); j != boxes.end(); ++j) {
				if (this->foldBox(target, fae, i, *j))
					i = 1;
			}
		}

	}

	struct CompareVariablesF {
		bool operator()(size_t i, const TA<label_type>& ta1, const TA<label_type>& ta2) {
			if (i)
				return true;
			const TT<label_type>& t1 = ta1.getAcceptingTransition();
			const TT<label_type>& t2 = ta2.getAcceptingTransition();
			return (t1.label() == t2.label()) && (t1.lhs() == t2.lhs());
		}
	};

	struct FuseNonZeroF {
		bool operator()(size_t root, const FAE*) {
			return root != 0;
		}
	};

	void mergeFixpoint(CfgStateExt* target, FAE& fae) {
		std::vector<FAE*> tmp;
		ContainerGuard<std::vector<FAE*> > g(tmp);
		FAE::loadCompatibleFAs(tmp, target->fwdConf, this->taBackend, this->boxMan, &fae, 0, CompareVariablesF());
//		for (size_t i = 0; i < tmp.size(); ++i)
//			CL_CDEBUG("accelerator " << std::endl << *tmp[i]);
		fae.fuse(tmp, FuseNonZeroF());
//		fae.fuse(target->fwdConf, FuseNonZeroF());
		CL_CDEBUG("fused " << std::endl << fae);
	}

	void fold(CfgStateExt* target, FAE& fae) {

		bool matched = false;

		// do not fold at 0
		for (size_t i = 1; i < fae.getRootCount(); ++i) {
			for (std::vector<const Box*>::const_iterator j = this->boxes.begin(); j != this->boxes.end(); ++j) {
				CL_CDEBUG("trying " << *(const AbstractBox*)*j << " at " << i);
				if (Folding(fae).foldBox(i, *j)) {
					matched = true;
					CL_CDEBUG("match");
				}
			}
		}

		if (matched) {
			std::set<size_t> tmp;
			VirtualMachine vm(fae);
			vm.getNearbyReferences(vm.varGet(ABP_INDEX).d_ref.root, tmp);
//			NormInfo normInfo;
			Normalization(fae).normalize(tmp);
		}

	}

	void abstract(CfgStateExt* target, FAE& fae) {

//		this->recAbstractAndFold(target, fae, this->basicBoxes);

		CL_CDEBUG("abstracting ... " << target->absHeight);
		for (size_t i = 1; i < fae.getRootCount(); ++i)
			Abstraction(fae).heightAbstraction(i, target->absHeight, SmartTMatchF());

	}

	struct DestroySimpleF {

		DestroySimpleF() {}

		void operator()(SymState* state) {

			CfgState* cfgState = CFG_FROM_FAE(*state->fae);
			if (cfgState->hasExt)
				((CfgStateExt*)cfgState)->extendFixpoint(state->fae);
//			else
//				CFG_FROM_FAE(*node->fae)->invalidate(node->fae);			

		}

	};

	struct ExecInfo {

		SymState* parent;
		CfgState* cfg;
		FAE* fae;
		
		ExecInfo(SymState* parent, CfgState* cfg, FAE* fae)
			: parent(parent), cfg(cfg), fae(fae) {}

		const CodeStorage::Insn* insn() const {
			return *this->cfg->insn;
		}
		
	};

	void enqueue(ExecInfo& info) {

		VirtualMachine vm(*info.fae);

		vm.varSet(IP_INDEX, Data::createNativePtr((void*)info.cfg));

		if (!info.cfg->hasExt) {
			
			Normalization(*info.fae).check();

		} else {

			std::set<size_t> tmp;
			vm.getNearbyReferences(vm.varGet(ABP_INDEX).d_ref.root, tmp);
	
			Normalization(*info.fae).normalize(tmp);
	
			this->fold((CfgStateExt*)info.cfg, *info.fae);
			this->mergeFixpoint((CfgStateExt*)info.cfg, *info.fae);
			this->abstract((CfgStateExt*)info.cfg, *info.fae);
//			if (target->absHeight > 1)
//				fae->minimizeRootsCombo();

		}

		SymState* state = new SymState(info.parent, info.fae, this->queue.end(), (void*)1);

		state->queueTag = this->queue.insert(this->queue.end(), state);
		
		CL_CDEBUG("enqueued: " << state << std::endl << *state->fae);

	}

	void enqueueNextInsn(ExecInfo& info) {

		info.cfg->killDeadVariables(*info.fae, (*info.cfg->insn)->varsToKill);

		ExecInfo next(info.parent, this->getCfgState(info.cfg->insn + 1, info.cfg->ctx), info.fae);

		this->enqueue(next);
		
	}

	void execAssignment(ExecInfo& info) {

		OperandInfo dst, src;
		info.cfg->ctx->parseOperand(dst, *info.fae, &info.insn()->operands[0]);
		info.cfg->ctx->parseOperand(src, *info.fae, &info.insn()->operands[1]);

		assert(src.type->code == dst.type->code);

		RevInfo rev;

		if (
			src.type->code == cl_type_e::CL_TYPE_PTR &&
			src.type->items[0].type->code == cl_type_e::CL_TYPE_VOID &&
			dst.type->items[0].type->code != cl_type_e::CL_TYPE_VOID
		) {
			Data data = src.readData(*info.fae, itov((size_t)0));
			assert(data.isVoidPtr());
			if (dst.type->items[0].type->size != (int)data.d_void_ptr)
				throw ProgramError("allocated block's size mismatch");
			vector<SelData> sels;
			NodeBuilder::buildNode(sels, dst.type->items[0].type);
			std::string typeName;
			if (dst.type->items[0].type->name)
				typeName = std::string(dst.type->items[0].type->name);
			else {
				std::ostringstream ss;
				ss << dst.type->items[0].type->uid;
				typeName = ss.str();
			}
			dst.writeData(
				*info.fae,
				Data::createRef(VirtualMachine(*info.fae).nodeCreate(sels, this->boxMan.getTypeInfo(typeName))),
				rev
			);
		} else {
//			assert(*(src.type) == *(dst.type));
			vector<size_t> offs;
			NodeBuilder::buildNode(offs, dst.type);
			dst.writeData(*info.fae, src.readData(*info.fae, offs), rev);
		}

		this->enqueueNextInsn(info);
		
	}

	void execTruthNot(ExecInfo& info) {

		OperandInfo dst, src;
		info.cfg->ctx->parseOperand(dst, *info.fae, &info.insn()->operands[0]);
		info.cfg->ctx->parseOperand(src, *info.fae, &info.insn()->operands[1]);

		assert(dst.type->code == cl_type_e::CL_TYPE_BOOL);
		assert(src.type->code == cl_type_e::CL_TYPE_BOOL || src.type->code == cl_type_e::CL_TYPE_INT);

		vector<size_t> offs;
		NodeBuilder::buildNode(offs, src.type);

		Data data = src.readData(*info.fae, offs), res;

		switch (data.type) {
			case data_type_e::t_bool:
				res = Data::createBool(!data.d_bool);
				break;
			case data_type_e::t_int:
				res = Data::createBool(!data.d_int);
				break;
			default:
				assert(false);
		}

		RevInfo rev;

		dst.writeData(*info.fae, res, rev);

		this->enqueueNextInsn(info);
		
	}

	template <class F>
	static void dataCmp(vector<Data>& res, const Data& x, const Data& y, F f) {
		if ((x.isUnknw() || x.isUndef()) || (y.isUnknw() || y.isUndef())) {
			if ((float)random()/RAND_MAX < 0.5) {
				res.push_back(Data::createBool(false));
				res.push_back(Data::createBool(true));
			} else {
				res.push_back(Data::createBool(true));
				res.push_back(Data::createBool(false));
			}
		} else
			res.push_back(Data::createBool(f(x, y)));
	}

	template <class F>
	void execCmp(ExecInfo& info, F f) {

		OperandInfo dst, src1, src2;
		info.cfg->ctx->parseOperand(dst, *info.fae, &info.insn()->operands[0]);
		info.cfg->ctx->parseOperand(src1, *info.fae, &info.insn()->operands[1]);
		info.cfg->ctx->parseOperand(src2, *info.fae, &info.insn()->operands[2]);

//		assert(*src1.type == *src2.type);
		assert(OperandInfo::isLValue(dst.flag));
		assert(dst.type->code == cl_type_e::CL_TYPE_BOOL);

		vector<size_t> offs1;
		NodeBuilder::buildNode(offs1, src1.type);

		vector<size_t> offs2;
		NodeBuilder::buildNode(offs2, src2.type);

		Data data1 = src1.readData(*info.fae, offs1);
		Data data2 = src2.readData(*info.fae, offs2);
		vector<Data> res;
		Engine::dataCmp(res, data1, data2, f);
		RevInfo rev;

		for (vector<Data>::iterator j = res.begin(); j != res.end(); ++j) {

			ExecInfo newInfo(info.parent, info.cfg, new FAE(*info.fae));

			Guard<FAE> g(newInfo.fae);

			dst.writeData(*newInfo.fae, *j, rev);

			this->enqueueNextInsn(newInfo);

			g.release();

		}

		delete info.fae;

	}

	void execEq(SymState* state, const FAE* parent, const CodeStorage::Insn* insn, bool neg) {

		OperandInfo dst, src1, src2;
		state->ctx->parseOperand(dst, *parent, &insn->operands[0]);
		state->ctx->parseOperand(src1, *parent, &insn->operands[1]);
		state->ctx->parseOperand(src2, *parent, &insn->operands[2]);

//		assert(*src1.type == *src2.type);
		assert(OperandInfo::isLValue(dst.flag));
		assert(dst.type->code == cl_type_e::CL_TYPE_BOOL);

		vector<size_t> offs1;
		NodeBuilder::buildNode(offs1, src1.type);

		vector<size_t> offs2;
		NodeBuilder::buildNode(offs2, src2.type);

		Data data1 = src1.readData(*parent, offs1);
		Data data2 = src2.readData(*parent, offs2);
		vector<Data> res;
		Engine::dataEq(data1, data2, neg, res);
		RevInfo rev;
		for (vector<Data>::iterator j = res.begin(); j != res.end(); ++j) {
			FAE* fae = new FAE(*parent);
			Guard<FAE> g(fae);
			dst.writeData(*fae, *j, rev);
			g.release();
			this->enqueueNextInsn(state, parent, fae);
		}

	}

	void execPlus(ExecInfo& info) {

		OperandInfo dst, src1, src2;
		info.cfg->ctx->parseOperand(dst, *info.fae, &info.insn()->operands[0]);
		info.cfg->ctx->parseOperand(src1, *info.fae, &info.insn()->operands[1]);
		info.cfg->ctx->parseOperand(src2, *info.fae, &info.insn()->operands[2]);

		assert(dst.type->code == cl_type_e::CL_TYPE_INT);
		assert(src1.type->code == cl_type_e::CL_TYPE_INT);
		assert(src2.type->code == cl_type_e::CL_TYPE_INT);

		vector<size_t> offs1, offs2;
		NodeBuilder::buildNode(offs1, src1.type);
		NodeBuilder::buildNode(offs2, src2.type);

		Data data1 = src1.readData(*info.fae, offs1);
		Data data2 = src2.readData(*info.fae, offs2);
		assert(data1.isInt() && data2.isInt());
		Data res = Data::createInt((data1.d_int + data2.d_int > 0)?(1):(0));

		RevInfo rev;

		dst.writeData(*info.fae, res, rev);

		this->enqueueNextInsn(info);
		
	}

	void execPointerPlus(ExecInfo& info) {

		OperandInfo dst, src1, src2;
		info.cfg->ctx->parseOperand(dst, *info.fae, &info.insn()->operands[0]);
		info.cfg->ctx->parseOperand(src1, *info.fae, &info.insn()->operands[1]);
		info.cfg->ctx->parseOperand(src2, *info.fae, &info.insn()->operands[2]);

		assert(dst.type->code == cl_type_e::CL_TYPE_PTR);
		assert(src1.type->code == cl_type_e::CL_TYPE_PTR);
		assert(src2.type->code == cl_type_e::CL_TYPE_INT);

		vector<size_t> offs1, offs2;
		NodeBuilder::buildNode(offs1, src1.type);
		NodeBuilder::buildNode(offs2, src2.type);

		Data data1 = src1.readData(*info.fae, offs1);
		Data data2 = src2.readData(*info.fae, offs2);
		assert(data1.isRef() && data2.isInt());
		Data res = Data::createRef(data1.d_ref.root, data1.d_ref.displ + data2.d_int);

		RevInfo rev;

		dst.writeData(*info.fae, res, rev);

		this->enqueueNextInsn(info);
		
	}

	void execMalloc(ExecInfo& info) {

		OperandInfo dst, src;
		info.cfg->ctx->parseOperand(dst, *info.fae, &info.insn()->operands[0]);
		info.cfg->ctx->parseOperand(src, *info.fae, &info.insn()->operands[2]);
		assert(src.type->code == cl_type_e::CL_TYPE_INT);

		Data data = src.readData(*info.fae, itov((size_t)0));
		assert(data.isInt());
		RevInfo rev;

		if (
			dst.type->code == cl_type_e::CL_TYPE_PTR &&
			dst.type->items[0].type->code != cl_type_e::CL_TYPE_VOID
		) {
			if (dst.type->items[0].type->size != (int)data.d_int)
				throw ProgramError("allocated block's size mismatch");
			vector<SelData> sels;
			NodeBuilder::buildNode(sels, dst.type->items[0].type);
			std::string typeName;
			if (dst.type->items[0].type->name)
				typeName = std::string(dst.type->items[0].type->name);
			else {
				std::ostringstream ss;
				ss << dst.type->items[0].type->uid;
				typeName = ss.str();
			}
			dst.writeData(
				*info.fae,
				Data::createRef(VirtualMachine(*info.fae).nodeCreate(sels, this->boxMan.getTypeInfo(typeName))),
				rev
			);

		} else {
			dst.writeData(*info.fae, Data::createVoidPtr(data.d_int), rev);
		}
		
		this->enqueueNextInsn(info);
	
	}

	void execFree(ExecInfo& info) {

		OperandInfo src;
		info.cfg->ctx->parseOperand(src, *info.fae, &info.insn()->operands[2]);
		Data data = src.readData(*info.fae, itov((size_t)0));
		if (!data.isRef())
			throw ProgramError("releasing non-pointer value");
		if (data.d_ref.displ != 0)
			throw ProgramError("releasing a pointer which points inside the block");
		VirtualMachine(*info.fae).nodeDelete(data.d_ref.root);

		this->enqueueNextInsn(info);
		
	}

	void execNondet(ExecInfo& info) {

		OperandInfo dst;
		info.cfg->ctx->parseOperand(dst, *info.fae, &info.insn()->operands[0]);

		RevInfo rev;

		dst.writeData(*info.fae, Data::createUnknw(), rev);
		
		this->enqueueNextInsn(info);

	}

	void execJmp(ExecInfo& info) {

		ExecInfo newInfo(info.parent, this->getCfgState(info.insn()->targets[0]->begin(), info.cfg->ctx), info.fae);

		this->enqueue(newInfo);

	}

	void execCond(ExecInfo& info) {

		OperandInfo src;
		info.cfg->ctx->parseOperand(src, *info.fae, &info.insn()->operands[0]);

		assert(src.type->code == cl_type_e::CL_TYPE_BOOL);

		Data data = src.readData(*info.fae, itov((size_t)0));

		if (!data.isBool())
			throw runtime_error("Engine::execCond(): non boolean condition argument!");

		info.cfg->killDeadVariables(*info.fae, (*info.cfg->insn)->varsToKill);
		info.cfg->killDeadVariables(*info.fae, (*info.cfg->insn)->killPerTarget[((data.d_bool))?(0):(1)]);

		ExecInfo newInfo(info.parent, this->getCfgState(info.insn()->targets[((data.d_bool))?(0):(1)]->begin(), info.cfg->ctx), info.fae);

		this->enqueue(newInfo);

	}

	// TODO: implement proper return
	void execRet(ExecInfo& info) {

		bool b = info.cfg->ctx->destroyStackFrame(*info.fae);
		assert(!b);

		Normalization(*info.fae).check();
		
	}

	struct CmpEq {
		bool operator()(const Data& x, const Data& y) const {
			return x == y;
		}
	};

	struct CmpNeq {
		bool operator()(const Data& x, const Data& y) const {
			return x != y;
		}
	};

	struct CmpLt {
		bool operator()(const Data& x, const Data& y) const {
			if (x.isInt() && y.isInt())
				return x.d_int < y.d_int;
			if (x.isBool() && y.isBool())
				return x.d_bool < y.d_bool;
			throw std::runtime_error("SymExec::Engine::CmpLt(): comparison of the corresponding types not supported");
		}
	};

	void execInsn(ExecInfo& info) {

		const CodeStorage::Insn* insn = info.insn();

		switch (insn->code) {

			case cl_insn_e::CL_INSN_UNOP:
				switch (insn->subCode) {
					case cl_unop_e::CL_UNOP_ASSIGN:
						this->execAssignment(info);
						break;
					case cl_unop_e::CL_UNOP_TRUTH_NOT:
						this->execTruthNot(info);
						break;
					default:
						throw std::runtime_error("feature not implemented");
				}
				break;

			case cl_insn_e::CL_INSN_BINOP:
				switch (insn->subCode) {
					case cl_binop_e::CL_BINOP_EQ:
						this->execCmp(info, CmpEq());
						break;
					case cl_binop_e::CL_BINOP_NE:
						this->execCmp(info, CmpNeq());
						break;
					case cl_binop_e::CL_BINOP_LT:
						this->execCmp(info, CmpLt());
						break;
					case cl_binop_e::CL_BINOP_PLUS:
						this->execPlus(info);
						break;
//					case cl_binop_e::CL_BINOP_MINUS:
//						this->execMinus(state, parent, insn);
//						break;
					case cl_binop_e::CL_BINOP_POINTER_PLUS:
						this->execPointerPlus(info);
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
						this->execMalloc(info);
						break;
					case builtin_e::biFree:
						this->execFree(info);
						break;
					case builtin_e::biNondet:
						this->execNondet(info);
						break;
					default:
						throw std::runtime_error("feature not implemented");
				}
				break;

			case cl_insn_e::CL_INSN_RET:
				this->execRet(info);
				break;

			case cl_insn_e::CL_INSN_JMP:
				this->execJmp(info);
				break;

			case cl_insn_e::CL_INSN_COND:
				this->execCond(info);
				break;

			default:
				throw std::runtime_error("feature not implemented");

		}
				
	}

	struct InvalidateF {

		list<SymState*>& queue;
		set<CfgStateExt*>& s;
		
		InvalidateF(list<SymState*>& queue, set<CfgStateExt*>& s) : queue(queue), s(s) {}

		void operator()(SymState* state) {
			CfgState* cfgState = CFG_FROM_FAE(*state->fae);
			if (state->queueTag != this->queue.end())
				this->queue.erase(state->queueTag);
//			state->invalidate(this->queue, node->fae);
			if (cfgState->hasExt)
				s.insert((CfgStateExt*)cfgState);
		}

	};

	void processState(ExecInfo& info) {

		assert(info.fae);
		assert(info.cfg);

		this->currentConf = info.fae;
		this->currentInsn = info.insn();

		const cl_loc& loc = info.insn()->loc;
		CL_CDEBUG(loc << ' ' << *info.insn());
		CL_CDEBUG("processing " << info.fae);
		CL_CDEBUG(std::endl << SymCtx::Dump(*info.cfg->ctx, *info.fae));
		CL_CDEBUG(std::endl << *info.fae);

		this->execInsn(info);

	}

	void processState(SymState* state) {

		assert(state);

		CfgState* cfgState = CFG_FROM_FAE(*state->fae);

		if (cfgState->hasExt && ((CfgStateExt*)cfgState)->testInclusion(*state->fae)) {

			++this->tracesEvaluated;

			CL_CDEBUG("hit");

			this->traceRecorder.destroyBranch(state, DestroySimpleF());

			return;

		}

		state->queueTag = this->queue.end();

		try {

			const cl_loc& loc = (*cfgState->insn)->loc;

			CL_CDEBUG(loc << ' ' << **cfgState->insn);
			CL_CDEBUG("preprocessing " << state);
			CL_CDEBUG(std::endl << SymCtx::Dump(*cfgState->ctx, *state->fae));
			CL_CDEBUG(std::endl << *state->fae);

			std::vector<FAE*> tmp;

			ContainerGuard<std::vector<FAE*> > g(tmp);

			cfgState->prepareOperands(tmp, *state->fae);

			for (FAE*& fae : tmp) {

				ExecInfo info(state, cfgState, fae);

				this->processState(info);

				fae = NULL;

			}

			g.release();

		} catch (const ProgramError& e) {

			CL_CDEBUG(e.what());

			this->printTrace(state);

			throw;

			TraceRecorder::Item* item = this->revRun(*fae);

			if (!item)

				throw ProgramError(e.what(), &(*state->insn)->loc);

			CL_DEBUG("spurious counter example ...");

			this->printTrace(*fae);

			throw;

			state = STATE_FROM_FAE(*item->fae);

			assert(state->entryPoint);

			set<SymState*> s;

			this->traceRecorder.invalidateChildren(item, InvalidateF(this->queue, s));

			const FAE* tmp2 = item->fae;
			
			TraceRecorder::Item* parent = item->parent;

			InvalidateF(this->queue, s)(item);

			this->traceRecorder.remove(tmp2);

			assert(parent);

			parent->removeChild(item);

			for (set<SymState*>::iterator i = s.begin(); i != s.end(); ++i) {
				(*i)->recompute();
				CL_CDEBUG("new fixpoint:" << std::endl << (*i)->fwdConf);
			}

			const cl_loc& loc = (*state->insn)->loc;

			CL_CDEBUG("adjusting abstraction ... " << ++state->absHeight);
			CL_CDEBUG("resuming execution ... ");
			CL_CDEBUG(loc << ' ' << **state->insn);

			parent->queueTag = this->queue.insert(this->queue.end(), parent->fae);

		}

	}

	void printInfo(SymState* state) {
		if (this->dbgFlag) {
			CfgState* cfgState = CFG_FROM_FAE(*state->fae);
			assert(cfgState);
			if (!cfgState->hasExt)
				return;
			CL_DEBUG(std::endl << SymCtx::Dump(*((CfgStateExt*)cfgState)->ctx, *state->fae));
			CL_DEBUG(std::endl << *state->fae);
			CL_DEBUG("evaluated states: " << this->statesEvaluated << ", evaluated traces: " << this->tracesEvaluated);
			this->dbgFlag = false;
		}
	}
*/
/*
	void mainLoop() {

		while (!this->queue.empty()) {
//			const FAE* fae = this->queue.front();
//			this->queue.pop_front();
			SymState* state = this->queue.back();
			this->queue.pop_back();
			++this->statesEvaluated;
			this->printInfo(state);
			this->processState(state);
		}

	}
*/
/*
	void printTrace(SymState* state) {

		vector<pair<SymState*, const CodeStorage::Insn*> > trace;

		while (state) {

			trace.push_back(make_pair(state, *CFG_FROM_FAE(*state->fae)->insn));
			state = state->parent;

		}

//		trace.push_back(make_pair(item->fae, *state->insn));

		CL_DEBUG("trace:");

		for (auto i = trace.rbegin(); i != trace.rend(); ++i) {
			if (i->first->payload) {
				CfgState* state = CFG_FROM_FAE(*i->first->fae);
				CL_DEBUG(std::endl << SymCtx::Dump(*state->ctx, *i->first->fae));
				CL_DEBUG(std::endl << *i->first->fae);
				CL_NOTE_MSG(&i->second->loc, *(i->second));
			}
//			STATE_FROM_FAE(*i->first)->ctx->dumpContext(*i->first);
//			CL_CDEBUG(std::endl << *(i->first));
		}

//		state = STATE_FROM_FAE(fae);
//		CL_CDEBUG(std::endl << SymCtx::Dump(*state->ctx, fae));
//		CL_CDEBUG(std::endl << fae);
//		CL_NOTE_MSG(this->currentInsn->loc, *this->currentInsn);

	}
*/
/*
	TraceRecorder::Item* revRun(const FAE& fae) {

		CL_CDEBUG("reconstructing abstract trace ...");

		vector<pair<const FAE*, const CodeStorage::Insn*> > trace;

		TraceRecorder::Item* item = this->traceRecorder.find(&fae);

		FAE tmp(fae);

		SymState* state = NULL;
		
		while (item->parent) {

			CL_CDEBUG(std::endl << SymCtx::Dump(*STATE_FROM_FAE(*item->fae)->ctx, *item->fae));
			CL_CDEBUG(std::endl << tmp);

			state = STATE_FROM_FAE(*item->parent->fae);

			CL_CDEBUG("rewinding " << (*state->insn)->loc << ' ' << **state->insn);

			switch (item->itemType) {

				case tr_item_type::itDenormalize: {

					CL_CDEBUG("denormalizing " << std::endl << tmp << "with" << std::endl << *item->fae);
					CL_CDEBUG(item->normInfo);

					if (!Normalization(tmp).denormalize(*item->fae, item->normInfo)) {
						CL_CDEBUG("spurious counter example (denormalization)!" << std::endl << *item->fae);
						return item;
					}

					break;

				}

				case tr_item_type::itReverse: {

					CL_CDEBUG("reversing " << std::endl << tmp << "with" << std::endl << *item->parent->fae);

					if (!ReverseRun(tmp).reverse(*item->parent->fae)) {
						CL_CDEBUG("spurious counter example (reversal)!" << std::endl << *item->parent->fae);
						return item;
					}

					NormInfo normInfo;

					VirtualMachine vm(tmp);

					std::set<size_t> s;
					vm.getNearbyReferences(vm.varGet(ABP_INDEX).d_ref.root, s);
					Normalization(tmp).normalize(normInfo, s);

					break;

				}

			}

			if (item->itemType == tr_item_type::itDenormalize)
				trace.push_back(make_pair(item->fae, *state->insn));

			item = item->parent;

		}

		assert(state);

//		trace.push_back(make_pair(item->fae, *state->insn));

		CL_CDEBUG("trace:");

		for (vector<pair<const FAE*, const CodeStorage::Insn*> >::reverse_iterator i = trace.rbegin(); i != trace.rend(); ++i) {
			if (i->second)
				CL_NOTE_MSG(&i->second->loc, *(i->second));
//			STATE_FROM_FAE(*i->first)->ctx->dumpContext(*i->first);
//			CL_CDEBUG(std::endl << *(i->first));
		}

		CL_NOTE_MSG(&this->currentInsn->loc, *this->currentInsn);

		return NULL;

	}
*/
	void loadTypes(const CodeStorage::Storage& stor) {

	    CL_CDEBUG("loading types ...");

		for (auto type : stor.types) {
			if (type->code != cl_type_e::CL_TYPE_STRUCT)
				continue;
			std::string name;
			if (type->name)
				name = std::string(type->name);
			else {
				std::ostringstream ss;
				ss << type->uid;
				name = ss.str();
			}
				
			std::vector<size_t> v;
			NodeBuilder::buildNode(v, type);
			this->boxMan.createTypeInfo(type->name, v);
		}

	}
/*
	void printQueue() const {
		for (SymState* state : this->queue)
			std::cerr << *state->fae;
	}
*/
public:

	Engine() : boxMan(this->taBackend),
		compiler_(this->fixpointBackend, this->taBackend, this->boxMan, this->boxes),
		dbgFlag(false) {}
/*
	~Engine() {
		utils::eraseMap(this->stateStore);
		utils::eraseMap(this->ctxStore);
	}
*/
	void loadBoxes(const boost::unordered_map<std::string, std::string>& db) {

	    CL_DEBUG("loading boxes ...");

		for (boost::unordered_map<std::string, std::string>::const_iterator i = db.begin(); i != db.end(); ++i) {
			this->boxes.push_back((const Box*)this->boxMan.loadBox(i->first, db));
			CL_DEBUG(i->first << ':' << std::endl << *(const FA*)this->boxes.back());
		}

		this->boxMan.buildBoxHierarchy(this->hierarchy, this->basicBoxes);
		
	}

	void compile(const CodeStorage::Storage& stor) {

		CL_CDEBUG("compiling ...");

		this->compiler_.compile(this->assembly_, stor);

		CL_CDEBUG("assembly:" << std::endl << this->assembly_);		

		this->loadTypes(stor);

	}

	void run(const CodeStorage::Fnc& main) {

		assert(this->assembly_.code_.size());

		this->execMan.clear();
/*
	    CL_CDEBUG("calculating loop entry points ...");
		// compute loop entry points
		this->loopAnalyser.init(main.cfg.entry());
*/		
	    CL_CDEBUG("creating main context ...");
		// create main context
		SymCtx mainCtx(main);
/*
		SymCtx* mainCtx = this->getCtx(&main);

	    CL_CDEBUG("creating initial state ...");
		// create an initial state
		CfgState* init = this->getCfgState(main.cfg.entry()->begin(), mainCtx);
*/
	    CL_CDEBUG("creating empty heap ...");
		// create empty heap with no local variables
		std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(this->taBackend, this->boxMan));

	    CL_CDEBUG("allocating global registers ...");
		// add global registers
		SymCtx::init(*fae);

	    CL_CDEBUG("entering main stack frame ...");
		// enter main stack frame
		mainCtx.createStackFrame(*fae);

//		SymState* state = new SymState(NULL, new FAE(fae), this->queue.end(), (void*)1);

	    CL_CDEBUG("sheduling initial state ...");
		// schedule initial state for processing
		this->execMan.init(
			std::vector<Data>(this->assembly_.regFileSize_, Data::createUndef()), fae, this->assembly_.getEntry(&main)
		);
/*
		this->queue.push_back(state);

		state->queueTag = this->queue.begin();

		this->traceRecorder.init(state);

		this->statesEvaluated = 0;
		this->tracesEvaluated = 0;
*/
		try {

			AbstractInstruction::StateType state;

			while (this->execMan.dequeueBFS(state)) {

				this->execMan.execute(state);

			}

/*
			for (state_store_type::iterator i = this->stateStore.begin(); i != this->stateStore.end(); ++i) {
				if (!i->second->hasExt)
					continue;
				CL_DEBUG("fixpoint at " << (*i->second->insn)->loc);
				CL_DEBUG(std::endl << ((CfgStateExt*)i->second)->fwdConf);
//				Index<size_t> index;
//				i->second->fwdConf.buildStateIndex(index);
//				std::cerr << index << std::endl;
//				vector<vector<bool> > rel;
//				i->second->fwdConf.downwardSimulation(rel, index);
//				utils::relPrint(std::cerr, rel);
//				TA<label_type> ta(this->taBackend);
//				i->second->fwdConf.minimized(ta);
//				std::cerr << ta;
			}				

			CL_DEBUG("evaluated states: " << this->statesEvaluated << ", evaluated traces: " << this->tracesEvaluated);
*/
		} catch (std::exception& e) {
			CL_CDEBUG(e.what());
			throw;
		}
		
	}

	void setDbgFlag() {
		this->dbgFlag = 1;
	}	

};

SymExec::SymExec() : engine(new Engine()) {}

SymExec::~SymExec() {
	delete this->engine;
}

void SymExec::loadBoxes(const boost::unordered_map<std::string, std::string>& db) {
	this->engine->loadBoxes(db);
}

void SymExec::compile(const CodeStorage::Storage& stor) {
	this->engine->compile(stor);
}

void SymExec::run(const CodeStorage::Fnc& main) {
	this->engine->run(main);
}

void SymExec::setDbgFlag() {
	this->engine->setDbgFlag();
}
