
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <ostream>

#include <boost/unordered_map.hpp>

#include "treeaut.hh"
#include "simalg.hh"
#include "antichainext.hh"

using std::vector;
using std::set;
using std::map;
using std::multimap;
using std::pair;

using boost::unordered_set;
using boost::unordered_map;

TA::TACache TA::defaultTACache;

struct LhsEnv {

	size_t index;
	vector<size_t> data;

	LhsEnv(const vector<size_t>& lhs, size_t index) : index(index) {
		this->data.insert(this->data.end(), lhs.begin(), lhs.begin() + index);
		this->data.insert(this->data.end(), lhs.begin() + index + 1, lhs.end());
	}
	
	static set<LhsEnv>::iterator get(const vector<size_t>& lhs, size_t index, set<LhsEnv>& s) {
		return s.insert(LhsEnv(lhs, index)).first;
	}

	static set<LhsEnv>::iterator find(const vector<size_t>& lhs, size_t index, set<LhsEnv>& s) {
		set<LhsEnv>::iterator x = s.find(LhsEnv(lhs, index));
		if (x == s.end())
			throw std::runtime_error("LhsEnv::find - lookup failed");
		return x;
	}

	static bool sim(const LhsEnv& e1, const LhsEnv& e2, const vector<vector<bool> >& sim) {
		if ((e1.index != e2.index) || (e1.data.size() != e2.data.size()))
			return false;
		for (size_t i = 0; i < e1.data.size(); ++i) {
			if (!sim[e1.data[i]][e2.data[i]])
				return false;
		}
		return true;
	}

	static bool eq(const LhsEnv& e1, const LhsEnv& e2, const vector<vector<bool> >& sim) {
		if ((e1.index != e2.index) || (e1.data.size() != e2.data.size()))
			return false;
		for (size_t i = 0; i < e1.data.size(); ++i) {
			if (!sim[e1.data[i]][e2.data[i]] || !sim[e2.data[i]][e1.data[i]])
				return false;
		}
		return true;
	}

	bool operator<(const LhsEnv& rhs) const {
		return (this->index < rhs.index) || (
			(this->index == rhs.index) && (
				this->data < rhs.data
			)
		);
	}

	bool operator==(const LhsEnv& rhs) const {
		return (this->index == rhs.index) && (this->data == rhs.data);	
	}
/*
	bool operator!=(const LhsEnv& rhs) const {
		std::cout << "!=\n";
		return (this->index != rhs.index) || (this->data != rhs.data);	
	}
*/
	friend std::ostream& operator<<(std::ostream& os, const LhsEnv& env) {
		os << "[" << env.data.size() << "]";
		os << env.index << "|";
		for (vector<size_t>::const_iterator i = env.data.begin(); i != env.data.end(); ++i)
			os << "|" << *i;
		return os;
	}

};

struct Env {

	set<LhsEnv>::iterator lhs;
	size_t label;
	size_t rhs;

	Env(set<LhsEnv>::iterator lhs, size_t label, size_t rhs) : lhs(lhs), label(label), rhs(rhs) {}
	
	static map<Env, size_t>::iterator get(set<LhsEnv>::const_iterator lhs, size_t label, size_t rhs, map<Env, size_t>& m) {
		return m.insert(pair<Env, size_t>(Env(lhs, label, rhs), m.size())).first;
	}
	
	static map<Env, size_t>::iterator get(set<LhsEnv>::const_iterator lhs, size_t label, size_t rhs, map<Env, size_t>& m, bool& inserted) {
		pair<map<Env, size_t>::iterator, bool> x = m.insert(pair<Env, size_t>(Env(lhs, label, rhs), m.size()));
		inserted = x.second;
		return x.first;
	}

	static map<Env, size_t>::iterator find(set<LhsEnv>::const_iterator lhs, size_t label, size_t rhs, map<Env, size_t>& m) {
		map<Env, size_t>::iterator x = m.find(Env(lhs, label, rhs));
		if (x == m.end())
			throw std::runtime_error("Env::find - lookup failed");
		return x;
	}

	static bool sim(const Env& e1, const Env& e2, const vector<vector<bool> >& sim) {
		return (e1.label == e2.label) && LhsEnv::sim(*e1.lhs, *e2.lhs, sim);
	}
	
	static bool eq(const Env& e1, const Env& e2, const vector<vector<bool> >& sim) {
		return (e1.label == e2.label) && LhsEnv::eq(*e1.lhs, *e2.lhs, sim);
	}

