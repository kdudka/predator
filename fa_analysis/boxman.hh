#ifndef BOX_MAN_H
#define BOX_MAN_H

#include "cache.hh"
#include "box.hh"

class BoxMan {
	
	Cache<Box> store;

public:

	BoxMan() {}

	const Box& lookup(const Box& x) {
		return this->store.lookup(x)->first;
	}

};

#endif
