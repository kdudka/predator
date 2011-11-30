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

#ifndef FOLDING_H
#define FOLDING_H

#include <vector>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

#include "forestautext.hh"
#include "abstractbox.hh"
#include "boxman.hh"
#include "connection_graph.hh"

class Folding {

	FAE& fae;
	BoxMan& boxMan;

	std::unordered_map<Box, std::set<std::pair<size_t, size_t>>, boost::hash<Box>> boxCache;

protected:

	static void copyBox(std::vector<size_t>& lhs, std::vector<const AbstractBox*>& label,
		const AbstractBox* box, const std::vector<size_t>& srcLhs, const size_t& srcOffset) {

		for (size_t i = 0; i < box->getArity(); ++i)
			lhs.push_back(srcLhs[srcOffset + i]);

		label.push_back(box);

	}

	static const ConnectionGraph::CutpointSignature& getSignature(size_t state,
		const ConnectionGraph::StateToCutpointSignatureMap& signatures) {

		auto iter = signatures.find(state);

		assert(iter != signatures.end());

		return iter->second;

	}

	static bool isSignaturesCompatible(const ConnectionGraph::CutpointSignature& s1,
		const ConnectionGraph::CutpointSignature& s2) {

		if (s1.size() != s2.size())
			return false;

		for (size_t i = 0; i < s1.size(); ++i) {

			if (s1[i].root != s2[i].root)
				return false;

			if (s1[i].forwardSelector != s2[i].forwardSelector)
				return false;

			if (s1[i].backwardSelector != s2[i].backwardSelector)
				return false;

			if (s1[i].defines != s2[i].defines)
				return false;

		}

		return true;

	}

	void cutBox(TA<label_type>& res, TA<label_type>& complement,
		ConnectionGraph::CutpointSignature& complementSignature, size_t root, size_t target,
		const ConnectionGraph::StateToCutpointSignatureMap& signatures) {

		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);

		const TA<label_type>& src = *this->fae.roots[root];

		std::unordered_set<const AbstractBox*> boxes;

		// enumerate which boxes to fold
		for (auto k = src.getFinalStates().begin(); k != src.getFinalStates().end(); ++k) {

			for (auto i = src.begin(*k); i != src.end(*k, i); ++i) {

				size_t lhsOffset = 0;

				const TT<label_type>& t = *i;

				for (auto& box : t.label()->getNode()) {

					// look for target cutpoint
					for (size_t j = 0; j < box->getArity(); ++j) {

						assert(lhsOffset + j < t.lhs().size());

						if (ConnectionGraph::containsCutpoint(
							Folding::getSignature(t.lhs()[lhsOffset + j], signatures), target)
						) {

							boxes.insert(box);

							break;

						}

					}

					lhsOffset += box->getArity();

				}

			}

		}

		ConnectionGraph::CutpointSignature tmp;

		for (auto k = src.getFinalStates().begin(); k != src.getFinalStates().end(); ++k) {

			size_t freshFinalState = this->fae.freshState();

			for (auto i = src.begin(*k); i != src.end(*k, i); ++i) {

				std::vector<size_t> lhs, cLhs;
				std::vector<const AbstractBox*> label, cLabel;

				size_t lhsOffset = 0;

				const TT<label_type>& t = *i;

				tmp.clear();

				// split transition
				for (auto& box : t.label()->getNode()) {

					if (boxes.count(box) == 0) {

						Folding::copyBox(lhs, label, box, t.lhs(), lhsOffset);

					} else {

						for (size_t j = 0; j < box->getArity(); ++j) {

							assert(lhsOffset + j < t.lhs().size());

							ConnectionGraph::processStateSignature(
								tmp,
								(StructuralBox*)box,
								j,
								t.lhs()[lhsOffset + j],
								Folding::getSignature(t.lhs()[lhsOffset + j], signatures)
							);

						}

						Folding::copyBox(cLhs, cLabel, box, t.lhs(), lhsOffset);

					}

					lhsOffset += box->getArity();

				}

				ConnectionGraph::normalizeSignature(tmp);

				assert(tmp.size());

				if (complementSignature.empty())
					complementSignature = tmp;

				// a bit hacky but who cares
				assert(Folding::isSignaturesCompatible(complementSignature, tmp));

				for (size_t i = 0; i < tmp.size(); ++i) {

					if (complementSignature[i].joint)
						complementSignature[i].joint = tmp[i].joint;

				}

				assert(label.size());
				FAE::reorderBoxes(label, lhs);
				res.addTransition(lhs, this->fae.boxMan->lookupLabel(label), freshFinalState);

				assert(cLabel.size());
				FAE::reorderBoxes(cLabel, cLhs);
				complement.addTransition(cLhs, this->fae.boxMan->lookupLabel(cLabel), freshFinalState);

			}

			res.addFinalState(freshFinalState);
			complement.addFinalState(freshFinalState);

		}

