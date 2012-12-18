/*
 * Copyright (C) 2012  Ondrej Lengal
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

/**
 * @file microcode_rev.cc
 * Reverse operations for microcode instructions.
 */

// Forester headers
#include "executionmanager.hh"
#include "microcode.hh"
#include "splitting.hh"
#include "streams.hh"
#include "virtualmachine.hh"

SymState* FI_acc_sel::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)fwdPred;

//	const Data& data = bwdSucc.GetReg(dst_);
//	assert(data.isRef());
//
//	SymState* tmpState = execMan.copyState(bwdSucc);
//
//	FAE* fae = Splitting(*tmpState->GetFAE()).mergeOne(
//		/* index of the desired TA */ data.d_ref.root,
//		/* offset of the selector */ data.d_ref.displ + offset_
//	);
//
//	tmpState->SetFAE(std::shared_ptr<FAE>(fae));
//
//	FA_WARN("Executing !!VERY!! suspicious reverse operation FI_acc_sel");
//	return tmpState;

	FA_WARN("Skipping reverse operation FI_acc_set");
	return execMan.copyState(bwdSucc);
}

SymState* FI_acc_set::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)fwdPred;

	FA_WARN("Skipping reverse operation FI_acc_set");
	return execMan.copyState(bwdSucc);
}

SymState* FI_acc_all::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)fwdPred;

	FA_WARN("Skipping reverse operation FI_acc_all");
	return execMan.copyState(bwdSucc);
}

SymState* FI_pop_greg::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	// Assertions
	assert(fwdPred.GetFAE()->GetVarCount() == bwdSucc.GetFAE()->GetVarCount() + 1);

	SymState* tmpState = execMan.copyStateWithNewRegs(bwdSucc, fwdPred.GetInstr());

	// invert the pop
	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(tmpState->GetFAE())));
	VirtualMachine(*fae).varPush(tmpState->GetReg(dst_));
	tmpState->SetFAE(fae);
	// restore the original value in the dst_ register
	tmpState->SetReg(dst_, fwdPred.GetReg(dst_));

	return tmpState;
}

SymState* FI_push_greg::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	// Assertions
	assert(fwdPred.GetFAE()->GetVarCount() + 1 == bwdSucc.GetFAE()->GetVarCount());
	(void)fwdPred;

	SymState* tmpState = execMan.copyState(bwdSucc);

	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(tmpState->GetFAE())));
	VirtualMachine(*fae).varPop();
	tmpState->SetFAE(fae);

	return tmpState;
}

SymState* FI_abort::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)execMan;
	(void)fwdPred;
	(void)bwdSucc;

	throw std::runtime_error("Reached unreachable operation: FI_abort reversal!");
}

SymState* FI_node_free::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	// If there is something pointing from the deallocated node, we can use the
	// original values because during the backward run, we attempt to adjust the
	// FA into the shape from the forward run.
	// @todo: is this true? cannot the FA have e.g. more cutpoints?
	SymState* tmpState = execMan.copyState(bwdSucc);
	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(tmpState->GetFAE())));
	assert(nullptr != fae);

	// 'vm' is used to manipulate with 'fae'
	VirtualMachine vm(*fae);

	// load the number of the root we will be reconstructing
	const Data& data = tmpState->GetReg(dst_);
	assert(data.isRef());
	assert(0 == data.d_ref.displ);
	size_t root = data.d_ref.root;
	if (nullptr != fae->getRoot(root))
	{	// in case some unexpected TA is in the position

		// move the TA away
		fae->freePosition(data.d_ref.root);
	}

	// load the number of the root that was deleted in the forward configuration
	const Data& oldData = fwdPred.GetReg(dst_);
	assert(oldData.isRef());
	assert(0 == oldData.d_ref.displ);
	size_t oldRoot = oldData.d_ref.root;
	assert(nullptr != fwdPred.GetFAE()->getRoot(oldRoot));

	// check that the registers correspond
	assert(fwdPred.GetRegCount() == tmpState->GetRegCount());
	for (size_t i = 0; i < tmpState->GetRegCount(); ++i)
	{
		const Data& fwdRegVal = fwdPred.GetReg(i);
		if (fwdRegVal.isRef() && (fwdRegVal.d_ref.root == oldRoot))
		{	// in case there was a reference to the deleted TA in the forward run
			assert(tmpState->GetReg(i).isUndef() ||
				(tmpState->GetReg(i).isRef() &&
					(nullptr == fae->getRoot(tmpState->GetReg(i).d_ref.root))));
		}
	}

	// we need to copy a tree automaton from the forward configuration into
	// the backward configuration
	VirtualMachine fwdVM(*(fwdPred.GetFAE()));

	vm.nodeCopy(root, fwdVM, root);

	for (size_t i = 0; i < tmpState->GetRegCount(); ++i)
	{	// check local registers
		const Data& fwdRegVal = fwdPred.GetReg(i);
		if (fwdRegVal.isRef() && (fwdRegVal.d_ref.root == oldRoot))
		{	// in case there was a reference to the deleted TA in the forward run
			assert(tmpState->GetReg(i).isRef());
			assert(tmpState->GetReg(i).d_ref.root == root);

			// TODO: otherwise, we should load 'root' to reg 'i'
		}
	}

	tmpState->SetFAE(fae);

	// now, in the reversal, we need to fill in correct references to the inserted
	// tree (from the forward predecessor) in places where there are <undef>s
	// where they originally pointed to the inserted tree. More precisely, we need
	// to traverse in parallel FAE from tmpState and fwdPred (such as when doing
	// a product) and where there are references to 'oldRoot' in fwdPred and there
	// are '<undef>'s in tmpState (there should always be an undef, otherwise
	// there must be something fishy going on...), we need to change such
	// '<undef>'s to references to 'root'
	tmpState->SubstituteRefs(fwdPred, oldData, data);

	// note that we do not need to update the references in the copied node...
	// this is because when returning from the previous abstraction, the
	// components should not change

	FA_WARN("Executing !!VERY!! suspicious reverse operation FI_node_free");

	return tmpState;
}

