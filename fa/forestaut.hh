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
#include <memory>

#include <boost/unordered_map.hpp>

#include "types.hh"
#include "treeaut.hh"
#include "tatimint.hh"
#include "label.hh"
#include "abstractbox.hh"

#define _MSBM			((~(size_t)0) >> 1)
#define _MSB			(~_MSBM)
#define _MSB_TEST(x)	(x & _MSB)
#define _MSB_GET(x)		(x & _MSBM)
#define _MSB_ADD(x)		(x | _MSB)

class FA {

	friend class UFAE;

public:

	friend std::ostream& operator<<(std::ostream& os, const FA& fa) {
		os << '[';
		for (auto var : fa.variables)
			os << ' ' << var;
		os << " ]" << std::endl;
		for (size_t i = 0; i < fa.roots.size(); ++i) {
			if (!fa.roots[i])
				continue;
			os << "===" << std::endl << "root " << i << " o=[";
			for (size_t j = 0; j < fa.rootMap[i].size(); ++j)
				os << fa.rootMap[i][j].first << '(' << fa.rootMap[i][j].second << ')';
			os << ']';
			TA<label_type>& ta = *fa.roots[i];
			TAWriter<label_type> writer(os);
			for (std::set<size_t>::iterator j = ta.getFinalStates().begin(); j != ta.getFinalStates().end(); ++j)
				writer.writeState(*j);
//			writer.writeState(ta.getFinalState());
			writer.endl();
			writer.writeTransitions(ta, FA::WriteStateF());
		}
		return os;
	}

	typedef boost::unordered_map<size_t, std::vector<std::pair<size_t, bool> > > o_map_type;

public:

//	mutable TA<label_type>::Manager* taMan;

	TA<label_type>::Backend* backend;

	std::vector<Data> variables;
//	std::vector<TA<label_type>*> roots;
	std::vector<std::shared_ptr<TA<label_type> > > roots;
	std::vector<std::vector<std::pair<size_t, bool> > > rootMap;
/*
	template <class F>
	static void iterateLabel(const TT<label_type>& t, F f) {
		std::vector<size_t>::const_iterator lhsi = t.lhs().begin();
		for (std::vector<const AbstractBox*>::const_iterator i = t.label().dataB->begin(); i != t.label().dataB->end(); ++i) {
			if (!f(*i, lhsi))
				break;
			lhsi += (*i)->getArity();
		}
	}
*/
/*
	static bool isData(const AbstractBox* box, const Data*& data) {
		if (!box->isType(box_type_e::bData))
			return false;
		data = &((DataBox*)box)->getData();
		return true;
	}
*/
	static void removeMultOcc(std::vector<std::pair<size_t, bool> >& x) {
		boost::unordered_map<size_t, bool*> m;
		size_t offset = 0;
		for (size_t i = 0; i < x.size(); ++i) {
			std::pair<boost::unordered_map<size_t, bool*>::iterator, bool> p =
				m.insert(std::make_pair(x[i].first, &x[offset].second));
			if (p.second)
				x[offset++] = x[i];
			else
				*p.first->second = true;
		}
		x.resize(offset);
	}

	struct BoxCmpF {
		bool operator()(const std::pair<const AbstractBox*, std::vector<size_t> >& v1, const std::pair<const AbstractBox*, std::vector<size_t> >& v2) {
			if (v1.first->isType(box_type_e::bTypeInfo))
				return true;
			return v1.first->getOrder() < v2.first->getOrder();
		}
	};

	static void reorderBoxes(vector<const AbstractBox*>& label, std::vector<size_t>& lhs) {
		std::vector<std::pair<const AbstractBox*, std::vector<size_t> > > tmp;
		std::vector<size_t>::iterator lhsBegin = lhs.end(), lhsEnd = lhs.begin();
		for (size_t i = 0; i < label.size(); ++i) {
			lhsBegin = lhsEnd;
			lhsEnd += label[i]->getArity();
			tmp.push_back(std::make_pair(label[i], std::vector<size_t>(lhsBegin, lhsEnd)));
		}
		std::sort(tmp.begin(), tmp.end(), BoxCmpF());
		lhs.clear();
		for (size_t i = 0; i < tmp.size(); ++i) {
			label[i] = tmp[i].first;
			lhs.insert(lhs.end(), tmp[i].second.begin(), tmp[i].second.end());
		}
	}

