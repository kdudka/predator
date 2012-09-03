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

#ifndef TREE_AUT_H
#define TREE_AUT_H

// Standard library headers
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <cassert>
#include <stdexcept>

// Forester headers
#include "cache.hh"
#include "utils.hh"
#include "lts.hh"

template <class T> class TA;

template <class T>
class TTBase {

	friend class TA<T>;

public:

	typedef Cache<std::vector<size_t> > lhs_cache_type;

public:

	lhs_cache_type::value_type* _lhs;
	T _label;
	size_t _rhs;

private:  // methods

	TTBase(const TTBase&);
	TTBase& operator=(const TTBase&);

protected:

	TTBase(lhs_cache_type::value_type* lhs = nullptr, const T& label = T(), size_t rhs = 0)
		: _lhs(lhs), _label(label), _rhs(rhs) {}

public:

	const std::vector<size_t>& lhs() const {
		assert(this->_lhs);
		return this->_lhs->first;
	}

	const T& label() const { return this->_label; }

	size_t rhs() const { return this->_rhs; }

	bool operator==(const TTBase& rhs) const {
		return (this->_label == rhs._label) && (this->_lhs == rhs._lhs) && (this->_rhs == rhs._rhs);
	}

	bool operator<(const TTBase& rhs) const {
		// this is completely illegible
		return (this->_rhs < rhs._rhs) || (
			(this->_rhs == rhs._rhs) && (
				(this->_label < rhs._label) || (
					(this->_label == rhs._label) && (
						this->_lhs < rhs._lhs
					)
				)
			)
		);
/*		if (this->_rhs < rhs._rhs)
			return true;
		if (this->_rhs != rhs._rhs)
			return false;
		if (this->_label < rhs._label)
			return true;
		if (this->_label != rhs._label)
			return false;
		return this->_lhs < rhs._lhs;*/
/*		assert(this->_lhs);
		assert(rhs._lhs);
		return this->_lhs->first < rhs._lhs->first;*/

	}

	friend size_t hash_value(const TTBase& t) {
		size_t h = boost::hash_value(t._lhs);
		boost::hash_combine(h, t._label);
		boost::hash_combine(h, t._rhs);
		return h;
	}

	friend std::ostream& operator<<(std::ostream& os, const TTBase& t) {
		os << t._label << '(';
		if (t._lhs) {
			if (t._lhs->first.size() > 0) {
				os << t._lhs->first[0];
				for (size_t i = 1; i < t._lhs->first.size(); ++i)
					os << ',' << t._lhs->first[i];
			}
		}
		return os << ")->" << t._rhs;
	}

	/**
	 * @brief  Run a visitor on the instance
	 *
	 * This method is the @p accept method of the Visitor design pattern.
	 *
	 * @param[in]  visitor  The visitor of the type @p TVisitor
	 *
	 * @tparam  TVisitor  The type of the visitor
	 */
	template <class TVisitor>
	void accept(TVisitor& visitor) const
	{
		visitor(*this);
	}

	/**
	 * @brief  Virtual destructor
	 */
	virtual ~TTBase()
	{ }
};

template <class T>
class TT : public TTBase<T> {

	friend class TA<T>;

private:

	typename TTBase<T>::lhs_cache_type& lhsCache;

public:

	TT(const TT& t)
		: TTBase<T>(t._lhs, t._label, t._rhs), lhsCache(t.lhsCache) {
		this->lhsCache.addRef(this->_lhs);
	}

	TT(const std::vector<size_t>& lhs, const T& label, size_t rhs, typename TTBase<T>::lhs_cache_type& lhsCache)
		: TTBase<T>(lhsCache.lookup(lhs), label, rhs), lhsCache(lhsCache) {}

	TT(const std::vector<size_t>& lhs, const T& label, size_t rhs, const std::vector<size_t>& index, typename TTBase<T>::lhs_cache_type& lhsCache)
		: TTBase<T>(nullptr, label, index[rhs]), lhsCache(lhsCache) {
		std::vector<size_t> tmp(lhs.size());
		for (size_t i = 0; i < lhs.size(); ++i)
			tmp[i] = index[lhs[i]];
		this->_lhs = this->lhsCache.lookup(tmp);
	}

	TT(const TT& t, typename TTBase<T>::lhs_cache_type& lhsCache)
		: TTBase<T>(lhsCache.lookup(t._lhs->first), t._label, t._rhs), lhsCache(lhsCache) {}

	TT(const TT& t, const std::vector<size_t>& index, typename TTBase<T>::lhs_cache_type& lhsCache)
		: TTBase<T>(nullptr, t._label, index[t._rhs]), lhsCache(lhsCache) {
		std::vector<size_t> tmp(t._lhs->first.size());
		for (size_t i = 0; i < t._lhs->first.size(); ++i)
			tmp[i] = index[t._lhs->first[i]];
		this->_lhs = this->lhsCache.lookup(tmp);
	}

	~TT() { this->lhsCache.release(this->_lhs);	}

	bool llhsLessThan(const TT& rhs, const std::vector<std::vector<bool> >& cons, const Index<size_t>& stateIndex) const {
		if (this->_label != rhs._label)
			return false;
		for (size_t i = 0; i < this->_lhs->first.size(); ++i) {
			if (!cons[stateIndex[this->_lhs->first[i]]][stateIndex[rhs._lhs->first[i]]])
				return false;
		}
		return true;
	}
/*
	bool operator<(const TT& rhs) const {
		return (this->_label < rhs._label) || (
			(this->_label == rhs._label) && (
				(this->_lhs->first < rhs._lhs->first) || (
					(this->_lhs == rhs._lhs) && (
						this->_rhs < rhs._rhs
					)
				)
			)
		);
		return (this->_lhs->first < rhs._lhs->first) || (
			(this->_lhs->first == rhs._lhs->first) && (
				(this->_label < rhs._label) || (
					(this->_label == rhs._label) && (
						this->_rhs < rhs._rhs
					)
				)
			)
		);
	}
*/
};

