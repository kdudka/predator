/*
 * Copyright (C) 2011 Jiri Simacek
 *
 * This file is part of forester.
 *
 * forester is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * forester is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with forester.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef REVERSE_RUN_H
#define REVERSE_RUN_H

#include <vector>
#include <set>
#include <stdexcept>
#include <algorithm>

#include "forestautext.hh"
#include "abstractbox.hh"

struct CustomIntersectF {

	FAE& fae;
	TreeAut& dst;
	const TreeAut& src1;
	const TreeAut& src2;

	CustomIntersectF(FAE& fae, TreeAut& dst, const TreeAut& src1, const TreeAut& src2)
	 : fae(fae), dst(dst), src1(src1), src2(src2) {}

	void operator()(const TT<label_type>* t1, const TT<label_type>* t2, const std::vector<size_t>& lhs, size_t& rhs) {
		const Data* data;
		if (this->fae.isData(t2->rhs(), data))
			rhs = t2->rhs();
		this->dst.addTransition(lhs, t2->_label, rhs);
		if (this->src1.isFinalState(t1->rhs()) && this->src2.isFinalState(t2->rhs()))
			this->dst.addFinalState(rhs);
	}

};
/*
struct IntersectionIndex {

	boost::unordered_map<std::pair<size_t, size_t>, size_t>& stateMap;

	size_t getState(size_t& s, size_t s1, size_t s2) {
		return this->stateMap.insert(std::make_pair(std::make_pair(s1, s2), this->stateMap.size())).first->second;
	}

	bool addState(size_t s1, size_t s2) {
		return this->stateMap.insert(std::make_pair(std::make_pair(s1, s2), this->stateMap.size())).second;
	}

};
*/
enum class intersect_result_e { iFail, iSuccess, iUnfold1, iUnfold2 };

struct IntersectInfo {
	Index<std::pair<size_t, size_t> > index;
	intersect_result_e result;
	size_t root;
	size_t state;
	const AbstractBox* box;
};

/**
 * @brief  Obsolete?
 *
 * @todo remove?
 */
class ReverseRun {

	FAE& fae;

public:

	bool reverse(const FAE& fae) {

		this->fae.variables = fae.variables;
		this->fae.rootMap = fae.rootMap;

		if (this->fae.roots.size() > fae.roots.size())
			this->fae.roots.pop_back();

		assert(this->fae.roots.size() == fae.roots.size());

		FAE tmp(this->fae);
//		for (std::vector<TreeAut*>::iterator i = this->fae.roots.begin(); i != this->fae.roots.end(); ++i)
//			this->fae.updateRoot(*i, nullptr);

		for (size_t i = 0; i < this->fae.roots.size(); ++i) {
			if (!tmp.roots[i]) {
				this->fae.roots[i] = fae.roots[i];
				continue;
			}
			TreeAut::lt_cache_type cache1, cache2;
			tmp.buildLTCacheExt(*tmp.roots[i], cache1);
			tmp.buildLTCacheExt(*fae.roots[i], cache2);

			this->fae.roots[i] = std::shared_ptr<TreeAut>(this->fae.allocTA());

			size_t stateCount = TreeAut::buProduct(
				cache1,
				cache2,
				CustomIntersectF(this->fae, *this->fae.roots[i], *tmp.roots[i], *fae.roots[i]),
				this->fae.nextState()
			);

			this->fae.incrementStateOffset(stateCount);

			if (this->fae.roots[i]->getFinalStates().empty())
				return false;

			this->fae.roots[i] = std::shared_ptr<TreeAut>(&this->fae.roots[i]->unreachableFree(*this->fae.allocTA()));

		}

		return true;

	}
/*
	template <class F>
	static void intersection(IntersectInfo& info, TreeAut& dst, TreeAut& src1, TreeAut& src2, F f) {
		const std::set<size_t>& s1 = src1.getFinalStates();
		const std::set<size_t>& s2 = src2.getFinalStates();
		std::vector<std::pair<size_t, size_t> > stack;
		for (std::set<size_t>::const_iterator i = s1.begin(); i != s1.end(); ++i) {
			for (std::set<size_t>::const_iterator j = s2.begin(); j != s12.end(); ++j) {
				if (info.index.add(std::make_pair(*i, *j))) {
					stack.push_back(std::make_pair(*i, *j));
					dst.addFinalState(info.index.getState(std::make_pair(*i, *j)));
				}
			}
		}
		while (!stack.empty()) {
			std::pair<size_t, size_t> s = stack.back();
			stack.pop_back();
			for (TreeAut::iterator i = src1.begin(s.first), i != src1.end(s.first); ++i) {
				for (TreeAut::iterator j = src2.begin(s.second), j != src2.end(s.second); ++j) {
					f(info, i->label(), j->label());
					switch (info.result) {
						case iFail: break;
						case iSuccess: {
							assert(i->lhs().size() == i->lhs().size());
							std::vector<size_t> lhs;
							for (size_t k = 0; k < i->lhs().size(); ++k) {
								if (info.index.add(std::make_pair(i->lhs()[k], j->lhs()[k])))
									stack.push_back(std::make_pair(i->lhs()[k], j->lhs()[k]));
								lhs.push_back(info.index.get(std::make_pair(i->lhs()[k], j->lhs()[k])));
							}
							dst.addTransition(lhs, i->label(), info.index.get(s));
							break;
						}
						case iUnfold1:
							info.state = s.first;
							return iUnfold1;
						case iUnfold2:
							info.state = s.second;
							return iUnfold2;
					}
				}
			}
		}
	}

	template <class F>
	static bool intersection(IntersectInfo& info, FAE& dst, const FAE& src1, const FAE& src2, F f) {

	}
*/
public:

	ReverseRun(FAE& fae) : fae(fae) {}

};

#endif
