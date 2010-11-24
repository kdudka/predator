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

#ifndef ANTICHAIN_H
#define ANTICHAIN_H

#include <vector>
#include <list>
#include <set>
#include <map>
#include <functional>
#include <algorithm>

#include <boost/unordered_map.hpp>

#include "cache.hh"

using std::vector;
using std::list;
using std::set;
using std::map;
using std::multimap;
using std::pair;

using boost::unordered_map;
using boost::unordered_multimap;

class Antichain {

protected:

	typedef Cache<set<size_t> > state_cache_type;

	state_cache_type stateCache;

	CachedBinaryOp<state_cache_type::value_type*, bool> cachedLte;
	
/*
	typedef unordered_map<pair<state_cache_type::value_type*, state_cache_type::value_type*>, bool> lte_cache_type;
	typedef unordered_map<state_cache_type::value_type*, set<lte_cache_type::value_type*> > lte_cache_map_type;
*/
//	typedef multimap<size_t, state_cache_type::value_type*> antichain_type;
	typedef list<state_cache_type::value_type*> antichain_item_type;
	typedef unordered_map<size_t, antichain_item_type> antichain_type;

	const vector<vector<bool> >& rel; 
	
	vector<vector<size_t> > relIndex;
	vector<vector<size_t> > invRelIndex;

	struct Lte {
		
		Antichain& ac;
		
		Lte(Antichain& ac) : ac(ac) {}
		
		bool operator()(state_cache_type::value_type* x, state_cache_type::value_type* y) {
			for (set<size_t>::const_iterator i = x->first.begin(); i != x->first.end(); ++i) {
				if (!utils::checkIntersection(y->first, this->ac.relIndex[*i]))
					return false;
			}
			return true;
		}
/*
		bool operator()(Cache<set<size_t> >::value_type* x, Cache<set<size_t> >::value_type* y) {
			return utils::checkInclusion(x->first, y->first));
		}
*/
	};

	bool lte(state_cache_type::value_type* x, state_cache_type::value_type* y) {
		return (x == y)?(true):(this->cachedLte.lookup(x, y, Lte(*this)));
	}

	bool check(const multimap<size_t, state_cache_type::value_type*>& ac, const pair<size_t, state_cache_type::value_type*>& el) {
		for (vector<size_t>::const_iterator i = this->relIndex[el.first].begin(); i != this->relIndex[el.first].end(); ++i) {
			for (pair<multimap<size_t, state_cache_type::value_type*>::const_iterator, multimap<size_t, state_cache_type::value_type*>::const_iterator> p = ac.equal_range(*i); p.first != p.second; ++p.first) {
				if (this->lte(p.first->second, el.second))
					return true;
			}
				
		}
		return false;
	}

	bool check(const antichain_type& ac, const pair<size_t, state_cache_type::value_type*>& el) {
		for (vector<size_t>::const_iterator i = this->relIndex[el.first].begin(); i != this->relIndex[el.first].end(); ++i) {
			antichain_type::const_iterator j = ac.find(*i);
			if (j == ac.end())
				continue;
			for (antichain_item_type::const_iterator k = j->second.begin(); k != j->second.end(); ++k) {
				if (this->lte(*k, el.second))
					return true;
			}
				
		}
		return false;
	}

	bool check(const list<pair<size_t, state_cache_type::value_type*> >& ac, const pair<size_t, state_cache_type::value_type*>& el) {
		for (list<pair<size_t, state_cache_type::value_type*> >::const_iterator i = ac.begin(); i != ac.end(); ++i) {
			if (rel[el.first][i->first] && this->lte(i->second, el.second))
				return true;
		}
		return false;
	}

	void refine(multimap<size_t, state_cache_type::value_type*>& ac, const pair<const size_t, state_cache_type::value_type*>& el) {
		for (vector<size_t>::iterator i = this->invRelIndex[el.first].begin(); i != this->invRelIndex[el.first].end(); ++i) {
			for (pair<multimap<size_t, state_cache_type::value_type*>::iterator, multimap<size_t, state_cache_type::value_type*>::iterator> p = ac.equal_range(*i); p.first != p.second; ) {
				if (this->lte(el.second, p.first->second)) {
					this->stateCache.release(p.first->second);
					multimap<size_t, state_cache_type::value_type*>::iterator j = p.first++;
					ac.erase(j);
				} else {
					++p.first;
				}
			}
		}
	}

