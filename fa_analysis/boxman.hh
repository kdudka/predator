#ifndef BOX_MAN_H
#define BOX_MAN_H

#include "box.hh"

class BoxMan {
	
	boost::unordered_map<std::pair<BoxTemplate*, vector<size_t> >, Box> boxes;

public:

	BoxMan() {}

	const Box& lookup(const Box& x) {
		return this->store.lookup(x)->first;
	}

};

#endif
