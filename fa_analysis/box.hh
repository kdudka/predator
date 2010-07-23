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

#ifndef BOX_H
#define BOX_H

#include <vector>
#include <set>
#include <string>
#include <stdexcept>
#include <fstream>
#include <ostream>
#include <sstream>

#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>

#include "cache.hh"
#include "varinfo.hh"
#include "labman.hh"
#include "forestaut.hh"
#include "tatimint.hh"

using std::vector;
using std::string;
using std::pair;
using std::set;
using std::make_pair;
using std::runtime_error;

class Box : public FA {

	friend class BoxManager;

	size_t type;
	size_t tag;
	std::string name;

	std::vector<std::pair<std::vector<size_t>, std::set<size_t> > > selCoverage;

public:

	static const size_t boxID = 0;
	static const size_t selID = 1;
	static const size_t refID = 2;

	size_t getType() const {
		return this->type;
	}

	const std::set<size_t>& getSelCoverage(size_t x = 0) const {
		assert(x < this->roots.size());
		return this->selCoverage[x].second;
	}

	bool isBox() const {
		return this->type == Box::boxID;
	}

	bool isSelector() const {
		return this->type == Box::selID;
	}

	bool isSelector(size_t which) const {
		return this->type == Box::selID && this->tag == which;
	}

	size_t getSelector() const {
		assert(this->isSelector());
		return this->tag;
	}

	size_t getSelector(size_t& offset) const {
		assert(this->isSelector());
		offset = this->variables[0].offset;
		return this->tag;
	}

	size_t getSelectorOffset() const {
		assert(this->isSelector());
		return this->variables[0].offset;
	}

	bool isReference() const {
		return this->type == Box::refID;
	}

	bool isReference(size_t which) const {
		return this->type == Box::refID && this->tag == which;
	}

	size_t getReference() const {
		return this->tag;
	}

	size_t getArity() const {
		switch (this->type) {
			case selID: return 1;
			case refID: return 0;
			default: return this->variables.size() - 1;
		}
	}
	
	const std::string& getName() const {
		return this->name;
	}

public:

	static vector<size_t> getDownwardCoverage(const vector<const Box*>& label) {
		vector<size_t> v;
		for (vector<const Box*>::const_iterator i = label.begin(); i != label.end(); ++i) {
			switch ((*i)->type) {
				case selID: v.push_back((*i)->getSelector());
				case refID: continue;
				default:
					assert((*i)->roots.size());
					vector<size_t> v2 = Box::getDownwardCoverage(*(*i)->roots[0]);
					v.insert(v.end(), v2.begin(), v2.end());
					break;
			}
		}
		return v;
	}
	
	static vector<size_t> getDownwardCoverage(const TA<label_type>& ta) {
		vector<size_t> v;
		bool b = false;
		for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {
			if (!ta.isFinalState(i->rhs()))
				continue;
			vector<size_t> v2 = Box::getDownwardCoverage(*i->label().dataB);
			if (!b) {
				v = v2;
				b = true;
			} else {
				if (v != v2)
					throw runtime_error("Box::getDownwardCoverage(): Inconsistent accepting rules while computing selector coverage!");
			}			
		}
		return v;
	}

public:

	const std::pair<std::vector<size_t>, std::set<size_t> >& getDownwardCoverage(size_t index) const {
		assert(index < this->selCoverage.size());
		return this->selCoverage[index];
	}

	void computeCoverage() {
		assert(this->isBox());
		this->selCoverage.clear();
		for (vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i) {
			vector<size_t> v = Box::getDownwardCoverage(**i);
			set<size_t> s(v.begin(), v.end());
			if (v.size() != s.size())
				throw runtime_error("Box::computeCoverage(): A selector was defined more than once!");
			this->selCoverage.push_back(make_pair(v, s));
		}
	}

	bool outputCovers(size_t selector) const {
		switch (this->type) {
			case Box::selID: return this->getSelector() == selector;
			case Box::boxID: return this->selCoverage.front().second.count(selector) > 0;
			default: return false;
		}
	}

protected:

	Box(TA<FA::label_type>::Manager& taMan, size_t type, size_t tag, const std::string& name)
	 : FA(taMan), type(type), tag(tag), name(name) {}

public:

