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

#ifndef OLRT_H
#define OLRT_H

#include <cassert>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "smartset.hh"
#include "relation.hh"
#include "lts.hh"

class Counter {

//	int _labels;
//	int _states;
	std::vector<std::vector<size_t> > _data;
	const std::vector<std::vector<size_t> >* _key;
	const std::vector<size_t>* _range;
	
private:  // methods

	Counter& operator=(const Counter&);

public:

	Counter(size_t labels, const std::vector<std::vector<size_t> >& key, const std::vector<size_t>& range)
		: _data(labels), _key(&key), _range(&range) {}

	Counter(const Counter& counter)
		: _data(counter._data.size()), _key(counter._key), _range(counter._range) {}
/*
	void setKey(const std::vector<std::vector<int> >& key, const std::vector<int>& range) {
		this->_key = &key;
		this->_range = &range;
	}
*/
	bool isZero(size_t label, size_t state) const {
		return (this->_data[label].size() == 0) || (this->_data[label][state] == 0);
	}
	
	size_t incr(size_t label, size_t state) {
		if (this->_data[label].size() == 0)
			this->_data[label].resize((*this->_range)[label]);
		return ++this->_data[label][(*this->_key)[label][state]];
	} 
	
	size_t decr(size_t label, size_t state) {
		if ((*this->_key)[label][state] >= this->_data[label].size()) {
			for (size_t a = 0; a < this->_key->size(); ++a) {
				for (size_t i = 0; i < (*this->_key)[a].size(); ++i)
					std::cout << (*this->_key)[a][i] << " ";
				std::cout << "\n";
			}
			this->dump();
			throw std::runtime_error("Counter::decr() : location not allocated");
		}
		return --this->_data[label][(*this->_key)[label][state]];
	}

	const std::vector<size_t>& getRow(size_t label) const {
		return this->_data[label]; 
	}
	
	void copyRow(size_t label, const Counter& c) {
		this->_data[label] = c.getRow(label);
	}

	void dump() const {
		for (std::vector<std::vector<size_t> >::const_iterator i = this->_data.begin(); i != this->_data.end(); ++i) {
			if (i->size() > 0) {
				std::cout << (i - this->_data.begin()) << ": ";
				for (std::vector<size_t>::const_iterator j = i->begin(); j != i->end(); ++j)
					std::cout << *j;
				std::cout << std::endl;
			}
		}
	}

};

class OLRTBlock;

class StateListElem {
	
	size_t _state;
	OLRTBlock* _block;
	StateListElem* _next;
	StateListElem* _prev;

protected:

	void insert(StateListElem*& dst) {
		if (!dst) {
			dst = this;
			this->_next = this;
			this->_prev = this;
		} else {
			this->_prev = dst->_prev;
			this->_next = dst;
			this->_prev->_next = this;
			this->_next->_prev = this;
		}	
	}

private:  // methods

	StateListElem(const StateListElem&);
	StateListElem& operator=(const StateListElem&);

public:

	StateListElem(size_t state, OLRTBlock* block)
		: _state(state), _block(block), _next(this), _prev(this) {}

	StateListElem(size_t state, OLRTBlock* block, StateListElem*& dst)
		: _state(state), _block(block), _next{}, _prev{} {
		this->insert(dst);
	}
	
	~StateListElem() {
		if (this != this->_next) {
			this->_next->_prev = this->_prev;
			this->_prev->_next = this->_next;
		}
	}
	
	void moveToList(StateListElem*& src, StateListElem*& dst) {
		assert(src);
		if (this == this->_next)
			src = nullptr;
		else {
			src = this->_next;
			this->_next->_prev = this->_prev;
			this->_prev->_next = this->_next;
		}
		this->insert(dst);
	}
	
	size_t state() const {
		return this->_state;
	}
	
	OLRTBlock* block() const {
/*		if (!this->_block)
			throw std::runtime_error("StateListElem::block() : block empty");*/
		return this->_block;
	}
	
