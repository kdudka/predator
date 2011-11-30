/*
 * Copyright (C) 2010 Jiri Simacek
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

#ifndef SYM_EXEC_H
#define SYM_EXEC_H

#include <unordered_map>

#include "compiler.hh"

namespace CodeStorage {
    struct Fnc;
    struct Storage;
}

class SymExec {

public:

	SymExec();
	~SymExec();

	void loadTypes(const CodeStorage::Storage& stor);
//	void loadBoxes(const std::unordered_map<std::string, std::string>& db);
	void compile(const CodeStorage::Storage &stor, const CodeStorage::Fnc& entry);
	void run();
	void setDbgFlag();

private:

	class Engine;
	Engine *engine;

};

#endif
