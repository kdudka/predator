/*
 * Copyright (C) 2010 Jiri Simacek
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

#ifndef SYM_CTX_H
#define SYM_CTX_H

// Standard library headers
#include <vector>
#include <unordered_map>

// Code Listener headers
#include <cl/storage.hh>
#include <cl/clutil.hh>
#include <cl/cl_msg.hh>

// Forester headers
#include "types.hh"
#include "nodebuilder.hh"
#include "notimpl_except.hh"

#define ABP_OFFSET		0
#define ABP_SIZE		SymCtx::size_of_data
#define RET_OFFSET		(ABP_OFFSET + ABP_SIZE)
#define RET_SIZE		SymCtx::size_of_code

struct SymCtx {

	// must be initialized externally!
	static int size_of_data;
	static int size_of_code;

	// initialize size_of_void
	static void initCtx(const CodeStorage::Storage& stor) {
		size_of_code = stor.types.codePtrSizeof();
		if (size_of_code == -1)
			size_of_code = sizeof(void(*)());
		size_of_data = stor.types.dataPtrSizeof();
		if (size_of_data == -1)
			size_of_data = sizeof(void*);
	}

	const CodeStorage::Fnc& fnc;

	std::vector<SelData> sfLayout;

	class VarInfo
	{
	private:  // data members 

		bool isStack_;
		union
		{
			size_t stackOffset_;
			size_t regIndex_;
		};

	private:  // methods

		VarInfo(bool isStack, size_t offsetIndex)
			: isStack_(isStack)
		{
			if (isOnStack())
			{
				stackOffset_ = offsetIndex;
			}
			else
			{
				regIndex_ = offsetIndex;
			}
		}

	public:   // methods

		bool isOnStack() const { return isStack_;}
		size_t getStackOffset() const
		{
			// Assertions
			assert(isOnStack());

			return stackOffset_;
		}

		size_t getRegIndex() const
		{
			// Assertions
			assert(!isOnStack());

			return regIndex_;
		}

		static VarInfo createOnStack(size_t offset)
		{
			return VarInfo(true, offset);
		}

		static VarInfo createInReg(size_t index)
		{
			return VarInfo(false, index);
		}
	};

	// uid -> stack x offset/index
	typedef std::unordered_map<int, VarInfo> var_map_type;

	var_map_type varMap;

	size_t regCount;
	size_t argCount;

	SymCtx(const CodeStorage::Fnc& fnc) : fnc(fnc), regCount(2), argCount(0) {

		// pointer to previous stack frame
		this->sfLayout.push_back(SelData(ABP_OFFSET, ABP_SIZE, 0));

		// pointer to context info
		this->sfLayout.push_back(SelData(RET_OFFSET, RET_SIZE, 0));

		size_t offset = ABP_SIZE + RET_SIZE;

		for (CodeStorage::TVarSet::const_iterator i = fnc.vars.begin(); i != fnc.vars.end(); ++i) {

			const CodeStorage::Var& var = fnc.stor->vars[*i];

			switch (var.code) {
				case CodeStorage::EVar::VAR_LC:
					if (!SymCtx::isStacked(var)) {
						this->varMap.insert(
							std::make_pair(var.uid, VarInfo::createInReg(this->regCount++))
						);
						break;
					}
					// no break
				case CodeStorage::EVar::VAR_FNC_ARG:
					NodeBuilder::buildNode(this->sfLayout, var.type, offset);
					this->varMap.insert(std::make_pair(var.uid, VarInfo::createOnStack(offset)));
					offset += var.type->size;
					if (var.code == CodeStorage::EVar::VAR_FNC_ARG)
						++this->argCount;
					break;
				case CodeStorage::EVar::VAR_GL:
					throw NotImplementedException("global variables", &(var.loc));
					break;
				default:
					assert(false);
			}

		}

	}

	static bool isStacked(const CodeStorage::Var& var) {
		switch (var.code) {
			case CodeStorage::EVar::VAR_FNC_ARG: return true;
			case CodeStorage::EVar::VAR_LC: return !var.name.empty();
			case CodeStorage::EVar::VAR_GL: return false;
			default: return false;
		}
	}

	bool isReg(const cl_operand* op, size_t& id) const {
		if (op->code != cl_operand_e::CL_OPERAND_VAR)
			return false;
		var_map_type::const_iterator i = this->varMap.find(varIdFromOperand(op));
		assert(i != this->varMap.end());
		if (i->second.isOnStack())
			return false;
		id = i->second.isOnStack();
		return true;
	}

	const VarInfo& getVarInfo(size_t id) const {
		var_map_type::const_iterator i = this->varMap.find(id);
		assert(i != this->varMap.end());
		return i->second;
	}
/*
	struct Dump {

		const SymCtx& ctx;
		const FAE& fae;

		Dump(const SymCtx& ctx, const FAE& fae) : ctx(ctx), fae(fae) {}

		friend std::ostream& operator<<(std::ostream& os, const Dump& cd) {

			VirtualMachine vm(cd.fae);

			std::vector<size_t> offs;

			for (std::vector<SelData>::const_iterator i = cd.ctx.sfLayout.begin(); i != cd.ctx.sfLayout.end(); ++i)
				offs.push_back((*i).offset);

			Data data;

			vm.nodeLookupMultiple(vm.varGet(ABP_INDEX).d_ref.root, 0, offs, data);

			boost::unordered_map<size_t, Data> tmp;
			for (std::vector<Data::item_info>::const_iterator i = data.d_struct->begin(); i != data.d_struct->end(); ++i)
				tmp.insert(make_pair(i->first, i->second));

			for (CodeStorage::TVarSet::const_iterator i = cd.ctx.fnc.vars.begin(); i != cd.ctx.fnc.vars.end(); ++i) {

				const CodeStorage::Var& var = cd.ctx.fnc.stor->vars[*i];

				var_map_type::const_iterator j = cd.ctx.varMap.find(var.uid);
				assert(j != cd.ctx.varMap.end());

				switch (var.code) {
					case CodeStorage::EVar::VAR_LC:
						if (SymCtx::isStacked(var)) {
							boost::unordered_map<size_t, Data>::iterator k = tmp.find(j->second.second);
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
*/
};

#endif
