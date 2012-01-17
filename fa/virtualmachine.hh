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

#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include <vector>
#include <cassert>

#include "types.hh"
#include "forestautext.hh"
#include "programerror.hh"

/**
 * @file virtualmachine.hh
 * VirtualMachine - the representation of the symbolic execution environment
 */

/**
 * @brief  Represents the symbolic execution environment
 *
 * Represents the symbolic execution environment, i.e. the memory configuration
 * of the execution engine.
 */
class VirtualMachine {

	/// Reference to the forest automaton representing the environment
	FAE& fae;

protected:

	/**
	 * @brief  Sets displacement of type and value information
	 *
	 * Sets displacement of the given type and value information of a data field
	 * according to the selector information, if the type is a reference.
	 *
	 * @param[in]      sel   The selector information
	 * @param[in,out]  data  The type and value information to be updated
	 */
	static void displToData(const SelData& sel, Data& data) {
		if (data.isRef()) {
			data.d_ref.displ = sel.displ;
		}
	}

	/**
	 * @brief  Sets displacement of selector information
	 *
	 * This function sets displacement of selector information @p sel of type
	 * SelData from object @p data of type Data and removes the original
	 * displacement information from @p sel.
	 *
	 * @param[in, out]  sel   The selector information
	 * @param[in, out]  data  Type and value information
	 */
	static void displToSel(SelData& sel, Data& data) {
		if (data.isRef()) {
			sel.displ = data.d_ref.displ;
			data.d_ref.displ = 0;
		}
	}

	/**
	 * @brief  Is the box a selector?
	 *
	 * Returns @p true if the box is a selector, @p false otherwise.
	 *
	 * @param[in]  box  The box to be checked
	 *
	 * @returns  @p true if the box is a selector, @p false otherwise.
	 *
	 * @todo Sanitize the Box hierarchy + move functions where they belong
	 */
	static bool isSelector(const AbstractBox* box) {
		// Assertions
		assert(box != nullptr);

		return box->isType(box_type_e::bSel);
	}

	/**
	 * @brief  Reads selector data from a SelBox
	 *
	 * The function reads selector data from a SelBox
	 *
	 * @param[in]  box  The box (needs to be a SelBox)
	 *
	 * @returns  Selector data
	 *
	 * @todo Sanitize the Box hierarchy + move functions where they belong
	 */
	static const SelData& readSelector(const AbstractBox* box) {
		// Assertions
		assert(box != nullptr);
		assert(isSelector(box));
		assert(dynamic_cast<const SelBox*>(box) != nullptr);

		return static_cast<const SelBox*>(box)->getData();
	}

	/**
	 * @brief  Is the box a selector with given offset?
	 *
	 * Returns @p true if the box is a selector with given offset, @p false
	 * otherwise.
	 *
	 * @param[in]  box     The box to be checked
	 * @param[in]  offset  The offset
	 *
	 * @returns  @p true if the box is a selector with given offset, @p false
	 *           otherwise.
	 *
	 * @todo Sanitize the Box hierarchy + move functions where they belong
	 */
	static bool isSelectorWithOffset(const AbstractBox* box, size_t offset) {
		// Assertions
		assert(box != nullptr);

		if (!VirtualMachine::isSelector(box)) {
			return false;
		}

		return VirtualMachine::readSelector(box).offset == offset;
	}

	/**
	 * @brief  Retrieve data from given transition
	 *
	 * Retrieves data at given offset from a tree automaton transition.
	 *
	 * @param[in]   transition  Transition of a tree automaton
	 * @param[in]   offset      Offset from which the data is to retrieved
	 * @param[out]  data        The retrieved data
	 *
	 * @todo: sanitize the interface
	 */
	void transitionLookup(const TT<label_type>& transition,
		size_t offset, Data& data) const {

		// retrieve the item at given offset
		const NodeLabel::NodeItem& ni = transition.label()->boxLookup(offset);
		// Assertions
		assert(VirtualMachine::isSelectorWithOffset(ni.aBox, offset));

		const Data* tmp = nullptr;
		if (!this->fae.isData(transition.lhs()[ni.offset], tmp)) {
			throw ProgramError("transitionLookup(): destination is not a leaf!");
		}

		data = *tmp;
		VirtualMachine::displToData(VirtualMachine::readSelector(ni.aBox), data);
	}

