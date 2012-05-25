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

#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <set>
#include <ostream>

#include <unordered_set>
#include <unordered_map>

#ifndef NDEBUG
#define CL_CDEBUG(l, x) CL_DEBUG_AT(l, x)
#define CL_CDEBUG_MSG(l, x) CL_DEBUG_MSG_AT(l, x)
#else
#define CL_CDEBUG(l, x)
#define CL_CDEBUG_MSG(l, x)
#endif

template <class T>
std::vector<T> itov(const T &item) {
	std::vector<T> vec;
	vec.push_back(item);
	return vec;
}

template <class T>
struct Index {

	typedef typename std::unordered_map<T, size_t> map_type;
	
	map_type map;

	typedef typename map_type::const_iterator iterator;

	/**
	 * @brief  Default constructor
	 */
	Index() :
		map{}
	{ }

	typename Index<T>::iterator begin() const {
		return this->map.begin();
	}
	
	typename Index<T>::iterator end() const {
		return this->map.end();
	}

	void clear() {
		this->map.clear();
	}

	void set(const T& x, size_t v) {
		if (!this->map.insert(std::make_pair(x, v)).second)
			throw std::runtime_error("Index::set() : value already exists");
	}

	size_t get(const T& x, size_t offset = 0) {
		return this->map.insert(std::make_pair(x, this->map.size() + offset)).first->second;
	}

	bool add(const T& x, size_t offset = 0) {
		return this->map.insert(std::make_pair(x, this->map.size() + offset)).second;
	}
	
	size_t size() const {
		return this->map.size();
	}
	
	std::pair<size_t, bool> find(const T& x) const {
		typename map_type::const_iterator i = this->map.find(x);
		if (i == this->map.end())
			return std::make_pair(0, false);
		return std::make_pair(i->second, true);
	}

	size_t translate(const T& x) const {
		typename map_type::const_iterator i = this->map.find(x);
		if (i == this->map.end())
			throw std::runtime_error("Index::translate() : lookup failed");
		return i->second;
	}
	
	// HACK: better keep this static (not virtual)
	size_t translateOTF(const T& x) {
		return this->map.insert(std::make_pair(x, this->map.size())).first->second;
	}

	size_t operator[](const T& x) const {
		return this->translate(x);
	}

	void translate(std::vector<size_t>& dst, const std::vector<T>& src, size_t offset = 0) const {
		dst.clear();
		for (typename std::vector<T>::const_iterator i = src.begin(); i != src.end(); ++i)
			dst.push_back(this->translate(*i) + offset);
	}	

	void translateOTF(std::vector<size_t>& dst, const std::vector<T>& src, size_t offset = 0) {
		dst.clear();
		for (typename std::vector<T>::const_iterator i = src.begin(); i != src.end(); ++i)
			dst.push_back(this->translateOTF(*i) + offset);
	}	

	friend std::ostream& operator<<(std::ostream& os, const Index<T>& x) {
		os << '[';
		for (typename map_type::const_iterator i = x.begin(); i != x.end(); ++i)
			os << '(' << i->first << ',' << i->second << ')';
		return os << ']';
	}

};

template <class T>
struct FullIndex : public Index<T> {
	
	std::vector<T> index;

	void clear() {
		Index<T>::clear();
		this->index.clear();
	}

	size_t add(const T& x) {
		std::pair<typename Index<T>::map_type::iterator, bool> y = this->map.insert(std::make_pair(x, this->map.size()));
		if (y.second)
			this->index.push_back(x);
		return y.first->second;
	}

	size_t translateOTF(const T& x) {
		std::pair<typename Index<T>::map_type::iterator, bool> y = this->map.insert(std::make_pair(x, this->map.size()));
		if (y.second)
			this->index.push_back(x);
		return y.first->second;
	}

	void translateOTF(std::vector<size_t>& dst, const std::vector<T>& src, size_t offset = 0) {
		dst.clear();
		for (typename std::vector<T>::const_iterator i = src.begin(); i != src.end(); ++i)
			dst.push_back(this->translateOTF(*i) + offset);
	}	

};

