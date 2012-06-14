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

#ifndef UNFOLDING_H
#define UNFOLDING_H

#include <vector>
#include <set>
#include <stdexcept>
#include <algorithm>

#include "forestautext.hh"
#include "abstractbox.hh"
#include "utils.hh"

class Unfolding {

	FAE& fae;

protected:

	void boxMerge(TreeAut& dst, const TreeAut& src, const TreeAut& boxRoot, const Box* box, const std::vector<size_t>& rootIndex) {
		TreeAut tmp(*this->fae.backend), tmp2(*this->fae.backend);
//		this->fae.boxMan->adjustLeaves(tmp2, boxRoot);
		this->fae.relabelReferences(tmp, boxRoot, rootIndex);
		this->fae.unique(tmp2, tmp);
		src.copyTransitions(dst, TreeAut::NonAcceptingF(src));
		tmp2.copyTransitions(dst, TreeAut::NonAcceptingF(tmp2));
		dst.addFinalStates(tmp2.getFinalStates());
		for (std::set<size_t>::const_iterator j = src.getFinalStates().begin(); j != src.getFinalStates().end(); ++j) {
			for (TreeAut::iterator i = src.begin(*j); i != src.end(*j, i); ++i) {
				std::vector<size_t> lhs;
				std::vector<const AbstractBox*> label;
				size_t lhsOffset = 0;
				if (box) {
					bool found = false;
					for (std::vector<const AbstractBox*>::const_iterator j = i->label()->getNode().begin(); j != i->label()->getNode().end(); ++j) {
						if (!(*j)->isStructural()) {
							label.push_back(*j);
							continue;
						}
						const StructuralBox* b = static_cast<const StructuralBox*>(*j);
						if (b != static_cast<const StructuralBox*>(box)) {
							// this box is not interesting
							for (size_t k = 0; k < b->getArity(); ++k, ++lhsOffset)
								lhs.push_back(i->lhs()[lhsOffset]);
							label.push_back(b);
							continue;
						}
						lhsOffset += box->getArity();

						if (found)
							assert(false);

						found = true;
					}

					if (!found)
						assert(false);

				} else {
					lhs = i->lhs();
					label = i->label()->getNode();
				}
				for (TreeAut::iterator j = tmp2.accBegin(); j != tmp2.accEnd(j); ++j) {
					std::vector<size_t> lhs2 = lhs;
					std::vector<const AbstractBox*> label2 = label;
					lhs2.insert(lhs2.end(), j->lhs().begin(), j->lhs().end());
					label2.insert(label2.end(), j->label()->getNode().begin(), j->label()->getNode().end());
					FA::reorderBoxes(label2, lhs2);
					dst.addTransition(lhs2, this->fae.boxMan->lookupLabel(label2), j->rhs());
				}
			}
		}
	}

public:

	void unfoldBox(size_t root, const Box* box) {

//		CL_CDEBUG(3, "unfolding " << *(AbstractBox*)box << " at root " << root << std::endl << this->fae);

		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);
		assert(box);

		const TT<label_type>& t = this->fae.roots[root]->getAcceptingTransition();

		size_t lhsOffset = 0;
		std::vector<size_t> index = { root };

		for (std::vector<const AbstractBox*>::const_iterator i = t.label()->getNode().begin(); i != t.label()->getNode().end(); ++i) {

			if (static_cast<const AbstractBox*>(box) != *i) {

				lhsOffset += (*i)->getArity();

				continue;

			}

			for (size_t j = 0; j < box->getArity(); ++j) {

				const Data& data = this->fae.getData(t.lhs()[lhsOffset + j]);

				if (data.isUndef())
					index.push_back(static_cast<size_t>(-1));
				else
					index.push_back(data.d_ref.root);

			}

			break;

		}

		auto ta = std::shared_ptr<TreeAut>(this->fae.allocTA());

		this->boxMerge(*ta, *this->fae.roots[root], *box->getOutput(), box, index);

		this->fae.roots[root] = ta;
		this->fae.connectionGraph.invalidate(root);

		if (!box->getInput())
			return;

		assert(box->getInputIndex() < index.size());

		size_t aux = index[box->getInputIndex() + 1];

		assert(aux != static_cast<size_t>(-1));
		assert(aux < this->fae.roots.size());

		TreeAut tmp(*this->fae.backend);

		this->fae.roots[aux]->unfoldAtRoot(tmp, this->fae.freshState());
		this->fae.roots[aux] = std::shared_ptr<TreeAut>(this->fae.allocTA());

		this->boxMerge(*this->fae.roots[aux], tmp, *box->getInput(), nullptr, index);

		this->fae.connectionGraph.invalidate(aux);

//		this->fae.updateConnectionGraph();

	}

	void unfoldBoxes(size_t root, const std::set<const Box*>& boxes) {
		for (std::set<const Box*>::const_iterator i = boxes.begin(); i != boxes.end(); ++i)
			this->unfoldBox(root, *i);
	}

public:

	Unfolding(FAE& fae) : fae(fae) {}

};

#endif
