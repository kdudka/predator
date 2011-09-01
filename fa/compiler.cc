/*
 * Copyright (C) 2011 Jiri Simacek
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

#include <list>
#include <sstream>
#include <cstdlib>

#include <cl/cldebug.hh>

#include "ufae.hh"

#include "symstate.hh"
#include "executionmanager.hh"
#include "splitting.hh"
#include "virtualmachine.hh"
#include "programerror.hh"
#include "loopanalyser.hh"
#include "abstraction.hh"
#include "normalization.hh"
#include "folding.hh"
#include "symctx.hh"
#include "operandinfo.hh"
#include "nodebuilder.hh"
#include "builtintable.hh"

#include "compiler.hh"

struct OpWrapper {

	const cl_operand* op_;
	
	OpWrapper(const cl_operand& op) : op_(&op) {}

	friend std::ostream& operator<<(std::ostream& os, const OpWrapper& op) {

		os << "operand: ";
		cltToStream(os, op.op_->type, false);
		os << ",";

		const cl_accessor* acc = op.op_->accessor;

		while (acc) {
			os << ' ' << acc->code << ": ";
			cltToStream(os, acc->type, false);
			acc = acc->next;
		}
	
		return os;
	
	}
	
};


class FI_jmp : public AbstractInstruction {

	const CodeStorage::Block* target_;
	AbstractInstruction* next_;

public:

	FI_jmp(const CodeStorage::Block* target, const CodeStorage::Insn* insn = NULL)
		: AbstractInstruction(insn, false, true), target_(target) {}

	virtual void execute(ExecutionManager&, const AbstractInstruction::StateType&) {

		assert(false);

	}

	static AbstractInstruction* getTarget(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex,
		const CodeStorage::Block* target
	) {
		auto tmp = codeIndex.find(target);
		assert(tmp != codeIndex.end());
		return tmp->second;
	}

	AbstractInstruction* getTarget(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex
	) {
		return getTarget(codeIndex, this->target_);
	}

	virtual void finalize(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex,
		std::vector<AbstractInstruction*>::const_iterator
	) {
		this->next_ = this->getTarget(codeIndex);
	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "jmp\t" << this->next_;
	}

};

class FI_ret : public AbstractInstruction {

public:

	FI_ret(const CodeStorage::Insn* insn = NULL) : AbstractInstruction(insn) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

		VirtualMachine vm(*fae);

		const Data& abp = vm.varGet(ABP_INDEX);

		assert(abp.isRef());
		assert(abp.d_ref.displ == 0);
		
		Data data;

		vm.nodeLookup(abp.d_ref.root, IP_OFFSET, data);

		assert(data.isNativePtr());

		AbstractInstruction* next = (AbstractInstruction*)data.d_native_ptr;

		vm.nodeLookup(abp.d_ref.root, ABP_OFFSET, data);
		vm.unsafeNodeDelete(abp.d_ref.root);
		vm.varSet(ABP_INDEX, data);

		Normalization(*fae).check();

		if (next)
			execMan.enqueue(state.second, state.first, fae, next);

	}

	virtual void finalize(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>&,
		std::vector<AbstractInstruction*>::const_iterator
	) {}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "ret";
	}

};

class FI_cond : public AbstractInstruction {

	size_t src_;

	AbstractInstruction* next_[2];

public:

	FI_cond(size_t src, AbstractInstruction* next[2], const CodeStorage::Insn* insn = NULL)
		: AbstractInstruction(insn), src_(src), next_({ next[0], next[1] }) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		assert((*state.first)[this->src_].isBool());

		execMan.enqueue(state, this->next_[((*state.first)[this->src_].d_bool)?(0):(1)]);

	}

	virtual void finalize(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex,
		std::vector<AbstractInstruction*>::const_iterator
	) {

		for (auto i : { 0, 1 }) {

			if (this->next_[i]->isJump())
				this->next_[i] = ((FI_jmp*)this->next_[i])->getTarget(codeIndex);

		}
		
	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "cjmp\tr" << this->src_ << ", " << this->next_[0] << ", " << this->next_[1];
	}

};

class SequentialInstruction : public AbstractInstruction {

protected:

	AbstractInstruction* next_;

public:

	SequentialInstruction(const CodeStorage::Insn* insn, bool computesFixpoint = false)
		: AbstractInstruction(insn, computesFixpoint) {}

	virtual void finalize(
		const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex,
		std::vector<AbstractInstruction*>::const_iterator cur
	) {

		this->next_ = *(cur + 1);

		// shorcut jump instruction
		if (this->next_->isJump())
			this->next_ = ((FI_jmp*)this->next_)->getTarget(codeIndex);
		
	}
	
};

class FixpointInstruction : public SequentialInstruction {

public:

	FixpointInstruction() : SequentialInstruction(NULL, true) {}

	virtual void extendFixpoint(const std::shared_ptr<const FAE>& fae) = 0;

};

class FI_fix : public FixpointInstruction {

	// configuration obtained in forward run
	TA<label_type> fwdConf;

	UFAE fwdConfWrapper;

	std::vector<std::shared_ptr<const FAE>> fixpoint;

	TA<label_type>::Backend& taBackend;

	BoxMan& boxMan;
	
	const std::vector<const Box*>& boxes;

public:

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

	struct DestroySimpleF {

		DestroySimpleF() {}

		void operator()(SymState* state) {

			AbstractInstruction* instr = state->instr;
			if (instr->computesFixpoint())
				((FixpointInstruction*)instr)->extendFixpoint(state->fae);
//			else
//				CFG_FROM_FAE(*node->fae)->invalidate(node->fae);			

		}

	};

public:

	virtual void extendFixpoint(const std::shared_ptr<const FAE>& fae) {
		this->fixpoint.push_back(fae);
	}

	void recompute() {
		this->fwdConfWrapper.clear();
		this->fwdConf.clear();
		TA<label_type> ta(*this->fwdConf.backend);
		Index<size_t> index;

		for (auto fae : this->fixpoint)
			this->fwdConfWrapper.fae2ta(ta, index, *fae);

		if (!ta.getTransitions().empty()) {
			this->fwdConfWrapper.adjust(index);
			ta.minimized(this->fwdConf);
		}		
//		this->fwdConfWrapper.setStateOffset(this->fixpointWrapper.getStateOffset());
//		this->fwdConf = this->fixpoint;
	}

	bool testInclusion(FAE& fae) {

		TA<label_type> ta(*this->fwdConf.backend);
		Index<size_t> index;

		this->fwdConfWrapper.fae2ta(ta, index, fae);

//		CL_CDEBUG("challenge" << std::endl << ta);
//		CL_CDEBUG("response" << std::endl << this->fwdConf);

		if (TA<label_type>::subseteq(ta, this->fwdConf))
			return true;

//		CL_CDEBUG("extending fixpoint with:" << std::endl << fae);

		this->fwdConfWrapper.join(ta, index);
		ta.clear();
		this->fwdConf.minimized(ta);
		this->fwdConf = ta;

		return false;

	}

	void mergeFixpoint(FAE& fae) {
		std::vector<FAE*> tmp;
		ContainerGuard<std::vector<FAE*> > g(tmp);
		FAE::loadCompatibleFAs(tmp, this->fwdConf, this->taBackend, this->boxMan, &fae, 0, CompareVariablesF());
//		for (size_t i = 0; i < tmp.size(); ++i)
//			CL_CDEBUG("accelerator " << std::endl << *tmp[i]);
		fae.fuse(tmp, FuseNonZeroF());
//		fae.fuse(target->fwdConf, FuseNonZeroF());
		CL_CDEBUG("fused " << std::endl << fae);
	}

	bool fold(FAE& fae) {

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

		return matched;

	}

	void abstract(FAE& fae) {

		CL_CDEBUG("abstracting ... " << 1);
		for (size_t i = 1; i < fae.getRootCount(); ++i)
			Abstraction(fae).heightAbstraction(i, 1, SmartTMatchF());

	}

public:

	FI_fix(TA<label_type>::Backend& fixpointBackend, TA<label_type>::Backend& taBackend,
		BoxMan& boxMan, const std::vector<const Box*>& boxes)
		: FixpointInstruction(), fwdConf(fixpointBackend), fwdConfWrapper(this->fwdConf, boxMan),
		taBackend(taBackend), boxMan(boxMan), boxes(boxes) {}

	virtual ~FI_fix() {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

		VirtualMachine vm(*fae);
		Normalization norm(*fae);

		std::set<size_t> tmp;

		vm.getNearbyReferences(vm.varGet(ABP_INDEX).d_ref.root, tmp);
	
		norm.normalize(tmp);
		if (this->fold(*fae))
			norm.normalize(tmp);

		if (this->testInclusion(*fae)) {
			CL_CDEBUG("hit");
			execMan.traceFinished(state.second, DestroySimpleF());
			return;
		}

		this->mergeFixpoint(*fae);
		this->abstract(*fae);

		execMan.enqueue(state.second, state.first, fae, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "fix";
	}

};

class FI_check : public SequentialInstruction {

public:

	FI_check() : SequentialInstruction(NULL) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		Normalization((FAE&)*state.second->fae).check();

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "check";
	}

};

class FI_acc_sel : public SequentialInstruction {

	size_t dst_;
	size_t offset_;

public:

	FI_acc_sel(size_t dst, size_t offset)
		: SequentialInstruction(NULL), dst_(dst), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		auto data = (*state.first)[this->dst_];

		if (data == VirtualMachine(*state.second->fae).varGet(ABP_INDEX)) {
			execMan.enqueue(state, this->next_);
			return;
		}		

		if (!data.isRef()) {

			std::stringstream ss;
			ss << "dereferenced value is not a valid reference [" << data << ']';
			throw ProgramError(ss.str());

		}

		std::vector<FAE*> dst;

		Splitting(*state.second->fae).isolateOne(dst, data.d_ref.root, data.d_ref.displ + this->offset_);

		for (auto fae : dst)
			execMan.enqueue(state.second, execMan.allocRegisters(*state.first), std::shared_ptr<const FAE>(fae), this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "acc\t[r" << this->dst_ << " + " << this->offset_ << "]";
	}

};

class FI_acc_set : public SequentialInstruction {

	size_t dst_;
	int base_;
	std::vector<size_t> offsets_;

public:

	FI_acc_set(size_t dst, int base, const std::vector<size_t>& offsets)
		: SequentialInstruction(NULL), dst_(dst), base_(base), offsets_(offsets) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		auto data = (*state.first)[this->dst_];

		if (data == VirtualMachine(*state.second->fae).varGet(ABP_INDEX)) {
			execMan.enqueue(state, this->next_);
			return;
		}		

		if (!data.isRef()) {

			std::stringstream ss;
			ss << "dereferenced value is not a valid reference [" << data << ']';
			throw ProgramError(ss.str());

		}

		std::vector<FAE*> dst;

		Splitting(*state.second->fae).isolateSet(
			dst, data.d_ref.root, data.d_ref.displ + this->base_, this->offsets_
		);

		for (auto fae : dst)
			execMan.enqueue(state.second, execMan.allocRegisters(*state.first), std::shared_ptr<const FAE>(fae), this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return utils::printCont(os << "acc\t[r" << this->dst_ << " + " << this->base_ << " + ", this->offsets_) << ']';
	}

};

class FI_acc_all : public SequentialInstruction {

	size_t dst_;

public:

	FI_acc_all(size_t dst)
		: SequentialInstruction(NULL), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		auto data = (*state.first)[this->dst_];

		if (data == VirtualMachine(*state.second->fae).varGet(ABP_INDEX)) {
			execMan.enqueue(state, this->next_);
			return;
		}		

		if (!data.isRef()) {

			std::stringstream ss;
			ss << "dereferenced value is not a valid reference [" << data << ']';
			throw ProgramError(ss.str());

		}

		std::vector<FAE*> dst;

		Splitting(*state.second->fae).isolateSet(
			dst, data.d_ref.root, 0, state.second->fae->getType(data.d_ref.root)->getSelectors()
		);

		for (auto fae : dst)
			execMan.enqueue(state.second, execMan.allocRegisters(*state.first), std::shared_ptr<const FAE>(fae), this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "acca\t[r" << this->dst_ << ']';
	}

};

class FI_load_cst : public SequentialInstruction {

	size_t dst_;
	Data data_;

public:

	FI_load_cst(size_t dst, const Data& data)
		: SequentialInstruction(NULL), dst_(dst), data_(data) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		(*state.first)[this->dst_] = this->data_;

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov\tr" << this->dst_ << ", " << this->data_;
	}

};

class FI_move_reg : public SequentialInstruction {

	size_t dst_;
	size_t src_;

public:

	FI_move_reg(size_t dst, size_t src)
		: SequentialInstruction(NULL), dst_(dst), src_(src) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		(*state.first)[this->dst_] = (*state.first)[this->src_];

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov\tr" << this->dst_ << ", r" << this->src_;
	}

};

class FI_bnot : public SequentialInstruction {

	size_t dst_;

public:

	FI_bnot(size_t dst) : SequentialInstruction(NULL), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		assert((*state.first)[this->dst_].isBool());

		(*state.first)[this->dst_] = Data::createBool(!(*state.first)[this->dst_].d_bool);

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "not\tr" << this->dst_;
	}

};

class FI_inot : public SequentialInstruction {

	size_t dst_;

public:

	FI_inot(size_t dst) : SequentialInstruction(NULL), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		assert((*state.first)[this->dst_].isInt());

		(*state.first)[this->dst_] = Data::createBool(!(*state.first)[this->dst_].d_int);

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "not\tr" << this->dst_;
	}

};

class FI_move_reg_offs : public SequentialInstruction {

	size_t dst_;
	size_t src_;
	int offset_;

public:

	FI_move_reg_offs(size_t dst, size_t src, int offset)
		: SequentialInstruction(NULL), dst_(dst), src_(src), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		auto data = (*state.first)[this->src_];

		if (!data.isRef()) {

			std::stringstream ss;
			ss << "dereferenced value is not a valid reference [" << data << ']';
			throw ProgramError(ss.str());

		}

		(*state.first)[this->dst_] = data;
		(*state.first)[this->dst_].d_ref.displ += this->offset_;

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov\tr" << this->dst_ << ", r" << this->src_ << " + " << this->offset_;
	}

};

class FI_move_reg_inc : public SequentialInstruction {

	size_t dst_;
	size_t src1_;
	size_t src2_;

public:

	FI_move_reg_inc(size_t dst, size_t src1, size_t src2)
		: SequentialInstruction(NULL), dst_(dst), src1_(src1), src2_(src2) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		auto data = (*state.first)[this->src1_];

		if (!data.isRef()) {

			std::stringstream ss;
			ss << "dereferenced value is not a valid reference [" << data << ']';
			throw ProgramError(ss.str());

		}

		(*state.first)[this->dst_] = data;
		(*state.first)[this->dst_].d_ref.displ += (*state.first)[this->src2_].d_int;

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov\tr" << this->dst_ << ", r" << this->src1_ << " + r" << this->src2_;
	}

};

class FI_move_sreg : public SequentialInstruction {

	size_t dst_;
	size_t src_;

public:

	FI_move_sreg(size_t dst, size_t src)
		: SequentialInstruction(NULL), dst_(dst), src_(src) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		(*state.first)[this->dst_] = VirtualMachine(*state.second->fae).varGet(this->src_);

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov\tr" << this->dst_ << ", sr" << this->src_;
	}

};

class FI_move_ABP : public SequentialInstruction {

	size_t dst_;
	int offset_;

public:

	FI_move_ABP(size_t dst, int offset)
		: SequentialInstruction(NULL), dst_(dst), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		(*state.first)[this->dst_] = VirtualMachine(*state.second->fae).varGet(ABP_INDEX);
		(*state.first)[this->dst_].d_ref.displ += this->offset_;

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov\tr" << this->dst_ << ", ABP + " << this->offset_;
	}

};

class FI_is_type : public SequentialInstruction {

	size_t dst_;
	data_type_e type_;

public:

	FI_is_type(size_t dst, data_type_e type)
		: SequentialInstruction(NULL), dst_(dst), type_(type) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		if (!(*state.first)[this->dst_].type != this->type_) {

			std::stringstream ss;
			ss << "unexpected data type: " << (*state.first)[this->dst_];
			throw ProgramError(ss.str());

		}

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "istype\tr" << this->dst_ << ", " << this->type_;
	}

};

class FI_is_ref : public SequentialInstruction {

	size_t dst_;

public:

	FI_is_ref(size_t dst)
		: SequentialInstruction(NULL), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		if (!(*state.first)[this->dst_].isRef()) {

			std::stringstream ss;
			ss << "dereferenced value is not a valid reference [" << (*state.first)[this->dst_] << ']';
			throw ProgramError(ss.str());

		}

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "isref\tr" << this->dst_;
	}

};
/*
class FI_inc_off : public SequentialInstruction {

	size_t dst_;
	size_t src_;
	int offset_;

public:

	FI_inc_off(size_t dst, size_t src, int offset)
		: SequentialInstruction(NULL), dst_(dst), src_(src), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		assert((*state.first)[this->src_].isRef());

		(*state.first)[this->dst_] = Data::createRef(
			(*state.first)[this->src_].d_ref.root,
			(*state.first)[this->src_].d_ref.offset + this->offset_
		);

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) {
		return os << "inc\tr" << this->dst_ << ", " << this->offset_;
	}

};
*/
class FI_load : public SequentialInstruction {

