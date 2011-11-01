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

#ifndef EXPLORER_H
#define EXPLORER_H

#include <vector>
#include <set>
#include <stdexcept>
#include <algorithm>

#include "forestautext.hh"
#include "abstractbox.hh"
#include "boxman.hh"

class Explorer {

	FAE& fae;
	BoxMan& boxMan;

protected:

	static bool sigFind(const FA::RootSignature& signature, size_t what) {

		return std::find_if(
			signature.begin(),
			signature.end(),
			[&what](const FA::RootInfo& p) { return p.root == what; }
		) != signature.end();

	}

	static void copyBox(std::vector<size_t>& lhs, std::vector<const AbstractBox*>& label,
		const AbstractBox* box, const std::vector<size_t>& srcLhs, size_t& srcOffset) {

		for (size_t i = 0; i < box->getArity(); ++i, ++srcOffset)
			lhs.push_back(srcLhs[srcOffset]);

		label.push_back(box);

	}

	FA::RootSignature boxCut(TA<label_type>& dst, TA<label_type>& complement, size_t root, size_t target,
		const FA::o_map_type& signatures) {

		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);

		const TA<label_type>& src = *this->fae.roots[root];

		FA::RootSignature cutpoints, tmp;

		for (auto k = src.getFinalStates().begin(); k != src.getFinalStates().end(); ++k) {

			for (auto i = src.begin(*k); i != src.end(*k, i); ++i) {

				std::vector<size_t> lhs, cLhs;

				std::vector<const AbstractBox*> label, cLabel;

				size_t lhsOffset = 0;

				const TT<label_type>& t = *i;

				tmp.clear();

				for (auto& box : t.label()->getNode()) {

					bool found = false;

					for (size_t j = 0; j < box->getArity(); ++j) {

						assert(lhsOffset + j < t.lhs().size());

						auto iter = signatures.find(t.lhs()[lhsOffset + j]);

						if (iter == signatures.end())
							continue;

						if (!found && !Explorer::sigFind(iter->second, target))
							continue;

						found = true;

						tmp.insert(tmp.end(), iter->second.begin(), iter->second.end());

						FA::removeMultOcc(tmp);

					}

					if (found) {

						Explorer::copyBox(cLhs, cLabel, box, t.lhs(), lhsOffset);

					} else {

						Explorer::copyBox(lhs, label, box, t.lhs(), lhsOffset);

					}

				}

				if (cutpoints.empty())
					cutpoints = tmp;

				assert(cutpoints == tmp);

				assert(label.size());
				FAE::reorderBoxes(label, lhs);
				dst.addTransition(lhs, this->fae.boxMan->lookupLabel(label), *k);

				assert(cLabel.size());
				FAE::reorderBoxes(cLabel, cLhs);
				complement.addTransition(cLhs, this->fae.boxMan->lookupLabel(cLabel), this->fae.nextState());

			}

		}

		dst.addFinalStates(src.getFinalStates());
		complement.addFinalState(this->fae.nextState());
		src.copyTransitions(dst, TA<label_type>::NonAcceptingF(src));
		src.copyTransitions(complement, TA<label_type>::NonAcceptingF(src));

