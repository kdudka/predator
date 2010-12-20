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

#ifndef UFAE_H
#define UFAE_H

#include <vector>
#include <ostream>

#include "utils.hh"
#include "treeaut.hh"
#include "labman.hh"
#include "forestautext.hh"

class UFAE {
	
	TA<label_type>& backend;
	
	size_t stateOffset;
	
	mutable LabMan& labMan;
	
public:

	UFAE(TA<label_type>& backend, LabMan& labMan) : backend(backend), stateOffset(1), labMan(labMan) {
		// let 0 be the only accepting state
		this->backend.addFinalState(0);
	}

	void clear() { this->stateOffset = 1; }
/*
	struct RenameNonleafF {

		Index<size_t>& index;

		size_t offset;
		
		RenameNonleafF(Index<size_t>& index, size_t offset = 0)
			: index(index), offset(offset) {}

		size_t operator()(size_t s) {
			if (_MSB_TEST(s))
				return s;
			return this->index.translateOTF(s) + this->offset;
		}

	};
*/
	TA<label_type>& fae2ta(TA<label_type>& dst, Index<size_t>& index, const FAE& src) const {
		dst.addFinalState(0);
		std::vector<size_t> lhs;
		for (std::vector<TA<label_type>*>::const_iterator i = src.roots.begin(); i != src.roots.end(); ++i) {
			TA<label_type>::rename(dst, **i, FAE::RenameNonleafF(index, this->stateOffset), false);
			lhs.push_back(index[(*i)->getFinalState()] + this->stateOffset);
		}
		dst.addTransition(lhs, &labMan.lookup(src.variables, lhs.size()), 0);
		return dst;
	}

	void join(const TA<label_type>& src, const Index<size_t>& index) {
		TA<label_type>::disjointUnion(this->backend, src, false);
		this->stateOffset += index.size();
	}

	void adjust(const Index<size_t>& index) {
		this->stateOffset += index.size();
	}
/*
	void ta2fae(vector<FAE*>& dst, TA<label_type>::Manager& taMan, LabMan& labMan, BoxManager& boxMan) const {
		TA<label_type>::td_cache_type cache;
		this->backend.buildTDCache(cache);
		vector<const TT<label_type>*>& v = cache.insert(make_pair(0, vector<const TT<label_type>*>())).first->second;
		// iterate over all "synthetic" transitions and constuct new FAE for each
		for (vector<const TT<label_type>*>::iterator i = v.begin(); i != v.end(); ++i) {
			FAE* fae = new FAE(taMan, labMan, boxMan);
			dst.push_back(fae);
			fae->loadTA(this->backend, cache, *i, this->stateOffset);
		}
	}
*/
	friend std::ostream& operator<<(std::ostream& os, const UFAE& ufae) {
		TAWriter<label_type>(os).writeOne(ufae.backend);
		return os;
	}

};

#endif