template <class T>
class TA {

//	friend template <class U> class TAManager<T>;

public:

	///	the type of a tree automaton transition
	typedef TT<T> Transition;

    typedef Cache<Transition> trans_cache_type;

	// this is the place where transitions are stored
	struct Backend {

		typename TTBase<T>::lhs_cache_type lhsCache;
		trans_cache_type transCache;

		Backend() :
			lhsCache{},
			transCache{}
		{ }
	};

	Backend* backend;

	struct CmpF {
		bool operator()(typename trans_cache_type::value_type* lhs, typename trans_cache_type::value_type* rhs) const {
			return lhs->first < rhs->first;
		}
	};

	typedef std::set<typename trans_cache_type::value_type*, CmpF> trans_set_type;

	typename Transition::lhs_cache_type& lhsCache() const { return this->backend->lhsCache; }

	trans_cache_type& transCache() const { return this->backend->transCache; }

	class Iterator {
		typename trans_set_type::const_iterator _i;
	public:
		Iterator(typename trans_set_type::const_iterator i) : _i(i) {}

		Iterator& operator++() { return ++this->_i, *this; }

		Iterator operator++(int) { return Iterator(this->_i++); }

		Iterator& operator--() { return --this->_i, *this; }

		Iterator operator--(int) { return Iterator(this->_i--); }

		const Transition& operator*() const { return (*this->_i)->first; }

		const Transition* operator->() const { return &(*this->_i)->first; }

		bool operator==(const Iterator& rhs) const { return this->_i == rhs._i; }

		bool operator!=(const Iterator& rhs) const { return this->_i != rhs._i; }

	};

public:

	typedef typename std::unordered_map<size_t, std::vector<const Transition*> > td_cache_type;

	class TDIterator {

		const td_cache_type& _cache;
		std::set<size_t> _visited;
		std::vector<
			std::pair<
				typename std::vector<const Transition*>::const_iterator,
				typename std::vector<const Transition*>::const_iterator
			>
		> _stack;

		void insertLhs(const std::vector<size_t>& lhs) {
			for (std::vector<size_t>::const_reverse_iterator i = lhs.rbegin(); i != lhs.rend(); ++i) {
				if (this->_visited.insert(*i).second) {
					typename td_cache_type::const_iterator j = this->_cache.find(*i);
					if (j != this->_cache.end())
						this->_stack.push_back(make_pair(j->second.begin(), j->second.end()));
				}
			}
		}

	public:
		TDIterator(const td_cache_type& cache, const std::vector<size_t>& stack)
			: _cache(cache), _visited(), _stack{} {
			this->insertLhs(stack);
		}

		bool isValid() const { return !this->_stack.empty(); }

		bool next() {
			size_t oldSize = this->_stack.size();
			this->insertLhs((*this->_stack.back().first)->_lhs->first);
			// if something changed then we have a new "working" item on the top of the stack
			if (this->_stack.size() != oldSize)
				return true;
			++this->_stack.back().first;
			do {
				// is there something to process ?
				if (this->_stack.back().first != this->_stack.back().second)
					return true;
				// discard processed queue
				this->_stack.pop_back();
			} while (!this->_stack.empty());
			// nothing else remains
			return false;
		}

		const Transition& operator*() const { return **this->_stack.back().first; }

		const Transition* operator->() const { return *this->_stack.back().first; }

	};

	typedef typename std::unordered_map<size_t, std::vector<const Transition*> > bu_cache_type;

	typedef std::unordered_map<T, std::vector<const Transition*> > lt_cache_type;
//	typedef boost::unordered_map<size_t, lt_cache_type> slt_cache_type;

public:

	typedef Iterator iterator;
	typedef TDIterator td_iterator;

public:

//	int labels;
	size_t next_state;
	size_t maxRank;

	trans_set_type transitions;
	std::set<size_t> finalStates;

//	std::map<const std::vector<int>*, int> lhsMap;

	typename trans_cache_type::value_type* internalAdd(const Transition& t) {
		typename trans_cache_type::value_type* x = this->transCache().lookup(t);
		if (this->transitions.insert(x).second) {
			if (t._lhs->first.size() > this->maxRank)
				this->maxRank = t._lhs->first.size();
		} else this->transCache().release(x);
		return x;
	}

public:

	TA(Backend& backend) : backend(&backend), next_state(0), maxRank(0), transitions{}, finalStates{} {}

