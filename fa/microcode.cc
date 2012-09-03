/*
 * Copyright (C) 2011 Jiri Simacek
 *
 * This file is part of forester.
 *
 * forester is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * forester is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with forester.  If not, see <http://www.gnu.org/licenses/>.
 */

// Standard library headers
#include <list>
#include <sstream>

// Code Listener headers
#include <cl/cldebug.hh>

// Forester headers
#include "executionmanager.hh"
#include "jump.hh"
#include "memplot.hh"
#include "microcode.hh"
#include "normalization.hh"
#include "programerror.hh"
#include "regdef.hh"
#include "splitting.hh"
#include "symctx.hh"
#include "virtualmachine.hh"

// anonymous namespace
namespace
{

inline const cl_loc* getLoc(const ExecState& state)
{
	// Assertions
	assert(state.GetMem() != nullptr);
	assert(state.GetMem()->GetInstr() != nullptr);

	if (!state.GetMem()->GetInstr()->insn())
		return nullptr;

	return &(state.GetMem()->GetInstr()->insn()->loc);
}

} // namespace

// FI_cond
void FI_cond::execute(ExecutionManager& execMan, const ExecState& state)
{
	// Assertions
	assert(state.GetReg(src_).isBool());

	execMan.enqueue(state, next_[(state.GetReg(src_).d_bool)?(0):(1)]);
}

void FI_cond::finalize(
	const std::unordered_map<const CodeStorage::Block*,
		AbstractInstruction*>& codeIndex,
	std::vector<AbstractInstruction*>::const_iterator
)
{
	for (auto i : { 0, 1 })
	{
		if (next_[i]->getType() == fi_type_e::fiJump)
		{
			do
			{
				next_[i] = (static_cast<FI_jmp*>(next_[i]))->getTarget(codeIndex);
			} while (next_[i]->getType() == fi_type_e::fiJump);
		}

		next_[i]->setTarget();
	}
}

// FI_acc_sel
void FI_acc_sel::execute(ExecutionManager& execMan, const ExecState& state)
{
	auto data = state.GetReg(dst_);

	if (!data.isRef())
	{
		std::stringstream ss;
		ss << "dereferenced value is not a valid reference [" << data << ']';
		throw ProgramError(ss.str(), getLoc(state));
	}

	std::vector<FAE*> dst;

	Splitting(*state.GetMem()->GetFAE()).isolateOne(dst, data.d_ref.root,
		data.d_ref.displ + offset_);

	for (auto fae : dst) {
		execMan.enqueue(state.GetMem(), execMan.allocRegisters(state.GetRegs()),
			std::shared_ptr<const FAE>(fae), next_);
	}
}

// FI_acc_set
void FI_acc_set::execute(ExecutionManager& execMan, const ExecState& state)
{
	auto data = state.GetReg(dst_);

	if (!data.isRef())
	{
		std::stringstream ss;
		ss << "dereferenced value is not a valid reference [" << data << ']';
		throw ProgramError(ss.str(), getLoc(state));
	}

	std::vector<FAE*> dst;

	Splitting(*state.GetMem()->GetFAE()).isolateSet(
		dst, data.d_ref.root, data.d_ref.displ + base_, offsets_
	);

	for (auto fae : dst)
	{
		execMan.enqueue(state.GetMem(), execMan.allocRegisters(state.GetRegs()),
			std::shared_ptr<const FAE>(fae), next_);
	}
}

// FI_acc_all
void FI_acc_all::execute(ExecutionManager& execMan, const ExecState& state)
{
	auto data = state.GetReg(dst_);

	if (!data.isRef())
	{
		std::stringstream ss;
		ss << "dereferenced value is not a valid reference [" << data << ']';
		throw ProgramError(ss.str(), getLoc(state));
	}

	std::vector<FAE*> dst;

	Splitting(*state.GetMem()->GetFAE()).isolateSet(
		dst, data.d_ref.root, 0,
		state.GetMem()->GetFAE()->getType(data.d_ref.root)->getSelectors()
	);

	for (auto fae : dst)
	{
		execMan.enqueue(state.GetMem(), execMan.allocRegisters(state.GetRegs()),
			std::shared_ptr<const FAE>(fae), next_);
	}
}

// FI_load_cst
void FI_load_cst::execute(ExecutionManager& execMan, const ExecState& state)
{
	ExecState tmpState = state;
	tmpState.SetReg(dst_, data_);

	execMan.enqueue(tmpState, next_);
}

// FI_move_reg
void FI_move_reg::execute(ExecutionManager& execMan, const ExecState& state)
{
	ExecState tmpState = state;
	tmpState.SetReg(dst_, tmpState.GetReg(src_));

	execMan.enqueue(tmpState, next_);
}

