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

// Standard library headers
#include <vector>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <ctime>
#include <cstdlib>
#include <signal.h>

// Boost headers
#include <boost/algorithm/string.hpp>

// Code Listener headers
#include <cl/easy.hh>
#include "../cl/ssd.h"

// Forester headers
#include "symctx.hh"
#include "symexec.hh"
#include "programerror.hh"
#include "notimpl_except.hh"

SymExec se;

void setDbgFlag(int) {
	se.setDbgFlag();
}

// required by the gcc plug-in API
extern "C" { int plugin_is_GPL_compatible; }

struct Config {

	std::string dbRoot;

	void processArg(const std::string& key, const std::string& value) {
		if (key == "db-root")
			this->dbRoot = value;
	}

	Config(const std::string& c) {
		std::vector<std::string> args;
		boost::split(args, c, boost::is_from_range(';', ';'));
		for (std::vector<std::string>::iterator i = args.begin(); i != args.end(); ++i) {
			std::vector<std::string> data;
			boost::split(data, *i, boost::is_from_range(':', ':'));
			if (data.size() == 2)
				this->processArg(data[0], data[1]);
		}
	}

};
/*
struct BoxDb {

	std::unordered_map<std::string, std::string> store;

	BoxDb(const std::string& root, const std::string& fileName) {
		std::ifstream input((root + "/" + fileName).c_str());
		if (!input.good())
			throw std::runtime_error("Unable to open " + root + "/" + fileName);
		std::string buf;
		while (std::getline(input, buf)) {
			if (buf.empty())
				continue;
			if (buf[0] == '#')
				continue;
			std::vector<std::string> data;
			boost::split(data, buf, boost::is_from_range(':', ':'));
			if (data.size() == 2)
				this->store[data[0]] = root + "/" + data[1];
		}
	}

};
*/
void clEasyRun(const CodeStorage::Storage& stor, const char* configString) {

	ssd::ColorConsole::enableForTerm(STDERR_FILENO);
#ifdef I_DONT_NEED_REPRODUCIBLE_RUNS_OF_FORESTER
	// initialize random numbers
	srandom(time(NULL));
#endif
    using namespace CodeStorage;

	CL_DEBUG("config: " << configString);

    // look for main() by name
    CL_DEBUG("looking for 'main()' at gl scope...");
    const NameDb::TNameMap &glNames = stor.fncNames.glNames;
    const NameDb::TNameMap::const_iterator iter = glNames.find("main");
    if (glNames.end() == iter) {
        CL_ERROR("main() not found at global scope");
        return;
    }

	// initialize context
	SymCtx::initCtx(stor);

    // look for definition of main()
    const FncDb &fncs = stor.fncs;
    const Fnc *main = fncs[iter->second];
    if (!main || !isDefined(*main)) {
        CL_ERROR("main() not defined");
        return;
    }

    CL_DEBUG("starting verification stuff ...");
    try {
		signal(SIGUSR1, setDbgFlag);
		se.loadTypes(stor);
/*		Config c(configString);
		if (!c.dbRoot.empty()){
			BoxDb db(c.dbRoot, "index");
			se.loadBoxes(db.store);
		}*/
		se.compile(stor, *main);
		se.run();
		CL_NOTE("the program is safe ...");
	} catch (const ProgramError& e) {
		if (e.location())
			CL_ERROR_MSG(e.location(), e.what());
		else
			CL_ERROR(e.what());
	} catch (const NotImplementedException& e) {
		if (e.location())
			CL_ERROR_MSG(e.location(), "not implemented: " + std::string(e.what()));
		else
			CL_ERROR("not implemented: " + std::string(e.what()));
	} catch (const std::exception& e) {
		CL_ERROR(e.what());
	}

}
