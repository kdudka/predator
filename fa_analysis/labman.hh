#ifndef LAB_MAN_H
#define LAB_MAN_H

#include <vector>

#include "cache.hh"

class LabMan {
	
	Cache<std::pair<size_t, std::vector<size_t> > > store;

public:

	LabMan() {}
	
	const std::vector<size_t>* lookup(const std::vector<size_t>& x, size_t arity) {
		return &this->store.lookup(std::make_pair(arity, x))->first.second;
	}
	
};

#endif
