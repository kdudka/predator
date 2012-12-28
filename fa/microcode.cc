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

inline const cl_loc* getLoc(const SymState& state)
{
	// Assertions
	assert(nullptr != state.GetInstr());

	if (!state.GetInstr()->insn())
		return nullptr;

	return &(state.GetInstr()->insn()->loc);
}

} // namespace

// FI_cond
void FI_cond::execute(ExecutionManager& execMan, SymState& state)
{
	// Assertions
	assert(state.GetReg(src_).isBool());

	SymState* tmpState = execMan.createChildState(state,
		next_[(state.GetReg(src_).d_bool)?(0):(1)]);

	execMan.enqueue(tmpState);
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
void FI_acc_sel::execute(ExecutionManager& execMan, SymState& state)
{
	const Data& data = state.GetReg(dst_);

	if (!data.isRef())
	{
		std::stringstream ss;
		ss << "dereferenced value is not a valid reference [" << data << ']';
		throw ProgramError(ss.str(), &state, getLoc(state));
	}

	std::vector<FAE*> res;

	Splitting(*state.GetFAE()).isolateOne(
		/* vector for results */ res,
		/* index of the desired TA */ data.d_ref.root,
		/* offset of the selector */ data.d_ref.displ + offset_
	);

	for (auto fae : res)
	{
		SymState* tmpState = execMan.createChildState(state, next_);
		tmpState->SetFAE(std::shared_ptr<FAE>(fae));
		execMan.enqueue(tmpState);
	}
}

// FI_acc_set
void FI_acc_set::execute(ExecutionManager& execMan, SymState& state)
{
	auto data = state.GetReg(dst_);

	if (!data.isRef())
	{
		std::stringstream ss;
		ss << "dereferenced value is not a valid reference [" << data << ']';
		throw ProgramError(ss.str(), &state, getLoc(state));
	}

	std::vector<FAE*> res;

	Splitting(*state.GetFAE()).isolateSet(
		/* vector for results */ res,
		/* index of the desired TA */ data.d_ref.root,
		/* base of offsets */ data.d_ref.displ + base_,
		/* offsets of selectors */ offsets_
	);

	for (auto fae : res)
	{
		SymState* tmpState = execMan.createChildState(state, next_);
		tmpState->SetFAE(std::shared_ptr<FAE>(fae));
		execMan.enqueue(tmpState);
	}
}

// FI_acc_all
void FI_acc_all::execute(ExecutionManager& execMan, SymState& state)
{
	auto data = state.GetReg(dst_);

	if (!data.isRef())
	{
		std::stringstream ss;
		ss << "dereferenced value is not a valid reference [" << data << ']';
		throw ProgramError(ss.str(), &state, getLoc(state));
	}

	std::vector<FAE*> res;

	Splitting(*state.GetFAE()).isolateSet(
		/* vector for results */ res,
		/* index of the desired TA */ data.d_ref.root,
		/* base of offsets */ 0,
		/* offsets of selectors */ state.GetFAE()->getType(data.d_ref.root)->getSelectors()
	);

	for (auto fae : res)
	{
		SymState* tmpState = execMan.createChildState(state, next_);
		tmpState->SetFAE(std::shared_ptr<FAE>(fae));
		execMan.enqueue(tmpState);
	}
}

// FI_load_cst
void FI_load_cst::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);
	tmpState->SetReg(dstReg_, data_);

	execMan.enqueue(tmpState);
}

// FI_move_reg
void FI_move_reg::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);
	tmpState->SetReg(dstReg_, tmpState->GetReg(src_));

	execMan.enqueue(tmpState);
}

// FI_bnot
void FI_bnot::execute(ExecutionManager& execMan, SymState& state)
{
	// Assertions
	assert(state.GetReg(dstReg_).isBool());

	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);
	tmpState->SetReg(dstReg_, Data::createBool(!tmpState->GetReg(dstReg_).d_bool));

	execMan.enqueue(tmpState);
}

