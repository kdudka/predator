#ifndef FOREST_AUT_H
#define FOREST_AUT_H

#include <vector>
#include <stdexcept>
#include <cassert>

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

		const class Box& head() const {
			assert(this->dataB);
			assert(this->dataB->size() > 0);
			return *(*this->dataB)[0];
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
		this->clear();
	}
	
	void clear() {
		for (vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			this->taMan.release(*i);
		this->roots.clear();
		this->variables.clear()
	}

};

#endif
