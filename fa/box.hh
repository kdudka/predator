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
#include "databox.hh"

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

	friend class BoxMan;

	std::string name;

	std::vector<std::set<size_t> > selCoverage;

	std::vector<std::set<const AbstractBox*> > triggers;

	bool composed;
	bool initialized;

protected:

	Box(TA<label_type>::Manager& taMan, const std::string& name)
		: FA(taMan), StructuralBox(box_type_e::bBox), name(name), composed(false), initialized(false) {}

	struct LeafEnumF {

		const TA<label_type>& ta;
		size_t target;
		std::set<size_t>& selectors;

		bool getRef(size_t state, size_t& ref) const {
			TA<label_type>::Iterator i = this->ta.begin(state);
			if (i == this->ta.end(state))
				return false;
			if (!i->label().head()->isType(box_type_e::bData))
				return false;
			const Data& data = ((const DataBox*)i->label().head())->getData();
			if (!data.isRef())
				return false;
			ref = data.d_ref.root;
			return true;
		}

		LeafEnumF(const TA<label_type>& ta, size_t target, std::set<size_t>& selectors)
			: ta(ta), target(target), selectors(selectors) {}

		bool operator()(const AbstractBox* abox, std::vector<size_t>::const_iterator lhsi) {
			if (!abox->isType(box_type_e::bBox))
				return true;
			const Box* box = (const Box*)abox;
			for (size_t k = 0; k < box->getArity(); ++k, ++lhsi) {
				size_t ref;
				if (this->getRef(*lhsi, ref) && ref == this->target)
					this->selectors.insert(box->inputCoverage(k).begin(), box->inputCoverage(k).end());
			}
			return true;
		}

	};

	// enumerates upwards selectors
	void enumerateSelectorsAtLeaf(std::set<size_t>& selectors, size_t root, size_t target) const {
		assert(root < this->roots.size());
		assert(this->roots[root]);

		for (TA<label_type>::iterator i = this->roots[root]->begin(); i != this->roots[root]->end(); ++i)
			FA::iterateLabel(*i, LeafEnumF(*this->roots[root], target, selectors));
	}

public:

	static void getDownwardCoverage(const std::vector<const AbstractBox*>& label, std::vector<size_t>& v) {
		for (std::vector<const AbstractBox*>::const_iterator i = label.begin(); i != label.end(); ++i) {
			switch ((*i)->getType()) {
				case box_type_e::bSel:
					v.push_back(((const SelBox*)*i)->getData().offset);
					break;
				case box_type_e::bBox: {
					const Box* box = (const Box*)*i;
					assert(box->roots.size());
					std::vector<size_t> v2;
					Box::getDownwardCoverage(*box->roots[0], v2);
					v.insert(v.end(), v2.begin(), v2.end());
					break;
				}
				default: continue;
			}
		}
	}
	
	static void getDownwardCoverage(const TA<label_type>& ta, std::vector<size_t>& v) {
		bool b = false;
		for (TA<label_type>::iterator i = ta.accBegin(); i != ta.accEnd(i); ++i) {
			std::vector<size_t> v2;
			Box::getDownwardCoverage(*i->label().dataB, v2);
			if (!b) {
				v = v2;
				b = true;
			} else {
				if (v != v2)
					throw std::runtime_error("Box::getDownwardCoverage(): Inconsistent accepting rules while computing selector coverage!");
			}			
		}
	}

	virtual bool outputCovers(size_t offset) const {
		assert(this->selCoverage.size());
		return this->selCoverage.front().count(offset) > 0;
	}
	
	virtual const std::set<size_t>& outputCoverage() const {
		assert(this->selCoverage.size());
		return this->selCoverage.front();
	}

	bool inputCovers(size_t index, size_t offset) const {
		assert((index + 1) < this->selCoverage.size());
		return this->selCoverage[index + 1].count(offset) > 0;
	}

	const std::set<size_t>& inputCoverage(size_t index) const {
		assert((index + 1) < this->selCoverage.size());
		return this->selCoverage[index + 1];
	}
/*
	const std::set<size_t>& getSelCoverage(size_t x = 0) const {
		assert(x < this->selCoverage.size());
		return this->selCoverage[x].second;
	}
*/
	void computeCoverage() {
		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i) {
			std::vector<size_t> v;
			Box::getDownwardCoverage(**i, v);
			std::set<size_t> s(v.begin(), v.end());
			if (v.size() != s.size())
				throw std::runtime_error("Box::computeCoverage(): A selector was defined more than once!");
			this->selCoverage.push_back(s);
		}
	}

	void computeUpwardCoverage() {
		for (size_t i = 0; i < this->roots.size(); ++i) {
			for (std::vector<size_t>::iterator j = this->rootMap[i].begin(); j != this->rootMap[i].end(); ++j)
				this->enumerateSelectorsAtLeaf(this->selCoverage[*j], i, *j);
		}
	}

	void computeTriggers() {
		for (size_t i = 0; i < this->roots.size(); ++i) {
			this->triggers.push_back(std::set<const AbstractBox*>());
			for (TA<label_type>::iterator j = this->roots[i]->accBegin(); j != this->roots[i]->accEnd(j); ++j)
				this->triggers.back().insert(j->label().dataB->begin(), j->label().dataB->end());
		}
	}

	const std::set<const AbstractBox*>& getTrigger(size_t root) const {
		assert(root < this->triggers.size());
		return this->triggers[root];
	}

	const std::vector<size_t>& getO(size_t root) const {
		assert(root < this->rootMap.size());
		return this->rootMap[root];
	}

public:

	virtual void toStream(std::ostream& os) const {
		os << this->name;
	}

	virtual size_t getArity() const {
		return this->roots.size() - 1;
	}

	void initialize() {

		if (this->initialized)
			return;

		this->initialized = true;

		for (std::vector<TA<label_type>*>::iterator i = this->roots.begin(); i != this->roots.end(); ++i) {
			o_map_type o;
			FA::computeDownwardO(**i, o);
			this->rootMap.push_back(o[(*i)->getFinalState()]);
		}

		this->computeCoverage();
		this->computeUpwardCoverage();
		this->computeTriggers();
		
	}

	bool isComposed() const {
		return this->composed;
	}

	void dumpSelectorCoverage() const {

		for (std::vector<std::set<size_t> >::const_iterator i = this->selCoverage.begin(); i != this->selCoverage.end(); ++i) {
			utils::printCont(std::cerr, *i);
			std::cerr << std::endl;
		}
		
	}

};

#endif
