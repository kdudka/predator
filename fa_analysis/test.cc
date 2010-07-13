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

#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "ufae.hh"

class Instr {

public:

	typedef enum {
		add_var = 0,
		rm_var = 1,
		x_ass_new = 2,
		del_x = 3,
		x_ass_null = 4,
		x_ass_y = 5,
		x_ass_y_next = 6,
		x_next_ass_y = 7,
		x_not_null = 8
	} op_code_type;

	static const char* op_code_names[];

private:

	op_code_type op_code;
	std::string dstId;
	size_t arg;
	std::string srcId;

	static size_t findVarId(const std::string& v, const std::map<std::string, size_t>& vars) {
		std::map<std::string, size_t>::const_iterator i = vars.find(v);
		assert(i != vars.end());
		return i->second;
	}
	
public:

	Instr(op_code_type op_code, const std::string& dstId, size_t arg = 0, const std::string& srcId = "<undef>")
		: op_code(op_code), dstId(dstId), arg(arg), srcId(srcId) {}
	
	void execute(std::vector<FAE*>& dst, std::map<std::string, size_t>& vars, const FAE* src) const {
		assert(src);
		switch (this->op_code) {
			case add_var: {
				size_t x;
				src->addVar(dst, x);
				assert(x == vars.size());
				assert(vars.insert(std::make_pair(this->dstId, x)).second);
				break;
			}
			case rm_var: {
				size_t x = Instr::findVarId(this->dstId, vars);
				vars.erase(this->dstId);
				assert(x == vars.size());
				src->dropVars(dst, 1);
				break;
			}
			case x_ass_new: {
				size_t x = Instr::findVarId(this->dstId, vars);
				src->x_ass_new(dst, x, this->arg, 0);
				break;
			}
			case del_x: {
				size_t x = Instr::findVarId(this->dstId, vars);
				src->del_x(dst, x);
				break;
			}
			case x_ass_null: {
				size_t x = Instr::findVarId(this->dstId, vars);
				src->x_ass_null(dst, x);
				break;
			}
			case x_ass_y: {
				size_t x = Instr::findVarId(this->dstId, vars);
				size_t y = Instr::findVarId(this->srcId, vars);
				src->x_ass_y(dst, x, y, this->arg);
				break;
			}
			case x_ass_y_next: {
				size_t x = Instr::findVarId(this->dstId, vars);
				size_t y = Instr::findVarId(this->srcId, vars);
				src->x_ass_y_next(dst, x, y, this->arg);
				break;
			}
			case x_next_ass_y: {
				size_t x = Instr::findVarId(this->dstId, vars);
				size_t y = Instr::findVarId(this->srcId, vars);
				src->x_next_ass_y(dst, x, y, this->arg);
				break;
			}
			case x_not_null: {
				size_t x = Instr::findVarId(this->dstId, vars);
				if (src->x_not_null(x))
					dst.push_back(new FAE(*src));
				break;
			}
		}
	}
	
	static size_t cmp(const std::string& dst, const std::string& src, const std::map<std::string, size_t>& vars, const std::vector<FAE*>& c) {
		size_t x = Instr::findVarId(dst, vars);
		size_t y = Instr::findVarId(src, vars);
		size_t res = (size_t)(-1);
		for (std::vector<FAE*>::const_iterator i = c.begin(); i != c.end(); ++i) {
			switch (res) {
				case (size_t)(-1): res = (*i)->x_eq_y(x, y)?(1):(0); break;
				case 0: if ((*i)->x_eq_y(x, y)) return -2; break;
				case 1: if (!(*i)->x_eq_y(x, y)) return -2; break;
			};
		}
		return res;
	}
	
	friend std::ostream& operator<<(std::ostream& os, const Instr& instr) {
		return os << Instr::op_code_names[instr.op_code] << '(' << instr.dstId << ',' << instr.arg << ',' << instr.srcId << ')';
	}
	
};

const char* Instr::op_code_names[] = {
	"add_var",
	"rm_var",
	"x_ass_new",
	"del_x",
	"x_ass_null",
	"x_ass_y",
	"x_ass_y_next",
	"x_next_ass_y",
	"x_not_null"
};

void sym_exec(std::vector<FAE*>& dst, std::map<std::string, size_t>& vars, const std::vector<FAE*>& src, const Instr& instr) {
	for (std::vector<FAE*>::const_iterator i = src.begin(); i != src.end(); ++i)
		instr.execute(dst, vars, *i);
	
}

