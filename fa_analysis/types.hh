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

#ifndef VAR_INFO_H
#define VAR_INFO_H

#include <ostream>
#include <cassert>

#include <boost/unordered_map.hpp>

using boost::hash_value;

struct Data {

	typedef enum {
		t_undef,
		t_native_ptr,
		t_void_ptr,
		t_ref,
//		t_sel,
		t_int,
		t_bool,
		t_other
	} type_enum;

	type_enum type;

	union {
		void*	d_native_ptr;
		size_t	d_void_ptr;
		struct {
			size_t	root;
			int		offset;			
		}		d_ref;
/*		struct {
			size_t	sel;
			int		offset;			
		}		d_sel;*/
		int		d_int;
		bool	d_bool;
	};

	static Data createUndef() {
		Data data;
		data.type = type_enum::t_undef;
		return data;
	}

	static Data createNativePtr(void* ptr) {
		Data data;
		data.type = type_enum::t_native_ptr;
		data.d_native_ptr = ptr;
		return data;
	}

	static Data createVoidPtr(size_t size = 0) {
		Data data;
		data.type = type_enum::t_void_ptr;
		data.d_void_ptr = size;
		return data;
	}

	static Data createRef(size_t root, int offset = 0) {
		Data data;
		data.type = type_enum::t_ptr;
		data.d_ref.root = root;
		data.d_ref.offset = offset;
		return data;
	}

	void clear() {
		this->type = type_enum::t_undef;
	}

	bool isDefined() const {
		return this->type != type_enum::t_undef;
	}

	bool isNativePtr() const {
		return this->type == type_enum::t_native_ptr;
	}

	bool isNull() const {
		return this->type == type_enum::t_void_ptr && this->d_void_ptr == 0;
	}

	bool isRef() const {
		return this->type == type_enum::t_ref;
	}

	bool isRef(size_t root) const {
		return this->type == type_enum::t_ref && this->d_ref.root == root;
	}

	friend size_t hash_value(const Data& v) {
		switch (v.type) {
			case t_undef: return hash_value(v.type);
			case t_void_ptr: return hash_value(v.type + v.d_void_ptr);
			case t_ref: return hash_value(v.type + v.d_ref.root + v.d_ref.offset);
//			case t_sel: return hash_value(v.type + hash_value(v.d_sel));
			case t_int: return hash_value(v.type + v.d_int);
			case t_bool: return hash_value(v.type + v.d_bool);
			default: return hash_value(v.type + v.d_void_ptr);
		}
	}

	bool operator==(const Data& rhs) const {
		if (this->type != rhs.type)
			return false;
		switch (this->type) {
			case t_undef: return true;
			case t_void_ptr: return this->d_void_ptr == rhs.d_void_ptr;
			case t_ref: return this->d_ref.root == rhs.d_ref.root && this->d_ref.offset == rhs.d_ref.offset;
//			case t_sel: return this->d_sel == rhs.d_sel;
			case t_int: return this->d_int == rhs.d_int;
			case t_bool: return this->d_bool == rhs.d_bool;
			default: return false;
		}
	}

	friend std::ostream& operator<<(std::ostream& os, const Data& x) {
		switch (x.type) {
			case t_undef: os << "(undef)"; break;
			case t_void_ptr: os << "(void_ptr)" << x.d_void_ptr; break;
			case t_ref: os << "(ptr)" << x.d_ref.root << '+' << x.d_ref.offset; break;
//			case t_sel: os << "(sel)" << x.d_sel.sel << '+' << x.d_sel.offset; break;
			case t_int: os << "(int)" << x.d_int; break;
			case t_bool: os << "(bool)" << x.d_bool; break;
			default: os << "(unknown)"; break;
		}
		return os;
	}

};

struct SelData {

	size_t	offset;
	int		size;
	int		aux;

	static SelData create(size_t offset, int size, int aux) {
		SelData selData;
		selData.offset = offset;
		selData.size = size;
		selData.aux = aux;
		return selData;
	}
	
};

/*
struct VarInfo {
		
	Data data;
	int aux;

	var_info(size_t data, int aux) : data(data), aux(aux) {}

	friend size_t hash_value(const var_info& v) {
		return hash_value(v.data + v.aux);
	}

	bool operator==(const var_info& rhs) const {
		return (this->data == rhs.data) && (this->aux == rhs.aux);
	}
	
	friend std::ostream& operator<<(std::ostream& os, const var_info& x) {
		os << "v:";
		switch (x.index) {
			case (size_t)(-1): return os << "null";
			case (size_t)(-2): return os << "undef";
			default: return os << x.index << '+' << x.offset;
		}
	}
};
*/
#endif