	bool operator<(const Env& rhs) const {
		return (this->label < rhs.label) || (
			(this->label == rhs.label) && (
				(*this->lhs < *rhs.lhs) || (
					(*this->lhs == *rhs.lhs) && (
						this->rhs < rhs.rhs
					)
				)
			)
		);
	}

};

void TA::downwardTranslation(LTS& lts, const SLIndex& index) const {
	// build an index of non-translated left-hand sides
	Index<const vector<size_t>*> lhs;
	this->buildLhsIndex(lhs);
	lts = LTS(index.labels.size() + this->maxRank, index.states.size() + lhs.size());
	for (Index<const vector<size_t>*>::iterator i = lhs.begin(); i != lhs.end(); ++i) {
		for (size_t j = 0; j < i->first->size(); ++j)
			lts.addTransition(index.states.size() + i->second, index.labels.size() + j, index.states[(*i->first)[j]]);
	}
	for (std::set<trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i)
		lts.addTransition(index.states[(*i)->first._rhs], index.labels[(*i)->first._label], index.states.size() + lhs[&((*i)->first._lhs->first)]);
}

void TA::downwardSimulation(vector<vector<bool> >& rel, const Index<size_t>& stateIndex) const {
	LTS lts;
	SLIndex slIndex;
	slIndex.states.map = stateIndex.map;
	this->buildTransIndex(slIndex.labels);
	this->downwardTranslation(lts, slIndex);
	OLRTAlgorithm alg(lts);
	alg.init();
	alg.run();
	alg.buildRel(stateIndex.size(), rel);
}

void TA::upwardTranslation(LTS& lts, vector<vector<size_t> >& part, vector<vector<bool> >& rel, const SLIndex& slIndex, const vector<vector<bool> >& sim) const {
	set<LhsEnv> lhsEnvSet;
	map<Env, size_t> envMap;
	vector<const Env*> head;
	part.clear();
	for (std::set<trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
		vector<size_t> lhs;
		slIndex.states.translate(lhs, (*i)->first._lhs->first);
		size_t label = slIndex.labels[(*i)->first._label];
		size_t rhs = slIndex.states[(*i)->first._rhs];
		for (size_t j = 0; j < lhs.size(); ++j) {
			// insert required items into lhsEnv and lhsMap and build equivalence classes
			bool inserted;
			map<Env, size_t>::const_iterator env = Env::get(LhsEnv::get(lhs, j, lhsEnvSet), label, rhs, envMap, inserted);
			if (inserted) {
				inserted = false;
				for (size_t k = 0; k < head.size(); ++k) {
					if (Env::eq(*head[k], env->first, sim)) {
						part[k].push_back(env->second + slIndex.states.size());
						inserted = true;
						break;
					}
				}
				if (!inserted) {
					head.push_back(&env->first);
					part.push_back(std::vector<size_t>(1, env->second + slIndex.states.size()));
				}
			}
		}
	}
	lts = LTS(slIndex.labels.size() + 1, slIndex.states.size() + envMap.size());
	for (set<trans_cache_type::value_type*>::const_iterator i = this->transitions.begin(); i != this->transitions.end(); ++i) {
		vector<size_t> lhs;
		slIndex.states.translate(lhs, (*i)->first._lhs->first);
		size_t label = slIndex.labels[(*i)->first._label];
		size_t rhs = slIndex.states[(*i)->first._rhs];
		for (size_t j = 0; j < lhs.size(); ++j) {
			// find particular env
			map<Env, size_t>::iterator env = Env::find(LhsEnv::find(lhs, j, lhsEnvSet), label, rhs, envMap);
			lts.addTransition(lhs[j], slIndex.labels.size(), env->second);
			lts.addTransition(env->second, label, rhs);
		}
	}
	rel = vector<vector<bool> >(part.size() + 2, vector<bool>(part.size() + 2, false));
	// 0 non-accepting, 1 accepting, 2 .. environments
	rel[0][0] = true;
	rel[0][1] = true;
	rel[1][1] = true;
	for (size_t i = 0; i < head.size(); ++i) {
		for (size_t j = 0; j < head.size(); ++j) {
			if (Env::sim(*head[i], *head[j], sim))
				rel[i + 2][j + 2] = true;
		}
	}
}

