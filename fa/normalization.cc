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

// Code Listener headers
#include <cl/storage.hh>

// Forester headers
#include "abstractinstruction.hh"
#include "normalization.hh"
#include "programerror.hh"
#include "regdef.hh"
#include "symstate.hh"
#include "virtualmachine.hh"


TreeAut* Normalization::mergeRoot(
	TreeAut&                          dst,
	size_t                            ref,
	TreeAut&                          src,
	std::vector<size_t>&              joinStates)
{
	// Assertions
	assert(ref < this->fae.getRootCount());

	TreeAut* ta = this->fae.allocTA();
	ta->addFinalStates(dst.getFinalStates());

	size_t refState = _MSB_ADD(this->fae.boxMan->getDataId(Data::createRef(ref)));
	std::unordered_map<size_t, size_t> joinStatesMap;

	for (size_t finState : src.getFinalStates())
	{
		joinStates.push_back(this->fae.nextState());
		joinStatesMap.insert(std::make_pair(finState, this->fae.freshState()));
	}

	bool hit = false;
	for (TreeAut::iterator i = dst.begin(); i != dst.end(); ++i)
	{
		std::vector<size_t> tmp = i->lhs();
		std::vector<size_t>::iterator j = std::find(tmp.begin(), tmp.end(), refState);
		if (j != tmp.end())
		{
			for (std::vector<size_t>::iterator k = joinStates.begin();
				k != joinStates.end(); ++k)
			{
				*j = *k;
				ta->addTransition(tmp, i->label(), i->rhs());
			}
			hit = true;
		} else ta->addTransition(*i);
	}
	if (!hit) {assert(false);}
	// avoid screwing up things
	src.unfoldAtRoot(*ta, joinStatesMap, false);
	return ta;
}


void Normalization::traverse(
	std::vector<bool>&                visited,
	std::vector<size_t>&              order,
	std::vector<bool>&                marked) const
{
	visited = std::vector<bool>(this->fae.getRootCount(), false);
	marked = std::vector<bool>(this->fae.getRootCount(), false);

	order.clear();

	for (const Data& var : this->fae.GetVariables())
	{	// start from program variables and perform DFT
		// skip everything what is not a root reference
		if (!var.isRef())
			continue;

		size_t root = var.d_ref.root;

		// mark rootpoint pointed by a variable
		marked[root] = true;

		// check whether we traversed this one before
		if (visited[root])
			continue;

		this->fae.connectionGraph.visit(root, visited, order, marked);
	}
}


void Normalization::traverse(
	std::vector<bool>&                visited) const
{
	visited = std::vector<bool>(this->fae.getRootCount(), false);

	for (const Data& var : this->fae.GetVariables())
	{
		// skip everything what is not a root reference
		if (!var.isRef())
			continue;

		size_t root = var.d_ref.root;

		// check whether we traversed this one before
		if (visited[root])
			continue;

		this->fae.connectionGraph.visit(root, visited);
	}
}


void Normalization::checkGarbage(
	const std::vector<bool>&          visited) const
{
	bool garbage = false;

	for (size_t i = 0; i < this->fae.getRootCount(); ++i)
	{
		if (!this->fae.getRoot(i))
			continue;

		if (!visited[i])
		{
			FA_DEBUG_AT(1, "the root " << i << " is not referenced anymore ... "
				<< this->fae.connectionGraph.data[i]);

			garbage = true;
		}
	}

	if (garbage)
	{
		const cl_loc* loc = nullptr;
		if (nullptr != state_ &&
			nullptr != state_->GetInstr() &&
			nullptr != state_->GetInstr()->insn())
		{
			loc = &state_->GetInstr()->insn()->loc;
		}

		throw ProgramError("garbage detected", state_, loc);
	}
}


void Normalization::check() const
{
	// compute reachable roots
	std::vector<bool> visited(this->fae.getRootCount(), false);
	this->traverse(visited);

	// check garbage
	this->checkGarbage(visited);
}