	TA(const TA<T>& ta, bool copyFinalStates = true)
		: backend(ta.backend), next_state(ta.next_state), maxRank(ta.maxRank),
		transitions(ta.transitions), finalStates{} {
		if (copyFinalStates)
			this->finalStates = ta.finalStates;
		for (typename std::set<typename trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			this->transCache().addRef(*i);
	}

	template <class F>
	TA(const TA<T>& ta, F f, bool copyFinalStates = true)
		: backend(ta.backend), next_state(ta.next_state), maxRank(ta.maxRank),
		transitions() {
		if (copyFinalStates)
			this->finalStates = ta.finalStates;
		for (typename std::set<typename trans_cache_type::value_type*>::iterator i = ta.transitions.begin(); i != ta.transitions.end(); ++i) {
			if (f(&(*i)->first))
				this->addTransition(*i);
		}
	}

	~TA() { this->clear(); }

	typename TA<T>::Iterator begin() const { return typename TA<T>::Iterator(this->transitions.begin()); }
	typename TA<T>::Iterator end() const { return typename TA<T>::Iterator(this->transitions.end()); }

	typename trans_set_type::const_iterator _lookup(size_t rhs) const {
		char buffer[sizeof(std::pair<const Transition, size_t>)];
		std::pair<const Transition, size_t>* tPtr = reinterpret_cast<std::pair<const Transition, size_t>*>(buffer);
		new (reinterpret_cast<TTBase<T>*>(const_cast<Transition*>(&tPtr->first))) TTBase<T>(nullptr, T(), rhs);
		typename trans_set_type::const_iterator i = this->transitions.lower_bound(tPtr);
		(reinterpret_cast<TTBase<T>*>(const_cast<Transition*>(&tPtr->first)))->~TTBase();
		return i;
	}

	typename TA<T>::Iterator begin(size_t rhs) const {
		return Iterator(this->_lookup(rhs));
	}

	typename TA<T>::Iterator end(size_t rhs) const {
		typename TA<T>::Iterator i = this->begin(rhs);
		for (; i != this->end() && i->rhs() == rhs; ++i);
		return Iterator(i);
	}

	typename TA<T>::Iterator end(size_t rhs, typename TA<T>::Iterator i) const {
		for (; i != this->end() && i->rhs() == rhs; ++i);
		return Iterator(i);
	}

	typename TA<T>::Iterator accBegin() const {
		return this->begin(this->getFinalState());
	}

	typename TA<T>::Iterator accEnd() const {
		typename TA<T>::Iterator i = this->accBegin();
		return this->end(this->getFinalState(), i);
	}

	typename TA<T>::Iterator accEnd(typename TA<T>::Iterator i) const {
		return this->end(this->getFinalState(), i);
	}

/*
	typename TA<T>::RhsIterator getRhsIterator(size_t state) const {
		return TA<T>::RhsIterator(*this, state);
	}*/
//	typename TA<T>::AcceptingIterator a_end() const {
//		assert(this->acceptingValid);
//		return TA<T>::AcceptingIterator(this->acceptingTransitions.end());
//	}

	typename TA<T>::TDIterator tdStart(const td_cache_type& cache) const {
		return typename TA<T>::TDIterator(cache, std::vector<size_t>(this->finalStates.begin(), this->finalStates.end()));
	}

	typename TA<T>::TDIterator tdStart(const td_cache_type& cache, const std::vector<size_t>& stack) const {
		return typename TA<T>::TDIterator(cache, stack);
	}

	TA<T>& operator=(const TA<T>& rhs) {
		this->clear();
		this->next_state = rhs.next_state;
		this->maxRank = rhs.maxRank;
		this->backend = rhs.backend;
		this->transitions = rhs.transitions;
		this->finalStates = rhs.finalStates;
		for (typename std::set<typename trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			this->transCache().addRef(*i);
		return *this;
	}

	void clear() {
		this->maxRank = 0;
		this->next_state = 0;
		for (typename std::set<typename trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			this->transCache().release(*i);
		this->transitions.clear();
		this->finalStates.clear();
	}
/*
	void loadFromDFS(const TA<T>::dfs_cache_type& dfsCache, const TA<T>& ta, const vector<size_t>& stack, bool registerFinalStates = true) {
		for (TA<T>::dfs_iterator i = ta.dfsStart(dfsCache, stack); i.isValid(); i.next())
			this->addTransition(*i);
		if (registerFinalStates)
			this->addFinalStates(stack);
	}
*/
	size_t newState() { return this->next_state++; }

	void updateStateCounter() {
		this->next_state = 0;
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			this->next_state = std::max(this->next_state, 1 + std::max((*i)->first._rhs, *std::max_element((*i)->first._lhs->first.begin(), (*i)->first._lhs->first.end())));
	}

	void buildStateIndex(Index<size_t>& index) const {
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			for (std::vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j)
				index.add(*j);
			index.add((*i)->first._rhs);
		}
		for (std::set<size_t>::const_iterator i = this->finalStates.begin(); i != this->finalStates.end(); ++i)
			index.add(*i);
	}

	void buildSortedStateIndex(Index<size_t>& index) const {
		std::set<size_t> s;
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			for (std::vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j)
				s.insert(*j);
			s.insert((*i)->first._rhs);
		}
		s.insert(this->finalStates.begin(), this->finalStates.end());
		for (std::set<size_t>::iterator i = s.begin(); i != s.end(); ++i)
			index.add(*i);
	}

	void buildLabelIndex(Index<T>& index) const {
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			index.add((*i)->first._label);
	}
/*
	void buildIndex(Index<T>& stateIndex, Index<T>& labelIndex) const {
		stateIndex.clear();
		labelIndex.clear();
		for (std::set<trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			for (vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j)
				stateIndex.add(*j);
			labelIndex.add((*i)->first._label);
			stateIndex.add((*i)->first._rhs);
		}
	}
*/
	void buildLhsIndex(Index<const std::vector<size_t>*>& index) const {
		for (typename std::set<typename trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			index.add(&(*i)->first._lhs->first);
	}

	void buildTDCache(td_cache_type& cache) const {
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			cache.insert(make_pair((*i)->first._rhs, std::vector<const Transition*>())).first->second.push_back(&(*i)->first);
	}

	void buildBUCache(bu_cache_type& cache) const {
		std::unordered_set<size_t> s;
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			s.clear();
			for (std::vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j) {
				if (s.insert(*j).second)
					cache.insert(make_pair(*j, std::vector<const Transition*>())).first->second.push_back(&(*i)->first);
			}
		}
	}
/*
	void buildSLTBUCache(slt_cache_type& cache, leaf_cache_type& leafCache) const {
		boost::unordered_map<std::pair<size_t, const T*>, std::set<const Transition*> > tmp;
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			if ((*i)->first._lhs->first.empty()) {
				leafCache.insert(make_pair(&(*i)->first._label, std::set<const Transition*>())).first->second.insert(&(*i)->first);
				continue;
			}
			for (std::vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j)
				tmp.insert(make_pair(make_pair(*j, &(*i)->first._label), vector<const Transition*>())).first->second.insert(&(*i)->first);
		}
		for (boost::unordered_map<std::pair<size_t, const T*>, std::set<const Transition*> >::const_iterator i = tmp.begin(); i != tmp.end(); ++i) {
			cache.insert(
				make_pair(i->first.first, boost::unordered_map<const T*, std::vector<const Transition*> >())
			).first->second.insert(
				make_pair(i->first.second, std::vector<const Transition*>(i->second.begin(), i->second.end()))
			);
		}
	}
*/
	void buildLTCache(lt_cache_type& cache) const {
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			cache.insert(make_pair((*i)->first._label, std::vector<const Transition*>())).first->second.push_back(&(*i)->first);
	}

	typename trans_cache_type::value_type* addTransition(const std::vector<size_t>& lhs, const T& label, size_t rhs) {
		return this->internalAdd(Transition(lhs, label, rhs, this->lhsCache()));
	}

	typename trans_cache_type::value_type* addTransition(const std::vector<size_t>& lhs, const T& label, size_t rhs, const std::vector<size_t>& index) {
		return this->internalAdd(Transition(lhs, label, rhs, index, this->lhsCache()));
	}

	typename trans_cache_type::value_type* addTransition(const typename trans_cache_type::value_type* transition) {
		return this->internalAdd(Transition(transition->first, this->lhsCache()));
	}

	typename trans_cache_type::value_type* addTransition(const typename trans_cache_type::value_type* transition, const std::vector<size_t>& index) {
		return this->internalAdd(Transition(transition->first, index, this->lhsCache()));
	}

	typename trans_cache_type::value_type* addTransition(const Transition& transition) {
		return this->internalAdd(Transition(transition, this->lhsCache()));
	}

	typename trans_cache_type::value_type* addTransition(const Transition& transition, const std::vector<size_t>& index) {
		return this->internalAdd(Transition(transition, index, this->lhsCache()));
	}

	void addFinalState(size_t state) {
		this->finalStates.insert(state);
	}

	void addFinalStates(const std::vector<size_t>& states) {
		this->finalStates.insert(states.begin(), states.end());
	}

	void addFinalStates(const std::set<size_t>& states) {
		this->finalStates.insert(states.begin(), states.end());
	}

	void removeFinalState(size_t state) {
		this->finalStates.erase(state);
	}

	void clearFinalStates() {
		this->finalStates.clear();
	}

	bool isFinalState(size_t state) const { return (this->finalStates.find(state) != this->finalStates.end()); }

	const std::set<size_t>& getFinalStates() const { return this->finalStates; }

	size_t getFinalState() const {
		assert(this->finalStates.size() == 1);
		return *this->finalStates.begin();
	}

	const trans_set_type& getTransitions() const { return this->transitions; }
/*
	TA<T>::RhsIterator getAcceptingTransitions() const {
		return this->getRhsIterator(this->getFinalState());
	}
*/
	const Transition& getAcceptingTransition() const
	{
		// Assertions
		assert(this->accBegin() != this->accEnd());
		assert(++(this->accBegin()) == this->accEnd());

		return *(this->accBegin());
	}

	void downwardTranslation(LTS& lts, const Index<size_t>& stateIndex, const Index<T>& labelIndex) const;

	void downwardSimulation(std::vector<std::vector<bool> >& rel, const Index<size_t>& stateIndex) const;

	void upwardTranslation(LTS& lts, std::vector<std::vector<size_t> >& part, std::vector<std::vector<bool> >& rel, const Index<size_t>& stateIndex, const Index<T>& labelIndex, const std::vector<std::vector<bool> >& sim) const;

	void upwardSimulation(std::vector<std::vector<bool> >& rel, const Index<size_t>& stateIndex, const std::vector<std::vector<bool> >& param) const;

	static void combinedSimulation(std::vector<std::vector<bool> >& dst, const std::vector<std::vector<bool> >& dwn, const std::vector<std::vector<bool> >& up) {
		size_t size = dwn.size();
		std::vector<std::vector<bool> > dut(size, std::vector<bool>(size, false));
		for (size_t i = 0; i < size; ++i) {
			for (size_t j = 0; j < size; ++j) {
				for (size_t k = 0; k < size; ++k) {
					if (dwn[i][k] && up[j][k]) {
						dut[i][j] = true;
						break;
					}
				}
			}
		}
		dst = dut;
		for (size_t i = 0; i < size; ++i) {
			for (size_t j = 0; j < size; ++j) {
				if (!dst[i][j])
					continue;
				for (size_t k = 0; k < size; ++k) {
					if (dwn[j][k] && !dut[i][k]) {
						dst[i][j] = false;
						break;
					}
				}
			}
		}
	}

	template <class F>
	static size_t buProduct(const lt_cache_type& cache1, const lt_cache_type& cache2, F f, size_t stateOffset = 0) {
		std::unordered_map<std::pair<size_t, size_t>, size_t, boost::hash<std::pair<size_t, size_t>>> product;
		for (typename lt_cache_type::const_iterator i = cache1.begin(); i != cache1.end(); ++i) {
			if (!i->second.front()->_lhs->first.empty())
				continue;
			typename lt_cache_type::const_iterator j = cache2.find(i->first);
			if (j == cache2.end())
				continue;
			for (typename std::vector<const Transition*>::const_iterator k = i->second.begin(); k != i->second.end(); ++k) {
				for (typename std::vector<const Transition*>::const_iterator l = j->second.begin(); l != j->second.end(); ++l) {
					std::pair<std::unordered_map<std::pair<size_t, size_t>, size_t, boost::hash<std::pair<size_t, size_t>>>::iterator, bool> p =
						product.insert(std::make_pair(std::make_pair((*k)->_rhs, (*l)->_rhs), product.size() + stateOffset));
					f(*k, *l, std::vector<size_t>(), p.first->second);
				}
			}
		}
		bool changed = true;
		while (changed) {
			changed = false;
			for (typename lt_cache_type::const_iterator i = cache1.begin(); i != cache1.end(); ++i) {
				if (i->second.front()->_lhs->first.empty())
					continue;
				typename lt_cache_type::const_iterator j = cache2.find(i->first);
				if (j == cache2.end())
					continue;
				for (typename std::vector<const Transition*>::const_iterator k = i->second.begin(); k != i->second.end(); ++k) {
					for (typename std::vector<const Transition*>::const_iterator l = j->second.begin(); l != j->second.end(); ++l) {
						assert((*k)->_lhs->first.size() == (*l)->_lhs->first.size());
						std::vector<size_t> lhs;
						for (size_t m = 0; m < (*k)->_lhs->first.size(); ++m) {
							std::unordered_map<std::pair<size_t, size_t>, size_t, boost::hash<std::pair<size_t, size_t>>>::iterator n = product.find(
								std::make_pair((*k)->_lhs->first[m], (*l)->_lhs->first[m])
							);
							if (n == product.end())
								break;
							lhs.push_back(n->second);
						}
						if (lhs.size() < (*k)->_lhs->first.size())
							continue;
						std::pair<std::unordered_map<std::pair<size_t, size_t>, size_t, boost::hash<std::pair<size_t, size_t>>>::iterator, bool> p =
							product.insert(std::make_pair(std::make_pair((*k)->_rhs, (*l)->_rhs), product.size() + stateOffset));
						f(*k, *l, lhs, p.first->second);
						if (p.second)
							changed = true;
					}
				}
			}
		}
		return product.size();
	}

	struct IntersectF {

		TA<T>& dst;
		const TA<T>& src1;
		const TA<T>& src2;

		IntersectF(TA<T>& dst, const TA<T>& src1, const TA<T>& src2) : dst(dst), src1(src1), src2(src2) {}

		void operator()(const Transition* t1, const Transition* t2, const std::vector<size_t>& lhs, size_t rhs) {
			if (this->src1.isFinalState(t1->_rhs) && this->src2.isFinalState(t2->_rhs))
				this->dst.addFinalState(rhs);
			this->dst.addTransition(lhs, t1->_label, rhs);
		}

	};

	static size_t intersection(TA<T>& dst, const TA<T>& src1, const TA<T>& src2, size_t stateOffset = 0) {
		lt_cache_type cache1, cache2;
		src1.buildLTCache(cache1);
		src2.buildLTCache(cache2);
		return TA<T>::buProduct(cache1, cache2, TA<T>::IntersectF(dst, src1, src2), stateOffset);
	}

	struct PredicateF {

		std::vector<size_t>& dst;
		const TA<T>& predicate;

		PredicateF(std::vector<size_t>& dst, const TA<T>& predicate) : dst(dst), predicate(predicate) {}

		void operator()(const Transition* /* t1 */, const Transition* t2,
			const std::vector<size_t>& /* lhs */, size_t /* rhs */) {
			if (predicate.isFinalState(t2->_rhs))
				this->dst.push_back(t2->_rhs);
		}

	};

	void intersectingStates(std::vector<size_t>& dst, const TA<T>& predicate) const {
		lt_cache_type cache1, cache2;
		this->buildLTCache(cache1);
		predicate.buildLTCache(cache2);
		TA<T>::buProduct(cache1, cache2, TA<T>::PredicateF(dst, predicate));
	}
/*
	template <class F>
	static bool tdMatch(const TA<T> ta1, size_t s1, const TA<T> ta2, size_t s2, F f) {
		std::unordered_map<size_t, size_t> matching;
		std::vector<std::pair<size_t, size_t> > stack;
		std::pair<size_t, size_t> s(s1, s2);
		product.insert(s);
		stack.push_back(s);
		while (!stack.empty()) {
			std::pair<size_t, size_t> el = stack.back();
			stack.pop_back();
			typename TA<T>::trans_set_type::const_iterator i = ta1._lookup(el.first);
			if (i == ta1.transitions.end())
				continue;
			typename TA<T>::trans_set_type::const_iterator j = ta2._lookup(el.second);
			if (j == ta2.transitions.end())
				continue;
			typename TA<T>::trans_set_type::const_iterator k = j;
			for (; i != ta1.transitions.end() && (*i)->first._rhs == el.first; ++i) {
				bool matched = false;
				for (; j != ta2.transitions.end() && (*j)->first._rhs == el.second; ++j) {
					std::vector<size_t> lhs1((*i)->first._lhs->first), lhs2((*j)->first._lhs->first);
					if (!f((*i)->first, lhs1, (*j)->first, lhs2))
						continue;
					matched = true;
					for (size_t m = 0; m < lhs1.size(); ++m) {
						std::pair<size_t, size_t> el(lhs1[m], lhs2[m]);

						if (product.insert(el).second)
							stack.push_back(el);
					}
				}
				if (!matched)
					return false;
				j = k;
			}
		}
		return true;
	}
*/
	template <class F>
	static bool transMatch(const Transition* t1, const Transition* t2, F f, const std::vector<std::vector<bool> >& mat, const Index<size_t>& stateIndex) {

		if (!f(*t1, *t2))
			return false;

		if (t1->_lhs->first.size() != t2->_lhs->first.size())
			return false;

		bool match = true;
		for (size_t m = 0; m < t1->_lhs->first.size(); ++m) {
			if (!mat[stateIndex[t1->_lhs->first[m]]][stateIndex[t2->_lhs->first[m]]]) {
				match = false;
				break;
			}
		}
		return match;

	}

	// currently erases '1' from the relation
	template <class F>
	void heightAbstraction(std::vector<std::vector<bool> >& result, size_t height, F f, const Index<size_t>& stateIndex) const {

		td_cache_type cache;
		this->buildTDCache(cache);

		std::vector<std::vector<bool> > tmp;

		while (height--) {
			tmp = result;

			for (Index<size_t>::iterator i = stateIndex.begin(); i != stateIndex.end(); ++i) {
				size_t state1 = i->second;
				typename td_cache_type::iterator j = cache.insert(
					std::make_pair(i->first, std::vector<const Transition*>())
				).first;
				for (Index<size_t>::iterator k = stateIndex.begin(); k != stateIndex.end(); ++k) {
					size_t state2 = k->second;
					if ((state1 == state2) || !tmp[state1][state2])
						continue;
					typename td_cache_type::iterator l = cache.insert(
						std::make_pair(k->first, std::vector<const Transition*>())
					).first;
					bool match = true;
					for (typename std::vector<const Transition*>::const_iterator m = j->second.begin(); m != j->second.end(); ++m) {
						for (typename std::vector<const Transition*>::const_iterator n = l->second.begin(); n != l->second.end(); ++n) {
							if (!TA<T>::transMatch(*m, *n, f, tmp, stateIndex)) {
								match = false;
								break;
							}
						}
						if (!match)
							break;
					}
					if (!match)
						result[state1][state2] = false;
				}
			}
		}

		for (size_t i = 0; i < result.size(); ++i) {
			for (size_t j = 0; j < i; ++j) {
				if (!result[i][j])
					result[j][i] = false;
				if (!result[j][i])
					result[i][j] = false;
			}
		}

	}

	void predicateAbstraction(std::vector<std::vector<bool> >& result, const TA<T>& predicate, const Index<size_t>& stateIndex) const {
		std::vector<size_t> states;
		this->intersectingStates(states, predicate);
		std::set<size_t> s;
		for (std::vector<size_t>::iterator i = states.begin(); i != states.end(); ++i)
			s.insert(stateIndex[*i]);
		for (size_t i = 0; i < result.size(); ++i) {
			if (s.count(i) == 1)
				continue;
			for (size_t j = 0; j < i; ++j) {
				result[i][j] = 0;
				result[j][i] = 0;
			}
			for (size_t j = i + 1; j < result.size(); ++j) {
				result[i][j] = 0;
				result[j][i] = 0;
			}
		}
	}

	// collapses states according to a given relation
	TA<T>& collapsed(TA<T>& dst, const std::vector<std::vector<bool> >& rel, const Index<size_t>& stateIndex) const {
		std::vector<size_t> headIndex;
		utils::relBuildClasses(rel, headIndex);
		// TODO: perhaps improve indexing
		std::vector<size_t> invStateIndex(stateIndex.size());
		for (Index<size_t>::iterator i = stateIndex.begin(); i != stateIndex.end(); ++i)
			invStateIndex[i->second] = i->first;
		for (std::vector<size_t>::iterator i = headIndex.begin(); i != headIndex.end(); ++i)
			*i = invStateIndex[*i];
		for (std::set<size_t>::const_iterator i = this->finalStates.begin(); i != this->finalStates.end(); ++i)
			dst.addFinalState(headIndex[stateIndex[*i]]);
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			std::vector<size_t> lhs;
			stateIndex.translate(lhs, (*i)->first._lhs->first);
			for (size_t j = 0; j < lhs.size(); ++j)
				lhs[j] = headIndex[lhs[j]];
			dst.addTransition(lhs, (*i)->first._label, headIndex[stateIndex[(*i)->first._rhs]]);
		}
		return dst;
	}

