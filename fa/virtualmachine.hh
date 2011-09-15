/*
 * Copyright (C) 2011 Jiri Simacek
 *
 * This file is part of forester.
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

#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include <vector>
#include <cassert>

#include "types.hh"
#include "forestautext.hh"
#include "programerror.hh"

class VirtualMachine {

	FAE& fae;

protected:

	static void displToData(const SelData& sel, Data& data) {
		if (data.isRef())
			data.d_ref.displ = sel.displ;
	}

	static void displToSel(SelData& sel, Data& data) {
		if (data.isRef()) {
			sel.displ = data.d_ref.displ;
			data.d_ref.displ = 0;
		}
	}

	static bool isSelector(const AbstractBox* box) {
		return box->isType(box_type_e::bSel);
	}

	static const SelData& readSelector(const AbstractBox* box) {
		return ((SelBox*)box)->getData();
	}

	static bool isSelector(const AbstractBox* box, size_t offset) {
		if (!VirtualMachine::isSelector(box))
			return false;
		return VirtualMachine::readSelector(box).offset == offset;
	}

	void transitionLookup(const TT<label_type>& transition, size_t offset, Data& data) const {
		const NodeLabel::NodeItem& ni = transition.label()->boxLookup(offset);
		assert(VirtualMachine::isSelector(ni.aBox, offset));
		const Data* tmp;
		if (!this->fae.isData(transition.lhs()[ni.offset], tmp))
			throw ProgramError("transitionLookup(): destination is not a leaf!");
		data = *tmp;
		VirtualMachine::displToData(VirtualMachine::readSelector(ni.aBox), data);
	}

	void transitionLookup(const TT<label_type>& transition, size_t base, const std::vector<size_t>& offsets, Data& data) const {
		data = Data::createStruct();
		for (std::vector<size_t>::const_iterator i = offsets.begin(); i != offsets.end(); ++i) {
			const NodeLabel::NodeItem& ni = transition.label()->boxLookup(*i);
			assert(VirtualMachine::isSelector(ni.aBox, *i));
			const Data* tmp;
			if (!this->fae.isData(transition.lhs()[ni.offset], tmp))
				throw ProgramError("transitionLookup(): destination is not a leaf!");
			data.d_struct->push_back(Data::item_info(*i - base, *tmp));
			VirtualMachine::displToData(VirtualMachine::readSelector(ni.aBox), data.d_struct->back().second);
		}
	}

	void transitionLookup(const TT<label_type>& transition, std::vector<std::pair<SelData, Data> >& nodeInfo) const {
		size_t lhsOffset = 0;
		for (std::vector<const AbstractBox*>::const_iterator i = transition.label()->getNode().begin(); i != transition.label()->getNode().end(); ++i) {
			assert(VirtualMachine::isSelector(*i));
			const Data* tmp;
			if (!this->fae.isData(transition.lhs()[lhsOffset], tmp))
				throw ProgramError("transitionLookup(): destination is not a leaf!");
			nodeInfo.push_back(std::make_pair(VirtualMachine::readSelector(*i), *tmp));
			VirtualMachine::displToData(nodeInfo.back().first, nodeInfo.back().second);
			lhsOffset += (*i)->getArity();
		}
	}

	void transitionModify(TA<label_type>& dst, const TT<label_type>& transition, size_t offset, const Data& in, Data& out) {
		size_t state = this->fae.freshState();
		dst.addFinalState(state);
		std::vector<size_t> lhs = transition.lhs();
		std::vector<const AbstractBox*> label = transition.label()->getNode();
		const NodeLabel::NodeItem& ni = transition.label()->boxLookup(offset);
		assert(VirtualMachine::isSelector(ni.aBox, offset));
		const Data* tmp;
		if (!this->fae.isData(transition.lhs()[ni.offset], tmp))
			throw ProgramError("transitionModify(): destination is not a leaf!");
		out = *tmp;
		SelData s = VirtualMachine::readSelector(ni.aBox);
		VirtualMachine::displToData(s, out);
		Data d = in;
		VirtualMachine::displToSel(s, d);
		lhs[ni.offset] = this->fae.addData(dst, d);
		label[ni.index] = this->fae.boxMan->getSelector(s);
		FAE::reorderBoxes(label, lhs);
		dst.addTransition(lhs, this->fae.boxMan->lookupLabel(label), state);
	}

	void transitionModify(TA<label_type>& dst, const TT<label_type>& transition, size_t base, const std::vector<std::pair<size_t, Data> >& in, Data& out) {
		size_t state = this->fae.freshState();
		dst.addFinalState(state);
		std::vector<size_t> lhs = transition.lhs();
		std::vector<const AbstractBox*> label = transition.label()->getNode();
		out = Data::createStruct();
		for (std::vector<std::pair<size_t, Data>>::const_iterator i = in.begin(); i != in.end(); ++i) {
			const NodeLabel::NodeItem& ni = transition.label()->boxLookup(i->first);
			assert(VirtualMachine::isSelector(ni.aBox, i->first));
			const Data* tmp;
			if (!this->fae.isData(transition.lhs()[ni.offset], tmp))
				throw ProgramError("transitionModify(): destination is not a leaf!");
			out.d_struct->push_back(Data::item_info(i->first - base, *tmp));
			SelData s = VirtualMachine::readSelector(ni.aBox);
			VirtualMachine::displToData(s, out.d_struct->back().second);
			Data d = i->second;
			VirtualMachine::displToSel(s, d);
			lhs[ni.offset] = this->fae.addData(dst, d);
			label[ni.index] = this->fae.boxMan->getSelector(s);
		}
		FAE::reorderBoxes(label, lhs);
		dst.addTransition(lhs, this->fae.boxMan->lookupLabel(label), state);
	}

public:

	/* execution bits */
	size_t varCount() const {
		return this->fae.variables.size();
	}

	size_t varPush(const Data& data) {
		size_t id = this->fae.variables.size();
		this->fae.variables.push_back(data);
		return id;
	}

	void varPop(Data& data) {
		data = this->fae.variables.back();
		this->fae.variables.pop_back();
	}

	void varPopulate(size_t count) {
		this->fae.variables.resize(this->fae.variables.size() + count, Data::createUndef());
	}

	void varRemove(size_t count) {
		assert(count <= this->fae.variables.size());
		while (count-- > 0) this->fae.variables.pop_back();
	}

	const Data& varGet(size_t id) const {
		assert(id < this->fae.variables.size());
		return this->fae.variables[id];
	}

	void varSet(size_t id, const Data& data) {
		assert(id < this->fae.variables.size());
		this->fae.variables[id] = data;
	}

	size_t nodeCreate(const std::vector<std::pair<SelData, Data> >& nodeInfo, const TypeBox* typeInfo = NULL) {
		size_t root = this->fae.roots.size();
		TA<label_type>* ta = this->fae.allocTA();
		size_t f = this->fae.freshState();
		ta->addFinalState(f);
		std::vector<const AbstractBox*> label;
		std::vector<size_t> lhs;
		std::vector<std::pair<size_t, bool> > o;
		if (typeInfo)
			label.push_back(typeInfo);
		for (std::vector<std::pair<SelData, Data> >::const_iterator i = nodeInfo.begin(); i != nodeInfo.end(); ++i) {
			SelData sel = i->first;
			Data data = i->second;
			VirtualMachine::displToSel(sel, data);
			// label
			label.push_back(this->fae.boxMan->getSelector(sel));
			// lhs
			lhs.push_back(this->fae.addData(*ta, data));
			// o
			if (data.isRef())
				o.push_back(std::make_pair(i->second.d_ref.root, false));
		}
		FAE::reorderBoxes(label, lhs);
		ta->addTransition(lhs, this->fae.boxMan->lookupLabel(label), f);
		this->fae.appendRoot(ta);
		this->fae.rootMap.push_back(o);
		return root;
	}

	size_t nodeCreate(const std::vector<SelData>& nodeInfo, const TypeBox* typeInfo = NULL) {
		size_t root = this->fae.roots.size();
		TA<label_type>* ta = this->fae.allocTA();
		size_t f = this->fae.freshState();
		ta->addFinalState(f);
		// build label
		std::vector<const AbstractBox*> label;
		if (typeInfo)
			label.push_back(typeInfo);
		for (std::vector<SelData>::const_iterator i = nodeInfo.begin(); i != nodeInfo.end(); ++i)
			label.push_back(this->fae.boxMan->getSelector(*i));
		// build lhs
		vector<size_t> lhs(nodeInfo.size(), this->fae.addData(*ta, Data::createUndef()));
		// reorder
		FAE::reorderBoxes(label, lhs);
		// fill the rest
		ta->addTransition(lhs, this->fae.boxMan->lookupLabel(label), f);
		this->fae.appendRoot(ta);
		this->fae.rootMap.push_back(std::vector<std::pair<size_t, bool> >());
		return root;
	}

	void nodeDelete(size_t root) {
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);
		// update content of the variables
		for (std::vector<Data>::iterator i = this->fae.variables.begin(); i != this->fae.variables.end(); ++i) {
			if (i->isRef(root))
				*i = Data::createUndef();
		}
		// erase node
		this->fae.roots[root] = NULL;
		// make all references to this rootpoint dangling
		size_t i = 0;
		for (; i < root; ++i) {
			if (!this->fae.roots[i])
				continue;
			this->fae.roots[i] = std::shared_ptr<TA<label_type>>(this->fae.invalidateReference(this->fae.roots[i].get(), root));
			FAE::invalidateReference(this->fae.rootMap[i], root);
		}
		// skip 'root'
		++i;
		for (; i < this->fae.roots.size(); ++i) {
			if (!this->fae.roots[i])
				continue;
			this->fae.roots[i] = std::shared_ptr<TA<label_type>>(this->fae.invalidateReference(this->fae.roots[i].get(), root));
			FAE::invalidateReference(this->fae.rootMap[i], root);
		}
		
	}

	void unsafeNodeDelete(size_t root) {
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);
		// erase node
		this->fae.roots[root] = NULL;
	}

	void nodeLookup(size_t root, size_t offset, Data& data) const {
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);
		this->transitionLookup(this->fae.roots[root]->getAcceptingTransition(), offset, data);
	}	

	void nodeLookup(size_t root, std::vector<std::pair<SelData, Data> >& data) const {
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);
		this->transitionLookup(this->fae.roots[root]->getAcceptingTransition(), data);
	}	

	void nodeLookupMultiple(size_t root, size_t base, const std::vector<size_t>& offsets, Data& data) const {
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);
		this->transitionLookup(this->fae.roots[root]->getAcceptingTransition(), base, offsets, data);
	}	

	void nodeModify(size_t root, size_t offset, const Data& in, Data& out) {
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);
		CL_CDEBUG(2, std::endl << *this->fae.roots[root]);
		TA<label_type> ta(*this->fae.backend);
		this->transitionModify(ta, this->fae.roots[root]->getAcceptingTransition(), offset, in, out);
		this->fae.roots[root]->copyTransitions(ta);
		TA<label_type>* tmp = this->fae.allocTA();
		ta.unreachableFree(*tmp);
		this->fae.roots[root] = std::shared_ptr<TA<label_type>>(tmp);
		FA::o_map_type o;
		FAE::computeDownwardO(*tmp, o);
		this->fae.rootMap[root] = o[tmp->getFinalState()];
	}	

	void nodeModifyMultiple(size_t root, size_t offset, const Data& in, Data& out) {
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);
		assert(in.isStruct());
		TA<label_type> ta(*this->fae.backend);
		this->transitionModify(ta, this->fae.roots[root]->getAcceptingTransition(), offset, *in.d_struct, out);
		this->fae.roots[root]->copyTransitions(ta);
		TA<label_type>* tmp = this->fae.allocTA();
		ta.unreachableFree(*tmp);
		this->fae.roots[root] = std::shared_ptr<TA<label_type>>(tmp);
		FA::o_map_type o;
		FAE::computeDownwardO(*tmp, o);
		this->fae.rootMap[root] = o[tmp->getFinalState()];
	}	

	void getNearbyReferences(size_t root, std::set<size_t>& out) const {
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);
		const TT<label_type>& t = this->fae.roots[root]->getAcceptingTransition();
		for (std::vector<size_t>::const_iterator i = t.lhs().begin(); i != t.lhs().end(); ++i) {
			const Data* data;
			if (this->fae.isData(*i, data) && data->isRef())
				out.insert(data->d_ref.root);
		}
	}	

public:

	VirtualMachine(FAE& fae) : fae(fae) {}
	VirtualMachine(const FAE& fae) : fae(*(FAE*)&fae) {}

};

#endif

