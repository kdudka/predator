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

#ifndef BOX_MANAGER_H
#define BOX_MANAGER_H

#include <vector>
#include <string>
#include <fstream>

#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>

#include "treeaut.hh"
#include "tatimint.hh"
#include "label.hh"
#include "labman.hh"
#include "types.hh"
#include "box.hh"
#include "utils.hh"

class BoxMan {

	mutable TA<label_type>::Manager& taMan;
	mutable LabMan& labMan;

	TA<std::string>::Backend sBackend;

	boost::unordered_map<Data, const DataBox*> dataIndex;
	std::vector<const DataBox*> invDataIndex; 
	boost::unordered_map<SelData, const SelBox*> selIndex;
	boost::unordered_map<std::string, const TypeBox*> typeIndex;
	boost::unordered_map<std::string, const Box*> boxIndex;

public:

	const DataBox* getData(const Data& data) {
		std::pair<const Data, const DataBox*>& p = *this->dataIndex.insert(
			std::make_pair(data, (const DataBox*)NULL)
		).first;
		if (!p.second) {
			p.second = new DataBox(this->invDataIndex.size(), &p.first);
			this->invDataIndex.push_back(p.second);
		}
		return p.second;
	}

	const DataBox* getData(size_t index) const {
		assert(index < this->invDataIndex.size());
		return this->invDataIndex[index];
	}

	const SelBox* getSelector(const SelData& sel) {
		std::pair<const SelData, const SelBox*>& p = *this->selIndex.insert(
			std::make_pair(sel, (const SelBox*)NULL)
		).first;
		if (!p.second)
			p.second = new SelBox(&p.first);
		return p.second;
	}

	const TypeBox* getTypeInfo(const std::string& name) {
		boost::unordered_map<std::string, const TypeBox*>::const_iterator i = this->typeIndex.find(name);
		if (i == this->typeIndex.end())
			throw std::runtime_error("BoxMan::getTypeInfo(): type for " + name + " not found!");
		return i->second;
	}

	const TypeBox* createTypeInfo(const std::string& name, const std::vector<size_t>& selectors) {
		std::pair<const std::string, const TypeBox*>& p = *this->typeIndex.insert(
			std::make_pair(name, (const TypeBox*)NULL)
		).first;
		if (p.second)
			throw std::runtime_error("BoxMan::createTypeInfo(): type already exists!");
		p.second = new TypeBox(name, selectors);
		return p.second;
	}

protected:

	TA<label_type>& translateRoot(TA<label_type>& dst, bool& composed, const TA<std::string>& src, const boost::unordered_map<std::string, std::string>& database) {
		dst.clear();
		for (TA<std::string>::iterator i = src.begin(); i != src.end(); ++i) {
			std::vector<std::string> strs;
			boost::split(strs, i->label(), boost::is_from_range(',', ','));
			std::vector<const AbstractBox*> label;
			for (vector<std::string>::iterator j = strs.begin(); j != strs.end(); ++j) {
				const AbstractBox* box = this->loadBox(*j, database);
				if (box->isType(box_type_e::bBox))
					composed = true;
				label.push_back(box);
			}
			std::vector<size_t> lhs(i->lhs());
			FA::reorderBoxes(label, lhs);
			dst.addTransition(lhs, &this->labMan.lookup(label), i->rhs());
		}
		dst.addFinalState(src.getFinalState());
		return dst;
	}

public:

	struct RenameSelectedF {

		const boost::unordered_map<size_t, size_t>& index;
		
		RenameSelectedF(const boost::unordered_map<size_t, size_t>& index)
			: index(index) {}

		size_t operator()(size_t s) {
			boost::unordered_map<size_t, size_t>::const_iterator i = this->index.find(s);
			if (i == this->index.end())
				return s;
			return i->second;
		}

	};

	TA<label_type>& adjustLeaves(TA<label_type>& dst, const TA<label_type>& src) {
		boost::unordered_map<size_t, size_t> leafIndex;
		for (TA<label_type>::iterator i = src.begin(); i != src.end(); ++i) {
			if (i->label().head()->isData()) {
				const DataBox* b = this->getData(((const DataBox*)i->label().head())->getData());
				leafIndex.insert(std::make_pair(i->rhs(), _MSB_ADD(b->getId())));
			}
		}
		TA<label_type>::rename(dst, src, RenameSelectedF(leafIndex));
		return dst;
	}

