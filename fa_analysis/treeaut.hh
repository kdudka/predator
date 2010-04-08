#ifndef TA_H
#define TA_H

#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <cassert>

#include <boost/unordered_map.hpp>

#include "cache.hh"
#include "utils.hh"
#include "timreader.hh"
#include "timwriter.hh"
#include "lts.hh"

using std::vector;
using std::set;
using std::map;
using std::pair;
using std::make_pair;

using boost::hash_value;
using boost::unordered_map;

struct SLIndex {

	FullIndex<size_t> states;
	FullIndex<size_t> labels;
	
	void clear() {
		this->states.clear();
		this->labels.clear();
	}

};

class TT {

	friend class TA;

public:

	typedef Cache<vector<size_t> > lhs_cache_type;

private:

	mutable lhs_cache_type& lhsCache;

public:

	lhs_cache_type::value_type* _lhs;
	size_t _label;
	size_t _rhs;

	TT(const TT& transition)
		: lhsCache(transition.lhsCache) {
		this->_lhs = this->lhsCache.lookup(transition._lhs->first);
		this->_label = transition._label;
		this->_rhs = transition._rhs;
	}

	TT(const vector<size_t>& lhs, size_t label, size_t rhs, lhs_cache_type& lhsCache)
		: lhsCache(lhsCache) {
		this->_lhs = this->lhsCache.lookup(lhs);
		this->_label = label;
		this->_rhs = rhs;
	}
	
	TT(const std::vector<size_t>& lhs, size_t label, size_t rhs, const vector<size_t>& index, lhs_cache_type& lhsCache)
		: lhsCache(lhsCache) {
		vector<size_t> tmp(lhs.size());
		for (size_t i = 0; i < lhs.size(); ++i)
			tmp[i] = index[lhs[i]];
		this->_lhs = this->lhsCache.lookup(tmp);
		this->_label = label;
		this->_rhs = index[rhs];
	}

	TT(const TT& transition, lhs_cache_type& lhsCache)
		: lhsCache(lhsCache) {
		this->_lhs = this->lhsCache.lookup(transition._lhs->first);
		this->_label = transition._label;
		this->_rhs = transition._rhs;
	}
  
	TT(const TT& transition, const vector<size_t>& index, lhs_cache_type& lhsCache)
		: lhsCache(lhsCache) {
		vector<size_t> tmp(transition._lhs->first.size());
		for (size_t i = 0; i < transition._lhs->first.size(); ++i)
			tmp[i] = index[transition._lhs->first[i]];
		this->_lhs = this->lhsCache.lookup(tmp);
		this->_label = transition._label;
		this->_rhs = index[transition._rhs];
	}

	~TT() { this->lhsCache.release(this->_lhs);	}

	const vector<size_t>& lhs() const { return this->_lhs->first; }
	size_t label() const { return this->_label; }
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

class TA {

public:

    typedef Cache<TT> trans_cache_type;

	struct TACache {

		TT::lhs_cache_type lhsCache;
		trans_cache_type transCache;

	};

	static TACache defaultTACache;

	mutable TACache* taCache;
	
	trans_cache_type& transCache() const { return this->taCache->transCache; }

	class Iterator {
		set<trans_cache_type::value_type*>::const_iterator _i;
	public:
		Iterator(set<trans_cache_type::value_type*>::const_iterator i) : _i(i) {}

		Iterator& operator++() {
			return ++this->_i, *this;
		}

		Iterator operator++(int) {
			return Iterator(this->_i++);
		}
		
		const std::vector<size_t>& lhs() {
			return (*this->_i)->first._lhs->first;
		}
		
		int label() {
			return (*this->_i)->first._label;
		}

		int rhs() {
			return (*this->_i)->first._rhs;
		}

		bool operator==(const Iterator& rhs) {
			return this->_i == rhs._i;
		}

		bool operator!=(const Iterator& rhs) {
			return this->_i != rhs._i;
		}

	};