	void refine(antichain_type& ac, const pair<const size_t, state_cache_type::value_type*>& el) {
		for (vector<size_t>::iterator i = this->invRelIndex[el.first].begin(); i != this->invRelIndex[el.first].end(); ++i) {
			antichain_type::iterator j = ac.find(*i);
			if (j == ac.end())
				continue;
			for (antichain_item_type::iterator k = j->second.begin(); k != j->second.end(); ) {
				if (this->lte(el.second, *k)) {
					this->stateCache.release(*k);
					k = j->second.erase(k);
				} else ++k;
			}
			if (j->second.empty())
				ac.erase(j);
		}
	}

	void refine(list<pair<size_t, state_cache_type::value_type*> >& ac, const pair<size_t, state_cache_type::value_type*>& el) {
		for (list<pair<size_t, state_cache_type::value_type*> >::iterator i = ac.begin(); i != ac.end(); ) {
			if (rel[i->first][el.first] && this->lte(el.second, i->second)) {
				this->stateCache.release(i->second);
				i = ac.erase(i);
			} else ++i;
		}
	}

protected:

	struct StateCacheListener : public state_cache_type::Listener {
		
		struct NoOp { void operator()(bool) {} };
		
		Antichain& antichain;
		
		StateCacheListener(Antichain& antichain) : antichain(antichain) {
			this->antichain.stateCache.addListener(this);
		}
		
		virtual void drop(state_cache_type::value_type* x) {
			this->antichain.cachedLte.invalidateKey(x, NoOp());
		}
		
	};
	
	StateCacheListener stateCacheListener;
	
	antichain_type processed;
//	list<pair<size_t, state_cache_type::value_type*> > next;
	multimap<size_t, state_cache_type::value_type*> next;

public:

	Antichain(const vector<vector<bool> >& rel) : rel(rel), stateCacheListener(*this) {
		utils::relIndex(this->relIndex, rel);
		vector<vector<bool> > invRel;
		utils::relInv(invRel, rel);
		utils::relIndex(this->invRelIndex, invRel);
	}
	
	void initialize(const vector<pair<size_t, set<size_t> > >& post) {
		for (vector<pair<size_t, set<size_t> > >::const_iterator i = post.begin(); i != post.end(); ++i) {
			pair<size_t, state_cache_type::value_type*> p = make_pair(i->first, this->stateCache.lookup(i->second));
			// antichain acceleration
			if (this->check(this->next, p)) {
				this->stateCache.release(p.second);
				continue;
			}
			this->refine(this->next, p);
//			this->next.push_back(p);
			this->next.insert(p);
		}
	}

	void update(const vector<pair<size_t, set<size_t> > >& post) {
		for (vector<pair<size_t, set<size_t> > >::const_iterator i = post.begin(); i != post.end(); ++i) {
			pair<size_t, state_cache_type::value_type*> p = make_pair(i->first, this->stateCache.lookup(i->second));
			// antichain acceleration
			if (this->check(this->processed, p) || this->check(this->next, p)) {
				this->stateCache.release(p.second);
				continue;
			}
			this->refine(this->processed, p);
			this->refine(this->next, p);
//			this->next.push_back(p);
			this->next.insert(p);
		}
	}
/*
	bool nextElement(antichain_type::iterator& el) {
		if (this->next.empty())
			return false;
		el = this->processed.insert(this->next.front());
		this->next.pop_front();
		return true;
	}
*//*
	bool nextElement(pair<size_t, state_cache_type::value_type*>& el) {
		if (this->next.empty())
			return false;
		el = this->next.front();
		this->next.pop_front();
		this->processed.insert(make_pair(el.first, antichain_item_type())).first->second.push_back(el.second);
		return true;
	}
*/
	bool nextElement(pair<size_t, state_cache_type::value_type*>& el) {
		if (this->next.empty())
			return false;
		el = *this->next.begin();
		this->next.erase(this->next.begin());
		this->processed.insert(make_pair(el.first, antichain_item_type())).first->second.push_back(el.second);
		return true;
	}

	void printStatus() const {
		std::cout << "processed: " << this->processed.size() << ", next: " << this->next.size() << std::endl;
	}
/*
	class Response {
		
			friend class Antichain;

			antichain_type::const_iterator b;
			antichain_type::const_iterator e;
			antichain_type::const_iterator i;
			
		protected:
		
			Response(antichain_type::const_iterator b, antichain_type::const_iterator e) : b(b), e(e), i(b) {}
		
		public:
		
			bool next() {
				if (++this->i != this->e)
					return true;
				this->i = this->b;
				return false;
			}
			
			pair<size_t, const set<size_t>*> current() const {
				return make_pair(this->i->first, &this->i->second->first);
			}
		
	};

	bool getResponse(size_t s, Response& r) {
		pair<antichain_type::iterator, antichain_type::iterator> p = this->processed.equal_range(s);
		if (p.first == p.second)
			return false;
		r = Antichain::Response(p.first, p.second);
		return true;
	}
*/
};

#endif