void Normalization::normalizeRoot(
	std::vector<bool>&                normalized,
	size_t                            root,
	const std::vector<bool>&          marked)
{
	if (normalized[root])
		return;

	normalized[root] = true;

	// we need a copy here!
	ConnectionGraph::CutpointSignature signature =
		this->fae.connectionGraph.data[root].signature;

	for (auto& cutpoint : signature)
	{
		this->normalizeRoot(normalized, cutpoint.root, marked);

		if (marked[cutpoint.root])
			continue;

		assert(root != cutpoint.root);

		std::vector<size_t> refStates;

		TreeAut* ta = this->mergeRoot(
			*this->fae.getRoot(root),
			cutpoint.root,
			*this->fae.getRoot(cutpoint.root),
			refStates
		);

		this->fae.setRoot(root, std::shared_ptr<TreeAut>(ta));
		this->fae.setRoot(cutpoint.root, nullptr);

		this->fae.connectionGraph.mergeCutpoint(root, cutpoint.root);
	}
}


bool Normalization::selfReachable(
	size_t                            root,
	size_t                            self,
	const std::vector<bool>&          marked)
{
	for (auto& cutpoint : this->fae.connectionGraph.data[root].signature)
	{
		if (cutpoint.root == self)
			return true;

		if (marked[cutpoint.root])
			continue;

		if (this->selfReachable(cutpoint.root, self, marked))
			return true;
	}

	return false;
}


void Normalization::scan(
	std::vector<bool>&                marked,
	std::vector<size_t>&              order,
	const std::set<size_t>&           forbidden,
	bool                              extended)
{
	assert(this->fae.connectionGraph.isValid());

	std::vector<bool> visited(this->fae.getRootCount(), false);

	marked = std::vector<bool>(this->fae.getRootCount(), false);

	order.clear();

	// compute canonical root ordering
	this->traverse(visited, order, marked);

	// check garbage
	this->checkGarbage(visited);

	if (!extended)
	{
		for (auto& x : forbidden)
			marked[x] = true;

		return;
	}

	for (auto& x : forbidden)
	{
		marked[x] = true;

		for (auto& cutpoint : this->fae.connectionGraph.data[x].signature)
		{
			if ((cutpoint.root != x) && !this->selfReachable(cutpoint.root, x, marked))
				continue;

			marked[cutpoint.root] = true;

			break;
		}
	}
}


bool Normalization::normalize(
	const std::vector<bool>&          marked,
	const std::vector<size_t>&        order)
{
	bool merged = false;

	size_t i;

	for (i = 0; i < order.size(); ++i)
	{
		if (!marked[i] || (order[i] != i))
			break;
	}

	if (i == order.size())
	{	// in case the FA is in the canonical form
		this->fae.resizeRoots(order.size());
		this->fae.connectionGraph.data.resize(order.size());
		return false;
	}

	// in case the FA is not in the canonical form

	// reindex roots
	std::vector<size_t> index(this->fae.getRootCount(), static_cast<size_t>(-1));
	std::vector<bool> normalized(this->fae.getRootCount(), false);
	std::vector<std::shared_ptr<TreeAut>> newRoots;
	size_t offset = 0;

	for (auto& i : order)
	{	// push tree automata into a new tuple in the right order
		this->normalizeRoot(normalized, i, marked);

		if (!marked[i])
		{	// if a root was merged, do not put it in the new tuple!
			merged = true;

			continue;
		}

		newRoots.push_back(this->fae.getRoot(i));

		index[i] = offset++;
	}

	// update representation
	this->fae.swapRoots(newRoots);

	for (size_t i = 0; i < this->fae.getRootCount(); ++i)
	{
		this->fae.setRoot(i, std::shared_ptr<TreeAut>(
			this->fae.relabelReferences(this->fae.getRoot(i).get(), index)
		));
	}

	this->fae.connectionGraph.finishNormalization(this->fae.getRootCount(), index);

	// update variables
	this->fae.UpdateVarsRootRefs(index);

	return merged;
}


std::set<size_t> Normalization::computeForbiddenSet(FAE& fae)
{
	// Assertions
	assert(fae.getRootCount() == fae.connectionGraph.data.size());
	assert(fae.getRootCount() >= FIXED_REG_COUNT);

	std::set<size_t> forbidden;

	VirtualMachine vm(fae);

	for (size_t i = 0; i < FIXED_REG_COUNT; ++i)
	{
		assert(fae.getRoot(vm.varGet(i).d_ref.root));
		forbidden.insert(vm.varGet(i).d_ref.root);
	}

	for (size_t i = 0; i < FIXED_REG_COUNT; ++i)
	{
		vm.getNearbyReferences(vm.varGet(i).d_ref.root, forbidden);
	}

	return forbidden;
}