void TA::upwardSimulation(vector<vector<bool> >& rel, const Index<size_t>& stateIndex, const vector<vector<bool> >& param) const {
	LTS lts;
	SLIndex slIndex;
	slIndex.states.map = stateIndex.map;
	this->buildTransIndex(slIndex.labels);
	std::vector<std::vector<size_t> > part;
	std::vector<std::vector<bool> > initRel;
	this->upwardTranslation(lts, part, initRel, slIndex, param);
	OLRTAlgorithm alg(lts);
	// accepting states to block 1
	std::vector<size_t> finalStates;
	slIndex.states.translate(finalStates, vector<size_t>(this->finalStates.begin(), this->finalStates.end())); 
	alg.fakeSplit(finalStates);
	// environments to blocks 2, 3, ...
	for (size_t i = 0; i < part.size(); ++i)
		alg.fakeSplit(part[i]);
	alg.getRelation().load(initRel);
	alg.init();
	alg.run();
	alg.buildRel(stateIndex.size(), rel);
}
/*
inline void computeDwn(vector<vector<bool> >& dst, const TA& a, const SLIndex& slIndex) {
	LTS lts;
	a.downwardTranslation(lts, slIndex);
	OLRTAlgorithm alg(lts);
	alg.init();
	alg.run();
	alg.buildRel(slIndex.states.size(), dst);
}

inline void computeUp(vector<vector<bool> >& dst, const TA& a, const SLIndex& slIndex, const vector<vector<bool> >& sim) {
	LTS lts;
	std::vector<std::vector<size_t> > part;
	std::vector<std::vector<bool> > rel;
	a.upwardTranslation(lts, part, rel, slIndex, sim);
	OLRTAlgorithm alg(lts);
	// accepting states to block 1
	std::vector<size_t> finalStates;
	slIndex.states.translate(finalStates, vector<size_t>(a.finalStates.begin(), a.finalStates.end())); 
	alg.fakeSplit(finalStates);
	// environments to blocks 2, 3, ...
	for (size_t i = 0; i < part.size(); ++i)
		alg.fakeSplit(part[i]);
	alg.getRelation().load(rel);
	alg.init();
	alg.run();
	alg.buildRel(slIndex.states.size(), dst);
}

inline void computeComb(vector<vector<bool> >& dst, const vector<vector<bool> >& dwn, const vector<vector<bool> >& up, size_t size) {
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
*//*
TA& TA::minimized(TA& dst, const vector<vector<bool> >& cons, const SLIndex& slIndex) const {
	TA::TACache taCache;
	// compute downward simulation
//	std::cout << "downward" << std::endl;
	vector<vector<bool> > dwnsim;
	computeDwn(dwnsim, *this, slIndex);
	utils::relAnd(dwnsim, cons, dwnsim);
	TA min1(taCache);
	this->collapsed(min1, dwnsim, slIndex);
// TODO: optimize reindexing
	// build new index
	SLIndex slIndex2;
	min1.buildIndex(slIndex2);
	// resize cons and dwnsim
	vector<size_t> iCache(slIndex2.states.size());
	for (size_t i = 0; i < slIndex2.states.size(); ++i)
		iCache[i] = slIndex.states[slIndex2.states.index[i]];
	vector<vector<bool> > cons2(slIndex2.states.size(), vector<bool>(slIndex2.states.size()));
	vector<vector<bool> > dwnsim2(slIndex2.states.size(), vector<bool>(slIndex2.states.size()));
	for (size_t i = 0; i < slIndex2.states.size(); ++i) {
		size_t ii = slIndex.states[slIndex2.states.index[i]];
		for (size_t j = 0; j < slIndex2.states.size(); ++j) {
			cons2[i][j] = cons[ii][iCache[j]];
			dwnsim2[i][j] = dwnsim[ii][iCache[j]];
		}
	}
	// compute upward simulation
//	std::cout << "upward" << std::endl;
	vector<vector<bool> > upsim;
	computeUp(upsim, min1, slIndex2, dwnsim2);
//	std::cout << "combined" << std::endl;
	// compute combined simulation
	vector<vector<bool> > comb;
	computeComb(comb, dwnsim2, upsim, slIndex2.states.size());
	utils::relAnd(comb, cons2, comb);
	TA tmp1(taCache), tmp2(taCache);
	return min1.collapsed(tmp1, comb, slIndex2).uselessFree(tmp2).unreachableFree(dst);
}*/
/*
inline void printEl(size_t x, const set<size_t>& y) {
	std::cout << '(' << x << ", ";
	utils::printCont(std::cout, y);
	std::cout << ")\n";
}
*/

bool TA::subseteq(const TA& a, const TA& b) {
//	std::cout << "TA::subseteq()\n";
	return AntichainExt::subseteq(a, b);
}

