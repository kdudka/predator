#ifndef VAR_INFO_H
#define VAR_INFO_H

#include <boost/unordered_map.hpp>

using boost::hash_value;

struct var_info {
		
	size_t index;
	size_t offset;

	var_info(size_t index, size_t offset) : index(index), offset(offset) {}
//	var_info(const var_info& x) : index(x.index), offset(x.offset) {}

	friend size_t hash_value(const var_info& v) {
		return hash_value(v.index + v.offset);
	}

	bool operator==(const var_info& rhs) const {
		return (this->index == rhs.index) && (this->offset == rhs.offset);
	}

};

#endif