	void block(OLRTBlock* b) {
		this->_block = b;
	}
	
	StateListElem* next() const {
		return this->_next;
	}
	
};

class OLRTBlock {

	size_t _index;
	StateListElem* _states;
	std::vector<std::vector<size_t>*> _remove;
	Counter _counter;
	OLRTBlock* _intersection;
	SmartSet _inset;
	StateListElem* _tmp;
	
private:  // methods

	OLRTBlock(const OLRTBlock&);
	OLRTBlock& operator=(const OLRTBlock&);

public:

	OLRTBlock(size_t index, const LTS& lts, const std::vector<std::vector<size_t> >& key, const std::vector<size_t>& range)
		: _index(index), _states(nullptr), _remove(lts.labels()), _counter(lts.labels(), key, range), _intersection(nullptr), _inset(lts.labels()), _tmp(nullptr) {
		for (size_t i = 0; i < lts.states(); ++i) {
			new StateListElem(i, this, this->_states);
			for (SmartSet::iterator j = lts.lPre()[i].begin(); j != lts.lPre()[i].end(); ++j)
				this->_inset.add(*j);
		}
	}
	
	OLRTBlock(size_t index, OLRTBlock* parent, const LTS& lts)
		: _index(index), _states(parent->_tmp), _remove(lts.labels(), nullptr), _counter(parent->_counter), _intersection{},_inset(lts.labels()), _tmp(nullptr) {
		parent->_tmp = nullptr;
		parent->_intersection = this;
		StateListElem* elem = this->_states;
		do {
			for (SmartSet::iterator i = lts.lPre()[elem->state()].begin(); i != lts.lPre()[elem->state()].end(); ++i) {
				parent->_inset.remove(*i);
				this->_inset.add(*i);
			}
			elem->block(this);
			elem = elem->next();
		} while (elem != this->_states);
	}
	
	~OLRTBlock() {
		while (this->_states != this->_states->next())
			delete this->_states->next();
		delete this->_states;
	}

	StateListElem* states() {
		return this->_states;
	}
	
	void storeStates(std::vector<StateListElem*>& v) const {
		v.clear();
		StateListElem* elem = this->_states;
		do {
			v.push_back(elem);
			elem = elem->next();
		} while (elem != this->_states);
	}
	
	void moveToTmp(StateListElem* elem) {
		elem->moveToList(this->_states, this->_tmp);	
	}
	
	bool checkEmpty() {
		if (this->_states != nullptr)
			return false;
		this->_states = this->_tmp;
		this->_tmp = nullptr;
		return true;
	}
	
	Counter& counter() {
		return this->_counter;
	}
	
	OLRTBlock* intersection() const {
		return this->_intersection;
	}

	void intersection(OLRTBlock* b) {
		this->_intersection = b;
	}
	
	SmartSet& inset() {
		return this->_inset;
	}
	
	std::vector<std::vector<size_t>*>& remove() {
		return this->_remove;
	}
	
	size_t index() const {
		return this->_index;
	}
	
	StateListElem* tmp() {
		return this->_tmp;
	}
	
	void dump() const {
		std::cout <<  "block: " << this->_index << ", states: ";
		const StateListElem* elem = this->_states;
		do {
			std::cout << elem->state() << " ";
			elem = elem->next();
		} while (elem != this->_states);
		std::cout << std::endl;
/*		std::cout << "remove: " << std::endl;
		for (std::vector<std::vector<int>* >::const_iterator i = this->_remove.begin(); i != this->_remove.end(); ++i) {
			if (*i) {
				std::cout << (i - this->_remove.begin()) << ": ";
				for (std::vector<int>::const_iterator j = (*i)->begin(); j != (*i)->end(); ++j)
					std::cout << *j << " ";
				std::cout << std::endl;
			}
		}*/
		this->_counter.dump();
/*		this->_inset.dump();*/
	}
	
};

