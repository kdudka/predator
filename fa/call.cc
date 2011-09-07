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
#include "virtualmachine.hh"
#include "normalization.hh"
#include "programerror.hh"
#include "regdef.hh"
#include "symctx.hh"

#include "call.hh"
/*
// FI_call
void FI_call::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

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
	else
		execMan.traceFinished(state.second);

}

void FI_call::finalize(
	const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>&,
	std::vector<AbstractInstruction*>::const_iterator
) {}
*/
// FI_ret
void FI_ret::execute(ExecutionManager& execMan, const AbstractInstruction::StateType& state) {

	assert((*state.first)[this->dst_].isNativePtr());
	assert((AbstractInstruction*)(*state.first)[this->dst_].d_native_ptr);

	execMan.enqueue(state, (AbstractInstruction*)(*state.first)[this->dst_].d_native_ptr);

}

void FI_ret::finalize(
	const std::unordered_map<const CodeStorage::Block*, AbstractInstruction*>&,
	std::vector<AbstractInstruction*>::const_iterator
) {}
