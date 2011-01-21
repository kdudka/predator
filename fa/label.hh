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

#ifndef LABEL_H
#define LABEL_H

#include <vector>
#include <stdexcept>

#include <boost/unordered_map.hpp>

#include "types.hh"
#include "abstractbox.hh"

struct NodeLabel {

	typedef enum { n_unknown, n_node, n_data, n_vData } node_type;

	node_type type;

	struct NodeItem{
		const AbstractBox* aBox;
		size_t index;
		size_t offset;
		NodeItem(const AbstractBox* aBox, size_t index, size_t offset)
			: aBox(aBox), index(index), offset(offset) {}
	};

	union {
		struct {
			const Data* data;
			size_t id;
		} data;
		struct {
			const std::vector<const AbstractBox*>* v;
			boost::unordered_map<size_t, NodeItem>* m;
			void* tag;
		} node;
		const std::vector<Data>* vData;
	};

	NodeLabel() : type(node_type::n_unknown) {}
	NodeLabel(const Data* data, size_t id) : type(node_type::n_data) {
		this->data.data = data;
		this->data.id = id;
	}
	NodeLabel(const std::vector<const AbstractBox*>* v)
		: type(node_type::n_node) {
		this->node.v = v;
		this->node.m = new boost::unordered_map<size_t, NodeItem>();
	}
	NodeLabel(const std::vector<Data>* vData) : type(node_type::n_vData), vData(vData) {}

	~NodeLabel() {
		if (this->type == node_type::n_node)
			delete this->node.m;
	}

	void addMapItem(size_t key, const AbstractBox* aBox, size_t index, size_t offset) {
		bool b = this->node.m->insert(std::make_pair(key, NodeItem(aBox, index, offset))).second;
		assert(b);
	}

	bool isData() const {
		return this->type == node_type::n_data;
	}

	bool isData(const Data*& data) const {
		if (this->type != node_type::n_data)
			return false;
		data = this->data.data;
		return true;
	}

	const Data& getData() const {
		assert(this->type == node_type::n_data);
		return *this->data.data;
	}

	size_t getDataId() const {
		assert(this->type == node_type::n_data);
		return this->data.id;
	}

	const AbstractBox* boxLookup(size_t offset, const AbstractBox* def) const {
		assert(this->type == node_type::n_node);
		boost::unordered_map<size_t, NodeItem>::const_iterator i = this->node.m->find(offset);
		if (i == this->node.m->end())
			return def;
		return i->second.aBox;
	}

	const NodeItem& boxLookup(size_t offset) const {
		assert(this->type == node_type::n_node);
		boost::unordered_map<size_t, NodeItem>::const_iterator i = this->node.m->find(offset);
//		assert(i != this->node.m->end());
		if (i == this->node.m->end())
			throw std::runtime_error("NodeLabel::boxLookup(): required box not found!");
		return i->second;
	}

	const std::vector<Data>& getVData() const {
		assert(this->type == node_type::n_vData);
		return *this->vData;
	}

	bool isNode() const {
		return this->type == node_type::n_node;
	}

	const std::vector<const AbstractBox*>& getNode() const {
		assert(this->type == node_type::n_node);
		return *this->node.v;
	}

	void* getTag() const {
		assert(this->type == node_type::n_node);
		return this->node.tag;
	}

	void setTag(void* tag) {
		assert(this->type == node_type::n_node);
		this->node.tag = tag;
	}

	template <class F>
	void iterate(F f) const {
		assert(this->type == node_type::n_node);
		for (size_t i = 0, offset = 0; i < this->node.v->size(); ++i) {
			const AbstractBox* aBox = (*this->node.v)[i];
			if (!f(aBox, i, offset))
				break;
			offset += aBox->getArity();
		}
	}

	bool operator<(const NodeLabel& rhs) const {
		return this->data.data < rhs.data.data;
	}

	bool operator==(const NodeLabel& rhs) const { return this->data.data == rhs.data.data; }

	bool operator!=(const NodeLabel& rhs) const { return this->data.data != rhs.data.data; }

	friend size_t hash_value(const NodeLabel& label) {
		size_t h = boost::hash_value(label.type);
		boost::hash_combine(h, label.data.data);
		return h;
	}

	friend std::ostream& operator<<(std::ostream& os, const NodeLabel& label) {
		os << '<';
		switch (label.type) {
			case node_type::n_unknown:
				os << "unknown";
				break;			
			case node_type::n_data:
				assert(label.data.data);
				os << *label.data.data;
				break;
			case node_type::n_vData: {
				assert(label.vData);
				assert(label.vData->size());
				std::vector<Data>::const_iterator i = label.vData->begin();
				os << *i;
				for (++i; i != label.vData->end(); ++i)
					os << ',' << *i;
				break;
			}
			case node_type::n_node: {
				assert(label.node.v);
				assert(label.node.v->size());
				std::vector<const AbstractBox*>::const_iterator i = label.node.v->begin();
				os << **i;
				for (++i; i != label.node.v->end(); ++i)
					os << ',' << **i;
				break;
			}
		}
		return os << '>';
	}

};

struct label_type {

	const NodeLabel* _obj;

	label_type() : _obj(NULL) {}
	label_type(const NodeLabel* obj) : _obj(obj) {}

	const NodeLabel& operator*() const {
		assert(this->_obj);
		return *this->_obj;
	}

	const NodeLabel* operator->() const {
		assert(this->_obj);
		return this->_obj;
	}

	bool operator<(const label_type& rhs) const {
		return this->_obj < rhs._obj;
	}

	bool operator==(const label_type& rhs) const {
		return this->_obj == rhs._obj;
	}

	bool operator!=(const label_type& rhs) const {
		return this->_obj != rhs._obj;
	}

	friend size_t hash_value(const label_type& label) {
		return boost::hash_value(label._obj);
	}

	friend std::ostream& operator<<(std::ostream& os, const label_type& label) {
		return os << *label._obj;
	}
	
};

#endif
