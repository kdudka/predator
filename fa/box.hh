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

#include <string>
#include <stdexcept>
#include <cassert>
#include <ostream>

#include "types.hh"
#include "labman.hh"
#include "forestaut.hh"
#include "abstractbox.hh"
/*
using std::vector;
using std::string;
using std::pair;
using std::set;
using std::make_pair;
using std::runtime_error;
*/
class TypeBox : public AbstractBox {
	
	std::string name;

	void* typeInfo;

public:

	TypeBox(const std::string& name, void* typeInfo)
		: AbstractBox(box_type_e::bTypeInfo), name(name), typeInfo(typeInfo) {}

	const std::string& getName() const {
		return this->name;
	}

	void* getTypeInfo() const {
		return this->typeInfo;
	}

	virtual void toStream(std::ostream& os) const {
		os << this->name;
	}

};

class DataBox : public AbstractBox {

	size_t id;

	const Data* data;

public:

	DataBox(size_t id, const Data* data)
		: AbstractBox(box_type_e::bData), id(id), data(data) {}

	size_t getId() const {
		return this->id;
	}

	const Data& getData() const {
		return *this->data;
	}

	virtual void toStream(std::ostream& os) const {
		os << *this->data;
	}

};

class StructuralBox : public AbstractBox {

protected:

	StructuralBox(box_type_e type) : AbstractBox(type) {
	}

public:

	virtual bool outputCovers(size_t offset) const = 0;

	virtual const std::set<size_t>& outputCoverage() const = 0;

};

class SelBox : public StructuralBox {

	const SelData* data;

	std::set<size_t> s;

public:

	SelBox(const SelData* data)
		: StructuralBox(box_type_e::bSel), data(data) {
		s.insert(data->offset);
	}

	const SelData& getData() const {
		return *this->data;
	}

	virtual void toStream(std::ostream& os) const {
		os << *this->data;
	}

	virtual size_t getArity() const {
		return 1;
	}

	virtual bool outputCovers(size_t offset) const {
		return this->data->offset == offset;
	}

	virtual const std::set<size_t>& outputCoverage() const {
		return s;
	}

};

class Box : public FA, public StructuralBox {

	friend class BoxManager;

	std::string name;

	std::vector<
		std::pair<std::vector<size_t>, std::set<size_t> >
	> selCoverage;

protected:

	Box(TA<label_type>::Manager& taMan, std::string& name)
		: FA(taMan), StructuralBox(box_type_e::bBox), name(name) {}

public:
/*
	static void getDownwardCoverage(const std::vector<const AbstractBox*>& label, std::vector<size_t>& v) {
		for (std::vector<const AbstractBox*>::const_iterator i = label.begin(); i != label.end(); ++i) {
			switch ((*i)->type) {
				case selID: v.push_back((*i)->getSelector().offset);
				case dataID: continue;
				default:
					assert((*i)->roots.size());
					std::vector<size_t> v2;
					Box::getDownwardCoverage(*(*i)->roots[0], v2);
					v.insert(v.end(), v2.begin(), v2.end());
					break;
			}
		}
	}
	
	static void getDownwardCoverage(const TA<label_type>& ta, std::vector<size_t>& v) {
		bool b = false;
		for (TA<label_type>::iterator i = ta.begin(); i != ta.end(); ++i) {
			if (!ta.isFinalState(i->rhs()))
				continue;
			std::vector<size_t> v2;
			Box::getDownwardCoverage(*i->label().dataB, v2);
			if (!b) {
				v = v2;
				b = true;
			} else {
				if (v != v2)
					throw runtime_error("Box::getDownwardCoverage(): Inconsistent accepting rules while computing selector coverage!");
			}			
		}
	}
*/

	virtual bool outputCovers(size_t offset) const {
		assert(this->selCoverage.size());
		return this->selCoverage.front().second.count(offset) > 0;
	}
	
	virtual const std::set<size_t>& outputCoverage() const {
		assert(this->selCoverage.size());
		return this->selCoverage.front().second;
	}
/*
	const std::set<size_t>& getSelCoverage(size_t x = 0) const {
		assert(x < this->selCoverage.size());
		return this->selCoverage[x].second;
	}
*/
/*
	void computeCoverage() {
		this->selCoverage.clear();
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i) {
			std::vector<size_t> v;
			Box::getDownwardCoverage(**i);
			std::set<size_t> s(v.begin(), v.end());
			if (v.size() != s.size())
				throw runtime_error("Box::computeCoverage(): A selector was defined more than once!");
			this->selCoverage.push_back(make_pair(v, s));
		}
	}
*/
/*
	void computeTrigger(vector<const Box*>& boxes) {
		boxes.clear();
		set<const Box*> s;
		for (TA<label_type>::iterator i = this->roots[0]->begin(); i != this->roots[0]->end(); ++i) {
			if (this->roots[0]->isFinalState(i->rhs()))
				s.insert(i->label().dataB->begin(), i->label().dataB->end());
		}
		boxes = vector<const Box*>(s.begin(), s.end());
	}
*/
	bool inputCovers(size_t offset) const {
		throw std::runtime_error("Box::inputCovers(): not implemented!");
	}

public:

	virtual void toStream(std::ostream& os) const {
		os << this->name;
	}

	virtual size_t getArity() const {
		return this->variables.size() - 1;
	}

};

/*
class Box : public FA {

	friend class BoxManager;

	size_t type;
	std::string name;

	union {
		Data* data;
		SelData* selInfo;
		void* info;
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
		return this->type == Box::selID && this->selInfo->offset == offset;
	}

	const SelData& getSelector() const {
		assert(this->isSelector());
		return *this->selInfo;
	}

	bool isData() const {
		return this->type == Box::dataID;
	}

	bool isData(const Data*& data) const {
		if (this->type != Box::dataID)
			return false;
		data = this->data;
		return true;
	}

	const Data& getData() const {
		assert(this->isData());
		return *this->data;
	}

	void* getInfo() const {
		assert(this->isBox());
		return this->info;
	}

	size_t getArity() const {
		switch (this->type) {
			case selID: return 1;
			case dataID: return 0;
			default: return this->variables.size()?(this->variables.size() - 1):(0);
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
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i) {
			std::vector<size_t> v = Box::getDownwardCoverage(**i);
			std::set<size_t> s(v.begin(), v.end());
			if (v.size() != s.size())
				throw runtime_error("Box::computeCoverage(): A selector was defined more than once!");
			this->selCoverage.push_back(make_pair(v, s));
		}
	}

	bool outputCovers(size_t offset) const {
		switch (this->type) {
			case Box::selID: return this->getSelector().offset == offset;
			case Box::boxID:
				if (this->selCoverage.empty())
					return false;
				return this->selCoverage.front().second.count(offset) > 0;
			default: return false;
		}
	}

	bool inputCovers(size_t offset) const {
		throw runtime_error("Box::inputCovers(): not implemented!");
	}

protected:

	Box(TA<label_type>::Manager& taMan, const std::string& name, void* info = NULL)
	 : FA(taMan), type(Box::boxID), name(name), info(info) {
		this->selCoverage.push_back(make_pair(std::vector<size_t>(), std::set<size_t>()));
	}

	Box(TA<label_type>::Manager& taMan, const std::string& name, const SelData& selInfo)
	 : FA(taMan), type(Box::selID), name(name), selInfo(new SelData(selInfo)) {
		std::set<size_t> coverage;
		coverage.insert(selInfo.offset);
		this->selCoverage.push_back(make_pair(itov(selInfo.offset), coverage));
	}

	Box(TA<label_type>::Manager& taMan, const std::string& name, const Data& data)
	 : FA(taMan), type(Box::dataID), name(name), data(new Data(data)) {}

public:

	Box(const Box& b) : FA(b), type(b.type), name(b.name), selCoverage(b.selCoverage) {
		switch (this->type) {
			case Box::selID: this->selInfo = new SelData(*b.selInfo); break;
			case Box::dataID: this->data = new Data(*b.data); break;
		}
	}

	~Box() {
		switch (this->type) {
			case Box::selID: delete this->selInfo; break;
			case Box::dataID: delete this->data; break;
		}
	}

	Box& operator=(const Box& b) {
		((FA*)this)->operator=(b);
		this->type = b.type;
		this->name = b.name;
		this->selCoverage = b.selCoverage;
		switch (this->type) {
			case Box::selID: this->selInfo = new SelData(*b.selInfo); break;
			case Box::dataID: this->data = new Data(*b.data); break;
		}
		return *this;
	}

public:

	static Box createBox(TA<label_type>::Manager& taMan, const std::string& name, void* info = NULL) {
		return Box(taMan, name, info);
	}

	static Box createSelector(TA<label_type>::Manager& taMan, const std::string& name, const SelData& selInfo) {
		return Box(taMan, name, selInfo);
	}

	static Box createData(TA<label_type>::Manager& taMan, const std::string& name, const Data& data) {
		return Box(taMan, name, data);
	}

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
*/
#endif
