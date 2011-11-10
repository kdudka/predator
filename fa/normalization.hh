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

#ifndef NORMALIZATION_H
#define NORMALIZATION_H

#include <vector>
#include <map>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "treeaut.hh"
#include "forestautext.hh"
#include "abstractbox.hh"
#include "utils.hh"

class Normalization {

	FAE& fae;

protected:

	TA<label_type>* mergeRoot(TA<label_type>& dst, size_t ref, TA<label_type>& src, std::vector<size_t>& joinStates) {
		assert(ref < this->fae.roots.size());
		TA<label_type>* ta = this->fae.allocTA();
		ta->addFinalStates(dst.getFinalStates());
		size_t refState = _MSB_ADD(this->fae.boxMan->getDataId(Data::createRef(ref)));
		boost::unordered_map<size_t, size_t> joinStatesMap;
		for (std::set<size_t>::const_iterator i = src.getFinalStates().begin(); i != src.getFinalStates().end(); ++i) {
			joinStates.push_back(this->fae.nextState());
			joinStatesMap.insert(std::make_pair(*i, this->fae.freshState()));
		}
		bool hit = false;
		for (TA<label_type>::iterator i = dst.begin(); i != dst.end(); ++i) {
			std::vector<size_t> tmp = i->lhs();
			std::vector<size_t>::iterator j = std::find(tmp.begin(), tmp.end(), refState);
			if (j != tmp.end()) {
				for (std::vector<size_t>::iterator k = joinStates.begin(); k != joinStates.end(); ++k) {
					*j = *k;
					ta->addTransition(tmp, i->label(), i->rhs());
				}
				hit = true;
			} else ta->addTransition(*i);
		}
//		std::cerr << joinState << std::endl;
		if (!hit) {assert(false);}
		// avoid screwing up things
		src.unfoldAtRoot(*ta, joinStatesMap, false);
		return ta;
	}

	void traverse(std::vector<bool>& visited, std::vector<size_t>& order, std::vector<bool>& marked) const {

		visited = std::vector<bool>(this->fae.roots.size(), false);
		marked = std::vector<bool>(this->fae.roots.size(), false);

		order.clear();

		for (std::vector<Data>::const_iterator i = this->fae.variables.begin(); i != this->fae.variables.end(); ++i) {

			// skip everything what is not a root reference
			if (!i->isRef())
				continue;

			size_t root = i->d_ref.root;

			// mark rootpoint pointed by a variable
			marked[root] = true;

			// check whether we traversed this one before
			if (visited[root])
				continue;

			this->fae.connectionGraph.visit(root, visited, order, marked);

		}

	}

	void traverse(std::vector<bool>& visited) const {

		visited = std::vector<bool>(this->fae.roots.size(), false);

		for (std::vector<Data>::const_iterator i = this->fae.variables.begin(); i != this->fae.variables.end(); ++i) {

			// skip everything what is not a root reference
			if (!i->isRef())
				continue;

			size_t root = i->d_ref.root;

			// check whether we traversed this one before
			if (visited[root])
				continue;

			this->fae.connectionGraph.visit(root, visited);

		}

	}

	void checkGarbage(const std::vector<bool>& visited) const {

		bool garbage = false;

		for (size_t i = 0; i < this->fae.roots.size(); ++i) {

			if (!this->fae.roots[i])
				continue;

			if (!visited[i]) {

				CL_CDEBUG(1, "the root " << i << " is not referenced anymore ... " << this->fae.connectionGraph.data[i]);

				garbage = true;

			}

		}

		if (garbage)
			throw ProgramError("garbage detected");

	}

public:

	// check garbage
	void check() const {

		// compute reachable roots
		std::vector<bool> visited(this->fae.roots.size(), false);

		this->traverse(visited);

		// check garbage
		this->checkGarbage(visited);

	}

	void normalizeRoot(std::vector<bool>& normalized, size_t root, std::vector<bool>& marked) {

		if (normalized[root])
			return;

		normalized[root] = true;

		auto tmp = this->fae.connectionGraph.data[root].signature;

		for (auto& cutpoint : tmp) {

			this->normalizeRoot(normalized, cutpoint.root, marked);

			if (marked[cutpoint.root])
				continue;
/*
			if (!this->fae.connectionGraph.isMergable(root, cutpoint.root)) {

				marked[cutpoint.root] = true;

				continue;

			}
*/
			std::vector<size_t> refStates;

			TA<label_type>* ta = this->mergeRoot(
				*this->fae.roots[root],
				cutpoint.root,
				*this->fae.roots[cutpoint.root],
				refStates
			);

			this->fae.roots[root] = std::shared_ptr<TA<label_type>>(ta);
			this->fae.roots[cutpoint.root] = nullptr;

			this->fae.connectionGraph.mergeCutpoint(root, cutpoint.root);

		}

	}

	void scan(std::vector<bool>& marked, std::vector<size_t>& order, const std::set<size_t>& forbidden = std::set<size_t>()) {

		std::vector<bool> visited(this->fae.roots.size(), false);

		marked = std::vector<bool>(this->fae.roots.size(), false);

		order.clear();

		this->fae.connectionGraph.updateIfNeeded(this->fae.roots);

		// compute canonical root ordering
		this->traverse(visited, order, marked);

		// check garbage
		this->checkGarbage(visited);

		// prevent merging of forbidden roots
		for (auto i = forbidden.begin(); i != forbidden.end(); ++i)
			marked[*i] = true;

	}

	// normalize representation
	void normalize(std::vector<bool>& marked, const std::vector<size_t>& order) {

		size_t i;

		for (i = 0; i < order.size(); ++i) {

			if (!marked[i] || (order[i] != i))
				break;

		}

		if (i == order.size()) {

			this->fae.roots.resize(order.size());
			this->fae.connectionGraph.data.resize(order.size());

			return;

		}

		// reindex roots
		std::vector<size_t> index(this->fae.roots.size(), (size_t)(-1));
		std::vector<bool> normalized(this->fae.roots.size(), false);
		std::vector<std::shared_ptr<TA<label_type>>> newRoots;
		size_t offset = 0;

		for (auto& i : order) {

			this->normalizeRoot(normalized, i, marked);
//			assert(marked[*i] || (this->fae.roots[*i] == NULL));

			if (!marked[i])
				continue;

			newRoots.push_back(this->fae.roots[i]);

			index[i] = offset++;

		}

		// update representation
		std::swap(this->fae.roots, newRoots);

		for (size_t i = 0; i < this->fae.roots.size(); ++i)
			this->fae.roots[i] = std::shared_ptr<TA<label_type>>(
				this->fae.relabelReferences(this->fae.roots[i].get(), index)
			);

		this->fae.connectionGraph.finishNormalization(this->fae.roots.size(), index);

		// update variables
		for (std::vector<Data>::iterator i = this->fae.variables.begin(); i != this->fae.variables.end(); ++i) {

			if (!i->isRef())
				continue;

			assert(index[i->d_ref.root] != (size_t)(-1));

			i->d_ref.root = index[i->d_ref.root];

		}

	}

public:

	Normalization(FAE& fae) : fae(fae) {}

};

#endif