		src.copyTransitions(res);
		src.copyTransitions(complement);

	}

	std::pair<std::shared_ptr<TA<label_type>>, std::shared_ptr<TA<label_type>>> separateCutpoint(
		ConnectionGraph::CutpointSignature& boxSignature, size_t root, size_t cutpoint,
		const ConnectionGraph::StateToCutpointSignatureMap& signatures) {

		auto ta = std::shared_ptr<TA<label_type>>(this->fae.allocTA());
		auto tmp = std::shared_ptr<TA<label_type>>(this->fae.allocTA());

		this->cutBox(*ta, *tmp, boxSignature, root, cutpoint, signatures);

		auto tmp2 = std::shared_ptr<TA<label_type>>(this->fae.allocTA());

		tmp->unreachableFree(*tmp2);

		return std::make_pair(ta, tmp2);

	}

	std::shared_ptr<TA<label_type>> relabelReferences(const TA<label_type>& ta,
		std::vector<size_t>& index) {

		auto tmp = std::shared_ptr<TA<label_type>>(this->fae.allocTA());

		this->fae.relabelReferences(*tmp, ta, index);

		return tmp;

	}

	std::shared_ptr<TA<label_type>> joinBox(const TA<label_type>& source, size_t root,
		const Box* box, const ConnectionGraph::CutpointSignature& signature) {

		auto ta = std::shared_ptr<TA<label_type>>(this->fae.allocTA());

		size_t state = source.getFinalState();

		ta->addFinalState(state);

		for (auto i = source.begin(); i != source.end(); ++i) {

			if (i->rhs() != state) {

				ta->addTransition(*i);

				continue;

			}

			std::vector<const AbstractBox*> label(i->label()->getNode());
			std::vector<size_t> lhs(i->lhs());

			label.push_back(box);

			for (auto& cutpoint : signature) {

				if (cutpoint.root == root)
					continue;

				lhs.push_back(
					this->fae.addData(*ta, Data::createRef(cutpoint.root))
				);

			}

			FAE::reorderBoxes(label, lhs);

			ta->addTransition(lhs, this->fae.boxMan->lookupLabel(label), state);

		}

		return ta;

	}

	static void updateSelectorMap(std::unordered_map<size_t, size_t>& m, size_t selector,
		const ConnectionGraph::CutpointSignature& signature) {

		for (auto& cutpoint : signature)

			m.insert(std::make_pair(cutpoint.root, selector));

	}

	// compute cutpoint-to-selector mapping, i.e. tell which selector one needs to take
	// in order to reach a given cutpoint
	static void computeSelectorMap(std::unordered_map<size_t, size_t>& selectorMap,
		const TT<label_type>& t, const ConnectionGraph::StateToCutpointSignatureMap& stateMap) {

		size_t lhsOffset = 0;

		for (auto& absBox : t.label()->getNode()) {

			switch (absBox->getType()) {

				case box_type_e::bSel: {

					auto iter = stateMap.find(t.lhs()[lhsOffset]);

					assert(iter != stateMap.end());

					Folding::updateSelectorMap(
						selectorMap, ((const SelBox*)absBox)->getData().offset, iter->second
					);

					break;
				}

				case box_type_e::bBox: {

					const Box* box = (const Box*)absBox;

					for (size_t i = 0; i < box->getArity(); ++i) {

						auto iter = stateMap.find(t.lhs()[lhsOffset + i]);

						assert(iter != stateMap.end());

						Folding::updateSelectorMap(
							selectorMap, box->getSelector(i), iter->second
						);

					}

					break;

				}

				default: break;

			}

			lhsOffset += absBox->getArity();

		}

	}

	static bool checkSelectorMap(const std::unordered_map<size_t, size_t>& selectorMap,
		const TA<label_type>& ta, const ConnectionGraph::StateToCutpointSignatureMap& stateMap) {

		for (TA<label_type>::iterator i = ta.accBegin(); i != ta.accEnd(i); ++i) {

			std::unordered_map<size_t, size_t> m;

			Folding::computeSelectorMap(m, *i, stateMap);

			if (m != selectorMap)
				return false;

		}

		return true;

	}

	static void computeSelectorMap(std::unordered_map<size_t, size_t>& selectorMap,
		const TA<label_type>& ta, const ConnectionGraph::StateToCutpointSignatureMap& stateMap) {

		assert(ta.accBegin() != ta.accEnd());

		Folding::computeSelectorMap(selectorMap, *ta.accBegin(), stateMap);

		assert(Folding::checkSelectorMap(selectorMap, ta, stateMap));

	}

	static size_t extractSelector(const std::unordered_map<size_t, size_t>& selectorMap,
		size_t target) {

		auto iter = selectorMap.find(target);

		assert(iter != selectorMap.end());

		return iter->second;

	}

	// transform
	static void extractInputMap(std::vector<size_t>& inputMap,
		const std::unordered_map<size_t, size_t>& selectorMap, size_t root,
		const std::vector<size_t>& index) {

		assert(index[root] == 0);

		inputMap.resize(selectorMap.size());

		size_t count = 0;

		for (auto& cutpointSelectorPair : selectorMap) {

			if (cutpointSelectorPair.first == root) {

				// reference to root does not appear in the box interface
				continue;

			}

			assert(cutpointSelectorPair.first < index.size());

			if (index[cutpointSelectorPair.first] == (size_t)(-1))
				continue;

			assert(index[cutpointSelectorPair.first] >= 1);
			assert(index[cutpointSelectorPair.first] < inputMap.size() + 1);

			inputMap[index[cutpointSelectorPair.first] - 1] = cutpointSelectorPair.second;

			++count;
		}

		inputMap.resize(count);

	}