	static Box createBox(TA<FA::label_type>::Manager& taMan, const std::string& name) {
		return Box(taMan, Box::boxID, 0, name);
	}

	static Box createSelector(TA<FA::label_type>::Manager& taMan, size_t selector, size_t offset = 0) {
		std::ostringstream ss;
		ss << "s:" << selector << '+' << offset;
		Box box(taMan, Box::selID, selector, ss.str());
		box.variables.push_back(var_info(0, offset));
		set<size_t> coverage;
		coverage.insert(selector);
		box.selCoverage.push_back(make_pair(itov(selector), coverage));
		return box;
	}

	static Box createReference(TA<FA::label_type>::Manager& taMan, size_t root) {
		std::ostringstream ss;
		switch (root) {
			case varNull: ss << "r:null"; break;
			case varUndef: ss << "r:undef"; break;
			default: ss << "r:" << root;
		}
		return Box(taMan, Box::refID, root, ss.str());
	}
/*
	static FA::label_type translateLabel(LabMan& labMan, const vector<const BoxTemplate*>* label, const boost::unordered_map<const BoxTemplate*, const Box*>& args) {
		vector<const Box*> v;
		for (vector<const BoxTemplate*>::const_iterator i = label->begin(); i != label->end(); ++i) {
			boost::unordered_map<const BoxTemplate*, const Box*>::const_iterator j = args.find(*i);
			if (j == args.end())
				throw std::runtime_error("template instance undefined");
			v.push_back(j->second);
		}
		return &labMan.lookup(v);
	}
*/
/*
public:

	Box(const BoxTemplate& templ, TAManager<FA::label_type>& taMan, LabMan& labMan, const vector<size_t>& offsets, const boost::unordered_map<const BoxTemplate*, const Box*>& args)
		: FA(taMan), templ(templ) {
		if (this->templ.type == BoxTemplate::referenceID) {
			assert(offsets.size() == 2);
			this->variables.push_back(var_info(0, offsets[0]));
			this->variables.push_back(var_info(1, offsets[1]));
		} else if (this->templ.type == BoxTemplate::boxID) {
			assert(offsets.size() == this->templ.variables.size());
			for (vector<BoxTemplate::root_type>::const_iterator j = this->templ.roots.begin(); j != this->templ.roots.end(); ++j) {
				TA<FA::label_type>* ta = taMan.alloc();
				for (BoxTemplate::root_type::iterator i = j->begin(); i != j->end(); ++i)
					ta->addTransition(i->lhs(), Box::translateLabel(labMan, i->label(), args), i->rhs());
				ta->addFinalState(j->getFinalState());
				this->roots.push_back(ta);
			}
			for (size_t i = 0; i < this->templ.variables.size(); ++i)
				this->variables.push_back(var_info(this->templ.variables[i].index, offsets[i]));
		}
	}
*/
/*
	Box(TAManager<label_type>& taMan, const class BoxTemplate& templ) : FA(taMan), templ(templ) {
		
	}
*/
/*
	const BoxTemplate& getTemplate() const {
		return this->templ;
	}
*/
/*
	bool isPrimitive() const {
		return this->type == 0;
	}

	bool isRootReference() const {
		return this->type == 1;
	}
*/
/*
	friend size_t hash_value(const Box& b) {
		return hash_value(hash_value(&b.templ) + hash_value(b.variables));
	}

	bool operator==(const Box& rhs) const {
		return (&this->templ == &rhs.templ) && (this->variables == rhs.variables);
	}
*/

	void computeTrigger(vector<const Box*>& boxes) {
		boxes.clear();
		set<const Box*> s;
		for (TA<label_type>::iterator i = this->roots[0]->begin(); i != this->roots[0]->end(); ++i) {
			if (this->roots[0]->isFinalState(i->rhs()))
				s.insert(i->label().dataB->begin(), i->label().dataB->end());
		}
		boxes = vector<const Box*>(s.begin(), s.end());
	}

	friend std::ostream& operator<<(std::ostream& os, const Box& x) {
		return os << x.name;
	}

};

class BoxManager {