class OLRTAlgorithm {

	const LTS* _lts;
	std::vector<OLRTBlock*> _partition;
	Relation _relation;
	std::vector<StateListElem*> _index;
	std::vector<std::pair<OLRTBlock*, size_t> > _queue;
	std::vector<bool> _tmp;
	std::vector<SmartSet> _delta;
	std::vector<SmartSet> _delta1;
	std::vector<std::vector<size_t> > _key;
	std::vector<size_t> _range;
	
	std::vector<std::vector<size_t>*> _removeCache;
	
private:  // methods

	OLRTAlgorithm(const OLRTAlgorithm&);
	OLRTAlgorithm& operator=(const OLRTAlgorithm&);

	std::vector<size_t>* rcAlloc() {
		if (this->_removeCache.empty())
			return new std::vector<size_t>;
		std::vector<size_t>* v = this->_removeCache.back();
		this->_removeCache.pop_back();
		return v->clear(), v;
	}
	
	void rcFree(std::vector<size_t>* v) {
		this->_removeCache.push_back(v);
//		delete v;
	}

	void rcCollect() {
		for (std::vector<std::vector<size_t>*>::iterator i = this->_removeCache.begin(); i != this->_removeCache.end(); ++i)
			delete *i;
	}

protected:

	void fastSplit(const std::vector<size_t>& remove) {
		std::vector<OLRTBlock*> splitList;
		for (std::vector<size_t>::const_iterator i = remove.begin(); i != remove.end(); ++i) {
			StateListElem* elem = this->_index[*i];
			elem->block()->moveToTmp(elem);
			elem->block()->checkEmpty();
		}
		for (std::vector<size_t>::const_iterator i = remove.begin(); i != remove.end(); ++i) {
			StateListElem* elem = this->_index[*i];
			if (elem->block()->tmp() != nullptr) {
				splitList.push_back(elem->block());
				OLRTBlock* block = new OLRTBlock(this->_relation.newEntry(), elem->block(), *this->_lts);
				this->_partition.push_back(block);
			}
		}
		for (std::vector<OLRTBlock*>::reverse_iterator i = splitList.rbegin(); i != splitList.rend(); ++i) {
			OLRTBlock* bint = (*i)->intersection();
			(*i)->intersection(nullptr);
//			for (std::vector<OLRTBlock*>::iterator j = this->_partition.begin(); j != this->_partition.end(); ++j) {
			for (std::vector<OLRTBlock*>::reverse_iterator j = this->_partition.rbegin(); j != this->_partition.rend(); ++j) {
				this->_relation.data()[(*j)->index()][bint->index()] = this->_relation.data()[(*j)->index()][(*i)->index()];
				this->_relation.data()[bint->index()][(*j)->index()] = this->_relation.data()[(*i)->index()][(*j)->index()];
			}
		}
	}

