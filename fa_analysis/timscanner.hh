#ifndef TIMBUK_SCANNER_H
#define TIMBUK_SCANNER_H

#include <cstdio>
#include <istream>
#include <iostream>
#include <string>
#include <map>

#include "error.hh"

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

#endif