	class Reader : public TimbukReader {

		TA* dst;
		string name;

	protected:
	
		virtual void newLabel(const string& name, size_t arity, size_t id) {}
		
		virtual void beginModel(const string& name) {
			this->dst->clear();
			this->name = name;
		}
		
		virtual void newState(const string& name, size_t id) {}
		
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
		
		TA& run(TA& dst) {
			this->dst = &dst;
			this->run_simple();
			return dst;
		}

	};

	class MultiReader : public TimbukReader {

		TACache& cache;

	public:

		vector<TA> automata;
		vector<string> names;

	protected:
	
		virtual void newLabel(const string& name, size_t arity, size_t id) {}
		
		virtual void beginModel(const string& name) {
			this->automata.push_back(TA(this->cache));
			this->names.push_back(name);
		}
		
		virtual void newState(const string& name, size_t id) {}
		
		virtual void newFinalState(size_t id) {
			this->automata.back().addFinalState(id);
		}
		
		virtual void endDeclaration() {}

		virtual void newTransition(const vector<size_t>& lhs, size_t label, size_t rhs) {
			this->automata.back().addTransition(lhs, label, rhs);
		}
		
		virtual void endModel() {}
		
	public:

		MultiReader(std::istream& input, const string& name = "", TACache& cache = defaultTACache)
			: TimbukReader(input, name), cache(cache) {}

		void clear() {
			this->automata.clear();
			this->names.clear();
		}

		void run() {
			this->run_main();
		}

	};

public:

	typedef Iterator iterator;

public:

//	int labels;
	size_t next_state;
	size_t maxRank;

	set<trans_cache_type::value_type*> transitions;
	set<size_t> finalStates;
	
//	std::map<const std::vector<int>*, int> lhsMap;

public:

	static TA& fromStream(TA& dst, std::istream& input = std::cin, const string& name = "") {
		return TA::Reader(input, name).run(dst);
	}
	
	static TA& fromStream(TA& dst, TA::Reader& reader) {
		return reader.run(dst);
	}

