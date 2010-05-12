#ifndef LAB_MAN_H
#define LAB_MAN_H

#include <vector>
#include <utility>

#include "cache.hh"
#include "varinfo.hh"

class LabMan {
	
	Cache<std::pair<size_t, std::vector<var_info> > > varStore;
	Cache<std::vector<const class Box*> > boxStore;

public:

	LabMan() {}
	
	const std::vector<var_info>& lookup(const std::vector<var_info>& x, size_t arity) {
		return this->varStore.lookup(std::make_pair(arity, x))->first.second;
	}
	
	const std::vector<const class Box*>& lookup(const std::vector<const class Box*>& x) {
		return this->boxStore.lookup(x)->first;
	}

};

#endif
