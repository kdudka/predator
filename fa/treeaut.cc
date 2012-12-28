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

// Standard library headers
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <ostream>

// Forester headers
#include "treeaut.hh"
#include "simalg.hh"
#include "antichainext.hh"

struct LhsEnv
{
	size_t index;
	std::vector<size_t> data;

	LhsEnv(
		const std::vector<size_t>&   lhs,
		size_t                       index) :
		index(index),
		data{}
	{
		this->data.insert(this->data.end(), lhs.begin(), lhs.begin() + index);
		this->data.insert(this->data.end(), lhs.begin() + index + 1, lhs.end());
	}

	static std::set<LhsEnv>::iterator get(
		const std::vector<size_t>&    lhs,
		size_t                        index,
		std::set<LhsEnv>&             s)
	{
		return s.insert(LhsEnv(lhs, index)).first;
	}

	static std::set<LhsEnv>::iterator find(
		const std::vector<size_t>&     lhs,
		size_t                         index,
		std::set<LhsEnv>&              s)
	{
		std::set<LhsEnv>::iterator x = s.find(LhsEnv(lhs, index));
		if (s.end() == x)
			throw std::runtime_error("LhsEnv::find - lookup failed");

		return x;
	}

	static bool sim(
		const LhsEnv&                              e1,
		const LhsEnv&                              e2,
		const std::vector<std::vector<bool>>&      sim)
	{
		if ((e1.index != e2.index) || (e1.data.size() != e2.data.size()))
			return false;

		for (size_t i = 0; i < e1.data.size(); ++i)
		{
			if (!sim[e1.data[i]][e2.data[i]])
				return false;
		}

		return true;
	}

	static bool eq(
		const LhsEnv&                           e1,
		const LhsEnv&                           e2,
		const std::vector<std::vector<bool>>&   sim)
	{
		if ((e1.index != e2.index) || (e1.data.size() != e2.data.size()))
			return false;

		for (size_t i = 0; i < e1.data.size(); ++i)
		{
			if (!sim[e1.data[i]][e2.data[i]] || !sim[e2.data[i]][e1.data[i]])
				return false;
		}

		return true;
	}

	bool operator<(const LhsEnv& rhs) const
	{
		return (this->index < rhs.index) || (
			(this->index == rhs.index) && (this->data < rhs.data));
	}

	bool operator==(const LhsEnv& rhs) const
	{
		return (this->index == rhs.index) && (this->data == rhs.data);
	}

	friend std::ostream& operator<<(
		std::ostream&        os,
		const LhsEnv&        env)
	{
		os << "[" << env.data.size() << "]";
		os << env.index << "|";
		for (size_t i : env.data)
			os << "|" << i;
		return os;
	}
};

struct Env
{
	std::set<LhsEnv>::iterator lhs;
	size_t label;
	size_t rhs;

	Env(
		std::set<LhsEnv>::iterator    lhs,
		size_t                        label,
		size_t                        rhs) :
		lhs(lhs),
		label(label),
		rhs(rhs)
	{ }

	static std::map<Env, size_t>::iterator get(
		std::set<LhsEnv>::const_iterator   lhs,
		size_t                             label,
		size_t                             rhs,
		std::map<Env, size_t>&             m)
	{
		return m.insert(std::pair<Env, size_t>(Env(lhs, label, rhs), m.size())).first;
	}

	static std::map<Env, size_t>::iterator get(
		std::set<LhsEnv>::const_iterator    lhs,
		size_t                              label,
		size_t                              rhs,
		std::map<Env, size_t>&              m,
		bool&                               inserted)
	{
		std::pair<std::map<Env, size_t>::iterator, bool> x = m.insert(std::pair<Env, size_t>(Env(lhs, label, rhs), m.size()));
		inserted = x.second;
		return x.first;
	}