// FI_inot
void FI_inot::execute(ExecutionManager& execMan, SymState& state)
{
	// Assertions
	assert(state.GetReg(dstReg_).isInt());

	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);
	tmpState->SetReg(dstReg_, Data::createBool(!tmpState->GetReg(dstReg_).d_int));

	execMan.enqueue(tmpState);
}

// FI_move_reg_offs
void FI_move_reg_offs::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);
	Data data = tmpState->GetReg(src_);

	if (!data.isRef())
	{
		std::stringstream ss;
		ss << "dereferenced value is not a valid reference [" << data << ']';
		throw ProgramError(ss.str(), &state, getLoc(state));
	}

	data.d_ref.displ += offset_;

	tmpState->SetReg(dstReg_, data);

	execMan.enqueue(tmpState);
}

// FI_move_reg_inc
void FI_move_reg_inc::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);
	Data data = tmpState->GetReg(src1_);

	if (!data.isRef())
	{
		std::stringstream ss;
		ss << "dereferenced value is not a valid reference [" << data << ']';
		throw ProgramError(ss.str(), &state, getLoc(state));
	}

	// make sure that the value is really integer
	assert(tmpState->GetReg(src2_).isInt());

	data.d_ref.displ += tmpState->GetReg(src2_).d_int;
	tmpState->SetReg(dstReg_, data);

	execMan.enqueue(tmpState);
}

// FI_get_greg
void FI_get_greg::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);

	tmpState->SetReg(dstReg_, VirtualMachine(*(tmpState->GetFAE())).varGet(src_));

	execMan.enqueue(tmpState);
}

// FI_set_greg
void FI_set_greg::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildState(state, next_);

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(tmpState->GetFAE())));
	VirtualMachine(*fae).varSet(dst_, tmpState->GetReg(src_));
	tmpState->SetFAE(fae);

	execMan.enqueue(tmpState);
}

// FI_get_ABP
void FI_get_ABP::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);
	Data data = VirtualMachine(*(tmpState->GetFAE())).varGet(ABP_INDEX);
	data.d_ref.displ += offset_;

	tmpState->SetReg(dstReg_, data);

	execMan.enqueue(tmpState);
}

// FI_get_GLOB
void FI_get_GLOB::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);
	Data data = VirtualMachine(*(tmpState->GetFAE())).varGet(GLOB_INDEX);
	data.d_ref.displ += offset_;

	tmpState->SetReg(dstReg_, data);

	execMan.enqueue(tmpState);
}

// FI_load
void FI_load::execute(ExecutionManager& execMan, SymState& state)
{
	// Assertions
	assert(state.GetReg(src_).isRef());

	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);

	Data data = tmpState->GetReg(src_);

	Data out;
	VirtualMachine(*(tmpState->GetFAE())).nodeLookup(
		data.d_ref.root, data.d_ref.displ + offset_, out
	);

	tmpState->SetReg(dstReg_, out);

	execMan.enqueue(tmpState);
}

// FI_load_ABP
void FI_load_ABP::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);
	VirtualMachine vm(*(tmpState->GetFAE()));

	Data data = vm.varGet(ABP_INDEX);
	Data out;
	vm.nodeLookup(data.d_ref.root, static_cast<size_t>(offset_), out);
	tmpState->SetReg(dstReg_, out);
	execMan.enqueue(tmpState);
}

// FI_load_GLOB
void FI_load_GLOB::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);
	VirtualMachine vm(*(tmpState->GetFAE()));

	Data data = vm.varGet(GLOB_INDEX);
	// make sure that the value is really a tree reference
	assert(data.isRef());

	Data out;
	vm.nodeLookup(data.d_ref.root, static_cast<size_t>(offset_), out);
	tmpState->SetReg(dstReg_, out);

	execMan.enqueue(tmpState);
}