		return cutpoints;

	}

	std::pair<std::shared_ptr<TA<label_type>>, FA::RootSignature> separateCutpoint(size_t root,
		size_t cutpoint, std::vector<size_t>& index) {

		assert(root < this->fae.roots.size());
		assert(cutpoint < this->fae.roots.size());

		index.resize(this->fae.roots.size());

		size_t i = 0;
		for (auto& rootInfo : this->fae.rootMap[root])
			index[rootInfo.root] = i++;

		FA::o_map_type signatures;

		FA::computeDownwardO(*this->fae.roots[root], signatures);

		auto ta = std::shared_ptr<TA<label_type>>(this->fae.allocTA());
		auto tmp = std::shared_ptr<TA<label_type>>(this->fae.allocTA());

		FA::RootSignature signature = this->boxCut(*ta, *tmp, root, cutpoint, signatures);

		this->fae.roots[root] = ta;

		auto tmp2 = std::shared_ptr<TA<label_type>>(this->fae.allocTA());

		this->fae.relabelReferences(*tmp2, *tmp, index);

		tmp->clear();
		tmp2->unreachableFree(*tmp);

		return std::make_pair(tmp, signature);

	}

	std::shared_ptr<TA<label_type>> separateAux(size_t root, size_t cutpoint,
		const std::vector<size_t>& index) {

		assert(root < this->fae.roots.size());
		assert(cutpoint < this->fae.roots.size());

		FA::o_map_type signatures;

		FA::computeDownwardO(*this->fae.roots[root], signatures);

		auto ta = std::shared_ptr<TA<label_type>>(this->fae.allocTA());
		auto tmp = std::shared_ptr<TA<label_type>>(this->fae.allocTA());

		this->boxCut(*ta, *tmp, root, cutpoint, signatures);

		this->fae.roots[root] = ta;
		this->fae.updateRootMap(root);

		auto tmp2 = std::shared_ptr<TA<label_type>>(this->fae.allocTA());

		this->fae.relabelReferences(*tmp2, *tmp, index);

		tmp->clear();
		tmp2->unreachableFree(*tmp);

		return tmp;

	}

	void joinBox(size_t root, size_t aux, const Box* box, const FA::RootSignature& signature) {

		assert(root < this->fae.roots.size());

		auto ta = std::shared_ptr<TA<label_type>>(this->fae.allocTA());

		size_t state = this->fae.roots[root]->getFinalState();

		ta->addFinalState(state);

		for (auto i = this->fae.roots[root]->begin(); i != this->fae.roots[root]->end(); ++i) {

			if (i->rhs() != state) {

				ta->addTransition(*i);

				continue;

			}

			std::vector<const AbstractBox*> label(i->label()->getNode());
			std::vector<size_t> lhs(i->lhs());

			label.push_back(box);

			for (auto& rootInfo : signature) {

				if (rootInfo.root == root)
					continue;

				lhs.push_back(
					this->fae.addData(*ta, Data::createRef(rootInfo.root))
				);

			}

			FAE::reorderBoxes(label, lhs);

			ta->addTransition(lhs, this->fae.boxMan->lookupLabel(label), state);

		}

		// replace
		this->fae.roots[root] = ta;
		this->fae.updateRootMap(root);

	}

	void makeType1Box(size_t root) {

		std::vector<size_t> index;

		auto p = this->separateCutpoint(root, root, index);

		this->joinBox(root, root, this->boxMan.createBox(0, { p.first }), p.second);

	}

	void makeType2Box(size_t root, size_t aux) {

		throw std::runtime_error("not implemented");

	}

	void makeType3Box(size_t root, size_t aux) {

		std::vector<size_t> index;

		auto p = this->separateCutpoint(root, aux, index);

		size_t arity = p.second.size();

		if (Explorer::sigFind(p.second, root))
			--arity;

		auto auxTA = this->separateAux(aux, root, index);

		this->joinBox(root, aux, this->boxMan.createBox(arity, { p.first, auxTA }), p.second);

	}

public:

	bool discover(size_t root) {

		assert(this->fae.roots.size() == this->fae.rootMap.size());
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);

		std::vector<size_t> aux;

		for (auto& rootInfo : this->fae.rootMap[root]) {

			if (rootInfo.root == root) {

				CL_CDEBUG(1, "type 1 cutpoint detected at root " << root);

				this->makeType1Box(root);

				return true;

			}

			if (rootInfo.joint) {

				CL_CDEBUG(1, "type 2 cutpoint detected at root " << root);

				this->makeType2Box(root, rootInfo.root);

				return true;

			}

			if (Explorer::sigFind(this->fae.rootMap[rootInfo.root], root)) {

				CL_CDEBUG(1, "type 3 cutpoint detected at root " << root);

				this->makeType3Box(root, rootInfo.root);

				return true;

			}

		}

		return false;

	}

public:

	Explorer(FAE& fae, BoxMan& boxMan) : fae(fae), boxMan(boxMan) {}

};

#endif
