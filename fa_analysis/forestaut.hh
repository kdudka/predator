#ifndef FOREST_AUT_H
#define FOREST_AUT_H

#include <vector>

#include "treeaut.hh"
#include "labman.hh"

using std::vector;

class FA {

	vector<size_t> variables;

	vector<TA*> roots;

protected:

	

public:

	FA() {}

	void reorder(const vector<size_t>& variables) {
	}

	static bool subseteq(const FA& a, const FA& b) {
		
		
		
	}

};

class UFA {
	
public:

	UFA() {}

};

#endif
