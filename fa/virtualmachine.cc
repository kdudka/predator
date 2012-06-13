/*
 * Copyright (C) 2012 Jiri Simacek
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

// Forester headers
#include "virtualmachine.hh"

void VirtualMachine::transitionLookup(
	const TT<label_type>& transition,
	size_t base,
	const std::vector<size_t>& offsets,
	Data& data) const
{
	data = Data::createStruct();

	// for every offset, add an item
	for (auto i = offsets.begin(); i != offsets.end(); ++i)
	{
		const NodeLabel::NodeItem& ni = transition.label()->boxLookup(*i + base);
		// Assertions
		assert(VirtualMachine::isSelectorWithOffset(ni.aBox, *i + base));

		const Data* tmp = nullptr;
		if (!fae_.isData(transition.lhs()[ni.offset], tmp))
		{
			throw ProgramError("transitionLookup(): destination is not a leaf!");
		}
		data.d_struct->push_back(Data::item_info(*i, *tmp));
		VirtualMachine::displToData(VirtualMachine::readSelector(ni.aBox),
			data.d_struct->back().second);
	}
}


void VirtualMachine::transitionLookup(
	const TT<label_type>& transition,
	size_t offset,
	Data& data) const
{
	// retrieve the item at given offset
	const NodeLabel::NodeItem& ni = transition.label()->boxLookup(offset);
	// Assertions
	assert(VirtualMachine::isSelectorWithOffset(ni.aBox, offset));

	const Data* tmp = nullptr;
	if (!fae_.isData(transition.lhs()[ni.offset], tmp))
	{
		throw ProgramError("transitionLookup(): destination is not a leaf!");
	}

	data = *tmp;
	VirtualMachine::displToData(VirtualMachine::readSelector(ni.aBox), data);
}


void VirtualMachine::transitionModify(
	TreeAut& dst,
	const TT<label_type>& transition,
	size_t offset,
	const Data& in,
	Data& out)
{
	// Create a new final state
	size_t state = fae_.freshState();
	dst.addFinalState(state);

	std::vector<size_t> lhs = transition.lhs();

	// Retrieve the item with given offset from the transition
	std::vector<const AbstractBox*> label = transition.label()->getNode();
	const NodeLabel::NodeItem& ni = transition.label()->boxLookup(offset);
	// Assertions
	assert(VirtualMachine::isSelectorWithOffset(ni.aBox, offset));

	const Data* tmp = nullptr;
	if (!fae_.isData(transition.lhs()[ni.offset], tmp))
	{
		throw ProgramError("transitionModify(): destination is not a leaf!");
	}

	out = *tmp;
	SelData s = VirtualMachine::readSelector(ni.aBox);
	VirtualMachine::displToData(s, out);
	Data d = in;
	VirtualMachine::displToSel(s, d);
	lhs[ni.offset] = fae_.addData(dst, d);
	label[ni.index] = fae_.boxMan->getSelector(s);
	FAE::reorderBoxes(label, lhs);
	dst.addTransition(lhs, fae_.boxMan->lookupLabel(label), state);
}


void VirtualMachine::transitionModify(
	TreeAut& dst,
	const TT<label_type>& transition,
	size_t base,
	const std::vector<std::pair<size_t, Data> >& in,
	Data& out)
{
	// Create a new final state
	size_t state = fae_.freshState();
	dst.addFinalState(state);

	std::vector<size_t> lhs = transition.lhs();

	// Get the label
	std::vector<const AbstractBox*> label = transition.label()->getNode();

	out = Data::createStruct();
	for (auto i = in.begin(); i != in.end(); ++i)
	{
		// Retrieve the item with the given offset
		const NodeLabel::NodeItem& ni = transition.label()->boxLookup(i->first + base);
		// Assertions
		assert(VirtualMachine::isSelectorWithOffset(ni.aBox, i->first + base));

		const Data* tmp = nullptr;
		if (!fae_.isData(transition.lhs()[ni.offset], tmp))
		{
			throw ProgramError("transitionModify(): destination is not a leaf!");
		}

		out.d_struct->push_back(Data::item_info(i->first, *tmp));
		SelData s = VirtualMachine::readSelector(ni.aBox);
		VirtualMachine::displToData(s, out.d_struct->back().second);
		Data d = i->second;
		VirtualMachine::displToSel(s, d);
		lhs[ni.offset] = fae_.addData(dst, d);
		label[ni.index] = fae_.boxMan->getSelector(s);
	}

	FAE::reorderBoxes(label, lhs);
	dst.addTransition(lhs, fae_.boxMan->lookupLabel(label), state);
}


size_t VirtualMachine::nodeCreate(
	const std::vector<SelData>& nodeInfo,
	const TypeBox* typeInfo)
{
	// Assertions
	assert(nullptr != fae_.boxMan);

	// create a new tree automaton
	size_t root = fae_.roots.size();
	TreeAut* ta = fae_.allocTA();
	size_t f = fae_.freshState();
	ta->addFinalState(f);

	const std::vector<SelData>* ptrNodeInfo = nullptr;

	// build the label
	std::vector<const AbstractBox*> label;
	if (typeInfo)
	{	// if there is a some box
		label.push_back(typeInfo);

		ptrNodeInfo = fae_.boxMan->LookupTypeDesc(typeInfo, nodeInfo);
	}

	for (auto i = nodeInfo.begin(); i != nodeInfo.end(); ++i)
	{	// push selector
		label.push_back(fae_.boxMan->getSelector(*i));
	}

	// build the tuple
	std::vector<size_t> lhs(nodeInfo.size(),
		fae_.addData(*ta, Data::createUndef()));

	// reorder
	FAE::reorderBoxes(label, lhs);

	// fill the rest
	ta->addTransition(lhs, fae_.boxMan->lookupLabel(label, ptrNodeInfo), f);

	// add the tree automaton into the forest automaton
	fae_.appendRoot(ta);
	fae_.connectionGraph.newRoot();
	return root;
}


void VirtualMachine::nodeDelete(size_t root)
{
	// Assertions
	assert(root < fae_.roots.size());
	assert(fae_.roots[root]);

	// update content of variables referencing the tree automaton
	fae_.SetVarsToUndefForRoot(root);

	// erase node
	fae_.roots[root] = nullptr;

	/// @todo: do in a better way (deobfuscate)
	// make all references to this rootpoint dangling
	size_t i = 0;
	for (; i < root; ++i)
	{
		if (!fae_.roots[i])
			continue;

		fae_.roots[i] = std::shared_ptr<TreeAut>(
			fae_.invalidateReference(fae_.roots[i].get(), root));
		fae_.connectionGraph.invalidate(i);
	}
	// skip 'root'
	fae_.connectionGraph.invalidate(i++);
	for (; i < fae_.roots.size(); ++i)
	{
		if (!fae_.roots[i])
			continue;

		fae_.roots[i] = std::shared_ptr<TreeAut>(
			fae_.invalidateReference(fae_.roots[i].get(), root));
		fae_.connectionGraph.invalidate(i);
	}
}


void VirtualMachine::nodeModify(
	size_t root,
	size_t offset,
	const Data& in,
	Data& out)
{
	// Assertions
	assert(root < fae_.roots.size());
	assert(fae_.roots[root]);

	TreeAut ta(*fae_.backend);
	this->transitionModify(ta, fae_.roots[root]->getAcceptingTransition(),
		offset, in, out);
	fae_.roots[root]->copyTransitions(ta);
	TreeAut* tmp = fae_.allocTA();
	ta.unreachableFree(*tmp);
	fae_.roots[root] = std::shared_ptr<TreeAut>(tmp);
	fae_.connectionGraph.invalidate(root);
}


void VirtualMachine::nodeModifyMultiple(
	size_t root,
	size_t offset,
	const Data& in,
	Data& out)
{
	// Assertions
	assert(root < fae_.roots.size());
	assert(fae_.roots[root]);
	assert(in.isStruct());

	TreeAut ta(*fae_.backend);
	this->transitionModify(ta, fae_.roots[root]->getAcceptingTransition(),
		offset, *in.d_struct, out);
	fae_.roots[root]->copyTransitions(ta);
	TreeAut* tmp = fae_.allocTA();
	ta.unreachableFree(*tmp);
	fae_.roots[root] = std::shared_ptr<TreeAut>(tmp);
	fae_.connectionGraph.invalidate(root);
}


void VirtualMachine::getNearbyReferences(
	size_t root,
	std::set<size_t>& out) const
{
	// Assertions
	assert(root < fae_.roots.size());
	assert(fae_.roots[root]);

	const TT<label_type>& t = fae_.roots[root]->getAcceptingTransition();
	for (auto i = t.lhs().begin(); i != t.lhs().end(); ++i)
	{
		const Data* data = nullptr;
		if (fae_.isData(*i, data) && data->isRef())
			out.insert(data->d_ref.root);
	}
}