SymState* FI_node_create::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	const Data& oldVal = fwdPred.GetReg(src_);

	if (oldVal.isRef() || oldVal.isNull())
	{	// in case the old value was a null pointer
		return execMan.copyState(bwdSucc);
	}

	// assert that src_ is a void pointer
	assert(oldVal.isVoidPtr());

	// assert that the sizes are OK
	assert(oldVal.d_void_ptr_size == size_);

	// retrieve the reference to the created node
	const Data& nodeRef = bwdSucc.GetReg(dst_);
	// it MUST be a TA reference (otherwise sth must have gone wrong somewhere..)
	assert(nodeRef.isRef());

	// copy the old value of the register
	SymState* tmpState = execMan.copyStateWithNewRegs(bwdSucc, fwdPred.GetInstr());
	tmpState->SetReg(dst_, fwdPred.GetReg(dst_));

	// create a new forest automaton
	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(tmpState->GetFAE())));
	VirtualMachine(*fae).nodeDelete(nodeRef.d_ref.root);
	tmpState->SetFAE(fae);

	return tmpState;
}

SymState* FI_store::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	// Assertions
	assert(fwdPred.GetReg(dst_) == bwdSucc.GetReg(dst_));
	assert(fwdPred.GetReg(src_) == bwdSucc.GetReg(src_));

	const Data& treeRef = bwdSucc.GetReg(dst_);
	// assert that the loaded value is valid
	assert(treeRef.isRef());

	// load the old value of the memory node
	Data oldVal;
	VirtualMachine(*(fwdPred.GetFAE())).nodeLookup(
		treeRef.d_ref.root, treeRef.d_ref.displ + offset_, oldVal
	);

	// modify the FA 
	SymState* tmpState = execMan.copyState(bwdSucc);
	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(tmpState->GetFAE())));

	Data tmp;            // necessary for the call of the nodeModify() method
	VirtualMachine(*fae).nodeModify(
		treeRef.d_ref.root, treeRef.d_ref.displ + offset_, oldVal, tmp
	);

	tmpState->SetFAE(fae);

	return tmpState;
}

SymState* FI_stores::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	// Assertions
	assert(fwdPred.GetReg(dst_) == bwdSucc.GetReg(dst_));
	assert(fwdPred.GetReg(src_) == bwdSucc.GetReg(src_));

	const Data& storedStruct = fwdPred.GetReg(src_);
	// assert that the loaded value is valid
	assert(storedStruct.isStruct());

	std::vector<size_t> offsets;
	for (auto offsetDataPair : storedStruct.GetStruct())
	{
		offsets.push_back(offsetDataPair.first);
	}

	const Data& treeRef = bwdSucc.GetReg(dst_);
	// assert that the loaded value is valid
	assert(treeRef.isRef());

	// load the old value of the memory node
	Data oldVal;
	VirtualMachine(*(fwdPred.GetFAE())).nodeLookupMultiple(
		treeRef.d_ref.root, treeRef.d_ref.displ + base_, offsets, oldVal
	);

	// modify the FA 
	SymState* tmpState = execMan.copyState(bwdSucc);
	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(tmpState->GetFAE())));

	Data tmp;            // necessary for the call of the nodeModify() method
	VirtualMachine(*fae).nodeModifyMultiple(
		treeRef.d_ref.root, treeRef.d_ref.displ + base_, oldVal, tmp
	);

	tmpState->SetFAE(fae);

	return tmpState;
}

SymState* FI_set_greg::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	// Assertions
	assert(fwdPred.GetReg(src_) == bwdSucc.GetReg(src_));

	// load the old value of the desired global register
	Data oldVal = VirtualMachine(*(fwdPred.GetFAE())).varGet(dst_);

	// modify the FA 
	SymState* tmpState = execMan.copyState(bwdSucc);
	std::shared_ptr<FAE> fae = std::shared_ptr<FAE>(new FAE(*(tmpState->GetFAE())));
	VirtualMachine(*fae).varSet(dst_, oldVal);
	tmpState->SetFAE(fae);

	return tmpState;
}
