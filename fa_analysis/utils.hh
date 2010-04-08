#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <map>
#include <ostream>

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

template <class T1, class T2 = size_t>
struct Index {
	
	std::map<T1, T2> map;
	
	typedef typename std::map<T1, T2>::const_iterator iterator;
	
	typename Index<T1, T2>::iterator begin() const {
		return this->map.begin();
	}
	
	typename Index<T1, T2>::iterator end() const {
		return this->map.end();
	}

	void clear() {
		this->map.clear();
	}

	size_t get(const T1& x) {
		return this->map.insert(std::pair<T1, T2>(x, this->map.size())).first->second;
	}

	bool add(const T1& x) {
		return this->map.insert(std::pair<T1, T2>(x, this->map.size())).second;
	}
	
	size_t size() const {
		return this->map.size();
	}
	
	T2 translate(const T1& x) const {
		typename std::map<T1, T2>::const_iterator i = this->map.find(x);
		if (i == this->map.end())
			throw std::runtime_error("Indexer::translate() : lookup failed");
		return i->second;
	}
	
	T2 translateOTF(const T1& x) {
		return this->map.insert(std::pair<T1, T2>(x, this->map.size())).first->second;
	}

	T2 operator[](const T1& x) const {
		return this->translate(x);
	}

	void translate(std::vector<T2>& dst, const std::vector<T1>& src, size_t offset = 0) const {
		dst.clear();
		for (typename std::vector<T1>::const_iterator i = src.begin(); i != src.end(); ++i)
			dst.push_back(this->translate(*i) + offset);
	}	

	void translateOTF(std::vector<T2>& dst, const std::vector<T1>& src, size_t offset = 0) {
		dst.clear();
		for (typename std::vector<T1>::const_iterator i = src.begin(); i != src.end(); ++i)
			dst.push_back(this->translateOTF(*i) + offset);
	}	

};

template <class T>
struct FullIndex : public Index<T> {
	
	std::vector<T> index;

	void clear() {
		Index<T>::clear();
		this->index.clear();
	}

	int add(const T& x) {
		std::pair<typename std::map<T, size_t>::iterator, bool> y = this->map.insert(std::pair<T, size_t>(x, this->map.size()));
		if (y.second)
			this->index.push_back(x);
		return y.first->second;
	}
	
};

class utils {
	
public:
	
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
	static void set_intersection(std::vector<T>& dst, const std::vector<std::vector<T> >& src) {
		boost::unordered_map<T, size_t> m;
		for (typename std::vector<std::vector<T> >::const_iterator i = src.begin(); i != src.end(); ++i) {
			for (typename std::vector<T>::const_iterator j = i->begin(); j != i->end(); ++j)
				++m.insert(make_pair(*j, 0)).first->second;
		}
		dst.clear();
		for (typename boost::unordered_map<T, size_t>::iterator i = m.begin(); i != m.end(); ++i) {
			if (i->second == src.size())
				dst.push_back(i->first);
		}
	}

	template <class T>
	static void set_intersection(std::set<T>& dst, const std::set<T>& src1, const std::set<T>& src2) {
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
	static void set_union(std::vector<T>& dst, const std::vector<std::vector<T> >& src) {
		boost::unordered_set<T> m;
		for (typename std::vector<std::vector<T> >::const_iterator i = src.begin(); i != src.end(); ++i) {
			for (typename std::vector<T>::const_iterator j = i->begin(); j != i->end(); ++j)
				m.insert(*j);
		}
		dst.assign(m.begin(), m.end());
	}

	// duplicates removal
	template <class T>
	static void unique(std::vector<T>& dst, const std::vector<T>& src) {
		boost::unordered_set<T> s;
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
		os << '[';
		typename T::const_iterator i = container.begin();
		if (i != container.end()) {
			os << *i;
			for (++i; i != container.end(); ++i)
				os << ',' << *i;
		}
		return os << ']';
	}

};

#endif