	TA<T>& uselessFree(TA<T>& dst) const {
		std::vector<typename trans_cache_type::value_type*> v1(this->transitions.begin(), this->transitions.end()), v2;
		std::set<size_t> states;
		bool changed = true;
		while (changed) {
			changed = false;
			for (typename std::vector<typename trans_cache_type::value_type*>::const_iterator i = v1.begin(); i != v1.end(); ++i) {
				bool matches = true;
				for (std::vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j) {
					if (!states.count(*j)) {
						matches = false;
						break;
					}
				}
				if (matches) {
					if (states.insert((*i)->first._rhs).second)
						changed = true;
					dst.addTransition(*i);
				} else {
					v2.push_back(*i);
				}
			}
			v1.clear();
			std::swap(v1, v2);
		}
		for (std::set<size_t>::iterator i = this->finalStates.begin(); i != this->finalStates.end(); ++i) {
			if (states.count(*i))
				dst.finalStates.insert(*i);
		}
		return dst;
	}

	TA<T>& unreachableFree(TA<T>& dst) const {
		std::vector<typename trans_cache_type::value_type*> v1(transitions.begin(), this->transitions.end()), v2;
		std::set<size_t> states(this->finalStates.begin(), this->finalStates.end());
		for (std::set<size_t>::const_iterator i = this->finalStates.begin(); i != this->finalStates.end(); ++i)
			dst.addFinalState(*i);
		bool changed = true;
		while (changed) {
			changed = false;
			for (typename std::vector<typename trans_cache_type::value_type*>::const_iterator i = v1.begin(); i != v1.end(); ++i) {
				if (states.count((*i)->first._rhs)) {
					dst.addTransition(*i);
					for (std::vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j) {
						if (states.insert(*j).second)
							changed = true;
					}
				} else {
					v2.push_back(*i);
				}
			}
			v1.clear();
			std::swap(v1, v2);
		}
		return dst;
	}