	size_t dst_;
	int offset_;

public:

	FI_load(size_t dst, int offset)
		: SequentialInstruction(NULL), dst_(dst), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		assert((*state.first)[this->dst_].isRef());

		const Data& data = (*state.first)[this->dst_];

		VirtualMachine(*state.second->fae).nodeLookup(
			data.d_ref.root, data.d_ref.displ + this->offset_, (*state.first)[this->dst_]
		);

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov\tr" << this->dst_ << ", [r" << this->dst_ << " + " << this->offset_ << ']';
	}

};

class FI_load_ABP : public SequentialInstruction {

	size_t dst_;
	int offset_;

public:

	FI_load_ABP(size_t dst, int offset)
		: SequentialInstruction(NULL), dst_(dst), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		VirtualMachine vm(*state.second->fae);

		const Data& data = vm.varGet(ABP_INDEX);

		vm.nodeLookup(data.d_ref.root, (size_t)this->offset_, (*state.first)[this->dst_]);

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov\tr" << this->dst_ << ", [ABP + " << this->offset_ << ']';
	}

};

class FI_store : public SequentialInstruction {

	size_t dst_;
	size_t src_;
	int offset_;

public:

	FI_store(size_t dst, size_t src, int offset)
		: SequentialInstruction(NULL), dst_(dst), src_(src), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		assert((*state.first)[this->dst_].isRef());

