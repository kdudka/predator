#ifndef TREEAUT_H
#define TREEAUT_H

#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <cassert>
#include <stdexcept>

#include <boost/unordered_map.hpp>

#include "cache.hh"
#include "utils.hh"
//#include "timreader.hh"
//#include "timwriter.hh"
#include "lts.hh"

using std::vector;
using std::set;
using std::map;
using std::pair;
using std::make_pair;

using boost::hash_value;
using boost::unordered_map;

template <class T> class TA;

template <class T>
class TT {

	friend class TA<T>;

public:

	typedef Cache<vector<size_t> > lhs_cache_type;

private:

	mutable lhs_cache_type& lhsCache;

public:

	lhs_cache_type::value_type* _lhs;
	T _label;
	size_t _rhs;

	TT(const TT& transition)
		: lhsCache(transition.lhsCache), _label(transition._label) {
		this->_lhs = this->lhsCache.lookup(transition._lhs->first);
//		this->_label = transition._label;
		this->_rhs = transition._rhs;
	}

	TT(const vector<size_t>& lhs, const T& label, size_t rhs, lhs_cache_type& lhsCache)
		: lhsCache(lhsCache), _label(label) {
		this->_lhs = this->lhsCache.lookup(lhs);
//		this->_label = label;
		this->_rhs = rhs;
	}
	
	TT(const vector<size_t>& lhs, const T& label, size_t rhs, const vector<size_t>& index, lhs_cache_type& lhsCache)
		: lhsCache(lhsCache), _label(label) {
		vector<size_t> tmp(lhs.size());
		for (size_t i = 0; i < lhs.size(); ++i)
			tmp[i] = index[lhs[i]];
		this->_lhs = this->lhsCache.lookup(tmp);
//		this->_label = label;
		this->_rhs = index[rhs];
	}

	TT(const TT& transition, lhs_cache_type& lhsCache)
		: lhsCache(lhsCache), _label(transition._label) {
		this->_lhs = this->lhsCache.lookup(transition._lhs->first);
//		this->_label = transition._label;
		this->_rhs = transition._rhs;
	}
  
	TT(const TT& transition, const vector<size_t>& index, lhs_cache_type& lhsCache)
		: lhsCache(lhsCache), _label(transition._label) {
		vector<size_t> tmp(transition._lhs->first.size());
		for (size_t i = 0; i < transition._lhs->first.size(); ++i)
			tmp[i] = index[transition._lhs->first[i]];
		this->_lhs = this->lhsCache.lookup(tmp);
//		this->_label = transition._label;
		this->_rhs = index[transition._rhs];
	}

	~TT() { this->lhsCache.release(this->_lhs);	}

	const vector<size_t>& lhs() const { return this->_lhs->first; }
	const T& label() const { return this->_label; }
	size_t rhs() const { return this->_rhs; }
	
	bool operator==(const TT& rhs) const {
		return (this->_label == rhs._label) && (this->_lhs == rhs._lhs) && (this->_rhs == rhs._rhs);
	}

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
/*		return (this->_lhs->first < rhs._lhs->first) || (
			(this->_lhs->first == rhs._lhs->first) && (
				(this->_label < rhs._label) || (
					(this->_label == rhs._label) && (
						this->_rhs < rhs._rhs
					)
				)
			)
		);*/
	}
	
	friend size_t hash_value(const TT& t) {
		return hash_value(hash_value(hash_value(t._lhs) + hash_value(t._label)) + hash_value(t._rhs));
	}
	
	friend std::ostream& operator<<(std::ostream& os, const TT& t) {
		os << t._label << '(';
		if (t._lhs->first.size() > 0) {
			os << t._lhs->first[0];
			for (size_t i = 1; i < t._lhs->first.size(); ++i)
				os << ',' << t._lhs->first[i];
		}
		return os << ")->" << t._rhs;
	}

};

template <class T>
class TA {

	friend class TAManager;

public:

    typedef Cache<TT<T> > trans_cache_type;

	struct Backend {

		typename TT<T>::lhs_cache_type lhsCache;
		trans_cache_type transCache;

	};

	mutable Backend* backend;
	
	typename TT<T>::lhs_cache_type& lhsCache() const { return this->backend->lhsCache; }

	trans_cache_type& transCache() const { return this->backend->transCache; }

	class Iterator {
		typename set<typename trans_cache_type::value_type*>::const_iterator _i;
	public:
		Iterator(typename set<typename trans_cache_type::value_type*>::const_iterator i) : _i(i) {}

