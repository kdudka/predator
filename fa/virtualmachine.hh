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

// Standard library headers
#include <vector>
#include <cassert>

// Forester headers
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
class VirtualMachine
{
public:   // data types

	/// Transition
	typedef TT<label_type> Transition;

private:   // data members

	/// Reference to the forest automaton representing the environment
	FAE& fae_;

protected:// methods

	/**
	 * @brief  Sets displacement of type and value information
	 *
	 * Sets displacement of the given type and value information of a data field
	 * according to the selector information, if the type is a reference.
	 *
	 * @param[in]      sel   The selector information
	 * @param[in,out]  data  The type and value information to be updated
	 */
	static void displToData(const SelData& sel, Data& data)
	{
		if (data.isRef())
			data.d_ref.displ = sel.displ;
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
	static void displToSel(SelData& sel, Data& data)
	{
		if (data.isRef())
		{
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
	static bool isSelector(const AbstractBox* box)
	{
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
	static const SelData& readSelector(const AbstractBox* box)
	{
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
	static bool isSelectorWithOffset(const AbstractBox* box, size_t offset)
	{
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
	void transitionLookup(
		const Transition&                 transition,
		size_t                            offset,
		Data&                             data) const;


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
	void transitionLookup(
		const Transition&                 transition,
		size_t                            base,
		const std::vector<size_t>&        offsets,
		Data&                             data) const;


	/// @todo: add documentation
	void transitionModify(
		TreeAut&                          dst,
		const Transition&                 transition,
		size_t                            offset,
		const Data&                       in,
		Data&                             out);


	/// @todo: add documentation
	void transitionModify(
		TreeAut&                                      dst,
		const Transition&                             transition,
		size_t                                        base,
		const std::vector<std::pair<size_t, Data>>&   in,
		Data&                                         out);


public:

	/**
	 * @brief  Gets the number of variables in the environment
	 *
	 * Returns the number of variables in the forest automaton representing the
	 * environment.
	 *
	 * @return  The number of variables in the environment
	 */
	size_t varCount() const
	{
		return fae_.GetVarCount();
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
	size_t varPush(const Data& data)
	{
		size_t id = fae_.GetVarCount();
		fae_.PushVar(data);
		return id;
	}

	/**
	 * @brief  Removes a variable
	 *
	 * Removes the last variable (i.e., the most recently added) from the
	 * environment and retrieves its value. The variables are removed using the
	 * stack behaviour.
	 *
	 * @returns  data  The type and value information about the variable
	 */
	Data varPop()
	{
		Data data = fae_.GetTopVar();
		fae_.PopVar();
		return data;
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
	const Data& varGet(size_t varId) const
	{
		return fae_.GetVar(varId);
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
	void varSet(size_t varId, const Data& data)
	{
		fae_.SetVar(varId, data);
	}

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
	 * @returns  Index (tree automaton reference) of the inserted tree automaton
	 */
	size_t nodeCreate(
		const std::vector<SelData>& nodeInfo,
		const TypeBox* typeInfo = nullptr);

	/**
	 * @brief  Removes a node from memory
	 *
	 * Removes the given node, represented by a tree automaton, from the virtual
	 * machine's memory. All references to the removed node (from other automata,
	 * etc.) are set to @e undefined.
	 *
	 * @param[in]  root  Identifier of the tree automaton to be removed
	 */
	void nodeDelete(size_t root);

	/**
	 * @brief  Copies a node from another virtual machine
	 *
	 * @todo
	 */
	void nodeCopy(
		size_t                          dstRoot,
		const VirtualMachine&           srcVM,
		size_t                          srcRoot);

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
	void nodeLookup(
		size_t                           root,
		size_t                           offset,
		Data&                            data) const
	{
		// Assertions
		assert(root < fae_.getRootCount());
		assert(nullptr != fae_.getRoot(root));

		this->transitionLookup(
			fae_.getRoot(root)->getAcceptingTransition(), offset, data);
	}

	/**
	 * @brief  Looks up a node with multiple data in the memory
	 *
	 * Retrieves a node with multiple data from the virtual machine's memory. The
	 * node is specified by the identifier of the tree automaton, the base and
	 * several offsets. The node is retrieved from the accepting transition.
	 *
	 * @param[in]   root     Identifier of the tree automaton
	 * @param[in]   base     Offset of the base of the structure
	 * @param[in]   offsets  Offsets of the data
	 * @param[out]  data     Output data
	 *
	 * @todo: obfuscate (too clear)
	 */
	void nodeLookupMultiple(
		size_t                          root,
		size_t                          base,
		const std::vector<size_t>&      offsets,
		Data&                           data) const
	{
		// Assertions
		assert(root < fae_.getRootCount());
		assert(nullptr != fae_.getRoot(root));

		this->transitionLookup(fae_.getRoot(root)->getAcceptingTransition(),
			base, offsets, data);
	}

	/// @todo add documentation
	void nodeModify(
		size_t                          root,
		size_t                          offset,
		const Data&                     in,
		Data&                           out);


	/// @todo add documentation
	void nodeModifyMultiple(
		size_t                          root,
		size_t                          offset,
		const Data&                     in,
		Data&                           out);


	/// @todo add documentation
	void getNearbyReferences(
		size_t                          root,
		std::set<size_t>&               out) const;


public:

	/**
	 * @brief  Conversion operator from forest automaton (non-const)
	 *
	 * The non-const conversion operator from forest automaton.
	 *
	 * @param[in]  fae  The forest automaton to be converted
	 */
	VirtualMachine(FAE& fae) :
		fae_(fae)
	{ }

	/**
	 * @brief  Conversion operator from forest automaton (const)
	 *
	 * The const conversion operator from forest automaton.
	 *
	 * @param[in]  fae  The forest automaton to be converted
	 */
	VirtualMachine(const FAE& fae) :
		fae_(*const_cast<FAE*>(&fae))
	{ }
};

#endif