		std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

		const Data& dst = (*state.first)[this->dst_];
		const Data& src = (*state.first)[this->src_];

		Data out;

		VirtualMachine(*fae).nodeModify(
			dst.d_ref.root, dst.d_ref.displ + this->offset_, src, out
		);

		execMan.enqueue(state.second, state.first, fae, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov\t[r" << this->dst_ << " + " << this->offset_ << "], r" << this->src_;
	}

};

class FI_store_ABP : public SequentialInstruction {

	size_t src_;
	int offset_;

public:

	FI_store_ABP(size_t src, int offset)
		: SequentialInstruction(NULL), src_(src), offset_(offset) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

		VirtualMachine vm(*fae);

		const Data& data = vm.varGet(ABP_INDEX);

		Data out;

		vm.nodeModify(data.d_ref.root, this->offset_, (*state.first)[this->src_], out);

		execMan.enqueue(state.second, state.first, fae, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "mov\t[ABP + " << this->offset_ << "], r" << this->src_;
	}

};

class FI_loads : public SequentialInstruction {

	size_t dst_;
	int base_;
	std::vector<size_t> offsets_;

public:

	FI_loads(size_t dst, int base, const std::vector<size_t>& offsets)
		: SequentialInstruction(NULL), dst_(dst), base_(base), offsets_(offsets) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		assert((*state.first)[this->dst_].isRef());