// FI_store
void FI_store::execute(ExecutionManager& execMan, SymState& state)
{
	// Assertions
	assert(state.GetReg(dst_).isRef());

	SymState* tmpState = execMan.createChildState(state, next_);

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(tmpState->GetFAE())));

	const Data& dst = tmpState->GetReg(dst_);
	const Data& src = tmpState->GetReg(src_);

	Data out;

	VirtualMachine(*fae).nodeModify(
		dst.d_ref.root, dst.d_ref.displ + offset_, src, out
	);

	tmpState->SetFAE(fae);

	execMan.enqueue(tmpState);
}

// FI_loads
void FI_loads::execute(ExecutionManager& execMan, SymState& state)
{
	// Assertions
	assert(state.GetReg(src_).isRef());

	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);
	const Data& data = tmpState->GetReg(src_);

	Data out;
	VirtualMachine(*(tmpState->GetFAE())).nodeLookupMultiple(
		data.d_ref.root, data.d_ref.displ + base_, offsets_,
		out
	);

	tmpState->SetReg(dstReg_, out);
	execMan.enqueue(tmpState);
}

// FI_stores
void FI_stores::execute(ExecutionManager& execMan, SymState& state)
{
	// Assertions
	assert(state.GetReg(dst_).isRef());

	SymState* tmpState = execMan.createChildState(state, next_);

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(tmpState->GetFAE())));

	const Data& dst = tmpState->GetReg(dst_);
	const Data& src = tmpState->GetReg(src_);
	// check the loaded value
	assert(src.isStruct());

	Data tmp;            // necessary for the call of the nodeModify() method
	VirtualMachine(*fae).nodeModifyMultiple(
		dst.d_ref.root, dst.d_ref.displ + base_, src, tmp
	);

	tmpState->SetFAE(fae);

	execMan.enqueue(tmpState);
}

// FI_alloc
void FI_alloc::execute(ExecutionManager& execMan, SymState& state)
{
	// Assertions
	assert(state.GetReg(src_).isInt());

	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);

	const Data& srcData = tmpState->GetReg(src_);

	if (0 > srcData.d_int)
	{	// negative allocation size
		FA_ERROR_MSG(&(tmpState->GetInstr()->insn()->loc),
			"negative size arg of malloc(): " << srcData.d_int);
	}
	if (0 == srcData.d_int)
	{	// zero allocation size
		FA_WARN_MSG(&(tmpState->GetInstr()->insn()->loc),
			"POSIX says that, given zero size, the behaviour of "
			"malloc/calloc is implementation-defined");
	}

	// create a void pointer of given size, i.e. it points to a block of the size
	Data dstData = Data::createVoidPtr(srcData.d_int);
	tmpState->SetReg(dstReg_, dstData);

	execMan.enqueue(tmpState);
}


// FI_node_create
void FI_node_create::execute(ExecutionManager& execMan, SymState& state)
{
	const Data& srcData = state.GetReg(src_);

	// TODO: is this alright? Shouldn't dst_ be modified?
	if (srcData.isRef() || srcData.isNull())
	{	// this is to handle the special case that appears e.g. in test-p0001.c
		// Jiri claims it is a horrible solution of the issue and suggests to do it
		// in a better way :-)
		SymState* tmpState = execMan.createChildState(state, next_);
		execMan.enqueue(tmpState);
		return;
	}

	// assert that src_ is a void pointer
	assert(srcData.isVoidPtr());

	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);

	if (srcData.d_void_ptr_size != size_)
	{	// in case the type size differs from the allocated size
		throw ProgramError("allocated block size mismatch", &state, getLoc(state));
	}

	// create a new forest automaton
	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(tmpState->GetFAE())));

	// create a new node
	Data dstData = Data::createRef(
		VirtualMachine(*fae).nodeCreate(sels_, typeInfo_)
	);

	tmpState->SetReg(dst_, dstData);
	tmpState->SetFAE(fae);

	execMan.enqueue(tmpState);
}