	static bool updateO(o_map_type& o, size_t state, const std::vector<std::pair<size_t, bool> >& v) {
		std::pair<o_map_type::iterator, bool> p = o.insert(std::make_pair(state, v));

		if (p.second)
			return true;

		assert(p.first->second.size() <= v.size());

		if (p.first->second == v)
			return false;

		p.first->second = v;

		return true;

	}

	// computes downward 'o' function
	static void computeDownwardO(const TA<label_type>& ta, o_map_type& o) {
		o.clear();
		bool changed = true;
		while (changed) {
			changed = false;
			for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {
				const Data* data;
				std::vector<std::pair<size_t, bool> > v;
				if (i->label()->isData(data) && data->isRef()) {
					v.push_back(std::make_pair(data->d_ref.root, false));
					if (FA::updateO(o, i->rhs(), v))
						changed = true;
				} else {
					for (std::vector<size_t>::const_iterator j = i->lhs().begin(); j != i->lhs().end(); ++j) {
						o_map_type::iterator k = o.find(*j);
						if (k == o.end())
//							break;
							continue;
						v.insert(v.end(), k->second.begin(), k->second.end());
					}
					if (!v.empty()) {
						FA::removeMultOcc(v);
						if (FA::updateO(o, i->rhs(), v))
							changed = true;
					}
				}
			}
		}
	}
/*
	void updateRoot(TA<label_type>*& root, TA<label_type>* newRoot) {
		if (root)
			this->taMan->release(root);
		root = newRoot;
	}
*/
	void updateRootMap(size_t root) {

		assert(root < this->roots.size());
		assert(this->roots[root]);
		assert(this->roots[root]->getFinalStates().size());

		o_map_type o;
		FA::computeDownwardO(*this->roots[root], o);

		std::set<size_t>::const_iterator i = this->roots[root]->getFinalStates().begin();

		this->rootMap[root] = o[*i];

		for (++i; i != this->roots[root]->getFinalStates().end(); ++i)
			assert(this->rootMap[root] == o[*i]);

	}

	bool hasReference(size_t root, size_t target) const {
		assert(this->roots[root]);
		for (std::vector<std::pair<size_t, bool> >::const_iterator i = this->rootMap[root].begin(); i != this->rootMap[root].end(); ++i) {
			if (i->first == target)
				return true;
		}
		return false;
	}
/*
	void releaseRoots() {
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i)
			this->updateRoot(*i, NULL);
	}
*/

	TA<label_type>* allocTA() {
		return new TA<label_type>(*this->backend);
	}

public:

	static bool isData(size_t state) {
		return _MSB_TEST(state);
	}

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

//	FA(TA<label_type>::Manager& taMan) : taMan(&taMan) {}
	FA(TA<label_type>::Backend& backend) : backend(&backend) {}

	FA(const FA& src) : backend(src.backend), variables(src.variables), roots(src.roots), rootMap(src.rootMap) {
/*		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i) {
			if (*i)
				this->taMan->addRef(*i);
		}*/
	}

	FA& operator=(const FA& x) {
		this->backend = x.backend;
		this->variables = x.variables;
		this->roots = x.roots;
		this->rootMap = x.rootMap;
/*		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i) {
			if (*i)
				this->taMan->addRef(*i);
		}*/
		return *this;
	}

	void clear() {
//		this->releaseRoots();
		this->roots.clear();
		this->rootMap.clear();
		this->variables.clear();
	}

	size_t getRootCount() const {
		return this->roots.size();
	}

	const TA<label_type>* getRoot(size_t i) const {
		assert(i < this->roots.size());
		return this->roots[i].get();
	}

	void appendRoot(TA<label_type>* ta) {
		this->roots.push_back(std::shared_ptr<TA<label_type>>(ta));
	}

};

std::ostream& operator<<(std::ostream& os, const TA<label_type>& ta);

#endif
