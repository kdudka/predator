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

#ifndef CACHE_H
#define CACHE_H

// Standard library headers
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <unordered_map>

// Boost headers
#include <boost/functional/hash.hpp>

template <class T>
class Cache
{
public:   // data types

	typedef typename std::unordered_map<T, size_t, boost::hash<T>> store_type;
	typedef typename std::unordered_map<T, size_t, boost::hash<T>>::value_type value_type;

	struct Listener {
		virtual void drop(value_type* x) = 0;
		virtual ~Listener() {}
	};

private:  // data members

	store_type store;

	std::vector<Listener*> listeners;

public:

	Cache() :
		store{},
		listeners{}
	{ }

	void addListener(Listener* x)
	{
		this->listeners.push_back(x);
	}

	value_type* find(const T& x)
	{
		typename store_type::iterator i = this->store.find(x);
		return (i == this->store.end())?(nullptr):(&*i);
	}

	value_type* lookup(const T& x)
	{
		return this->addRef(&*this->store.insert(std::make_pair(x, 0)).first);
	}

	value_type* addRef(value_type* x)
	{
		return ++x->second, x;
	}

	size_t release(value_type* x)
	{
		if (x->second > 1)
			return --x->second;

		for (Listener* lsnr : this->listeners)
			lsnr->drop(x);

		this->store.erase(x->first);
		return 0;
	}

	void clear()
	{
		for (Listener* lsnr : this->listeners)
		{
			for (typename store_type::iterator j = this->store.begin(); j != this->store.end(); ++j)
				lsnr->drop(&*j);
		}
		this->store.clear();
	}

	bool empty() const
	{
		return this->store.empty();
	}
};

template <class T, class V>
class CachedBinaryOp {

public:

	typedef std::unordered_map<std::pair<T, T>, V, boost::hash<std::pair<T, T>>> store_type;
	typedef std::unordered_map<T, std::set<typename store_type::value_type*>, boost::hash<T>> store_map_type;

protected:

	store_type store;
	store_map_type storeMap;

	template <class F>
	void internalInvalidate(const T& x, const T& y, F f) {
		typename store_type::iterator i = this->store.find(make_pair(x, y));
		if (i == this->store.end())
			return;
		this->storeMap.find(x)->second.erase(&*i);
		this->storeMap.find(y)->second.erase(&*i);
		f(&*i);
		this->store.erase(i);
	}

	template <class F>
	void internalInvalidateKey(const T& x, F f) {
		typename store_map_type::iterator i = this->storeMap.find(x);
		if (i == this->storeMap.end())
			return;
		for (typename std::set<typename store_type::value_type*>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
			this->storeMap.find((x == (*j)->first.first)?((*j)->first.second):((*j)->first.first))->second.erase(*j);
			f(*j);
			this->store.erase((*j)->first);
		}
		this->storeMap.erase(i);
	}

	template <class F>
	typename store_type::value_type* internalLookup(const T& x, const T& y, F f) {
		assert(x != y);
		std::pair<typename store_type::iterator, bool> i = this->store.insert(std::make_pair(make_pair(x, y), V()));
		if (i.second) {
			f(x, y, &*i.first);
			this->storeMap.insert(std::make_pair(x, std::set<typename store_type::value_type*>())).first->second.insert(&*i.first);
			this->storeMap.insert(std::make_pair(y, std::set<typename store_type::value_type*>())).first->second.insert(&*i.first);
		}
		return &*i.first;
	}

	template <class F>
	struct EraseCallback {
		
		F f;
		
		EraseCallback(F f) : f(f) {}
		
		void operator()(typename store_type::value_type* x) {
			this->f(x->second);
		}

	};

	template <class F>
	struct InsertCallback {
		
		F f;
		
		InsertCallback(F f) : f(f) {}
		
		void operator()(const T& x, const T& y, typename store_type::value_type* r) {
			r->second = f(x, y);
		}

	};

public:

	CachedBinaryOp() :
		store{},
		storeMap{}
	{ }

	template <class F>
	void invalidate(const T& x, const T& y, F f) {
		this->internalInvalidate(x, y, EraseCallback<F>(f));
	}

	template <class F>
	void invalidateKey(const T& x, F f) {
		this->internalInvalidateKey(x, EraseCallback<F>(f));
	}

	template <class F>
	V lookup(const T& x, const T& y, F f) {
		return this->internalLookup(x, y, InsertCallback<F>(f))->second;
	}

};

template <class T, class V>
class CachedBinaryOpExt : public CachedBinaryOp<T, V> {

public:

	typedef typename CachedBinaryOp<T, V>::store_type store_type;
	typedef std::unordered_map<V, std::set<typename store_type::value_type*> > value_map_type;

protected:

	value_map_type valueMap;

	template <class F>
	struct EraseCallback {
		
		CachedBinaryOpExt& op;
		
		F f;
		
		EraseCallback(CachedBinaryOpExt& op, F f) : op(op), f(f) {}
		
		void operator()(typename store_type::value_type* x) {
			typename value_map_type::iterator i = this->op.valueMap.find(x->second);
			assert(i != this->op.valueMap.end());
			i->second.erase(x);
			if (i->second.empty())
				this->op.valueMap.erase(i);
			this->f(x->second);
		}

	};

	template <class F>
	struct InsertCallback {
		
		CachedBinaryOpExt& op;
		
		F f;
		
		InsertCallback(CachedBinaryOpExt& op, F f) : op(op), f(f) {}
		
		void operator()(const T& x, const T& y, typename store_type::value_type* r) {
			r->second = this->f(x, y);
			this->op.valueMap.insert(
				make_pair(r->second, std::set<typename store_type::value_type*>())
			).first->second.insert(r);
		}

	};


public:

	template <class F>
	void invalidate(const T& x, const T& y, F f) {
		this->internalInvalidate(x, y, EraseCallback<F>(*this, f));
	}

	template <class F>
	void invalidateKey(const T& x, F f) {
		this->internalInvalidateKey(x, EraseCallback<F>(*this, f));
	}

	template <class F>
	void invalidateValue(const V& x, F f) {
		typename value_map_type::iterator i = this->valueMap.find(x);
		if (i == this->valueMap.end())
			return;
		for (typename std::set<typename store_type::value_type*>::iterator j = i->second.begin(); j != i->second.end(); ++j)
			this->invalidate((*j)->first.first, (*j)->first.second, f);
		this->valueMap.erase(i);
	}

	template <class F>
	V lookup(const T& x, const T& y, F f) {
		return this->internalLookup(x, y, InsertCallback<F>(*this, f))->second;
	}

};
/*
template <class T, class V>
class CachedBinaryOpLRU : public CachedBinaryOp<T, std::pair<V, typename std::list<std::pair<T, T> >::iterator> > {

	typedef CachedBinaryOp<T, std::pair<V, typename std::list<std::pair<T, T> >::iterator> > parent_type;
	typedef typename parent_type::store_type store_type;
	typedef typename parent_type::store_map_type store_map_type;
	typedef std::list<std::pair<T, T> > lru_queue_type;

	lru_queue_type lruQueue;

public:

	template <class F>
	void invalidate(const T& x, F f) {
		typename store_map_type::iterator i = this->storeMap.find(x);
//		assert(i != this->storeMap.end());
		for (typename std::set<typename store_type::value_type*>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
			this->storeMap.find((x == (*j)->first.first)?((*j)->first.second):((*j)->first.first))->second.erase(*j);
			this->store.erase((*j)->first);
			this->lruQueue.erase((*j)->second.second);
			f((*j)->second.first);
		}
		this->storeMap.erase(i);
		
	}

	template <class F>
	void flush(size_t size, F f) {
		while (this->lruQueue.size() > size) {
			std::pair<T, T>& el = this->lruQueue.back();
			typename store_type::iterator i = this->store.find(el);
//			assert(i != this->store.end());
			this->storeMap.find(el.first)->second.erase(&*i);
			this->storeMap.find(el.second)->second.erase(&*i);
			this->store.erase(i);
			this->lruQueue.pop_back();
			f(i->second.first);
		}
	}

	template <class F>
	V lookup(const T& x, const T& y, F f) {
//		assert(x != y);
		pair<typename store_type::iterator, bool> i = this->store.insert(
			make_pair(std::make_pair(x, y), std::make_pair(V(), this->lruQueue.end()))
		);
		this->lruQueue.push_front(i.first->first);
		if (i.second) {
			i.first->second = std::make_pair(f(x, y), this->lruQueue.begin());
			this->storeMap.insert(make_pair(x, set<typename store_type::value_type*>())).first->second.insert(&*i.first);
			this->storeMap.insert(make_pair(y, set<typename store_type::value_type*>())).first->second.insert(&*i.first);
		} else {
			this->lruQueue.erase(i.first->second.second);
			i.first->second.second = this->lruQueue.begin();
		}
		return i.first->second.first;
	}
	
};
*/
#endif
