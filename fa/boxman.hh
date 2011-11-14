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
#include <unordered_map>
#include <string>
#include <sstream>
#include <fstream>

#include <unordered_map>

#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>

#include "treeaut.hh"
#include "tatimint.hh"
#include "label.hh"
#include "types.hh"
#include "box.hh"
#include "utils.hh"

class BoxMan {

//	TA<label_type>::Manager& taMan;
//	TA<label_type>::Backend& backend;

//	TA<std::string>::Backend sBackend;

	boost::unordered_map<Data, NodeLabel*> dataStore;
	std::vector<const Data*> dataIndex;
	boost::unordered_map<std::vector<const AbstractBox*>, NodeLabel*> nodeStore;
	boost::unordered_set<std::pair<const TypeBox*, std::vector<size_t> > > tagStore;
	boost::unordered_map<std::pair<size_t, std::vector<Data> >, NodeLabel*> vDataStore;

	boost::unordered_map<SelData, const SelBox*> selIndex;
	boost::unordered_map<std::string, const TypeBox*> typeIndex;
//	boost::unordered_map<std::string, const Box*> boxIndex;

//	typedef std::pair<size_t, std::vector<FA::RootSignature>> BoxSignature;

	size_t boxCount;

//	std::unordered_map<BoxSignature, std::vector<BoxPtr>, boost::hash<BoxSignature>> boxMap;

	std::unordered_set<Box, boost::hash<Box>> boxes;
	std::unordered_set<Box, boost::hash<Box>> boxCache;

	const std::pair<const Data, NodeLabel*>& insertData(const Data& data) {
		std::pair<boost::unordered_map<Data, NodeLabel*>::iterator, bool> p
			= this->dataStore.insert(std::make_pair(data, (NodeLabel*)NULL));
		if (p.second) {
			p.first->second = new NodeLabel(&p.first->first, this->dataIndex.size());
			this->dataIndex.push_back(&p.first->first);
		}
		return *p.first;
	}

	std::string getBoxName() const {

		std::stringstream sstr;

		sstr << "box" << this->boxCount;

		return sstr.str();

	}

public:

	label_type lookupLabel(const Data& data) {
		return this->insertData(data).second;
	}

	label_type lookupLabel(size_t arity, const std::vector<Data>& x) {
		std::pair<boost::unordered_map<std::pair<size_t, std::vector<Data> >, NodeLabel*>::iterator, bool> p
			= this->vDataStore.insert(std::make_pair(std::make_pair(arity, x), (NodeLabel*)NULL));
		if (p.second)
			p.first->second = new NodeLabel(&p.first->first.second);
		return p.first->second;
	}

	struct EvaluateBoxF {
		NodeLabel& label;
		std::vector<size_t>& tag;
		EvaluateBoxF(NodeLabel& label, std::vector<size_t>& tag) : label(label), tag(tag) {}
		bool operator()(const AbstractBox* aBox, size_t index, size_t offset) {
			switch (aBox->getType()) {
				case box_type_e::bSel: {
					const SelBox* sBox = (const SelBox*)aBox;
					this->label.addMapItem(sBox->getData().offset, aBox, index, offset);
					this->tag.push_back(sBox->getData().offset);
					break;
				}
				case box_type_e::bBox: {
					const Box* bBox = (const Box*)aBox;
					for (std::set<size_t>::const_iterator i= bBox->outputCoverage().begin(); i != bBox->outputCoverage().end(); ++i) {
						this->label.addMapItem(*i, aBox, index, offset);
						this->tag.push_back(*i);
					}
					break;
				}
				case box_type_e::bTypeInfo:
					this->label.addMapItem((size_t)(-1), aBox, index, (size_t)(-1));
					break;
				default:
					assert(false);
					break;
			}
			return true;
		}
	};

	label_type lookupLabel(const std::vector<const AbstractBox*>& x) {
		std::pair<boost::unordered_map<std::vector<const AbstractBox*>, NodeLabel*>::iterator, bool> p
			= this->nodeStore.insert(std::make_pair(x, (NodeLabel*)NULL));
		if (p.second) {
			NodeLabel* label = new NodeLabel(&p.first->first);
			std::vector<size_t> tag;
			label->iterate(EvaluateBoxF(*label, tag));
			std::sort(tag.begin(), tag.end());
			label->setTag(
				(void*)&*this->tagStore.insert(
					std::make_pair((const TypeBox*)label->boxLookup((size_t)(-1), NULL), tag)
				).first
			);
			p.first->second = label;
		}
		return p.first->second;
	}

