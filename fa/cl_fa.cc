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
#include <ctime>
#include <signal.h>

// Code Listener headers
#include <cl/cl_msg.hh>
#include <cl/easy.hh>
#include "../cl/ssd.h"

// Forester headers
#include "notimpl_except.hh"
#include "programconfig.hh"
#include "streams.hh"
#include "symctx.hh"
#include "symexec.hh"

SymExec* se = nullptr;

void setDbgFlag(int) {
	if (se)
		se->setDbgFlag();
}

void userRequestHandler(int) {
	if (se)
		se->setUserRequestFlag();
}

// required by the gcc plug-in API
extern "C" {
    __attribute__ ((__visibility__ ("default"))) int plugin_is_GPL_compatible;
}

#if 0
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
#endif

void clEasyRun(const CodeStorage::Storage& stor, const char* configString)
{
	ssd::ColorConsole::enableForTerm(STDERR_FILENO);

#ifdef I_DONT_NEED_REPRODUCIBLE_RUNS_OF_FORESTER
	// initialize random numbers
	srandom(time(nullptr));
#endif

	using namespace CodeStorage;

	FA_LOG("configuration string: \"" << configString << "\"");

	// look for main() by name
	FA_LOG("looking for 'main()' at global scope...");
	const NameDb::TNameMap &glNames = stor.fncNames.glNames;
	const NameDb::TNameMap::const_iterator iter = glNames.find("main");
	if (glNames.end() == iter) {
		FA_ERROR("main() not found at global scope");
		return;
	}

	// initialize context
	SymCtx::initCtx(stor);

	// look for definition of main()
	const FncDb &fncs = stor.fncs;
	const Fnc *main = fncs[iter->second];
	if (!main || !isDefined(*main)) {
		FA_ERROR("main() not defined");
		return;
	}

	// parse the configuration string
	ProgramConfig conf(configString);

	// set signal handlers
	signal(SIGUSR1, setDbgFlag);
	signal(SIGUSR2, userRequestHandler);

	// set the debugging level
	Streams::setDebugLevelAsForCL();

	FA_LOG("starting verification ...");
	try
	{
		se = new SymExec(conf);

		FA_LOG("loading types ...");
		se->loadTypes(stor);

/*
		if (!conf.dbRoot.empty()){
			BoxDb db(conf.dbRoot, "index");
			se.loadBoxes(db.store);
		}
*/

		FA_LOG("compiling to microcode ...");
		se->compile(stor, *main);
		if (conf.printUcode)
		{
			FA_LOG("Printing microcode");
			std::ostringstream os;
			Compiler::Assembly::printUcode(os, se->GetAssembly().code_);
			Streams::ucode(os.str().c_str());
		}

		if (conf.printOrigCode)
		{
			FA_LOG("Printing input code");
			std::ostringstream os;
			Compiler::Assembly::printOrigCode(os, se->GetAssembly().code_);
			Streams::origCode(os.str().c_str());
		}

		if (!conf.onlyCompile)
		{
			FA_LOG("starting symbolic execution ...");
			se->run();
		}
	}
	catch (const NotImplementedException& e)
	{
		if (nullptr != e.location())
			FA_ERROR_MSG(e.location(), "not implemented: " << e.what());
		else
			FA_ERROR("not implemented: " + std::string(e.what()));
	} catch (const std::exception& e)
	{
		FA_ERROR(e.what());
	}

	delete se;

	FA_LOG("Forester finished.");
}