		const Data& data = (*state.first)[this->dst_];

		VirtualMachine(*state.second->fae).nodeLookupMultiple(
			data.d_ref.root, data.d_ref.displ + this->base_, this->offsets_, (*state.first)[this->dst_]
		);

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return utils::printCont(os << "mov\tr" << this->dst_ << ", [r" << this->dst_ << " + " << this->base_ << " + ", this->offsets_) << ']';
	}

};

class FI_stores : public SequentialInstruction {

	size_t dst_;
	size_t src_;
	int base_;
	std::vector<size_t> offsets_;

public:

	FI_stores(size_t dst, size_t src, int base, const std::vector<size_t>& offsets)
		: SequentialInstruction(NULL), dst_(dst), src_(src), base_(base), offsets_(offsets) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		assert((*state.first)[this->dst_].isRef());

		std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

		const Data& dst = (*state.first)[this->dst_];
		const Data& src = (*state.first)[this->src_];

		Data out;

		VirtualMachine(*fae).nodeModifyMultiple(
			dst.d_ref.root, dst.d_ref.displ + this->base_, src, out
		);

		execMan.enqueue(state.second, state.first, fae, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return utils::printCont(os << "mov\t[r" << this->dst_ << " + " << this->base_ << " + ", this->offsets_) << "], r" << this->src_;
	}

};

class FI_alloc : public SequentialInstruction {

	size_t dst_;
	size_t src_;

public:

	FI_alloc(size_t dst, size_t src)
		: SequentialInstruction(NULL), dst_(dst), src_(src) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		assert((*state.first)[this->src_].isInt());

		(*state.first)[this->dst_] =
			Data::createVoidPtr((*state.first)[this->src_].d_int);
		
		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "alloc\tr" << this->dst_ << ", r" << this->src_;
	}

};

class FI_node_create : public SequentialInstruction {

	BoxMan& boxMan_;
	size_t dst_;
	size_t src_;
	const cl_type* type_;

public:

	FI_node_create(BoxMan& boxMan, size_t dst, size_t src, const cl_type* type)
		: SequentialInstruction(NULL), boxMan_(boxMan), dst_(dst), src_(src), type_(type) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		assert((*state.first)[this->src_].isVoidPtr());

		if ((int)(*state.first)[this->src_].d_void_ptr != this->type_->size)
			throw ProgramError("allocated block size mismatch");

		std::vector<SelData> sels;
		NodeBuilder::buildNode(sels, this->type_);

		std::string typeName;
		if (this->type_->name)
			typeName = std::string(this->type_->name);
		else {
			std::ostringstream ss;
			ss << this->type_->uid;
			typeName = ss.str();
		}

