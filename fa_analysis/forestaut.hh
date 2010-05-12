#ifndef FOREST_AUT_H
#define FOREST_AUT_H

#include <vector>
#include <stdexcept>

#include "varinfo.hh"
#include "treeaut.hh"
#include "labman.hh"

using std::vector;
using std::pair;

class FA {

	friend class UFA;

public:

	union label_type {

		const vector<var_info>* data;
		const vector<const class Box*>* dataB;

		label_type(const vector<var_info>* data) : data(data) {}
		label_type(const vector<const class Box*>* dataB) : dataB(dataB) {}

		friend size_t hash_value(const label_type& l) {
			return hash_value(l.data);
		}

		bool operator<(const label_type& rhs) const { return this->data < rhs.data; }

		bool operator==(const label_type& rhs) const { return this->data == rhs.data; }

	};

protected:

	vector<var_info> variables;

	vector<TA<label_type>*> roots;
	
	mutable TAManager<label_type>& taMan;

public:

	static const size_t varNull = (size_t)(-1);
	static const size_t varUndef = (size_t)(-2);

	FA(TAManager<label_type>& taMan) : taMan(taMan) {}
	
	FA(const FA& src) : variables(src.variables), roots(src.roots), taMan(src.taMan) {
		for (vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			this->taMan.addRef(*i);
	}

	~FA() {
		for (vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			this->taMan.release(*i);
	}

};

class UFA {
	
	TA<FA::label_type>& backend;
	
	size_t stateOffset;
	
	mutable LabMan& labMan;
	
public:

	UFA(TA<FA::label_type>& backend, LabMan& labMan) : backend(backend), stateOffset(1), labMan(labMan) {
		// let 0 be the only accepting state
		this->backend.addFinalState(0);
	}

	TA<FA::label_type>& fa2ta(TA<FA::label_type>& dst, Index<size_t>& index, const FA& src) {
		vector<size_t> lhs;
		dst.clear();
		for (vector<TA<FA::label_type>*>::const_iterator i = src.roots.begin(); i != src.roots.end(); ++i) {
			TA<FA::label_type>::reduce(dst, **i, index, this->stateOffset, false);
			lhs.push_back(index[(*i)->getFinalState()]);
		}
		dst.addTransition(lhs, &labMan.lookup(src.variables, lhs.size()), 0);
		dst.addFinalState(0);
		return dst;
	}

	void join(const TA<FA::label_type>& src, const Index<size_t>& index) {
		TA<FA::label_type>::disjointUnion(this->backend, src, false);
		this->stateOffset += index.size();
	}

};

#endif