/*
	static bool checkSingular(const TA<label_type>& ta, bool result,
		const ConnectionGraph::StateToCutpointSignatureMap& stateMap) {

		for (auto& state : ta.getFinalStates()) {

			auto iter = stateMap.find(state);

			assert(iter != stateMap.end());

			if (iter->second.empty() != result)
				return false;

		}

		return true;

	}

	static bool isSingular(const TA<label_type>& ta) {

		ConnectionGraph::StateToCutpointSignatureMap stateMap;

		ConnectionGraph::computeSignatures(stateMap, ta);

		assert(ta.getFinalStates().size());

		auto iter = stateMap.find(*ta.getFinalStates().begin());

		assert(iter != stateMap.end());

		bool result = iter->second.empty();

		assert(Folding::checkSingular(ta, result, stateMap));

		return result;

	}
*/
	bool makeType1Box(std::shared_ptr<const Box>& box, size_t root, size_t aux, const std::set<size_t>& forbidden, bool conditional = true, bool test = false) {

		box = nullptr;

		if (forbidden.count(aux))
			return false;

		assert(root < this->fae.roots.size());

		std::vector<size_t> index(this->fae.roots.size(), (size_t)(-1)), inputMap;
		std::unordered_map<size_t, size_t> selectorMap;
		ConnectionGraph::StateToCutpointSignatureMap signatures;
		ConnectionGraph::CutpointSignature outputSignature;

		size_t start = 0;

		ConnectionGraph::computeSignatures(signatures, *this->fae.roots[root]);

		auto p = this->separateCutpoint(outputSignature, root, aux, signatures);

		index[root] = start++;

		for (auto& cutpoint : outputSignature) {

			if (forbidden.count(cutpoint.root))
				return false;

			assert(cutpoint.root < index.size());

			if (cutpoint.root != root)
				index[cutpoint.root] = start++;

		}

		Folding::computeSelectorMap(selectorMap, *this->fae.roots[root], signatures);
		Folding::extractInputMap(inputMap, selectorMap, root, index);

		box = std::shared_ptr<Box>(
			this->boxMan.createType1Box(
				root,
				this->relabelReferences(*p.second, index),
				outputSignature,
				inputMap,
				index
			)
		);

		auto boxPtr = (conditional)?(this->boxMan.lookupBox(*box)):(this->boxMan.getBox(*box));

		if (!boxPtr)
			return false;

		if (test)
			return true;

		this->fae.roots[root] = this->joinBox(*p.first, root, boxPtr, outputSignature);
		this->fae.connectionGraph.invalidate(root);

		return true;

	}

	bool makeType2Box(std::shared_ptr<const Box>& box, size_t root, size_t aux, const std::set<size_t>& forbidden, bool conditional = true, bool test = false) {

		box = nullptr;

		if (forbidden.count(aux))
			return false;

		assert(root < this->fae.roots.size());
		assert(aux < this->fae.roots.size());

		std::vector<size_t> index(this->fae.roots.size(), (size_t)(-1)), index2, inputMap;
		std::vector<bool> rootMask(this->fae.roots.size(), false);
		std::unordered_map<size_t, size_t> selectorMap;
		ConnectionGraph::StateToCutpointSignatureMap signatures;
		ConnectionGraph::CutpointSignature outputSignature, inputSignature;

		size_t start = 0;

		ConnectionGraph::computeSignatures(signatures, *this->fae.roots[root]);

		auto p = this->separateCutpoint(outputSignature, root, aux, signatures);

		index[root] = start++;

		for (auto& cutpoint : outputSignature) {
/*
			// we assume type 1 box is not present
			assert(cutpoint.root != root);
*/
			if (cutpoint.root == root)
				return false;

			if (forbidden.count(cutpoint.root))
				return false;

			assert(cutpoint.root < index.size());

			if (cutpoint.root != root)
				index[cutpoint.root] = start++;

		}

		Folding::computeSelectorMap(selectorMap, *this->fae.roots[root], signatures);
		Folding::extractInputMap(inputMap, selectorMap, root, index);

		signatures.clear();

		ConnectionGraph::computeSignatures(signatures, *this->fae.roots[aux]);

		auto auxP = this->separateCutpoint(inputSignature, aux, root, signatures);
/*
		if (Folding::isSingular(*auxP.first))
			return false;
*/
		index2 = index;

		for (auto& cutpoint : inputSignature) {

			if (forbidden.count(cutpoint.root))
				return false;

			assert(cutpoint.root < index.size());

			if (index[cutpoint.root] == (size_t)(-1)) {

				assert(index2[cutpoint.root] == (size_t)(-1));

				index2[cutpoint.root] = start++;

			}

		}

		selectorMap.clear();

		Folding::computeSelectorMap(selectorMap, *this->fae.roots[aux], signatures);

		size_t selector = Folding::extractSelector(selectorMap, root);

		box = std::shared_ptr<Box>(
			this->boxMan.createType2Box(
				root,
				this->relabelReferences(*p.second, index),
				outputSignature,
				inputMap,
				aux,
				this->relabelReferences(*auxP.second, index2),
				inputSignature,
				selector,
				index
			)
		);

		auto boxPtr = (conditional)?(this->boxMan.lookupBox(*box)):(this->boxMan.getBox(*box));

		if (!boxPtr)
			return false;

		if (test)
			return true;

		this->fae.roots[root] = this->joinBox(*p.first, root, boxPtr, outputSignature);
		this->fae.connectionGraph.invalidate(root);

		this->fae.roots[aux] = auxP.first;
		this->fae.connectionGraph.invalidate(aux);

		return true;

	}