	static std::map<Env, size_t>::iterator find(
		std::set<LhsEnv>::const_iterator     lhs,
		size_t                               label,
		size_t                               rhs,
		std::map<Env, size_t>&               m)
	{
		std::map<Env, size_t>::iterator x = m.find(Env(lhs, label, rhs));
		if (x == m.end())
			throw std::runtime_error("Env::find - lookup failed");
		return x;
	}

	static bool sim(
		const Env&                              e1,
		const Env&                              e2,
		const std::vector<std::vector<bool>>&   sim)
	{
		return (e1.label == e2.label) && LhsEnv::sim(*e1.lhs, *e2.lhs, sim);
	}

	static bool eq(
		const Env&                              e1,
		const Env&                              e2,
		const std::vector<std::vector<bool>>&   sim)
	{
		return (e1.label == e2.label) && LhsEnv::eq(*e1.lhs, *e2.lhs, sim);
	}

	bool operator<(const Env& rhs) const
	{
		return (this->label < rhs.label) || (
			(this->label == rhs.label) && (
				(*this->lhs < *rhs.lhs) || (
					(*this->lhs == *rhs.lhs) && (
						this->rhs < rhs.rhs
					)
				)
			)
		);
	}
};

template <class T>
void TA<T>::downwardTranslation(
	LTS&                    lts,
	const Index<size_t>&    stateIndex,
	const Index<T>&         labelIndex) const
{
	// build an index of non-translated left-hand sides
	Index<const std::vector<size_t>*> lhs;
	this->buildLhsIndex(lhs);
	lts = LTS(labelIndex.size() + this->maxRank, stateIndex.size() + lhs.size());
	for (Index<const std::vector<size_t>*>::iterator i = lhs.begin();
		i != lhs.end(); ++i)
	{
		for (size_t j = 0; j < i->first->size(); ++j)
		{
			lts.addTransition(stateIndex.size() + i->second,
				labelIndex.size() + j, stateIndex[(*i->first)[j]]);
		}
	}

	for (const TransIDPair* ptrTransIDPair : this->transitions)
	{
		lts.addTransition(
			stateIndex[ptrTransIDPair->first.rhs()],
			labelIndex[ptrTransIDPair->first.label()],
			stateIndex.size() + lhs[&(ptrTransIDPair->first.lhs())]);
	}
}


template <class T>
void TA<T>::downwardSimulation(
	std::vector<std::vector<bool>>&   rel,
	const Index<size_t>&              stateIndex) const
{
	LTS lts;
	Index<T> labelIndex;
	this->buildLabelIndex(labelIndex);
	this->downwardTranslation(lts, stateIndex, labelIndex);
	OLRTAlgorithm alg(lts);
	alg.init();
	alg.run();
	alg.buildRel(stateIndex.size(), rel);
}

