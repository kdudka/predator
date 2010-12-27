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

#ifndef FOREST_AUT_H
#define FOREST_AUT_H

#include <vector>
#include <stdexcept>
#include <cassert>
#include <ostream>
#include <sstream>
#include <algorithm>

#include <boost/unordered_map.hpp>

#include "types.hh"
#include "treeaut.hh"
#include "tatimint.hh"
#include "label.hh"
#include "labman.hh"
#include "abstractbox.hh"
#include "databox.hh"

#define _MSBM			((~(size_t)0) >> 1)
#define _MSB			(~_MSBM)
#define _MSB_TEST(x)	(x & _MSB)
#define _MSB_GET(x)		(x & _MSBM)
#define _MSB_ADD(x)		(x | _MSB)

class FA {

	friend class UFAE;

public:

	friend std::ostream& operator<<(std::ostream& os, const FA& fa) {
		for (size_t i = 0; i < fa.roots.size(); ++i) {
			if (!fa.roots[i])
				continue;
			os << "===" << std::endl << "root " << i << " o=[";
			for (size_t j = 0; j < fa.rootMap[i].size(); ++j)
				os << fa.rootMap[i][j];
			os << ']';
			TA<label_type>& ta = *fa.roots[i];
			TAWriter<label_type> writer(os);
			writer.writeState(ta.getFinalState());
			writer.endl();
			writer.writeTransitions(ta, FA::WriteStateF());
		}
		return os;
	}

	typedef boost::unordered_map<size_t, vector<size_t> > o_map_type;

protected:

	mutable TA<label_type>::Manager* taMan;

	std::vector<Data> variables;
	std::vector<TA<label_type>*> roots;
	std::vector<std::vector<size_t> > rootMap;

	static bool isData(size_t state) {
		return _MSB_TEST(state);
	}

	static bool isData(const AbstractBox* box, const Data*& data) {
		if (!box->isType(box_type_e::bData))
			return false;
		data = &((DataBox*)box)->getData();
		return true;
	}

	static void removeMultOcc(std::vector<size_t>& x) {
		std::set<size_t> s;
		size_t offset = 0;
		for (size_t i = 0; i < x.size(); ++i) {
			if (s.insert(x[i]).second)
				x[offset++] = x[i];
		}
		x.resize(s.size());
	}

	static void reorderBoxes(vector<const AbstractBox*>& label, std::vector<size_t>& lhs) {
		std::vector<std::pair<const AbstractBox*, std::vector<size_t> > > tmp;
		std::vector<size_t>::iterator lhsBegin = lhs.end(), lhsEnd = lhs.begin();
		for (size_t i = 0; i < label.size(); ++i) {
			lhsBegin = lhsEnd;
			lhsEnd += label[i]->getArity();
			tmp.push_back(std::make_pair(label[i], std::vector<size_t>(lhsBegin, lhsEnd)));
		}
		std::sort(tmp.begin(), tmp.end());
		lhs.clear();
		for (size_t i = 0; i < tmp.size(); ++i) {
			label[i] = tmp[i].first;
			lhs.insert(lhs.end(), tmp[i].second.begin(), tmp[i].second.end());
		}
	}

	static bool updateO(o_map_type& o, size_t state, const std::vector<size_t>& v) {
		std::pair<o_map_type::iterator, bool> p = o.insert(std::make_pair(state, v));
		if (p.second)
			return true;
		if (p.first->second.size() >= v.size())
			return false;
/*		if (p.first->second.size() > v.size())
			throw runtime_error("FAE::updateO(): Inconsistent update of 'o' (length mismatch)!");*/
/*		for (size_t i = 0; i < p.first->second.size(); ++i) {
			if (v[i] != p.first->second[i])
				throw runtime_error("FAE::updateO(): Inconsistent update of 'o' (prefix mismatch)!");
		}*/
		if (p.first->second != v) {
			p.first->second = v;
			return true;
		}
		return false;
	}

	// computes downward 'o' function
	static void computeDownwardO(const TA<label_type>& ta, o_map_type& o) {
		o.clear();
		bool changed = true;
		while (changed) {
			changed = false;
			for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {
				const Data* data;
				std::vector<size_t> v;
				if (FA::isData(i->label().head(), data) && data->isRef()) {
					v.push_back(data->d_ref.root);
				} else {
//					vector<size_t> order;
//					FAE::evaluateLhsOrder(*i->label().dataB, order);
					for (std::vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j) {
						o_map_type::iterator k = o.find(*j);
						if (k == o.end())
							break;
						v.insert(v.end(), k->second.begin(), k->second.end());
					}
					FA::removeMultOcc(v);
				}
				if (FA::updateO(o, i->rhs(), v))
					changed = true;
			}
		}
	}

	void updateRoot(TA<label_type>*& root, TA<label_type>* newRoot) {
		if (root)
			this->taMan->release(root);
		root = newRoot;
	}

	void updateRootMap(size_t root) {
		assert(root < this->roots.size());
		boost::unordered_map<size_t, std::vector<size_t> > o;
		FA::computeDownwardO(*this->roots[root], o);
		this->rootMap[root] = o[this->roots[root]->getFinalState()];
	}

	void releaseRoots() {
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			this->updateRoot(*i, NULL);
	}

public:

	struct WriteStateF {

		std::string operator()(size_t state) const {
			std::ostringstream ss;
			if (_MSB_TEST(state))
				ss << 'r' << _MSB_GET(state);
			else
				ss << 'q' << state;
			return ss.str();
		}

	};

	FA(TA<label_type>::Manager& taMan) : taMan(&taMan) {}
	
	FA(const FA& src) : taMan(src.taMan), variables(src.variables), roots(src.roots), rootMap(src.rootMap) {
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i) {
			if (*i)
				this->taMan->addRef(*i);
		}
	}

	~FA() { this->clear(); }
	
	FA& operator=(const FA& x) {
		this->clear();
		this->taMan = x.taMan;
		this->variables = x.variables;
		this->roots = x.roots;
		this->rootMap = x.rootMap;
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i) {
			if (*i)
				this->taMan->addRef(*i);
		}
		return *this;		
	}
	
	void clear() {
		this->releaseRoots();
		this->roots.clear();
		this->rootMap.clear();
		this->variables.clear();
	}

	size_t getRootCount() const {
		return this->roots.size();
	}

	const TA<label_type>* getRoot(size_t i) const {
		assert(i < this->roots.size());
		return this->roots[i];
	}
	
};

std::ostream& operator<<(std::ostream& os, const TA<label_type>& ta);

#endif