	/**
	 * @brief  Retrieve data from given transition
	 *
	 * Retrieves structure with data at given offsets from a tree automaton
	 * transition.
	 *
	 * @param[in]   transition  Transition of a tree automaton
	 * @param[in]   base        Offset of the base of the structure
	 * @param[in]   offsets     Arrays of offsets from which to retrieve data
	 * @param[out]  data        The retrieved data
	 *
	 * @todo: sanitize the interface
	 */
	void transitionLookup(const TT<label_type>& transition,
		size_t base, const std::vector<size_t>& offsets, Data& data) const {

		data = Data::createStruct();

		// for every offset, add an item
		for (auto i = offsets.begin(); i != offsets.end(); ++i) {
			const NodeLabel::NodeItem& ni = transition.label()->boxLookup(*i + base);
			// Assertions
			assert(VirtualMachine::isSelectorWithOffset(ni.aBox, *i + base));

			const Data* tmp = nullptr;
			if (!this->fae.isData(transition.lhs()[ni.offset], tmp)) {
				throw ProgramError("transitionLookup(): destination is not a leaf!");
			}
			data.d_struct->push_back(Data::item_info(*i, *tmp));
			VirtualMachine::displToData(VirtualMachine::readSelector(ni.aBox),
				data.d_struct->back().second);
		}
	}

#if 0
	void transitionLookup(const TT<label_type>& transition,
		std::vector<std::pair<SelData, Data> >& nodeInfo) const {

		size_t lhsOffset = 0;
		for (auto i = transition.label()->getNode().begin();
			i != transition.label()->getNode().end(); ++i) {

			assert(VirtualMachine::isSelector(*i));
			const Data* tmp;
			if (!this->fae.isData(transition.lhs()[lhsOffset], tmp)) {
				throw ProgramError("transitionLookup(): destination is not a leaf!");
			}
			nodeInfo.push_back(std::make_pair(VirtualMachine::readSelector(*i), *tmp));
			VirtualMachine::displToData(nodeInfo.back().first, nodeInfo.back().second);
			lhsOffset += (*i)->getArity();
		}
	}
#endif