	void split(const std::vector<size_t>& remove, std::vector<OLRTBlock*>& removeList) {
		removeList.clear();
		std::vector<OLRTBlock*> splitList;
//		for (std::vector<int>::const_iterator i = remove.begin(); i != remove.end(); ++i) {
		for (std::vector<size_t>::const_reverse_iterator i = remove.rbegin(); i != remove.rend(); ++i) {
			StateListElem* elem = this->_index[*i];
			elem->block()->moveToTmp(elem);
			if (elem->block()->checkEmpty())
				removeList.push_back(elem->block());
		}
//		for (std::vector<int>::const_iterator i = remove.begin(); i != remove.end(); ++i) {
		for (std::vector<size_t>::const_reverse_iterator i = remove.rbegin(); i != remove.rend(); ++i) {
			StateListElem* elem = this->_index[*i];
			if (elem->block()->tmp() != nullptr) {
				splitList.push_back(elem->block());
				OLRTBlock* block = new OLRTBlock(this->_relation.newEntry(), elem->block(), *this->_lts);
				this->_partition.push_back(block);
				removeList.push_back(block);
			}
		}
//		for (std::vector<OLRTBlock*>::iterator i = splitList.begin(); i != splitList.end(); ++i) {
		for (std::vector<OLRTBlock*>::reverse_iterator i = splitList.rbegin(); i != splitList.rend(); ++i) {
			OLRTBlock* bint = (*i)->intersection();
			(*i)->intersection(nullptr);
//			for (std::vector<OLRTBlock*>::iterator j = this->_partition.begin(); j != this->_partition.end(); ++j) {
			for (std::vector<OLRTBlock*>::reverse_iterator j = this->_partition.rbegin(); j != this->_partition.rend(); ++j) {
				this->_relation.data()[(*j)->index()][bint->index()] = this->_relation.data()[(*j)->index()][(*i)->index()];
				this->_relation.data()[bint->index()][(*j)->index()] = this->_relation.data()[(*i)->index()][(*j)->index()];
			}
			for (SmartSet::iterator j = bint->inset().begin(); j != bint->inset().end(); ++j) {
				bint->counter().copyRow(*j, (*i)->counter());
				if ((*i)->remove()[*j]) {
					bint->remove()[*j] = this->rcAlloc();
					*bint->remove()[*j] = *(*i)->remove()[*j];
					this->_queue.push_back(std::pair<OLRTBlock*, size_t>(bint, *j));
				}
			}
		}
	}

	void processRemove(OLRTBlock* block, size_t label) {
		/*block->dump();*/
		std::vector<size_t>* remove = block->remove()[label];
		block->remove()[label] = nullptr;
/*		std::cout << block->index() << ":" << label << ": ";
		for (size_t i = 0; i < remove->size(); ++i)
			std::cout << (*remove)[i] << " ";
		std::cout << std::endl;*/
		std::vector<StateListElem*> prev;
		block->storeStates(prev);
		std::vector<OLRTBlock*> removeList;
		this->split(*remove, removeList);
		std::fill(this->_tmp.begin(), this->_tmp.end(), true);
		for (std::vector<StateListElem*>::iterator i = prev.begin(); i != prev.end(); ++i) {
			for (std::vector<size_t>::const_iterator j = this->_lts->dataPre()[label][(*i)->state()].begin(); j != this->_lts->dataPre()[label][(*i)->state()].end(); ++j) {
				StateListElem* elem = this->_index[*j];
				OLRTBlock* block2 = elem->block();
				if (this->_tmp[block2->index()]) {
					this->_tmp[block2->index()] = false;
					for (std::vector<OLRTBlock*>::iterator k = removeList.begin(); k != removeList.end(); ++k) {
						assert(block2->index() != (*k)->index());
						if (this->_relation.data()[block2->index()][(*k)->index()]) {
							this->_relation.data()[block2->index()][(*k)->index()] = false;
							for (SmartSet::iterator a = (*k)->inset().begin(); a != (*k)->inset().end(); ++a) {
								if (block2->inset().contains(*a)) {
									StateListElem* elem2 = (*k)->states();
									do {
										for (std::vector<size_t>::const_iterator l = this->_lts->dataPre()[*a][elem2->state()].begin(); l != this->_lts->dataPre()[*a][elem2->state()].end(); ++l) {
											if (!block2->counter().decr(*a, *l)) {
												if (!block2->remove()[*a]) {
													block2->remove()[*a] = this->rcAlloc();
													this->_queue.push_back(std::pair<OLRTBlock*, size_t>(block2, *a));
												}
												block2->remove()[*a]->push_back(*l);
											}
										}
										elem2 = elem2->next();
									} while (elem2 != (*k)->states());
								}
							}
						}
					}
				}
			}
		}
		this->rcFree(remove);
	}

public:

	OLRTAlgorithm(const LTS& lts)
		: _lts(&lts), _partition(), _relation(), _index{}, _queue{}, _tmp(lts.states()), _delta{}, _delta1{}, _key{}, _range{}, _removeCache() {
		OLRTBlock* block = new OLRTBlock(this->_relation.newEntry(), lts, this->_key, this->_range);
		block->storeStates(this->_index);
		this->_partition.push_back(block);
	}

	~OLRTAlgorithm() {
		for (std::vector<OLRTBlock*>::iterator i = this->_partition.begin(); i != this->_partition.end(); ++i)
			delete *i;
		this->rcCollect();
	}
	
	void reset(const LTS& lts) {
		for (std::vector<OLRTBlock*>::iterator i = this->_partition.begin(); i != this->_partition.end(); ++i)
			delete *i;
		this->_partition.clear();
		this->_relation.reset();
		this->_lts = &lts;
		this->_tmp.resize(lts.states());
		OLRTBlock* block = new OLRTBlock(this->_relation.newEntry(), lts, this->_key, this->_range);
		block->storeStates(this->_index);
		this->_partition.push_back(block);
	}

	void init() {
		this->_lts->buildDelta(this->_delta, this->_delta1);
		this->_key.resize(this->_lts->labels());
		this->_range.resize(this->_lts->labels());
		for (size_t a = 0; a < this->_lts->labels(); ++a) {
			this->_key[a].resize(this->_lts->states());
			this->_range[a] = this->_delta1[a].size();
			size_t x = 0;
			for (SmartSet::iterator i = this->_delta1[a].begin(); i != this->_delta1[a].end(); ++i)
				this->_key[a][*i] = x++;
		}
		std::vector<size_t> tmp2;
		for (size_t a = 0; a < this->_lts->labels(); ++a) {
//			this->dump();
			this->_delta1[a].buildVector(tmp2);
			this->fastSplit(tmp2);
		}
		std::vector<std::vector<bool> > tmp[2];
		tmp[0].resize(this->_lts->labels(), std::vector<bool>(this->_partition.size(), true));
		tmp[1].resize(this->_lts->labels(), std::vector<bool>(this->_partition.size(), true));
		for (size_t a = 0; a < this->_lts->labels(); ++a) {
			for (std::vector<OLRTBlock*>::iterator i = this->_partition.begin(); i != this->_partition.end(); ++i) {
				StateListElem* elem = (*i)->states();
				do {
					tmp[(this->_delta1[a].contains(elem->state()))?(1):(0)][a][(*i)->index()] = false;
					elem = elem->next();
				} while (elem != (*i)->states());
			}
		}
		for (size_t a = 0; a < this->_lts->labels(); ++a) {
			for (std::vector<OLRTBlock*>::iterator i = this->_partition.begin(); i != this->_partition.end(); ++i) {
				if (tmp[0][a][(*i)->index()]) {
					for (std::vector<OLRTBlock*>::iterator j = this->_partition.begin(); j != this->_partition.end(); ++j) {
						if (tmp[1][a][(*j)->index()])
							this->_relation.data()[(*i)->index()][(*j)->index()] = false;
					}
				}
			}			
		}		
		std::vector<std::vector<size_t> > post;
//		for (std::vector<OLRTBlock*>::iterator i = this->_partition.begin(); i != this->_partition.end(); ++i) {
		for (std::vector<OLRTBlock*>::reverse_iterator i = this->_partition.rbegin(); i != this->_partition.rend(); ++i) {
			for (SmartSet::iterator j = (*i)->inset().begin(); j != (*i)->inset().end(); ++j) {
				this->_lts->buildPost(*j, post);
				for (SmartSet::iterator k = this->_delta1[*j].begin(); k != this->_delta1[*j].end(); ++k) {
					for (std::vector<size_t>::iterator l = post[*k].begin(); l != post[*k].end(); ++l) {
						if (this->_relation.data()[(*i)->index()][this->_index[*l]->block()->index()])
							(*i)->counter().incr(*j, *k);
					}
				}
				for (size_t k = 0; k < this->_lts->states(); ++k)
					this->_tmp[k] = this->_delta1[*j].contains(k);
				for (std::vector<OLRTBlock*>::iterator k = this->_partition.begin(); k != this->_partition.end(); ++k) {
					if (this->_relation.data()[(*i)->index()][(*k)->index()]) {
						StateListElem* elem = (*k)->states();
						do {
							for (std::vector<size_t>::const_iterator l = this->_lts->dataPre()[*j][elem->state()].begin(); l != this->_lts->dataPre()[*j][elem->state()].end(); ++l)
								this->_tmp[*l] = false;
							elem = elem->next();
						} while (elem != (*k)->states());
					}
				}
				std::vector<size_t>* r = this->rcAlloc();
				for (size_t k = 0; k < this->_lts->states(); ++k) {
					if (this->_tmp[k])
						r->push_back(k);
				}
				if (!r->empty()) {
				    (*i)->remove()[*j] = r;
					this->_queue.push_back(std::pair<OLRTBlock*, size_t>(*i, *j));
				} else
					this->rcFree(r);					
			}
		}
	}