int main(int argc, char* argv[]) {
	
	DEBUG_MSG("creating initial configuration ...");
	
	TA<FA::label_type>::Backend backend;
	TA<FA::label_type>::Manager taMan(backend);
	LabMan labMan;
	BoxManager boxMan(taMan, labMan);
	
	std::vector<std::vector<FAE*> > store = { std::vector<FAE*>({ new FAE(taMan, labMan, boxMan) }) };
	
	DEBUG_MSG("ready");
	
	std::vector<Instr> code = {
		Instr(Instr::add_var, "x"),
		Instr(Instr::add_var, "y"),
		Instr(Instr::x_ass_null, "x"),				// x = null
		Instr(Instr::x_ass_null, "y"),				// y = null
		Instr(Instr::x_ass_new, "y", 1),			// y = new(1)
		Instr(Instr::x_next_ass_y, "y", 0, "x"),	// y.next(0) = x
		Instr(Instr::x_ass_y, "x", 0, "y"),			// x = y
		Instr(Instr::x_ass_new, "y", 1),			// y = new(1)
		Instr(Instr::x_next_ass_y, "y", 0, "x"),	// y.next(0) = x
		Instr(Instr::x_ass_y, "x", 0, "y"),			// x = y
		Instr(Instr::x_ass_new, "y", 1),			// y = new(1)
		Instr(Instr::x_next_ass_y, "y", 0, "x"),	// y.next(0) = x
		Instr(Instr::x_ass_y, "x", 0, "y"),			// x = y
		Instr(Instr::add_var, "z"),
		Instr(Instr::x_ass_null, "z"),
		Instr(Instr::x_not_null, "x"),
		Instr(Instr::x_ass_y, "y", 0, "x"),			// y = x
		Instr(Instr::x_ass_y_next, "x", 0, "x"),	// x = x.next
		Instr(Instr::x_next_ass_y, "y", 0, "z"),	// y.next = z
		Instr(Instr::x_ass_y, "z", 0, "y"),			// z = x
		Instr(Instr::x_not_null, "x"),
		Instr(Instr::x_ass_y, "y", 0, "x"),			// y = x
		Instr(Instr::x_ass_y_next, "x", 0, "x"),	// x = x.next
		Instr(Instr::x_next_ass_y, "y", 0, "z"),	// y.next = z
		Instr(Instr::x_ass_y, "z", 0, "y")			// z = x

/*		Instr(Instr::x_ass_y_next, "x", 0, "y"),	// x.next(0) = y
		Instr(Instr::del_x, "y"),
		Instr(Instr::x_ass_y, "y", 0, "x"),
		Instr(Instr::x_ass_y_next, "x", 0, "y"),
		Instr(Instr::del_x, "y"),
		Instr(Instr::x_ass_y, "y", 0, "x"),
		Instr(Instr::x_ass_y_next, "x", 0, "y"),
		Instr(Instr::del_x, "y"),
		Instr(Instr::x_ass_y, "y", 0, "x"),
		Instr(Instr::rm_var, "y"),
		Instr(Instr::rm_var, "x")*/
	};

	std::map<std::string, size_t> vars;

	for (std::vector<Instr>::iterator i = code.begin(); i != code.end(); ++i) {
		store.push_back(std::vector<FAE*>());
		std::vector<FAE*>& oldC = *(&store.back() - 1);
		std::vector<FAE*>& newC = store.back();
		DEBUG_MSG("===");
		DEBUG_MSG("executing " << *i);
		sym_exec(newC, vars, oldC, *i);
		for (std::vector<FAE*>::iterator j = newC.begin(); j != newC.end(); ++j) {
			(*j)->doAbstraction();
			std::cout << **j;
		}
	}
/*	
	TA<FA::label_type> ta(backend), ta2(backend);
	UFAE ufae(ta2, labMan);
	Index<size_t> index;

	for (size_t i = 0; i < store[4].size(); ++i)
		ufae.fae2ta(ta, index, *store[4][i]);
	ufae.join(ta, index);
	ta.clear();
	index.clear();
	for (size_t i = 0; i < store[7].size(); ++i)
		ufae.fae2ta(ta, index, *store[7][i]);
	ufae.join(ta, index);
	ta.clear();
	index.clear();
	for (size_t i = 0; i < store[10].size(); ++i)
		ufae.fae2ta(ta, index, *store[10][i]);
	ufae.join(ta, index);
	ta.clear();
	index.clear();
	for (size_t i = 0; i < store[13].size(); ++i)
		ufae.fae2ta(ta, index, *store[13][i]);
	ufae.join(ta, index);
	ta.clear();
	index.clear();

	ta2.minimized(ta);
	
	DEBUG_MSG(ta);
*/
	DEBUG_MSG("cleanup");
	
	for (size_t i = 0; i < store.size(); ++i) {
		for (size_t j = 0; j < store[i].size(); ++j)
			delete store[i][j];
	}
	
	return 0;
}