	mutable TA<FA::label_type>::Manager& taMan;
	mutable LabMan& labMan;

	boost::unordered_map<string, Box> boxIndex;

public:

	static const char* selPrefix;
	static const char* refPrefix;
	static const char* nullStr;

	static bool isSelector(const std::string& name) {
		return memcmp(name.c_str(), selPrefix, strlen(selPrefix)) == 0;
	}

	static size_t getSelector(const std::string& name) {
		assert(BoxManager::isSelector(name));
		return atol(name.c_str() + strlen(selPrefix));
	}

	static bool isReference(const std::string& name) {
		return (name == nullStr) || (memcmp(name.c_str(), refPrefix, strlen(refPrefix)) == 0);
	}

	static size_t getReference(const std::string& name) {
		assert(BoxManager::isReference(name));
		if (name == nullStr)
			return FA::varNull;
		return atol(name.c_str() + strlen(refPrefix));
	}

public:

	const Box& getSelector(size_t which, size_t offset = 0) {
		std::stringstream ss;
		ss << selPrefix << which;
		if (offset != 0)
			ss << '_' << offset;
		return this->boxIndex.insert(make_pair(ss.str(), Box::createSelector(this->taMan, which, offset))).first->second;
	}
	
	const Box& getReference(size_t root) {
		std::stringstream ss;
		if (root == FA::varNull)
			ss << nullStr;
		else
			ss << refPrefix << root;
		return this->boxIndex.insert(make_pair(ss.str(), Box::createReference(this->taMan, root))).first->second;
	}

protected:

	const Box& loadBox(const string& name, const boost::unordered_map<string, string>& database) {

		boost::unordered_map<string, Box>::iterator i = this->boxIndex.find(name);
		if (i != this->boxIndex.end())
			return i->second;

		if (BoxManager::isSelector(name))
			return this->getSelector(BoxManager::getSelector(name));

		if (BoxManager::isReference(name))
			return this->getReference(BoxManager::getReference(name));

		boost::unordered_map<string, string>::const_iterator j = database.find(name);
		if (j == database.end())
			throw std::runtime_error("Box '" + name + "' not found!");

		Box& box = this->boxIndex.insert(
			make_pair(name, Box::createBox(this->taMan, name))
		).first->second;

		std::fstream input(j->second.c_str());

		TAReader reader(input, j->second);

		TA<string>::Backend sBackend;
		TA<string> sta(sBackend);

		TA<FA::label_type>* ta = this->taMan.alloc();

		string autName;

		reader.readFirst(sta, autName);

		this->translateRoot(*ta, sta, database);
		box.variables.push_back(var_info(box.roots.size(), 0));
		box.roots.push_back(ta);

		while (reader.readNext(sta, autName)) {
			ta = taMan.alloc();
			this->translateRoot(*ta, sta, database);
			if (memcmp(autName.c_str(), "in", 2) == 0)
				box.variables.push_back(var_info(box.roots.size(), 0));
			box.roots.push_back(ta);
		}

		box.computeCoverage();

		return box;

	}

	void translateRoot(TA<FA::label_type>& dst, const TA<string>& src, const boost::unordered_map<string, string>& database) {
		dst.clear();
		for (TA<string>::iterator i = src.begin(); i != src.end(); ++i) {
			vector<string> strs;
			boost::split(strs, i->label(), boost::is_from_range('_', '_'));
			vector<const Box*> label;
			for (vector<string>::iterator j = strs.begin(); j != strs.end(); ++j)
				label.push_back(&this->loadBox(*j, database));
			dst.addTransition(i->lhs(), &this->labMan.lookup(label), i->rhs());
		}
		dst.addFinalState(src.getFinalState());
	}

public:

	BoxManager(TA<FA::label_type>::Manager& taMan, LabMan& labMan) : taMan(taMan), labMan(labMan) {}

	void loadTemplates(const boost::unordered_map<string, string>& database) {
		for (boost::unordered_map<string, string>::const_iterator i = database.begin(); i != database.end(); ++i)
			this->loadBox(i->first, database);
	}

	LabMan& getLabMan() const {
		return this->labMan;
	}

};

#endif
