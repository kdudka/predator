/*
 * Copyright (C) 2011 Jiri Simacek
 *
 * This file is part of forester.
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

#ifndef REVERSE_RUN_H
#define REVERSE_RUN_H

#include <vector>
#include <set>
#include <stdexcept>
#include <algorithm>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "forestautext.hh"

struct CustomIntersectF {

	FAE& fae;
	TA<label_type>& dst;
	const TA<label_type>& src1;
	const TA<label_type>& src2;

	CustomIntersectF(FAE& fae, TA<label_type>& dst, const TA<label_type>& src1, const TA<label_type>& src2)
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

class ReverseRun {

	FAE& fae;

public:

	bool reverse(const FAE& fae) {

		this->fae.variables = fae.variables;
		this->fae.rootMap = fae.rootMap;

		if (this->fae.roots.size() > fae.roots.size()) {
			this->fae.updateRoot(this->fae.roots.back(), NULL);
			this->fae.roots.pop_back();
		}

		assert(this->fae.roots.size() == fae.roots.size());

		FAE tmp(this->fae);
		for (std::vector<TA<label_type>*>::iterator i = this->fae.roots.begin(); i != this->fae.roots.end(); ++i)
			this->fae.updateRoot(*i, NULL);

		for (size_t i = 0; i < this->fae.roots.size(); ++i) {
			if (!tmp.roots[i]) {
				this->fae.roots[i] = this->fae.taMan->addRef(fae.roots[i]);
				continue;
			}
			TA<label_type>::lt_cache_type cache1, cache2;
			tmp.buildLTCacheExt(*tmp.roots[i], cache1);
			tmp.buildLTCacheExt(*fae.roots[i], cache2);

			this->fae.roots[i] = this->fae.taMan->alloc();
			
			size_t stateCount = TA<label_type>::buProduct(
				cache1,
				cache2,
				CustomIntersectF(this->fae, *this->fae.roots[i], *tmp.roots[i], *fae.roots[i]),
				this->fae.nextState()
			);

			this->fae.incrementStateOffset(stateCount);
				
			if (this->fae.roots[i]->getFinalStates().empty())
				return false;

			this->fae.updateRoot(this->fae.roots[i], &this->fae.roots[i]->unreachableFree(*this->fae.taMan->alloc()));

		}

		return true;

	}


public:

	ReverseRun(FAE& fae) : fae(fae) {}

};

#endif