		std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

		(*state.first)[this->dst_] = Data::createRef(
			VirtualMachine(*fae).nodeCreate(sels, this->boxMan_.getTypeInfo(typeName))
		);

		execMan.enqueue(state.second, state.first, fae, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "node\tr" << this->dst_ << ", r" << this->src_;
	}

};

class FI_node_alloc : public SequentialInstruction {

	BoxMan& boxMan_;
	size_t dst_;
	size_t src_;
	const cl_type* type_;

public:

	FI_node_alloc(BoxMan& boxMan, size_t dst, size_t src, const cl_type* type)
		: SequentialInstruction(NULL), boxMan_(boxMan), dst_(dst), src_(src), type_(type) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		assert((*state.first)[this->src_].isInt());

		if ((*state.first)[this->src_].d_int != this->type_->size)
			throw ProgramError("allocated block size mismatch");

		std::vector<SelData> sels;
		NodeBuilder::buildNode(sels, this->type_);

		std::string typeName;
		if (this->type_->name)
			typeName = std::string(this->type_->name);
		else {
			std::ostringstream ss;
			ss << this->type_->uid;
			typeName = ss.str();
		}

		std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

		(*state.first)[this->dst_] = Data::createRef(
			VirtualMachine(*fae).nodeCreate(sels, this->boxMan_.getTypeInfo(typeName))
		);

		execMan.enqueue(state.second, state.first, fae, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "alloc\tr" << this->dst_ << ", r" << this->src_;
	}

};

class FI_node_free : public SequentialInstruction {

	size_t dst_;

public:

	FI_node_free(size_t dst)
		: SequentialInstruction(NULL), dst_(dst) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		assert((*state.first)[this->dst_].isRef());

		std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

		const Data& data = (*state.first)[this->dst_];

		if (data.d_ref.displ != 0)
			throw ProgramError("releasing a pointer which points inside an allocated block");

		VirtualMachine(*fae).nodeDelete(data.d_ref.root);

		execMan.enqueue(state.second, state.first, fae, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "free\tr" << this->dst_;
	}

};

struct Eq {
	bool operator()(const Data& x, const Data& y) const {
		return x == y;
	}
	const char* name() const {
		return "eq";
	}
};

struct Neq {
	bool operator()(const Data& x, const Data& y) const {
		return x != y;
	}
	const char* name() const {
		return "neq";
	}
};

struct Lt {
	bool operator()(const Data& x, const Data& y) const {
		if (x.isInt() && y.isInt())
			return x.d_int < y.d_int;
		if (x.isBool() && y.isBool())
			return x.d_bool < y.d_bool;
		throw std::runtime_error("SymExec::Engine::CmpLt(): comparison of the corresponding types not supported");
	}
	const char* name() const {
		return "lt";
	}
};

template <class F>
class FI_cmp : public SequentialInstruction {

	size_t dst_;
	size_t src1_;
	size_t src2_;

	static void dataCmp(std::vector<bool>& res, const Data& x, const Data& y) {

		if ((x.isUnknw() || x.isUndef()) || (y.isUnknw() || y.isUndef())) {

			if ((float)random()/RAND_MAX < 0.5) {
				res.push_back(false);
				res.push_back(true);
			} else {
				res.push_back(true);
				res.push_back(false);
			}

		} else {

			res.push_back(F()(x, y));

		}

	}

public:

	FI_cmp(size_t dst, size_t src1, size_t src2)
		: SequentialInstruction(NULL), dst_(dst), src1_(src1), src2_(src2) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		std::vector<bool> res;

		FI_cmp::dataCmp(res, (*state.first)[this->src1_], (*state.first)[this->src2_]);

		for (auto v : res) {

			std::shared_ptr<std::vector<Data>> regs = execMan.allocRegisters(*state.first);

			(*regs)[this->dst_] = Data::createBool(v);
			
			execMan.enqueue(state.second, regs, state.second->fae, this->next_);

		}

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << F().name() << "\tr" << this->dst_ << ", r" << this->src1_ << ", r" << this->src2_;
	}

};

class FI_iadd : public SequentialInstruction {

	size_t dst_;
	size_t src1_;
	size_t src2_;

public:

	FI_iadd(size_t dst, size_t src1, size_t src2)
		: SequentialInstruction(NULL), dst_(dst), src1_(src1), src2_(src2) {}

	virtual void execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

		assert((*state.first)[this->src1_].isInt() && (*state.first)[this->src2_].isInt());

		(*state.first)[this->dst_] = Data::createInt(
			((*state.first)[this->src1_].d_int + (*state.first)[this->src2_].d_int > 0)?(1):(0)
		);

		execMan.enqueue(state, this->next_);

	}

	virtual std::ostream& toStream(std::ostream& os) const {
		return os << "iadd\tr" << this->dst_ << ", r" << this->src1_ << ", r" << this->src2_;
	}

};

class Compiler::Core {

	Compiler::Assembly* assembly;
	std::unordered_map<const CodeStorage::Block*, AbstractInstruction*> codeIndex;
	const SymCtx* curCtx;

	TA<label_type>::Backend& fixpointBackend;
	TA<label_type>::Backend& taBackend;
	BoxMan& boxMan;
	const std::vector<const Box*>& boxes;

	BuiltinTable builtinTable;
	LoopAnalyser loopAnalyser;

protected:

	void reset(Compiler::Assembly& assembly) {

		this->assembly = &assembly;
		this->assembly->clear();
		this->codeIndex.clear();

	}

	void append(AbstractInstruction* instr) {

		this->assembly->code_.push_back(instr);

	}

	void cLoadCst(size_t dst, const cl_operand& op) {
		
		switch (op.data.cst.code) {

			case cl_type_e::CL_TYPE_INT:
				this->append(
					new FI_load_cst(dst, Data::createInt(op.data.cst.data.cst_int.value))
				);
				break;

			default:
				assert(false);

		}

	}

