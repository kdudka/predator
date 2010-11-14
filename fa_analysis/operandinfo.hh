/*
 * Copyright (C) 2010 Jiri Simacek
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

#ifndef OPERAND_INFO_H
#define OPERAND_INFO_H

#include <ostream>
#include <sstream>
#include <stdexcept>

#include <cl/code_listener.h>

#include "regdef.hh"
#include "forestautext.hh"
#include "nodebuilder.hh"

typedef enum { r_none, r_ref, r_reg } r_flag_e;

struct RevInfo {

	r_flag_e flag;
	Data dest;
	Data data;
	Data aux;
	std::vector<std::pair<SelData, Data> > nodeInfo;

	RevInfo(const r_flag_e& flag = r_flag_e::r_none, const Data& dest = Data(), const Data& data = Data())
		: flag(flag), dest(dest), data(data) {}
/*
	friend std::ostream& operator<<(std::ostream& os, const RevInfo& ri) {
		switch (ri.flag) {
			case o_flag_e::malloc: os << "(malloc)"; break;
			case o_flag_e::free: os << "(free)"; break;
			case o_flag_e::ref: os << "(ref)" << ri.dest.d_ref.root << '+' << ri.dest.d_ref.displ; break;
			case o_flag_e::reg: os << "(reg)" << ri.dest.d_ref.root << '+' << ri.dest.d_ref.displ; break;
		}
		return os;
	}

	void apply(FAE& fae) {
		if (this->flag == r_flag_e::none)
			return;
		CL_DEBUG("rewinding: " << this->data);
		Data tmp;
		switch (this->flag) {
			case r_flag_e::malloc:
				fae.unsafeNodeDelete(this->dest.d_ref.root
				break;
			case r_flag_e::free:
				Data ref = Data::createRef(fae.nodeCreate(this->nodeInfo));
				fae.nodeModify(this->dest.d_ref.root, this->dest.d_ref.displ, ref, tmp);
				break;
			case r_flag_e::ref:
				if (this->data.isStruct())
					fae.nodeModifyMultiple(this->dest.d_ref.root, this->dest.d_ref.displ, this->data, tmp);
				else
					fae.nodeModify(this->dest.d_ref.root, this->dest.d_ref.displ, this->data, tmp);
				break;
			case r_flag_e::reg:
				fae.varSet(this->dest.d_ref.root, this->data);
				break;
			default:
				assert(false);
		}
	}
*/
};

typedef enum { safe_ref, ref, reg, val } o_flag_e;

struct OperandInfo {

	o_flag_e flag;
	Data data;
	const cl_type* type;

	friend std::ostream& operator<<(std::ostream& os, const OperandInfo& oi) {
		switch (oi.flag) {
			case o_flag_e::ref: os << "(ref)" << oi.data.d_ref.root << '+' << oi.data.d_ref.displ; break;
			case o_flag_e::safe_ref: os << "(safe_ref)" << oi.data.d_ref.root << '+' << oi.data.d_ref.displ; break;
			case o_flag_e::reg: os << "(reg)" << oi.data.d_ref.root << '+' << oi.data.d_ref.displ; break;
			case o_flag_e::val: os << "(val)" << oi.data; break;
		}
		return os;
	}

	const cl_accessor* parseItems(const cl_accessor* acc) {

		while (acc && (acc->code == CL_ACCESSOR_ITEM)) {
			this->data.d_ref.displ += acc->type->items[acc->data.item.id].offset;
			acc = acc->next;
		}

		return acc;

	}

	const cl_accessor* parseRef(const cl_accessor* acc) {

		assert((this->flag == o_flag_e::ref) || (this->flag == o_flag_e::safe_ref));

		if (acc && (acc->code == CL_ACCESSOR_REF)) {
			this->flag = o_flag_e::val;
			acc = acc->next;
		} 

		return acc;
		
	}

	void parseCst(const cl_operand* op) {
		
		this->flag = o_flag_e::val;
		this->type = op->type;

		switch (op->data.cst.code) {
			case cl_type_e::CL_TYPE_INT:
				this->data = Data::createInt(op->data.cst.data.cst_int.value);
				break;
			default:
				assert(false);
		}

	}

	void parseVar(const FAE& fae, const cl_operand* op, size_t offset) {

		this->data = Data::createRef(fae.varGet(ABP_INDEX).d_ref.root, (int)offset);
		this->flag = o_flag_e::safe_ref;
		this->type = op->type;

		const cl_accessor* acc = op->accessor;

		if (acc && (acc->code == CL_ACCESSOR_DEREF)) {
			
			fae.nodeLookup(this->data.d_ref.root, this->data.d_ref.displ, this->data);
			if (!this->data.isRef()) {
				std::stringstream ss;
				ss << "OperandInfo::parseVar(): dereferenced value is not a valid reference - " << data << '!';
				throw std::runtime_error(ss.str());
			}

			this->flag = o_flag_e::ref;

			acc = acc->next;

		}

		acc = this->parseItems(acc);
		acc = this->parseRef(acc);
		assert(acc == NULL);

	}