	TA<T>& uselessAndUnreachableFree(TA<T>& dst) const {
		std::vector<typename trans_cache_type::value_type*> v1(this->transitions.begin(), this->transitions.end()), v2, v3;
		std::set<size_t> states;
		bool changed = true;
		while (changed) {
			changed = false;
			for (typename std::vector<typename trans_cache_type::value_type*>::const_iterator i = v1.begin(); i != v1.end(); ++i) {
				bool matches = true;
				for (std::vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j) {
					if (!states.count(*j)) {
						matches = false;
						break;
					}
				}
				if (matches) {
					if (states.insert((*i)->first._rhs).second)
						changed = true;
					v3.push_back(*i);
				} else {
					v2.push_back(*i);
				}
			}
			v1.clear();
			std::swap(v1, v2);
		}
		for (std::set<size_t>::iterator i = this->finalStates.begin(); i != this->finalStates.end(); ++i) {
			if (states.count(*i))
				dst.finalStates.insert(*i);
		}
		std::swap(v1, v3);
		v2.clear();
		states = std::set<size_t>(dst.finalStates.begin(), dst.finalStates.end());
		changed = true;
		while (changed) {
			changed = false;
			for (typename std::vector<typename trans_cache_type::value_type*>::const_iterator i = v1.begin(); i != v1.end(); ++i) {
				if (states.count((*i)->first._rhs)) {
					dst.addTransition(*i);
					for (std::vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j) {
						if (states.insert(*j).second)
							changed = true;
					}
				} else {
					v2.push_back(*i);
				}
			}
			v1.clear();
			std::swap(v1, v2);
		}
		return dst;
	}