	/// @todo: add documentation
	void transitionModify(TA<label_type>& dst, const TT<label_type>& transition,
		size_t offset, const Data& in, Data& out) {

		// Create a new final state
		size_t state = this->fae.freshState();
		dst.addFinalState(state);

		std::vector<size_t> lhs = transition.lhs();

		// Retrieve the item with given offset from the transition
		std::vector<const AbstractBox*> label = transition.label()->getNode();
		const NodeLabel::NodeItem& ni = transition.label()->boxLookup(offset);
		// Assertions
		assert(VirtualMachine::isSelectorWithOffset(ni.aBox, offset));

		const Data* tmp = nullptr;
		if (!this->fae.isData(transition.lhs()[ni.offset], tmp)) {
			throw ProgramError("transitionModify(): destination is not a leaf!");
		}

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

	/// @todo: add documentation
	void transitionModify(TA<label_type>& dst, const TT<label_type>& transition,
		size_t base, const std::vector<std::pair<size_t, Data> >& in, Data& out) {

		// Create a new final state
		size_t state = this->fae.freshState();
		dst.addFinalState(state);

		std::vector<size_t> lhs = transition.lhs();

		// Get the label
		std::vector<const AbstractBox*> label = transition.label()->getNode();

		out = Data::createStruct();
		for (auto i = in.begin(); i != in.end(); ++i) {
			// Retrieve the item with the given offset
			const NodeLabel::NodeItem& ni = transition.label()->boxLookup(i->first + base);
			// Assertions
			assert(VirtualMachine::isSelectorWithOffset(ni.aBox, i->first + base));

			const Data* tmp = nullptr;
			if (!this->fae.isData(transition.lhs()[ni.offset], tmp)) {
				throw ProgramError("transitionModify(): destination is not a leaf!");
			}

			out.d_struct->push_back(Data::item_info(i->first, *tmp));
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

	/**
	 * @brief  Gets the number of variables in the environment
	 *
	 * Returns the number of variables in the forest automaton representing the
	 * environment.
	 *
	 * @return  The number of variables in the environment
	 */
	size_t varCount() const {
		return this->fae.variables.size();
	}

	/**
	 * @brief  Creates a new variable
	 *
	 * Creates a new variable in the symbolic execution environment and returns
	 * its identifier. The variables are added using the stack behaviour.
	 *
	 * @param[in]  data  The type and value information about the variable
	 *
	 * @returns  The identifier of the new variable
	 */
	size_t varPush(const Data& data) {
		size_t id = this->fae.variables.size();
		this->fae.variables.push_back(data);
		return id;
	}

	/**
	 * @brief  Removes a variable
	 *
	 * Removes the last variable (i.e., the most recently added) from the
	 * environment and retrieves its value. The variables are removed using the
	 * stack behaviour.
	 *
	 * @param[out]  data  The type and value information about the variable
	 */
	void varPop(Data& data) {
		data = this->fae.variables.back();
		this->fae.variables.pop_back();
	}

	/**
	 * @brief  Appends a number of new variables
	 *
	 * Appends the given number of variables to the environment and initializes
	 * them to @e undefined.
	 *
	 * @param[in]  count  The number of variables to be added
	 */
	void varPopulate(size_t count) {
		this->fae.variables.resize(this->fae.variables.size() + count,
			Data::createUndef());
	}

	/**
	 * @brief  Removes a number of variables
	 *
	 * Removes the specified number of variables from the top of the environment
	 * stack.
	 *
	 * @param[in]  count  The number of variables to be removed
	 */
	void varRemove(size_t count) {
		// Assertions
		assert(count <= this->fae.variables.size());

		// remove variables
		/// @todo std::vector::resize() could be more efficient
		while (count-- > 0) {
			this->fae.variables.pop_back();
		}
	}

	/**
	 * @brief  Gets information about a variable
	 *
	 * Returns the type and value information about the variable with the given
	 * identifier.
	 *
	 * @param[in]  varId  Identifier of the desired variable
	 *
	 * @returns  Type and value information about the variable
	 */
	const Data& varGet(size_t varId) const {
		// Assertions
		assert(varId < this->fae.variables.size());

		return this->fae.variables[varId];
	}

	/**
	 * @brief  Sets information about a variable
	 *
	 * Sets the type and value information about the variable with the given
	 * identifier.
	 *
	 * @param[in]  varId  Identifier of the desired variable
	 * @param[in]  data   Type and value information
	 */
	void varSet(size_t varId, const Data& data) {
		// Assertions
		assert(varId < this->fae.variables.size());

		this->fae.variables[varId] = data;
	}

#if 0
	size_t nodeCreate(const std::vector<std::pair<SelData, Data>>& nodeInfo,
		const TypeBox* typeInfo = nullptr) {

		// create a new tree automaton in the forest automaton
		size_t root = this->fae.roots.size();
		TA<label_type>* ta = this->fae.allocTA();
		size_t f = this->fae.freshState();
		ta->addFinalState(f);

		std::vector<const AbstractBox*> label;     // symbol of a TA transition
		std::vector<size_t> lhs;                   // tuple of a TA transition

		if (typeInfo) {
			label.push_back(typeInfo);
		}

		for (auto i = nodeInfo.begin(); i != nodeInfo.end(); ++i) {
			SelData sel = i->first;
			Data data = i->second;
			VirtualMachine::displToSel(sel, data);
			// label
			label.push_back(this->fae.boxMan->getSelector(sel));
			// lhs
			lhs.push_back(this->fae.addData(*ta, data));
		}

		FAE::reorderBoxes(label, lhs);
		ta->addTransition(lhs, this->fae.boxMan->lookupLabel(label), f);
		this->fae.appendRoot(ta);
		this->fae.connectionGraph.newRoot();

		return root;
	}
#endif

	/**
	 * @brief  Creates a node in the virtual machine memory
	 *
	 * This method creates a new node with given data represented by a tree
	 * automaton in the virtual machine's memory.
	 *
	 * @param[in]  nodeInfo  Description of the node
	 * @param[in]  typeInfo  Description of the type of data stored in the node
	 *                       (if present)
	 *
	 * @returns  Identifier of the inserted tree automaton
	 */
	size_t nodeCreate(const std::vector<SelData>& nodeInfo,
		const TypeBox* typeInfo = nullptr) {

		// create a new tree automaton
		size_t root = this->fae.roots.size();
		TA<label_type>* ta = this->fae.allocTA();
		size_t f = this->fae.freshState();
		ta->addFinalState(f);

		// build the label
		std::vector<const AbstractBox*> label;
		if (typeInfo)
		{	// if there is a some box
			label.push_back(typeInfo);
		}

		for (auto i = nodeInfo.begin(); i != nodeInfo.end(); ++i)
		{	// push selector
			label.push_back(this->fae.boxMan->getSelector(*i));
		}

		// build the tuple
		vector<size_t> lhs(nodeInfo.size(),
			this->fae.addData(*ta, Data::createUndef()));

		// reorder
		FAE::reorderBoxes(label, lhs);

		// fill the rest
		ta->addTransition(lhs, this->fae.boxMan->lookupLabel(label), f);

		// add the tree automaton into the forest automaton
		this->fae.appendRoot(ta);
		this->fae.connectionGraph.newRoot();
		return root;
	}

	/**
	 * @brief  Removes a node from memory
	 *
	 * Removes the given node, represented by a tree automaton, from the virtual
	 * machine's memory. All references to the removed node (from other automata,
	 * etc.) are set to @e undefined.
	 *
	 * @param[in]  root  Identifier of the tree automaton to be removed
	 */
	void nodeDelete(size_t root) {

		// Assertions
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);

		// update content of variables referencing the tree automaton
		for (auto& var : this->fae.variables) {
			if (var.isRef(root)) {
				var = Data::createUndef();
			}
		}

		// erase node
		this->fae.roots[root] = nullptr;

		/// @todo: do in a better way (deobfuscate)
		// make all references to this rootpoint dangling
		size_t i = 0;
		for (; i < root; ++i) {
			if (!this->fae.roots[i]) {
				continue;
			}

			this->fae.roots[i] = std::shared_ptr<TA<label_type>>(
				this->fae.invalidateReference(this->fae.roots[i].get(), root));
			this->fae.connectionGraph.invalidate(i);
		}
		// skip 'root'
		this->fae.connectionGraph.invalidate(i++);
		for (; i < this->fae.roots.size(); ++i) {
			if (!this->fae.roots[i]) {
				continue;
			}

			this->fae.roots[i] = std::shared_ptr<TA<label_type>>(
				this->fae.invalidateReference(this->fae.roots[i].get(), root));
			this->fae.connectionGraph.invalidate(i);
		}
	}

#if 0
	void unsafeNodeDelete(size_t root) {

		// Assertions
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);

		// erase node
		this->fae.roots[root] = nullptr;
	}
#endif

	/**
	 * @brief  Looks up a node in the memory
	 *
	 * Retrieves a node from the virtual machine's memory. The node is specified
	 * by the identifier of the tree automaton and an offset. The node is
	 * retrieved from the accepting transition.
	 *
	 * @param[in]   root    Identifier of the tree automaton
	 * @param[in]   offset  Offset of the data
	 * @param[out]  data    Output data
	 *
	 * @todo: obfuscate (too clear)
	 */
	void nodeLookup(size_t root, size_t offset, Data& data) const {

		// Assertions
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);

		this->transitionLookup(
			this->fae.roots[root]->getAcceptingTransition(), offset, data);
	}

#if 0
	void nodeLookup(size_t root, std::vector<std::pair<SelData, Data> >& data) const {
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);
		this->transitionLookup(this->fae.roots[root]->getAcceptingTransition(), data);
	}
