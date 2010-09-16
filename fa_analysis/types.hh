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
		t_void_ptr,
		t_ptr,
//		t_sel,
		t_ref,
		t_int,
		t_bool,
		t_other
	} type_enum;

	type_enum type;

	union {
		size_t	d_void_ptr;
		struct {
			size_t	root;
			int		offset;			
		}		d_ptr;
/*		struct {
			size_t	sel;
			int		offset;			
		}		d_sel;*/
		size_t	d_ref;
		int		d_int;
		bool	d_bool;
	};

	static Data createUndef() {
		Data data;
		data.type = type_enum::t_undef;
		return data;
	}

	static Data createVoidPtr(size_t size = 0) {
		Data data;
		data.type = type_enum::t_void_ptr;
		data.d_void_ptr = size;
		return data;
	}

	static Data createPtr(size_t root, int offset) {
		Data data;
		data.type = type_enum::t_ptr;
		data.d_ptr.root = root;
		data.d_ptr.offset = offset;
		return data;
	}

	static Data createRef(size_t ref) {
		Data data;
		data.type = type_enum::t_ref;
		data.d_ref= ref;
		return data;
	}

	void clear() {
		this->type = type_enum::t_undef;
	}

	bool isDefined() const {
		return this->type != type_enum::t_undef;
	}

	bool isPtr() const {
		return this->type == type_enum::t_ptr;
	}

	bool isRef() const {
		return this->type == type_enum::t_ref;
	}

	bool isRef(size_t ref) const {
		return this->type == type_enum::t_ref && this->d_ref == ref;
	}

	friend size_t hash_value(const Data& v) {
		switch (v.type) {
			case t_undef: return hash_value(v.type);
			case t_void_ptr: return hash_value(v.type + v.d_void_ptr);
			case t_ptr: return hash_value(v.type + v.d_ptr.root + v.d_ptr.offset);
//			case t_sel: return hash_value(v.type + hash_value(v.d_sel));
			case t_ref: return hash_value(v.type + v.d_ref);
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
			case t_ptr: return this->d_ptr.root == rhs.d_ptr.root && this->d_ptr.offset == rhs.d_ptr.offset;
//			case t_sel: return this->d_sel == rhs.d_sel;
			case t_ref: return this->d_ref == rhs.d_ref;
			case t_int: return this->d_int == rhs.d_int;
			case t_bool: return this->d_bool == rhs.d_bool;
			default: return false;
		}
	}

	friend std::ostream& operator<<(std::ostream& os, const Data& x) {
		switch (x.type) {
			case t_undef: os << "(undef)"; break;
			case t_void_ptr: os << "(void_ptr)" << x.d_void_ptr; break;
			case t_ptr: os << "(ptr)" << x.d_ptr.root << '+' << x.d_ptr.offset; break;
//			case t_sel: os << "(sel)" << x.d_sel.sel << '+' << x.d_sel.offset; break;
			case t_ref: os << "(ref)" << x.d_ref; break;
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