// FI_bnot
void FI_bnot::execute(ExecutionManager& execMan, const ExecState& state)
{
	// Assertions
	assert(state.GetReg(dst_).isBool());

	ExecState tmpState = state;

	tmpState.SetReg(dst_, Data::createBool(!tmpState.GetReg(dst_).d_bool));

	execMan.enqueue(tmpState, next_);
}

// FI_inot
void FI_inot::execute(ExecutionManager& execMan, const ExecState& state)
{
	// Assertions
	assert(state.GetReg(dst_).isInt());

	ExecState tmpState = state;

	tmpState.SetReg(dst_, Data::createBool(!tmpState.GetReg(dst_).d_int));

	execMan.enqueue(tmpState, next_);
}

// FI_move_reg_offs
void FI_move_reg_offs::execute(ExecutionManager& execMan, const ExecState& state)
{
	ExecState tmpState = state;
	Data data = tmpState.GetReg(src_);

	if (!data.isRef())
	{
		std::stringstream ss;
		ss << "dereferenced value is not a valid reference [" << data << ']';
		throw ProgramError(ss.str(), getLoc(tmpState));
	}

	data.d_ref.displ += offset_;

	tmpState.SetReg(dst_, data);

	execMan.enqueue(tmpState, next_);
}

// FI_move_reg_inc
void FI_move_reg_inc::execute(ExecutionManager& execMan, const ExecState& state)
{
	ExecState tmpState = state;
	Data data = tmpState.GetReg(src1_);

	if (!data.isRef())
	{
		std::stringstream ss;
		ss << "dereferenced value is not a valid reference [" << data << ']';
		throw ProgramError(ss.str(), getLoc(tmpState));
	}

	data.d_ref.displ += tmpState.GetReg(src2_).d_int;
	tmpState.SetReg(dst_, data);

	execMan.enqueue(tmpState, next_);
}

// FI_get_greg
void FI_get_greg::execute(ExecutionManager& execMan, const ExecState& state)
{
	ExecState tmpState = state;
	tmpState.SetReg(dst_, VirtualMachine(*tmpState.GetMem()->GetFAE()).varGet(src_));

	execMan.enqueue(tmpState, next_);
}

// FI_set_greg
void FI_set_greg::execute(ExecutionManager& execMan, const ExecState& state)
{
	VirtualMachine(*state.GetMem()->GetFAE()).varSet(dst_, state.GetReg(src_));

	execMan.enqueue(state, next_);
}

// FI_get_ABP
void FI_get_ABP::execute(ExecutionManager& execMan, const ExecState& state)
{
	ExecState tmpState = state;
	Data data = VirtualMachine(*tmpState.GetMem()->GetFAE()).varGet(ABP_INDEX);
	data.d_ref.displ += offset_;

	tmpState.SetReg(dst_, data);

	execMan.enqueue(tmpState, next_);
}

// FI_get_GLOB
void FI_get_GLOB::execute(ExecutionManager& execMan, const ExecState& state)
{
	ExecState tmpState = state;
	Data data = VirtualMachine(*tmpState.GetMem()->GetFAE()).varGet(GLOB_INDEX);
	data.d_ref.displ += offset_;

	tmpState.SetReg(dst_, data);

	execMan.enqueue(tmpState, next_);
}

// FI_load
void FI_load::execute(ExecutionManager& execMan, const ExecState& state)
{
	// Assertions
	assert(state.GetReg(src_).isRef());

	ExecState tmpState = state;

	Data data = tmpState.GetReg(src_);
	Data out;

	VirtualMachine(*tmpState.GetMem()->GetFAE()).nodeLookup(
		data.d_ref.root, data.d_ref.displ + offset_, out
	);

	tmpState.SetReg(dst_, out);

	execMan.enqueue(tmpState, next_);
}

// FI_load_ABP
void FI_load_ABP::execute(ExecutionManager& execMan, const ExecState& state)
{
	ExecState tmpState = state;
	VirtualMachine vm(*tmpState.GetMem()->GetFAE());

	Data data = vm.varGet(ABP_INDEX);
	Data out;
	vm.nodeLookup(data.d_ref.root, static_cast<size_t>(offset_), out);
	tmpState.SetReg(dst_, out);
	execMan.enqueue(tmpState, next_);
}