public:

	bool discover(size_t root, const std::set<size_t>& forbidden, bool conditional) {

		assert(this->fae.connectionGraph.isValid());
		assert(this->fae.roots.size() == this->fae.connectionGraph.data.size());
		assert(root < this->fae.roots.size());
		assert(this->fae.roots[root]);

		CL_CDEBUG(3, "folding: " << this->fae);

		std::shared_ptr<const Box> box;

		// save state offset
		this->fae.pushStateOffset();

		for (auto& cutpoint : this->fae.connectionGraph.data[root].signature) {

			if (cutpoint.root == root) {

				CL_CDEBUG(3, "type 1 cutpoint detected at root " << root);

				if (this->makeType1Box(box, root, root, forbidden, conditional))
					return true;

				this->fae.popStateOffset();

				continue;

			}

			if (cutpoint.joint) {

				CL_CDEBUG(3, "type 2 cutpoint detected at root " << root);

				if (this->makeType1Box(box, root, cutpoint.root, forbidden, conditional))
					return true;

				this->fae.popStateOffset();

				continue;

			}

			size_t selectorToRoot = ConnectionGraph::getSelectorToTarget(
				this->fae.connectionGraph.data[cutpoint.root].signature, root
			);

			if (selectorToRoot == (size_t)(-1))
				continue;
/*
			if (selectorToRoot == cutpoint.forwardSelector)
				continue;
*/
			size_t root1 = root, root2 = cutpoint.root;

			if (!conditional && (selectorToRoot < cutpoint.forwardSelector) &&
				this->makeType2Box(box, root2, root1, forbidden, true, true))
					continue;

			CL_CDEBUG(3, "type 3 cutpoint detected at roots " << root1 << " and " << root2);

			if (this->makeType2Box(box, root1, root2, forbidden, conditional))
				return true;

			this->fae.popStateOffset();

		}

		return false;

	}

public:

	Folding(FAE& fae, BoxMan& boxMan) : fae(fae), boxMan(boxMan) {}

};

#endif