template <class T>
class Guard {
	T* obj;
public:
	Guard(T* obj) : obj(obj) {}
	~Guard() {
		if (this->obj)
			delete this->obj;
	}
	void release() {
		this->obj = nullptr;
	}
};

template <class T>
class ContainerGuard {
	T* _cont;

private:  // methods

	ContainerGuard(const ContainerGuard&);
	ContainerGuard& operator=(const ContainerGuard&);

public:
	ContainerGuard(T& cont) : _cont(&cont) {}
	~ContainerGuard() {
		if (this->_cont) {
			for (typename T::iterator i = this->_cont->begin(); i != this->_cont->end(); ++i) {
				if (*i)
					delete *i;
			}
			this->_cont->clear();
		}
	}
	void release() {
		this->_cont = nullptr;
	}
};

template <class T>
struct ContWrapper {

	const T& cont_;
	ContWrapper(const T& c) : cont_(c) {}

	friend std::ostream& operator<<(std::ostream& os, const ContWrapper& w) {
		os << '{';
		auto i = w.cont_.begin();
		if (i != w.cont_.end()) {
			os << *i;
			for (++i; i != w.cont_.end(); ++i)
				os << ',' << *i;
		}
		return os << '}';
	}
};

class utils {
	
public:
	
	// build equivalence classes
	static void relBuildClasses(const std::vector<std::vector<bool> >& rel, std::vector<size_t>& headIndex) {
		headIndex.resize(rel.size());
		std::vector<size_t> head;
		for (size_t i = 0; i < rel.size(); ++i) {
			bool found = false;
			for (size_t j = 0; j < head.size(); ++j) {
				if (rel[i][head[j]] && rel[head[j]][i]) {
					headIndex[i] = head[j];
					found = true;
					break;
				}
			}
			if (!found) {
				headIndex[i] = i;
				head.push_back(i);
			}
		}
	}

	// build equivalence classes
	static void relBuildClasses(const std::vector<std::vector<bool> >& rel, std::vector<size_t>& index, std::vector<size_t>& head) {
		index.resize(rel.size());
		head.clear();
		for (size_t i = 0; i < rel.size(); ++i) {
			bool found = false;
			for (size_t j = 0; j < head.size(); ++j) {
				if (rel[i][head[j]] && rel[head[j]][i]) {
					index[i] = j;
					found = true;
					break;
				}
			}
			if (!found) {
				index[i] = head.size();
				head.push_back(i);
			}
		}
	}

	// and composition
	static void relAnd(std::vector<std::vector<bool> >& dst, const std::vector<std::vector<bool> >& src1, const std::vector<std::vector<bool> >& src2) {
		dst.resize(src1.size(), std::vector<bool>(src1.size()));
		for (size_t i = 0; i < src1.size(); ++i) {
			for (size_t j = 0; j < src1.size(); ++j)
				dst[i][j] = src1[i][j] && src2[i][j];
		}
	}
	
	// transposition
	static void relInv(std::vector<std::vector<bool> >& dst, const std::vector<std::vector<bool> >& src) {
		dst.resize(src.size(), std::vector<bool>(src.size()));
		for (size_t i = 0; i < src.size(); ++i) {
			for (size_t j = 0; j < src.size(); ++j)
				dst[j][i] = src[i][j];
		}
	}

	// relation index
	static void relIndex(std::vector<std::vector<size_t> >& dst, const std::vector<std::vector<bool> >& src) {
		dst.resize(src.size());
		for (size_t i = 0; i < src.size(); ++i) {
			for (size_t j = 0; j < src.size(); ++j) {
				if (src[i][j])
					dst[i].push_back(j);					
			}
		}
	}

	// intersection	
	template <class T1, class T2>
  	static bool checkIntersection(const T1& x, const T2& y) {
		typename T1::const_iterator f1 = x.begin(), l1 = x.end();
		typename T2::const_iterator f2 = y.begin(), l2 = y.end();
		while ((f1 != l1) && (f2 != l2)) {
			if (*f1 == *f2)
				return true;
			(*f1 < *f2)?(++f1,0):(++f2,0);
		}
		return false;
	}
	
