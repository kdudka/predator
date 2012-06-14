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

#ifndef NORMALIZATION_H
#define NORMALIZATION_H

#include <vector>
#include <map>

#include "forestautext.hh"
#include "abstractbox.hh"
#include "utils.hh"

class Normalization {

	FAE& fae;

protected:

	TreeAut* mergeRoot(TreeAut& dst, size_t ref, TreeAut& src, std::vector<size_t>& joinStates) {
		assert(ref < this->fae.roots.size());
		TreeAut* ta = this->fae.allocTA();
		ta->addFinalStates(dst.getFinalStates());
		size_t refState = _MSB_ADD(this->fae.boxMan->getDataId(Data::createRef(ref)));
		std::unordered_map<size_t, size_t> joinStatesMap;
		for (std::set<size_t>::const_iterator i = src.getFinalStates().begin(); i != src.getFinalStates().end(); ++i) {
			joinStates.push_back(this->fae.nextState());
			joinStatesMap.insert(std::make_pair(*i, this->fae.freshState()));
		}
		bool hit = false;
		for (TreeAut::iterator i = dst.begin(); i != dst.end(); ++i) {
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

		for (const Data& var : this->fae.GetVariables()) {

			// skip everything what is not a root reference
			if (!var.isRef())
				continue;

			size_t root = var.d_ref.root;

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

		for (const Data& var : this->fae.GetVariables()) {

			// skip everything what is not a root reference
			if (!var.isRef())
				continue;

			size_t root = var.d_ref.root;

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

	void normalizeRoot(std::vector<bool>& normalized, size_t root, const std::vector<bool>& marked) {

		if (normalized[root])
			return;

		normalized[root] = true;

		// we need a copy here!
		auto signature = this->fae.connectionGraph.data[root].signature;

		for (auto& cutpoint : signature) {

			this->normalizeRoot(normalized, cutpoint.root, marked);

			if (marked[cutpoint.root])
				continue;

			assert(root != cutpoint.root);

			std::vector<size_t> refStates;

			TreeAut* ta = this->mergeRoot(
				*this->fae.roots[root],
				cutpoint.root,
				*this->fae.roots[cutpoint.root],
				refStates
			);

			this->fae.roots[root] = std::shared_ptr<TreeAut>(ta);
			this->fae.roots[cutpoint.root] = nullptr;

			this->fae.connectionGraph.mergeCutpoint(root, cutpoint.root);

		}

	}

	bool selfReachable(size_t root, size_t self, const std::vector<bool>& marked) {

		for (auto& cutpoint : this->fae.connectionGraph.data[root].signature) {

			if (cutpoint.root == self)
				return true;

			if (marked[cutpoint.root])
				continue;

			if (this->selfReachable(cutpoint.root, self, marked))
				return true;

		}

		return false;

	}

	void scan(std::vector<bool>& marked, std::vector<size_t>& order, const std::set<size_t>& forbidden = std::set<size_t>(), bool extended = false) {

		assert(this->fae.connectionGraph.isValid());

		std::vector<bool> visited(this->fae.roots.size(), false);

		marked = std::vector<bool>(this->fae.roots.size(), false);

		order.clear();

		// compute canonical root ordering
		this->traverse(visited, order, marked);

		// check garbage
		this->checkGarbage(visited);

		if (!extended) {

			for (auto& x : forbidden)
				marked[x] = true;

			return;

		}

		for (auto& x : forbidden) {

			marked[x] = true;

			for (auto& cutpoint : this->fae.connectionGraph.data[x].signature) {

				if ((cutpoint.root != x) && !this->selfReachable(cutpoint.root, x, marked))
					continue;

				marked[cutpoint.root] = true;

				break;

			}

		}

	}

	// normalize representation
	bool normalize(const std::vector<bool>& marked, const std::vector<size_t>& order) {

		bool merged = false;

		size_t i;

		for (i = 0; i < order.size(); ++i) {

			if (!marked[i] || (order[i] != i))
				break;

		}

		if (i == order.size()) {

			this->fae.roots.resize(order.size());
			this->fae.connectionGraph.data.resize(order.size());
			return false;

		}

		// reindex roots
		std::vector<size_t> index(this->fae.roots.size(), static_cast<size_t>(-1));
		std::vector<bool> normalized(this->fae.roots.size(), false);
		std::vector<std::shared_ptr<TreeAut>> newRoots;
		size_t offset = 0;

		for (auto& i : order) {

			this->normalizeRoot(normalized, i, marked);
//			assert(marked[*i] || (this->fae.roots[*i] == nullptr));

			if (!marked[i]) {

				merged = true;

				continue;

			}

			newRoots.push_back(this->fae.roots[i]);

			index[i] = offset++;

		}

		// update representation
		std::swap(this->fae.roots, newRoots);

		for (size_t i = 0; i < this->fae.roots.size(); ++i) {

			this->fae.roots[i] = std::shared_ptr<TreeAut>(
				this->fae.relabelReferences(this->fae.roots[i].get(), index)
			);

		}

		this->fae.connectionGraph.finishNormalization(this->fae.roots.size(), index);

		// update variables
		this->fae.UpdateVarsRootRefs(index);

		return merged;
	}

public:

	Normalization(FAE& fae) : fae(fae) {}

};

#endif
