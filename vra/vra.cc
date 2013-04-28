/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   vra.cc
* @brief  Performs value-range analysis.
* @date   2012
*/

#undef NDEBUG   // It is necessary for using assertions.

#include <iostream>
#include <boost/foreach.hpp>
#include <cl/easy.hh>

#include "ValueAnalysis.h"
#include "LoopFinder.h"
#include "GlobAnalysis.h"

// required by the gcc plug-in API
extern "C" {
    __attribute__ ((__visibility__ ("default"))) int plugin_is_GPL_compatible;
}

using CodeStorage::Fnc;
using CodeStorage::Storage;

void clEasyRun(const Storage &stor, const char *)
{
	LoopFinder::computeLoopAnalysis(stor);
	// LoopFinder::printLoopAnalysis(std::cout);

	GlobAnalysis::computeGlobAnalysis(stor);
	// GlobAnalysis::printGlobAnalysis(std::cout);

	BOOST_FOREACH(const Fnc* pFnc, stor.fncs) {
		const Fnc &fnc = *pFnc;

		if (!isDefined(fnc))
			continue;

		ValueAnalysis::computeAnalysisForFnc(fnc);
	}

	ValueAnalysis::printRanges(std::cout, stor);
}
