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

#ifndef TIMBUK_H
#define TIMBUK_H

#include <cstdio>
#include <istream>
#include <ostream>
#include <iostream>
#include <string>
#include <map>

#include "error.hh"

using std::string;
using std::vector;
using std::pair;
using std::make_pair;
using std::map;

class TimbukScanner {

	std::istream* input;
	std::string name;
	int token;
	std::string val;
	int lineno;
	bool frozen;
	
public:

	static const int tt_eof = 0x100;
	static const int tt_id = 0x101;
	static const int tt_int = 0x102;
	static const int tt_arr = 0x103;
	static const int tt_ops = 0x104;
	static const int tt_aut = 0x105;
	static const int tt_stat = 0x106;
	static const int tt_fin = 0x107;
	static const int tt_trans = 0x108;

private:

	std::map<std::string, int> keywords;

    void initKeywords() {
		this->keywords["Ops"] = tt_ops;
		this->keywords["Automaton"] = tt_aut;
		this->keywords["States"] = tt_stat;
		this->keywords["Final"] = tt_fin;
		this->keywords["Transitions"] = tt_trans;
	}
	
	int keywordLookup(const std::string& s) const {
		std::map<std::string, int>::const_iterator i = this->keywords.find(s);
		return (i != this->keywords.end())?(i->second):(tt_id);
	}

public:

	TimbukScanner(std::istream& input = std::cin, const std::string& name = "")
		: input(&input), name(name), lineno(1), frozen(false) {
		this->initKeywords();
	}

	int nextToken() {

		if (this->frozen) {
			this->frozen = false;
			return this->token;
		}

		this->val = "";

		for (int state = 0; ; ) {

			int c = this->input->get();

			switch (state) {
		
				case 0:

					switch (c) {

						case EOF: this->val = "EOF"; return this->token = tt_eof;
						case '\n': ++this->lineno; break;
						case '-': this->val += '-'; state = 1; break;
						default:
							
							if (isalpha(c) || c == '_') {
								this->val += c;
								state = 2;
							} else if (isdigit(c)) {
								this->val += c;
								state = 3;
							} else if (!isspace(c)) {
								this->val += c;
								return this->token = c;
							}

							break;
					
					}

					break;
					
				case 1:

					if (c != '>') {
						this->input->putback(c);
						return '-';
					}

					this->val += c;
					return this->token = tt_arr;

				case 2:

					if (!isalnum(c) && c != '_') {
						this->input->putback(c);
						return this->token = this->keywordLookup(this->val);
					}

					this->val += c;
					break;

				case 3:

					if (!isdigit(c)) {
						this->input->putback(c);
						return this->token = tt_int;
					}

					this->val += c;
					break;

			}

		}

	}

	int getToken() const { return this->token; }

	const std::string& getVal() const { return this->val; }

	int getLine() const { return this->lineno; }

	const std::string& getName() const { return this->name; }

	void assertToken(int token, const std::string& expected) {
		if (this->nextToken() != token)
			Error::tokenUnexpected(this->name, this->getLine(), expected, this->getVal());
	}

	void freezeToken() { this->frozen = true; }

	int peekToken() {
		this->nextToken();
		this->freezeToken();
		return this->token;
	}

};

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
	vector<string> labelNames;
	map<string, size_t> states; // index
	vector<string> stateNames;

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
		pair<map<string, pair<size_t, size_t> >::iterator, bool> p = this->labels.insert(
			make_pair(name, make_pair(this->labels.size(), arity))
		);
		if (p.first->second.second != arity)
			Error::general(this->scanner.getName(), this->scanner.getLine(), "(duplicated) label arity mismatch");
		if (p.second)
			this->labelNames.push_back(name);
		return p.first->second.first;
	}

	string getLabelName(size_t id) {
		return this->labelNames[id];
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
		pair<map<string, size_t>::iterator, bool> p = this->states.insert(make_pair(name, this->states.size()));
		if (p.second)
			this->stateNames.push_back(name);
		return p.first->second;
	}
	
	string getStateName(size_t id) {
		return this->stateNames[id];
	}

	void readFirst() {
		this->r_alph();
		this->r_aut();
	}

	bool readNext() {
		if (this->scanner.peekToken() != TimbukScanner::tt_aut)
			return false;
		this->r_aut();
		return true;
	}
	
	void readOne() {
		this->r_simple();
	}

	void readAll() {
		this->r_main();
	}

};

class TimbukWriter {

	std::ostream& out;
	
public:

	TimbukWriter(std::ostream& out) : out(out) {}
	
	void startAlphabet() {
		this->out << "Ops";
	}
	
	void writeLabel(size_t label, size_t arity) {
		this->out << " l" << label << ':' << arity;
	}

	void writeLabel(const std::string& label, size_t arity) {
		this->out << ' ' << label << ':' << arity;
	}
	
	void newModel(const std::string& name) {
		this->out << std::endl << "Automaton " << name;
	}
	
	void startStates() {
		this->out << std::endl << "States";
	}
	
	void writeState(size_t state) {
		this->out << " s" << state;
	}

	void writeState(const std::string& state) {
		this->out << ' ' << state;
	}
	
	void startFinalStates() {
		this->out << std::endl << "Final States";
	}
	
	void startTransitions() {
		this->out << std::endl << "Transitions";
	}
	
	void writeTransition(const std::vector<size_t>& lhs, size_t label, size_t rhs) {
		this->out << std::endl << 'l' << label << '(';
		if (lhs.size() > 0) {
			this->out << 's' << lhs[0];
			for (size_t i = 1; i < lhs.size(); ++i)
				this->out << ",s" << lhs[i];
		}
		this->out << ")->s" << rhs;
	}

	void writeTransition(const std::vector<size_t>& lhs, const std::string& label, size_t rhs) {
		this->out << std::endl << label << '(';
		if (lhs.size() > 0) {
			this->out << 's' << lhs[0];
			for (size_t i = 1; i < lhs.size(); ++i)
				this->out << ",s" << lhs[i];
		}
		this->out << ")->s" << rhs;
	}

	void writeTransition(const std::vector<string>& lhs, const std::string& label, const std::string& rhs) {
		this->out << std::endl << label << '(';
		if (lhs.size() > 0) {
			this->out << lhs[0];
			for (size_t i = 1; i < lhs.size(); ++i)
				this->out << ',' << lhs[i];
		}
		this->out << ")->" << rhs;
	}
	
	void terminate() {
		this->out << std::endl;
	}

};

#endif