#endif

	/**
	 * @brief  Looks up a node with multiple data in the memory
	 *
	 * Retrieves a node with multiple data from the virtual machine's memory. The
	 * node is specified by the identifier of the tree automaton and an offset.
	 * The node is retrieved from the accepting transition.
	 *
	 * @param[in]   root     Identifier of the tree automaton
	 * @param[in]   base     Offset of the base of the structure
	 * @param[in]   offsets  Offsets of the data
	 * @param[out]  data     Output data
	 *
	 * @todo: obfuscate (too clear)
	 */
	void nodeLookupMultiple(size_t root, size_t base,
		const std::vector<size_t>& offsets, Data& data) const {

		// Assertions
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);

		this->transitionLookup(this->fae.roots[root]->getAcceptingTransition(),
			base, offsets, data);
	}

	/// @todo add documentation
	void nodeModify(size_t root, size_t offset, const Data& in, Data& out) {

		// Assertions
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);

		TA<label_type> ta(*this->fae.backend);
		this->transitionModify(ta, this->fae.roots[root]->getAcceptingTransition(),
			offset, in, out);
		this->fae.roots[root]->copyTransitions(ta);
		TA<label_type>* tmp = this->fae.allocTA();
		ta.unreachableFree(*tmp);
		this->fae.roots[root] = std::shared_ptr<TA<label_type>>(tmp);
		this->fae.connectionGraph.invalidate(root);
	}

	/// @todo add documentation
	void nodeModifyMultiple(size_t root, size_t offset, const Data& in, Data& out) {

		// Assertions
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);
		assert(in.isStruct());

		TA<label_type> ta(*this->fae.backend);
		this->transitionModify(ta, this->fae.roots[root]->getAcceptingTransition(),
			offset, *in.d_struct, out);
		this->fae.roots[root]->copyTransitions(ta);
		TA<label_type>* tmp = this->fae.allocTA();
		ta.unreachableFree(*tmp);
		this->fae.roots[root] = std::shared_ptr<TA<label_type>>(tmp);
		this->fae.connectionGraph.invalidate(root);
	}

	/// @todo add documentation
	void getNearbyReferences(size_t root, std::set<size_t>& out) const {

		// Assertions
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);

		const TT<label_type>& t = this->fae.roots[root]->getAcceptingTransition();
		for (auto i = t.lhs().begin(); i != t.lhs().end(); ++i) {
			const Data* data = nullptr;
			if (this->fae.isData(*i, data) && data->isRef())
				out.insert(data->d_ref.root);
		}
	}

public:

	/**
	 * @brief  Conversion operator from forest automaton (non-const)
	 *
	 * The non-const conversion operator from forest automaton.
	 *
	 * @param[in]  fae  The forest automaton to be converted
	 */
	VirtualMachine(FAE& fae) : fae(fae) {}

	/**
	 * @brief  Conversion operator from forest automaton (const)
	 *
	 * The const conversion operator from forest automaton.
	 *
	 * @param[in]  fae  The forest automaton to be converted
	 */
	VirtualMachine(const FAE& fae) : fae(*const_cast<FAE*>(&fae)) {}

};

#endif

