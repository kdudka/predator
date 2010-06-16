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

#ifndef LTS_H
#define LTS_H

#include "smartset.hh"

class LTS {

	size_t _labels;
	size_t _states;
	size_t _transitions;
	std::vector<std::vector<std::vector<size_t> > > _dataPre;
	std::vector<SmartSet> _lPre;

public:

	LTS(size_t labels = 0, size_t states = 0) :
		_labels(labels), _states(states), _transitions(0),
		_dataPre(labels, std::vector<std::vector<size_t> >(states)),
		_lPre(states, SmartSet(labels)) {}
/*
	LTS(const LTS& lts) :
		_labels(lts._labels), _states(lts._states), _transitions(lts._transitions),
		_dataPre(lts._dataPre),
		_lPre(lts._lPre) {}
*/
	void addTransition(size_t q, size_t a, size_t r) {
		if (a >= this->_dataPre.size())
			throw std::runtime_error("label index out of range");
		if (r >= this->_dataPre[a].size()) {
			std::cout << r << "<" << this->_dataPre[a].size() << "\n";
			throw std::runtime_error("state index out of range");
		}
		if (q >= this->_dataPre[a].size())
			throw std::runtime_error("state index out of range");
		this->_dataPre[a][r].push_back(q);
		this->_lPre[r].add(a);
		++this->_transitions;
	}

	const std::vector<size_t>& pre(size_t q, size_t a) const {
		return this->_dataPre[a][q];
	}
/*
	method iter f =
    Array.iteri (fun a pre_a -> Array.iteri (fun r pre_a_r -> List.iter (fun q -> f q a r) pre_a_r) pre_a) _data_pre
*/
	const std::vector<std::vector<std::vector<size_t> > >& dataPre() const { return this->_dataPre; }

	void buildPost(size_t label, std::vector<std::vector<size_t> >& dst) const {
		dst.clear();
		dst.resize(this->_states);
		for (size_t i = 0; i < this->_dataPre[label].size(); ++i) {
			for (std::vector<size_t>::const_iterator j = this->_dataPre[label][i].begin(); j != this->_dataPre[label][i].end(); ++j)
				dst[*j].push_back(i);
		}
	}
	
	void buildDelta(std::vector<SmartSet>& delta, std::vector<SmartSet>& delta1) const {
		delta.resize(this->_labels);
		delta1.resize(this->_labels);
		for (size_t a = 0; a < this->_labels; ++a) {
			delta[a] = SmartSet(this->_states);
			delta1[a] = SmartSet(this->_states);
			for (size_t i = 0; i < this->_dataPre[a].size(); ++i) {
				for (std::vector<size_t>::const_iterator j = this->_dataPre[a][i].begin(); j != this->_dataPre[a][i].end(); ++j) {
					delta[a].add(i);
					delta1[a].add(*j);
				}
			}
		}
	}

	const std::vector<SmartSet>& lPre() const {
		return this->_lPre;
	}
	
	size_t labels() const {
		return this->_labels;
	}

	size_t states() const {
		return this->_states;
	}

	void dump() const {
		std::cout << "states: " << this->_states << ", labels: " << this->_labels << std::endl;
		for (size_t i = 0; i < this->_labels; ++i) {
			for (size_t j = 0; j < this->_states; ++j) {
				for (size_t k = 0; k < this->_dataPre[i][j].size(); ++k)
					std::cout << this->_dataPre[i][j][k] << " --" << i << "--> " << j << std::endl;
			}
		}
	}

/*
(*
		method setData d = (
		_data <- d;
		_nsymbols <- Array.length _data;
		_nstates <- Array.length _data.(0)
	)
*)	
	method nsymbols = _nsymbols
	method nstates = _nstates
	method ntransitions = _ntransitions
	
	method stats =
		Printf.sprintf "states: %d, symbols: %d, transitions: %d" _nstates _nsymbols _ntransitions

*/
};

#endif