		Iterator& operator++() { return ++this->_i, *this; }

		Iterator operator++(int) { return Iterator(this->_i++); }

		const TT<T>& operator*() const { return (*this->_i)->first; }

		const TT<T>* operator->() const { return &(*this->_i)->first; }

		bool operator==(const Iterator& rhs) const { return this->_i == rhs._i; }

		bool operator!=(const Iterator& rhs) const { return this->_i != rhs._i; }

	};
/*
	class Reader : public TimbukReader {

		TA<size_t>* dst;
		string name;

	protected:
	
		virtual void newLabel(const string&, size_t, size_t) {}
		
		virtual void beginModel(const string& name) {
			this->dst->clear();
			this->name = name;
		}
		
		virtual void newState(const string&, size_t) {}
		
		virtual void newFinalState(size_t id) {
			this->dst->addFinalState(id);
		}
		
		virtual void endDeclaration() {}

		virtual void newTransition(const vector<size_t>& lhs, size_t label, size_t rhs) {
			this->dst->addTransition(lhs, label, rhs);
		}
		
		virtual void endModel() {}
		
	public:

		Reader(std::istream& input = std::cin, const string& name = "")
			: TimbukReader(input, name), dst(NULL) {}
		
		TA<size_t>& run(TA<size_t>& dst) {
			this->dst = &dst;
			this->run_simple();
			return dst;
		}

	};

	class MultiReader : public TimbukReader {

		Backend& backend;

	public:

		vector<TA<size_t> > automata;
		vector<string> names;

	protected:
	
		virtual void newLabel(const string&, size_t, size_t) {}
		
		virtual void beginModel(const string& name) {
			this->automata.push_back(TA<size_t>(this->cache));
			this->names.push_back(name);
		}
		
		virtual void newState(const string&, size_t) {}
		
		virtual void newFinalState(size_t id) {
			this->automata.back().addFinalState(id);
		}
		
		virtual void endDeclaration() {}

		virtual void newTransition(const vector<size_t>& lhs, size_t label, size_t rhs) {
			this->automata.back().addTransition(lhs, label, rhs);
		}
		
		virtual void endModel() {}
		
	public:

		MultiReader(std::istream& input, Backend& backend, const string& name = "")
			: TimbukReader(input, name), backend(backend) {}

		void clear() {
			this->automata.clear();
			this->names.clear();
		}

		void run() {
			this->run_main();
		}

	};
*/
public:

	typedef Iterator iterator;

public:

//	int labels;
	size_t next_state;
	size_t maxRank;

	set<typename trans_cache_type::value_type*> transitions;
	set<size_t> finalStates;
	
//	std::map<const std::vector<int>*, int> lhsMap;

public:
/*
	static TA<size_t>& fromStream(TA<size_t>& dst, std::istream& input = std::cin, const string& name = "") {
		return Reader(input, name).run(dst);
	}
	
	static TA<size_t>& fromStream(TA<size_t>& dst, Reader& reader) {
		return reader.run(dst);
	}

	void toStream(std::ostream& output, const string& name = "TreeAutomaton") {
		map<T, size_t> labels;
		set<size_t> states;
		for (typename set<typename trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			labels.insert(make_pair((*i)->first._label, (*i)->first._lhs->first.size()));
			states.insert((*i)->first._rhs);
			for (size_t j = 0; j < (*i)->first._lhs->first.size(); ++j)
				states.insert((*i)->first._lhs->first[j]);
		}
		TimbukWriter writer(output);
		writer.startAlphabet();
		for (typename map<T, size_t>::iterator i = labels.begin(); i != labels.end(); ++i)
			writer.writeLabel(i->first, i->second);
		writer.newModel(name);
		writer.startStates();
		for (set<size_t>::iterator i = states.begin(); i != states.end(); ++i)
			writer.writeState(*i);
		writer.startFinalStates();
		for (set<size_t>::iterator i = this->finalStates.begin(); i != this->finalStates.end(); ++i)
			writer.writeState(*i);
		writer.startTransitions();
		for (typename set<typename trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			writer.writeTransition((*i)->first._lhs->first, (*i)->first._label, (*i)->first._rhs);
		writer.terminate();
	}
*/
	TA(Backend& backend) : backend(&backend), next_state(0), maxRank(0) {}
	
