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
/*
class InternalCfg {

public:

	struct Ctx {

		struct VarKey {
			size_t callId;
			size_t varId;
		};

		unordered_map<VarKey, size_t> vars;

	public:

		Ctx() {}

	}

	struct Op {
	};

	struct State {

		// configuration obtained in forward run
		TA<FA::label_type> fwdConf;

		// outstanding configurations
		std::vector<FAE*> outConf;

		UFAE fwdConfWrapper;

		Ctx* ctx;

		CodeStorage::Block* block;

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
*/
struct SymCtx {

	unordered_map<int, size_t> vars;

	SymCtx() {}

	void loadVars(const CodeStorage::TVarList& vars, FAE& fae) {
	}

};

struct SymState {

	// configuration obtained in forward run
	TA<FA::label_type> fwdConf;

	// outstanding configurations
	std::vector<FAE*> outConf;

	UFAE fwdConfWrapper;

	SymCtx* ctx;

	CodeStorage::Block* block;

//	call isns -> nested call ctx ! ... >:)
	unordered_map<CodeStorage::Isns*, SymCtx*> callCtxCache;

	SymState(TA<FA::label_type>::Backend& taBackend, LabMan& labMan)
		: fwdConf(taBackend), fwdConfWrapper(this->fwdConf, labMan) {}

	~SymState() {
		for (std::vector<FAE*>::iterator i = this->outConf.begin(); this->outConf.end(); ++i)
			delete *i;
	}

};

class SymExec::Engine {

	const CodeStorage::Storage& stor;

	TA<FA::label_type>::Backend taBackend;

	TA<FA::label_type> taMan;
	LabMan labMan;
	BoxManager boxMan;

	std::list<SymCtx*> ctxStore;

	// call ctx * call isns -> nested call ctx ! ... >:)
	unordered_map<SymCtx*, CodeStorage::Isns*> ctxCache;

	SymState* tmp;

	typedef unordered_map<std::pair<SymCtx*, CodeStorage::Block*>, SymState*> state_store_type;

	state_store_type stateStore;

	std::vector<SymState*> todo;

	std::vector<SymOp*> trace;

protected:

	SymState* getState(const SymCtx* ctx, const CodeStorage::Block* block) {

		std::pair<state_store_type::iterator, bool> p =
			this->stateStore.insert(std::make_pair(std::make_pair(ctx, block), this->tmp));

		if (p.second) {
			this->tmp->ctx = ctx;
			this->tmp->block = block;
			// construct new SymState
			this->tmp = new SymState(this->taBackend, this->labMan);
		}
		
		return p.first->second;

	}

	SymCtx* newCtx() {
		this->ctxStore.push_back(new SymCtx());
		return this->ctxStore.back();
	}

	void porcessState(SymState* state) {
	}

public:

	Engine(const CodeStorage::Storage& stor)
		: stor(stor), tmp(new SymState(this->taBackend, this->labMan)), boxMan(this->taMan, this->labMan) {}

	~Engine() {
		for (state_store_type::iterator i = this->stateStore.begin(); i != this->stateStore.end(); ++i)
			delete i->second;
		delete this->tmp;
	}

	void run(const CodeStorage::Fnc& main) {

		// create main context
		SymCtx* mainCtx = this->newCtx();
		
		// create empty heap
		FAE fae(this->taMan, this->labMan, this->boxMan);

		// load variables into the main context
		mainCtx->loadVars(main.vars(), fae);

		// create an initial state
		SymState* init = this->getState(mainCtx, main.entry());

		// push empty heap into initial state
		init->outConf.push_back(new FAE(fae));

		// schedule initial state for processing
		this->todo.push_back(init);

		while (!todo.empty()) {

			SymState* state = this->todo.back();
			this->todo.pop_back();
			this->processState(state);
			
		}
		
	}

};

SymExec::SymExec(const CodeStorage::Storage &stor)
	: engine(new Engine(stor)) {}
