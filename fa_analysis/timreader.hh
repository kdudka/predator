#ifndef TIMBUK_READER_H
#define TIMBUK_READER_H

#include <cstdlib>
#include <string>
#include <vector>
#include <map>

#include "error.hh"
#include "timscanner.hh"

using std::string;
using std::vector;
using std::pair;
using std::make_pair;
using std::map;

class TimbukReader {

	TimbukScanner scanner;

protected:

	// main -> alph aut EOF
	void r_simple() {
		this->r_alph();
		this->r_aut();
		this->scanner.assertToken(TimbukScanner::tt_eof, "end of file");
	}

	// main -> alph aut_list EOF
	void r_main() {
		this->r_alph();
		this->r_aut_list();
		this->scanner.assertToken(TimbukScanner::tt_eof, "end of file");
	}

	// alph -> OPS sym_list
	void r_alph() {
		this->scanner.assertToken(TimbukScanner::tt_ops, "'Ops'");
		this->r_sym_list();
	}
	
	// sym_list -> sym sim_list | eps
	void r_sym_list() {
		this->clearLabels();
		while (this->scanner.peekToken() == TimbukScanner::tt_id)
			this->r_sym();
	}

	// sym -> ID ':' INT
	void r_sym() {
		this->scanner.assertToken(TimbukScanner::tt_id, "identifier");
		string label = this->scanner.getVal();
		this->scanner.assertToken(':', ":");
		this->scanner.assertToken(TimbukScanner::tt_int, "number");
		int arity = std::atoi(this->scanner.getVal().c_str());
		this->newLabel(label, arity, this->addLabel(label, arity));
	}
	
	// aut_list -> aut aut_list | eps
	void r_aut_list() {
		while (this->scanner.peekToken() == TimbukScanner::tt_aut)
			this->r_aut();
	}
	
	// aut -> AUT ID decl transitions
	void r_aut() {
		this->scanner.assertToken(TimbukScanner::tt_aut, "'Automaton'");
		this->scanner.assertToken(TimbukScanner::tt_id, "identifier");
		this->beginModel(this->scanner.getVal());
		this->r_decl();
		this->endDeclaration();
		this->r_transitions();
		this->endModel();
	}
	
	// decl -> STAT decl_list FIN STAT final_list
	void r_decl() {
		this->scanner.assertToken(TimbukScanner::tt_stat, "'States'");
		this->r_decl_list();
		this->scanner.assertToken(TimbukScanner::tt_fin, "'Final'");
		this->scanner.assertToken(TimbukScanner::tt_stat, "'States'");
		this->r_final_list();
	}
	
	// decl_list -> state_decl decl_list | eps
	void r_decl_list() {
		this->clearStates();
		while (this->scanner.peekToken() == TimbukScanner::tt_id)
			this->r_state_decl();
	}
	
	// state_decl -> ID | ID COL NUM
	void r_state_decl() {
		this->scanner.assertToken(TimbukScanner::tt_id, "identifier");
		std::string name = this->scanner.getVal();
		if (this->scanner.nextToken() == ':')
			this->scanner.assertToken(TimbukScanner::tt_int, "number");
		else
			this->scanner.freezeToken();
		this->newState(name, this->addState(name));
	}
	
	// final_list -> ID final_list | eps
	void r_final_list() {
		while (this->scanner.nextToken() == TimbukScanner::tt_id)
			this->newFinalState(this->getState(this->scanner.getVal()));
		this->scanner.freezeToken();
	}
	
	// transitions -> TRANS trans_list
	void r_transitions() {
		this->scanner.assertToken(TimbukScanner::tt_trans, "'Transitions'");
		this->r_trans_list();
	}
	
	// trans_list -> trans trans_list | eps
	void r_trans_list() {
		while (this->scanner.peekToken() == TimbukScanner::tt_id)
			this->r_trans();
	}
	
	// trans -> ID state_list_par ARR state
	void r_trans() {
		this->scanner.assertToken(TimbukScanner::tt_id, "identifier");
		pair<size_t, size_t> label = this->getLabel(this->scanner.getVal());
		vector<size_t> lhs;
		this->r_state_list_par(lhs);
		this->scanner.assertToken(TimbukScanner::tt_arr, "->");
		this->scanner.assertToken(TimbukScanner::tt_id, "identifier");
		if (lhs.size() != label.second)
			Error::general(this->scanner.getName(), this->scanner.getLine(), "label arity mismatch");
		this->newTransition(lhs, label.first, this->getState(this->scanner.getVal()));
	}	
	
	// state_list_par -> state_list | eps
	void r_state_list_par(vector<size_t>& lhs) {
		if (this->scanner.peekToken() == '(')
			this->r_state_list(lhs);
	}
	
	// state_list
	void r_state_list(vector<size_t>& lhs) {
		this->scanner.assertToken('(', "(");
		if (this->scanner.nextToken() == TimbukScanner::tt_id) {
			lhs.push_back(this->getState(this->scanner.getVal()));
			while (this->scanner.nextToken() == ',') {
				this->scanner.assertToken(TimbukScanner::tt_id, "identifier");
				lhs.push_back(this->getState(this->scanner.getVal()));			
			}
		}
		this->scanner.freezeToken();
		this->scanner.assertToken(')', ")");
	}

public:

	map<string, pair<size_t, size_t> > labels; // index, arity
	map<string, size_t> states; // index

protected:

	virtual void newLabel(const string& name, size_t arity, size_t id) = 0;
	virtual void beginModel(const string& name) = 0;
	virtual void newState(const string& name, size_t id) = 0;
	virtual void newFinalState(size_t id) = 0;
	virtual void endDeclaration() = 0;
	virtual void newTransition(const vector<size_t>& lhs, size_t label, size_t rhs) = 0;
	virtual void endModel() = 0;

public:

	TimbukReader(std::istream& input = std::cin, const string& name = "") {
		this->resetInput(input, name);
	}

	void resetInput(std::istream& input = std::cin, const string& name = "") {
		this->scanner = TimbukScanner(input, name);
	}

	void clearLabels() {
		this->labels.clear();
	}

	const std::pair<size_t, size_t>& getLabel(const string& name) const {
		map<string, pair<size_t, size_t> >::const_iterator i = this->labels.find(name);
		if (i == this->labels.end())
			Error::general(this->scanner.getName(), this->scanner.getLine(), "unknown label");
		return i->second;
	}

	int addLabel(const string& name, size_t arity) {
		map<string, pair<size_t, size_t> >::iterator i = this->labels.insert(
			make_pair(name, make_pair(this->labels.size(), arity))
		).first;
		if (i->second.second != arity)
			Error::general(this->scanner.getName(), this->scanner.getLine(), "(duplicated) label arity mismatch");
		return i->second.first;
	}
	
	void clearStates() {
		this->states.clear();
	}

	size_t getState(const string& name) const {
		map<string, size_t>::const_iterator i = this->states.find(name);
		if (i == this->states.end())
			Error::general(this->scanner.getName(), this->scanner.getLine(), "unknown state");
		return i->second;
	}

	size_t addState(const string& name) {
		return this->states.insert(make_pair(name, this->states.size())).first->second;
	}
	
	void run_simple() {
		this->r_simple();
	}

	void run_main() {
		this->r_main();
	}

};
#endif