	const AbstractBox* loadBox(const std::string& name, const boost::unordered_map<std::string, std::string>& database) {

		std::vector<std::string> args;

		boost::split(args, name, boost::is_from_range('_', '_'));

		if (args[0] == "sel")
			return this->getSelector(SelData::fromArgs(args));
		if (args[0] == "data")
			return this->getData(Data::fromArgs(args));
		if (args[0] == "type") {
			if (args.size() != 2)
				throw std::runtime_error("Incorrect number of arguments in type specification!");
			return this->getTypeInfo(args[1]);
		}

		std::pair<boost::unordered_map<std::string, const Box*>::iterator, bool> p =
			this->boxIndex.insert(std::make_pair(name, (const Box*)NULL));
		if (!p.second)
			return p.first->second;

		boost::unordered_map<std::string, std::string>::const_iterator j = database.find(name);
		if (j == database.end())
			throw std::runtime_error("Source for box '" + name + "' not found!");

		Box* box = new Box(this->taMan, name);

		p.first->second = box;

		std::fstream input(j->second.c_str());

		if (!input.good())
			throw std::runtime_error("Unable to open " + j->second);

		TAReader reader(input, j->second);

		TA<std::string> sta(this->sBackend);

		std::string autName;

		reader.readFirst(sta, autName);

		TA<label_type> tmp(this->taMan.getBackend());

		bool composed = false;

//		box.variables.push_back(Data::createRef(box.roots.size(), 0));
		this->translateRoot(tmp, composed, sta, database);
		box->roots.push_back(&this->adjustLeaves(*this->taMan.alloc(), tmp));

		while (reader.readNext(sta, autName)) {
			tmp.clear();
			this->translateRoot(tmp, composed, sta, database);
			box->roots.push_back(&this->adjustLeaves(*this->taMan.alloc(), tmp));
//			if (memcmp(autName.c_str(), "in", 2) == 0)
//				box.variables.push_back(Data::createRef(box.roots.size(), 0));
		}

		box->composed = composed;
		box->initialize();

		return box;

	}

public:

	BoxMan(TA<label_type>::Manager& taMan, LabMan& labMan)
		: taMan(taMan), labMan(labMan) {}

	~BoxMan() {
		utils::eraseMap(this->dataIndex);
		utils::eraseMap(this->selIndex);
		utils::eraseMap(this->typeIndex);
		utils::eraseMap(this->boxIndex);
	}
/*
	void loadTemplates(const boost::unordered_map<string, string>& database) {
		for (boost::unordered_map<string, string>::const_iterator i = database.begin(); i != database.end(); ++i)
			this->loadBox(i->first, database);
	}
*/
	LabMan& getLabMan() const {
		return this->labMan;
	}

	std::vector<const Box*> getBoxList() const {
		std::vector<const Box*> result;
		for (boost::unordered_map<std::string, const Box*>::const_iterator i = this->boxIndex.begin(); i != this->boxIndex.end(); ++i)
			result.push_back(i->second);
		return result;
	}

	 void buildBoxHierarchy(boost::unordered_map<const Box*, std::vector<const Box*> >& hierarchy, std::vector<const Box*>& basic) const {
		for (boost::unordered_map<std::string, const Box*>::const_iterator i = this->boxIndex.begin(); i != this->boxIndex.end(); ++i) {
			const std::set<const AbstractBox*>& trigger = i->second->getTrigger(0);
			bool composed = false;
			for (std::set<const AbstractBox*>::const_iterator j = trigger.begin(); j != trigger.end(); ++j) {
				if ((*j)->isBox()) {
					hierarchy.insert(std::make_pair((const Box*)*j, std::vector<const Box*>())).first->second.push_back(i->second);
					composed = true;
				}
			}
			if (!composed)
				basic.push_back(i->second);
		}
	}

public:

};

#endif