	TA<T>& downwardSieve(TA<T>& dst, const std::vector<std::vector<bool> >& cons, const Index<size_t>& stateIndex) const {

		td_cache_type cache;
		this->buildTDCache(cache);

		for (std::set<size_t>::const_iterator i = this->finalStates.begin(); i != this->finalStates.end(); ++i)
			dst.addFinalState(*i);
		for (typename td_cache_type::iterator i = cache.begin(); i != cache.end(); ++i) {
			std::list<const Transition*> tmp;
			for (typename std::vector<const Transition*>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
				bool noskip = true;
				for (typename std::list<const Transition*>::iterator k = tmp.begin(); k != tmp.end(); ) {
					if ((*j)->llhsLessThan(**k, cons, stateIndex)) {
						noskip = false;
						break;
					}
					if ((*k)->llhsLessThan(**j, cons, stateIndex)) {
						typename std::list<const Transition*>::iterator l = k++;
						tmp.erase(l);
					} else ++k;
				}
				if (noskip)
					tmp.push_back(*j);
			}
			for (typename std::list<const Transition*>::iterator j = tmp.begin(); j != tmp.end(); ++j)
				dst.addTransition(**j);
		}
		return dst;

	}

	TA<T>& minimized(TA<T>& dst, const std::vector<std::vector<bool> >& cons, const Index<size_t>& stateIndex) const {
		typename TA<T>::Backend backend;
		std::vector<std::vector<bool> > dwn;
		this->downwardSimulation(dwn, stateIndex);
		utils::relAnd(dwn, cons, dwn);
		TA<T> tmp1(backend), tmp2(backend), tmp3(backend);
		return this->collapsed(tmp1, dwn, stateIndex).uselessFree(tmp2).downwardSieve(tmp3, dwn, stateIndex).unreachableFree(dst);
	}