// FI_node_free
void FI_node_free::execute(ExecutionManager& execMan, SymState& state)
{
	// Assertions
	assert(state.GetReg(dst_).isRef());

	SymState* tmpState = execMan.createChildState(state, next_);

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(tmpState->GetFAE())));

	const Data& data = tmpState->GetReg(dst_);

	if (data.d_ref.displ != 0)
	{
		throw ProgramError(
			"releasing a pointer which points inside an allocated block",
			&state,
			getLoc(state));
	}

	VirtualMachine(*fae).nodeDelete(data.d_ref.root);
	tmpState->SetFAE(fae);

	execMan.enqueue(tmpState);
}

// FI_iadd
void FI_iadd::execute(ExecutionManager& execMan, SymState& state)
{
	// Assertions
	assert(state.GetReg(src1_).isInt() && state.GetReg(src2_).isInt());

	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);

	int sum = tmpState->GetReg(src1_).d_int + tmpState->GetReg(src2_).d_int;
	tmpState->SetReg(dstReg_, Data::createInt((sum > 0)? 1 : 0));

	execMan.enqueue(tmpState);
}

// FI_check
void FI_check::execute(ExecutionManager& execMan, SymState& state)
{
	state.GetFAE()->updateConnectionGraph();

	Normalization(const_cast<FAE&>(*(state.GetFAE())), &state).check();

	SymState* tmpState = execMan.createChildState(state, next_);
	execMan.enqueue(tmpState);
}

// FI_assert
void FI_assert::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildState(state, next_);

	if (tmpState->GetReg(dst_) != cst_)
	{
		FA_DEBUG_AT(1, "registers: " << utils::wrap(state.GetRegs()) << ", heap:"
			<< std::endl << *(state.GetFAE()));
		throw std::runtime_error("assertion failed");
	}

	execMan.enqueue(tmpState);
}

// FI_abort
void FI_abort::execute(ExecutionManager& execMan, SymState& state)
{
	execMan.pathFinished(&state);
}

// FI_build_struct
void FI_build_struct::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);

	std::vector<Data::item_info> items;

	for (size_t i = 0; i < offsets_.size(); ++i)
	{
		items.push_back(std::make_pair(offsets_[i], tmpState->GetReg(start_ + i)));
	}

	tmpState->SetReg(dstReg_, Data::createStruct(items));

	execMan.enqueue(tmpState);
}

// FI_push_greg
void FI_push_greg::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildState(state, next_);

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(tmpState->GetFAE())));
	VirtualMachine(*fae).varPush(tmpState->GetReg(src_));
	tmpState->SetFAE(fae);

	execMan.enqueue(tmpState);
}

// FI_pop_greg
void FI_pop_greg::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildStateWithNewRegs(state, next_);

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(tmpState->GetFAE())));
	tmpState->SetReg(dst_, VirtualMachine(*fae).varPop());
	tmpState->SetFAE(fae);

	execMan.enqueue(tmpState);
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
void FI_print_heap::execute(ExecutionManager& execMan, SymState& state)
{
	SymState* tmpState = execMan.createChildState(state, next_);

	FA_NOTE("local variables: " << DumpCtx(*ctx_, *(tmpState->GetFAE())));
	FA_NOTE("heap:" << *(tmpState->GetFAE()));

	execMan.enqueue(tmpState);
}


void FI_plot_heap::execute(ExecutionManager& execMan, SymState& state)
{
	// Assertions
	assert(nullptr != this->insn());

	SymState* tmpState = execMan.createChildState(state, next_);

	MemPlotter::handlePlot(*tmpState, *this->insn());

	execMan.enqueue(tmpState);
}


void FI_error::execute(ExecutionManager& execMan, SymState& state)
{
	(void)execMan;

	FA_DEBUG_AT(1, "registers: " << utils::wrap(state.GetRegs()) << ", heap:"
		<< std::endl << *(state.GetFAE()));
	throw ProgramError(msg_, &state, getLoc(state));
}