	void parseReg(const FAE& fae, const cl_operand* op, size_t index) {

		// HACK: this is a bit ugly
		this->data = Data::createRef(index);
		this->type = op->type;

		const cl_accessor* acc = op->accessor;

		if (acc && (acc->code == CL_ACCESSOR_DEREF)) {

			this->data = fae.varGet(this->data.d_ref.root);
			if (!this->data.isRef()) {
				std::stringstream ss;
				ss << "OperandInfo::parseReg(): dereferenced value is not a valid reference - " << data << '!';
				throw std::runtime_error(ss.str());
			}

			this->flag = o_flag_e::ref;
			acc = this->parseItems(acc->next);
			acc = this->parseRef(acc);

		} else {

			this->flag = o_flag_e::reg;
			acc = this->parseItems(acc);

		}

		assert(acc == NULL);

	}

	static bool isRef(o_flag_e flag) {

		return flag == o_flag_e::ref || flag == o_flag_e::safe_ref;

	}

	static bool isLValue(o_flag_e flag) {

		return flag == o_flag_e::ref || flag == o_flag_e::safe_ref || flag == o_flag_e::reg;

	}

	static Data extractNestedStruct(const Data& data, size_t base, const std::vector<size_t>& offsets) {
		assert(data.isStruct());
		std::map<size_t, const Data*> m;
		for (std::vector<Data::item_info>::const_iterator i = data.d_struct->begin(); i != data.d_struct->end(); ++i)
			m.insert(make_pair(i->first, &i->second));
		Data tmp = Data::createStruct();
		for (std::vector<size_t>::const_iterator i = offsets.begin(); i != offsets.end(); ++i) {
			std::map<size_t, const Data*>::iterator j = m.find(*i + base);
			if (j == m.end())
				throw std::runtime_error("OperandInfo::extractNestedStruct(): selectors mismatch!");
			tmp.d_struct->push_back(make_pair(*i, *j->second));
		}
		return tmp;
	}

	static void modifyNestedStruct(Data& dst, size_t base, const Data& src) {
		assert(dst.isStruct());
		assert(src.isStruct());
		std::map<size_t, const Data*> m;
		for (std::vector<Data::item_info>::const_iterator i = src.d_struct->begin(); i != src.d_struct->end(); ++i)
			m.insert(make_pair(base + i->first, &i->second));
		Data tmp = Data::createStruct();
		size_t matched = 0;
		for (std::vector<Data::item_info>::iterator i = dst.d_struct->begin(); i != dst.d_struct->end(); ++i) {
			std::map<size_t, const Data*>::iterator j = m.find(i->first);
			if (j != m.end()) {
				i->second = *j->second;
				++matched;
			}
		}
		if (matched != src.d_struct->size())
			throw std::runtime_error("OperandInfo::modifyNestedStruct(): selectors mismatch!");
	}

	Data readData(const FAE& fae, const vector<size_t>& offs) {
		Data data;
		switch (this->flag) {
			case o_flag_e::ref:
			case o_flag_e::safe_ref:
				if (offs.size() > 1)
					fae.nodeLookupMultiple(this->data.d_ref.root, this->data.d_ref.displ, offs, data);
				else
					fae.nodeLookup(this->data.d_ref.root, this->data.d_ref.displ, data);
				break;
			case o_flag_e::reg:
				if (offs.size() > 1)
					data = OperandInfo::extractNestedStruct(fae.varGet(this->data.d_ref.root), this->data.d_ref.displ, offs);
				else
					data = fae.varGet(this->data.d_ref.root);
				break;
			case o_flag_e::val:
				data = this->data;
				break;
			default:
				assert(false);
		}
		CL_DEBUG("read: " << *this << " -> " << data);
		return data;
	}

	void writeData(FAE& fae, const Data& in, RevInfo& info) {
		CL_DEBUG("write: " << in << " -> " << *this);
		info.dest = this->data;
		switch (this->flag) {
			case o_flag_e::ref:
			case o_flag_e::safe_ref:
				info.flag = r_flag_e::r_ref;
				if (in.isStruct())
					fae.nodeModifyMultiple(this->data.d_ref.root, this->data.d_ref.displ, in, info.data);
				else
					fae.nodeModify(this->data.d_ref.root, this->data.d_ref.displ, in, info.data);
				break;
			case o_flag_e::reg:
				info.flag = r_flag_e::r_reg;
				info.data = fae.varGet(this->data.d_ref.root);
				if (in.isStruct()) {
					Data tmp = info.data;
					OperandInfo::modifyNestedStruct(tmp, this->data.d_ref.displ, in);
					fae.varSet(this->data.d_ref.root, tmp);
				} else {
					fae.varSet(this->data.d_ref.root, in);
				}
				break;
			default:
				assert(false);
		}
	}

};

#endif
