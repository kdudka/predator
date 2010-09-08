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

#include <vector>
#include <boost/unordered_map.hpp>

#include <cl/cl_msg.hh>
#include <cl/storage.hh>

#include "treeaut.hh"
#include "forestaut.hh"
#include "forestautext.hh"
#include "ufae.hh"
#include "symexec.hh"

using boost::unordered_map;

class InternalCfg {

public:

	struct Op {
	};

	struct State {

		// configuration obtained in forward run
		TA<FA::label_type> fwdConf;
		UFAE fwdConfWrapper;

		// outstanding forest automata
		std::vector<FAE*> outConf;

		std::vector<Trans*> transitions;

		State(TA<FA::label_type>::Backend& taBackend, LabMan& labMan)
			: fwdConf(taBackend), fwdConfWrapper(this->fwdConf, labMan) {}

		~State() {
			for (std::vector<FAE*>::iterator i = this->outConf.begin(); this->outConf.end(); ++i)
				delete *i;
		}
	
	};

	struct Trans {

		State* src;
		State* dst;
		Op op;

		Trans(State* src, State* dst, Op op) src(src), dst(dst), op(op) {}
	
	};

private:

	TA<FA::label_type>::Backend taBackend;
	LabMan labMan;
	State* entry;
	State* tmp;
	std::vector<State*> states;
	std::vector<Trans*> transitions;

	typedef unordered_map<CodeStorage::Block*, State*> block_map_type;

	block_map_type blockMap;

protected:

	bool findBlockEntry(const CodeStorage::Block* block, State*& state) {

		std::pair<block_map_type::iterator, bool> p =
			this->blockMap.insert(std::make_pair(block, this->tmp));

		if (p.second)
			this->tmp = new State(this->taBackend, this->labMan);

		state = p.first;

		return p.second;

	}

	void processBlock(const CodeStorage::Block* block, State* blockEntry) {

		
		
	}

public:

	InternalCfg(TA<FA::label_type>::Backend& taBackend, LabMan& labMan, const CodeStorage::Fnc& fnc)
		: taBackEnd(taBackEnd), labMan(labMan), tmp(new SymState(this->taBackend, this->labMan)) {
	}

	~InternalCfg() {
		for (std::vector<State*>::iterator i = this->states.begin(); i != this->states.end(); ++i)
			delete *i;
		for (std::vector<Trans*>::iterator i = this->states.begin(); i != this->states.end(); ++i)
			delete *i;
	}

	static fromFunction() {

		
		
	}

};


// calling context
struct SymLocation {

	size_t ctxId;
	CodeStorage::Isns* isns;

	SymLocation(size_t ctxId, CodeStorage::Isns* isns) : ctxId(ctxId), isns(isns) {}

};

class SymExec::Engine {

	const CodeStorage::Storage& stor;

	TA<FA::label_type>::Backend taBackend;

	LabMan labMan;

	SymState* tmp;

	typedef unordered_map<SymLocation, SymState*> state_store_type;

	state_store_type stateStore;

protected:

	SymState* getState(const SymLocation& location) {

		std::pair<state_store_type::iterator, bool> p =
			this->stateStore.insert(std::make_pair(location, this->tmp));

		if (p.second)
			this->tmp = new SymState(this->taBackend, this->labMan);
		
		return p.first;

	}

	void buildInternalCfg(const CodeStorage::Fnc& main) {
		
	}

public:

	Engine(const CodeStorage::Storage& stor) : stor(stor), tmp(new SymState(this->taBackend, this->labMan)) {}

	~Engine() {
		for (state_store_type::iterator i = this->stateStore.begin(); i != this->stateStore.end(); ++i)
			delete i->second;
	}

	void run(const CodeStorage::Fnc& main) {

		std::vector<const CodeStorage::Block*> todo;

		todo.push_back(main.entry());

		while (!todo.empty()) {
		}
		
	}

	void mainLoop() {
	}

};

SymExec::SymExec(const CodeStorage::Storage &stor)
	: engine(new Engine(stor)) {}