	void fakeSplit(const std::vector<size_t>& remove) {
		for (std::vector<size_t>::const_iterator i = remove.begin(); i != remove.end(); ++i) {
			StateListElem* elem = this->_index[*i];
			elem->block()->moveToTmp(elem);
			elem->block()->checkEmpty();
		}
		for (std::vector<size_t>::const_iterator i = remove.begin(); i != remove.end(); ++i) {
			StateListElem* elem = this->_index[*i];
			if (elem->block()->tmp() != nullptr) {
				OLRTBlock* block = new OLRTBlock(this->_relation.newEntry(), elem->block(), *this->_lts);
				this->_partition.push_back(block);
			}
		}
	}

	void run() {
	    while (!this->_queue.empty()) {
/*			this->dump();
			for (size_t i = 0; i < this->_queue.size(); ++i) {
				std::cout << this->_queue[i].first->index() << ":" << this->_queue[i].second << "[ ";
				for (size_t j = 0; j < this->_queue[i].first->remove()[this->_queue[i].second]->size(); ++j) {
					std::cout << (*this->_queue[i].first->remove()[this->_queue[i].second])[j] << " ";
				}
				std::cout << "] ";
			}
			std::cout << std::endl;*/
			std::pair<OLRTBlock*, size_t> tmp(this->_queue.back());
			this->_queue.pop_back();
			assert(tmp.first->remove()[tmp.second]);
			this->processRemove(tmp.first, tmp.second);
		}
	}
	
	Relation& getRelation() {
		return this->_relation;
	}
	
	void buildRel(size_t size, std::vector<std::vector<bool> >& rel) const {
		rel.clear();
		rel.resize(size, std::vector<bool>(size));
		for (size_t i = 0; i < size; ++i) {
			size_t ii = this->_index[i]->block()->index();
			for (size_t j = 0; j < size; ++j)
				rel[i][j] = this->_relation.data()[ii][this->_index[j]->block()->index()];
		}
	}
	
	size_t buildIndex(std::vector<size_t>& index, size_t size) const {
		size_t blockCount = 0;
		std::vector<bool> tmp(this->_partition.size(), false);
		index.resize(size);
		for (size_t i = 0; i < size; ++i) {
			index[i] = this->_index[i]->block()->index();
			if (!tmp[index[i]]) {
				tmp[index[i]] = true;
				++blockCount;
			}
		}
		return blockCount;
	}

	void dump() const {
		for (size_t i = 0; i < this->_partition.size(); ++i)
	  		this->_partition[i]->dump();
		std::cout << "relation:" << std::endl;
/*		self#dump_part _lts#nstates*/
		this->_relation.dump();
	}

};

#endif