	TA(const TA<T>& ta) : backend(ta.backend), next_state(ta.next_state), maxRank(ta.maxRank), transitions(ta.transitions), finalStates(ta.finalStates) {
		for (typename std::set<typename trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			this->transCache().addRef(*i);
	}
	
	~TA() { this->clear(); }

	TA<T>::Iterator begin() const { return TA<T>::Iterator(this->transitions.begin()); }
	TA<T>::Iterator end() const { return TA<T>::Iterator(this->transitions.end()); }
	
	TA<T>& operator=(const TT<T>& rhs) {
		this->clear();
		this->next_state = rhs.next_state;
		this->maxRank = rhs.maxRank;
		this->backend = rhs.backend;
		this->transitions = rhs.transitions;
		this->finalStates = rhs.finalStates;
		for (typename set<typename trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			this->transCache().addRef(*i);
		return *this;
	}

	void clear() {
		this->maxRank = 0;
		this->next_state = 0;
		for (typename set<typename trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			this->transCache().release(*i);
		this->transitions.clear();
		this->finalStates.clear();
	}
	
	size_t newState() { return this->next_state++; }
	
	void updateStateCounter() {
		this->next_state = 0;
		for (typename set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			this->next_state = std::max(this->next_state, 1 + std::max((*i)->first._rhs, *std::max_element((*i)->first._lhs->first.begin(), (*i)->first._lhs->first.end())));
	}
	
	void buildStateIndex(Index<size_t>& index) const {
		index.clear();
		for (typename set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			for (vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j)
				index.add(*j);
			index.add((*i)->first._rhs);
		}
	}

	void buildLabelIndex(Index<T>& index) const {
		index.clear();
		for (typename set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			index.add((*i)->first._label);
	}
/*
	void buildIndex(Index<T>& stateIndex, Index<T>& labelIndex) const {
		stateIndex.clear();
		labelIndex.clear();
		for (set<trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			for (vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j)
				stateIndex.add(*j);
			labelIndex.add((*i)->first._label);
			stateIndex.add((*i)->first._rhs);
		}
	}
*/
	void buildLhsIndex(Index<vector<size_t>*>& index) const {
		index.clear();
		for (typename set<typename trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			index.add(&(*i)->first._lhs->first);
	}
	
	typename trans_cache_type::value_type* addTransition(const vector<size_t>& lhs, const T& label, size_t rhs) {
		if (lhs.size() > this->maxRank)
			this->maxRank = lhs.size();
		typename trans_cache_type::value_type* x = this->transCache().lookup(TT<T>(lhs, label, rhs, this->lhsCache()));
		if (!this->transitions.insert(x).second)
			this->transCache().release(x);
		return x;
	}

	typename trans_cache_type::value_type* addTransition(const vector<size_t>& lhs, const T& label, size_t rhs, const vector<size_t>& index) {
		if (lhs.size() > this->maxRank)
			this->maxRank = lhs.size();
		typename trans_cache_type::value_type* x = this->transCache().lookup(TT<T>(lhs, label, rhs, index, this->lhsCache()));
		if (!this->transitions.insert(x).second)
			this->transCache().release(x);
		return x;
	}
/* !! uncommenting this will screw it up
protected:

	void addTransition(TA::cache_type::iterator transition) {
		if (transition->first.lhs().size() > this->maxRank)
			this->maxRank = transition->first.lhs().size();
		TA::cache_type::iterator x = this->cacheAddRef(transition);
		if (!this->transitions.insert(x).second)
			this->cacheRelease(x);
	}

public:
*/
	typename trans_cache_type::value_type* addTransition(const typename trans_cache_type::value_type* transition) {
		if (transition->first.lhs().size() > this->maxRank)
			this->maxRank = transition->first.lhs().size();
		typename trans_cache_type::value_type* x = this->transCache().lookup(TT<T>(transition->first, this->lhsCache()));
		if (!this->transitions.insert(x).second)
			this->transCache().release(x);
		return x;
	}

	typename trans_cache_type::value_type* addTransition(const typename trans_cache_type::value_type* transition, const vector<size_t>& index) {
		if (transition->first.lhs().size() > this->maxRank)
			this->maxRank = transition->first.lhs().size();
		typename trans_cache_type::value_type* x = this->transCache().lookup(TT<T>(transition->first, index, this->lhsCache()));
		if (!this->transitions.insert(x).second)
			this->transCache().release(x);
		return x;
	}

	typename trans_cache_type::value_type* addTransition(const TT<T>& transition) {
		if (transition.lhs().size() > this->maxRank)
			this->maxRank = transition.lhs().size();
		typename trans_cache_type::value_type* x = this->transCache().lookup(TT<T>(transition, this->lhsCache()));
		if (!this->transitions.insert(x).second)
			this->transCache().release(x);
		return x;
	}

	typename trans_cache_type::value_type* addTransition(const TT<T>& transition, const vector<size_t>& index) {
		if (transition.lhs().size() > this->maxRank)
			this->maxRank = transition.lhs().size();
		typename trans_cache_type::value_type* x = this->transCache().lookup(TT<T>(transition, index, this->lhsCache()));
		if (!this->transitions.insert(x).second)
			this->transCache().release(x);
		return x;
	}

	void addFinalState(size_t state) { this->finalStates.insert(state); }
	
	void removeFinalState(size_t state) { this->finalStates.erase(state); }
	
	void clearFinalStates() { this->finalStates.clear(); }

	bool isFinalState(size_t state) const { return (this->finalStates.find(state) != this->finalStates.end()); }
	
	const set<size_t>& getFinalStates() const { return this->finalStates; }
	
	size_t getFinalState() const {
		assert(this->finalStates.size() == 1);
		return *this->finalStates.begin();
	}
	
	const set<typename trans_cache_type::value_type*>& getTransitions() const { return this->transitions; }

	void downwardTranslation(LTS& lts, const Index<size_t>& stateIndex, const Index<T>& labelIndex) const;
	
	void downwardSimulation(vector<vector<bool> >& rel, const Index<size_t>& stateIndex) const;
	
	void upwardTranslation(LTS& lts, vector<vector<size_t> >& part, vector<vector<bool> >& rel, const Index<size_t>& stateIndex, const Index<T>& labelIndex, const vector<vector<bool> >& sim) const;

	void upwardSimulation(vector<vector<bool> >& rel, const Index<size_t>& stateIndex, const vector<vector<bool> >& param) const;
	
	static void combinedSimulation(vector<vector<bool> >& dst, const vector<vector<bool> >& dwn, const vector<vector<bool> >& up) {
		size_t size = dwn.size();
		vector<vector<bool> > dut(size, vector<bool>(size, false));
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
	
	// collapses states according to a given relation
	TA<T>& collapsed(TA<T>& dst, const vector<vector<bool> >& rel, const Index<size_t>& stateIndex) const {
		vector<size_t> index, head;
		utils::relBuildClasses(rel, index, head);
		for (typename set<typename trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			vector<size_t> lhs;
			stateIndex.translate(lhs, (*i)->first._lhs->first);
			for (size_t j = 0; j < lhs.size(); ++j)
				lhs[j] = head[index[lhs[j]]];
			dst.addTransition(lhs, (*i)->first._label, head[index[stateIndex[(*i)->first._rhs]]]);
		}
		for (set<size_t>::const_iterator i = this->finalStates.begin(); i != this->finalStates.end(); ++i)
			dst.addFinalState(head[index[stateIndex[*i]]]);
		return dst; 
	}
	
	TA<T>& uselessFree(TA<T>& dst) const {
		vector<typename trans_cache_type::value_type*> v1(this->transitions.begin(), this->transitions.end()), v2;
		set<size_t> states;
		bool changed = true;
		while (changed) {
			changed = false;
			for (typename vector<typename trans_cache_type::value_type*>::const_iterator i = v1.begin(); i != v1.end(); ++i) {
				bool matches = true;
				for (vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j) {
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
		for (set<size_t>::const_iterator i = this->finalStates.begin(); i != this->finalStates.end(); ++i) {
			if (states.count(*i))
				dst.addFinalState(*i);
		}
		return dst;
	}
	
	TA<T>& unreachableFree(TA<T>& dst) const {
		vector<typename trans_cache_type::value_type*> v1(transitions.begin(), this->transitions.end()), v2;
		set<size_t> states(this->finalStates.begin(), this->finalStates.end());
		bool changed = true;
		while (changed) {
			changed = false;
			for (typename vector<typename trans_cache_type::value_type*>::const_iterator i = v1.begin(); i != v1.end(); ++i) {
				if (states.count((*i)->first._rhs)) {
					dst.addTransition(*i);
					for (vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j) {
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
		for (set<size_t>::const_iterator i = this->finalStates.begin(); i != this->finalStates.end(); ++i)
			dst.addFinalState(*i);
		return dst;
	}
	
	TA<T>& minimized(TA<T>& dst, const vector<vector<bool> >& cons, const Index<size_t>& stateIndex) const {
		TA<T>::Backend backend;
		vector<vector<bool> > dwn;
		this->downwardSimulation(dwn, stateIndex);
		utils::relAnd(dwn, cons, dwn);
		TA<T> tmp1(backend), tmp2(backend);
		return this->collapsed(tmp1, dwn, stateIndex).uselessFree(tmp2).unreachableFree(dst);
	}
	
	TA<T>& minimized(TA<T>& dst) const {
		Index<size_t> stateIndex;
		this->buildStateIndex(stateIndex);
		vector<vector<bool> > cons(stateIndex.size(), vector<bool>(stateIndex.size(), true));
		return this->minimized(dst, cons, stateIndex);
	}
	
	static bool subseteq(const TA<T>& a, const TA<T>& b);

	static TA<T>& reduce(TA<T>& dst, const TA<T>& src, Index<size_t>& index, size_t offset = 0, bool addFinalStates = true) {
		vector<size_t> lhs;
		for (typename set<typename trans_cache_type::value_type*>::const_iterator i = src.transitions.begin(); i != src.transitions.end(); ++i) {
			lhs.clear();
			index.translateOTF(lhs, (*i)->first._lhs->first, offset);
			dst.addTransition(lhs, (*i)->first._label, index.translateOTF((*i)->first._rhs) + offset);
		}
		if (addFinalStates) {
			for (set<size_t>::const_iterator i = src.finalStates.begin(); i != src.finalStates.end(); ++i)
				dst.addFinalState(index.translateOTF(*i) + offset);
		}
		return dst;
	}

public:
	
	// makes state numbers contiguous
	TA& reduced(TA<T>& dst, Index<size_t>& index) const {
		return reduce(dst, *this, index);
	}
	
	static TA<T>& disjointUnion(TA<T>& dst, const TA<T>& a, const TA<T>& b) {
		for (typename set<typename trans_cache_type::value_type*>::const_iterator i = a.transitions.begin(); i != a.transitions.end(); ++i)
			dst.addTransition(*i);
		for (typename set<typename trans_cache_type::value_type*>::const_iterator i = b.transitions.begin(); i != b.transitions.end(); ++i)
			dst.addTransition(*i);
		for (set<size_t>::const_iterator i = a.finalStates.begin(); i != a.finalStates.end(); ++i)
			dst.addFinalState(*i);
		for (set<size_t>::const_iterator i = b.finalStates.begin(); i != b.finalStates.end(); ++i)
			dst.addFinalState(*i);
		return dst;
	}

	static TA<T>& disjointUnion(TA<T>& dst, const TA<T>& src, bool addFinalStates = true) {
		for (typename set<typename trans_cache_type::value_type*>::const_iterator i = src.transitions.begin(); i != src.transitions.end(); ++i)
			dst.addTransition(*i);
		if (addFinalStates) {
			for (set<size_t>::const_iterator i = src.finalStates.begin(); i != src.finalStates.end(); ++i)
				dst.addFinalState(*i);
		}
		return dst;
	}

	static TA<T>& renamedUnion(TA<T>& dst, const TA<T>& a, const TA<T>& b, size_t& aSize) {
		Index<size_t> index;
		reduce(dst, a, index);
		aSize = index.size();
		index.clear();
		reduce(dst, b, index, aSize);
		return dst;
	}

	static TA<T>& renamedUnion(TA<T>& dst, const TA<T>& src, size_t offset, size_t& srcSize) {
		Index<size_t> index;
		reduce(dst, src, index, offset);
		srcSize = index.size();
		return dst;
	}

	TA<T>& unfoldAtRoot(TA<T>& dst, size_t newState) {
		// TODO: 
		return dst;
	}
	
	TA<T>& unfoldAtLeaf(TA<T>& dst, size_t selector) {
	}

};

template <class T>
class TAManager : Cache<TA<T>*>::Listener {
	
	mutable typename TA<T>::Backend& backend;
	
	Cache<TA<T>*> taCache;
	vector<TA<T>*> taPool;

protected:

	virtual void drop(TA<T>* x) {
		x->clear();
		this->taPool.push_back(x);
	}
	
public:

	TAManager(typename TA<T>::Backend& backend) : backend(backend) {}
	
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
	
	TA<T>* clone(TA<T>* src) {
		assert(src->backend == &this->backend);
		return this->taCache.lookup(new TA<T>(src))->first;
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
		for (typename vector<TA<T>*>::iterator i = this->taPool.begin(); i != this->taPool.end(); ++i)
			delete *i;
		this->taPool.clear();
	}
	
};

#endif