// FI_load_GLOB
void FI_load_GLOB::execute(ExecutionManager& execMan, const ExecState& state)
{
	ExecState tmpState = state;
	VirtualMachine vm(*tmpState.GetMem()->GetFAE());

	Data data = vm.varGet(GLOB_INDEX);
	Data out;
	vm.nodeLookup(data.d_ref.root, static_cast<size_t>(offset_), out);
	tmpState.SetReg(dst_, out);

	execMan.enqueue(tmpState, next_);
}

// FI_store
void FI_store::execute(ExecutionManager& execMan, const ExecState& state)
{
	// Assertions
	assert(state.GetReg(dst_).isRef());

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.GetMem()->GetFAE()));

	const Data& dst = state.GetReg(dst_);
	const Data& src = state.GetReg(src_);

	Data out;

	VirtualMachine(*fae).nodeModify(
		dst.d_ref.root, dst.d_ref.displ + offset_, src, out
	);

	execMan.enqueue(state.GetMem(), state.GetRegsShPtr(), fae, next_);
}

// FI_loads
void FI_loads::execute(ExecutionManager& execMan, const ExecState& state)
{
	// Assertions
	assert(state.GetReg(src_).isRef());

	ExecState tmpState = state;
	Data data = tmpState.GetReg(src_);
	Data out;

	VirtualMachine(*state.GetMem()->GetFAE()).nodeLookupMultiple(
		data.d_ref.root, data.d_ref.displ + base_, offsets_,
		out
	);

	tmpState.SetReg(dst_, out);
	execMan.enqueue(tmpState, next_);
}

// FI_stores
void FI_stores::execute(ExecutionManager& execMan, const ExecState& state)
{
	// Assertions
	assert(state.GetReg(dst_).isRef());

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.GetMem()->GetFAE()));

	const Data& dst = state.GetReg(dst_);
	const Data& src = state.GetReg(src_);

	Data out;

	VirtualMachine(*fae).nodeModifyMultiple(
		dst.d_ref.root, dst.d_ref.displ + base_, src, out
	);

	execMan.enqueue(state.GetMem(), state.GetRegsShPtr(), fae, next_);
}

// FI_alloc
void FI_alloc::execute(ExecutionManager& execMan, const ExecState& state)
{
	ExecState tmpState = state;

	const Data& srcData = tmpState.GetReg(src_);

	// assert that the source operand is an integer, i.e. the size
	assert(srcData.isInt());

	// create a void pointer of given size, i.e. it points to a block of the size
	Data dstData = Data::createVoidPtr(srcData.d_int);
	tmpState.SetReg(dst_, dstData);

	execMan.enqueue(tmpState, next_);
}


// FI_node_create
void FI_node_create::execute(ExecutionManager& execMan, const ExecState& state)
{
	ExecState tmpState = state;

	const Data& srcData = tmpState.GetReg(src_);

	if (srcData.isRef() || srcData.isNull())
	{	// in case src_ is a null pointer
		execMan.enqueue(tmpState.GetMem(), tmpState.GetRegsShPtr(),
			tmpState.GetMem()->GetFAE(), next_);
		return;
	}

	// assert that src_ is a void pointer
	assert(srcData.isVoidPtr());

	if (srcData.d_void_ptr_size != size_)
	{	// in case the type size differs from the allocated size
		throw ProgramError("allocated block size mismatch", getLoc(tmpState));
	}

	// create a new forest automaton
	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*tmpState.GetMem()->GetFAE()));

	// create a new node
	Data dstData = Data::createRef(
		VirtualMachine(*fae).nodeCreate(sels_, typeInfo_)
	);

	tmpState.SetReg(dst_, dstData);

	execMan.enqueue(tmpState.GetMem(), tmpState.GetRegsShPtr(), fae, next_);
}

// FI_node_free
void FI_node_free::execute(ExecutionManager& execMan, const ExecState& state)
{
	// Assertions
	assert(state.GetReg(dst_).isRef());

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.GetMem()->GetFAE()));

	const Data& data = state.GetReg(dst_);

	if (data.d_ref.displ != 0)
		throw ProgramError(
			"releasing a pointer which points inside an allocated block",
			getLoc(state));

	VirtualMachine(*fae).nodeDelete(data.d_ref.root);

	execMan.enqueue(state.GetMem(), state.GetRegsShPtr(), fae, next_);
}

// FI_iadd
void FI_iadd::execute(ExecutionManager& execMan, const ExecState& state)
{
	// Assertions
	assert(state.GetReg(src1_).isInt() && state.GetReg(src2_).isInt());

	ExecState tmpState = state;
	tmpState.SetReg(dst_,
		Data::createInt(
			(tmpState.GetReg(src1_).d_int + tmpState.GetReg(src2_).d_int > 0)?(1):(0)
		)
	);

	execMan.enqueue(tmpState, next_);
}

