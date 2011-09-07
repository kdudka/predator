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
//#include "labman.hh"
#include "forestaut.hh"
#include "abstractbox.hh"
//#include "databox.hh"

class NodeHead : public AbstractBox {

	size_t size;

public:

	NodeHead(size_t size)
		: AbstractBox(box_type_e::bHead), size(size) {}

	size_t getSize() const {
		return this->size;
	}

	virtual void toStream(std::ostream& os) const {
		os << "Node[" << this->getSize() << ']';
	}

};

class TypeBox : public AbstractBox {
	
	std::string name;
	std::vector<size_t> selectors;

public:

	TypeBox(const std::string& name, const std::vector<size_t>& selectors)
		: AbstractBox(box_type_e::bTypeInfo), name(name), selectors(selectors) {}

	const std::string& getName() const {
		return this->name;
	}

	const std::vector<size_t>& getSelectors() const {
		return this->selectors;
	}

	virtual void toStream(std::ostream& os) const {
		os << this->name;
		if (this->selectors.empty())
			return;
		os << '{';
		for (std::vector<size_t>::const_iterator i = this->selectors.begin(); i != this->selectors.end(); ++i)
			os << *i << ';';
		os << '}';
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
		this->arity = 1;
		this->order = data->offset;
	}

	const SelData& getData() const {
		return *this->data;
	}

	virtual void toStream(std::ostream& os) const {
		os << *this->data;
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
	std::vector<std::vector<size_t> > rootSig;

	bool composed;
	bool initialized;

protected:

	Box(TA<label_type>::Backend& backend, const std::string& name)
		: FA(backend), StructuralBox(box_type_e::bBox), name(name), composed(false), initialized(false) {}

	struct LeafEnumF {

		const TA<label_type>& ta;
		const TT<label_type>& t;
		size_t target;
		std::set<size_t>& selectors;

		bool getRef(size_t state, size_t& ref) const {
			TA<label_type>::Iterator i = this->ta.begin(state);
			if (i == this->ta.end(state))
				return false;
			if (!i->label()->isData())
				return false;
			const Data& data = i->label()->getData();
			if (!data.isRef())
				return false;
			ref = data.d_ref.root;
			return true;
		}

		LeafEnumF(const TA<label_type>& ta, const TT<label_type>& t, size_t target, std::set<size_t>& selectors)
			: ta(ta), t(t), target(target), selectors(selectors) {}

		bool operator()(const AbstractBox* abox, size_t, size_t offset) {
			if (!abox->isType(box_type_e::bBox))
				return true;
			const Box* box = (const Box*)abox;
			for (size_t k = 0; k < box->getArity(); ++k, ++offset) {
				size_t ref;
				if (this->getRef(t.lhs()[offset], ref) && ref == this->target)
					this->selectors.insert(box->inputCoverage(k).begin(), box->inputCoverage(k).end());
			}
			return true;
		}

	};

	// enumerates upward selectors
	void enumerateSelectorsAtLeaf(std::set<size_t>& selectors, size_t root, size_t target) const {
		assert(root < this->roots.size());
		assert(this->roots[root]);

		for (TA<label_type>::iterator i = this->roots[root]->begin(); i != this->roots[root]->end(); ++i) {
			if (i->label()->isNode())
				i->label()->iterate(LeafEnumF(*this->roots[root], *i, target, selectors));
		}
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
			Box::getDownwardCoverage(i->label()->getNode(), v2);
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

	void computeCoverage() {
		for (auto root : this->roots) {
			std::vector<size_t> v;
			Box::getDownwardCoverage(*root, v);
			std::set<size_t> s(v.begin(), v.end());
			if (v.empty())
				throw std::runtime_error("Box::computeCoverage(): No outgoing selectors found!");
			if (v.size() != s.size())
				throw std::runtime_error("Box::computeCoverage(): A selector was defined more than once!");
			this->selCoverage.push_back(s);
		}
	}

	void computeUpwardCoverage() {
		for (size_t i = 0; i < this->roots.size(); ++i) {
			for (std::vector<std::pair<size_t, bool> >::iterator j = this->rootMap[i].begin(); j != this->rootMap[i].end(); ++j)
				this->enumerateSelectorsAtLeaf(this->selCoverage[j->first], i, j->first);
		}
	}

	void computeTriggers() {
		for (size_t i = 0; i < this->roots.size(); ++i) {
			this->triggers.push_back(std::set<const AbstractBox*>());
			for (TA<label_type>::iterator j = this->roots[i]->accBegin(); j != this->roots[i]->accEnd(j); ++j)
				this->triggers.back().insert(j->label()->getNode().begin(), j->label()->getNode().end());
		}
	}

	const std::set<const AbstractBox*>& getTrigger(size_t root) const {
		assert(root < this->triggers.size());
		return this->triggers[root];
	}

	const std::vector<std::pair<size_t, bool> >& getO(size_t root) const {
		assert(root < this->rootMap.size());
		return this->rootMap[root];
	}

	const std::vector<size_t>& getSig(size_t root) const {
		assert(root < this->rootSig.size());
		return this->rootSig[root];
	}

public:

	virtual void toStream(std::ostream& os) const {
		os << this->name;
	}

	void initialize() {

		if (this->initialized)
			return;

		this->initialized = true;
		this->arity = this->roots.size() - 1;
		
		for (auto root : this->roots) {
			o_map_type o;
			FA::computeDownwardO(*root, o);
			this->rootMap.push_back(o[root->getFinalState()]);
			std::vector<size_t> v;
			for (auto i : o[root->getFinalState()])
				v.push_back(i.first);
			this->rootSig.push_back(v);
		}

		this->computeCoverage();
		this->order = *this->selCoverage[0].begin();
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
