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
#include "types.hh"
//#include "varinfo.hh"
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
	std::string name;

	union {
		Data data;
		SelData selInfo;
	};

	std::vector<std::pair<std::vector<size_t>, std::set<size_t> > > selCoverage;
	
public:

	static const size_t boxID = 0;
	static const size_t selID = 1;
	static const size_t dataID = 2;

	size_t getType() const {
		return this->type;
	}

	bool isType(size_t type) const {
		return this->type == type;
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

	bool isSelector(size_t offset) const {
		return this->type == Box::selID && this->selInfo.offset == offset;
	}

	const SelData& getSelector() const {
		assert(this->isSelector());
		return this->selInfo;
	}

	bool isData() const {
		return this->type == Box::dataID;
	}

	bool isData(const Data*& data) const {
		if (this->type != Box::dataID)
			return false;
		data = &this->data;
		return true;
	}

	const Data& getData() const {
		assert(this->isData());
		return this->data;
	}
/*
	bool isReference() const {
		return this->type == Box::refID;
	}

	bool isReference(size_t which) const {
		return this->type == Box::refID && this->uintTag == which;
	}

	size_t getReference() const {
		assert(this->isReference());
		return this->uintTag;
	}

*/
	size_t getArity() const {
		switch (this->type) {
			case selID: return 1;
			case dataID: return 0;
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
				case selID: v.push_back((*i)->getSelector().offset);
				case dataID: continue;
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

	bool outputCovers(size_t offset) const {
		switch (this->type) {
			case Box::selID: return this->getSelector().offset == offset;
			case Box::boxID: return this->selCoverage.front().second.count(offset) > 0;
			default: return false;
		}
	}

	bool inputCovers(size_t offset) const {
		throw runtime_error("Box::inputCovers(): not implemented!");
	}

protected:

	Box(TA<FA::label_type>::Manager& taMan, const std::string& name)
	 : FA(taMan), type(Box::boxID), name(name) {}

	Box(TA<FA::label_type>::Manager& taMan, const std::string& name, const SelData& selInfo)
	 : FA(taMan), type(Box::selID), name(name), selInfo(selInfo) {
		set<size_t> coverage;
		coverage.insert(selInfo.offset);
		this->selCoverage.push_back(make_pair(itov(selInfo.offset), coverage));
	}

	Box(TA<FA::label_type>::Manager& taMan, const std::string& name, const Data& data)
	 : FA(taMan), type(Box::dataID), name(name), data(data) {}

public:

	static Box createBox(TA<FA::label_type>::Manager& taMan, const std::string& name) {
		return Box(taMan, name);
	}

	static Box createSelector(TA<FA::label_type>::Manager& taMan, const std::string& name, const SelData& selInfo) {
		return Box(taMan, name, selInfo);
	}
/*
	static Box createReference(TA<FA::label_type>::Manager& taMan, size_t root) {
		std::ostringstream ss;
		switch (root) {
			case varNull: ss << "null"; break;
			case varUndef: ss << "undef"; break;
			default: ss << "r:" << root;
		}
		return Box(taMan, Box::refID, root, ss.str());
	}
*/
	static Box createData(TA<FA::label_type>::Manager& taMan, const std::string& name, const Data& data) {
		return Box(taMan, name, data);
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
	static const char* dataPrefix;
	static const char* nullStr;
	static const char* undefStr;

	static bool isSelectorName(const std::string& name) {
		return memcmp(name.c_str(), selPrefix, strlen(selPrefix)) == 0;
	}

	// TODO: parse size and aux
	static SelData getSelectorFromName(const std::string& name) {
		assert(BoxManager::isSelectorName(name));
		SelData data;
		data.offset = atol(name.c_str() + strlen(selPrefix));
		return data;
	}

	static bool isReferenceName(const std::string& name) {
		return (name == nullStr) || (name == undefStr) || (memcmp(name.c_str(), refPrefix, strlen(refPrefix)) == 0);
	}

	static Data getReferenceFromName(const std::string& name) {
		assert(BoxManager::isReferenceName(name));
		if (name == nullStr)
			return Data::createVoidPtr(0);
		if (name == undefStr)
			return Data::createUndef();
		return Data::createRef(atol(name.c_str() + strlen(refPrefix)));
	}
/*
	static bool isDataName(const std::string& name) {
		return (memcmp(name.c_str(), refPrefix, strlen(dataPrefix)) == 0);
	}

	static Data getDataFromName(const std::string& name) {
		assert(BoxManager::isData(name));
		return atol(name.c_str() + strlen(dataPrefix));
	}
*/
public:

	const Box& getSelector(const SelData& selInfo) {
		std::stringstream ss;
		ss << selPrefix << '|' << selInfo.offset << '|' << selInfo.size << '|' << selInfo.aux;
		return this->boxIndex.insert(
			make_pair(ss.str(), Box::createSelector(this->taMan, ss.str(), selInfo))
		).first->second;
	}

	const Box& getData(const Data& data) {
		std::stringstream ss;
		ss << data;
		return this->boxIndex.insert(
			make_pair(ss.str(), Box::createData(this->taMan, ss.str(), data))
		).first->second;
	}

protected:

	const Box& loadBox(const string& name, const boost::unordered_map<string, string>& database) {

		boost::unordered_map<string, Box>::iterator i = this->boxIndex.find(name);
		if (i != this->boxIndex.end())
			return i->second;

		if (BoxManager::isSelectorName(name))
			return this->getSelector(BoxManager::getSelectorFromName(name));

		if (BoxManager::isReferenceName(name))
			return this->getData(BoxManager::getReferenceFromName(name));

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
		box.variables.push_back(Data::createPtr(box.roots.size(), 0));
		box.roots.push_back(ta);

		while (reader.readNext(sta, autName)) {
			ta = taMan.alloc();
			this->translateRoot(*ta, sta, database);
			if (memcmp(autName.c_str(), "in", 2) == 0)
				box.variables.push_back(Data::createPtr(box.roots.size(), 0));
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