	void toStream(std::ostream& output, const string& name = "TreeAutomaton") {
		map<size_t, size_t> labels;
		set<size_t> states;
		for (set<trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			labels.insert(make_pair((*i)->first._label, (*i)->first._lhs->first.size()));
			states.insert((*i)->first._rhs);
			for (size_t j = 0; j < (*i)->first._lhs->first.size(); ++j)
				states.insert((*i)->first._lhs->first[j]);
		}
		TimbukWriter writer(output);
		writer.startAlphabet();
		for (map<size_t, size_t>::iterator i = labels.begin(); i != labels.end(); ++i)
			writer.writeLabel(i->first, i->second);
		writer.newModel(name);
		writer.startStates();
		for (set<size_t>::iterator i = states.begin(); i != states.end(); ++i)
			writer.writeState(*i);
		writer.startFinalStates();
		for (set<size_t>::iterator i = this->finalStates.begin(); i != this->finalStates.end(); ++i)
			writer.writeState(*i);
		writer.startTransitions();
		for (set<trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			writer.writeTransition((*i)->first._lhs->first, (*i)->first._label, (*i)->first._rhs);
		writer.terminate();
	}

	TA(TACache& taCache = defaultTACache) : taCache(&taCache), next_state(0), maxRank(0) {}
	
	TA(const TA& ta) : taCache(ta.taCache), next_state(ta.next_state), maxRank(ta.maxRank), transitions(ta.transitions), finalStates(ta.finalStates) {
		for (std::set<trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			this->transCache().addRef(*i);
	}
	
	~TA() {
		this->clear();
	}
	
	TA& operator=(const TA& rhs) {
		this->clear();
		this->next_state = rhs.next_state;
		this->maxRank = rhs.maxRank;
		this->taCache = rhs.taCache;
		this->transitions = rhs.transitions;
		this->finalStates = rhs.finalStates;
		for (set<trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			this->transCache().addRef(*i);
		return *this;
	}

	void clear() {
		this->maxRank = 0;
		this->next_state = 0;
		for (set<trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			this->transCache().release(*i);
		this->transitions.clear();
		this->finalStates.clear();
	}
	
	size_t newState() {
		return this->next_state++;
	}
	
	void updateStateCounter() {
		this->next_state = 0;
		for (set<trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			this->next_state = std::max(this->next_state, 1 + std::max((*i)->first._rhs, *std::max_element((*i)->first._lhs->first.begin(), (*i)->first._lhs->first.end())));
	}
	
	void buildStateIndex(Index<size_t>& index) const {
		index.clear();
		for (set<trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			for (vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j)
				index.add(*j);
			index.add((*i)->first._rhs);
		}
	}

	void buildTransIndex(Index<size_t>& index) const {
		index.clear();
		for (set<trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			index.add((*i)->first._label);
	}

	void buildIndex(SLIndex& index) const {
		index.clear();
		for (set<trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
			for (vector<size_t>::const_iterator j = (*i)->first._lhs->first.begin(); j != (*i)->first._lhs->first.end(); ++j)
				index.states.add(*j);
			index.labels.add((*i)->first._label);
			index.states.add((*i)->first._rhs);
		}
	}

	void buildLhsIndex(Index<const vector<size_t>*>& index) const {
		index.clear();
		for (set<trans_cache_type::value_type*>::iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			index.add(&(*i)->first._lhs->first);
	}
	
	void addTransition(const vector<size_t>& lhs, size_t label, size_t rhs) {
		if (lhs.size() > this->maxRank)
			this->maxRank = lhs.size();
		trans_cache_type::value_type* x = this->transCache().lookup(TT(lhs, label, rhs, this->taCache->lhsCache));
		if (!this->transitions.insert(x).second)
			this->transCache().release(x);
	}
	
	void addTransition(const vector<size_t>& lhs, size_t label, size_t rhs, const vector<size_t>& index) {
		if (lhs.size() > this->maxRank)
			this->maxRank = lhs.size();
		trans_cache_type::value_type* x = this->transCache().lookup(TT(lhs, label, rhs, index, this->taCache->lhsCache));
		if (!this->transitions.insert(x).second)
			this->transCache().release(x);
	}
/* !! uncommenting this will screw things up
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
	void addTransition(const trans_cache_type::value_type* transition) {
		if (transition->first.lhs().size() > this->maxRank)
			this->maxRank = transition->first.lhs().size();
		trans_cache_type::value_type* x = this->transCache().lookup(TT(transition->first, this->taCache->lhsCache));
		if (!this->transitions.insert(x).second)
			this->transCache().release(x);
	}

	void addTransition(const trans_cache_type::value_type* transition, const vector<size_t>& index) {
		if (transition->first.lhs().size() > this->maxRank)
			this->maxRank = transition->first.lhs().size();
		trans_cache_type::value_type* x = this->transCache().lookup(TT(transition->first, index, this->taCache->lhsCache));
		if (!this->transitions.insert(x).second)
			this->transCache().release(x);
	}

	void addFinalState(size_t state) {
		this->finalStates.insert(state);
	}
	
	bool isFinalState(size_t state) const {
		return (this->finalStates.find(state) != this->finalStates.end());
	}
	
	const set<trans_cache_type::value_type*>& getTransitions() const { return this->transitions; }

	void downwardTranslation(LTS& lts, const SLIndex& slIndex) const;
	
	void downwardSimulation(vector<vector<bool> >& rel, const Index<size_t>& stateIndex) const;
	
	void upwardTranslation(LTS& lts, vector<vector<size_t> >& part, vector<vector<bool> >& rel, const SLIndex& slIndex, const vector<vector<bool> >& sim) const;

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
	
	// collapses states according a given relation
	TA& collapsed(TA& dst, const vector<vector<bool> >& rel, const Index<size_t>& stateIndex) const {
		vector<size_t> index, head;
		utils::relBuildClasses(rel, index, head);
		dst.clear();
		for (set<trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
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
	
	TA& uselessFree(TA& dst) const {
		vector<trans_cache_type::value_type*> v1(this->transitions.begin(), this->transitions.end()), v2;
		set<size_t> states;
		dst.clear();
		bool changed = true;
		while (changed) {
			changed = false;
			for (vector<trans_cache_type::value_type*>::const_iterator i = v1.begin(); i != v1.end(); ++i) {
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
	
	TA& unreachableFree(TA& dst) const {
		vector<trans_cache_type::value_type*> v1(transitions.begin(), this->transitions.end()), v2;
		set<size_t> states(this->finalStates.begin(), this->finalStates.end());
		dst.clear();
		bool changed = true;
		while (changed) {
			changed = false;
			for (vector<trans_cache_type::value_type*>::const_iterator i = v1.begin(); i != v1.end(); ++i) {
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
	
	TA& minimized(TA& dst, const vector<vector<bool> >& cons, const Index<size_t>& stateIndex) const {
		TA::TACache taCache;
		vector<vector<bool> > dwn;
		this->downwardSimulation(dwn, stateIndex);
		utils::relAnd(dwn, cons, dwn);
		TA tmp1(taCache), tmp2(taCache);
		return this->collapsed(tmp1, dwn, stateIndex).uselessFree(tmp2).unreachableFree(dst);
	}
	
	TA& minimized(TA& dst) const {
		Index<size_t> stateIndex;
		this->buildStateIndex(stateIndex);
		vector<vector<bool> > cons(stateIndex.size(), vector<bool>(stateIndex.size(), true));
		return this->minimized(dst, cons, stateIndex);
	}
	
	static bool subseteq(const TA& a, const TA& b);

protected:
	
	static TA& reduce(TA& dst, const TA& src, Index<size_t>& index, size_t offset = 0) {
		vector<size_t> lhs;
		for (set<trans_cache_type::value_type*>::const_iterator i = src.transitions.begin(); i != src.transitions.end(); ++i) {
			lhs.clear();
			index.translateOTF(lhs, (*i)->first._lhs->first, offset);
			dst.addTransition(lhs, (*i)->first._label, index.translateOTF((*i)->first._rhs) + offset);
		}
		for (set<size_t>::const_iterator i = src.finalStates.begin(); i != src.finalStates.end(); ++i)
			dst.addFinalState(index.translateOTF(*i) + offset);
		return dst;
	}

public:
	
	// makes state numbers contiguous
	TA& reduced(TA& dst, Index<size_t>& index) const {
		return reduce(dst, *this, index);
	}
	
	TA& uniteDisjoint(TA& dst, const TA& a) const {
		for (set<trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
			dst.addTransition(*i);
		for (set<trans_cache_type::value_type*>::const_iterator i = a.transitions.begin(); i != a.transitions.end(); ++i)
			dst.addTransition(*i);
		for (set<size_t>::const_iterator i = this->finalStates.begin(); i != this->finalStates.end(); ++i)
			dst.addFinalState(*i);
		for (set<size_t>::const_iterator i = a.finalStates.begin(); i != a.finalStates.end(); ++i)
			dst.addFinalState(*i);
		return dst;
	}

	TA& uniteRenamed(TA& dst, const TA& a, size_t& stateCount) const {
		dst.clear();
		Index<size_t> index;
		reduce(dst, *this, index);
		stateCount = index.size();
		index.clear();
		reduce(dst, a, index, stateCount);
		return dst;
	}

	TA::Iterator begin() const { return TA::Iterator(this->transitions.begin()); }
	TA::Iterator end() const { return TA::Iterator(this->transitions.end()); }
	
};

#endif