	TA<T>& minimizedCombo(TA<T>& dst) const {
		Index<size_t> stateIndex;
		this->buildSortedStateIndex(stateIndex);
		typename TA<T>::Backend backend;
		std::vector<std::vector<bool> > dwn;
		this->downwardSimulation(dwn, stateIndex);
		std::vector<std::vector<bool> > up;
		this->upwardSimulation(up, stateIndex, dwn);
		std::vector<std::vector<bool> > rel;
		TA<T>::combinedSimulation(rel, dwn, up);
		TA<T> tmp(backend);
		return this->collapsed(tmp, rel, stateIndex).minimized(dst);
	}

	TA<T>& minimized(TA<T>& dst) const {
		Index<size_t> stateIndex;
		this->buildSortedStateIndex(stateIndex);
		std::vector<std::vector<bool> > cons(stateIndex.size(), std::vector<bool>(stateIndex.size(), true));
		return this->minimized(dst, cons, stateIndex);
	}

	static bool subseteq(const TA<T>& a, const TA<T>& b);

	template <class F>
	static TA<T>& rename(TA<T>& dst, const TA<T>& src, F f, bool addFinalStates = true) {
		std::vector<size_t> lhs;
		if (addFinalStates) {
			for (std::set<size_t>::const_iterator i = src.finalStates.begin(); i != src.finalStates.end(); ++i)
				dst.addFinalState(f(*i));
		}
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = src.transitions.begin(); i != src.transitions.end(); ++i) {
			lhs.resize((*i)->first._lhs->first.size());
			for (size_t j = 0; j < (*i)->first._lhs->first.size(); ++j)
				lhs[j] = f((*i)->first._lhs->first[j]);
			dst.addTransition(lhs, (*i)->first._label, f((*i)->first._rhs));
		}
		return dst;
	}

	static TA<T>& reduce(TA<T>& dst, const TA<T>& src, Index<size_t>& index, size_t offset = 0, bool addFinalStates = true) {
		std::vector<size_t> lhs;
		if (addFinalStates) {
			for (std::set<size_t>::const_iterator i = src.finalStates.begin(); i != src.finalStates.end(); ++i)
				dst.addFinalState(index.translateOTF(*i) + offset);
		}
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = src.transitions.begin(); i != src.transitions.end(); ++i) {
			lhs.clear();
			index.translateOTF(lhs, (*i)->first._lhs->first, offset);
			dst.addTransition(lhs, (*i)->first._label, index.translateOTF((*i)->first._rhs) + offset);
		}
		return dst;
	}