	void cLoadVar(size_t dst, size_t offset) {

		this->append(new FI_load_ABP(dst, (int)offset));

	}

	void cMoveReg(size_t dst, size_t src, int offset) {

		if (offset > 0) {

			this->append(new FI_move_reg_offs(dst, src, offset));

		} else {

			if (src != dst)
				this->append(new FI_move_reg(dst, src));

		}				

	}

	static const cl_accessor* computeOffset(int& offset, const cl_accessor* acc) {

		while (acc && (acc->code == CL_ACCESSOR_ITEM)) {

			offset += acc->type->items[acc->data.item.id].offset;
			acc = acc->next;

		}

		return acc;

	}

	void cLoadReg(size_t dst, size_t src, const cl_operand& op) {

		const cl_accessor* acc = op.accessor;

		int offset = 0;

		if (acc && (acc->code == CL_ACCESSOR_DEREF)) {

			assert(op.type->code == cl_type_e::CL_TYPE_PTR);

			acc = Core::computeOffset(offset, acc->next);

			if (acc && (acc->code == CL_ACCESSOR_REF)) {

				assert(acc->next == NULL);
				this->cMoveReg(dst, src, offset);
				return;

			}

			assert(acc == NULL);

			if (op.type->code == cl_type_e::CL_TYPE_STRUCT) {

				std::vector<size_t> offs;
				NodeBuilder::buildNode(offs, op.type);

				this->append(new FI_acc_set(dst, offset, offs));
				this->append(new FI_loads(dst, offset, offs));

			} else {

				this->append(new FI_acc_sel(dst, offset));
				this->append(new FI_load(dst, offset));

			}

		} else {

			acc = Core::computeOffset(offset, acc);

			assert(acc == NULL);

			this->cMoveReg(dst, src, offset);

		}

	}

	void cStoreReg(const cl_operand& op, size_t src, size_t tmp) {

		const cl_accessor* acc = op.accessor;

		int offset = 0;

		if (acc && (acc->code == CL_ACCESSOR_DEREF)) {

			assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

			acc = Core::computeOffset(offset, acc->next);

			assert(acc == NULL);

			if (op.type->code == cl_type_e::CL_TYPE_STRUCT) {

				std::vector<size_t> offs;
				NodeBuilder::buildNode(offs, op.type);

				this->append(new FI_acc_set(tmp, offset, offs));
				this->append(new FI_stores(tmp, src, offset, offs));

			} else {

				this->append(new FI_acc_sel(tmp, offset));
				this->append(new FI_store(tmp, src, offset));

			}

			this->append(new FI_check());

		} else {

			acc = Core::computeOffset(offset, acc);

			assert(acc == NULL);
			assert(offset == 0);

			if (src != tmp)
				this->append(new FI_move_reg(tmp, src));

		}

	}

	size_t cLoadOperand(size_t dst, const cl_operand& op) {

		switch (op.code) {

			case cl_operand_e::CL_OPERAND_VAR: {

				auto varInfo = this->curCtx->getVarInfo(varIdFromOperand(&op));

				if (varInfo.first) {

					// stack variable
					this->cLoadVar(dst, varInfo.second);

				} else {
					
					// register
					dst = varInfo.second;

				}

				this->cLoadReg(dst, dst, op);
				break;
				
			}

			case cl_operand_e::CL_OPERAND_CST:
				this->cLoadCst(dst, op);
				break;

			default:
				assert(false);

		}

		return dst;
		
	}

	size_t lookupStoreReg(const cl_operand& op, size_t src) {

		size_t tmp = src;

		switch (op.code) {

			case cl_operand_e::CL_OPERAND_VAR: {

				auto varInfo = this->curCtx->getVarInfo(varIdFromOperand(&op));

				if (varInfo.first) {

					// stack variable
					return src;

				} else {
					
					// register
					tmp = varInfo.second;

				}

				const cl_accessor* acc = op.accessor;
		
				return (acc && (acc->code == CL_ACCESSOR_DEREF))?(src):(tmp);
				
			}

			default:
				assert(false);

		}

	}

	void cStoreOperand(const cl_operand& op, size_t src, size_t tmp) {

		switch (op.code) {

			case cl_operand_e::CL_OPERAND_VAR: {

				auto varInfo = this->curCtx->getVarInfo(varIdFromOperand(&op));

				if (varInfo.first) {

					// stack variable
					this->append(new FI_move_ABP(tmp, varInfo.second));

					const cl_accessor* acc = op.accessor;
			
					int offset = 0;

					bool needsAcc = false;

					if (acc) {

						if (acc->code == CL_ACCESSOR_DEREF) {

							assert(acc->type->code == cl_type_e::CL_TYPE_PTR);

							this->append(new FI_load(tmp, 0));

							needsAcc = true;

						}

						acc = Core::computeOffset(offset, acc->next);

					}
			
					assert(acc == NULL);
					
					if (op.type->code == cl_type_e::CL_TYPE_STRUCT) {
			
						std::vector<size_t> offs;
						NodeBuilder::buildNode(offs, op.type);
			
						if (needsAcc)
							this->append(new FI_acc_set(tmp, offset, offs));
						this->append(new FI_stores(tmp, src, offset, offs));
			
					} else {
			
						if (needsAcc)
							this->append(new FI_acc_sel(tmp, offset));
						this->append(new FI_store(tmp, src, offset));
			
					}
			
					this->append(new FI_check());
			
				} else {
					
					// register
					this->cStoreReg(op, src, varInfo.second);

				}

				break;
				
			}

			default:
				assert(false);

		}

	}