	template <class T>
	static void setIntersection(std::vector<T>& dst, const std::vector<std::vector<T> >& src) {
		std::unordered_map<T, size_t> m;
		for (typename std::vector<std::vector<T> >::const_iterator i = src.begin(); i != src.end(); ++i) {
			for (typename std::vector<T>::const_iterator j = i->begin(); j != i->end(); ++j)
				++m.insert(std::make_pair(*j, 0)).first->second;
		}
		dst.clear();
		for (typename std::unordered_map<T, size_t>::iterator i = m.begin(); i != m.end(); ++i) {
			if (i->second == src.size())
				dst.push_back(i->first);
		}
	}

	template <class T>
	static void setIntersection(std::set<T>& dst, const std::set<T>& src1, const std::set<T>& src2) {
		typename std::set<T>::const_iterator i1 = src1.begin(), i2 = src2.begin();
		dst.clear();
		while (i1 != src1.end() && i2 != src2.end()) {
			if (*i1 == *i2) {
				dst.insert(*i1);
				++i1; ++i2;
			} else (*i1 < *i2)?(++i1):(++i2);
		}
	}

	// union
	template <class T>
	static void setUnion(std::vector<T>& dst, const std::vector<std::vector<T> >& src) {
		std::unordered_set<T> m;
		for (typename std::vector<std::vector<T> >::const_iterator i = src.begin(); i != src.end(); ++i) {
			for (typename std::vector<T>::const_iterator j = i->begin(); j != i->end(); ++j)
				m.insert(*j);
		}
		dst.assign(m.begin(), m.end());
	}

	// duplicates removal
	template <class T>
	static void unique(std::vector<T>& dst, const std::vector<T>& src) {
		std::unordered_set<T> s;
		dst.clear();
		for (typename std::vector<T>::const_iterator i = src.begin(); i != src.end(); ++i) {
			if (s.insert(*i).second)
				dst.push_back(*i);
		}
	}

	// inclusion	
	template <class T1, class T2>
  	static bool checkInclusion(const T1& x, const T2& y) {
		typename T1::const_iterator f1 = x.begin(), l1 = x.end();
		typename T2::const_iterator f2 = y.begin(), l2 = y.end();
		for ( ; (f1 != l1) && (f2 != l2); ++f2) {
			if (*f1 < *f2)
				return false;
			if (*f1 == *f2)
				++f1; 
		}
		return true;
	}

	// print
	static std::ostream& relPrint(std::ostream& os, const std::vector<std::vector<bool> >& src) {
		for (size_t i = 0; i < src.size(); ++i) {
			for (size_t j = 0; j < src.size(); ++j)
				os << src[i][j];
			os << std::endl;
		}
		return os;
	}

	template <class T>
	static std::ostream& printCont(std::ostream& os, const T& container) {
		os << '{';
		typename T::const_iterator i = container.begin();
		if (i != container.end()) {
			os << *i;
			for (++i; i != container.end(); ++i)
				os << ',' << *i;
		}
		return os << '}';
	}

	template <class T, class F>
	static void iterate(T& x, F f) {
		for (typename T::iterator i = x.begin(); i != x.end(); ++i)
			f(*i);
	}

	template <class T>
	static void erase(T& x) {
		for (typename T::iterator i = x.begin(); i != x.end(); ++i)
			delete *i;
		x.clear();
	}

	template <class T>
	static void eraseMapFirst(T& x) {
		for (typename T::iterator i = x.begin(); i != x.end(); ++i)
			delete i->first;
		x.clear();
	}

	template <class T>
	static void eraseMap(T& x) {
		for (typename T::iterator i = x.begin(); i != x.end(); ++i)
			delete i->second;
		x.clear();
	}

	template <class T>
	static ContWrapper<T> wrap(T& x) {
		return ContWrapper<T>(x);
	}

};

#endif