// FI_check
void FI_check::execute(ExecutionManager& execMan, const ExecState& state)
{
	state.GetMem()->GetFAE()->updateConnectionGraph();

	Normalization(const_cast<FAE&>(*state.GetMem()->GetFAE()),
		state.GetMem()).check();

	execMan.enqueue(state, next_);
}

// FI_assert
void FI_assert::execute(ExecutionManager& execMan, const ExecState& state)
{
	if (state.GetReg(dst_) != cst_)
	{
		FA_DEBUG_AT(1, "registers: " << utils::wrap(state.GetRegs()) << ", heap:"
			<< std::endl << *state.GetMem()->GetFAE());
		throw std::runtime_error("assertion failed");
	}

	execMan.enqueue(state, next_);
}

// FI_abort
void FI_abort::execute(ExecutionManager& execMan, const ExecState& state)
{
	execMan.pathFinished(state.GetMem());
}

// FI_build_struct
void FI_build_struct::execute(ExecutionManager& execMan, const ExecState& state)
{
	ExecState tmpState = state;

	std::vector<Data::item_info> items;

	for (size_t i = 0; i < offsets_.size(); ++i)
	{
		items.push_back(std::make_pair(offsets_[i], tmpState.GetReg(start_ + i)));
	}

	tmpState.SetReg(dst_, Data::createStruct(items));

	execMan.enqueue(tmpState, next_);
}

// FI_push_greg
void FI_push_greg::execute(ExecutionManager& execMan, const ExecState& state)
{
	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*state.GetMem()->GetFAE()));

	VirtualMachine(*fae).varPush(state.GetReg(src_));

	execMan.enqueue(state.GetMem(), state.GetRegsShPtr(), fae, next_);
}

// FI_pop_greg
void FI_pop_greg::execute(ExecutionManager& execMan, const ExecState& state)
{
	ExecState tmpState = state;

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*tmpState.GetMem()->GetFAE()));

	tmpState.SetReg(dst_, VirtualMachine(*fae).varPop());

	execMan.enqueue(tmpState.GetMem(), tmpState.GetRegsShPtr(), fae, next_);
}

struct DumpCtx {

	const SymCtx& ctx;
	const FAE& fae;

	DumpCtx(const SymCtx& ctx, const FAE& fae) : ctx(ctx), fae(fae) {}

	friend std::ostream& operator<<(std::ostream& os, const DumpCtx& cd) {

		VirtualMachine vm(cd.fae);

		std::vector<size_t> offs;

		for (auto& selData : cd.ctx.GetStackFrameLayout())
		{
			offs.push_back(selData.offset);
		}

		Data data;

		vm.nodeLookupMultiple(vm.varGet(ABP_INDEX).d_ref.root, 0, offs, data);

		std::unordered_map<size_t, Data> tmp;
		for (std::vector<Data::item_info>::const_iterator i = data.d_struct->begin();
			i != data.d_struct->end(); ++i)
			tmp.insert(std::make_pair(i->first, i->second));

		for (CodeStorage::TVarSet::const_iterator i = cd.ctx.GetFnc().vars.begin();
			i != cd.ctx.GetFnc().vars.end(); ++i) {

			const CodeStorage::Var& var = cd.ctx.GetFnc().stor->vars[*i];

			SymCtx::var_map_type::const_iterator j = cd.ctx.GetVarMap().find(var.uid);
			assert(j != cd.ctx.GetVarMap().end());

			switch (var.code) {
				case CodeStorage::EVar::VAR_LC:
					if (SymCtx::isStacked(var)) {
						std::unordered_map<size_t, Data>::iterator k = tmp.find(j->second.getStackOffset());
						assert(k != tmp.end());
						os << '#' << var.uid << ':' << var.name << " = " << k->second << std::endl;
					} else {
//							os << '#' << var.uid << " = " << fae.varGet(j->second.second) << std::endl;
					}
					break;
				default:
					break;
			}

		}

		return os;

	}

};

// FI_print_heap
void FI_print_heap::execute(ExecutionManager& execMan, const ExecState& state)
{
	FA_NOTE("local variables: " << DumpCtx(*ctx_, *state.GetMem()->GetFAE()));
	FA_NOTE("heap:" << *state.GetMem()->GetFAE());

	execMan.enqueue(state, next_);
}


void FI_plot_heap::execute(ExecutionManager& execMan, const ExecState& state)
{
	// Assertions
	assert(nullptr != this->insn());
	assert(nullptr != state.GetMem());

	MemPlotter::handlePlot(*state.GetMem(), *this->insn());

	execMan.enqueue(state, next_);
}