	const Data& getData(const Data& data) {
		return this->insertData(data).first;
	}

	size_t getDataId(const Data& data) {
		return this->insertData(data).second->getDataId();
	}

	const Data& getData(size_t index) const {
		assert(index < this->dataIndex.size());
		return *this->dataIndex[index];
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

	static size_t translateSignature(ConnectionGraph::CutpointSignature& result,
		std::vector<std::pair<size_t, size_t>>& selectors, size_t root,
		const ConnectionGraph::CutpointSignature& signature, size_t aux,
		const std::vector<size_t>& index) {

		size_t auxSelector = (size_t)(-1);

		for (auto& cutpoint : signature) {

			assert(cutpoint.root < index.size());

			result.push_back(cutpoint);
			result.back().root = index[cutpoint.root];

			if (cutpoint.root == aux)
				auxSelector = cutpoint.forwardSelector;

			if (cutpoint.root != root) {

				selectors.push_back(
					std::make_pair(cutpoint.forwardSelector, cutpoint.backwardSelector)
				);

			}

		}

		return auxSelector;

	}

	Box* createType1Box(size_t root, const std::shared_ptr<TA<label_type>>& output,
		const ConnectionGraph::CutpointSignature& signature, std::vector<size_t>& inputMap,
		const std::vector<size_t>& index) {

		ConnectionGraph::CutpointSignature outputSignature;
		std::vector<std::pair<size_t, size_t>> selectors;

		BoxMan::translateSignature(
			outputSignature, selectors, root, signature, (size_t)(-1), index
		);

		return new Box(
			this->getBoxName(),
			output,
			outputSignature,
			inputMap,
			std::shared_ptr<TA<label_type>>(nullptr),
			0,
			ConnectionGraph::CutpointSignature(),
			selectors
		);

	}

	Box* createType2Box(size_t root, const std::shared_ptr<TA<label_type>>& output,
		const ConnectionGraph::CutpointSignature& signature, std::vector<size_t>& inputMap,
		size_t aux, const std::shared_ptr<TA<label_type>>& input,
		const ConnectionGraph::CutpointSignature& signature2, size_t inputSelector,
		std::vector<size_t>& index) {

		assert(aux < index.size());
		assert(index[aux] >= 1);

		ConnectionGraph::CutpointSignature outputSignature, inputSignature;
		std::vector<std::pair<size_t, size_t>> selectors;

		size_t auxSelector = BoxMan::translateSignature(
			outputSignature, selectors, root, signature, aux, index
		);

		size_t start = selectors.size();

		for (auto& cutpoint : signature2) {

			assert(cutpoint.root < index.size());

			if (index[cutpoint.root] == (size_t)(-1)) {

				index[cutpoint.root] = start++;

//				backwardSelectors.push_back(cutpoint.backwardSelector);
				selectors.push_back(std::make_pair(auxSelector, (size_t)(-1)));

			}

			inputSignature.push_back(cutpoint);
			inputSignature.back().root = index[cutpoint.root];

		}

		size_t inputIndex = index[aux] - 1;

		assert(inputIndex < selectors.size());

		if (selectors[inputIndex].second > inputSelector)
			selectors[inputIndex].second = inputSelector;

		return new Box(
			this->getBoxName(),
			output,
			outputSignature,
			inputMap,
			input,
			inputIndex,
			inputSignature,
			selectors
		);

	}

	const Box* lookupBox(const Box& box) {

		auto p = this->boxes.insert(box);

		if (!p.second)
			return (const Box*)&*p.first;

		auto p2 = this->boxCache.insert(box);

		if (p2.second) {

			this->boxes.erase(p.first);

			return nullptr;

		}

		this->boxCache.erase(p2.first);

		const_cast<Box*>(&*p.first)->initialize();

		CL_CDEBUG(1, "new box identified (" << *(AbstractBox*)&*p.first << "): " << std::endl << *p.first);

		++this->boxCount;

		return (const Box*)&*p.first;

	}

	void clearBoxCache() {

		this->boxCache.clear();

	}

protected:
/*
	void translateLabel(TA<label_type>& dst, const TT<std::string>& t, bool& composed, const std::unordered_map<std::string, std::string>& database) {
		std::vector<std::string> strs;
		boost::split(strs, t.label(), boost::is_from_range(',', ','));
		std::vector<const AbstractBox*> label;
		for (vector<std::string>::iterator j = strs.begin(); j != strs.end(); ++j) {
			if (*j == "")
				continue;
			std::vector<std::string> args;
			boost::split(args, *j, boost::is_from_range('_', '_'));
			if (args[0] == "data") {
				if (strs.size() != 1)
					throw std::runtime_error("Only one item expected when parsing data label!");
				dst.addTransition(std::vector<size_t>(), this->lookupLabel(Data::fromArgs(args)), t.rhs());
				return;
			}
			const AbstractBox* aBox = this->loadBox(*j, database);
			if (aBox->isType(box_type_e::bBox))
				composed = true;
			label.push_back(aBox);
		}
		std::vector<size_t> lhs(t.lhs());
		FA::reorderBoxes(label, lhs);
		dst.addTransition(lhs, this->lookupLabel(label), t.rhs());
	}

	TA<label_type>& translateRoot(TA<label_type>& dst, bool& composed, const TA<std::string>& src, const std::unordered_map<std::string, std::string>& database) {
		dst.clear();
		for (TA<std::string>::iterator i = src.begin(); i != src.end(); ++i)
			this->translateLabel(dst, *i, composed, database);
		dst.addFinalState(src.getFinalState());
		return dst;
	}
*/
public:
/*
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
			if (i->label()->isData())
				leafIndex.insert(std::make_pair(i->rhs(), _MSB_ADD(i->label()->getDataId())));
		}
		TA<label_type>::rename(dst, src, RenameSelectedF(leafIndex));
		return dst;
	}

	const AbstractBox* loadBox(const std::string& name, const std::unordered_map<std::string, std::string>& database) {

		if (boost::starts_with(name, "type_"))
			return this->getTypeInfo(name.substr(5));

		std::vector<std::string> args;
		boost::split(args, name, boost::is_from_range('_', '_'));

		if (args[0] == "sel")
			return this->getSelector(SelData::fromArgs(args));

		std::pair<boost::unordered_map<std::string, const Box*>::iterator, bool> p =
			this->boxIndex.insert(std::make_pair(name, (const Box*)NULL));
		if (!p.second)
			return p.first->second;

		auto j = database.find(name);
		if (j == database.end())
			throw std::runtime_error("Source for box '" + name + "' not found!");

		Box* box = new Box(this->backend, name);

		p.first->second = box;

		std::fstream input(j->second.c_str());

		if (!input.good())
			throw std::runtime_error("Unable to open " + j->second);

		TAReader reader(input, j->second);

		TA<std::string> sta(this->sBackend);

		std::string autName;

		reader.readFirst(sta, autName);

		TA<label_type> tmp(this->backend);

		bool composed = false;

//		box.variables.push_back(Data::createRef(box.roots.size(), 0));
		this->translateRoot(tmp, composed, sta, database);
		box->appendRoot(&this->adjustLeaves(*(new TA<label_type>(this->backend)), tmp));

		while (reader.readNext(sta, autName)) {
			tmp.clear();
			this->translateRoot(tmp, composed, sta, database);
			box->appendRoot(&this->adjustLeaves(*(new TA<label_type>(this->backend)), tmp));
//			if (memcmp(autName.c_str(), "in", 2) == 0)
//				box.variables.push_back(Data::createRef(box.roots.size(), 0));
		}

		box->composed = composed;
		box->initialize(box->roots.size() - 1);

		return box;

	}
*/
public:

	BoxMan(/*TA<label_type>::Backend& backend*/)
		: /*backend(backend),*/ boxCount() {}

	~BoxMan() {
		this->clear();
	}

	void clear() {

		utils::eraseMap(this->dataStore);
		this->dataIndex.clear();
		utils::eraseMap(this->nodeStore);
		this->tagStore.clear();
		utils::eraseMap(this->vDataStore);
		utils::eraseMap(this->selIndex);
		utils::eraseMap(this->typeIndex);
//		utils::eraseMap(this->boxIndex);

	}

	const std::unordered_set<Box, boost::hash<Box>>& getBoxes() const {
		return this->boxes;
	}

/*
	void loadTemplates(const boost::unordered_map<string, string>& database) {
		for (boost::unordered_map<string, string>::const_iterator i = database.begin(); i != database.end(); ++i)
			this->loadBox(i->first, database);
	}
*//*
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

*/

public:

};

#endif