	AbstractInstruction* cKillDeadVariables(const CodeStorage::TKillVarList& vars) {

		std::set<size_t> offs;

		for (auto var : vars) {

			if (var.onlyIfNotPointed)
				continue;

			const std::pair<bool, size_t>& varInfo = this->curCtx->getVarInfo(var.uid);
			
			if (!varInfo.first)
				continue;

			offs.insert(varInfo.second);

		}

		if (offs.empty())
			return NULL;

		AbstractInstruction* result = new FI_load_cst(0, Data::createUndef());

		this->append(result);

		for (auto offset : offs)
			this->append(new FI_store_ABP(0, offset));

		this->append(new FI_check());

		return result;

	}

	void compileAssignment(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src = insn.operands[1];

		assert(src.type->code == dst.type->code);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t srcReg = this->cLoadOperand(dstReg, src);

		if (
			src.type->code == cl_type_e::CL_TYPE_PTR &&
			src.type->items[0].type->code == cl_type_e::CL_TYPE_VOID &&
			dst.type->items[0].type->code != cl_type_e::CL_TYPE_VOID
		) {

//			this->append(new FI_is_type(srcReg, data_type_e::t_void_ptr));
			this->append(new FI_node_create(this->boxMan, srcReg, srcReg, dst.type->items[0].type));
			
		}

		this->cStoreOperand(dst, srcReg, 1);
		this->cKillDeadVariables(insn.varsToKill);

	}

	void compileTruthNot(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src = insn.operands[1];

		assert(dst.type->code == cl_type_e::CL_TYPE_BOOL);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t srcReg = this->cLoadOperand(dstReg, src);

		switch (src.type->code) {

			case cl_type_e::CL_TYPE_BOOL:
				this->append(new FI_bnot(srcReg));
				break;

			case cl_type_e::CL_TYPE_INT:
				this->append(new FI_inot(srcReg));
				break;

			default:
				assert(false);

		}
	
		this->cStoreOperand(dst, srcReg, 1);
		this->cKillDeadVariables(insn.varsToKill);
		
	}

	void compileMalloc(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src = insn.operands[2];

		assert(src.type->code == cl_type_e::CL_TYPE_INT);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t srcReg = this->cLoadOperand(dstReg, src);

		if (
			dst.type->code == cl_type_e::CL_TYPE_PTR &&
			dst.type->items[0].type->code != cl_type_e::CL_TYPE_VOID
		) {

			this->append(new FI_node_alloc(this->boxMan, srcReg, srcReg, dst.type->items[0].type));

		}

		this->cStoreOperand(dst, srcReg, 1);
		this->cKillDeadVariables(insn.varsToKill);

	}

	void compileFree(const CodeStorage::Insn& insn) {

		const cl_operand& src = insn.operands[2];

		size_t srcReg = this->cLoadOperand(0, src);

		this->append(new FI_acc_all(srcReg));
		this->append(new FI_node_free(srcReg));
		this->append(new FI_check());

	}

	template <class F>
	void compileCmp(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src1 = insn.operands[1];
		const cl_operand& src2 = insn.operands[2];

		assert(dst.type->code == cl_type_e::CL_TYPE_BOOL);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t src1Reg = this->cLoadOperand(0, src1);
		size_t src2Reg = this->cLoadOperand(1, src2);

		this->append(new FI_cmp<F>(dstReg, src1Reg, src2Reg));
		this->cStoreOperand(dst, dstReg, 1); 
		this->cKillDeadVariables(insn.varsToKill);

	}

	void compilePlus(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src1 = insn.operands[1];
		const cl_operand& src2 = insn.operands[2];

		assert(dst.type->code == cl_type_e::CL_TYPE_INT);
		assert(src1.type->code == cl_type_e::CL_TYPE_INT);
		assert(src2.type->code == cl_type_e::CL_TYPE_INT);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t src1Reg = this->cLoadOperand(0, src1);
		size_t src2Reg = this->cLoadOperand(1, src2);

		this->append(new FI_iadd(dstReg, src1Reg, src2Reg));
		this->cStoreOperand(dst, dstReg, 1); 
		this->cKillDeadVariables(insn.varsToKill);
		
	}

	void compilePointerPlus(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];
		const cl_operand& src1 = insn.operands[1];
		const cl_operand& src2 = insn.operands[2];

		assert(dst.type->code == cl_type_e::CL_TYPE_INT);
		assert(src1.type->code == cl_type_e::CL_TYPE_INT);
		assert(src2.type->code == cl_type_e::CL_TYPE_INT);

		size_t dstReg = this->lookupStoreReg(dst, 0);
		size_t src1Reg = this->cLoadOperand(0, src1);
		size_t src2Reg = this->cLoadOperand(1, src2);

