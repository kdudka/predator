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

#include <cl/cldebug.hh>

#include "executionmanager.hh"
#include "splitting.hh"
#include "virtualmachine.hh"
#include "programerror.hh"
#include "normalization.hh"
#include "regdef.hh"
#include "symctx.hh"
#include "jump.hh"

#include "microcode.hh"

// FI_cond
void FI_cond::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	assert((*state.first)[this->src_].isBool());

	execMan.enqueue(state, this->next_[((*state.first)[this->src_].d_bool)?(0):(1)]);

}

void FI_cond::finalize(
	const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>& codeIndex,
	std::vector<AbstractInstruction*>::const_iterator
) {

	for (auto i : { 0, 1 }) {

		if (this->next_[i]->getType() == e_fi_type::fiJump) {
			do {
				this->next_[i] = ((FI_jmp*)this->next_[i])->getTarget(codeIndex);
			} while (this->next_[i]->getType() == e_fi_type::fiJump);
		}
		this->next_[i]->setTarget();

	}
	
}

// FI_acc_sel
void FI_acc_sel::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

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

// FI_acc_set
void FI_acc_set::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

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

// FI_acc_all
void FI_acc_all::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

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

// FI_load_cst
void FI_load_cst::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	(*state.first)[this->dst_] = this->data_;

	execMan.enqueue(state, this->next_);

}

// FI_move_reg
void FI_move_reg::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	(*state.first)[this->dst_] = (*state.first)[this->src_];

	execMan.enqueue(state, this->next_);

}

// FI_bnot
void FI_bnot::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	assert((*state.first)[this->dst_].isBool());

	(*state.first)[this->dst_] = Data::createBool(!(*state.first)[this->dst_].d_bool);

	execMan.enqueue(state, this->next_);

}

// FI_inot
void FI_inot::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	assert((*state.first)[this->dst_].isInt());

	(*state.first)[this->dst_] = Data::createBool(!(*state.first)[this->dst_].d_int);

	execMan.enqueue(state, this->next_);

}

// FI_move_reg_offs
void FI_move_reg_offs::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

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

// FI_move_reg_inc
void FI_move_reg_inc::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

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

// FI_get_sreg
void FI_get_greg::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	(*state.first)[this->dst_] = VirtualMachine(*state.second->fae).varGet(this->src_);

	execMan.enqueue(state, this->next_);

}

// FI_set_sreg
void FI_set_greg::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	VirtualMachine(*state.second->fae).varSet(this->dst_, (*state.first)[this->src_]);

	execMan.enqueue(state, this->next_);

}

// FI_move_ABP
void FI_move_ABP::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	(*state.first)[this->dst_] = VirtualMachine(*state.second->fae).varGet(ABP_INDEX);
	(*state.first)[this->dst_].d_ref.displ += this->offset_;

	execMan.enqueue(state, this->next_);

}

// FI_load
void FI_load::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	assert((*state.first)[this->dst_].isRef());

	const Data& data = (*state.first)[this->dst_];

	VirtualMachine(*state.second->fae).nodeLookup(
		data.d_ref.root, data.d_ref.displ + this->offset_, (*state.first)[this->dst_]
	);

	execMan.enqueue(state, this->next_);

}

// FI_load_ABP
void FI_load_ABP::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	VirtualMachine vm(*state.second->fae);

	const Data& data = vm.varGet(ABP_INDEX);

	vm.nodeLookup(data.d_ref.root, (size_t)this->offset_, (*state.first)[this->dst_]);

	execMan.enqueue(state, this->next_);

}

// FI_store
void FI_store::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

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
/*
// FI_store_ABP
void FI_store_ABP::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

	VirtualMachine vm(*fae);

	const Data& data = vm.varGet(ABP_INDEX);

	Data out;

	vm.nodeModify(data.d_ref.root, this->offset_, (*state.first)[this->src_], out);

	execMan.enqueue(state.second, state.first, fae, this->next_);

}
*/
// FI_loads
void FI_loads::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	assert((*state.first)[this->dst_].isRef());

	const Data& data = (*state.first)[this->dst_];

	VirtualMachine(*state.second->fae).nodeLookupMultiple(
		data.d_ref.root, data.d_ref.displ + this->base_, this->offsets_, (*state.first)[this->dst_]
	);

	execMan.enqueue(state, this->next_);

}

// FI_stores
void FI_stores::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

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

// FI_alloc
void FI_alloc::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	assert((*state.first)[this->src_].isInt());

	(*state.first)[this->dst_] =
		Data::createVoidPtr((*state.first)[this->src_].d_int);
	
	execMan.enqueue(state, this->next_);

}

// FI_node_create
void FI_node_create::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	assert((*state.first)[this->src_].isVoidPtr());

	if ((int)(*state.first)[this->src_].d_void_ptr != this->size_)
		throw ProgramError("allocated block size mismatch");

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

	(*state.first)[this->dst_] = Data::createRef(
		VirtualMachine(*fae).nodeCreate(this->sels_, this->typeInfo_)
	);

	execMan.enqueue(state.second, state.first, fae, this->next_);

}
/*
// FI_node_alloc
void FI_node_alloc::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

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
*/
// FI_node_free
void FI_node_free::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	assert((*state.first)[this->dst_].isRef());

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

	const Data& data = (*state.first)[this->dst_];

	if (data.d_ref.displ != 0)
		throw ProgramError("releasing a pointer which points inside an allocated block");

	VirtualMachine(*fae).nodeDelete(data.d_ref.root);

	execMan.enqueue(state.second, state.first, fae, this->next_);

}

// FI_iadd
void FI_iadd::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	assert((*state.first)[this->src1_].isInt() && (*state.first)[this->src2_].isInt());

	(*state.first)[this->dst_] = Data::createInt(
		((*state.first)[this->src1_].d_int + (*state.first)[this->src2_].d_int > 0)?(1):(0)
	);

	execMan.enqueue(state, this->next_);

}

// FI_check
void FI_check::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	Normalization((FAE&)*state.second->fae).check();

	execMan.enqueue(state, this->next_);

}

// FI_assert
void FI_assert::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.second->fae));

	VirtualMachine vm(*state.second->fae);

	const Data& abp = vm.varGet(ABP_INDEX);

	assert(abp.isRef());
	assert(abp.d_ref.displ == 0);
	
	Data data;

	vm.nodeLookup(abp.d_ref.root, this->offset_, data);

	if (data != this->cst_)
		throw std::runtime_error("assertion failed");

	execMan.enqueue(state, this->next_);

}