template <class T>
void TA<T>::upwardTranslation(
	LTS&                                    lts,
	std::vector<std::vector<size_t>>&       part,
	std::vector<std::vector<bool>>&         rel,
	const Index<size_t>&                    stateIndex,
	const Index<T>&                         labelIndex,
	const std::vector<std::vector<bool>>&   sim) const
{
	std::set<LhsEnv> lhsEnvSet;
	std::map<Env, size_t> envMap;
	std::vector<const Env*> head;
	part.clear();
	for (const TransIDPair* ptrTransIDPair : this->transitions)
	{
		std::vector<size_t> lhs;
		stateIndex.translate(lhs, ptrTransIDPair->first.lhs());
		size_t label = labelIndex[ptrTransIDPair->first.label()];
		size_t rhs = stateIndex[ptrTransIDPair->first.rhs()];
		for (size_t j = 0; j < lhs.size(); ++j)
		{ // insert required items into lhsEnv and lhsMap and build equivalence
			// classes
			bool inserted;
			std::map<Env, size_t>::const_iterator env =
				Env::get(LhsEnv::get(lhs, j, lhsEnvSet), label, rhs, envMap, inserted);

			if (inserted)
			{
				inserted = false;
				for (size_t k = 0; k < head.size(); ++k)
				{
					if (Env::eq(*head[k], env->first, sim))
					{
						part[k].push_back(env->second + stateIndex.size());
						inserted = true;
						break;
					}
				}
				if (!inserted)
				{
					head.push_back(&env->first);
					part.push_back(std::vector<size_t>(
						1, env->second + stateIndex.size()));
				}
			}
		}
	}

	lts = LTS(labelIndex.size() + 1, stateIndex.size() + envMap.size());
	for (const TransIDPair* ptrTransIDPair : this->transitions)
	{
		std::vector<size_t> lhs;
		stateIndex.translate(lhs, ptrTransIDPair->first.lhs());
		size_t label = labelIndex[ptrTransIDPair->first.label()];
		size_t rhs = stateIndex[ptrTransIDPair->first.rhs()];
		for (size_t j = 0; j < lhs.size(); ++j)
		{
			// find particular env
			std::map<Env, size_t>::iterator env =
				Env::find(LhsEnv::find(lhs, j, lhsEnvSet), label, rhs, envMap);
			lts.addTransition(lhs[j], labelIndex.size(), env->second);
			lts.addTransition(env->second, label, rhs);
		}
	}

	rel = std::vector<std::vector<bool>>(
		part.size() + 2, std::vector<bool>(part.size() + 2, false));

	// 0 non-accepting, 1 accepting, 2 .. environments
	rel[0][0] = true;
	rel[0][1] = true;
	rel[1][1] = true;
	for (size_t i = 0; i < head.size(); ++i)
	{
		for (size_t j = 0; j < head.size(); ++j)
		{
			if (Env::sim(*head[i], *head[j], sim))
				rel[i + 2][j + 2] = true;
		}
	}
}

template <class T>
void TA<T>::upwardSimulation(
	std::vector<std::vector<bool>>&         rel,
	const Index<size_t>&                    stateIndex,
	const std::vector<std::vector<bool>>&   param) const
{
	LTS lts;
	Index<T> labelIndex;
	this->buildLabelIndex(labelIndex);
	std::vector<std::vector<size_t>> part;
	std::vector<std::vector<bool>> initRel;
	this->upwardTranslation(lts, part, initRel, stateIndex, labelIndex, param);
	OLRTAlgorithm alg(lts);
	// accepting states to block 1
	std::vector<size_t> finalStates;
	stateIndex.translate(finalStates, std::vector<size_t>(finalStates_.begin(), finalStates_.end()));
	alg.fakeSplit(finalStates);
	// environments to blocks 2, 3, ...
	for (size_t i = 0; i < part.size(); ++i)
		alg.fakeSplit(part[i]);
	alg.getRelation().load(initRel);
	alg.init();
	alg.run();
	alg.buildRel(stateIndex.size(), rel);
}

template <class T>
void TA<T>::combinedSimulation(
	std::vector<std::vector<bool>>&           dst,
	const std::vector<std::vector<bool>>&     dwn,
	const std::vector<std::vector<bool>>&     up)
{
	size_t size = dwn.size();
	std::vector<std::vector<bool> > dut(size, std::vector<bool>(size, false));
	for (size_t i = 0; i < size; ++i)
	{
		for (size_t j = 0; j < size; ++j)
		{
			for (size_t k = 0; k < size; ++k)
			{
				if (dwn[i][k] && up[j][k])
				{
					dut[i][j] = true;
					break;
				}
			}
		}
	}
	dst = dut;
	for (size_t i = 0; i < size; ++i)
	{
		for (size_t j = 0; j < size; ++j)
		{
			if (!dst[i][j])
				continue;

			for (size_t k = 0; k < size; ++k)
			{
				if (dwn[j][k] && !dut[i][k])
				{
					dst[i][j] = false;
					break;
				}
			}
		}
	}
}

template <class T>
bool TA<T>::subseteq(const TA<T>& a, const TA<T>& b)
{
	return AntichainExt<T>::subseteq(a, b);
}

// this is really sad :-(
#include "forestaut.hh"
template class TA<label_type>;
