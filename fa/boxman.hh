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

	const TypeBox* getInfo(const std::string& name, void* info = NULL) {
		std::pair<const std::string, const TypeBox*>& p = *this->typeIndex.insert(
			std::make_pair(name, (const TypeBox*)NULL)
		).first;
		if (!p.second)
			p.second = new TypeBox(name, info);
		return p.second;

	}

protected:

	TA<label_type>& translateRoot(TA<label_type>& dst, const TA<std::string>& src, const boost::unordered_map<std::string, std::string>& database) {
		dst.clear();
		for (TA<std::string>::iterator i = src.begin(); i != src.end(); ++i) {
			std::vector<std::string> strs;
			boost::split(strs, i->label(), boost::is_from_range(',', ','));
			std::vector<const AbstractBox*> label;
			for (vector<std::string>::iterator j = strs.begin(); j != strs.end(); ++j)
				label.push_back(this->loadBox(*j, database));
//			std::vector<size_t> lhs(i->lhs());
//			FA::reorderBoxes(label, lhs);
			dst.addTransition(i->lhs(), &this->labMan.lookup(label), i->rhs());
		}
		dst.addFinalState(src.getFinalState());
		return dst;
	}

public:

	const AbstractBox* loadBox(const std::string& name, const boost::unordered_map<std::string, std::string>& database) {

		std::vector<std::string> args;

		boost::split(args, name, boost::is_from_range('_', '_'));

		if (args[0] == "sel")
			return this->getSelector(SelData::fromArgs(args));
		if (args[0] == "data")
			return this->getData(Data::fromArgs(args));

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

		TAReader reader(input, j->second);

		TA<std::string> sta(this->sBackend);

		std::string autName;

		reader.readFirst(sta, autName);

		TA<label_type> tmp(this->taMan.getBackend());

//		box.variables.push_back(Data::createRef(box.roots.size(), 0));
		this->translateRoot(tmp, sta, database);
		box->roots.push_back(this->taMan.clone(&tmp));

		while (reader.readNext(sta, autName)) {
			tmp.clear();
			this->translateRoot(tmp, sta, database);
			box->roots.push_back(this->taMan.clone(&tmp));
//			if (memcmp(autName.c_str(), "in", 2) == 0)
//				box.variables.push_back(Data::createRef(box.roots.size(), 0));
		}

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

public:
/*
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
		return SelData(atol(name.c_str() + strlen(selPrefix)), 0, 0);
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

	static bool isDataName(const std::string& name) {
		return (memcmp(name.c_str(), refPrefix, strlen(dataPrefix)) == 0);
	}

	static Data getDataFromName(const std::string& name) {
		assert(BoxManager::isData(name));
		return atol(name.c_str() + strlen(dataPrefix));
	}
*/
};

/*
class BoxManager {

	mutable TA<label_type>::Manager& taMan;
	mutable LabMan& labMan;

	boost::unordered_map<string, Box> boxIndex;

	LeafManager<const Data> dataMan;

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
		return SelData(atol(name.c_str() + strlen(selPrefix)), 0, 0);
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

	static bool isDataName(const std::string& name) {
		return (memcmp(name.c_str(), refPrefix, strlen(dataPrefix)) == 0);
	}

	static Data getDataFromName(const std::string& name) {
		assert(BoxManager::isData(name));
		return atol(name.c_str() + strlen(dataPrefix));
	}

public:

	const Box& getSelector(const SelData& sel) {
		std::stringstream ss;
		ss << selPrefix << '|' << sel.offset << '|' << sel.size << '|' << sel.displ;
		return this->boxIndex.insert(
			make_pair(ss.str(), Box::createSelector(this->taMan, ss.str(), sel))
		).first->second;
	}

	const Box& getData(const Data& data) {
		std::stringstream ss;
		ss << data;
		return this->boxIndex.insert(
			make_pair(ss.str(), Box::createData(this->taMan, ss.str(), data))
		).first->second;
	}

	const Box& getInfo(const std::string& name, void* info = NULL) {
		return this->boxIndex.insert(
			make_pair("$" + name, Box::createBox(this->taMan, "$" + name, info))
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

		TA<label_type>* ta = this->taMan.alloc();

		string autName;

		reader.readFirst(sta, autName);

		this->translateRoot(*ta, sta, database);
		box.variables.push_back(Data::createRef(box.roots.size(), 0));
		box.roots.push_back(ta);

		while (reader.readNext(sta, autName)) {
			ta = taMan.alloc();
			this->translateRoot(*ta, sta, database);
			if (memcmp(autName.c_str(), "in", 2) == 0)
				box.variables.push_back(Data::createRef(box.roots.size(), 0));
			box.roots.push_back(ta);
		}

		box.computeCoverage();

		return box;

	}

	void translateRoot(TA<label_type>& dst, const TA<string>& src, const boost::unordered_map<string, string>& database) {
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

	BoxManager(TA<label_type>::Manager& taMan, LabMan& labMan) : taMan(taMan), labMan(labMan) {}

	void loadTemplates(const boost::unordered_map<string, string>& database) {
		for (boost::unordered_map<string, string>::const_iterator i = database.begin(); i != database.end(); ++i)
			this->loadBox(i->first, database);
	}

	LabMan& getLabMan() const {
		return this->labMan;
	}

};
*/

#endif