public:

	struct AcceptingF {
		const TA<T>& ta;
		AcceptingF(const TA<T>& ta) : ta(ta) {}
		bool operator()(const Transition* t) { return ta.isFinalState(t->_rhs); }
	};

	struct NonAcceptingF {
		const TA<T>& ta;
		NonAcceptingF(const TA<T>& ta) : ta(ta) {}
		bool operator()(const Transition* t) { return !ta.isFinalState(t->_rhs); }
	};

	TA& copyTransitions(TA<T>& dst) const {
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			dst.addTransition(*i);
		return dst;
	}

	template <class F>
	TA& copyTransitions(TA<T>& dst, F f) const {
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			if (f(&(*i)->first))
				dst.addTransition(*i);
		}
		return dst;
	}

	// makes state numbers contiguous
	TA& reduced(TA<T>& dst, Index<size_t>& index) const {
		return TA<T>::reduce(dst, *this, index);
	}

	static TA<T>& disjointUnion(TA<T>& dst, const TA<T>& a, const TA<T>& b) {
		for (std::set<size_t>::const_iterator i = a.finalStates.begin(); i != a.finalStates.end(); ++i)
			dst.addFinalState(*i);
		for (std::set<size_t>::const_iterator i = b.finalStates.begin(); i != b.finalStates.end(); ++i)
			dst.addFinalState(*i);
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = a.transitions.begin(); i != a.transitions.end(); ++i)
			dst.addTransition(*i);
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = b.transitions.begin(); i != b.transitions.end(); ++i)
			dst.addTransition(*i);
		return dst;
	}

	static TA<T>& disjointUnion(TA<T>& dst, const TA<T>& src, bool addFinalStates = true) {
		if (addFinalStates) {
			for (std::set<size_t>::const_iterator i = src.finalStates.begin(); i != src.finalStates.end(); ++i)
				dst.addFinalState(*i);
		}
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = src.transitions.begin(); i != src.transitions.end(); ++i)
			dst.addTransition(*i);
		return dst;
	}

	static TA<T>& renamedUnion(TA<T>& dst, const TA<T>& a, const TA<T>& b, size_t& aSize, size_t offset = 0) {
		Index<size_t> index;
		reduce(dst, a, index, offset);
		aSize = index.size();
		index.clear();
		reduce(dst, b, index, aSize, offset);
		return dst;
	}

	static TA<T>& renamedUnion(TA<T>& dst, const TA<T>& src, size_t offset, size_t& srcSize) {
		Index<size_t> index;
		reduce(dst, src, index, offset);
		srcSize = index.size();
		return dst;
	}

	TA<T>& unfoldAtRoot(TA<T>& dst, size_t newState, bool registerFinalState = true) const {
		if (registerFinalState)
			dst.addFinalState(newState);
		for (typename std::set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			dst.addTransition(*i);
			if (this->isFinalState((*i)->first._rhs))
				dst.addTransition((*i)->first._lhs->first, (*i)->first._label, newState);
		}
		return dst;
	}

	TA<T>& unfoldAtRoot(TA<T>& dst, const std::unordered_map<size_t, size_t>& states, bool registerFinalState = true) const {
		this->copyTransitions(dst);
		for (std::set<size_t>::const_iterator i = this->finalStates.begin(); i != this->finalStates.end(); ++i) {
			std::unordered_map<size_t, size_t>::const_iterator j = states.find(*i);
			assert(j != states.end());
			for (typename trans_set_type::const_iterator k = this->_lookup(*i); k != this->transitions.end() && (*k)->first._rhs == *i; ++k)
				dst.addTransition((*k)->first._lhs->first, (*k)->first._label, j->second);
			if (registerFinalState)
				dst.addFinalState(j->second);
		}
		return dst;
	}

	/**
	 * @brief  Run a visitor on the instance
	 *
	 * This method is the @p accept method of the Visitor design pattern.
	 *
	 * @param[in]  visitor  The visitor of the type @p TVisitor
	 *
	 * @tparam  TVisitor  The type of the visitor
	 */
	template <class TVisitor>
	void accept(TVisitor& visitor) const
	{
		visitor(*this);
	}

/*
	TA<T>& unfoldAtLeaf(TA<T>& dst, size_t selector) const {
		// TODO:
		return dst;
	}
*/
	class Manager : Cache<TA<T>*>::Listener {

		typename TA<T>::Backend& backend;

		Cache<TA<T>*> taCache;
		std::vector<TA<T>*> taPool;

	protected:

		virtual void drop(typename Cache<TA<T>*>::value_type* x) {
			x->first->clear();
			this->taPool.push_back(x->first);
		}

	public:

		Manager(typename TA<T>::Backend& backend) :
			backend(backend),
			taCache{},
			taPool{}
		{
			this->taCache.addListener(this);
		}

		~Manager() {
			utils::erase(this->taPool);
			assert(this->taCache.empty());
		}

		const Cache<TA<T>*>& getCache() const {
			return this->taCache;
		}

		TA<T>* alloc() {
			TA<T>* dst;
			if (!this->taPool.empty()) {
				dst = this->taPool.back();
				this->taPool.pop_back();
			} else {
				dst = new TA<T>(this->backend);
			}
			return this->taCache.lookup(dst)->first;
		}

		TA<T>* clone(TA<T>* src, bool copyFinalStates = true) {
			assert(src);
			assert(src->backend == &this->backend);
			return this->taCache.lookup(new TA<T>(*src, copyFinalStates))->first;
		}

		TA<T>* addRef(TA<T>* x) {
			typename Cache<TA<T>*>::value_type* v = this->taCache.find(x);
			assert(v);
			return this->taCache.addRef(v), x;
		}

		size_t release(TA<T>* x) {
			typename Cache<TA<T>*>::value_type* v = this->taCache.find(x);
			assert(v);
			return this->taCache.release(v);
		}

		void clear() {
			this->taCache.clear();
			for (typename std::vector<TA<T>*>::iterator i = this->taPool.begin(); i != this->taPool.end(); ++i)
				delete *i;
			this->taPool.clear();
		}

		bool isAlive(TA<T>* x) {
			return this->taCache.find(x) != nullptr;
		}

		typename TA<T>::Backend& getBackend() {
			return this->backend;
		}

	};

};
#endif
