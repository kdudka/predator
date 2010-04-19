#ifndef FOREST_AUT_H
#define FOREST_AUT_H

#include <vector>
#include <stdexcept>

#include "treeaut.hh"
#include "labman.hh"

using std::vector;

class FA {

	friend class UFA;

	vector<size_t> variables;

	vector<TA<const vector<size_t>*>*> roots;
	
	size_t stateOffset;

	mutable TAManager<const vector<size_t>*>& taMan;
	mutable LabMan& labMan;

protected:

	

public:

	static const size_t varNull = (size_t)(-1);
	static const size_t varUndef = (size_t)(-2);

	FA(TAManager<const vector<size_t>*>& taMan, LabMan& labMan) : stateOffset(0), taMan(taMan), labMan(labMan) {}
	
	FA(const FA& src) : variables(src.variables), roots(src.roots), stateOffset(src.stateOffset), taMan(src.taMan), labMan(src.labMan) {
		for (vector<TA<const vector<size_t>*>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			this->taMan.addRef(*i);
	}

	~FA() {
		for (vector<TA<const vector<size_t>*>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			this->taMan.release(*i);
	}

	void mergeRoot(size_t dst, size_t src) {
	}

	// try to indetify which roots to merge
	void collect() {
	}

	void reorder(const vector<size_t>& variables) {
	}

/* execution bits */
	size_t newVar() {
		size_t id = this->variables.size();
		this->variables.push_back(varUndef);
		return id;
	}
	
	void dropVars(size_t count) {
		assert(count <= this->variables.size());
		this->variables.resize(this->variables.size() - count);
	}
	
	bool x_eq_y(size_t x, size_t y) {
		return this->variables[x] == this->variables[y];
	}
	
	void x_ass_new(size_t x, size_t pointerSlots, size_t dataSlots) {
		if (dataSlots > 0)
			throw std::runtime_error("Data handling not implemented! (désolé)");
		
	}
	
	void x_ass_null(size_t x) {
		// TODO: identify garbage
		this->variables[x] = varNull;
		// TODO: reorder
	}
	
	void x_ass_y(size_t x, size_t y) {
		// TODO: identify garbage
		this->variables[x] = this->variables[y];
		// TODO: reorder
	}
	
	void x_ass_y_next(size_t x, size_t y, size_t selector) {
		// TODO: identify garbage
		
	}	

};

class UFA {
	
	TA<const vector<size_t>*>& backend;
	
	size_t stateOffset;
	
	mutable LabMan& labMan;
	
public:

	UFA(TA<const vector<size_t>*>& backend, LabMan& labMan) : backend(backend), stateOffset(1), labMan(labMan) {
		// let 0 be the only accepting state
		this->backend.addFinalState(0);
	}

	TA<const vector<size_t>*>& fa2ta(TA<const vector<size_t>*>& dst, Index<size_t>& index, const FA& src) {
		vector<size_t> lhs;
		dst.clear();
		for (vector<TA<const vector<size_t>*>*>::const_iterator i = src.roots.begin(); i != src.roots.end(); ++i) {
			TA<const vector<size_t>*>::reduce(dst, **i, index, this->stateOffset + index.size());
			lhs.push_back(index[(*i)->getFinalState()]);
		}
		dst.addTransition(lhs, labMan.lookup(src.variables, lhs.size()), 0);
		dst.addFinalState(0);
		return dst;
	}

	void join(const TA<const vector<size_t>*>& src, const Index<size_t>& index) {
		TA<const vector<size_t>*>::disjointUnion(this->backend, src);
		this->stateOffset += index.size();
	}

};

#endif