		this->append(new FI_move_reg_inc(dstReg, src1Reg, src2Reg));
		this->cStoreOperand(dst, dstReg, 1); 
		this->cKillDeadVariables(insn.varsToKill);
		
	}

	void compileJmp(const CodeStorage::Insn& insn) {

		this->append(new FI_jmp(insn.targets[0], &insn));

	}

	void compileRet(const CodeStorage::Insn& insn) {

		this->append(new FI_ret(&insn));

	}

	void compileCond(const CodeStorage::Insn& insn) {

		const cl_operand& src = insn.operands[0];

		size_t srcReg = this->cLoadOperand(0, src);

		this->cKillDeadVariables(insn.varsToKill);

		AbstractInstruction* tmp[2] = { NULL, NULL };

		size_t blockHead = this->assembly->code_.size();

		this->append(NULL);

		for (auto i : { 0, 1 }) {

			tmp[i] = this->cKillDeadVariables(insn.killPerTarget[i]);

			this->append(new FI_jmp(insn.targets[i]));

			if (!tmp[i])
				tmp[i] = this->assembly->code_.back();

		}

		this->assembly->code_[blockHead] = new FI_cond(srcReg, tmp); 

	}

	void compileNondet(const CodeStorage::Insn& insn) {

		const cl_operand& dst = insn.operands[0];

		size_t dstReg = this->lookupStoreReg(dst, 0);

		this->append(new FI_load_cst(dstReg, Data::createUnknw()));
		this->cStoreOperand(dst, dstReg, 1);

	}

	void compileInstruction(const CodeStorage::Insn& insn) {

		CL_CDEBUG(insn.loc << ' ' << insn);
		
		switch (insn.code) {

			case cl_insn_e::CL_INSN_UNOP:
				switch (insn.subCode) {
					case cl_unop_e::CL_UNOP_ASSIGN:
						this->compileAssignment(insn);
						break;
					case cl_unop_e::CL_UNOP_TRUTH_NOT:
						this->compileTruthNot(insn);
						break;
					default:
						throw std::runtime_error("feature not implemented");
				}
				break;

			case cl_insn_e::CL_INSN_BINOP:
				switch (insn.subCode) {
					case cl_binop_e::CL_BINOP_EQ:
						this->compileCmp<Eq>(insn);
						break;
					case cl_binop_e::CL_BINOP_NE:
						this->compileCmp<Neq>(insn);
						break;
					case cl_binop_e::CL_BINOP_LT:
						this->compileCmp<Lt>(insn);
						break;
					case cl_binop_e::CL_BINOP_PLUS:
						this->compilePlus(insn);
						break;
/*					case cl_binop_e::CL_BINOP_MINUS:
						this->execMinus(state, parent, insn);
						break;*/
					case cl_binop_e::CL_BINOP_POINTER_PLUS:
						this->compilePointerPlus(insn);
						break;
					default:
						throw std::runtime_error("feature not implemented");
				}
				break;

			case cl_insn_e::CL_INSN_CALL:
				assert(insn.operands[1].code == cl_operand_e::CL_OPERAND_CST);
				assert(insn.operands[1].data.cst.code == cl_type_e::CL_TYPE_FNC);
				switch (this->builtinTable[insn.operands[1].data.cst.data.cst_fnc.name]) {
					case builtin_e::biMalloc:
						this->compileMalloc(insn);
						break;
					case builtin_e::biFree:
						this->compileFree(insn);
						break;
					case builtin_e::biNondet:
						this->compileNondet(insn);
						break;
					default:
						throw std::runtime_error("feature not implemented");
				}
				break;

			case cl_insn_e::CL_INSN_RET:
				this->compileRet(insn);
				break;

			case cl_insn_e::CL_INSN_JMP:
				this->compileJmp(insn);
				break;

			case cl_insn_e::CL_INSN_COND:
				this->compileCond(insn);
				break;

			default:
				throw std::runtime_error("feature not implemented");

		}
		
	}

	void compileBlock(const CodeStorage::Block* block) {

		if (this->loopAnalyser.isEntryPoint(*block->begin())) {
			this->append(
				new FI_fix(this->fixpointBackend, this->taBackend, this->boxMan, this->boxes)
			);
		}

		for (auto insn : *block) {

			size_t head = this->assembly->code_.size();

			this->compileInstruction(*insn);

			for (size_t i = head; i < this->assembly->code_.size(); ++i)
				CL_CDEBUG(this->assembly->code_[i] << ":\t" << *this->assembly->code_[i]);

		}
		
	}

	void compileFunction(const CodeStorage::Fnc& fnc) {

		// build context
		SymCtx ctx(fnc);
		this->curCtx = &ctx;

		if (this->assembly->regFileSize_ < ctx.regCount)
			this->assembly->regFileSize_ = ctx.regCount;
		
		// compute loop entry points
		this->loopAnalyser.init(fnc.cfg.entry());

		auto functionHead = this->assembly->code_.size();

		// compile underlying CFG
		std::list<const CodeStorage::Block*> queue;

		queue.push_back(fnc.cfg.entry());

		while (!queue.empty()) {

			const CodeStorage::Block* block = queue.front();
			queue.pop_front();

			auto p = this->codeIndex.insert(std::make_pair(block, (AbstractInstruction*)NULL));
			if (!p.second)
				continue;

			size_t blockHead = this->assembly->code_.size();

			this->compileBlock(block);

			assert(blockHead < this->assembly->code_.size());
			p.first->second = this->assembly->code_[blockHead];

			for (auto target : block->targets())
				queue.push_back(target);

		}

		auto iter = this->codeIndex.find(fnc.cfg.entry());
		assert(iter != this->codeIndex.end());
		this->assembly->functionIndex_.insert(std::make_pair(&fnc, iter->second));

		for (auto i = functionHead; i < this->assembly->code_.size(); ++i)
			this->assembly->code_[i]->finalize(this->codeIndex, this->assembly->code_.begin() + i);
		
	}

public:

	Core(TA<label_type>::Backend& fixpointBackend, TA<label_type>::Backend& taBackend,
		BoxMan& boxMan, const std::vector<const Box*>& boxes)
		: fixpointBackend(fixpointBackend), taBackend(taBackend), boxMan(boxMan), boxes(boxes) {}

	void compile(Compiler::Assembly& assembly, const CodeStorage::Storage& stor) {

		this->reset(assembly);

		for (auto fnc : stor.fncs) {

			if (isDefined(*fnc))
				this->compileFunction(*fnc);

		}

	}

};

Compiler::Compiler(TA<label_type>::Backend& fixpointBackend, TA<label_type>::Backend& taBackend,
BoxMan& boxMan, const std::vector<const Box*>& boxes)
	: core_(new Core(fixpointBackend, taBackend, boxMan, boxes)) {}

Compiler::~Compiler() {
	delete this->core_;
}

void Compiler::compile(Compiler::Assembly& assembly, const CodeStorage::Storage& stor) {
	this->core_->compile(assembly, stor);
}
